// Camera pose and lens transform. Projection is x/z + centerX and
// centerY - y/z. Defaults describe the MM_CLIMB_002 view.
// The clone approximates the native x87 rounding sequence.

#pragma once

#include <array>
#include <filesystem>
#include <optional>
#include <string>

namespace webo::app {

struct CameraParams {
  int width = 640;
  int height = 480;
  double cx = 319.0;
  double cy = 239.0;
  double aspect = 4.0 / 3.0;
  double angleDegrees = 19.03;
  double yawUnits = 32.0;
  double yawOffset = 128.0;
  double yawUnitsPerTurn = 256.0;
  int zoomShift = 0;
  double levelEyeHeight = 96.0;
  double heightBias = 17.392;
  double eyeDistance = 377.516;
  double fixedPitchTurns = -0.03135770272046;
  double posePi = 3.1415927410125732;
  double lensPi = 3.1415927410125732;
  double angleUnitsPerTurn = 360.0;
  double elevation = 0.0;
  double zoomLateralOffset = 0.0;  // camera-local eye X, used only when zoomed
  // Opening-pose parameters.
  bool introActive = false;
  double introSeconds = 0.0;

  // Optional control_case input for the grabbed-key and mouse yaw calculation.
  bool hasControlCase = false;
  double mouseGain = 10.0;
  double rotationUnit = 24.0;
  int leftScan = 75;
  int rightScan = 77;
  double inversion = 1.0;
  int updates = 0;
  int scanCode = -1;   // -1 when the case drives no key
  double mouseDx = 0.0;
};

struct CameraBasis {
  std::array<double, 12> worldToView{};  // 9 basis values, then translation
  // Camera-context scales before native normalization.
  std::array<double, 3> projectionVector{};
  std::array<double, 3> inverseProjectionVector{};
  std::array<double, 3> eyeWorld{};
  double focalX = 0.0;
  double focalY = 0.0;
  int width = 640;
  int height = 480;
  double cx = 319.0;
  double cy = 239.0;
  // Camera angle in turns, (yaw + yaw_offset) / yaw_units_per_turn.
  // The tube walker (`0x43A4EC`) converts it to a bucket centre with
  // `0x43c990 -> fsub 0.5, fmul 32.0`.
  double selectorTurns = 0.0;
  // View-Z translation from this camera to zoom 0 at the same yaw/intro phase.
  // PROVISIONAL sprite presentation: preserve the zoom-0 pixel size, then
  // magnify offsets by the same depth ratio as terrain at the sprite origin.
  // Zero at default zoom, including throughout the opening camera movement.
  double defaultZoomDepthOffset = 0.0;
  double zoomScaleAtDepth(double viewDepth) const {
    return (viewDepth + defaultZoomDepthOffset) / viewDepth;
  }
  std::array<double, 3> worldRay(double screenX, double screenY) const {
    const double x = (screenX - cx) / focalX;
    const double y = (cy - screenY) / focalY;
    const auto& m = worldToView;
    return {m[0] * x + m[1] * y + m[2],
            m[3] * x + m[4] * y + m[5],
            m[6] * x + m[7] * y + m[8]};
  }
};

struct CameraZoomPose {
  double lateralOffset;
  double elevation;
};

struct BillboardCameraChain {
  std::array<double, 12> parentCamera{};
  std::array<double, 12> sourceTransform{};
  std::array<double, 12> projectionTransform{};
};

// Reads the flat numeric keys of a camera params JSON. Returns false and an
// error when the file cannot be read or a required key is missing.
bool loadCameraParams(const std::filesystem::path& path, CameraParams& out, std::string& error);

// Computes grabbed-key and mouse yaw. With no control_case, returns the input yaw.
double resolvedYawUnits(const CameraParams& p);

CameraBasis buildCamera(const CameraParams& p);
// Provisional clone input: preserve the displayed point at the given pixel
// and view depth when switching to zoom 1. zoomBase supplies the next pose's
// intro phase, if any. A missing or unreachable depth leaves zoom unanchored.
std::optional<CameraZoomPose> cursorZoomPose(const CameraBasis& displayed,
    CameraParams zoomBase, double screenX, double screenY, double viewDepth);
BillboardCameraChain buildBillboardCameraChain(const CameraBasis& camera);

}  // namespace webo::app
