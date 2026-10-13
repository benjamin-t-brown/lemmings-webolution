#include "CylinderView.h"

#include <cmath>
#include <numbers>

namespace webo::gfx {
namespace {

constexpr float kTwoPi = 2.0f * std::numbers::pi_v<float>;

// Column-major index: column c, row r.
constexpr int at(int c, int r) { return c * 4 + r; }

}  // namespace

Mat4 identity() {
  Mat4 m{};
  m[at(0, 0)] = m[at(1, 1)] = m[at(2, 2)] = m[at(3, 3)] = 1.0f;
  return m;
}

Mat4 multiply(const Mat4& a, const Mat4& b) {
  Mat4 out{};
  for (int c = 0; c < 4; ++c)
    for (int r = 0; r < 4; ++r) {
      float sum = 0;
      for (int k = 0; k < 4; ++k)
        sum += a[at(k, r)] * b[at(c, k)];
      out[at(c, r)] = sum;
    }
  return out;
}

Mat4 perspective(float fovYRadians, float aspect, float zNear, float zFar) {
  Mat4 m{};
  const float f = 1.0f / std::tan(fovYRadians * 0.5f);
  m[at(0, 0)] = f / (aspect > 0 ? aspect : 1.0f);
  m[at(1, 1)] = f;
  m[at(2, 2)] = (zFar + zNear) / (zNear - zFar);
  m[at(2, 3)] = -1.0f;
  m[at(3, 2)] = (2.0f * zFar * zNear) / (zNear - zFar);
  return m;
}

Mat4 lookAt(float eyeX, float eyeY, float eyeZ, float atX, float atY, float atZ) {
  // Forward, from the eye towards the target.
  float fx = atX - eyeX, fy = atY - eyeY, fz = atZ - eyeZ;
  const float flen = std::sqrt(fx * fx + fy * fy + fz * fz);
  if (flen > 0) {
    fx /= flen;
    fy /= flen;
    fz /= flen;
  }

  // The world's Y points down, so "up" for the camera is -Y. Taking it as a
  // constant is right here: the camera orbits the tube's axis and never rolls,
  // so there is no case where up should be anything else.
  const float ux = 0.0f, uy = -1.0f, uz = 0.0f;

  // side = forward x up
  float sx = fy * uz - fz * uy;
  float sy = fz * ux - fx * uz;
  float sz = fx * uy - fy * ux;
  const float slen = std::sqrt(sx * sx + sy * sy + sz * sz);
  if (slen > 0) {
    sx /= slen;
    sy /= slen;
    sz /= slen;
  }

  // The orthogonalised up: side x forward.
  const float vx = sy * fz - sz * fy;
  const float vy = sz * fx - sx * fz;
  const float vz = sx * fy - sy * fx;

  Mat4 m = identity();
  m[at(0, 0)] = sx;  m[at(1, 0)] = sy;  m[at(2, 0)] = sz;
  m[at(0, 1)] = vx;  m[at(1, 1)] = vy;  m[at(2, 1)] = vz;
  m[at(0, 2)] = -fx; m[at(1, 2)] = -fy; m[at(2, 2)] = -fz;
  m[at(3, 0)] = -(sx * eyeX + sy * eyeY + sz * eyeZ);
  m[at(3, 1)] = -(vx * eyeX + vy * eyeY + vz * eyeZ);
  m[at(3, 2)] = fx * eyeX + fy * eyeY + fz * eyeZ;
  return m;
}

void CylinderView::clamp() {
  if (radius < 1.0f)
    radius = 1.0f;
  if (height < 1.0f)
    height = 1.0f;
  // Outside the tube, with enough margin that the near plane does not slice
  // into the surface.
  const float minimum = radius + zNear + 1.0f;
  if (distance < minimum)
    distance = minimum;
  if (distance > zFar * 0.5f)
    distance = zFar * 0.5f;
  if (fovY < 0.2f)
    fovY = 0.2f;
  if (fovY > 2.6f)
    fovY = 2.6f;
  // Keep the angle in [0, 2pi) so it stays readable and never drifts far
  // enough to lose precision.
  angle = std::fmod(angle, kTwoPi);
  if (angle < 0)
    angle += kTwoPi;
}

void CylinderView::eye(float& x, float& y, float& z) const {
  x = std::sin(angle) * distance;
  y = eyeHeight;
  z = std::cos(angle) * distance;
}

Mat4 CylinderView::viewProjection(int viewportWidth, int viewportHeight) const {
  const float aspect = viewportHeight > 0 ? static_cast<float>(viewportWidth) /
                                                static_cast<float>(viewportHeight)
                                          : 1.0f;
  float ex = 0, ey = 0, ez = 0;
  eye(ex, ey, ez);
  const Mat4 proj = perspective(fovY, aspect, zNear, zFar);
  const Mat4 view = lookAt(ex, ey, ez, 0.0f, lookHeight, 0.0f);
  return multiply(proj, view);
}

float CylinderView::centredWorldX(float worldWidth) const {
  if (worldWidth <= 0)
    return 0;
  return angle / kTwoPi * worldWidth;
}

void CylinderView::centreOnWorldX(float worldX, float worldWidth) {
  if (worldWidth <= 0)
    return;
  angle = worldX / worldWidth * kTwoPi;
  clamp();
}

}  // namespace webo::gfx
