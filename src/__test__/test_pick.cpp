#include "app/Pick.h"

#include <cstdio>
#include <cstdlib>
#include <vector>
#include <cmath>

using namespace webo::app;

namespace {
void check(bool condition, const char* message) {
  if (!condition) { std::fprintf(stderr, "FAIL: %s\n", message); std::exit(1); }
}
}  // namespace

int main() {
  // Reference candidates have category 0x12, priority 0 and eligibility byte 1.
  // Their original hover result is actor 3.
  {
    const std::array<double, 3> reference{32.0, 48.0, 0.5};
    int actor2 = 2, actor3 = 3;
    const PickCandidate candidates[] = {
        {&actor2, {452, 275, 463, 295}, true, 0,
         pickDistanceSquared({60.53015899658203, 32.0, 0.25}, reference)},
        {&actor3, {434, 286, 445, 305}, true, 0,
         pickDistanceSquared({53.0374641418457, 32.0, 0.75}, reference)}};
    check(std::abs(candidates[0].distanceSquared - 1070.0324723702506) < 1e-9,
          "retained actor2 raw-vector distance");
    check(std::abs(candidates[1].distanceSquared - 698.6373975194438) < 1e-9,
          "retained actor3 raw-vector distance");
    check(pickHoveredLemming(candidates, {432, 274, 464, 306})->id == &actor3,
          "retained native overlap selects actor3, not first actor2");
    check(pickDistanceSquared({1, 2, 3}, {1, 2, 5}) == 4,
          "distance includes third component");
    check(pickDistanceSquared({255, 0, 0}, {1, 0, 0}) == 64516,
          "raw X subtraction does not wrap around cylinder seam");
  }
  // 0x0045578C uses a 32x32 cursor sprite. The lemming hit region shrinks by
  // 4 px horizontally and grows by 4 px vertically.
  const PickBox cursor{90, 90, 150, 150};
  const PickBox hit{110, 110, 120, 120};
  check(pickBoxOverlaps(hit, cursor), "one hit: comfortably inside the adjusted cursor box");

  // 0x00454CB9 uses jg/jl/jg/jl for misses, making each threshold inclusive.
  // Move exactly one cursor edge at a time.

  // left: lemming.left <= cursor.right - 4
  check(pickBoxOverlaps(hit, PickBox{90, 90, /*right=*/114, 150}),
        "left edge: cursor.right-4 == lemming.left is still a hit");
  check(!pickBoxOverlaps(hit, PickBox{90, 90, /*right=*/113, 150}),
        "left edge: one pixel further is a miss");

  // right: lemming.right >= cursor.left + 4
  check(pickBoxOverlaps(hit, PickBox{/*left=*/116, 90, 150, 150}),
        "right edge: cursor.left+4 == lemming.right is still a hit");
  check(!pickBoxOverlaps(hit, PickBox{/*left=*/117, 90, 150, 150}),
        "right edge: one pixel further is a miss");

  // top: lemming.top <= cursor.bottom + 4
  check(pickBoxOverlaps(hit, PickBox{90, 90, 150, /*bottom=*/106}),
        "top edge: cursor.bottom+4 == lemming.top is still a hit");
  check(!pickBoxOverlaps(hit, PickBox{90, 90, 150, /*bottom=*/105}),
        "top edge: one pixel further is a miss");

  // bottom: lemming.bottom >= cursor.top - 4
  check(pickBoxOverlaps(hit, PickBox{90, /*top=*/124, 150, 150}),
        "bottom edge: cursor.top-4 == lemming.bottom is still a hit");
  check(!pickBoxOverlaps(hit, PickBox{90, /*top=*/125, 150, 150}),
        "bottom edge: one pixel further is a miss");

  {
    std::vector<PickCandidate> none;
    check(pickHoveredLemming(none, cursor) == nullptr, "no candidates: nullptr, matching +0x16D8 cleared");

    PickCandidate missed; missed.box = PickBox{0, 0, 10, 10};
    std::vector<PickCandidate> missedList{missed};
    check(pickHoveredLemming(missedList, cursor) == nullptr, "a non-overlapping candidate picks nothing");
  }

  // 0x004550A3-0x00455236 favors higher priority, then strictly smaller
  // distanceSquared. Exact ties preserve the first eligible candidate.
  {
    int a = 1, b = 2;
    PickCandidate closer; closer.id = &a; closer.box = hit; closer.priority = 0; closer.distanceSquared = 10.0;
    PickCandidate farther; farther.id = &b; farther.box = hit; farther.priority = 0; farther.distanceSquared = 20.0;

    std::vector<PickCandidate> closerFirst{closer, farther};
    check(pickHoveredLemming(closerFirst, cursor)->id == &a,
          "equal priority: the closer candidate wins when listed first");

    std::vector<PickCandidate> fartherFirst{farther, closer};
    check(pickHoveredLemming(fartherFirst, cursor)->id == &a,
          "equal priority: the closer candidate wins even when listed second (not a first-wins or last-wins rule)");

    PickCandidate tiedA; tiedA.id = &a; tiedA.box = hit; tiedA.priority = 0; tiedA.distanceSquared = 15.0;
    PickCandidate tiedB; tiedB.id = &b; tiedB.box = hit; tiedB.priority = 0; tiedB.distanceSquared = 15.0;
    std::vector<PickCandidate> exactTie{tiedA, tiedB};
    check(pickHoveredLemming(exactTie, cursor)->id == &a,
          "exact tie on priority and distance keeps the earlier-listed candidate, not the later one "
          "(falsifies 'last overlapping in list wins')");

    PickCandidate lowPriorityClose; lowPriorityClose.id = &a; lowPriorityClose.box = hit;
    lowPriorityClose.priority = 0; lowPriorityClose.distanceSquared = 1.0;
    PickCandidate highPriorityFar; highPriorityFar.id = &b; highPriorityFar.box = hit;
    highPriorityFar.priority = 1; highPriorityFar.distanceSquared = 99.0;
    std::vector<PickCandidate> priorityBeatsDistance{lowPriorityClose, highPriorityFar};
    check(pickHoveredLemming(priorityBeatsDistance, cursor)->id == &b,
          "higher priority wins outright over a much closer, lower-priority candidate");
  }

  // 0x00455104 (visibility byte lemming+0x29) and 0x00455159 (byte lemming+0x370
  // via 0x00455F20) reject candidates before the box test.
  {
    int a = 1, b = 2;
    PickCandidate ineligible; ineligible.id = &a; ineligible.box = hit;
    ineligible.eligible = false; ineligible.priority = 5; ineligible.distanceSquared = 0.0;
    PickCandidate eligible; eligible.id = &b; eligible.box = hit;
    eligible.eligible = true; eligible.priority = 0; eligible.distanceSquared = 50.0;

    std::vector<PickCandidate> withSkip{ineligible, eligible};
    check(pickHoveredLemming(withSkip, cursor)->id == &b,
          "an ineligible candidate is skipped even though its box and priority would otherwise win");

    std::vector<PickCandidate> onlyIneligible{ineligible};
    check(pickHoveredLemming(onlyIneligible, cursor) == nullptr,
          "the only candidate being ineligible picks nothing, matching +0x16D8 cleared");
  }

  std::puts("Pick: all checks passed");
  return 0;
}
