#include "app/CameraInput.h"

#include <algorithm>
#include <cmath>

namespace webo::app {
namespace {
double clamp(double v, double lo, double hi) { return std::max(lo, std::min(hi, v)); }
double wrapYaw(double v) {
  while (v >= 256.0) v -= 256.0;
  while (v < 0.0) v += 256.0;
  return v;
}
}

void FixedCameraClock::advance(double elapsedSeconds) {
  remainder_ += std::max(0.0, std::min(elapsedSeconds, 0.25));
}

bool FixedCameraClock::takeStep() {
  if (remainder_ + 1e-12 < stepSeconds) return false;
  remainder_ -= stepSeconds;
  return true;
}

CameraController::CameraController(CameraParams initial) : camera_(initial) {}

void CameraController::update(const CameraInput& in, double dt) {
  constexpr double flipDuration = 0.384;
  grabbed_ = in.uiAllowsCamera && (in.uiState == 0 || in.uiState == 1) && in.rightMouseDown;

  if (in.zoomPressed && in.uiAllowsCamera && in.uiState == 0) {
    camera_.zoomShift = camera_.zoomShift == 0 ? 1 : 0;
    if (camera_.zoomShift != 0) {
      camera_.elevation = clamp(47.5 - 58.0 * ((cursorY_ - 130.0) / 220.0), -10.5, 47.5);
      const double edgeRatio = (cursorX_ - 320.0) / 90.0;
      if (std::abs(edgeRatio) >= 0.1)
        camera_.yawUnits += 30.0 * clamp(edgeRatio, -1.0, 1.0);
    }
  }

  if (in.backspacePressed && in.uiAllowsCamera && (in.uiState == 0 || in.uiState == 1)) {
    flipElapsed_ = 0.0;
    flipApplied_ = 0.0;
  }
  if (flipElapsed_ >= 0.0 && in.uiAllowsCamera && (in.uiState == 0 || in.uiState == 1)) {
    flipElapsed_ = std::min(flipDuration, flipElapsed_ + dt);
    const double u = clamp(flipElapsed_ / flipDuration, 0.0, 1.0);
    const double a = (2.0 - u) * u;
    const double total = 128.0 * ((2.0 - a) * a);
    camera_.yawUnits += total - flipApplied_;
    flipApplied_ = total;
    if (flipElapsed_ >= flipDuration) flipElapsed_ = -1.0;
  }

  if (grabbed_) {
    const double step = in.leftShift ? 3.0 : 1.0;
    if (in.numpad4) camera_.yawUnits += step;
    if (in.numpad6) camera_.yawUnits -= step;
    camera_.yawUnits -= (10.0 / 24.0) * in.mouseDx;
    if (camera_.zoomShift == 0) {
      displayedElevation_ -= 0.5 * in.mouseDy;
    } else {
      camera_.elevation = clamp(camera_.elevation + 1.5 * in.mouseDy, -10.5, 47.5);
      displayedElevation_ -= 0.125 * in.mouseDy;
    }
  } else {
    if (in.numpad4) cursorX_ -= 4.8;
    if (in.numpad6) cursorX_ += 4.8;
    cursorX_ += 2.0 * in.mouseDx;
    cursorY_ += 2.4 * in.mouseDy;
    cursorX_ = clamp(cursorX_, 0.0, static_cast<double>(camera_.width - 1));
    cursorY_ = clamp(cursorY_, 0.0, static_cast<double>(camera_.height - 1));
  }
  camera_.yawUnits = wrapYaw(camera_.yawUnits);
}

}  // namespace webo::app
