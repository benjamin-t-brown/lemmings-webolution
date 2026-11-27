#pragma once

#include "app/Pick.h"
#include "app/WindowScale.h"
#include "assets/Bitmap.h"
#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <span>
#include <utility>

namespace webo::app {

// The level cursor uses a half-open rectangle of visible scene pixels.
inline bool pointerOverSceneBounds(int x, int y, int left, int top, int right, int bottom) {
  return x >= left && x < right && y >= top && y < bottom;
}

inline void addHoverRgb(std::uint8_t* destination, const std::uint8_t* source) {
  for (int c = 0; c < 3; ++c)
    destination[c] = std::min(255, destination[c] + source[c]);
}

// Displayed hatch geometry, in framebuffer pixels; index is the serialized
// hatch order also used by World::setup().entrances. No simulation state.
struct HatchHoverTarget {
  std::size_t index = 0;
  int left = 0, top = 0, right = 0, bottom = 0;
  int remaining = 0;
};

int hatchRemaining(int total, int spawned, std::size_t hatchCount, std::size_t index);
inline std::optional<std::size_t> hoveredHatch(std::span<const HatchHoverTarget> targets, int x, int y,
                                              std::optional<std::size_t> requiredHatch = std::nullopt) {
  for (std::size_t i = 0; i < targets.size(); ++i) {
    const auto& h = targets[i];
    if (requiredHatch && h.index != *requiredHatch) continue;
    if (pointerOverSceneBounds(x, y, h.left, h.top, h.right, h.bottom)) return i;
  }
  return std::nullopt;
}
// adjustingRate adds the held-slide RATE line. noticeRate temporarily replaces
// the remaining count with a RATE line after a one-step button adjustment.
void drawHatchHover(Image&, const Image& font, const HatchHoverTarget&,
                    std::optional<int> adjustingRate = std::nullopt,
                    std::optional<int> noticeRate = std::nullopt);

// Clone addition: tap text flanking each entrance. Direction +1 raises the
// release rate and -1 lowers it; hatch is the serialized entrance index.
struct HatchRateButton {
  std::size_t hatch = 0;
  int direction = 0;
};
std::optional<HatchRateButton> hatchRateButtonAt(std::span<const HatchHoverTarget> targets,
                                                 int x, int y, int width = kGameWidth,
                                                 int height = kGameHeight);
void drawHatchRateButtons(Image&, const Image& font, std::span<const HatchHoverTarget> targets,
                          std::optional<std::pair<int, int>> hover = std::nullopt);

void drawExitHover(Image&, const Image& font, const PickBox&, int saveTarget);

} // namespace webo::app
