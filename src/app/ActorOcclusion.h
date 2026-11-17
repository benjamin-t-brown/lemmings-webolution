#pragma once

#include "app/Camera.h"
#include "assets/CollisionGrid.h"
#include "assets/Cylinder.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <limits>
#include <numbers>

namespace webo::app {

// Playtest 9 Y9-Y11, PROVISIONAL clone presentation. Native SPR pixels retain
// their screen placement; their scene-occlusion proxy is an upright plane
// through the actual actor position, facing the camera in the horizontal plane.
// Screen-aligned constant Z gives the feet the head's depth at pitched views.
// A foreground allowance further exposes the figure through walls and floors.
// Neither is needed here; no state/facing-dependent allowance or scenery depth writes.
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

  std::array<double, 3> worldAt(const CameraBasis& camera, double x, double y) const {
    const double z = depthAt(x, y);
    const double vx = (x - camera.cx) / camera.focalX;
    const double vy = -(y - camera.cy) / camera.focalY;
    const auto& m = camera.worldToView;
    std::array<double, 3> world;
    for (int k = 0; k < 3; ++k)
      world[k] = camera.eyeWorld[k] + z * (m[k * 3] * vx + m[k * 3 + 1] * vy + m[k * 3 + 2]);
    return world;
  }

 private:
  double numerator_, dx_, dy_, constant_;
};

// PROVISIONAL solid footprint of a body texel on the accepted upright plane.
// Installed cylinders have 32 angular sectors, 8 collision columns per sector,
// and horizontal rings. Invert the polygonal template, rather than treating
// its chord as a circular arc. This excludes body pixels occupying a solid
// collision cell even when the plane bulges in front of that cell's curved
// drawn skin. Scenery Z alone cannot reject such pixels (playtest 10 Z1).
inline bool actorTexelInsideTerrain(const CylinderMesh& mesh, const CollisionGrid& grid,
                                   const std::array<double, 3>& p) {
  if (mesh.angularSteps != 32 || mesh.ringCount < 2 || grid.width != 256 ||
      mesh.vertices.size() < mesh.ringCount * 64 || !std::isfinite(p[0]) ||
      !std::isfinite(p[1]) || !std::isfinite(p[2]) || p[1] < 0 || p[1] >= grid.height)
    return false;
  int ring = 0;
  while (ring + 1 < int(mesh.ringCount) - 1 && mesh.vertex(ring + 1, 0, false).y <= p[1]) ++ring;
  const double bottom = mesh.vertex(ring, 0, false).y;
  const double top = mesh.vertex(ring + 1, 0, false).y;
  if (!(top > bottom) || p[1] < bottom || p[1] > top) return false;
  const double v = (p[1] - bottom) / (top - bottom);
  double angle = std::atan2(-p[0], p[2]) / (2 * std::numbers::pi);
  if (angle < 0) angle += 1;
  const int sector = std::min(31, int(angle * 32));
  const auto vertex = [&](int step, bool outer) {
    const auto a = mesh.vertex(ring, step & 31, outer), b = mesh.vertex(ring + 1, step & 31, outer);
    return std::array{std::lerp(a.x,b.x,v), std::lerp(a.z,b.z,v)};
  };
  const auto a = vertex(sector,false), b = vertex(sector+1,false);
  const double determinant = a[0]*b[1] - a[1]*b[0];
  if (std::abs(determinant) < 1e-12) return false;
  const double wa = (p[0]*b[1] - p[2]*b[0]) / determinant;
  const double wb = (a[0]*p[2] - a[1]*p[0]) / determinant;
  if (!(wa + wb > 0)) return false;
  const double u = std::clamp(wb / (wa + wb), 0.0, 1.0);
  const auto c = vertex(sector,true), d = vertex(sector+1,true);
  const double ix = std::lerp(a[0],b[0],u), iz = std::lerp(a[1],b[1],u);
  const double dx = std::lerp(c[0],d[0],u)-ix, dz = std::lerp(c[1],d[1],u)-iz;
  const double thickness2 = dx*dx + dz*dz;
  if (!(thickness2 > 0)) return false;
  const double layer = ((p[0]-ix)*dx + (p[2]-iz)*dz) / thickness2;
  // Match CellMesh's solid/door topology; zone, blocker and fluid flags alone
  // are not opaque terrain (CollisionGrid::solid is a broader legacy query).
  return layer >= 0 && layer <= 1 &&
      (grid.at((sector*8 + int(u*8)) & 255, int(p[1])) & (kCellSolid | kCellFlag0x400)) != 0;
}

}  // namespace webo::app
