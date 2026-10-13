// The camera that looks at the tube.
//
// Lemmings Revolution is played on the outside of a vertical cylinder, so the
// natural camera orbits it: an angle around the axis, a height, and a
// distance. This builds the view-projection matrix for that, and nothing else
// -- no GL, no SDL, so it can be tested on its own.
//
// The cylinder's own numbers come from the level file and are the same in all
// 102 retail levels (see assets/Cylinder.h): 32 angular steps, 7 rings 16
// apart, outer radius 60. The defaults here match that, so the camera frames a
// real level without being told about one.
//
// Conventions, chosen to match the simulation rather than any graphics habit:
//   * The simulation's X wraps; it maps to the angle around the tube, with
//     x = 0 at angle 0 and x = worldWidth back at angle 0 again.
//   * The simulation's Y grows *downward*, as it does in a 2D terrain mask.
//     The world's Y therefore points down here too, which keeps the renderer
//     free of sign flips and makes "lemming at y+1 is lower" true everywhere.
//   * Angles are radians, measured so that increasing the simulation's x
//     moves a lemming in the direction the camera sees as rightward at
//     angle 0.

#pragma once

#include <array>

namespace webo::gfx {

// Column-major 4x4, as GL wants it.
using Mat4 = std::array<float, 16>;

Mat4 identity();
Mat4 multiply(const Mat4& a, const Mat4& b);

// Right-handed perspective, mapping depth to [-1, 1].
Mat4 perspective(float fovYRadians, float aspect, float zNear, float zFar);

// Right-handed look-at. `up` is +Y in this file's downward-Y world, so the
// caller does not pass one: the view is always upright around the tube's axis.
Mat4 lookAt(float eyeX, float eyeY, float eyeZ, float atX, float atY, float atZ);

struct CylinderView {
  // Geometry of the tube, defaulted to the retail values.
  float radius = 60.0f;      // the outer skin
  float height = 96.0f;      // 6 bands of 16

  // Where the camera is.
  float angle = 0.0f;        // radians around the axis
  float distance = 150.0f;   // from the axis, so it must exceed `radius`
  float eyeHeight = 48.0f;   // world Y, downward, so this is mid-tube
  float lookHeight = 48.0f;  // world Y the camera aims at

  float fovY = 0.9f;         // radians
  float zNear = 1.0f;
  float zFar = 2000.0f;

  // Keeps the camera outside the tube and the clip planes sane, so a bad
  // level or a wild control cannot put the view inside the geometry.
  void clamp();

  Mat4 viewProjection(int viewportWidth, int viewportHeight) const;

  // Where the camera sits, for callers that need it (depth sorting, say).
  void eye(float& x, float& y, float& z) const;

  // The simulation's X that is currently centred in view. Useful for keeping
  // a flat-view camera and a cylinder-view camera in step.
  float centredWorldX(float worldWidth) const;

  // Points the camera at a given simulation X.
  void centreOnWorldX(float worldX, float worldWidth);
};

}  // namespace webo::gfx
