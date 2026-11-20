#pragma once

#include <algorithm>
#include <cmath>

namespace webo::app {

// Connected level entry. Installed door tracks have 70 keys over 2.8 seconds.
// 0x451121 adds dt without scaling. Provisional clone sequencing starts the
// doors on entry and enables release after the full clip and EntryStartCue.
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
  bool releaseEnabled(bool cueActive) const { return !cueActive && elapsed_ >= duration_; }
 private:
  double duration_ = 0.0;
  double elapsed_ = 0.0;
};
}  // namespace webo::app
