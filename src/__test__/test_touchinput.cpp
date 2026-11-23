#include "app/TouchInput.h"

#include <cstdio>
#include <cstdlib>
#include <limits>

using namespace webo::app;

namespace {
using Kind = TouchActionKind;
constexpr TouchId first{1, 7}, second{1, 9}, third{2, 7};

void check(bool ok, const char* message) {
  if (!ok) { std::fprintf(stderr, "FAIL: %s\n", message); std::exit(1); }
}

void tapsAndDrags() {
  TouchInput touch;
  check(touch.down(first, {140, 100}, TouchTarget::Tap, 20).kind == Kind::None,
        "a HUD or menu tap waits for release");
  check(touch.motion(first, {144, 104}).kind == Kind::None, "slop preserves tap");
  const auto tap = touch.up(first, {144, 104}, 100);
  check(tap.kind == Kind::Tap && tap.point.x == 140 && tap.point.y == 100,
        "tap clicks the original touch point");

  touch.down(first, {140, 100}, TouchTarget::Scene, 120);
  const auto drag = touch.motion(first, {150, 102});
  check(drag.kind == Kind::BeginDrag && drag.delta.x == 10 && drag.delta.y == 2,
        "scene slop crossing forwards the full drag displacement");
  const auto move = touch.motion(first, {153, 107});
  check(move.kind == Kind::Drag && move.delta.x == 3 && move.delta.y == 5,
        "drag uses successive positions");
  const auto end = touch.up(first, {154, 107}, 160);
  check(end.kind == Kind::EndDrag && end.delta.x == 1, "drag release carries its last delta and no tap");

  touch.down(first, {50, 50}, TouchTarget::Tap, 200);
  check(touch.motion(first, {90, 50}).kind == Kind::None, "drag on a tap-only target cannot turn the camera");
  check(touch.up(first, {50, 50}, 220).kind == Kind::None, "return through slop cannot revive a tap");
  touch.down(first, {50, 50}, TouchTarget::Scene, 300);
  check(touch.up(first, {50, 50}, 700).kind == Kind::None, "a long scene press cannot assign a skill");
  touch.down(first, {50, 50}, TouchTarget::Tap, 800);
  touch.reset();
  check(touch.up(first, {50, 50}, 810).kind == Kind::None, "focus or screen reset discards a pending tap");
}

void assignmentsAndPrecision() {
  TouchInput touch;
  const auto down = touch.down(first, {140, 100}, TouchTarget::AssignableActor, 100);
  check(down.kind == Kind::Assign && down.point.x == 140 && down.point.y == 100,
        "an eligible actor is assigned at the finger's down point");
  check(touch.down(first, {140, 100}, TouchTarget::AssignableActor, 105).kind == Kind::None,
        "duplicate finger-down cannot assign again");
  check(touch.advance(300).kind == Kind::None, "an assigned touch does not activate precision");
  const auto drag = touch.motion(first, {155, 99});
  check(drag.kind == Kind::BeginDrag && drag.delta.x == 15 && drag.delta.y == -1,
        "the assigned finger can still cross slop and rotate");
  check(touch.up(first, {155, 99}, 320).kind == Kind::EndDrag, "drag release cannot assign again");
  touch.down(first, {140, 100}, TouchTarget::AssignableActor, 350);
  check(touch.up(first, {140, 100}, 360).kind == Kind::None,
        "lifting an assigned finger without a drag has no tap");

  touch.down(first, {140, 100}, TouchTarget::Scene, 400);
  check(touch.up(first, {140, 100}, 450).kind == Kind::None, "empty scene taps cannot assign on lift");
  touch.down(first, {140, 100}, TouchTarget::Scene, 500);
  touch.motion(first, {142, 104});
  check(touch.advance(679).kind == Kind::None, "precision waits for the hold deadline");
  const auto begin = touch.advance(680);
  check(begin.kind == Kind::Precision && begin.point.x == 142 && begin.point.y == 76,
        "the precision cursor starts above the current finger, including motion inside slop");
  const auto edge = touch.motion(first, {-2, 5});
  check(edge.kind == Kind::Precision && edge.point.x == 0 && edge.point.y == 0,
        "precision cursor clamps to the upper and left edges");
  const auto lift = touch.up(first, {648, 530}, 700);
  check(lift.kind == Kind::EndPrecision && lift.point.x == 639 && lift.point.y == 479,
        "precision lift picks at its final clamped, offset position");

  touch.down(first, {140, 100}, TouchTarget::Scene, 800);
  check(touch.advance(980).kind == Kind::Precision, "a new hold activates precision again");
  check(touch.down(second, {300, 100}, TouchTarget::AssignableActor, 990).kind == Kind::CancelPrecision,
        "a second finger cancels precision even over an assignable actor");
  check(touch.up(first, {140, 100}, 995).kind == Kind::None &&
        touch.up(second, {300, 100}, 996).kind == Kind::None,
        "neither precision nor a secondary touch assigns on release after cancellation");

  touch.down(first, {140, 100}, TouchTarget::Scene, 1000);
  check(touch.motion(first, {149, 100}).kind == Kind::BeginDrag &&
        touch.advance(1300).kind == Kind::None,
        "slop crossed before the hold deadline disables precision for this drag");
  touch.up(first, {149, 100}, 1301);
  touch.down(first, {140, 100}, TouchTarget::DragOnly, 1400);
  check(touch.advance(1580).kind == Kind::None &&
        touch.motion(first, {149, 100}).kind == Kind::BeginDrag,
        "a drag-only target keeps the camera gesture after the deadline");
}

void hatchSlides() {
  TouchInput touch;
  using Slide = TouchHatchSlide;
  touch.down(first, {200, 220}, TouchTarget::Hatch, 10);
  check(touch.up(first, {200, 220}, 20).kind == Kind::None, "a quick hatch tap never changes the rate");
  touch.down(first, {200, 220}, TouchTarget::Hatch, 30);
  check(touch.motion(first, {200, 207}).kind == Kind::None &&
        touch.advance(250).kind == Kind::None, "moving off before holding cannot start hatch adjustment");
  touch.up(first, {200, 207}, 260);
  touch.down(first, {200, 220}, TouchTarget::Hatch, 300);
  const auto hold = touch.advance(480);
  check(hold.kind == Kind::HatchSlide && hold.hatchSlide == Slide::Neutral, "holding a hatch starts neutral");
  check(touch.motion(first, {280, 208}).kind == Kind::None, "horizontal and exact-threshold motion is neutral");
  const auto up = touch.motion(first, {280, 207});
  check(up.kind == Kind::HatchSlide && up.hatchSlide == Slide::Up && up.point.x == 200 && up.point.y == 220,
        "upward slide starts a raise at the original hatch point");
  check(touch.motion(first, {80, 180}).kind == Kind::None, "further upward motion preserves the repeat clock");
  const auto neutral = touch.motion(first, {210, 223});
  check(neutral.kind == Kind::HatchSlide && neutral.hatchSlide == Slide::Neutral, "returning inside the threshold stops");
  const auto down = touch.motion(first, {200, 233});
  check(down.kind == Kind::HatchSlide && down.hatchSlide == Slide::Down, "downward slide starts a decrease");
  check(touch.motion(first, {200, 207}).hatchSlide == Slide::Up, "one motion can reverse the slide directly");
  check(touch.up(first, {200, 207}, 520).kind == Kind::EndHatchHold, "lifting stops a hatch slide");

  touch.down(first, {200, 220}, TouchTarget::Hatch, 600);
  touch.advance(780);
  check(touch.down(second, {350, 220}, TouchTarget::Scene, 790).kind == Kind::EndHatchHold,
        "a second finger stops hatch adjustment before starting a pinch");
}

void holds() {
  TouchInput touch;
  constexpr auto start = std::numeric_limits<std::uint32_t>::max() - 10;
  touch.down(first, {50, 350}, TouchTarget::Holdable, start);
  check(touch.advance(start - 1u).kind == Kind::None, "a poll sample before down cannot start a hold");
  check(touch.advance(start + 179u).kind == Kind::None, "hold defers action through the pinch grace period");
  check(touch.advance(start + 180u).kind == Kind::BeginHold, "hold uses elapsed milliseconds across tick wrap");
  check(touch.advance(start + 181u).kind == Kind::None, "hold starts once");
  check(touch.motion(first, {110, 350}).kind == Kind::MoveHold, "hold may drag off the control");
  check(touch.up(first, {110, 350}, start + 200u).kind == Kind::EndHold, "hold release is not a tap");

  const auto quickRelease = 280u;
  touch.down(first, {50, 350}, TouchTarget::Holdable, quickRelease - 179u);
  check(touch.advance(quickRelease).kind == Kind::None &&
        touch.up(first, {50, 350}, quickRelease).kind == Kind::Tap,
        "a holdable control released just before the deadline still taps");

  for (const bool pollWhileHeld : {false, true}) {
    constexpr auto down = 1000u, deadline = down + 180u, release = down + 200u;
    touch.down(first, {50, 350}, TouchTarget::Holdable, down);
    const auto begin = touch.advance(pollWhileHeld ? deadline : release);
    const auto settled = touch.advance(release);
    const auto end = touch.up(first, {50, 350}, release);
    check(begin.kind == Kind::BeginHold && settled.kind == Kind::None && end.kind == Kind::EndHold,
          "the same timed press starts and ends a hold with or without an intervening poll");
  }

  touch.down(first, {50, 350}, TouchTarget::Holdable, 300);
  touch.down(second, {150, 350}, TouchTarget::Scene, 360);
  check(touch.advance(600).kind == Kind::None, "a second finger before hold activation prevents a press");
  check(touch.up(first, {50, 350}, 610).kind == Kind::None, "pinch cannot click the holdable control");
  touch.up(second, {150, 350}, 620);

  touch.down(first, {50, 350}, TouchTarget::Holdable, 800);
  touch.advance(1000);
  check(touch.down(second, {150, 350}, TouchTarget::Scene, 1001).kind == Kind::CancelHold,
        "pinch stops a hold without a click release");
}

void pinches() {
  TouchInput touch;
  touch.down(first, {200, 240}, TouchTarget::Scene, 0);
  touch.motion(first, {212, 240});
  check(touch.down(second, {312, 240}, TouchTarget::Scene, 10).kind == Kind::CancelDrag,
        "pinch cancels an active drag");
  check(touch.motion(second, {331, 240}).kind == Kind::None, "small pinch does not toggle zoom");
  const auto zoom = touch.motion(second, {340, 240});
  check(zoom.kind == Kind::ZoomIn && zoom.point.x == 276 && zoom.point.y == 240,
        "spread grows past threshold and zooms at the current midpoint");
  check(touch.motion(second, {412, 240}).kind == Kind::None, "further spreading cannot retrigger zoom");
  check(touch.motion(second, {390, 240}).kind == Kind::None, "small reversal is ignored");
  check(touch.motion(second, {372, 240}).kind == Kind::ZoomOut, "shrink from peak can reverse zoom");
  check(touch.motion(second, {310, 240}).kind == Kind::None, "further shrinking cannot retrigger zoom");
  check(touch.motion(second, {332, 240}).kind == Kind::ZoomIn, "spread from trough can reverse again");
  check(touch.up(second, {332, 240}, 70).kind == Kind::None, "lifting one finger does not click");
  check(touch.motion(first, {120, 240}).kind == Kind::None, "remaining finger does not start a drag");
  touch.down(second, {250, 240}, TouchTarget::Scene, 80);
  check(touch.motion(second, {350, 240}).kind == Kind::None, "remaining finger cannot start a new pinch");
  touch.up(first, {120, 240}, 90);
  touch.up(second, {350, 240}, 95);

  touch.down(first, {200, 240}, TouchTarget::Scene, 100);
  touch.down(second, {300, 240}, TouchTarget::Scene, 110);
  touch.down(third, {400, 240}, TouchTarget::Scene, 120);
  check(touch.motion(second, {360, 240}).kind == Kind::None, "third contact suppresses pinch");
  touch.up(third, {400, 240}, 130);
  check(touch.motion(second, {390, 240}).kind == Kind::None, "lifting third contact does not resume pinch");
  touch.up(first, {200, 240}, 140);
  touch.up(second, {390, 240}, 150);
  touch.down(first, {200, 240}, TouchTarget::Scene, 160);
  touch.down(second, {300, 240}, TouchTarget::Scene, 170);
  check(touch.motion(second, {275, 240}).kind == Kind::ZoomOut, "fresh pinch can start by shrinking");
}
}  // namespace

int main() {
  tapsAndDrags();
  assignmentsAndPrecision();
  hatchSlides();
  holds();
  pinches();
}
