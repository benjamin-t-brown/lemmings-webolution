#include "app/CameraIntro.h"

#include <cmath>

namespace webo::app {

void CameraIntro::reset(bool enabled) {
  active_ = enabled;
  seconds_ = 0.0;
}

CameraParams CameraIntro::pose(const CameraParams& base) const {
  CameraParams result = base;
  result.introActive = active_;
  result.introSeconds = active_ ? seconds_ : 0.0;
  return result;
}

void CameraIntro::advanceAfterFrame(double elapsedSeconds, bool paused) {
  if (!active_ || paused || !std::isfinite(elapsedSeconds) || elapsedSeconds < 0.0)
    return;
  seconds_ += elapsedSeconds;
  if (seconds_ > 3.0)
    active_ = false;
}

}  // namespace webo::app
