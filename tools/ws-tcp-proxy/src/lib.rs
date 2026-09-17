//! WebSocket-to-TCP proxy for the WebAssembly build of Mana.
//!
//! A browser cannot open raw TCP connections, so the wasm client talks to
//! tmwAthena through a WebSocket. This proxy accepts such a WebSocket and pipes
//! its binary frames to and from a TCP connection.
//!
//! The request path names the target: `/<host>/<port>`. Anything before those
//! two segments is ignored, so `/tmwa/<host>/<port>` works too, which is handy
//! when the proxy is mounted under a path by a front-end web server.
//!
//! Only hosts on the allowlist can be reached. The client hops from the login
//! server to the char and map servers using the addresses the server sends, so
//! those addresses have to be allowed as well.

use std::fmt;
use std::io;
use std::net::SocketAddr;
use std::sync::atomic::{AtomicU64, AtomicUsize, Ordering};
use std::sync::Arc;
use std::time::Duration;

use futures_util::{SinkExt, StreamExt};
use log::{debug, info, warn};
use tokio::io::{AsyncReadExt, AsyncWriteExt};
use tokio::net::{TcpListener, TcpStream};
use tokio::time::timeout;
use tokio_tungstenite::tungstenite::handshake::server::{Request, Response};
use tokio_tungstenite::tungstenite::http::header::{HeaderValue, SEC_WEBSOCKET_PROTOCOL};
use tokio_tungstenite::tungstenite::protocol::frame::coding::CloseCode;
use tokio_tungstenite::tungstenite::protocol::CloseFrame;
use tokio_tungstenite::tungstenite::{Error as WsError, Message};
use tokio_tungstenite::WebSocketStream;

/// Size of a single TCP read, and so the largest binary frame sent to a client.
const BUFFER_SIZE: usize = 65536;

/// Targets that may be reached when no --allow option is given.
pub const DEFAULT_ALLOW: [&str; 2] = ["127.0.0.1", "localhost"];

/// Splits "host" or "host:port" into a (host, port) pair.
pub fn parse_host_port(
    value: &str,
    default_port: Option<u16>,
) -> Result<(String, Option<u16>), String> {
    if let Some(rest) = value.strip_prefix('[') {
        // IPv6 literal in brackets, optionally followed by :port
        let end = rest
            .find(']')
            .ok_or_else(|| format!("unterminated IPv6 address: {value}"))?;
        let host = rest[..end].to_string();
        let tail = &rest[end + 1..];
        let port = match tail.strip_prefix(':') {
            Some(port) => Some(parse_port(port)?),
            None => default_port,
        };
        return Ok((host, port));
    }

    if value.matches(':').count() == 1 {
        let (host, port) = value.split_once(':').expect("one colon");
        return Ok((host.to_string(), Some(parse_port(port)?)));
    }

    Ok((value.to_string(), default_port))
}

fn parse_port(value: &str) -> Result<u16, String> {
    match value.parse::<u16>() {
        Ok(port) if port > 0 => Ok(port),
        _ => Err(format!("not a port number: {value}")),
    }
}

/// Decides which target addresses may be connected to.
#[derive(Clone, Debug, Default)]
pub struct Allowlist {
    entries: Vec<(String, Option<u16>)>,
}

impl Allowlist {
    /// Parses "host" and "host:port" entries.
    pub fn parse<I, S>(entries: I) -> Result<Self, String>
    where
        I: IntoIterator<Item = S>,
        S: AsRef<str>,
    {
        let entries = entries
            .into_iter()
            .map(|entry| parse_host_port(entry.as_ref(), None))
            .collect::<Result<Vec<_>, _>>()?;
        Ok(Self { entries })
    }

    pub fn allows(&self, host: &str, port: u16) -> bool {
        self.entries.iter().any(|(allowed_host, allowed_port)| {
            allowed_host.eq_ignore_ascii_case(host)
                && allowed_port.is_none_or(|allowed| allowed == port)
        })
    }
}

impl fmt::Display for Allowlist {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        for (index, (host, port)) in self.entries.iter().enumerate() {
            if index > 0 {
                f.write_str(", ")?;
            }
            match port {
                Some(port) => write!(f, "{host}:{port}")?,
                None => f.write_str(host)?,
            }
        }
        Ok(())
    }
}

