#pragma once

#include "app/Camera.h"

namespace webo::app {

// Entry pose follows the equation at 45CC09.
class CameraIntro {
 public:
  explicit CameraIntro(bool enabled = false) { reset(enabled); }
  void reset(bool enabled);
  CameraParams pose(const CameraParams& base) const;
  void advanceAfterFrame(double elapsedSeconds, bool paused);
  bool active() const { return active_; }
  double seconds() const { return seconds_; }

 private:
  bool active_ = false;
  double seconds_ = 0.0;
};

}  // namespace webo::app
