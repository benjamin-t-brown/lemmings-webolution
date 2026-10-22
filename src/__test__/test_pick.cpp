#include "app/Pick.h"

#include <cstdio>
#include <cstdlib>
#include <vector>

using namespace webo::app;

namespace {
void check(bool condition, const char* message) {
  if (!condition) { std::fprintf(stderr, "FAIL: %s\n", message); std::exit(1); }
}
}  // namespace

int main() {
  // ---- pickBoxOverlaps: one hit -------------------------------------------
  // A cursor box with comfortable margin on every side (0x0045578C: the real
  // sprite is 32x32; the exact size does not matter to the formula, only
  // that it is a sane box), against a lemming box well inside the adjusted
  // (x shrunk 4, y grown 4) hit region.
  const PickBox cursor{90, 90, 150, 150};
  const PickBox hit{110, 110, 120, 120};
  check(pickBoxOverlaps(hit, cursor), "one hit: comfortably inside the adjusted cursor box");

  // ---- pickBoxOverlaps: a miss by one pixel on each of the four edges ----
  // 0x00454CB9 compares with <=/>= (jg/jl/jg/jl -> miss), so each edge is
  // inclusive: sitting exactly on the adjusted threshold is still a hit, and
  // only one pixel further is a miss. Each case below holds three of the
  // cursor box's four sides at the comfortable baseline and moves only the
  // one side that bounds the edge under test, so the other three conditions
  // stay far from their own thresholds throughout.

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

  // ---- pickHoveredLemming: no candidates / nothing overlapping -----------
  {
    std::vector<PickCandidate> none;
    check(pickHoveredLemming(none, cursor) == nullptr, "no candidates: nullptr, matching +0x16D8 cleared");

    PickCandidate missed; missed.box = PickBox{0, 0, 10, 10};
    std::vector<PickCandidate> missedList{missed};
    check(pickHoveredLemming(missedList, cursor) == nullptr, "a non-overlapping candidate picks nothing");
  }

  // ---- pickHoveredLemming: two overlapping lemmings, tie-break ------------
  // 0x004550A3-0x00455236: priority (desc) then distanceSquared (asc,
  // strict) decide among overlapping, eligible candidates. This is read as
  // NOT "last overlapping lemming in list order wins" -- the brief's own
  // unverified hypothesis -- which the exact-tie case below falsifies.
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

  // ---- pickHoveredLemming: a skipped (ineligible) lemming -----------------
  // 0x00455104 (byte lemming+0x29, the hatch-emergence flag) and 0x00455159
  // (byte lemming+0x370 via 0x00455F20) both skip a candidate outright
  // before the box test; folded here into PickCandidate::eligible.
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
