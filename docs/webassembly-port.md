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
   `/data`. The local data directory is `/home/web_user/mana`, an IDBFS
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

and open the page with `?proxy=ws://127.0.0.1:8765/`. The page also accepts
`server`, `port`, `update-host`, `skip-update` and `default` query parameters,
which map to the matching command line options.

### Testing against a local tmwAthena

This is how the port was verified (see step 8). A local server does not have
an update host, but the browser cannot read the server's `client-data`
directory, so it is packed into a single zip that the client downloads as an
update. Keep the update files under the same origin as the page, since a
plain `python3 -m http.server` sends no CORS headers:

```
mkdir build-wasm/src/updates
(cd /path/to/serverdata/client-data && zip -r ../../build-wasm/src/updates/local-data.zip . -x '.git/*')
python3 -c 'import zlib,sys; print("%x" % (zlib.adler32(open(sys.argv[1],"rb").read()) & 0xffffffff))' build-wasm/src/updates/local-data.zip
```

Write `build-wasm/src/updates/resources.xml` with that hash:

```
<?xml version="1.0"?><updates>
<update type="data" file="local-data.zip" hash="<adler32>"/>
</updates>
```

plus a `news.txt`, then open

```
http://127.0.0.1:8000/mana.html?proxy=ws://127.0.0.1:8765/&server=127.0.0.1&port=6901&update-host=http://127.0.0.1:8000/updates/
```

Headless Chromium (`--headless=new --remote-debugging-port=...`) driven over
the DevTools protocol works for automated checks; `--use-angle=swiftshader
--enable-unsafe-swiftshader` gives it a WebGL context. Synthetic key events
must be held for longer than a frame, because the game's shortcut handling
reads SDL's live key state.

## Steps

Status markers: [x] done, [~] partially done, [ ] not started.

### 1. Build system
- [x] An `if(EMSCRIPTEN)` branch in the root `CMakeLists.txt`, placed before
      the `option()` calls, forces `WITH_OPENGL`, `ENABLE_MANASERV`,
      `ENABLE_NLS` and `USE_SYSTEM_GUICHAN` to OFF as cache entries, so a
      stale cache or a stray `-D` cannot produce an unlinkable configuration.
      The desktop defaults are untouched.
- [x] PhysicsFS (`icculus/physfs`, `release-3.2.0`, static, ZIP archiver only,
      tests/docs/install off) and libxml2 (`GNOME/libxml2`, `v2.13.8`, static,
      only OUTPUT/READER/WRITER/SAX1/TREE/XPATH on) are fetched with CPM,
      vendored as `cmake/CPM_0.40.5.cmake`. PhysicsFS needs
      `CMAKE_POLICY_VERSION_MINIMUM 3.5` around it, like the ENet submodule.
      The resulting targets are `physfs-static` and `LibXml2::LibXml2`.
      Sources are cached in `.cpm-cache/` (gitignored, `CPM_SOURCE_CACHE`
      from the environment wins).
- [x] `src/CMakeLists.txt` wraps the pkg-config and `find_package` block in
      `if(NOT EMSCRIPTEN)`. The port flags
      `-sUSE_SDL=2 -sUSE_SDL_IMAGE=2 -sSDL2_IMAGE_FORMATS=png -sUSE_SDL_MIXER=2
      -sSDL2_MIXER_FORMATS=ogg -sUSE_SDL_TTF=2 -sUSE_ZLIB=1` go to both
      `target_compile_options` and `target_link_options`. Link options add
      `-sFETCH -lwebsocket.js -lidbfs.js -sALLOW_MEMORY_GROWTH=1
      -sINITIAL_MEMORY=128MB -sSTACK_SIZE=1MB -sEXIT_RUNTIME=0
      -sEXPORTED_RUNTIME_METHODS=FS,ccall,cwrap,stringToNewUTF8,UTF8ToString
      -sEXPORTED_FUNCTIONS=_main,_malloc,_free`, `--preload-file data@/data`,
      `--shell-file packaging/web/shell.html`, `--pre-js packaging/web/pre.js`,
      `-O2` for Release and `-g -sASSERTIONS=1` for Debug. The executable
      suffix is `.html`; the shell and pre.js are `LINK_DEPENDS`, so editing
      them relinks. SDL2_net is not linked, curl and Intl are not either.
      `net/tmwa/network.cpp` is still compiled and switches internally on
      `__EMSCRIPTEN__`.
