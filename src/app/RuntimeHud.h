#pragma once

#include "assets/Bitmap.h"
#include "shell/PauseCaption.h"
#include "sim/World.h"
#include "app/SimClock.h"
#include "app/LevelEndTransition.h"

#include <functional>

namespace webo::app {

// Production window composition: values are read from the same World that
// supplies the scene actors. The caller owns the cached font sheet.
shell::HudValues runtimeHudValues(const sim::World& world);
void drawRuntimeHud(webo::Image& frame, const webo::Image& font,
                    const sim::World& world, bool paused);

// Shared production state gate used by the SDL loop and the deterministic
// offline integration test. Pausing discards the accumulator and elapsed
// samples while paused; resuming begins a fresh SimClock interval.
class RuntimeSimState {
 public:
  bool paused() const { return paused_; }
  void togglePause();
  void resume();
  // afterTick, when set, runs once after every World::tick() so per-tick
  // products (the sound cues) are taken before the next tick can coalesce them.
  //
  // LEGACY / COMPATIBILITY WRAPPER (SimClock + World::tick, fixed 0.2 s ticks). Deterministic
  // tick-based fixtures keep their documented contract through it; connected play uses
  // advanceElapsed below.
  bool advance(sim::World& world, double elapsedSeconds, bool trackCompletion = false,
               const std::function<void()>& afterTick = {});

  // Shared explicit elapsed-time path used by the native application (shared-timing-impl-001).
  // Input contract (frame loop 0x45CFF9, gate-cadence-research-001 section 1):
  //  * `elapsedSeconds` becomes INTEGER milliseconds. The original gets integer timeGetTime
  //    deltas; this clone measures a double, so the sub-millisecond fraction is carried
  //    (the sum of integer dt equals real elapsed ms). A frame that yields 0 ms still runs one
  //    zero-delta world update per pass: contact and bookkeeping execute, nothing moves.
  //  * `stallCapMs` > 0 clamps a frame to that many ms (the original's 100.0 < dt test; exactly
  //    100 is unclamped). The discarded time is NOT carried (kStallCapMs for the wall clock).
  //    0 disables the clamp: used only for owned deterministic clocks (`--frames`,
  //    ApplicationRunOptions::frameSeconds), where the input is a declared simulated step.
  //  * Pause discards input and resets the carry; the first unpaused frame carries only its own
  //    delta. A fresh RuntimeSimState plus a freshly loaded World is a reload.
  //  * fast-forward repeats the same integer dt for kFastForwardPasses passes; each pass is one
  //    World::advanceElapsedMs call (level-second splitting, lemmings, then lever contact).
  // `afterTick` runs after every world update (cue drain); the completion slice is fed that
  // update's own dt. Returns true when at least one world update ran.
  bool advanceElapsed(sim::World& world, double elapsedSeconds, bool trackCompletion = false,
                      const std::function<void()>& afterTick = {}, int stallCapMs = kStallCapMs);
  // Same shared-dt path with an explicit start-only hatch gate. Retain the
  // five-argument entry above for existing callers and frozen demonstrations.
  bool advanceElapsed(sim::World& world, double elapsedSeconds, bool trackCompletion,
                      const std::function<void()>& afterTick, int stallCapMs, bool releaseEnabled);
  static constexpr int kStallCapMs = 100;  // 0x45D927-0x45DA3C
  // Observations of the shared input contract (for tests and diagnostics).
  int lastPassMs() const { return lastPassMs_; }
  long long discardedStallMs() const { return discardedStallMs_; }
  double carriedMs() const { return carryMs_; }
  const LevelEndTransition& endTransition() const { return end_; }

  // Fast-forward pass budget: dword 0x4B15F8 (READ 0x45DB0E-0x45DD5C). This is
  // a PASS COUNT per frame, not a measured 7x wall-clock speedup and not a
  // 200 ms tick. Each pass reuses the whole frame delta through SimClock, so
  // SimClock quantization and its per-advance cap still apply to every pass.
  // advanceElapsed applies the shared 100 ms clamp for the real wall clock.
  static constexpr int kFastForwardPasses = 7;
  bool fastForward() const { return fastForward_; }
  // Explicit enable retained for deterministic fixtures and legacy callers.
  void latchFastForward() { fastForward_ = true; }
  // PROVISIONAL player policy (P17): another plate-8 gesture returns to normal
  // speed. Static 0x455AFF only sets input+0x20; retained reset paths do not
  // settle how a player cancels it in the original. Pause keeps this mode;
  // constructing a new per-level state clears it. Pass timing is unchanged.
  void toggleFastForward() { fastForward_ = !fastForward_; }

 private:
  SimClock clock_;
  double carryMs_ = 0.0;
  int lastPassMs_ = 0;
  long long discardedStallMs_ = 0;
  bool paused_ = false;
  bool fastForward_ = false;
  LevelEndTransition end_;
};

}  // namespace webo::app
