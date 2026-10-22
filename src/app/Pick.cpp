#include "app/Pick.h"

namespace webo::app {

bool pickBoxOverlaps(const PickBox& box, const PickBox& cursorBox) {
  // 0x00454CB9, re-read this round against the live image (see reads.txt):
  //   lemming.left   <= cursor.right  - 4   (0x00454CD3/0x00454CD6, jg->miss)
  //   lemming.right  >= cursor.left   + 4   (0x00454CE9/0x00454CEC, jl->miss)
  //   lemming.top    <= cursor.bottom + 4   (0x00454CFF/0x00454D02, jg->miss)
  //   lemming.bottom >= cursor.top    - 4   (0x00454D15/0x00454D18, jl->miss)
  // All four compares are inclusive: touching the adjusted edge is a hit.
  return box.left <= cursorBox.right - 4 &&
         box.right >= cursorBox.left + 4 &&
         box.top <= cursorBox.bottom + 4 &&
         box.bottom >= cursorBox.top - 4;
}

const PickCandidate* pickHoveredLemming(std::span<const PickCandidate> candidates,
                                         const PickBox& cursorBox) {
  const PickCandidate* best = nullptr;
  // Sentinels read at 0x00454D7D ([ebp-4] = 0xFFFFFFFF, i.e. -1) and
  // 0x00455069 ([ebp-0x58] qword = 100000000.0): any real priority beats -1
  // outright, and any real squared distance beats 1e8 once priorities tie.
  int bestPriority = -1;
  double bestDistanceSquared = 100000000.0;

  for (const PickCandidate& c : candidates) {
    // 0x00455104 (byte lemming+0x29) and 0x00455159 (byte lemming+0x370 via
    // 0x00455F20), folded into one flag -- both are plain skip-on-zero
    // tests with no further interaction with anything else in the routine.
    if (!c.eligible) continue;
    // 0x00455112/0x00455115: jl skips a lower-priority candidate before the
    // box test even runs. Order does not change the result here, since
    // every gate below is an independent necessary condition.
    if (c.priority < bestPriority) continue;
    if (!pickBoxOverlaps(c.box, cursorBox)) continue;
    if (c.priority == bestPriority) {
      // 0x004551F6-0x004551FE: fcomp + test ah,1 reads the FPU's C0 flag,
      // i.e. "new distance < stored distance", strictly. An exact tie on
      // both keys keeps the earlier-encountered candidate.
      if (!(c.distanceSquared < bestDistanceSquared)) continue;
    }
    // c.priority > bestPriority (0x004551F4 jg) always wins outright here,
    // bypassing the distance compare entirely, exactly as read.
    best = &c;
    bestPriority = c.priority;
    bestDistanceSquared = c.distanceSquared;
  }
  return best;
}

}  // namespace webo::app
