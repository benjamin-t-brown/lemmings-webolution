#pragma once

#include <algorithm>
#include <cmath>

namespace webo::app {

// Playtest R1/R2: connected level entry only. Installed door tracks contain
// 70 keys over 2.8 seconds; 0x451121 adds dt without a speed multiplier.
// PROVISIONAL phase: start opening at level entry and hold the hatch clock
// until BOTH the full clip and the retained entry delay have retired. The
// first-release text is presentation only and must not feed this gate. Original
// startup phase versus the cue is not established by the old ~0.723 s arm estimate.
class LevelStartSequence {
 public:
  explicit LevelStartSequence(double doorSeconds) : duration_(std::max(0.0, doorSeconds)) {}
  void advance(double seconds, bool playing) {
    if (playing && std::isfinite(seconds) && seconds >= 0.0)
      elapsed_ += seconds;
  }
  double openingProgress() const {
    return duration_ > 0.0 ? std::clamp(elapsed_ / duration_, 0.0, 1.0) : 1.0;
  }
  bool releaseEnabled(bool entryDelayActive) const { return !entryDelayActive && elapsed_ >= duration_; }
 private:
  double duration_ = 0.0;
  double elapsed_ = 0.0;
};
}  // namespace webo::app
