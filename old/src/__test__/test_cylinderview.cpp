// Tests for the cylinder camera.
//
// This is the one piece of the renderer that is pure arithmetic, so it is the
// one piece that can be checked without a GL context -- and it is worth
// checking, because a sign error in a view matrix shows up as a picture that
// looks almost right. The assertions below are the properties that fail
// loudly when a sign is wrong: the eye maps to the view-space origin, a
// greater world Y is lower on screen, and the tube's near side is nearer than
// its far side.

#include "gfx/CylinderView.h"

#include <cmath>
#include <format>
#include <iostream>
#include <numbers>
#include <string>
#include <string_view>

namespace {

int g_failures = 0;
int g_checks = 0;

void check(bool cond, std::string_view what) {
  ++g_checks;
  if (!cond) {
    ++g_failures;
    std::cerr << "  FAIL  " << what << "\n";
  }
}

using namespace webo::gfx;

constexpr float kPi = std::numbers::pi_v<float>;

bool close(float a, float b, float tol = 1e-4f) { return std::abs(a - b) < tol; }

// A point transformed by a column-major 4x4, before the perspective divide.
struct Projected {
  float x, y, z, w;
  bool behind() const { return w <= 0; }
  float ndcX() const { return x / w; }
  float ndcY() const { return y / w; }
  float ndcZ() const { return z / w; }
};

Projected apply(const Mat4& m, float px, float py, float pz) {
  Projected out{};
  out.x = m[0] * px + m[4] * py + m[8] * pz + m[12];
  out.y = m[1] * px + m[5] * py + m[9] * pz + m[13];
  out.z = m[2] * px + m[6] * py + m[10] * pz + m[14];
  out.w = m[3] * px + m[7] * py + m[11] * pz + m[15];
  return out;
}

void testMatrixBasics() {
  std::cout << "matrix basics\n";
  const Mat4 i = identity();
  const Projected p = apply(i, 3, -4, 5);
  check(close(p.x, 3) && close(p.y, -4) && close(p.z, 5) && close(p.w, 1),
        "identity leaves a point alone");
  check(multiply(i, i) == i, "identity times identity is identity");
}

void testLookAt() {
  std::cout << "look-at\n";
  const Mat4 v = lookAt(10, 20, 30, 0, 20, 0);

  const Projected e = apply(v, 10, 20, 30);
  check(close(e.x, 0) && close(e.y, 0) && close(e.z, 0), "the eye maps to the view-space origin");

  // The target sits straight ahead: centred, and in front of the camera,
  // which is -z in a right-handed view space.
  const Projected at = apply(v, 0, 20, 0);
  check(close(at.x, 0) && close(at.y, 0), "the target is centred");
  check(at.z < 0, "and lies in front of the camera");

  // World Y points down, so a point with a *greater* world Y must appear
  // lower. This is the assertion that catches an inverted up vector, which
  // otherwise just renders every level upside down.
  const Projected lower = apply(v, 0, 40, 0);
  check(lower.y < at.y, "a greater world Y is lower on screen");
}

void testPerspective() {
  std::cout << "perspective\n";
  const Mat4 p = perspective(1.0f, 16.0f / 9.0f, 1.0f, 100.0f);

  const Projected centre = apply(p, 0, 0, -10);
  check(!centre.behind(), "a point in front has positive w");
  check(close(centre.ndcX(), 0) && close(centre.ndcY(), 0), "it projects to the centre");

  // Nearer is nearer in depth, which is what the depth test relies on.
  const Projected nearPoint = apply(p, 0, 0, -5);
  const Projected farPoint = apply(p, 0, 0, -50);
  check(nearPoint.ndcZ() < farPoint.ndcZ(), "a nearer point has a smaller depth");

  // A wide aspect compresses x, not y.
  const Projected offX = apply(p, 1, 0, -10);
  const Projected offY = apply(p, 0, 1, -10);
  check(std::abs(offX.ndcX()) < std::abs(offY.ndcY()), "the aspect widens x, not y");
}

void testClamp() {
  std::cout << "clamping\n";
  CylinderView v;
  v.radius = 60;
  v.distance = 1;  // inside the tube
  v.clamp();
  check(v.distance > v.radius, "the camera is pushed back outside the tube");

  v.angle = 3.0f * 2.0f * kPi + 0.25f;
  v.clamp();
  check(v.angle >= 0 && v.angle < 2.0f * kPi + 1e-4f, "the angle wraps into a single turn");
  check(close(v.angle, 0.25f, 1e-3f), "and keeps its bearing");

  v.angle = -0.5f;
  v.clamp();
  check(v.angle > 0, "a negative angle wraps forwards");

  v.fovY = 99;
  v.clamp();
  check(v.fovY < 2.7f, "an absurd field of view is reined in");
}

void testWorldXMapping() {
  std::cout << "world x to angle\n";
  CylinderView v;
  v.radius = 60;

  v.centreOnWorldX(0, 320);
  check(close(v.centredWorldX(320), 0, 1e-2f), "x = 0 round-trips");

  v.centreOnWorldX(80, 320);
  check(close(v.angle, kPi * 0.5f, 1e-3f), "a quarter of the way round is a quarter turn");
  check(close(v.centredWorldX(320), 80, 1e-2f), "and round-trips");

  // A full lap returns to the start rather than drifting off.
  v.centreOnWorldX(320, 320);
  const float lap = v.centredWorldX(320);
  check(close(lap, 0, 1e-2f) || close(lap, 320, 1e-2f), "a full lap returns to the start");

  // A zero world width must not divide by zero.
  CylinderView z;
  z.centreOnWorldX(10, 0);
  check(z.centredWorldX(0) == 0, "a zero world width is harmless");
}

void testViewProjection() {
  std::cout << "the whole view-projection\n";
  CylinderView v;
  v.radius = 60;
  v.height = 96;
  v.angle = 0;
  v.distance = 300;
  v.eyeHeight = 48;
  v.lookHeight = 48;
  v.clamp();

  const Mat4 vp = v.viewProjection(1280, 800);

  // At angle 0 the camera sits on +z, so the tube's near surface is at
  // z = +radius and its far surface at z = -radius.
  const Projected nearSide = apply(vp, 0, 48, v.radius);
  const Projected farSide = apply(vp, 0, 48, -v.radius);
  check(!nearSide.behind() && !farSide.behind(), "both sides are in front of the camera");
  check(nearSide.ndcZ() < farSide.ndcZ(), "the near side of the tube is nearer");
  check(close(nearSide.ndcX(), 0, 1e-3f), "the near surface is centred horizontally");

  // The whole tube fits at this distance, the right way up.
  const Projected top = apply(vp, 0, 0, v.radius);
  const Projected bottom = apply(vp, 0, v.height, v.radius);
  check(std::abs(top.ndcY()) < 1.0f && std::abs(bottom.ndcY()) < 1.0f,
        "the tube's full height fits in the frame");
  check(top.ndcY() > bottom.ndcY(), "the top of the tube is above its bottom");

  // A quarter turn brings what was at the side round to the front.
  CylinderView turned = v;
  turned.angle = kPi * 0.5f;
  const Mat4 vp2 = turned.viewProjection(1280, 800);
  const Projected wasSide = apply(vp2, v.radius, 48, 0);
  check(close(wasSide.ndcX(), 0, 1e-3f), "a quarter turn centres what was at the side");
}

}  // namespace

int main() {
  testMatrixBasics();
  testLookAt();
  testPerspective();
  testClamp();
  testWorldXMapping();
  testViewProjection();

  std::cout << std::format("\n{} checks, {} failures\n", g_checks, g_failures);
  return g_failures == 0 ? 0 : 1;
}