- [x] Desktop-only icon formats are kept out of the preload package with
      `--exclude-file *.ico`, `*.icns` and `*.hqx`. These have to be written as
      `"SHELL:--exclude-file *.ico"`, otherwise CMake collapses the repeated
      flag and em++ sees a stray pattern.
- [~] `src/utils/gettext.h` now includes `<libintl.h>` only where the header
      exists (`__has_include`) and defines an `ngettext` fallback when
      `ENABLE_NLS` is off. Guarding the include on `ENABLE_NLS` alone breaks
      the desktop `-DENABLE_NLS=OFF` build, because another header pulls in
      `<libintl.h>` afterwards and the fallback macros then mangle its
      declarations. Emscripten's musl sysroot does have `<libintl.h>`, so the
      include is harmless there.
- [x] `PKG_DATADIR` is `"/"` and `LOCALEDIR` is `"/locale/"` under Emscripten,
      so the existing `PKG_DATADIR "data"` search path resolves to the
      preloaded `/data`.
- [x] The desktop-only `install()` rules (freedesktop files, icons, the
      executable) are skipped under Emscripten.

### 2. Main loop and lifecycle
- [x] The body of the old `while (mState != State::Exit)` loop is now
      `Client::runFrame()`, used by both the desktop loop and the browser.
      Under `__EMSCRIPTEN__`, `Client::exec()` registers it with
      `emscripten_set_main_loop_arg(emscriptenFrame, this, 0, 0)` (one
      iteration per animation frame, no simulated infinite loop) and returns
      immediately. `main()` then returns as well, so the client is allocated
      on the heap there to outlive it (`-sEXIT_RUNTIME=0` keeps the runtime
      alive).
- [x] `FpsManager::limitFps` is a no-op under Emscripten, including the
      `limitFps(10)` path for a hidden window. `SDL_Delay` would block the
      browser's main thread.
- [x] `emscriptenFrame` checks the state after each iteration, exactly where
      the desktop loop checks its condition. On `State::Exit` it cancels the
      main loop, calls `Net::unload()`, deletes the client (which writes the
      configuration in `~Client`), syncs IDBFS and puts a "you can close this
      tab" message in the page. Nothing touches the client afterwards.
- [x] The `SDL_AddTimer` FPS counter is gone. `Client::update()` counts the
      frames it draws and publishes them in the `fps` global once a second,
      on all platforms. `fps` is no longer `volatile`, since it is now only
      written and read from the main thread.
- [x] `Game::logic` resets the particle timer when it is more than a second
      behind, so a throttled or hidden tab does not run thousands of particle
      updates on resume. It is the only absolute-timer catch-up loop there;
      the map update already uses the clamped `Time::deltaTimeMs()`.
- [~] `Log::critical` shows the message in the page (`Module.setStatus`, or
      `alert` as a fallback) and cancels the main loop, but still ends in
      `exit(1)`: several of its callers cannot handle it returning (see
      "Loose ends").

### 3. Filesystem persistence
- [x] `packaging/web/pre.js` runs from `Module.preRun`: it creates
      `/home/web_user/mana`, mounts IDBFS there, holds `main()` back with a
      run dependency until `FS.syncfs(true)` has populated it from IndexedDB,
      and syncs once more on `beforeunload` as a best effort.
- [x] `FS::sync()` calls `FS.syncfs(false)` and is a no-op elsewhere. It runs
      after the configuration is written in `~Client`, after a screenshot is
      saved, after the server list download and after every completed update
      download. The sync itself is asynchronous, so it can still be in flight
      when the call returns.
