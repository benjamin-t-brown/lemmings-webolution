#include "PauseCaption.h"

#include <algorithm>
#include <cstddef>

namespace webo::shell {

namespace {
int half(int v) { return (v - (v >> 31)) >> 1; }  // cdq; sub; sar: truncates toward zero
}  // namespace

CaptionRect captionRectFromInputs(int p40, int p44, int halfW, int h) {
  return {half(p40) - halfW - 8, p44 - 2 * h, half(p40) + halfW + 8, p44 - h + 3};
}

void drawCaptionPanel(webo::Image& target, const CaptionParams& p) {
  const int a = std::clamp(p.alpha, 0, p.alphaScale);
  const int s = p.alphaScale;
  for (int y = std::max(0, p.rect.y0); y < std::min(target.height, p.rect.y1); ++y) {
    for (int x = std::max(0, p.rect.x0); x < std::min(target.width, p.rect.x1); ++x) {
      std::uint8_t* px = target.rgba.data() + (static_cast<std::size_t>(y) * target.width + x) * 4;
      for (int k = 0; k < 3; ++k)
        px[k] = static_cast<std::uint8_t>((px[k] * (s - a) + p.fill[k] * a + s / 2) / s);
    }
  }
}

HudLine captionLine(const CaptionParams& p) { return {p.text, p.textX, p.textY, p.textColour}; }

void drawCaption(webo::Image& target, const webo::Image& sheet, std::span<const GlyphCell> cells,
                 const CaptionParams& p) {
  drawCaptionPanel(target, p);
  const HudLine line = captionLine(p);
  drawHud(target, sheet, std::span<const HudLine>(&line, 1), cells);
}

}  // namespace webo::shell
