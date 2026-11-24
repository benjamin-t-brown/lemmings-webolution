#pragma once

#include <algorithm>
#include <cmath>
#include <optional>

namespace webo::app {

// Installed door tracks have 70 keys over 2.8 seconds. 0x451121 advances the
// clip and 0x425A95 starts the first countdown when the clip finishes.
// The opening update contributes only the time after clip completion.
class LevelStartSequence {
 public:
  explicit LevelStartSequence(double doorSeconds) : duration_(std::max(0.0, doorSeconds)) {}
  std::optional<double> advance(double seconds, bool playing) {
    if (!playing || !std::isfinite(seconds) || seconds < 0.0)
      return std::nullopt;
    const double untilOpen = duration_ - elapsed_;
    elapsed_ += seconds;
    if (untilOpen > 0.0 && releaseEnabled())
      return untilOpen;
    return std::nullopt;
  }
  double openingProgress() const {
    return duration_ > 0.0 ? std::clamp(elapsed_ / duration_, 0.0, 1.0) : 1.0;
  }
  bool releaseEnabled() const { return elapsed_ >= duration_; }
  // Preserve the former gate for historical source fixtures.
  bool releaseEnabled(bool cueActive) const { return !cueActive && releaseEnabled(); }
 private:
  double duration_ = 0.0;
  double elapsed_ = 0.0;
};
}  // namespace webo::app
