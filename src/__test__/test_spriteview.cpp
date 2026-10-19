#include "gfx/SpriteView.h"

#include <array>
#include <cstddef>
#include <cstdio>

using webo::gfx::SpriteViewSelection;
using webo::gfx::selectSpriteView;

int main() {
  const auto axialScores = webo::gfx::spriteViewScores({0, 0, 5}, {1, 0, 0}, 0);
  if (axialScores[0] < 0.38268344 || axialScores[0] > 0.38268345 ||
      axialScores[1] < 0.9999999 || axialScores[1] > 1.0000001) {
    std::fprintf(stderr, "source score producer mismatch: (%g,%g)\n",
                 axialScores[0], axialScores[1]);
    return 1;
  }
  const auto correctionScores = webo::gfx::spriteViewScores({4, 0, 0}, {3, 0, 0}, 0);
  if (correctionScores[0] < -1.0 || correctionScores[0] > 1.0 ||
      correctionScores[1] < -1.0 || correctionScores[1] > 1.0) {
    std::fprintf(stderr, "native +/-2 score correction mismatch\n");
    return 1;
  }
  const auto oneCorrection = webo::gfx::spriteViewScores({1, 0, 0}, {4, 0, 0}, -0.0625);
  if (oneCorrection[0] != 2.0) {
    std::fprintf(stderr, "source score must apply exactly one +/-2 correction, got %g\n",
                 oneCorrection[0]);
    return 1;
  }
  const auto negativeCorrection =
      webo::gfx::spriteViewScores({-1, 0, 0}, {4, 0, 0}, -0.0625);
  if (negativeCorrection[0] != -2.0) {
    std::fprintf(stderr, "source score must add exactly one +2 correction, got %g\n",
                 negativeCorrection[0]);
    return 1;
  }
  const auto zeroVector = webo::gfx::spriteViewScores({0, 0, 0}, {1, 0, 0}, 0);
  if (zeroVector[0] != 0.0 || zeroVector[1] != 0.0) {
    std::fprintf(stderr, "zero camera vector must produce zero dot scores\n");
    return 1;
  }

  struct Case {
    double x;
    double y;
    std::size_t count;
    std::size_t index;
    bool priorMirror;
    bool mirror;
  };
  // Independent quadrant and diagonal examples, with all diagonal ties fixed
  // to the <= branches visible in the original chooser.
  constexpr std::array cases{
      Case{0, 0, 5, 2, true, false}, Case{0, 1, 5, 3, false, true},
      Case{1, 0, 5, 0, true, false}, Case{-1, 0, 5, 3, true, false},
      Case{0, -1, 5, 2, true, false}, Case{1, 1, 5, 2, false, true},
      Case{-1, 1, 5, 3, false, true}, Case{-1, -1, 5, 2, true, false},
      Case{1, -1, 5, 1, true, false}, Case{2, 1, 5, 1, false, true},
      Case{1, 1.01, 5, 2, false, true}, Case{1.01, 1, 5, 1, false, true},
      Case{-1, 1.01, 5, 3, false, true}, Case{-1.01, 1, 5, 4, true, false},
      Case{-2, 1, 5, 4, true, false}, Case{1, 2, 8, 6, true, true},
      Case{2, 1, 8, 7, false, false}, Case{1, 1.01, 8, 6, true, true},
      Case{1.01, 1, 8, 7, false, false}, Case{-1, 1.01, 8, 5, true, true},
      Case{-1.01, 1, 8, 4, false, false}, Case{-1, 2, 8, 5, true, true},
      Case{-2, 1, 8, 4, false, false}, Case{2, -1, 8, 0, true, true},
      Case{1, -2, 8, 1, false, false}, Case{-1, -2, 8, 2, true, true},
      Case{-2, -1, 8, 3, false, false}, Case{0, 0, 8, 2, true, true},
      Case{0, 1, 8, 5, false, false}, Case{0, -1, 8, 2, true, true},
      Case{1, 0, 8, 0, false, false}, Case{-1, 0, 8, 3, true, true},
      Case{1, 1, 8, 6, false, false}, Case{-1, 1, 8, 5, true, true},
      Case{-1, -1, 8, 2, false, false}, Case{1, -1, 8, 1, true, true},
      Case{1, 2, 1, 0, true, true}, Case{-1, -1, 0, 0, false, false}, Case{1, 1, 7, 6, false, false},
  };
  for (const auto& c : cases) {
    const SpriteViewSelection actual =
        selectSpriteView(c.x, c.y, c.count, c.priorMirror);
    if (actual.index != c.index || actual.mirrored != c.mirror) {
      std::fprintf(stderr, "case (%g,%g,count=%zu,prior=%d): got (%zu,%d), expected (%zu,%d)\n",
                   c.x, c.y, c.count, c.priorMirror, actual.index, actual.mirrored,
                   c.index, c.mirror);
      return 1;
    }
  }

  // Five-view cases explicitly overwrite the incoming mirror state.
  if (selectSpriteView(1, 0, 5, true).mirrored ||
      !selectSpriteView(1, 2, 5, false).mirrored)
    return 1;
}
