#pragma once

#include "assets/Bitmap.h"

#include <array>
#include <optional>
#include <span>

namespace webo::shell {

struct HudGlyph {
  struct Source { int x, y, width; } source;
  int x, y;
  std::array<std::uint8_t, 3> colour;
  int height;
  std::optional<double> comparisonDepth;
};

inline void drawHudGlyphs(Image& target, const Image& sheet,
    std::span<const HudGlyph> draws, std::span<const double> depth = {}) {
  const bool hasDepth = depth.size() == target.pixelCount();
  for (const auto& draw : draws) {
    for (int dy = 0; dy < draw.height; ++dy) {
      for (int dx = 0; dx < draw.source.width; ++dx) {
        const int sx = draw.source.x + dx, sy = draw.source.y + dy;
        const int ox = draw.x + dx, oy = draw.y + dy;
        if (sx < 0 || sy < 0 || sx >= sheet.width || sy >= sheet.height ||
            ox < 0 || oy < 0 || ox >= target.width || oy >= target.height) continue;
        const auto pixel = static_cast<std::size_t>(oy) * target.width + ox;
        if (hasDepth && draw.comparisonDepth && !(*draw.comparisonDepth < depth[pixel])) continue;
        const int a = sheet.rgba[(static_cast<std::size_t>(sy) * sheet.width + sx) * 4 + 3];
        if (a == 0) continue;
        auto* px = target.rgba.data() + pixel * 4;
        for (int k = 0; k < 3; ++k)
          px[k] = static_cast<std::uint8_t>((px[k] * (255 - a) + draw.colour[k] * a + 127) / 255);
      }
    }
  }
}

}  // namespace webo::shell
