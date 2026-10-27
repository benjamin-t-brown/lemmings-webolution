// Explicit-input fresh-cache source-domain scene polygon clipper.
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
// Software binary64 finite-cache-miss model: near clip physical XYZ with native
// Z=scale*viewZ, overwrite generated viewZ=epsilon/scale; then project
// (cx+projectionX*x/z, cy-projectionY*y/z), and set biasedNativeZ=scale*z-bias.
// Screen planes are inclusive bottom/top/right/left. Endpoint H/L is sorted by
// clipping coordinate (and by biased Z for near); screen other-coordinate uses
// t, while XYZ/depth/attributes use C3bj p. Zero p denominator selects H. No
// cache-hit emulation, f32/x87 claim, final packing, device defaults or source
// submission claim. Rejects nonfinite/intermediate magnitude >1e100, invalid
// scale/epsilon, reversed bounds, or >4096 inputs before mutating caller data.
std::expected<std::vector<SceneClipVertex>,std::string> clipScenePolygon(
    const std::vector<SceneClipInputVertex>& polygon,const SceneClipInputs& inputs);
} // namespace webo::app
