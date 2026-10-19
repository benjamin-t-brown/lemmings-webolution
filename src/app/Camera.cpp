#include "Camera.h"

#include <cmath>
#include <cstdlib>
#include <fstream>
#include <sstream>

namespace webo::app {
namespace {

// Minimal scalar extractor for the camera params files: they are flat objects
// of numbers and short arrays, so a full JSON parser would be more rope than
// this needs. Every lookup is for an exact quoted key followed by ':'.
bool findValue(const std::string& text, const std::string& key, double& out) {
  const std::string needle = "\"" + key + "\"";
  const auto at = text.find(needle);
  if (at == std::string::npos)
    return false;
  const auto colon = text.find(':', at + needle.size());
  if (colon == std::string::npos)
    return false;
  const char* start = text.c_str() + colon + 1;
  char* end = nullptr;
  const double value = std::strtod(start, &end);
  if (end == start)
    return false;
  out = value;
  return true;
}

bool findInt(const std::string& text, const std::string& key, int& out) {
  double v = 0;
  if (!findValue(text, key, v))
    return false;
  out = static_cast<int>(std::llround(v));
  return true;
}

// The leading number of a "[a, b]" array named key.
bool findArrayFirst(const std::string& text, const std::string& key, double& out) {
  const std::string needle = "\"" + key + "\"";
  const auto at = text.find(needle);
  if (at == std::string::npos)
    return false;
  const auto bracket = text.find('[', at + needle.size());
  if (bracket == std::string::npos)
    return false;
  const char* start = text.c_str() + bracket + 1;
  char* end = nullptr;
  const double value = std::strtod(start, &end);
  if (end == start)
    return false;
  out = value;
  return true;
}

bool findArraySecond(const std::string& text, const std::string& key, double& out) {
  const std::string needle = "\"" + key + "\"";
  const auto at = text.find(needle);
  if (at == std::string::npos)
    return false;
  const auto bracket = text.find('[', at + needle.size());
  if (bracket == std::string::npos)
    return false;
  const auto comma = text.find(',', bracket);
  if (comma == std::string::npos)
    return false;
  const char* start = text.c_str() + comma + 1;
  char* end = nullptr;
  const double value = std::strtod(start, &end);
  if (end == start)
    return false;
  out = value;
  return true;
}

}  // namespace

bool loadCameraParams(const std::filesystem::path& path, CameraParams& out, std::string& error) {
  std::ifstream in(path, std::ios::binary);
  if (!in) {
    error = "cannot open camera params: " + path.string();
    return false;
  }
  std::stringstream ss;
  ss << in.rdbuf();
  const std::string text = ss.str();

  double v = 0;
  if (findArrayFirst(text, "viewport", v)) out.width = static_cast<int>(std::llround(v));
  if (findArraySecond(text, "viewport", v)) out.height = static_cast<int>(std::llround(v));
  if (findArrayFirst(text, "center", v)) out.cx = v;
  if (findArraySecond(text, "center", v)) out.cy = v;
  if (findValue(text, "aspect", v)) out.aspect = v;
  if (findValue(text, "angle_degrees", v)) out.angleDegrees = v;
  if (findValue(text, "yaw_units", v)) out.yawUnits = v;
  if (findValue(text, "yaw_offset", v)) out.yawOffset = v;
  if (findValue(text, "yaw_units_per_turn", v)) out.yawUnitsPerTurn = v;
  if (findInt(text, "zoom_shift", out.zoomShift)) { /* set */ }
  if (findValue(text, "level_eye_height", v)) out.levelEyeHeight = v;
  if (findValue(text, "height_bias", v)) out.heightBias = v;
  if (findValue(text, "eye_distance", v)) out.eyeDistance = v;
  if (findValue(text, "fixed_pitch_turns", v)) out.fixedPitchTurns = v;
  if (findValue(text, "pose_pi", v)) out.posePi = v;
  if (findValue(text, "lens_pi", v)) out.lensPi = v;
  if (findValue(text, "angle_units_per_turn", v)) out.angleUnitsPerTurn = v;
  if (findValue(text, "elevation", v)) out.elevation = v;

  if (text.find("\"control_case\"") != std::string::npos) {
    out.hasControlCase = true;
    if (findValue(text, "mouse_gain", v)) out.mouseGain = v;
    if (findValue(text, "rotation_unit", v)) out.rotationUnit = v;
    if (findInt(text, "left_scan", out.leftScan)) { /* set */ }
    if (findInt(text, "right_scan", out.rightScan)) { /* set */ }
    if (findValue(text, "inversion", v)) out.inversion = v;
    if (findInt(text, "updates", out.updates)) { /* set */ }
    if (findInt(text, "scan_code", out.scanCode)) { /* set */ }
    if (findValue(text, "mouse_dx", v)) out.mouseDx = v;
  }
  return true;
}

double resolvedYawUnits(const CameraParams& p) {
  if (!p.hasControlCase)
    return p.yawUnits;
  // Port of camera_fixture.grabbed_mode2_yaw. One step is one update of the
  // traced control path; mouse movement only contributes on the first update.
  double yaw = p.yawUnits;
  for (int step = 0; step < p.updates; ++step) {
    double local = 0.0;
    if (p.scanCode == p.leftScan)
      local -= p.rotationUnit;
    else if (p.scanCode == p.rightScan)
      local += p.rotationUnit;
    if (step == 0)
      local += p.mouseDx * p.mouseGain;
    yaw += -local * p.inversion / p.rotationUnit;
    while (yaw > 256.0) yaw -= 256.0;
    while (yaw < 0.0) yaw += 256.0;
  }
  return yaw;
}

CameraBasis buildCamera(const CameraParams& p) {
  CameraBasis out;
  out.width = p.width;
  out.height = p.height;
  out.cx = p.cx;
  out.cy = p.cy;

  const int n = p.zoomShift;
  const double scale = static_cast<double>(1u << n);
  const double tau = 2.0 * p.posePi;
  const double yawUnits = resolvedYawUnits(p);
  const double yawTurns = (yawUnits + p.yawOffset) / p.yawUnitsPerTurn;
  out.selectorTurns = yawTurns;
  const double yawAngle = tau * yawTurns;
  const double pitchAngle = tau * p.fixedPitchTurns;
  const double cy = std::cos(yawAngle), sy = std::sin(yawAngle);
  const double cp = std::cos(pitchAngle), sp = std::sin(pitchAngle);

  // Row/column entries as written by FUN_004821B0 in the two constructors.
  const double yaw[3][3] = {{cy, 0.0, -sy}, {0.0, 1.0, 0.0}, {sy, 0.0, cy}};
  const double pitch[3][3] = {{1.0, 0.0, 0.0}, {0.0, cp, sp}, {0.0, -sp, cp}};
  double pose[3][3];
  for (int i = 0; i < 3; ++i)
    for (int j = 0; j < 3; ++j) {
      double s = 0.0;
      for (int k = 0; k < 3; ++k)
        s += yaw[i][k] * pitch[k][j];
      pose[i][j] = s;
    }

  const double eyeLocal[3] = {
      0.0,
      (p.levelEyeHeight + p.heightBias) / scale + (n != 0 ? p.elevation : 0.0),
      -p.eyeDistance / scale};
  // The raw pose builder applies yaw to the local eye vector (not transpose(yaw)).
  double eyeWorld[3] = {0.0, 0.0, 0.0};
  for (int i = 0; i < 3; ++i)
    for (int j = 0; j < 3; ++j)
      eyeWorld[i] += yaw[i][j] * eyeLocal[j];
  out.eyeWorld = {eyeWorld[0], eyeWorld[1], eyeWorld[2]};

  // Physical pose is Y*X. The view basis is its transpose.
  for (int i = 0; i < 3; ++i)
    for (int j = 0; j < 3; ++j)
      out.worldToView[i * 3 + j] = pose[i][j];
  for (int i = 0; i < 3; ++i) {
    double s = 0.0;
    for (int j = 0; j < 3; ++j)
      s += pose[j][i] * eyeWorld[j];
    out.worldToView[9 + i] = -s;
  }

  const double thetaTurns = p.angleDegrees / p.angleUnitsPerTurn;
  const double halfAngle = p.lensPi * thetaTurns;
  const double g = 1.0 / (2.0 * std::tan(halfAngle));
  out.focalX = (g / p.aspect) / (1.0 / p.width);
  out.focalY = (g * p.height / p.width) / (1.0 / p.width);
  return out;
}

}  // namespace webo::app
