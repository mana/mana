//! End-to-end tests: a proxy, a TCP echo server and a WebSocket client, all
//! in-process on ephemeral ports.

use std::sync::Arc;
use std::time::Duration;

use futures_util::{SinkExt, StreamExt};
use tokio::io::{AsyncReadExt, AsyncWriteExt};
use tokio::net::TcpListener;
use tokio::net::TcpStream;
use tokio_tungstenite::tungstenite::client::IntoClientRequest;
use tokio_tungstenite::tungstenite::http::header::SEC_WEBSOCKET_PROTOCOL;
use tokio_tungstenite::tungstenite::http::HeaderValue;
use tokio_tungstenite::tungstenite::Message;
use tokio_tungstenite::WebSocketStream;
use ws_tcp_proxy::{Allowlist, Proxy};

/// Starts a proxy on an ephemeral port and returns that port.
async fn start_proxy(allow: &[&str], max_connections: usize) -> u16 {
    start_proxy_with(allow, max_connections, None).await
}

async fn start_proxy_with(
    allow: &[&str],
    max_connections: usize,
    idle_timeout: Option<Duration>,
) -> u16 {
    let listener = TcpListener::bind("127.0.0.1:0").await.unwrap();
    let port = listener.local_addr().unwrap().port();
    let allow = Allowlist::parse(allow.iter().copied()).unwrap();
    let proxy = Arc::new(Proxy::new(allow, max_connections, idle_timeout));
    tokio::spawn(proxy.serve(listener));
    port
}

/// Starts a TCP server that echoes everything it receives.
async fn start_echo() -> u16 {
    let listener = TcpListener::bind("127.0.0.1:0").await.unwrap();
    let port = listener.local_addr().unwrap().port();
    tokio::spawn(async move {
        while let Ok((mut stream, _)) = listener.accept().await {
            tokio::spawn(async move {
                let mut buffer = [0u8; 4096];
                loop {
                    match stream.read(&mut buffer).await {
                        Ok(0) | Err(_) => return,
                        Ok(read) => {
                            if stream.write_all(&buffer[..read]).await.is_err() {
                                return;
                            }
                        }
                    }
                }
            });
        }
    });
    port
}

/// Returns a port that nothing is listening on.
async fn closed_port() -> u16 {
    let listener = TcpListener::bind("127.0.0.1:0").await.unwrap();
    listener.local_addr().unwrap().port()
}

/// Opens a WebSocket to the proxy, offering the "binary" subprotocol.
async fn connect(proxy_port: u16, path: &str) -> (WebSocketStream<TcpStream>, Option<String>) {
    let mut request = format!("ws://127.0.0.1:{proxy_port}{path}")
        .into_client_request()
        .unwrap();
    request
        .headers_mut()
        .insert(SEC_WEBSOCKET_PROTOCOL, HeaderValue::from_static("binary"));

    let stream = TcpStream::connect(("127.0.0.1", proxy_port)).await.unwrap();
    let (websocket, response) = tokio_tungstenite::client_async(request, stream)
        .await
        .unwrap();
    let subprotocol = response
        .headers()
        .get(SEC_WEBSOCKET_PROTOCOL)
        .map(|value| value.to_str().unwrap().to_string());
    (websocket, subprotocol)
}

/// Reads until the close frame arrives and returns its code.
async fn close_code(websocket: &mut WebSocketStream<TcpStream>) -> u16 {
    let wait = async {
        while let Some(message) = websocket.next().await {
            match message {
                Ok(Message::Close(Some(frame))) => return u16::from(frame.code),
                Ok(Message::Close(None)) => panic!("close frame without a code"),
                Ok(_) => continue,
                Err(error) => panic!("websocket failed: {error}"),
            }
        }
        panic!("the connection ended without a close frame");
    };
    tokio::time::timeout(Duration::from_secs(5), wait)
        .await
        .expect("timed out waiting for the close frame")
}

#[tokio::test]
async fn echoes_binary_frames() {
    let echo_port = start_echo().await;
    let proxy_port = start_proxy(&["127.0.0.1"], 1000).await;

    let (mut websocket, subprotocol) =
        connect(proxy_port, &format!("/127.0.0.1/{echo_port}")).await;
    assert_eq!(subprotocol.as_deref(), Some("binary"));

    websocket
        .send(Message::binary(b"hello tmwa".to_vec()))
        .await
        .unwrap();

    let received = tokio::time::timeout(Duration::from_secs(5), websocket.next())
        .await
        .expect("timed out")
        .expect("the connection ended")
        .unwrap();
    assert_eq!(received, Message::binary(b"hello tmwa".to_vec()));

    websocket.close(None).await.unwrap();
}