- [x] Under Emscripten the local data directory and the config directory are
      `/home/web_user/mana` (the IDBFS mount) unless `--localdata-dir` or
      `--config-dir` says otherwise, the legacy `~/.config` migration is
      compiled out, and screenshots go to `/home/web_user/mana/screenshots`.
      After `IMG_SavePNG` the file is read back with `FS.readFile` and handed
      to the browser as a download through a Blob and an anchor click.

### 4. Networking (tmwAthena)
- [x] `TmwAthena::Network` Emscripten implementation with
      `emscripten_websocket_*`: `connect()` opens the socket and sets
      `CONNECTING`, onopen sets `CONNECTED` and flushes what was already
      queued, onmessage appends the binary payload to the in-buffer,
      onerror and onclose set `NET_ERROR`, and `flush()` sends the
      out-buffer with `emscripten_websocket_send_binary`. No thread and no
      `SDL_net`. The desktop implementation is unchanged, next to it under
      `#ifndef __EMSCRIPTEN__`.
- [x] Proxy URL construction. A TCP connection to `host:port` becomes a
      WebSocket to `<proxy base><host>/<port>` with the `binary`
      subprotocol. The base is read once per connect from
      `Module.manaProxyUrl` (through `EM_ASM_PTR` and `stringToNewUTF8`) and
      falls back to `ws(s)://<page host>/tmwa/`. A missing trailing slash is
      added. The resolved URL is logged on every connect.
      `ServerStatusBackend` needs nothing of its own: it probes through a
      `Network` of its own and so gets the same URL handling.
- [x] Server hopping (login to char to map) keeps working. No name
      resolution happens in the client, so the raw IP the login and char
      servers hand out is passed to the proxy as-is, and the proxy decides
      whether it may be reached.
- [x] `tools/ws-tcp-proxy.py` development proxy with an allowlist.
      `--listen HOST:PORT`, repeatable `--allow HOST[:PORT]` (default
      `127.0.0.1` and `localhost`) and `--verbose`. The request path is
      `/<host>/<port>`, and any prefix before those two segments is ignored,
      so `/tmwa/<host>/<port>` works when a front-end web server mounts the
      proxy under a path.
- [x] Close handling matches the desktop semantics the rest of the client
      expects. A clean close of an established connection (code 1000) means
      `IDLE`, which is how `ServerStatusBackend` recognises a server that
      answered and hung up. Anything else, including a connection that never
      opened, becomes `NET_ERROR` with the close code and reason in the
      message. The proxy plays along: it closes with 1000 once the TCP peer
      is gone, 1008 for a bad path or a target that is not allowed, and 1011
      when the target cannot be reached.
- [x] Callback lifetime. Events are delivered asynchronously, so the
      callbacks look the `Network` up in a table keyed by socket handle
      rather than trusting a `userData` pointer. `disconnect()` (and so the
      destructor) removes the entry before closing and deleting the socket,
      which makes a late event harmless.

### 5. Downloads
- [x] `Net::Download` has an `emscripten_fetch` implementation under
      `#ifdef __EMSCRIPTEN__`; the libcurl code is unchanged in the `#else`.
      The public API is the same, so `UpdaterWindow` and `ServerDialog` keep
      polling `getState()` once per frame. No thread and no `SDL_WaitThread`;
      the callbacks run on the main thread between frames. The `ThreadSafe`
      wrapper around the state was kept, since SDL mutexes are cheap and it
      keeps one code path for both builds.
- [x] `start()` fills an `emscripten_fetch_attr_t` with method GET,
      `EMSCRIPTEN_FETCH_LOAD_TO_MEMORY` (no synchronous flag), `userData =
      this` and onsuccess/onerror/onprogress callbacks. Memory mode copies the
      response into the malloc'd `mBuffer` with the same ownership as before.
      File mode writes it to `<file>.part`, checks adler32 with `fadler32`,
      then `remove()` plus `rename()` into place.
