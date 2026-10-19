#pragma once

#include <cstddef>
#include <array>

namespace webo::gfx {

struct SpriteViewSelection {
  std::size_t index = 0;
  bool mirrored = false;
};

struct SpriteViewVector {
  double x = 0.0;
  double y = 0.0;
  double z = 0.0;
};

// Reproduce the score inputs built at 0x00468456..0x004685D9. The helper
// normalizes cameraToActor (actor world position minus camera translation),
// rotates the basis stored at controller+0x1E0 by phase (controller+0x1D8)
// plus the native offsets, and applies the native one-shot score correction.
// The native path evaluates sin/cos through x87;
// this helper uses the platform math library, so exact tie decisions still
// belong to selectSpriteView's explicit score inputs.
std::array<double, 2> spriteViewScores(SpriteViewVector cameraToActor,
                                       SpriteViewVector basis, double phase);

// Select the sprite entry from the two signed orientation scores consumed by
// the original chooser at 0x0046840B. For 5-entry lists the chooser resets
// and may set the mirror bit. For 8-entry lists the original leaves the prior
// mirror bit untouched. Lists shorter than two force entry zero and likewise
// leave that bit untouched.
SpriteViewSelection selectSpriteView(double horizontalScore, double verticalScore,
                                     std::size_t entryCount, bool priorMirror = false);

}  // namespace webo::gfx
