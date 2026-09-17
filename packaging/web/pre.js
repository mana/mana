// Persistent storage for the browser build.
//
// The client keeps its configuration, server list, logs and downloaded updates
// in one directory. Under Emscripten that directory is /home/web_user/mana,
// backed by IndexedDB through IDBFS. It is loaded before main() runs and
// written back after the client changes something (FS.syncfs(false, ...) from
// C++) and once more when the page goes away.

var MANA_LOCAL_DATA_DIR = '/home/web_user/mana';

Module['preRun'] = Module['preRun'] || [];
Module['preRun'].push(function () {
  try {
    FS.mkdirTree(MANA_LOCAL_DATA_DIR);
    FS.mount(IDBFS, {}, MANA_LOCAL_DATA_DIR);
  } catch (e) {
    console.error('Could not mount IDBFS at ' + MANA_LOCAL_DATA_DIR + ': ' + e);
    return;
  }

  // Hold up main() until IndexedDB has been read into the in-memory filesystem
  Module['addRunDependency']('idbfs');
  FS.syncfs(true, function (err) {
    if (err)
      console.error('Could not load persistent storage: ' + err);
    Module['removeRunDependency']('idbfs');
  });
});

// Best-effort flush when the tab is closed or navigated away from. Browsers do
// not wait for the callback, but the write is usually started in time.
if (typeof window !== 'undefined') {
  window.addEventListener('beforeunload', function () {
    try {
      FS.syncfs(false, function () {});
    } catch (e) {
      // Nothing useful to do here, the page is going away
    }
  });
}
