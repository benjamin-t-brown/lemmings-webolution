#pragma once

#include "assets/Bitmap.h"
#include <cstddef>
#include <optional>
#include <span>

namespace webo::app {

// Displayed hatch geometry, in framebuffer pixels; index is the serialized
// hatch order also used by World::setup().entrances. No simulation state.
struct HatchHoverTarget {
  std::size_t index = 0;
  int left = 0, top = 0, right = 0, bottom = 0;
  int remaining = 0;
};

int hatchRemaining(int total, int spawned, std::size_t hatchCount, std::size_t index);
std::optional<std::size_t> hoveredHatch(std::span<const HatchHoverTarget>, int x, int y);
void drawHatchHover(Image&, const Image& font, const HatchHoverTarget&);

} // namespace webo::app