- [x] Custom headers (`noCache`) are split at the first colon into the
      null-terminated key/value array `emscripten_fetch` wants. The storage
      lives in the `Download` object, and the pointer array is only built in
      `start()`, once no more headers can be added.
- [x] `cancel()` and the destructor detach `userData` before
      `emscripten_fetch_close()`, so a callback fired while closing cannot
      touch a dead object. The handle is nulled and closed in every path.
- [x] Retries match the libcurl version: a failed adler32 is retried up to
      three attempts in total, an HTTP error is not retried. The error string
      is `HTTP <status>: <statusText>` instead of curl's message.

### 6. Browser viewport
- [x] `packaging/web/shell.html`: a full-viewport canvas, a loading overlay
      driven by `Module.setStatus` and `monitorRunDependencies`, and a
      `Module` object that resolves `manaProxyUrl` from the `proxy` query
      parameter (default: page origin plus `/tmwa/`) and builds
      `Module.arguments` from the `server`, `port`, `update-host`,
      `skip-update` and `default` parameters.
- [x] `Video::initialize` registers an `emscripten_set_resize_callback` on
      the window, which calls `SDL_SetWindowSize(windowInnerWidth,
      windowInnerHeight)`. SDL's Emscripten backend then sizes the canvas
      backing store to that times `devicePixelRatio` (the window is created
      with `SDL_WINDOW_ALLOW_HIGHDPI`), and leaves the CSS size alone because
      the page already styles the canvas. `SDL_GetRendererOutputSize` reports
      the backing size, which `SDLGraphics::updateSize` turns into the
      display scale. Verified against `SDL_emscriptenvideo.c` in the emsdk
      port cache (SDL 2.32.8).
- [x] `Video::initialize` under Emscripten ignores the stored
      `screenwidth`/`screenheight` and creates the window at the current
      canvas CSS size (falling back to `window.innerWidth/Height`), always
      windowed. `limitToUsableBounds` is a no-op and no minimum window size
      is set, so the canvas can follow a small browser window.
      `Client::handleWindowSizeChanged` does not persist the size.
- [x] Setup dialog: done as part of step 7.

### 7. Browser polish
- [x] Audio: nothing to do in the client. SDL 2.32.8's Emscripten audio
      backend deals with the suspended WebAudio context itself. When the
      context is suspended at `Mix_OpenAudio` time it feeds silence to the
      mixer and calls `resume()` as soon as `navigator.userActivation`
      reports an interaction; browsers without that API get Emscripten's
      `autoResumeAudioContext` keydown/mousedown/touchstart listeners.
- [x] Custom cursor: `SDL_CreateColorCursor` is in fact supported by the
      browser backend, which turns the surface into a CSS `url(data:...)`
      cursor, so custom cursors stay enabled. `Gui::loadCustomCursors` skips
      them above 128x128 pixels (user scale 4), the size at which browsers
      ignore a cursor image, so the contextual system cursors are used
      instead of silently falling back to the default arrow.
- [x] `SDL_OpenURL` for external links: works only inside a user gesture,
      which is the case for link clicks. `file://` screenshot links disabled.
- [x] Text input: `SDL_TEXTINPUT` events are queued by `SDLInput::pushInput`
      and need nothing Emscripten specific; `SDL_StartTextInput()` is called
      once after video init. Do not set `SDL_HINT_EMSCRIPTEN_KEYBOARD_ELEMENT`
      to `#window`: current Emscripten resolves that string with
      `querySelector`, finds nothing, and SDL then registers no keyboard
      handlers at all. Unset, SDL already listens on the window.
- [x] Setup dialog (the last item of step 6): the resolution dropdown, the
      VSync checkbox and "reduce input lag" (which calls `glFinish`) are
      disabled, the exclusive fullscreen entry is dropped from the window
      mode list and `Setup_Video::apply` no longer overrides the window size.
      The OpenGL checkbox is already disabled by the existing
      `#ifndef USE_OPENGL`, since `WITH_OPENGL` is forced off.