#[tokio::test]
async fn ignores_a_path_prefix() {
    let echo_port = start_echo().await;
    let proxy_port = start_proxy(&["127.0.0.1"], 1000).await;

    let (mut websocket, _) = connect(proxy_port, &format!("/tmwa/127.0.0.1/{echo_port}")).await;
    websocket
        .send(Message::binary(vec![1, 2, 3]))
        .await
        .unwrap();

    let received = websocket.next().await.unwrap().unwrap();
    assert_eq!(received, Message::binary(vec![1, 2, 3]));
}

#[tokio::test]
async fn forwards_without_a_subprotocol() {
    let echo_port = start_echo().await;
    let proxy_port = start_proxy(&["127.0.0.1"], 1000).await;

    let request = format!("ws://127.0.0.1:{proxy_port}/127.0.0.1/{echo_port}")
        .into_client_request()
        .unwrap();
    let stream = TcpStream::connect(("127.0.0.1", proxy_port)).await.unwrap();
    let (mut websocket, response) = tokio_tungstenite::client_async(request, stream)
        .await
        .unwrap();
    assert!(response.headers().get(SEC_WEBSOCKET_PROTOCOL).is_none());

    websocket.send(Message::binary(vec![7])).await.unwrap();
    assert_eq!(
        websocket.next().await.unwrap().unwrap(),
        Message::binary(vec![7])
    );
}

#[tokio::test]
async fn closes_the_websocket_when_the_target_disconnects() {
    let listener = TcpListener::bind("127.0.0.1:0").await.unwrap();
    let target_port = listener.local_addr().unwrap().port();
    tokio::spawn(async move {
        let (stream, _) = listener.accept().await.unwrap();
        drop(stream);
    });

    let proxy_port = start_proxy(&["127.0.0.1"], 1000).await;
    let (mut websocket, _) = connect(proxy_port, &format!("/127.0.0.1/{target_port}")).await;
    assert_eq!(close_code(&mut websocket).await, 1000);
}

#[tokio::test]
async fn rejects_a_bad_path() {
    let proxy_port = start_proxy(&["127.0.0.1"], 1000).await;
    let (mut websocket, _) = connect(proxy_port, "/nonsense").await;
    assert_eq!(close_code(&mut websocket).await, 1008);
}

#[tokio::test]
async fn rejects_a_host_that_is_not_allowed() {
    let echo_port = start_echo().await;
    let proxy_port = start_proxy(&["10.0.0.1"], 1000).await;
    let (mut websocket, _) = connect(proxy_port, &format!("/127.0.0.1/{echo_port}")).await;
    assert_eq!(close_code(&mut websocket).await, 1008);
}

#[tokio::test]
async fn rejects_a_port_that_is_not_allowed() {
    let echo_port = start_echo().await;
    let proxy_port = start_proxy(&["127.0.0.1:1"], 1000).await;
    let (mut websocket, _) = connect(proxy_port, &format!("/127.0.0.1/{echo_port}")).await;
    assert_eq!(close_code(&mut websocket).await, 1008);
}

#[tokio::test]
async fn reports_an_unreachable_target() {
    let port = closed_port().await;
    let proxy_port = start_proxy(&["127.0.0.1"], 1000).await;
    let (mut websocket, _) = connect(proxy_port, &format!("/127.0.0.1/{port}")).await;
    assert_eq!(close_code(&mut websocket).await, 1011);
}

#[tokio::test]
async fn closes_an_idle_session() {
    let echo_port = start_echo().await;
    let proxy_port = start_proxy_with(&["127.0.0.1"], 1000, Some(Duration::from_millis(300))).await;

    let (mut websocket, _) = connect(proxy_port, &format!("/127.0.0.1/{echo_port}")).await;
    assert_eq!(close_code(&mut websocket).await, 1000);
}

#[tokio::test]
async fn refuses_more_than_the_connection_limit() {
    let echo_port = start_echo().await;
    let proxy_port = start_proxy(&["127.0.0.1"], 1).await;

    let (mut first, _) = connect(proxy_port, &format!("/127.0.0.1/{echo_port}")).await;
    first.send(Message::binary(vec![1])).await.unwrap();
    assert_eq!(
        first.next().await.unwrap().unwrap(),
        Message::binary(vec![1])
    );

    let (mut second, _) = connect(proxy_port, &format!("/127.0.0.1/{echo_port}")).await;
    assert_eq!(close_code(&mut second).await, 1013);
}