/// Returns the (host, port) named by a request path, or None.
///
/// Only the last two segments matter, so a path prefix added by a reverse
/// proxy is ignored.
pub fn parse_target(path: &str) -> Option<(String, u16)> {
    let path = path.split('?').next().unwrap_or(path);
    let segments: Vec<&str> = path.split('/').filter(|s| !s.is_empty()).collect();
    if segments.len() < 2 {
        return None;
    }

    let host = segments[segments.len() - 2];
    if host.is_empty() || !host.bytes().all(is_host_byte) {
        return None;
    }

    let port = segments[segments.len() - 1].parse::<u16>().ok()?;
    if port == 0 {
        return None;
    }

    Some((host.to_string(), port))
}

fn is_host_byte(byte: u8) -> bool {
    byte.is_ascii_alphanumeric() || matches!(byte, b'.' | b'-' | b':')
}

/// A running proxy: its allowlist, its limits and its session bookkeeping.
pub struct Proxy {
    allow: Allowlist,
    max_connections: usize,
    idle_timeout: Option<Duration>,
    active: AtomicUsize,
    next_id: AtomicU64,
}

impl Proxy {
    pub fn new(allow: Allowlist, max_connections: usize, idle_timeout: Option<Duration>) -> Self {
        Self {
            allow,
            max_connections,
            idle_timeout,
            active: AtomicUsize::new(0),
            next_id: AtomicU64::new(1),
        }
    }

    pub fn allow(&self) -> &Allowlist {
        &self.allow
    }

    /// Accepts connections until the listener fails. Each session runs in its
    /// own task, so dropping this future drops the sessions with it.
    pub async fn serve(self: Arc<Self>, listener: TcpListener) {
        loop {
            let (stream, peer) = match listener.accept().await {
                Ok(accepted) => accepted,
                Err(error) => {
                    warn!("accept failed: {error}");
                    continue;
                }
            };

            let proxy = Arc::clone(&self);
            tokio::spawn(async move { proxy.handle(stream, peer).await });
        }
    }

    // The handshake callback returns tungstenite's own error response type,
    // which is a large one, so that lint has nothing to work with here.
    #[allow(clippy::result_large_err)]
    async fn handle(self: Arc<Self>, stream: TcpStream, peer: SocketAddr) {
        let id = format!("{:08x}", self.next_id.fetch_add(1, Ordering::Relaxed));
        let session = Session::open(&self);

        // The path is only known once the request has been read, so it is
        // picked up here and acted on after the upgrade: the client tests the
        // close code, which needs a WebSocket to arrive on.
        let mut path = String::new();
        let mut subprotocol = false;
        let callback = |request: &Request, mut response: Response| {
            path = request.uri().path().to_string();
            subprotocol = offers_binary(request);
            if subprotocol {
                response
                    .headers_mut()
                    .insert(SEC_WEBSOCKET_PROTOCOL, HeaderValue::from_static("binary"));
            }
            Ok(response)
        };

        let mut websocket = match tokio_tungstenite::accept_hdr_async(stream, callback).await {
            Ok(websocket) => websocket,
            Err(error) => {
                debug!("{id}: handshake with {peer} failed: {error}");
                return;
            }
        };

        if session.is_none() {
            warn!(
                "{id}: refusing {peer}, {} sessions are open",
                self.max_connections
            );
            close(&mut websocket, CloseCode::Again, "Try again later").await;
            return;
        }

        let Some((host, port)) = parse_target(&path) else {
            warn!("{id}: bad request path {path:?}");
            close(
                &mut websocket,
                CloseCode::Policy,
                "Expected a /<host>/<port> path",
            )
            .await;
            return;
        };

        if !self.allow.allows(&host, port) {
            warn!("{id}: refused target {host}:{port}");
            close(&mut websocket, CloseCode::Policy, "Target not allowed").await;
            return;
        }

        info!("{id}: connecting to {host}:{port}");

        let tcp = match TcpStream::connect((host.as_str(), port)).await {
            Ok(tcp) => tcp,
            Err(error) => {
                warn!("{id}: cannot reach {host}:{port}: {error}");
                close(
                    &mut websocket,
                    CloseCode::Error,
                    &format!("Cannot reach {host}:{port}"),
                )
                .await;
                return;
            }
        };

        if let Err(error) = tcp.set_nodelay(true) {
            debug!("{id}: cannot set TCP_NODELAY: {error}");
        }

        info!("{id}: connected to {host}:{port}");

        let session = pump(websocket, tcp, &id, self.idle_timeout).await;

        debug!(
            "{id}: {} bytes to {host}:{port}, {} bytes back",
            session.to_tcp, session.to_websocket
        );
        info!("{id}: closed {host}:{port}: {}", session.reason);
    }

