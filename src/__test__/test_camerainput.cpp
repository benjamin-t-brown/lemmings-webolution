#include "app/CameraInput.h"

#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <tuple>

using namespace webo::app;

namespace {
void check(bool condition, const char* message) {
  if (!condition) { std::fprintf(stderr, "FAIL: %s\n", message); std::exit(1); }
}
void near(double actual, double expected, const char* message, double eps = 1e-8) {
  check(std::abs(actual - expected) <= eps, message);
}
}

int main() {
  FixedCameraClock clock;
  clock.advance(0.5 * FixedCameraClock::stepSeconds);
  check(!clock.takeStep(), "sub-step elapsed time produces no simulation update");
  clock.advance(0.5 * FixedCameraClock::stepSeconds);
  check(clock.takeStep(), "elapsed partitions combine into one 144 Hz update");
  check(!clock.takeStep(), "one frame partition cannot produce a second update");
  clock.advance(2.0 * FixedCameraClock::stepSeconds);
  check(clock.takeStep() && clock.takeStep(), "catch-up consumes each accumulated fixed update");
  check(!clock.takeStep(), "catch-up leaves no extra update");
  FixedCameraClock grouped, partitioned;
  grouped.advance(4.0 * FixedCameraClock::stepSeconds);
  for (int i = 0; i < 4; ++i) partitioned.advance(FixedCameraClock::stepSeconds);
  int groupedSteps = 0, partitionedSteps = 0;
  while (grouped.takeStep()) ++groupedSteps;
  while (partitioned.takeStep()) ++partitionedSteps;
  check(groupedSteps == 4 && partitionedSteps == 4, "equal elapsed partitions produce equal 144 Hz updates");
  FixedCameraClock oneSecond;
  for (int i = 0; i < 4; ++i) oneSecond.advance(0.25);
  int oneSecondSteps = 0;
  while (oneSecond.takeStep()) ++oneSecondSteps;
  check(oneSecondSteps == 144, "one real second produces 144 fixed updates");

  PendingCameraEvents pending;
  pending.mouse(2, 3); pending.zoom(CameraZoomPose{9.0, 25.0});
  // A render poll below 1/144 s does not consume the queue.
  CameraInput held; held.numpad4 = true;
  const CameraInput consumed = pending.consumeInto(held);
  near(consumed.mouseDx, 2.0, "sub-step mouse event retained until fixed update");
  near(consumed.mouseDy, 3.0, "sub-step mouse y retained until fixed update");
  check(consumed.zoomPressed, "sub-step zoom edge retained until fixed update");
  check(consumed.zoomPose && consumed.zoomPose->lateralOffset == 9.0 &&
        consumed.zoomPose->elevation == 25.0, "pending zoom retains the pose picked at the key press");
  // Catch-up updates consume one-shot events once, then retain only held keys.
  const CameraInput catchup = pending.consumeInto(held);
  check(!catchup.zoomPressed && !catchup.zoomPose && catchup.mouseDx == 0.0,
        "catch-up does not replay consumed one-shot input");
  check(catchup.numpad4, "held key remains active on catch-up update");

  CameraParams p;
  {
    CameraController target(p, 96);
    near(target.pickReference()[0], 32, "target yaw starts at unanimated control yaw");
    near(target.pickReference()[1], 48, "target height starts at half level rows");
    near(target.pickReference()[2], 0.5, "ordinary startup shell coefficient");
    CameraController oddRows(p, 95);
    near(oddRows.pickReference()[1], 47, "startup half-height truncates odd rows");
    CameraInput drag; drag.rightMouseDown = true; drag.mouseDy = -200;
    target.update(drag);
    near(target.pickReference()[1], 96, "target clamps at rows, not rows minus one");
    drag.mouseDy = 10;
    target.update(drag);
    near(target.pickReference()[1], 91, "target reverses immediately after upper saturation");
    near(target.camera().elevation, 0, "target height is not camera eye elevation");
    drag.mouseDy = 400;
    target.update(drag);
    near(target.pickReference()[1], 0, "target lower clamp");
    drag.mouseDy = -2;
    target.update(drag);
    near(target.pickReference()[1], 1, "target reverses immediately after lower saturation");

    CameraParams seamParams = p; seamParams.yawUnits = 255;
    CameraController seam(seamParams);
    CameraInput turn; turn.rightMouseDown = true; turn.numpad4 = true;
    seam.update(turn);
    near(seam.pickReference()[0], 256, "target retains exact256 unlike periodic display yaw");
    turn.numpad4 = false;
    seam.update(turn);
    near(seam.pickReference()[0], 256, "exact256 target survives idle frame");
    turn.numpad4 = true;
    seam.update(turn);
    near(seam.pickReference()[0], 1, "target wraps only above256");
    CameraParams mixedParams = p; mixedParams.yawUnits = 188;
    CameraController mixed(mixedParams);
    CameraInput mixedInput; mixedInput.backspacePressed = true;
    mixedInput.rightMouseDown = true; mixedInput.mouseDx = -144;
    mixed.update(mixedInput, 0.384);
    near(mixed.pickReference()[0], 0,
         "flip target wraps before subsequent grab delta, not one aggregate wrap");

    CameraController zoomTarget(p);
    CameraInput zoom; zoom.zoomPressed = true;
    zoomTarget.update(zoom);
    near(zoomTarget.pickReference()[1], 48, "zoom-entry eye elevation does not move target height");
    zoom.zoomPressed = false; zoom.rightMouseDown = true; zoom.mouseDy = 8;
    zoomTarget.update(zoom);
    near(zoomTarget.pickReference()[1], 47, "zoom1 target height uses one eighth per dy");
    zoom.mouseDy = 0; zoom.mouseDx = 12;
    zoomTarget.update(zoom);
    near(zoomTarget.pickReference()[0], 37, "raw target follows grabbed yaw input");
    CameraParams animated = p; animated.introActive = true; animated.introSeconds = 2;
    CameraController introTarget(animated);
    near(introTarget.pickReference()[0], 32, "intro animation never enters target yaw");
  }
  CameraController c(p);
  CameraInput in;
  in.rightMouseDown = true; in.numpad4 = true;
  c.update(in);
  near(c.camera().yawUnits, 33.0, "NUMPAD4 grabbed yaw +1");
  in.numpad4 = false; in.numpad6 = true; in.leftShift = true;
  c.update(in);
  near(c.camera().yawUnits, 30.0, "LSHIFT multiplies NUMPAD6 by three");
  in.numpad6 = false; in.leftShift = false; in.mouseDx = 12; in.mouseDy = 4;
  c.update(in);
  near(c.camera().yawUnits, 35.0, "positive grabbed mouse dx adds yaw at 10/24");
  near(c.camera().elevation, 0.0, "zoom zero leaves applied elevation unchanged");
  near(c.displayedElevation(), -2.0, "zoom zero displayed elevation -0.5 per dy");

  in.mouseDx = in.mouseDy = 0; in.rightMouseDown = false; in.numpad4 = true;
  c.update(in);
  near(c.cursorX(), 315.2, "free NUMPAD4 moves cursor left by 4.8");
  near(c.camera().yawUnits, 35.0, "free key does not rotate camera");
  in.numpad4 = false; in.mouseDx = 3; in.mouseDy = 5;
  c.update(in);
  near(c.cursorX(), 321.2, "free mouse x uses 2 per dx");
  near(c.cursorY(), 252.0, "free mouse y uses 2.4 per dy");
  in.mouseDx = in.mouseDy = 0; in.numpad6 = true;
  c.update(in);
  near(c.cursorX(), 326.0, "free NUMPAD6 moves cursor right by 4.8");

  in.numpad6 = false; in.zoomPressed = true;
  in.zoomPose = CameraZoomPose{0.0, 20.0};
  c.update(in);
  check(c.camera().zoomShift == 1, "zoom key toggles 0 to 1");
  near(c.camera().elevation, 20.0, "zoom uses the picked eye elevation");
  near(c.camera().yawUnits, 35.0, "zoom keeps the view direction");
  in.zoomPressed = false; in.rightMouseDown = true; in.mouseDy = 2;
  c.update(in);
  near(c.camera().elevation, 23.0, "zoom one applies +1.5 per mouse dy");
  near(c.displayedElevation(), -2.25, "zoom one displayed elevation -0.125 per dy");

  in.mouseDy = 0; in.uiState = 2; in.zoomPressed = true;
  c.update(in);
  check(c.camera().zoomShift == 1, "ui gate blocks zoom");
  near(c.camera().yawUnits, 35.0, "ui gate blocks camera updates");

  CameraController flip(p);
  CameraInput key; key.backspacePressed = true;
  flip.update(key, 0.096);
  const double u = 0.096 / 0.384, a = (2.0 - u) * u;
  near(flip.camera().yawUnits, 32.0 + 128.0 * ((2.0 - a) * a), "flip follows accepted intermediate curve");
  key.backspacePressed = false;
  for (int i = 0; i < 42; ++i) flip.update(key, 1.0 / 144.0);
  near(flip.camera().yawUnits, 160.0, "flip reaches +128 at 0.384 seconds", 1e-7);

  CameraController concurrentFlip(p);
  key.backspacePressed = true; key.rightMouseDown = true; key.numpad4 = true;
  concurrentFlip.update(key, 1.0 / 144.0);
  key.backspacePressed = false;
  for (int i = 0; i < 55; ++i) concurrentFlip.update(key, 1.0 / 144.0);
  near(concurrentFlip.camera().yawUnits, 160.0 + 56.0, "flip preserves concurrent grabbed key yaw", 1e-7);

  CameraController uiOne(p);
  CameraInput uiGate; uiGate.uiState = 1; uiGate.zoomPressed = true;
  uiOne.update(uiGate);
  check(uiOne.camera().zoomShift == 0, "ui state one blocks zoom toggle");
  uiGate.zoomPressed = false; uiGate.backspacePressed = true;
  uiOne.update(uiGate, 0.048);
  check(uiOne.camera().yawUnits > 32.0, "ui state one allows flip update");

  CameraController pausedFlip(p);
  CameraInput pauseGate; pauseGate.backspacePressed = true;
  pausedFlip.update(pauseGate, 0.048);
  const double beforePause = pausedFlip.camera().yawUnits;
  pauseGate.backspacePressed = false; pauseGate.uiState = 2;
  pausedFlip.update(pauseGate, 0.12);
  near(pausedFlip.camera().yawUnits, beforePause, "ui state two freezes in-progress flip");
  pauseGate.uiState = 0;
  pausedFlip.update(pauseGate, 0.048);
  const double pausedU = 0.096 / 0.384, pausedA = (2.0 - pausedU) * pausedU;
  near(pausedFlip.camera().yawUnits, 32.0 + 128.0 * ((2.0 - pausedA) * pausedA),
       "flip resumes from its pre-pause elapsed time");

  CameraController noHit(p);
  CameraInput cursor; cursor.mouseDx = 100; cursor.mouseDy = -100;
  noHit.update(cursor);
  cursor = {}; cursor.zoomPressed = true;
  noHit.update(cursor);
  near(noHit.camera().zoomLateralOffset, 0.0, "zoom without a hit has no lateral offset");
  near(noHit.camera().elevation, 18.5, "zoom without a hit uses neutral elevation");

  CameraController zoomTransitions(p);
  CameraInput zoomInput; zoomInput.mouseDx = 90.0;
  zoomTransitions.update(zoomInput);
  zoomInput.mouseDx = 0.0; zoomInput.zoomPressed = true;
  zoomInput.zoomPose = CameraZoomPose{-12.0, 37.0};
  zoomTransitions.update(zoomInput);
  check(zoomTransitions.camera().zoomShift == 1, "off-center zoom transition enters zoom one");
  near(zoomTransitions.camera().zoomLateralOffset, -12.0,
       "zoom uses the picked lateral offset despite virtual cursor drift");
  near(zoomTransitions.pickReference()[0], 32.0, "zoom translation preserves target yaw");
  near(zoomTransitions.camera().elevation, 37.0, "zoom entry uses the picked elevation");
  zoomInput.zoomPressed = false; zoomInput.mouseDy = 10.0;
  zoomTransitions.update(zoomInput);
  zoomInput.mouseDy = 0.0; zoomInput.zoomPressed = true;
  zoomInput.zoomPose = CameraZoomPose{22.0, 14.0};
  zoomTransitions.update(zoomInput);
  check(zoomTransitions.camera().zoomShift == 0, "off-center zoom transition exits zoom one");
  near(zoomTransitions.camera().yawUnits, 32.0, "zoom exit retains yaw");
  near(zoomTransitions.camera().elevation, 37.0, "zoom exit retains elevation");

  CameraParams edgeParams; edgeParams.yawUnits = 255.5;
  CameraController edges(edgeParams);
  CameraInput edgeInput; edgeInput.rightMouseDown = true; edgeInput.numpad4 = true;
  edges.update(edgeInput);
  near(edges.camera().yawUnits, 0.5, "yaw wraps above one turn");
  edgeInput.numpad4 = false; edgeInput.numpad6 = true;
  edges.update(edgeInput);
  near(edges.camera().yawUnits, 255.5, "yaw wraps below zero");
  edgeInput = {}; edgeInput.mouseDx = -1000; edgeInput.mouseDy = -1000;
  edges.update(edgeInput);
  near(edges.cursorX(), 0.0, "free cursor clamps x to zero");
  near(edges.cursorY(), 0.0, "free cursor clamps y to zero");
  edgeInput.mouseDx = 1000; edgeInput.mouseDy = 1000;
  edges.update(edgeInput);
  near(edges.cursorX(), 639.0, "free cursor clamps x to viewport maximum");
  near(edges.cursorY(), 479.0, "free cursor clamps y to viewport maximum");

  for (const auto [dy, expected] : {std::tuple{1000.0, 47.5}, std::tuple{-1000.0, -10.5}}) {
    CameraController elevationClamp(p);
    CameraInput move; move.zoomPressed = true; elevationClamp.update(move);
    move.zoomPressed = false; move.rightMouseDown = true; move.mouseDy = dy;
    elevationClamp.update(move);
    near(elevationClamp.camera().elevation, expected, "zoom-one mouse elevation clamp");

    CameraController outsideRange(p);
    const double elevation = expected + 0.02 * dy;
    move = {}; move.zoomPressed = true; move.zoomPose = CameraZoomPose{0.0, elevation};
    outsideRange.update(move);
    near(outsideRange.camera().elevation, elevation, "zoom preserves an anchor beyond the drag limits");
    move = {}; move.rightMouseDown = true; move.mouseDx = 10.0;
    outsideRange.update(move);
    near(outsideRange.camera().elevation, elevation, "horizontal drag does not snap anchored elevation");
    move.mouseDx = 0.0; move.mouseDy = -0.004 * dy;
    outsideRange.update(move);
    near(outsideRange.camera().elevation, elevation + 1.5 * move.mouseDy,
         "vertical drag retains its gain from an out-of-range anchor");
  }
  std::puts("CameraInput: all checks passed");
  return 0;
}
