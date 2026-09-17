use std::process::ExitCode;
use std::sync::Arc;
use std::time::Duration;

use clap::Parser;
use log::{error, info};
use tokio::net::TcpListener;
use ws_tcp_proxy::{parse_host_port, Allowlist, Proxy, DEFAULT_ALLOW};

/// Proxies WebSocket connections to TCP for the wasm client.
#[derive(Parser)]
#[command(version, about)]
struct Args {
    /// Address to listen on
    #[arg(long, value_name = "HOST:PORT", default_value = "127.0.0.1:8765")]
    listen: String,

    /// Target that may be connected to, repeatable
    /// (default: 127.0.0.1 and localhost)
    #[arg(long, value_name = "HOST[:PORT]")]
    allow: Vec<String>,

    /// Log every frame decision
    #[arg(long)]
    verbose: bool,

    /// Number of sessions that may be open at once
    #[arg(long, value_name = "N", default_value_t = 1000)]
    max_connections: usize,

    /// Close a session that carried no traffic for this long (0 disables it)
    #[arg(long, value_name = "SECONDS", default_value_t = 0)]
    idle_timeout: u64,
}

fn main() -> ExitCode {
    let args = Args::parse();

    let default_level = if args.verbose { "debug" } else { "info" };
    env_logger::Builder::from_env(env_logger::Env::default().default_filter_or(default_level))
        .format_timestamp_secs()
        .init();

    match run(args) {
        Ok(()) => ExitCode::SUCCESS,
        Err(error) => {
            error!("{error}");
            ExitCode::FAILURE
        }
    }
}

fn run(args: Args) -> Result<(), String> {
    let (listen_host, listen_port) = parse_host_port(&args.listen, Some(8765))?;
    let listen_port = listen_port.expect("a default port");

    let allow = if args.allow.is_empty() {
        Allowlist::parse(DEFAULT_ALLOW)?
    } else {
        Allowlist::parse(&args.allow)?
    };

    if args.max_connections == 0 {
        return Err("--max-connections must be at least 1".to_string());
    }

    let idle_timeout = match args.idle_timeout {
        0 => None,
        seconds => Some(Duration::from_secs(seconds)),
    };

    let runtime = tokio::runtime::Runtime::new().map_err(|error| error.to_string())?;
    runtime.block_on(async move {
        let listener = TcpListener::bind((listen_host.as_str(), listen_port))
            .await
            .map_err(|error| format!("cannot listen on {}: {error}", args.listen))?;

        let proxy = Arc::new(Proxy::new(allow, args.max_connections, idle_timeout));
        info!(
            "Listening on ws://{}:{}/, allowing {}",
            listen_host,
            listen_port,
            proxy.allow()
        );

        tokio::select! {
            _ = Arc::clone(&proxy).serve(listener) => {}
            reason = shutdown() => info!("Stopping on {reason}"),
        }

        Ok(())
    })
}

/// Waits for the first termination signal and names it.
async fn shutdown() -> &'static str {
    #[cfg(unix)]
    {
        use tokio::signal::unix::{signal, SignalKind};

        let mut terminate = match signal(SignalKind::terminate()) {
            Ok(terminate) => terminate,
            Err(_) => {
                let _ = tokio::signal::ctrl_c().await;
                return "SIGINT";
            }
        };

        tokio::select! {
            _ = terminate.recv() => "SIGTERM",
            _ = tokio::signal::ctrl_c() => "SIGINT",
        }
    }

    #[cfg(not(unix))]
    {
        let _ = tokio::signal::ctrl_c().await;
        "interrupt"
    }
}
