#include "app/Pick.h"

namespace webo::app {

double pickDistanceSquared(const std::array<double, 3>& position,
                           const std::array<double, 3>& reference) {
  const double x = position[0] - reference[0];
  const double y = position[1] - reference[1];
  const double z = position[2] - reference[2];
  return x * x + y * y + z * z;
}

bool pickBoxOverlaps(const PickBox& box, const PickBox& cursorBox) {
  // 0x00454CB9 adjusts the cursor box before testing overlap.
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
  // Native sentinels are -1 ([ebp-4] = 0xFFFFFFFF at 0x00454D7D) and
  // 1e8 ([ebp-0x58] qword at 0x00455069).
  int bestPriority = -1;
  double bestDistanceSquared = 100000000.0;

  for (const PickCandidate& c : candidates) {
    // Skip zero at lemming+0x29 (0x00455104) or lemming+0x370
    // (0x00455159 via 0x00455F20).
    if (!c.eligible) continue;
    // 0x00455112/0x00455115 skips lower priority before overlap.
    if (c.priority < bestPriority) continue;
    if (!pickBoxOverlaps(c.box, cursorBox)) continue;
    if (c.priority == bestPriority) {
      // 0x004551F6-0x004551FE uses fcomp + test ah,1 (FPU C0).
      // Only a strictly smaller distance replaces the incumbent at equal priority.
      if (!(c.distanceSquared < bestDistanceSquared)) continue;
    }
    // Higher priority bypasses distance (0x004551F4 jg).
    best = &c;
    bestPriority = c.priority;
    bestDistanceSquared = c.distanceSquared;
  }
  return best;
}

}  // namespace webo::app
