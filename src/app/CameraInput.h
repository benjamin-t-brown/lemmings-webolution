#pragma once

#include "app/Camera.h"

namespace webo::app {

// SDL-free camera input. Mouse deltas describe one fixed camera update.
struct CameraInput {
  bool numpad4 = false;
  bool numpad6 = false;
  // Clone addition: arrow keys rotate the camera without a drag.
  bool arrowLeft = false;
  bool arrowRight = false;
  // Clone addition: invert drag rotation and tilt (Options setting).
  bool reverseDrag = false;
  bool leftShift = false;
  bool backspacePressed = false;
  bool zoomPressed = false;
  std::optional<CameraZoomPose> zoomPose;
  bool rightMouseDown = false;
  bool uiAllowsCamera = true;
  int uiState = 0;
  double mouseDx = 0.0;
  double mouseDy = 0.0;
};

class PendingCameraEvents {
 public:
  void mouse(double dx, double dy) { dx_ += dx; dy_ += dy; }
  void zoom(std::optional<CameraZoomPose> pose = std::nullopt) {
    zoom_ = true;
    zoomPose_ = pose;
  }
  void flip() { flip_ = true; }
  void clearMouse() { dx_ = dy_ = 0.0; }
  void clear() {
    clearMouse();
    zoom_ = flip_ = false;
    zoomPose_.reset();
  }
  CameraInput consumeInto(CameraInput input) {
    input.mouseDx = dx_; input.mouseDy = dy_;
    input.zoomPressed = zoom_; input.backspacePressed = flip_;
    input.zoomPose = zoomPose_;
    clear();
    return input;
  }
 private:
  double dx_ = 0.0, dy_ = 0.0;
  bool zoom_ = false, flip_ = false;
  std::optional<CameraZoomPose> zoomPose_;
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
  void toggleZoom(std::optional<CameraZoomPose> pose);
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
