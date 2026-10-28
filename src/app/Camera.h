// The A1 camera: the verified pose/lens operands, assembled into one transform.
//
// This is a direct port of the accepted A1 arithmetic
// (tools/harness/camera_fixture.py, docs/scratch/phase-a1-camera.md and
// phase-a1-live-arena.md), not a reconstruction and not a decompiler reading.
// Operands live in a small params file so the nine retained camera states can
// be replayed exactly; the defaults are the canonical MM_CLIMB_002 view.
//
// Projection is x/z + centerX, centerY - y/z with the measured lens scale
// vector. It does not use a fourth homogeneous coordinate. The exact x87
// rounding sequence is not reproduced; the accepted fixture is a candidate
// that passes the fixed pixel gate, not a bit-exact arithmetic replay.

#pragma once

#include <array>
#include <filesystem>
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
  // Explicit opening-pose state; defaults preserve the settled canonical view.
  bool introActive = false;
  double introSeconds = 0.0;

  // Bounded unmodified-key/mouse grab case. Absent unless the params file
  // carries a control_case, which is the only thing that consumes these.
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
  // A1 camera-context scale operands, before native context normalization.
  std::array<double, 3> projectionVector{};
  std::array<double, 3> inverseProjectionVector{};
  std::array<double, 3> eyeWorld{};
  double focalX = 0.0;
  double focalY = 0.0;
  int width = 640;
  int height = 480;
  double cx = 319.0;
  double cy = 239.0;
  // The camera's own angular position in turns: `(yaw + yaw_offset) /
  // yaw_units_per_turn`, the operand the tube walker (`0x43A4EC`) turns into a
  // bucket centre with `0x43c990 -> fsub 0.5, fmul 32.0`. Same expression as
  // the A1 fixture's `selector`, which drives the pass bounds there.
  double selectorTurns = 0.0;
};

struct BillboardCameraChain {
  std::array<double, 12> parentCamera{};
  std::array<double, 12> sourceTransform{};
  std::array<double, 12> projectionTransform{};
};

// Reads the flat numeric keys of a camera params JSON. Returns false and an
// error when the file cannot be read or a required key is missing.
bool loadCameraParams(const std::filesystem::path& path, CameraParams& out, std::string& error);

// The bounded grabbed-key/mouse yaw from the A1 control mapping. Returns the
// params' yaw unchanged when there is no control_case.
double resolvedYawUnits(const CameraParams& p);

CameraBasis buildCamera(const CameraParams& p);
BillboardCameraChain buildBillboardCameraChain(const CameraBasis& camera);

}  // namespace webo::app
