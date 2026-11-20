#pragma once

namespace webo::app {

struct TubePassBounds {
  double centre;
  int a, b, mid, c, d;
};

// 43A4EC..43A615 shifts the bucket bounds together. With finite inputs,
// a can equal 32 and d can exceed 32.
TubePassBounds tubePassBounds(double selectorTurns, double cylinderExtent);

// Submission gate at 43A94D..43AA47 writes actor+29 from actor+28 and raw +8/+18.
// Z < 1 uses inclusive angular buckets [a,d]. Z >= 1 bypasses the bucket gate.
bool actorSubmissionVisible(double terrainX, double shellZ, bool enabled,
                            const TubePassBounds& bounds);

}  // namespace webo::app
