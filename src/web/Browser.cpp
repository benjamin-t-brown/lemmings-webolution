#include "web/Browser.h"

#include <emscripten.h>

namespace webo::web {

// Scale the display with CSS. SDL maps it to the backing canvas.
EM_JS(void, applyCanvasScale, (int scale), {
  const canvas = Module['canvas'];
  if (canvas?.dataset) canvas.dataset.windowScale = scale;
});

void setCanvasScale(int scale) { applyCanvasScale(scale); }

EM_JS(void, getCanvasFillSize, (int* width, int* height), {
  if (typeof window === 'undefined') return;
  const w = Math.min(window.innerWidth, window.innerHeight * 4 / 3);
  HEAP32[width >> 2] = Math.round(w);
  HEAP32[height >> 2] = Math.round(w * 3 / 4);
});

void canvasFillSize(int& width, int& height) { getCanvasFillSize(&width, &height); }

EM_JS(int, portraitLevelMediaMatches, (), {
  const media = Module['portraitLevelMedia'] ||
      (Module['portraitLevelMedia'] = window.matchMedia('(orientation: portrait) and (pointer: coarse)'));
  return media.matches;
});

bool prefersPortraitLevel() { return portraitLevelMediaMatches() != 0; }

EM_JS(void, applyCanvasPortrait, (int portrait), {
  Module['canvas'].toggleAttribute('data-portrait-level', !!portrait);
});

void setCanvasPortrait(bool portrait) { applyCanvasPortrait(portrait); }

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

// Give replacements distinct mtimes for IDBFS. On sync failure restore the in-memory file
// so a later sidecar sync cannot publish the failed write.
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
