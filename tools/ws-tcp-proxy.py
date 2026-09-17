#!/usr/bin/env python3

"""Development WebSocket-to-TCP proxy for the WebAssembly build of Mana.

A browser cannot open raw TCP connections, so the wasm client talks to
tmwAthena through a WebSocket. This proxy accepts such a WebSocket and pipes
its binary frames to and from a TCP connection.

The request path names the target: /<host>/<port>. Anything before those two
segments is ignored, so /tmwa/<host>/<port> works too, which is handy when the
proxy is mounted under a path by a front-end web server.

Only hosts on the allowlist can be reached. The client hops from the login
server to the char and map servers using the addresses the server sends, so
those addresses have to be allowed as well.

Usage:

    python3 tools/ws-tcp-proxy.py --listen 127.0.0.1:8765 --allow 127.0.0.1

Then open the client page with ?proxy=ws://127.0.0.1:8765/

Requires the "websockets" package (version 14 or newer).
"""

import argparse
import asyncio
import logging
import signal

from websockets.asyncio.server import serve
from websockets.exceptions import ConnectionClosed

BUFFER_SIZE = 65536

DEFAULT_ALLOW = ["127.0.0.1", "localhost"]

logger = logging.getLogger("ws-tcp-proxy")


def parse_host_port(value, default_port=None):
    """Splits "host" or "host:port" into a (host, port) tuple."""
    if value.startswith("["):
        # IPv6 literal in brackets, optionally followed by :port
        end = value.find("]")
        if end < 0:
            raise ValueError(f"unterminated IPv6 address: {value}")
        host = value[1:end]
        rest = value[end + 1:]
        port = int(rest[1:]) if rest.startswith(":") else default_port
        return host, port

    if value.count(":") == 1:
        host, _, port = value.partition(":")
        return host, int(port)

    return value, default_port


class Allowlist:
    """Decides which target addresses may be connected to."""

    def __init__(self, entries):
        self.entries = [parse_host_port(entry) for entry in entries]

    def allows(self, host, port):
        for allowed_host, allowed_port in self.entries:
            if allowed_host.lower() != host.lower():
                continue
            if allowed_port is None or allowed_port == port:
                return True
        return False

    def __str__(self):
        return ", ".join(
            host if port is None else f"{host}:{port}"
            for host, port in self.entries
        )


def parse_target(path):
    """Returns the (host, port) named by a request path, or None."""
    path = path.split("?", 1)[0]
    segments = [segment for segment in path.split("/") if segment]
    if len(segments) < 2:
        return None

    host = segments[-2]
    try:
        port = int(segments[-1])
    except ValueError:
        return None

    if not 0 < port < 65536:
        return None

    return host, port


async def tcp_to_websocket(reader, websocket):
    while True:
        data = await reader.read(BUFFER_SIZE)
        if not data:
            logger.debug("%s: server closed the connection", websocket.id)
            return
        await websocket.send(data)


async def websocket_to_tcp(websocket, writer):
    async for message in websocket:
        if isinstance(message, str):
            logger.warning("%s: ignoring text frame of %d characters",
                           websocket.id, len(message))
            continue
        writer.write(message)
        await writer.drain()

    logger.debug("%s: client closed the connection", websocket.id)


async def pump(websocket, reader, writer):
    """Copies bytes in both directions until either side closes."""
    tasks = [
        asyncio.create_task(tcp_to_websocket(reader, websocket)),
        asyncio.create_task(websocket_to_tcp(websocket, writer)),
    ]

    done, pending = await asyncio.wait(tasks,
                                       return_when=asyncio.FIRST_COMPLETED)

    for task in pending:
        task.cancel()
    await asyncio.gather(*pending, return_exceptions=True)

    for task in done:
        error = task.exception()
        if error is not None and not isinstance(error, ConnectionClosed):
            raise error


def select_subprotocol(connection, subprotocols):
    """Accepts "binary" when offered, and no subprotocol otherwise.

    The default would reject a client that offers no subprotocol at all.
    """
    return "binary" if "binary" in subprotocols else None


def make_handler(allowlist):
    async def handle(websocket):
        path = websocket.request.path
        target = parse_target(path)
        if target is None:
            logger.warning("%s: bad request path %r", websocket.id, path)
            await websocket.close(1008, "Expected a /<host>/<port> path")
            return

        host, port = target
        if not allowlist.allows(host, port):
            logger.warning("%s: refused target %s:%d", websocket.id, host, port)
            await websocket.close(1008, "Target not allowed")
            return

        logger.info("%s: connecting to %s:%d", websocket.id, host, port)

        try:
            reader, writer = await asyncio.open_connection(host, port)
        except OSError as error:
            logger.warning("%s: cannot reach %s:%d: %s",
                           websocket.id, host, port, error)
            await websocket.close(1011, f"Cannot reach {host}:{port}")
            return

        logger.info("%s: connected to %s:%d", websocket.id, host, port)

        try:
            await pump(websocket, reader, writer)
        except Exception as error:
            logger.warning("%s: %s:%d failed: %s",
                           websocket.id, host, port, error)
            await websocket.close(1011, "Proxy error")
        else:
            await websocket.close(1000, "Connection closed")
        finally:
            writer.close()
            try:
                await writer.wait_closed()
            except OSError:
                pass
            logger.info("%s: closed %s:%d", websocket.id, host, port)

    return handle


async def run(listen_host, listen_port, allowlist):
    async with serve(make_handler(allowlist), listen_host, listen_port,
                     subprotocols=["binary"],
                     select_subprotocol=select_subprotocol):
        logger.info("Listening on ws://%s:%d/, allowing %s",
                    listen_host, listen_port, allowlist)

        stop = asyncio.get_running_loop().create_future()
        try:
            asyncio.get_running_loop().add_signal_handler(
                signal.SIGINT, lambda: stop.done() or stop.set_result(None))
        except NotImplementedError:
            pass
        await stop


def main():
    parser = argparse.ArgumentParser(
        description="Proxies WebSocket connections to TCP for the wasm client")
    parser.add_argument("--listen", default="127.0.0.1:8765",
                        metavar="HOST:PORT",
                        help="address to listen on (default: 127.0.0.1:8765)")
    parser.add_argument("--allow", action="append", default=[],
                        metavar="HOST[:PORT]",
                        help="target that may be connected to, repeatable "
                             "(default: 127.0.0.1 and localhost)")
    parser.add_argument("--verbose", action="store_true",
                        help="log every frame decision")
    args = parser.parse_args()

    logging.basicConfig(
        level=logging.DEBUG if args.verbose else logging.INFO,
        format="%(asctime)s %(levelname)s %(message)s")

    listen_host, listen_port = parse_host_port(args.listen, 8765)
    allowlist = Allowlist(args.allow or DEFAULT_ALLOW)

    try:
        asyncio.run(run(listen_host, listen_port, allowlist))
    except KeyboardInterrupt:
        pass


if __name__ == "__main__":
    main()
