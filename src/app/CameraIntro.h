#pragma once

#include "app/Camera.h"

namespace webo::app {

// Retained 45CC09 entry-pose equation, not a claim that a native runtime clock
// or live intro state has been observed.
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
