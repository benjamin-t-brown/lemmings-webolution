// Renders the main menu from the ^Font sheet, label text and panel geometry.

#pragma once

#include "assets/Bitmap.h"

#include <array>
#include <cstdint>
#include <span>
#include <string>
#include <string_view>
#include <vector>

namespace webo::shell {

// Each ^Font record holds X, Y and width. cell = table + ((ch - 0x20) & 0xff) * 3 + 4.
struct GlyphCell {
  std::uint8_t x = 0, y = 0, width = 0;
};

inline constexpr int kFirstChar = 0x20;  // READ 0x4403A7 (push 0x20)
inline constexpr int kGlyphCount = 95;   // 0x20..0x7E; the ASCII span the menu uses
inline constexpr int kLineHeight = 29;   // READ: [table+0] at 0x4BF968, via 0x440D40

// ^Font glyphs 0x20..0x7E from the original table at 0x4BF968 + 4.
const std::array<GlyphCell, kGlyphCount>& fontCells();
inline constexpr std::uint32_t kFontTableVa = 0x4BF968;

// 0x48A232 uses lea edx,[eax+ecx-0x14]. 0x489B39 insets border and fill one pixel each.
inline constexpr int kPanelOuterPad = 0x14;
inline constexpr int kPanelBorderInset = 1;
inline constexpr int kPanelFillInset = 1;

struct MenuParams {
  // 0x455F10 loads word [0x58CB40] (640). 0x488AF2..0x488AF8 halves it, signed, for label X.
  int xWord = 640;
  // Row 0 is 0xDC from [0x5D59E0] (0x48ECE6) plus 0x64 (0x488554).
  int baseY = 320;
  // 0x488554 adds row times lineHeight.
  int rowStep = kLineHeight;

  // Panel at 0x48A190 / 0x489B39: X = [+0x44] + [+0x34] - 0x14 (0x48A232), W = [+0x3C] + 0x28.
  // Border spans [X+1, X+W-1] and fill [X+2, X+W-2]. 0x48AC38..0x48AE62 fills the line bounds:
  // +0x34 / +0x38 are min(0, line X/Y). +0x3C is max(0, right edges) minus +0x34.
  // +0x40 is max(line Y + 0x489850(0x489760())) minus +0x38. A centred label gives 19 px
  // of horizontal border padding if [+0x44] is the label anchor (setters 0x489DF4/0x489E40).
  int panelPadX = kPanelOuterPad - kPanelBorderInset;
  // Panel top 1 and height 27 are measured pixels. The border uses Y+1 and H-2.
  // Original Y = [+0x48]+[+0x38] depends on [+0x48] and 0x48A720, and H on 0x489850.
  // The exact Y and H inputs remain unresolved.
  int panelTop = 1;
  int panelHeight = 27;
  // 0x489ADC selects the fill (quad +8) from flag [+0x358], marks [+0x359].
  // Nonzero calls 0x489A84(0xE0, 0x80, 0) at 0x489B08..0x489B11, zero uses
  // 0x489A84(0xE0, 0x0A, 0) at 0x489B20..0x489B2B. 0x489A84 masks to 0xFF and
  // passes doubles via 0x44DBC0 / 0x4017F0 to +0x178/+0x110/+0xA8/+0x40.
  // 0x489999 initializes fill to 255,0,0 and border (quad +0x1B0) to 64,64,64 (0x489A1C..0x489A2A).
  // Clone rendering maps doubles directly to bytes. Original conversion and row-flag callers 0x489F22/4C/90 are unresolved.
  std::array<std::uint8_t, 3> panelBorder{64, 64, 64};
  std::array<std::uint8_t, 3> panelHighlight{0xE0, 0x80, 0};
  std::array<std::uint8_t, 3> panelNormal{0xE0, 0x0A, 0};
  int highlight = 0;

  // 0x489890 zeros the label colour bytes (+0x14..0x16). The renderer uses this black default
  // with the ^Font alpha plane. The original final text colour is unresolved.
  std::array<std::uint8_t, 3> textColour{0, 0, 0};
};

// signed halving at 0x488AF2..0x488AF8 (movsx, cdq, sub, sar 1)
int halveSigned(int v);

// 0x48A60F sums every glyph width until '^' (0x48A667). The draw loop
// at 0x48ADCF..0x48ADE3 skips advance for a leading space.
int textWidth(std::string_view text, std::span<const GlyphCell> cells);

// 0x488AA3 pushes 0x840, 0x4862B0 stores it at +0xc, and 0x48B960 ORs it into +0x10
// at 0x489FCD. 0x48A720 tests 0x40 at 0x48A747 and offsets by negative half-width, centring the
// label at halve(xWord). 0x48A720 does not test 0x800.
int labelLeft(std::string_view text, std::span<const GlyphCell> cells, int xWord);

std::array<int, 4> menuPlateBounds(int textX, int textY, int width, const MenuParams& params);
void drawMenuRows(webo::Image& frame, const webo::Image& sheet, std::span<const std::string> labels,
                  const MenuParams& params, std::span<const GlyphCell> cells);

// Composites the five labels (UK indices 125 126 120 3 43, supplied as text) over the
// backdrop. `sheet` is the ^Font image with its alpha plane applied.
webo::Image renderMainMenu(const webo::Image& backdrop, const webo::Image& sheet,
                           std::span<const std::string> labels, const MenuParams& params,
                           std::span<const GlyphCell> cells);

// Clone input uses half-open panel bounds for clicks and hover. A miss preserves focus.
int menuPanelAt(std::span<const std::array<int, 4>> boxes, int x, int y);

inline constexpr std::array<int, 5> kLabelIndices{125, 126, 120, 3, 43};  // READ 0x4C4214

void drawVolumeMeter(Image& frame, int top, int percent);

}  // namespace webo::shell