### 8. Verification

All checked with Emscripten 5.0.1 and headless Chromium on 2026-09-17.

- [x] Wasm build links without warnings. The native build (with
      `-DENABLE_NLS=OFF`) still builds without warnings.
- [x] The page loads, the server list is fetched from manasource.org, the
      server dialog renders at 1280x720 and re-lays out when the viewport is
      changed to 800x500 without any window event from the page.
- [x] Quit writes `client.xml` to the IDBFS directory and a reload finds
      `client.xml`, `serverlist.xml`, `mana.log` and `logs/` again.
- [x] Against a local tmwAthena through `tools/ws-tcp-proxy.py`: the login
      dialog appears, registering an account works, login hops to the char
      server and then to the map server, a character is created, the map
      renders with NPCs, minimap and HUD at 60 FPS, a chat line is echoed
      back by the server, and the login music plays from the update zip.
- [x] Updates through `emscripten_fetch`: `news.txt`, `resources.xml` and a
      129 MB data zip are downloaded, the adler32 check passes, the zip is
      mounted, and a reload verifies the cached copy instead of downloading
      it again.

## Loose ends

Filled in as work progresses.

### Networking

- The proxy in `tools/ws-tcp-proxy.py` is for development only. It has no
  rate limiting, no origin check and no connection limit. A public
  deployment wants nginx (or another front end) terminating TLS and
  proxying `/tmwa/` to it, or websockify in front of a fixed target.
- A page served over HTTPS can only open `wss://`, so a public deployment
  needs a certificate for the proxy host. Serving the proxy under the page
  origin, so that the default `Module.manaProxyUrl` applies, avoids that
  problem entirely.
- The in-buffer grows on demand in the browser build, since everything that
  arrived between two frames lands in it at once, unlike the desktop receive
  loop that reads at most 64 KiB at a time. It never shrinks again.
- Server hopping sends the client to whatever address the login and char
  servers report, which for tmwAthena is usually a raw IP. Those IPs have to
  be on the proxy allowlist, so a deployment cannot simply allow the login
  server's hostname.
- The proxy negotiates `permessage-deflate` because that is the websockets
  default and browsers offer it. Compressing a small binary game protocol
  buys little and costs latency; disabling it is worth measuring.
- Nothing times out. If the proxy accepts the connection but never answers,
  `Network` stays in `CONNECTING` and `ServerStatusBackend` stays in
  `Checking` forever, exactly as on the desktop.
- `Log::critical` on an unknown packet still calls `exit(1)`, which is
  handled in step 2 but matters most for the network path.

- `Log::critical` still calls `exit(1)` under Emscripten. Falling through is
  not safe: `Video::initialize` returns a null `Graphics` when the window
  cannot be created, and `initHomeDir` would continue with an empty data
  directory. Emscripten turns the `exit` into an abort with the message in
  the console, and the page keeps the message that was set just before.
  Making the failures recoverable would be a separate cleanup.
- `SDL_SetWindowSize` is ignored by SDL while the window has
  `SDL_WINDOW_FULLSCREEN_DESKTOP` set, so a viewport resize during Fullscreen
  API fullscreen only takes effect after leaving fullscreen. SDL's own
  fullscreen handling sizes the canvas in that case; needs testing.
- The screenshot download uses an anchor click, which some browsers only
  allow from a user gesture. Taking a screenshot is triggered by a key press,
  so this should be fine in practice, but it is untested.
- `FS::sync()` is not called periodically. Chat logs written between two
  syncing events are only persisted by the next sync, or by the best-effort
  `beforeunload` sync in `pre.js`, which the browser may cut short.
- Downloaded update zips live in MEMFS (IDBFS keeps its working copy in
  memory) as well as in IndexedDB. The 65 MB TMW data zip is therefore held
  in browser memory for the whole session, plus its IndexedDB copy on disk.
  A leaner design would fetch the zip with `EMSCRIPTEN_FETCH_PERSIST_FILE`
  and stream it, or split the update into per-map archives.
