#include "web/Browser.h"

#include <emscripten.h>

namespace webo::web {

EM_ASYNC_JS(void, waitForFrame, (), {
  await new Promise(resolve => requestAnimationFrame(resolve));
});

void nextFrame() { waitForFrame(); }

EM_ASYNC_JS(int, waitForLevelInput, (), {
  let wait = Module['levelFrameWait'];
  if (!wait) {
    wait = {};
    Module['levelFrameWait'] = wait;
    const canvas = Module['canvas'];
    const click = event => {
      if (event.button !== 0 || !wait.resume) return;
      const resume = wait.resume;
      wait.resume = null;
      // The promise continuation polls SDL after the DOM event has dispatched.
      resume(0);
    };
    canvas.addEventListener('mousedown', click);
    requestAnimationFrame(() => {
      canvas.removeEventListener('mousedown', click);
      Module['levelFrameWait'] = null;
      wait.resume?.(1);
    });
  }
  return await new Promise(resolve => { wait.resume = resolve; });
});

bool waitForLevelFrameOrClick() { return waitForLevelInput() != 0; }

// IDBFS compares mtimes during sync. Make even same-millisecond replacements
// distinguishable. If the transaction fails, restore the prior in-memory file
// so a later write to the other sidecar cannot accidentally publish this one.
EM_ASYNC_JS(int, commitSave, (const char* temporary, const char* target), {
  const from = UTF8ToString(temporary);
  const to = UTF8ToString(target);
  let previous;
  let renamed = false;
  try {
    if (FS.analyzePath(to).exists) {
      previous = { bytes: FS.readFile(to), time: FS.stat(to).mtime.getTime() };
      const timestamp = Math.max(Date.now(), previous.time + 1);
      FS.utime(from, timestamp, timestamp);
    }
    FS.rename(from, to);
    renamed = true;
    await new Promise((resolve, reject) =>
      FS.syncfs(false, error => error ? reject(error) : resolve()));
    return 1;
  } catch (error) {
    if (renamed) {
      if (previous) {
        FS.writeFile(to, previous.bytes);
        FS.utime(to, previous.time, previous.time);
      } else {
        FS.unlink(to);
      }
    }
    err("Save transaction failed: " + error);
    return 0;
  }
});

bool replaceSave(const char* temporary, const char* target) {
  return commitSave(temporary, target) != 0;
}

} // namespace webo::web
