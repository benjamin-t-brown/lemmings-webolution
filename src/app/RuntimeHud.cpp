#include "app/RuntimeHud.h"

#include "shell/HudText.h"

#include <algorithm>
#include <cmath>

namespace webo::app {

shell::HudValues runtimeHudValues(const sim::World& world) {
  shell::HudValues values;
  values.out = world.spawned() - world.saved() - world.dead();
  values.saved = world.saved();
  values.saveTarget = world.setup().saveTarget;
  values.secondsRemaining = static_cast<int>(
      world.timeRemainingTicks() / world.rules().ticksPerSecond);
  return values;
}

void drawRuntimeHud(webo::Image& frame, const webo::Image& font,
                    const sim::World& world, bool paused) {
  const auto values = runtimeHudValues(world);
  const auto cells = shell::fontCells();
  const auto lines = shell::layoutHud(values, {}, cells);
  shell::drawHud(frame, font, lines, cells);
  if (paused) shell::drawCaption(frame, font, cells);
}

void RuntimeSimState::togglePause() {
  paused_ = !paused_;
  clock_ = SimClock{};
  carryMs_ = 0.0;
}

void RuntimeSimState::resume() {
  if (paused_) {
    paused_ = false;
    clock_ = SimClock{};
    carryMs_ = 0.0;
  }
}

bool RuntimeSimState::advanceElapsed(sim::World& world, double elapsedSeconds, bool trackCompletion,
                                     const std::function<void()>& afterTick, int stallCapMs) {
  return advanceElapsed(world, elapsedSeconds, trackCompletion, afterTick, stallCapMs, true);
}

bool RuntimeSimState::advanceElapsed(sim::World& world, double elapsedSeconds, bool trackCompletion,
                                     const std::function<void()>& afterTick, int stallCapMs, bool releaseEnabled) {
  if (paused_) return false;
  if (trackCompletion && end_.phase() == LevelEndPhase::Results) return false;
  // Integer milliseconds with the sub-ms fraction carried (the epsilon absorbs the binary
  // representation of values such as 0.016 s).
  const double exactMs = std::max(0.0, elapsedSeconds) * 1000.0 + carryMs_;
  long long wholeMs = static_cast<long long>(std::floor(exactMs + 1e-6));
  carryMs_ = std::max(0.0, exactMs - static_cast<double>(wholeMs));
  if (stallCapMs > 0 && wholeMs > stallCapMs) {
    // Stall: only the capped part is simulated; the rest is dropped, not carried.
    discardedStallMs_ += wholeMs - stallCapMs;
    wholeMs = stallCapMs;
    carryMs_ = 0.0;
  }
  const int dtMs = static_cast<int>(std::min<long long>(wholeMs, 1LL << 30));
  lastPassMs_ = dtMs;

  // READ 0x45DB0E-0x45DD5C: every fast-forward pass reuses the same integer dt.
  const int passes = fastForward_ ? kFastForwardPasses : 1;
  bool updated = false;
  for (int pass = 0; pass < passes; ++pass) {
    if (pass > 0 && trackCompletion && end_.phase() == LevelEndPhase::Results) break;
    world.advanceElapsedMsWithReleaseGate(dtMs, trackCompletion, releaseEnabled, [&](double segmentSeconds) {
      updated = true;
      if (afterTick) afterTick();
      if (trackCompletion) {
        int retainedNonRat = 0;
        for (const auto& lemming : world.lemmings())
          if (lemming.alive() && lemming.sheetType != 3) ++retainedNonRat;
        // The completion slice receives this update's own dt, not a fixed 1/ticksPerSecond.
        end_.afterSimulationSlice(segmentSeconds,
            {world.timedOut(), retainedNonRat, world.setup().totalLemmings - world.spawned(),
             world.saved(), world.setup().saveTarget});
        // The existing completion stop also ends the remaining updates and passes.
        if (end_.phase() == LevelEndPhase::Results) return false;
      }
      return true;
    });
  }
  return updated;
}

bool RuntimeSimState::advance(sim::World& world, double elapsedSeconds, bool trackCompletion,
                              const std::function<void()>& afterTick) {
  if (paused_) return false;
  // READ 0x45DB0E-0x45DD5C: the latch selects the 0x4B15F8 pass count (7)
  // instead of 1, and every pass reuses the full frame delta. Only this pass
  // loop repeats; input and camera clocks run once per frame in the caller.
  // Each pass goes through the unchanged SimClock path, so the base tick rate
  // and the per-advance tick cap apply to every pass.
  const int passes = fastForward_ ? kFastForwardPasses : 1;
  bool ticked = false;
  for (int pass = 0; pass < passes; ++pass) {
    // The existing completion stop also ends the remaining passes.
    if (pass > 0 && trackCompletion && end_.phase() == LevelEndPhase::Results) break;
    clock_.advance(elapsedSeconds);
    while ((!trackCompletion || end_.phase() != LevelEndPhase::Results) && clock_.takeStep()) {
      // Native hatches continue while timeout-pending (425A95). Default World
      // fixtures keep their old timeout-release policy; connected play opts in.
      world.tick(trackCompletion);
      ticked = true;
      if (afterTick) afterTick();
      if (trackCompletion) {
        int retainedNonRat = 0;
        for (const auto& lemming : world.lemmings())
          if (lemming.alive() && lemming.sheetType != 3) ++retainedNonRat;
        // DERIVED ordinary-population adapter. Native5098 is selected-type
        // unreleased hatch population, not an active-actor count. Nuke and
        // selected-type/preplaced population generalization remain separate.
        end_.afterSimulationSlice(1.0 / world.rules().ticksPerSecond,
            {world.timedOut(), retainedNonRat, world.setup().totalLemmings - world.spawned(),
             world.saved(), world.setup().saveTarget});
      }
    }
  }
  return ticked;
}

}  // namespace webo::app
