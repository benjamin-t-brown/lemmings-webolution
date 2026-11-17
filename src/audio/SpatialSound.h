#pragma once

#include "app/Camera.h"

#include <algorithm>
#include <array>
#include <cmath>

namespace webo::audio {

struct StereoGains {
  float left = 1.0f;
  float right = 1.0f;
};

// PROVISIONAL playable-v1 curve, inspired by 0x4375F0's camera-X/depth
// rule, not a reproduction of its transform scales or DirectSound dB curve.
// Screen edges pan fully; the back of the cylinder is 25% of front gain.
// A one-shot snapshots these gains: camera motion never retunes its tail.
inline StereoGains spatialSoundGains(const app::CameraBasis& camera,
                                     const std::array<double, 3>& point) {
  const auto& m = camera.worldToView;
  const double x = m[0]*point[0] + m[3]*point[1] + m[6]*point[2] + m[9];
  const double z = m[2]*point[0] + m[5]*point[1] + m[8]*point[2] + m[11];
  if (!std::isfinite(x) || !std::isfinite(z) || z <= 0 || camera.width <= 0)
    return {0.0f, 0.0f};
  const double screenX = camera.cx + camera.focalX * x/z;
  const double pan = std::clamp((screenX - camera.cx)/(camera.width*0.5), -1.0, 1.0);
  const double radius = std::hypot(point[0], point[2]);
  const double eyeRadius = std::hypot(camera.eyeWorld[0], camera.eyeWorld[2]);
  const double facing = radius > 0 && eyeRadius > 0 ? std::clamp(
      (point[0]*camera.eyeWorld[0] + point[2]*camera.eyeWorld[2])/(radius*eyeRadius),
      -1.0, 1.0) : 1.0;
  const double volume = 0.25 + 0.75*(facing + 1.0)*0.5;
  return {static_cast<float>(volume*(1.0-std::max(0.0, pan))),
          static_cast<float>(volume*(1.0+std::min(0.0, pan)))};
}

}  // namespace webo::audio
