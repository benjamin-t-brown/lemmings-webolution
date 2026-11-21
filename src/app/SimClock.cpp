#include "app/SimClock.h"

#include <algorithm>

namespace webo::app {

void SimClock::advance(double elapsedSeconds) {
  remainder_ += std::max(0.0, elapsedSeconds);
  ticksTakenThisFrame_ = 0;
}

bool SimClock::takeStep() {
  if (ticksTakenThisFrame_ >= maxTicksPerFrame_)
    return false;
  if (remainder_ + 1e-12 < stepSeconds)
    return false;
  remainder_ -= stepSeconds;
  ++ticksTakenThisFrame_;
  return true;
}

}  // namespace webo::app
