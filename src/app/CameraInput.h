#pragma once

#include "app/Camera.h"

namespace webo::app {

// SDL-free port of the accepted A1 input mapping. Mouse deltas describe one
// simulation update; PendingCameraEvents clears them when that update consumes them.
struct CameraInput {
  bool numpad4 = false;
  bool numpad6 = false;
  bool leftShift = false;
  bool backspacePressed = false;
  bool zoomPressed = false;
  bool rightMouseDown = false;
  bool uiAllowsCamera = true;
  int uiState = 0;
  double mouseDx = 0.0;
  double mouseDy = 0.0;
};

class PendingCameraEvents {
 public:
  void mouse(double dx, double dy) { dx_ += dx; dy_ += dy; }
  void zoom() { zoom_ = true; }
  void flip() { flip_ = true; }
  void clear() { dx_ = dy_ = 0.0; zoom_ = flip_ = false; }
  CameraInput consumeInto(CameraInput input) {
    input.mouseDx = dx_; input.mouseDy = dy_;
    input.zoomPressed = zoom_; input.backspacePressed = flip_;
    clear();
    return input;
  }
 private:
  double dx_ = 0.0, dy_ = 0.0;
  bool zoom_ = false, flip_ = false;
};

class FixedCameraClock {
 public:
  static constexpr double stepSeconds = 1.0 / 144.0;
  void advance(double elapsedSeconds);
  bool takeStep();
  double remainder() const { return remainder_; }
 private:
  double remainder_ = 0.0;
};

class CameraController {
 public:
  explicit CameraController(CameraParams initial = {}, int levelRows = 96);
  void update(const CameraInput& input, double dt = 1.0 / 144.0);
  const CameraParams& camera() const { return camera_; }
  double cursorX() const { return cursorX_; }
  double cursorY() const { return cursorY_; }
  double displayedElevation() const { return displayedElevation_; }
  bool grabbed() const { return grabbed_; }
  // Native controller +58 target in terrain coordinates, not eyeWorld and
  // not intro-adjusted display yaw. Ordinary mode2/startup route only.
  const std::array<double, 3>& pickReference() const { return pickReference_; }

 private:
  CameraParams camera_;
  std::array<double, 3> pickReference_;
  double levelRows_;
  double cursorX_ = 320.0;
  double cursorY_ = 240.0;
  double displayedElevation_ = 0.0;
  bool grabbed_ = false;
  double flipElapsed_ = -1.0;
  double flipApplied_ = 0.0;
};

}  // namespace webo::app
