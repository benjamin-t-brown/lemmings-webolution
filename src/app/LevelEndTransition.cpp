#include "app/LevelEndTransition.h"

#include <cstdint>

namespace webo::app {

void LevelEndTransition::afterSimulationSlice(double seconds, const LevelEndInputs& in) {
  if (phase_ == LevelEndPhase::Results) return;
  if (phase_ == LevelEndPhase::Running) {
    if (!in.timedOut && static_cast<std::int64_t>(in.retainedNonRat) + in.selectedUnreleased != 0)
      return;
    winLatched_ = in.saved >= in.needed;
    phase_ = LevelEndPhase::Pending;
    return; // Trigger call does not decrement/reseed the timer.
  }
  timer_ -= seconds;
  if (timer_ < 3.0) {
    if (!effectActive_) {
      effectActive_ = true;
      effectElapsed_ = 0.0;
    } else {
      effectElapsed_ += seconds;
      effectActive_ = effectElapsed_ <= 2.0;
    }
  }
  if (timer_ < 1.0) {
    effectActive_ = false;
    timer_ = 7.0;
    // Native globals publish CURRENT counts even though the result flag was
    // latched on the earlier trigger. Late rescues cannot upgrade a loss.
    snapshot_ = LevelEndSnapshot{winLatched_, in.saved, in.needed};
    phase_ = LevelEndPhase::Results;
  }
}

} // namespace webo::app