- Clipboard: SDL's Emscripten backend has no clipboard driver, so
  `SDL_GetClipboardText` returns SDL's own internal string, which starts out
  empty. Copy and paste therefore only works within the client, and pasting
  something copied in the browser or on the desktop does nothing. The call
  never returns null, but `insertFromClipboard` now handles that too. Reading
  the real clipboard needs the async `navigator.clipboard` API, which only
  resolves inside a user gesture.
- Audio stays silent until the first click or key press, because the WebAudio
  context is suspended until then. Music started earlier is not restarted, so
  the login theme comes in mid-track.
- Custom cursors above 128x128 pixels (user scale 4 and up) fall back to the
  system cursors, since browsers ignore CSS cursor images that large.
- Screenshot links in the chat log do nothing in the browser build, because
  there is no file system the browser could open them from.

### Build and downloads

- The preload package is about 3.9 MB (`mana.data`), on top of a 4 MB
  `mana.wasm` and 220 KB of JavaScript. Music is the bulk of it. Splitting the
  music out, or fetching it lazily, is the obvious next saving.
- The data directory is baked into `mana.data` at link time, so changing a
  file under `data/` needs a relink, not just a rebuild.
- Update hosts have to send CORS headers (`Access-Control-Allow-Origin`) for
  the page origin, otherwise `emscripten_fetch` fails for every update file
  and for `resources2.txt`. The same applies to the server list and to the
  news file. There is no way around this from the client side; serving the
  updates from the page origin avoids it.
- The `pragma: no-cache` and `Cache-Control: no-cache` request headers added
  by `Download::noCache` make the request non-simple, so those hosts also have
  to answer the CORS preflight.
- `emscripten_fetch` has no user agent option, so the browser's own user agent
  is sent. `PACKAGE_EXTENDED_VERSION` is unused in the browser build, and a
  server cannot tell Mana clients apart from ordinary page loads.
- `PACKAGE_OS` in `main.h` resolves to "Other" under Emscripten, since none of
  the platform macros it tests is defined.
- An HTTP error is not retried, matching the libcurl version, which only
  retried a failed checksum. A transient 5xx therefore fails the update
  straight away.
- Translations are disabled, but `<libintl.h>` does exist in Emscripten's musl
  sysroot. Only the catalog loading is missing, so a libintl-free catalog
  reader is all that step 7 of the architecture decisions needs. The
  `android-nls` branch has `src/utils/messagecatalog.{cpp,h}` for exactly
  this, behind a `USE_LIBINTL` option; once that lands, the browser build can
  preload `po/` catalogs and turn `ENABLE_NLS` back on.
- The update host of The Mana World (`updates.themanaworld.org`) sends no
  `Access-Control-Allow-Origin` header, so the browser build can only play
  there once the page is served from that origin, the host adds CORS headers,
  or a same-origin reverse proxy for `/updates/` is deployed next to the
  WebSocket proxy.
- Manaserv support is compiled out, since ENet is UDP. Supporting it would
  need a WebRTC data channel or a WebSocket bridge that speaks ENet.
- No CI job builds the wasm target yet. The first build needs network access
  for the Emscripten ports and the CPM sources; `embuilder build sdl2
  sdl2_image sdl2_mixer sdl2_ttf zlib` seeds the port cache.
- `packaging/web/pre.js` mounts IDBFS at `/home/web_user/mana`. That path is
  shared with the client code by convention only; nothing checks that the two
  agree.
- The shell HTML is minified by em++ at `-O2`, so `build-wasm/src/mana.html`
  does not look like `packaging/web/shell.html`. Edit the shell, not the
  output.
- `-sINITIAL_MEMORY=128MB` is a guess sized for the current data set. With
  `-sALLOW_MEMORY_GROWTH=1` it only decides how much is reserved up front.
