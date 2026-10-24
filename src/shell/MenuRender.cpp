#include "MenuRender.h"

#include <algorithm>
#include <cstddef>

namespace webo::shell {
namespace {

// READ: exe bytes at 0x4BF968 + 4, records for 0x20..0x7E.
constexpr std::array<GlyphCell, kGlyphCount> kCells{{
    {0, 0, 8}, {8, 0, 7}, {16, 0, 10}, {26, 0, 14}, {40, 0, 13},
    {54, 0, 20}, {74, 0, 0}, {74, 0, 6}, {80, 0, 8}, {88, 0, 8},
    {96, 0, 11}, {108, 0, 14}, {122, 0, 6}, {128, 0, 8}, {136, 0, 6},
    {142, 0, 6}, {148, 0, 13}, {162, 0, 13}, {176, 0, 13}, {190, 0, 13},
    {204, 0, 13}, {218, 0, 13}, {232, 0, 13}, {0, 30, 13}, {14, 30, 13},
    {28, 30, 13}, {42, 30, 6}, {48, 30, 6}, {54, 30, 6}, {60, 30, 14},
    {74, 30, 6}, {80, 30, 10}, {90, 30, 17}, {108, 30, 15}, {124, 30, 14},
    {138, 30, 14}, {152, 30, 15}, {168, 30, 13}, {182, 30, 13}, {196, 30, 15},
    {212, 30, 16}, {228, 30, 8}, {236, 30, 9}, {0, 60, 14}, {14, 60, 12},
    {26, 60, 19}, {46, 60, 16}, {62, 60, 16}, {78, 60, 14}, {92, 60, 15},
    {108, 60, 15}, {124, 60, 14}, {138, 60, 14}, {152, 60, 15}, {168, 60, 14},
    {182, 60, 20}, {202, 60, 14}, {216, 60, 14}, {230, 60, 13}, {244, 60, 8},
    {0, 90, 14}, {14, 90, 8}, {22, 90, 8}, {30, 90, 14}, {44, 90, 8},
    {52, 90, 11}, {64, 90, 12}, {76, 90, 11}, {88, 90, 11}, {100, 90, 11},
    {112, 90, 8}, {120, 90, 11}, {132, 90, 12}, {144, 90, 6}, {150, 90, 7},
    {158, 90, 11}, {170, 90, 6}, {176, 90, 19}, {196, 90, 12}, {208, 90, 12},
    {220, 90, 12}, {232, 90, 12}, {244, 90, 10}, {0, 120, 11}, {12, 120, 8},
    {20, 120, 12}, {32, 120, 11}, {44, 120, 16}, {60, 120, 11}, {72, 120, 11},
    {84, 120, 10}, {94, 120, 8}, {102, 120, 14}, {116, 120, 8}, {124, 120, 9},
}};

void put(webo::Image& img, int x, int y, const std::array<std::uint8_t, 3>& c) {
  if (x < 0 || y < 0 || x >= img.width || y >= img.height)
    return;
  std::uint8_t* p = img.rgba.data() + (static_cast<std::size_t>(y) * img.width + x) * 4;
  p[0] = c[0];
  p[1] = c[1];
  p[2] = c[2];
  p[3] = 255;
}

void fillRect(webo::Image& img, int x0, int y0, int x1, int y1,
              const std::array<std::uint8_t, 3>& c) {
  for (int y = y0; y < y1; ++y)
    for (int x = x0; x < x1; ++x)
      put(img, x, y, c);
}

const GlyphCell* cellFor(unsigned char ch, std::span<const GlyphCell> cells) {
  const unsigned idx = (static_cast<unsigned>(ch) - kFirstChar) & 0xff;
  return idx < cells.size() ? &cells[idx] : nullptr;
}

}  // namespace

const std::array<GlyphCell, kGlyphCount>& fontCells() { return kCells; }

int halveSigned(int v) { return (v - (v < 0 ? -1 : 0)) >> 1; }

int textWidth(std::string_view text, std::span<const GlyphCell> cells) {
  int w = 0;
  for (std::size_t i = 0; i < text.size(); ++i) {
    const unsigned char ch = static_cast<unsigned char>(text[i]);
    if (ch == '^')
      break;
    if (const GlyphCell* c = cellFor(ch, cells))
      w += c->width;
  }
  return w;
}

int labelLeft(std::string_view text, std::span<const GlyphCell> cells, int xWord) {
  return halveSigned(xWord) - halveSigned(textWidth(text, cells));
}

webo::Image renderMainMenu(const webo::Image& backdrop, const webo::Image& sheet,
                           std::span<const std::string> labels, const MenuParams& params,
                           std::span<const GlyphCell> cells) {
  webo::Image out = backdrop;
  out.alpha = webo::AlphaMode::Opaque;
  for (std::size_t row = 0; row < labels.size(); ++row) {
    const std::string_view text = labels[row];
    const int textY = params.baseY + static_cast<int>(row) * params.rowStep;
    const int left = labelLeft(text, cells, params.xWord);
    const int width = textWidth(text, cells);

    // panel: border then fill (X pad, insets and colours READ in B1y; top/height MEASURED)
    const int px0 = left - params.panelPadX, px1 = left + width + params.panelPadX;
    const int py0 = textY + params.panelTop, py1 = py0 + params.panelHeight;
    const auto& fill = static_cast<int>(row) == params.highlight ? params.panelHighlight
                                                                 : params.panelNormal;
    fillRect(out, px0, py0, px1, py1, params.panelBorder);
    fillRect(out, px0 + kPanelFillInset, py0 + kPanelFillInset, px1 - kPanelFillInset,
             py1 - kPanelFillInset, fill);

    // glyphs: quad [x, x+w) x [textY, textY+lineHeight) from the cell, 1:1, alpha only
    int x = left;
    for (std::size_t i = 0; i < text.size(); ++i) {
      const unsigned char ch = static_cast<unsigned char>(text[i]);
      if (ch == '^')
        break;
      const GlyphCell* c = cellFor(ch, cells);
      if (!c)
        continue;
      for (int dy = 0; dy < kLineHeight; ++dy) {
        for (int dx = 0; dx < c->width; ++dx) {
          const int sx = c->x + dx, sy = c->y + dy;
          if (sx >= sheet.width || sy >= sheet.height)
            continue;
          const int a = sheet.rgba[(static_cast<std::size_t>(sy) * sheet.width + sx) * 4 + 3];
          const int ox = x + dx, oy = textY + dy;
          if (a == 0 || ox < 0 || oy < 0 || ox >= out.width || oy >= out.height)
            continue;
          std::uint8_t* p = out.rgba.data() + (static_cast<std::size_t>(oy) * out.width + ox) * 4;
          for (int k = 0; k < 3; ++k)
            p[k] = static_cast<std::uint8_t>((p[k] * (255 - a) + params.textColour[k] * a + 127) / 255);
        }
      }
      if (!(i == 0 && ch == ' '))
        x += c->width;
    }
  }
  return out;
}

}  // namespace webo::shell
