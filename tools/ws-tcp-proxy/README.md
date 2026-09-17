# ws-tcp-proxy

A WebSocket-to-TCP proxy for the WebAssembly build of the Mana client.

A browser cannot open raw TCP connections, so the wasm client talks to
tmwAthena through a WebSocket. This proxy accepts such a WebSocket and pipes its
binary frames to and from a TCP connection to a login, char or map server.

It started as a Python script and was ported to Rust for production use.

## Usage

    cargo build --release
    target/release/ws-tcp-proxy --listen 127.0.0.1:8765 --allow 127.0.0.1

Then open the client page with `?proxy=ws://127.0.0.1:8765/`.

Options:

- `--listen HOST:PORT`: address to listen on (default `127.0.0.1:8765`)
- `--allow HOST[:PORT]`: target that may be connected to, repeatable
  (default: `127.0.0.1` and `localhost`)
- `--max-connections N`: sessions that may be open at once (default 1000);
  further clients are closed with code 1013
- `--idle-timeout SECONDS`: close a session that carried no traffic for this
  long (default 0, which never times out)
- `--verbose`: debug logging, as does `RUST_LOG=debug`

The client hops from the login server to the char and map servers using the
addresses the server sends, so those addresses have to be allowed as well.

## Path scheme

The request path names the target: `/<host>/<port>`. Anything before those two
segments is ignored, so `/tmwa/<host>/<port>` works too, which is handy when the
proxy is mounted under a path by a front-end web server.

The `binary` subprotocol is accepted when the client offers it, and a client
that offers none is accepted as well.

A session that cannot start is closed right after the handshake:

- 1008: the path is not `/<host>/<port>`, or the target is not allowed
- 1011: the TCP connection to the target failed
- 1013: the connection limit has been reached

## Behind nginx

Terminate TLS in nginx and let it pass the upgrade through, so the client can
use `wss://`:

    location /tmwa/ {
        proxy_pass http://127.0.0.1:8765/;
        proxy_http_version 1.1;
        proxy_set_header Upgrade $http_upgrade;
        proxy_set_header Connection "upgrade";
        proxy_read_timeout 1h;
    }

## systemd unit

`/etc/systemd/system/ws-tcp-proxy.service`:

    [Unit]
    Description=WebSocket to TCP proxy for the Mana web client
    After=network.target

    [Service]
    ExecStart=/usr/local/bin/ws-tcp-proxy --listen 127.0.0.1:8765 \
        --allow server.themanaworld.org --idle-timeout 300
    Restart=on-failure
    DynamicUser=yes
    NoNewPrivileges=yes
    PrivateTmp=yes
    ProtectSystem=strict
    ProtectHome=yes
    RestrictAddressFamilies=AF_INET AF_INET6

    [Install]
    WantedBy=multi-user.target

Then `systemctl enable --now ws-tcp-proxy`. Logs go to the journal.