    /// Number of sessions that are open right now.
    pub fn active(&self) -> usize {
        self.active.load(Ordering::Relaxed)
    }

    /// Claims a session slot, unless the limit has been reached.
    fn take_slot(&self) -> bool {
        let mut active = self.active.load(Ordering::Relaxed);
        loop {
            if active >= self.max_connections {
                return false;
            }
            match self.active.compare_exchange_weak(
                active,
                active + 1,
                Ordering::AcqRel,
                Ordering::Relaxed,
            ) {
                Ok(_) => return true,
                Err(current) => active = current,
            }
        }
    }
}

/// Keeps the session count up to date for as long as it lives.
struct Session<'a>(&'a AtomicUsize);

impl<'a> Session<'a> {
    fn open(proxy: &'a Proxy) -> Option<Self> {
        proxy.take_slot().then(|| Session(&proxy.active))
    }
}

impl Drop for Session<'_> {
    fn drop(&mut self) {
        self.0.fetch_sub(1, Ordering::Relaxed);
    }
}

fn offers_binary(request: &Request) -> bool {
    request
        .headers()
        .get_all(SEC_WEBSOCKET_PROTOCOL)
        .iter()
        .filter_map(|value| value.to_str().ok())
        .flat_map(|value| value.split(','))
        .any(|protocol| protocol.trim().eq_ignore_ascii_case("binary"))
}

async fn close<S>(websocket: &mut WebSocketStream<S>, code: CloseCode, reason: &str)
where
    S: tokio::io::AsyncRead + tokio::io::AsyncWrite + Unpin,
{
    let frame = CloseFrame {
        code,
        reason: reason.into(),
    };
    if websocket.close(Some(frame)).await.is_err() {
        return;
    }
    // Let the peer read the close frame and answer before the socket goes
    // away, but do not wait on a client that stays silent.
    let drain = async { while websocket.next().await.is_some() {} };
    let _ = timeout(Duration::from_secs(5), drain).await;
}

/// How a session ended, and how much traffic it carried.
struct Closed {
    reason: String,
    to_tcp: u64,
    to_websocket: u64,
}

/// What the session is waiting for.
enum Event {
    WebSocket(Option<Result<Message, WsError>>),
    Tcp(io::Result<usize>),
}

/// Why the pump stopped.
enum Stop {
    WebSocketClosed,
    TcpClosed,
    Idle,
    Failed(String),
}

