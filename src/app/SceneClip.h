// Explicit-input scene polygon clipper for a fresh cache entry.
#pragma once
#include <array>
#include <expected>
#include <string>
#include <vector>

namespace webo::app {
struct SceneClipInputVertex {
  double viewX=0, viewY=0, viewZ=0; // physical view-space XYZ before projection
  std::array<double,6> attributes{}; // UV, RGB, alpha by caller convention
};
struct SceneClipVertex {
  double viewX=0, viewY=0, viewZ=0; // screen-clip interpolated XYZ (physical view units)
  double screenX=0, screenY=0;      // projected XY; clipping coordinate uses t
  double biasedNativeZ=0;            // nativeScale*viewZ - nativeBias, applied once
  std::array<double,6> attributes{};
};
struct SceneClipInputs {
  double nativeScale;  // Pzz, physical view depth -> native pre-bias Z
  double nativeBias;   // camera+770 in native units; subtracted at projection
  double nearEpsilon;  // native domain; only generated near Z is overwritten
  double projectionX, projectionY; // screen ratio multipliers
  double centerX, centerY;
  int bottom, top, right, left; // signed-i32 bounds in this order
};
// Clip near in binary64 with native Z = scale*viewZ and set generated
// viewZ to epsilon/scale. Project XY with projectionX and projectionY.
// Set biasedNativeZ = scale*z-bias. Native f32/x87 rounding is unresolved.
// Screen planes are inclusive bottom/top/right/left. Sort endpoints by
// clipping coordinate, and by biased Z at near. Screen position uses t;
// XYZ, depth and attributes use p. A zero p denominator selects H.
// Reject invalid scale, epsilon, bounds, nonfinite or excessive coordinates
// (>1e100 in magnitude), and inputs over 4096 vertices.
std::expected<std::vector<SceneClipVertex>,std::string> clipScenePolygon(
    const std::vector<SceneClipInputVertex>& polygon,const SceneClipInputs& inputs);
} // namespace webo::app
