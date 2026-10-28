#pragma once

namespace webo::app {

struct TubePassBounds {
  double centre;
  int a, b, mid, c, d;
};

// READ 43A4EC..43A615. Finite camera selector and level cylinder extent.
// Bounds are shifted together; a can equal32, d can exceed32.
TubePassBounds tubePassBounds(double selectorTurns, double cylinderExtent);

// READ 43A94D..43AA47: native actor+29, given actor+28 and raw +8/+18.
// Z<1 uses inclusive angular buckets [a,d]; Z>=1 bypasses the bucket gate.
// This is submission visibility, NOT a per-pixel terrain/depth occlusion test.
bool actorSubmissionVisible(double terrainX, double shellZ, bool enabled,
                            const TubePassBounds& bounds);

}  // namespace webo::app
