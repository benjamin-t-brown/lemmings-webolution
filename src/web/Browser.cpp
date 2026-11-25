#include "web/Browser.h"

#include "app/AppEntry.h"
#include "app/RuntimePick.h"
#include "sim/World.h"

#include <algorithm>
#include <emscripten.h>

namespace webo::web {

EM_JS(bool, browserCanvasAvailable, (), {
  return typeof document !== 'undefined' && !!Module['canvas'];
});

EM_JS(bool, webGlSelectedAtStart, (), {
  return Module['preferredRenderer'] === 'gl';
});

bool hasBrowserCanvas() { return browserCanvasAvailable(); }
bool requestedWebGl() { return webGlSelectedAtStart(); }

// Optional comparison observer supplied by the browser harness before Start.
// The normal page does not request readbacks or software reference frames.
EM_JS(bool, hasGlFrameObserver, (), {
  return typeof Module['glFrameCheckpoint'] === 'function' &&
         typeof Module['glFrameComparison'] === 'function';
});

EM_JS(bool, wantsGlFrame, (int screen, int frame), {
  return !!Module['glFrameCheckpoint'](screen, frame);
});

EM_JS(void, observeGlFrame, (int screen, int frame, int width, int height,
                             const std::uint8_t* cpu, const std::uint8_t* gpu), {
  const bytes = width * height * 4;
  Module['glFrameComparison'](screen, frame, width, height,
    HEAPU8.slice(cpu, cpu + bytes), HEAPU8.slice(gpu, gpu + bytes));
});

app::GlFrameComparison glFrameComparison() {
  if (!hasGlFrameObserver()) return {};
  return {
    [](shell::Screen screen, int frame) { return wantsGlFrame(static_cast<int>(screen), frame); },
    [](shell::Screen screen, int frame, const Image& cpu, const Image& gpu) {
      if (cpu.width == gpu.width && cpu.height == gpu.height && cpu.rgba.size() == gpu.rgba.size())
        observeGlFrame(static_cast<int>(screen), frame, gpu.width, gpu.height, cpu.rgba.data(), gpu.rgba.data());
    }
  };
}

EM_JS(bool, hasGlLevelObserver, (), {
  return typeof Module['glLevelFrame'] === 'function';
});

EM_JS(void, observeGlLevel, (const char* level, int frame, double yaw, bool intro, bool paused,
                            int skill, int stock, std::size_t edits, int saved, int active,
                            const int* targets, std::size_t count), {
  Module['glLevelFrame']({level: UTF8ToString(level), frame, yaw, intro: !!intro, paused: !!paused,
    skill, stock, edits, saved, active, targets: Array.from(HEAP32.subarray(targets >> 2, (targets >> 2) + count))});
});

app::LevelFrameObserver glLevelFrameObserver() {
  if (!hasGlLevelObserver()) return {};
  return [](const app::LevelFrameView& frame) {
    const auto skill = static_cast<sim::Skill>(frame.selectedSkillIndex);
    const auto& lemmings = frame.world.lemmings();
    std::vector<int> targets;
    for (const auto& actor : frame.actors) {
      if (!frame.world.canAssign(actor.index, skill)) continue;
      if (frame.levelName == "MM_CLIMB_002" &&
          !app::firstLevelClimberSupport(lemmings[actor.index], skill, frame.world.terrain()).value_or(true)) continue;
      const auto& p = actor.placement;
      const int x = static_cast<int>((p.left + p.right) * 0.5);
      const int y = static_cast<int>((p.top + p.bottom) * 0.5);
      if (x < 100 || x >= 640 || y < 0 || y >= 480) continue;
      targets.push_back(static_cast<int>(actor.index));
      targets.push_back(x);
      targets.push_back(y);
    }
    const int active = static_cast<int>(std::ranges::count_if(lemmings, &sim::Lemming::active));
    observeGlLevel(frame.levelName.data(), frame.frame, frame.displayedPose.yawUnits, frame.cameraIntroActive,
                   frame.paused, frame.selectedSkillIndex, frame.world.setup().skills[frame.selectedSkillIndex],
                   frame.drawnTerrainCellsChanged, frame.world.saved(), active, targets.data(), targets.size());
  };
}

// Clone addition. Scale the display with CSS. SDL maps it to the 640x480 backing canvas.
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
