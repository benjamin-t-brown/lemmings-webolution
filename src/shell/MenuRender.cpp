#include "MenuRender.h"

#include <algorithm>
#include <cstddef>

namespace webo::shell {
namespace {

// Original ^Font table at 0x4BF968 + 4, records for 0x20..0x7E.
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

int menuPanelAt(std::span<const std::array<int, 4>> boxes, int x, int y) {
  for (std::size_t row = 0; row < boxes.size(); ++row) {
    const auto& [left, top, right, bottom] = boxes[row];
    if (left <= x && x < right && top <= y && y < bottom) return static_cast<int>(row);
  }
  return -1;
}

std::array<int, 4> menuPlateBounds(int textX, int textY, int width, const MenuParams& params) {
  const int top = textY + params.panelTop;
  return {textX - params.panelPadX, top, textX + width + params.panelPadX,
          top + params.panelHeight};
}

void drawScreenRect(Image& image, const ScreenRect& draw) {
  const auto& [x0, y0, x1, y1] = draw.bounds;
  for (int y = std::max(0, y0); y < std::min(image.height, y1); ++y)
    for (int x = std::max(0, x0); x < std::min(image.width, x1); ++x) {
      auto* px = image.rgba.data() + (static_cast<std::size_t>(y) * image.width + x) * 4;
      std::copy_n(draw.colour.data(), 3, px);
      if (draw.opaqueAlpha) px[3] = 255;
    }
}

PreparedMenuRows prepareMenuRows(const Image& font, std::span<const std::string> labels,
    const MenuParams& params, std::span<const GlyphCell> cells) {
  PreparedMenuRows prepared{&font, {}};
  prepared.rows.reserve(labels.size());
  for (std::size_t row = 0; row < labels.size(); ++row) {
    const std::string_view text = labels[row];
    const int textY = params.baseY + static_cast<int>(row) * params.rowStep;
    const int left = labelLeft(text, cells, params.xWord);
    const int width = textWidth(text, cells);

    // Draw the panel border before the fill.
    const auto [px0, py0, px1, py1] = menuPlateBounds(left, textY, width, params);
    const auto& fill = static_cast<int>(row) == params.highlight ? params.panelHighlight
                                                                 : params.panelNormal;
    PreparedMenuRows::Row draw{
        {{px0, py0, px1, py1}, params.panelBorder},
        {{px0 + kPanelFillInset, py0 + kPanelFillInset,
          px1 - kPanelFillInset, py1 - kPanelFillInset}, fill}, {}};

    // glyphs: quad [x, x+w) x [textY, textY+lineHeight) from the cell, 1:1, alpha only
    int x = left;
    for (std::size_t i = 0; i < text.size(); ++i) {
      const unsigned char ch = static_cast<unsigned char>(text[i]);
      if (ch == '^')
        break;
      const GlyphCell* c = cellFor(ch, cells);
      if (!c)
        continue;
      draw.glyphs.push_back({{c->x, c->y, c->width}, x, textY, params.textColour, kLineHeight, std::nullopt});
      if (!(i == 0 && ch == ' '))
        x += c->width;
    }
    prepared.rows.push_back(std::move(draw));
  }
  return prepared;
}

void drawMenuRows(Image& target, const PreparedMenuRows& prepared) {
  target.alpha = AlphaMode::Opaque;
  for (const auto& row : prepared.rows) {
    drawScreenRect(target, row.border);
    drawScreenRect(target, row.fill);
    drawHudGlyphs(target, *prepared.font, row.glyphs);
  }
}

void drawMenuRows(Image& target, const Image& sheet, std::span<const std::string> labels,
    const MenuParams& params, std::span<const GlyphCell> cells) {
  drawMenuRows(target, prepareMenuRows(sheet, labels, params, cells));
}

webo::Image renderMainMenu(const webo::Image& backdrop, const webo::Image& sheet,
                          std::span<const std::string> labels, const MenuParams& params,
                          std::span<const GlyphCell> cells) {
  auto out = backdrop;
  drawMenuRows(out, sheet, labels, params, cells);
  return out;
}

std::array<ScreenRect, 16> prepareVolumeMeter(int width, int top, int percent) {
  // 48E84C / 490D46: truncate gain * 255, then round to a 16-segment count.
  const int lit = (std::clamp(percent, 0, 100) * 255 / 100 + 8) / 16;
  std::array<ScreenRect, 16> rects;
  for (int i = 0; i < 16; ++i) {
    const int left = (width - 16 * 11) / 2 + i * 11;
    rects[i] = {{left, top, left + 8, top + 10},
        i < lit ? std::array<std::uint8_t, 3>{0, 255, 0} : std::array<std::uint8_t, 3>{255, 0, 0}};
  }
  return rects;
}

void drawVolumeMeter(Image& frame, int top, int percent) {
  for (const auto& rect : prepareVolumeMeter(frame.width, top, percent))
    drawScreenRect(frame, rect);
}

}  // namespace webo::shell
