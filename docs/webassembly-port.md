# WebAssembly (Emscripten) port

Goal: play The Mana World straight from a browser tab. The client is compiled
to WebAssembly with Emscripten, rendered into a canvas that fills the browser
viewport and follows its size, talks to tmwAthena through a WebSocket-to-TCP
proxy, and fetches updates over HTTP(S) from the page's origin.

This document is both the plan and the status log. Each step lists what was
done and what is still open. See "Loose ends" at the bottom for everything
that is known to be incomplete.

## Findings from the audit (September 2026)

The client is closer to browser-ready than expected:

- The main loop body is already a single non-blocking iteration:
  `Client::exec()` pumps `SDL_PollEvent` and calls `Client::update()`. No
  `SDL_WaitEvent`, no modal dialog loops. Connection and download progress
  are polled per frame through the `State` machine.
- The only `SDL_Delay` is the frame limiter in `FpsManager::limitFps`.
- The window size is re-read from SDL every frame
  (`Video::updateWindowSize()` then `Client::checkGraphicsSize()`), so once
  SDL's notion of the window follows the canvas, the whole GUI re-lays out
  with no extra code.
- The default renderer is `SDLGraphics` (SDL_Renderer), which Emscripten's
  SDL2 maps to WebGL. Every renderer call used is supported. The
  `OpenGLGraphics` backend is fixed-function GL 1.x (GL_QUADS, glBegin,
  rectangle textures) and is not portable; it is simply compiled out.
- All data files are PNG, TTF, WAV and Ogg Vorbis, all loaded through PhysFS
  RWops. No `IMG_Init`/`Mix_Init` calls, so formats are chosen at link time.
- Only two places create threads: `Net::Download` (libcurl in an SDL thread)
  and `TmwAthena::Network` (blocking SDL_net receive loop in an SDL thread).
  Both are consumed through per-frame polling, so both can be re-implemented
  as single-threaded, callback-driven objects behind the same interface.
- Manaserv (ENet/UDP) is fully behind `ENABLE_MANASERV` and stays off.
- User state lives in one PhysFS pref dir (`client.xml`, `serverlist.xml`,
  `offline.xml`, `mana.log`, `logs/`, `updates/`). That directory is backed
  by IDBFS in the browser.

Emscripten 5.0.1 ports exist for SDL2, SDL2_image (png), SDL2_mixer (ogg),
SDL2_ttf, SDL2_net and zlib. PhysicsFS and libxml2 have no port and are built
from source with CPM. libcurl and ENet are not used in the wasm build.
gettext/libintl has no port; translations are disabled for now.

## Architecture decisions

1. **Renderer**: SDL_Renderer over WebGL. `WITH_OPENGL` is forced off.
2. **Main loop**: `emscripten_set_main_loop` driven by requestAnimationFrame.
   One callback iteration equals one pass of the old `while` loop body. No
   ASYNCIFY; nothing needs to block.
3. **Game networking**: under `__EMSCRIPTEN__`, `TmwAthena::Network` is
   implemented on top of the browser WebSocket API (`emscripten/websocket.h`)
   instead of SDL_net. A raw TCP connection to `host:port` becomes a
   WebSocket to `<proxy base>/<host>/<port>` carrying the binary subprotocol.
   This avoids Emscripten's fake-DNS socket emulation and gives explicit
   open/close/error events. The proxy base URL comes from the page
   (`Module.manaProxyUrl`), defaulting to `<page origin>/tmwa/`.
   A development proxy is provided in `tools/ws-tcp-proxy.py`.
4. **HTTP downloads**: `Net::Download` is re-implemented with
   `emscripten_fetch`, keeping its public API and the per-frame `getState()`
   polling used by `UpdaterWindow` and `ServerDialog`. Update files are
   fetched to memory, checked with adler32 and written to the IDBFS-backed
   updates directory.
5. **Filesystem**: the packaged `data/` directory is preloaded into MEMFS at
   `/data`. The pref dir (`/home/web_user/.local/share/mana`) is an IDBFS
   mount, synced from IndexedDB before `main()` runs and synced back after
   config writes, update downloads and on exit.
6. **Viewport**: the canvas is styled to fill the viewport. A resize callback
   sizes the canvas backing store to CSS size times devicePixelRatio and calls
   `SDL_SetWindowSize` with the CSS size. The existing HiDPI logic in
   `SDLGraphics::updateSize` then computes the display scale. Persisted
   `screenwidth`/`screenheight` are ignored in the browser.
7. **Translations**: off (`ENABLE_NLS=OFF`) until a libintl-free catalog
   reader lands (the `android-nls` branch has one).

## Build

```
source ~/emsdk/emsdk_env.sh
emcmake cmake -S . -B build-wasm -DCMAKE_BUILD_TYPE=Release
cmake --build build-wasm -j
```

Output lands in `build-wasm/src/`: `mana.html`, `mana.js`, `mana.wasm`,
`mana.data`. Serve that directory over HTTP (`python3 -m http.server` works
for a smoke test) and open `mana.html`.

