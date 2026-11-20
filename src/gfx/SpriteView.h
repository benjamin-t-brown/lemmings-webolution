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

// Build the scores from 0x00468456..0x004685D9. Normalize cameraToActor (actor minus camera),
// rotate the basis at controller+0x1E0 by phase at controller+0x1D8 plus the native offsets,
// and apply the one-shot score correction. Native sin/cos uses x87. Pass explicit tie scores to selectSpriteView.
std::array<double, 2> spriteViewScores(SpriteViewVector cameraToActor,
                                       SpriteViewVector basis, double phase);

// At 0x0046840B, two signed scores select the entry. Five entries reset and may set the
// mirror bit. Eight entries preserve it. Fewer than two select entry zero and preserve it.
SpriteViewSelection selectSpriteView(double horizontalScore, double verticalScore,
                                     std::size_t entryCount, bool priorMirror = false);

}  // namespace webo::gfx