/// Copies bytes in both directions until either side closes.
async fn pump(
    websocket: WebSocketStream<TcpStream>,
    tcp: TcpStream,
    id: &str,
    idle_timeout: Option<Duration>,
) -> Closed {
    let (mut websocket_tx, mut websocket_rx) = websocket.split();
    let (mut tcp_rx, mut tcp_tx) = tcp.into_split();
    let mut buffer = vec![0u8; BUFFER_SIZE];
    let mut to_tcp = 0u64;
    let mut to_websocket = 0u64;

    let stop = loop {
        // The borrow of the buffer ends with this block, which leaves the
        // bytes that were read free to be forwarded below.
        let event = {
            let step = async {
                tokio::select! {
                    message = websocket_rx.next() => Event::WebSocket(message),
                    read = tcp_rx.read(&mut buffer) => Event::Tcp(read),
                }
            };
            match idle_timeout {
                Some(limit) => match timeout(limit, step).await {
                    Ok(event) => event,
                    Err(_) => break Stop::Idle,
                },
                None => step.await,
            }
        };

        match event {
            Event::WebSocket(None) => break Stop::WebSocketClosed,
            Event::WebSocket(Some(Ok(message))) => match message {
                Message::Binary(data) => {
                    to_tcp += data.len() as u64;
                    if let Err(error) = tcp_tx.write_all(&data).await {
                        break Stop::Failed(format!("write failed: {error}"));
                    }
                }
                Message::Text(text) => {
                    warn!(
                        "{id}: ignoring text frame of {} characters",
                        text.chars().count()
                    );
                }
                Message::Ping(_) => {
                    // Reading queued the pong, flushing puts it on the wire.
                    if websocket_tx.flush().await.is_err() {
                        break Stop::WebSocketClosed;
                    }
                }
                Message::Close(_) => break Stop::WebSocketClosed,
                Message::Pong(_) | Message::Frame(_) => {}
            },
            Event::WebSocket(Some(Err(error))) => match error {
                WsError::ConnectionClosed | WsError::AlreadyClosed => break Stop::WebSocketClosed,
                error => break Stop::Failed(format!("websocket failed: {error}")),
            },
            Event::Tcp(Ok(0)) => break Stop::TcpClosed,
            Event::Tcp(Ok(read)) => {
                to_websocket += read as u64;
                let frame = Message::binary(buffer[..read].to_vec());
                if websocket_tx.send(frame).await.is_err() {
                    break Stop::WebSocketClosed;
                }
            }
            Event::Tcp(Err(error)) => break Stop::Failed(format!("read failed: {error}")),
        }
    };

    let _ = tcp_tx.shutdown().await;

    let (code, reason) = match stop {
        Stop::WebSocketClosed => (CloseCode::Normal, "Connection closed".to_string()),
        Stop::TcpClosed => (CloseCode::Normal, "Connection closed".to_string()),
        Stop::Idle => (CloseCode::Normal, "Idle timeout".to_string()),
        Stop::Failed(reason) => (CloseCode::Error, reason),
    };

    let frame = CloseFrame {
        code,
        reason: reason.as_str().into(),
    };
    let _ = websocket_tx.send(Message::Close(Some(frame))).await;
    let _ = websocket_tx.close().await;

    Closed {
        reason,
        to_tcp,
        to_websocket,
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn parses_targets() {
        assert_eq!(
            parse_target("/127.0.0.1/6901"),
            Some(("127.0.0.1".into(), 6901))
        );
        assert_eq!(
            parse_target("/tmwa/server.org/6901"),
            Some(("server.org".into(), 6901))
        );
        assert_eq!(parse_target("/a/b/host/1?x=1"), Some(("host".into(), 1)));
        assert_eq!(parse_target("/6901"), None);
        assert_eq!(parse_target("/host/port"), None);
        assert_eq!(parse_target("/host/0"), None);
        assert_eq!(parse_target("/host/65536"), None);
        assert_eq!(parse_target("/ho st/1"), None);
        assert_eq!(parse_target("/host%2f/1"), None);
        assert_eq!(parse_target("/ho_st/1"), None);
    }

    #[test]
    fn parses_host_and_port() {
        assert_eq!(parse_host_port("host", None), Ok(("host".into(), None)));
        assert_eq!(
            parse_host_port("host", Some(8765)),
            Ok(("host".into(), Some(8765)))
        );
        assert_eq!(
            parse_host_port("host:1", None),
            Ok(("host".into(), Some(1)))
        );
        assert_eq!(
            parse_host_port("[::1]:1", None),
            Ok(("::1".into(), Some(1)))
        );
        assert_eq!(
            parse_host_port("[::1]", Some(2)),
            Ok(("::1".into(), Some(2)))
        );
        assert!(parse_host_port("host:x", None).is_err());
        assert!(parse_host_port("[::1", None).is_err());
    }

    #[test]
    fn applies_the_allowlist() {
        let allow = Allowlist::parse(["Localhost", "10.0.0.1:6901"]).unwrap();
        assert!(allow.allows("localhost", 1));
        assert!(allow.allows("LOCALHOST", 2));
        assert!(allow.allows("10.0.0.1", 6901));
        assert!(!allow.allows("10.0.0.1", 6902));
        assert!(!allow.allows("10.0.0.2", 6901));
        assert_eq!(allow.to_string(), "Localhost, 10.0.0.1:6901");
    }
}
