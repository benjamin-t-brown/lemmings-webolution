#pragma once

#include "app/Camera.h"

#include <array>
#include <limits>

namespace webo::app {

// Playtest 9 Y9-Y11, PROVISIONAL clone presentation. Native SPR pixels retain
// their screen placement; their scene-occlusion proxy is an upright plane
// through the actual actor position, facing the camera in the horizontal plane.
// Screen-aligned constant Z gives the feet the head's depth at pitched views.
// A foreground allowance further exposes the figure through walls and floors.
// Neither is needed here; no state/facing-dependent allowance or depth writes.
class ActorOcclusionPlane {
 public:
  ActorOcclusionPlane(const CameraBasis& camera, const std::array<double, 3>& origin) {
    const auto& m = camera.worldToView;
    const double nx = m[2], nz = m[8]; // camera forward projected onto world XZ
    numerator_ = nx * (origin[0] - camera.eyeWorld[0]) +
                 nz * (origin[2] - camera.eyeWorld[2]);
    dx_ = (nx * m[0] + nz * m[6]) / camera.focalX;
    dy_ = -(nx * m[1] + nz * m[7]) / camera.focalY;
    constant_ = nx * m[2] + nz * m[8] - dx_ * camera.cx - dy_ * camera.cy;
  }

  // Supply the scene raster's pixel centre (x+0.5, y+0.5), not the sprite
  // sampler's integer coordinates. Return physical view Z, smaller is nearer.
  double depthAt(double screenX, double screenY) const {
    const double denominator = constant_ + dx_ * screenX + dy_ * screenY;
    // Reject an invalid or behind-camera intersection. No fallback foreground
    // plane: it would allow a near-vertical camera to reveal covered actors.
    return denominator > 0.0 && numerator_ > 0.0 ? numerator_ / denominator :
        std::numeric_limits<double>::infinity();
  }

 private:
  double numerator_, dx_, dy_, constant_;
};

}  // namespace webo::app
