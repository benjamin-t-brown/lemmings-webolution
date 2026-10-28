#include "app/CameraIntro.h"

#include <array>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <limits>

using namespace webo::app;

namespace {
void check(bool ok, const char* message) {
  if (!ok) { std::fprintf(stderr, "FAIL: %s\n", message); std::exit(1); }
}
void near(double actual, double expected, const char* message, double eps = 1e-8) {
  check(std::abs(actual - expected) <= eps, message);
}
}

int main() {
  CameraParams base;
  base.yawUnits = 32.0;
  base.levelEyeHeight = 96.0;
  CameraIntro intro(true);
  const std::array<double, 7> times{0, .5, 1, 1.5, 2, 2.5, 3};
  const std::array<double, 7> distances{727.516, 574.391, 465.016, 399.391,
                                        377.516, 377.516, 377.516};
  const std::array<double, 7> yaws{32, 32, 32, 64, 160, 256, 288};
  for (std::size_t i = 0; i < times.size(); ++i) {
    intro.reset(true);
    intro.advanceAfterFrame(times[i], false);
    const CameraParams pose = intro.pose(base);
    check(pose.introActive, "state clock advance does not deactivate until >3");
    CameraBasis b = buildCamera(pose);
    const double distance = std::sqrt(b.eyeWorld[0] * b.eyeWorld[0] + b.eyeWorld[2] * b.eyeWorld[2]);
    near(distance, distances[i], "intro distance equation");
    // selector is the normalized pose phase, retaining strict >1 wrap semantics.
    double phase = (yaws[i] + 128.0) / 256.0;
    while (phase > 1.0) phase -= 1.0;
    near(b.selectorTurns, phase, "pose phase and strict wrap");
    near(b.eyeWorld[1], 113.392, "zoom-zero eye height is unchanged");
  }

  CameraIntro disabled;
  const CameraBasis canonical = buildCamera(base);
  const CameraBasis unchanged = buildCamera(disabled.pose(base));
  check(canonical.worldToView == unchanged.worldToView &&
        canonical.projectionVector == unchanged.projectionVector &&
        canonical.inverseProjectionVector == unchanged.inverseProjectionVector &&
        canonical.eyeWorld == unchanged.eyeWorld &&
        canonical.selectorTurns == unchanged.selectorTurns,
        "disabled intro basis is exactly canonical");
  check(!disabled.pose(base).introActive && disabled.pose(base).introSeconds == 0.0,
        "disabled pose carries neutral intro fields");

  CameraParams zoomed = base;
  zoomed.zoomShift = 1;
  zoomed.elevation = 17.0;
  intro.reset(true);
  const CameraBasis zoomPose = buildCamera(intro.pose(zoomed));
  const CameraBasis zoomBase = buildCamera(zoomed);
  near(std::sqrt(zoomPose.eyeWorld[0] * zoomPose.eyeWorld[0] + zoomPose.eyeWorld[2] * zoomPose.eyeWorld[2]) -
       std::sqrt(zoomBase.eyeWorld[0] * zoomBase.eyeWorld[0] + zoomBase.eyeWorld[2] * zoomBase.eyeWorld[2]),
       350.0, "intro extra distance is not zoom-scaled");
  near(zoomPose.eyeWorld[1], (96.0 + 17.392) / 2.0 + 17.0,
       "zoom elevation is added only to elevated eye height");
  check(base.yawUnits == 32.0 && base.zoomShift == 0 && base.elevation == 0.0,
        "pose generation does not mutate controller inputs");

  intro.reset(true);
  const CameraParams beforeAdvance = intro.pose(base);
  const auto beforeBasis = buildCamera(beforeAdvance);
  intro.advanceAfterFrame(0.5, false);
  check(beforeAdvance.introSeconds == 0.0 && intro.seconds() == 0.5 &&
        buildCamera(beforeAdvance).worldToView == beforeBasis.worldToView,
        "frame pose snapshot precedes post-draw clock advance");
  intro.reset(true);
  intro.advanceAfterFrame(0.5, true);
  check(intro.seconds() == 0.0 && buildCamera(intro.pose(base)).worldToView == beforeBasis.worldToView,
        "pause freezes intro clock and current-frame pose");
  intro.advanceAfterFrame(-1.0, false);
  intro.advanceAfterFrame(std::numeric_limits<double>::infinity(), false);
  intro.advanceAfterFrame(std::numeric_limits<double>::quiet_NaN(), false);
  check(intro.seconds() == 0.0, "negative and non-finite elapsed times are ignored");
  intro.advanceAfterFrame(3.0, false);
  check(intro.active() && intro.seconds() == 3.0, "intro remains active exactly at three seconds");
  const auto t3 = buildCamera(intro.pose(base));
  intro.advanceAfterFrame(0.001, false);
  check(!intro.active() && intro.seconds() > 3.0, "intro disables only after three seconds");
  check(buildCamera(intro.pose(base)).worldToView == canonical.worldToView,
        "disabled post-frame pose returns to canonical view");
  check(t3.worldToView == canonical.worldToView, "full revolution at t=3 returns to base pose");
  check(beforeBasis.worldToView != canonical.worldToView, "intro at t=0 is a positive control");
  auto boundary = base;
  boundary.yawUnits = 128.0;
  boundary.introActive = true;
  near(buildCamera(boundary).selectorTurns, 1.0, "phase exactly one is not wrapped to zero");
  intro.reset(true);
  check(intro.active() && intro.seconds() == 0.0, "reset re-arms intro at zero");
  std::puts("CameraIntro: all checks passed");
  return 0;
}