To reach a tmwAthena server from the page, run the proxy:

```
python3 tools/ws-tcp-proxy.py --listen 127.0.0.1:8765 --allow 127.0.0.1
```

and open the page with `?proxy=ws://127.0.0.1:8765/`.

## Steps

Status markers: [x] done, [~] partially done, [ ] not started.

### 1. Build system
- [ ] `if(EMSCRIPTEN)` branch in the root `CMakeLists.txt` forcing
      `WITH_OPENGL=OFF`, `ENABLE_MANASERV=OFF`, `ENABLE_NLS=OFF`,
      `USE_SYSTEM_GUICHAN=OFF`, and fetching PhysicsFS 3.2.0 and libxml2 with
      CPM (already vendored in `cmake/`).
- [ ] `src/CMakeLists.txt`: skip pkg-config and `find_package` for the SDL
      stack, curl, intl and physfs; add the `-sUSE_SDL*` port flags to both
      compile and link options; `-sFETCH`, `-lwebsocket.js`, `-lidbfs.js`,
      `-sALLOW_MEMORY_GROWTH`, `--preload-file data@/data`, custom shell HTML.
- [ ] Guard `#include <libintl.h>` in `src/utils/gettext.h` behind
      `ENABLE_NLS`.
- [ ] `PKG_DATADIR` set to `/` so the existing `PKG_DATADIR "data"` search
      path resolves to the preloaded `/data`.

### 2. Main loop and lifecycle
- [ ] Split `Client::exec()` into an iteration function; under
      `__EMSCRIPTEN__` register it with `emscripten_set_main_loop(fn, 0, 1)`.
- [ ] Make `FpsManager::limitFps` a no-op under Emscripten (rAF paces).
- [ ] On `State::Exit`, cancel the main loop and run the shutdown path
      explicitly (config save in `~Client`, `Net::unload`), then sync IDBFS.
- [ ] Replace the `SDL_AddTimer` FPS counter with main-thread accounting.
- [ ] Clamp the particle catch-up loop in `Game::logic` so a throttled tab
      does not freeze on resume.
- [ ] `Log::critical`: report through the page instead of `exit(1)`.

### 3. Filesystem persistence
- [ ] Pre-`main` JS: mount IDBFS at the pref dir and `FS.syncfs(true)` before
      `main()` runs (via `Module.preRun` and `addRunDependency`).
- [ ] `FS::sync()` helper that calls `FS.syncfs(false)`; invoked after config
      save, after each completed update download, after serverlist download.
- [ ] Skip legacy config migration and the desktop screenshot directory
      logic under Emscripten; screenshots trigger a browser download instead.

### 4. Networking (tmwAthena)
- [ ] `TmwAthena::Network` Emscripten implementation with
      `emscripten_websocket_*`: connect, onopen, onmessage appends to the
      in-buffer, onclose/onerror set `NET_ERROR`, `flush()` sends the
      out-buffer with `emscripten_websocket_send_binary`. No thread, no
      mutex.
- [ ] Proxy URL construction shared by `Network` and `ServerStatusBackend`.
- [ ] Server hopping (login to char to map) keeps working: the hostname or
      IP the server sends is passed to the proxy, which decides whether it
      is allowed.
- [ ] `tools/ws-tcp-proxy.py` development proxy with an allowlist.

### 5. Downloads
- [ ] `Net::Download` Emscripten implementation with `emscripten_fetch`:
      memory mode fills `mBuffer`; file mode writes the received bytes to
      `<file>.part`, verifies adler32, renames to the final name.
- [ ] Custom headers (`noCache`) mapped to fetch request headers.

### 6. Browser viewport
- [ ] Shell HTML with a full-viewport canvas, loading progress, and
      `Module.manaProxyUrl` resolution from a `?proxy=` query parameter.
- [ ] Resize callback: canvas backing store = CSS size * devicePixelRatio,
      then `SDL_SetWindowSize(cssW, cssH)`.
- [ ] `Video::initialize` under Emscripten: window created at the current
      canvas size, `limitToUsableBounds` skipped, minimum size not enforced.
- [ ] Setup dialog: hide the OpenGL checkbox, resolution list and exclusive
      fullscreen mode under Emscripten.

### 7. Browser polish
- [ ] Audio: resume the WebAudio context on the first user gesture.
- [ ] Custom cursor: `SDL_CreateColorCursor` is unsupported; disable custom
      cursors under Emscripten.
- [ ] `SDL_OpenURL` for external links: works only inside a user gesture,
      which is the case for link clicks. `file://` screenshot links disabled.
- [ ] Text input: call `SDL_StartTextInput()` at startup so soft keyboards
      and IME events reach the client.

### 8. Verification
- [ ] Wasm build links.
- [ ] Page loads, login screen renders and follows the viewport size.
- [ ] Config survives a reload (IDBFS).
- [ ] Login against a local tmwAthena through the dev proxy.
- [ ] Update download through fetch.

## Loose ends

Filled in as work progresses.
