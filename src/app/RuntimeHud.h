#pragma once

#include "assets/Bitmap.h"
#include "shell/PauseCaption.h"
#include "sim/World.h"
#include "app/SimClock.h"
#include "app/LevelEndTransition.h"

#include <functional>

namespace webo::app {

// Compose the HUD from World state and a caller-owned font sheet.
shell::HudValues runtimeHudValues(const sim::World& world);
void drawRuntimeHud(webo::Image& frame, const webo::Image& font,
                    const sim::World& world, bool paused);

// Pause discards accumulated and incoming time. Resume starts a fresh interval.
class RuntimeSimState {
 public:
  bool paused() const { return pause_ != Pause::Playing; }
  bool examining() const { return pause_ == Pause::Examine; }
  void togglePause();
  void toggleExaminePause();
  void resume();
  // Advance World::tick in 0.2 s steps through SimClock. afterTick can drain
  // each tick's sound cues before the next tick can coalesce them.
  bool advance(sim::World& world, double elapsedSeconds, bool trackCompletion = false,
               const std::function<void()>& afterTick = {});

  // Original frame loop 0x45CFF9 derives ms deltas from timeGetTime.
  // Convert seconds to whole ms with carry. Each pass updates contacts at zero ms.
  // A positive stallCapMs caps the frame and discards excess. Zero disables
  // the cap for supplied clocks. Pause resets the carry and discards input.
  // Fast-forward repeats the full integer delta once per pass. World splits
  // at level-second boundaries and updates lemmings before lever contact.
  // afterTick follows each update. The completion slice uses that update's
  // dt. Returns true if at least one update executed.
  bool advanceElapsed(sim::World& world, double elapsedSeconds, bool trackCompletion = false,
                      const std::function<void()>& afterTick = {}, int stallCapMs = kStallCapMs);
  // Explicit elapsed-time path with a start-only hatch gate.
  bool advanceElapsed(sim::World& world, double elapsedSeconds, bool trackCompletion,
                      const std::function<void()>& afterTick, int stallCapMs, bool releaseEnabled);
  static constexpr int kStallCapMs = 100;  // 0x45D927-0x45DA3C
  // Last pass budget, discarded stall time and carried fractional milliseconds.
  int lastPassMs() const { return lastPassMs_; }
  long long discardedStallMs() const { return discardedStallMs_; }
  double carriedMs() const { return carryMs_; }
  const LevelEndTransition& endTransition() const { return end_; }

  // Fast-forward passes per frame, from dword 0x4B15F8 (0x45DB0E-0x45DD5C).
  // Each pass reuses the full frame delta.
  static constexpr int kFastForwardPasses = 7;
  bool fastForward() const { return fastForward_; }
  void setFastForward(bool active) { fastForward_ = active; }
  // Change the flag that selects the pass count on the next advance.
  void latchFastForward() { setFastForward(true); }
  void toggleFastForward() { setFastForward(!fastForward_); }

 private:
  enum class Pause { Playing, Standard, Examine };
  void setPause(Pause pause);
  SimClock clock_;
  double carryMs_ = 0.0;
  int lastPassMs_ = 0;
  long long discardedStallMs_ = 0;
  Pause pause_ = Pause::Playing;
  bool fastForward_ = false;
  LevelEndTransition end_;
};

}  // namespace webo::app
