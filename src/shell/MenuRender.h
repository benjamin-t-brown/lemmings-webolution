// B1w: the highlight-0 main menu, rendered only from inputs that were read, observed or
// (labelled) measured from the reference screengrab.
//
// Every constant below carries one of these tags:
//   READ       bytes of the stock exe at an address, re-runnable with read_steppers.py
//   OBSERVED   a live read of the running original (b1m, b1v)
//   MEASURED-FROM-REFERENCE  taken from build/tmp/b1b-reference-screengrab/menu_highlight0.png.
//              That is a measurement of the picture, not a read of the original.
//   UNREAD     not read; the renderer states what it does instead
// Nothing here was tuned to the score.

#pragma once

#include "assets/Bitmap.h"

#include <array>
#include <cstdint>
#include <span>
#include <string>
#include <string_view>
#include <vector>

namespace webo::shell {

// One ^Font table record: cell origin in the sheet and glyph width. (READ, B1i: 3-byte
// {X, Y, width} records, `cell = table + ((ch - 0x20) & 0xff) * 3 + 4`.)
struct GlyphCell {
  std::uint8_t x = 0, y = 0, width = 0;
};

inline constexpr int kFirstChar = 0x20;  // READ 0x4403A7 (push 0x20)
inline constexpr int kGlyphCount = 95;   // 0x20..0x7E; the ASCII span the menu uses
inline constexpr int kLineHeight = 29;   // READ: [table+0] at 0x4BF968, via 0x440D40

// The ^Font glyph table for 0x20..0x7E, transcribed from the exe at 0x4BF968 + 4 (READ).
// The real-archive test re-reads the exe bytes and compares.
const std::array<GlyphCell, kGlyphCount>& fontCells();
inline constexpr std::uint32_t kFontTableVa = 0x4BF968;

// READ (B1y): 0x48A232 `lea edx,[eax+ecx-0x14]`; 0x489B39 puts the border one pixel inside that
// edge and the fill one pixel inside the border.
inline constexpr int kPanelOuterPad = 0x14;
inline constexpr int kPanelBorderInset = 1;
inline constexpr int kPanelFillInset = 1;

struct MenuParams {
  // OBSERVED live (B1m): word [0x58CB40] = 640. READ: 0x455F10 loads it, 0x488AF2..0x488AF8
  // halve it (signed). The halved value is the label anchor X.
  int xWord = 640;
  // READ (B1k): 0xDC at [0x5D59E0] (0x48ECE6) + 0x64 (0x488554) = 320 for row 0.
  int baseY = 320;
  // READ: 0x488554 adds (row) * lineHeight. Nothing else steps rows.
  int rowStep = kLineHeight;

  // B1y, panel geometry. READ (0x48A190, 0x489B39; round-note build/tmp/b1y-panel-fields-colour):
  // the rect handed to 0x489B39 starts at X = [+0x44] + [+0x34] - 0x14 (0x48A232) and is
  // W = [+0x3C] + 0x28 wide; the BORDER quad is [X+1, X+W-1] and the FILL quad [X+2, X+W-2]. The
  // fields are written by the layout function around 0x48AC38..0x48AE62 (one loop over the label
  // lines): +0x34 = min(0, line X offset), +0x38 = min(0, line Y), +0x3C = max(0, line right edge)
  // minus +0x34, +0x40 = max (line Y + 0x489850(0x489760())) minus +0x38. With the B1w line X
  // offset -(width/2) that makes the border's left edge anchor - halve(width) - 19 = left - 19
  // and its right edge left + width + 19, so the X pad is READ by derivation. It still rests on [+0x44]
  // (setter 0x489DF4/0x489E40, callers unread) equalling the label anchor halve(xWord).
  int panelPadX = kPanelOuterPad - kPanelBorderInset;
  // MEASURED-FROM-REFERENCE, still: top 1 and height 27. The border's top is Y+1 and its height
  // H-2 (READ), but Y = [+0x48]+[+0x38] needs the anchor [+0x48] (unread) and the line Y out of
  // 0x48A720 (unread), and H needs 0x489850 (unread; 29 only if it is the ^Font lineHeight).
  int panelTop = 1;
  int panelHeight = 27;
  // Colours. READ: the fill (quad +8) is set by 0x489ADC (flag byte [+0x358], marks [+0x359]):
  // flag != 0 -> 0x489A84(0xE0, 0x80, 0) (pushes 0x489B08..0x489B11), flag == 0 ->
  // 0x489A84(0xE0, 0x0A, 0) (0x489B20..0x489B2B). 0x489A84 masks each with 0xFF, converts with
  // fild to doubles and calls 0x44DBC0, which hands the three doubles to 0x4017F0 (a plain
  // 6-dword store, no scaling) and copies the record into the quad's four vertex slots
  // (+0x178/+0x110/+0xA8/+0x40). The ctor 0x489999 only sets the 255.0,0,0 default (overwritten by
  // the setter) and the border (quad +0x1B0) to 64.0 x3 (0x489A1C..0x489A2A). So the doubles are
  // the 0..255 channel values and equal the reference measurement exactly; no tint or
  // multiplier is needed to get them. UNREAD: the draw that turns the double into the framebuffer
  // byte (identity assumed from the exact agreement), the callers 0x489F22/4C/90 that choose the
  // flag per row, and any border recolour (none found in the setter).
  std::array<std::uint8_t, 3> panelBorder{64, 64, 64};
  std::array<std::uint8_t, 3> panelHighlight{0xE0, 0x80, 0};
  std::array<std::uint8_t, 3> panelNormal{0xE0, 0x0A, 0};
  int highlight = 0;

  // READ: the label object's colour bytes (+0x14..0x16) are zeroed by its init 0x489890. Setters
  // of those bytes were NOT searched, so black text is READ-as-default, not READ-as-final.
  // The ^Font sheet RGB is white (255,255,255) everywhere; only its alpha plane is used.
  std::array<std::uint8_t, 3> textColour{0, 0, 0};
};

// signed halving at 0x488AF2..0x488AF8 (movsx, cdq, sub, sar 1)
int halveSigned(int v);

// Sum of glyph widths, every character counted (READ 0x48A60F: no leading-space exception
// there). A '^' ends the line (0x48A667). The draw loop at 0x48ADCF..0x48ADE3 alone skips the
// advance for a leading space; renderMainMenu does the same.
int textWidth(std::string_view text, std::span<const GlyphCell> cells);

// Left edge of a label. READ: the label flags are 0x840 (push at 0x488AA3, stored to +0xc by
// 0x4862B0, OR-ed into the label object's +0x10 by 0x48B960 at 0x489FCD..). 0x48A720 tests bit
// 0x40 at 0x48A747 and stores -(signed width / 2) as the line's X offset, so the anchor is a
// CENTRE: left = halve(xWord) - halve(width). Bit 0x800 is tested nowhere in 0x48A720.
int labelLeft(std::string_view text, std::span<const GlyphCell> cells, int xWord);

// Composites the five labels (UK indices 125 126 120 3 43, supplied as text) over the
// backdrop. `sheet` is the ^Font image with its alpha plane applied.
webo::Image renderMainMenu(const webo::Image& backdrop, const webo::Image& sheet,
                           std::span<const std::string> labels, const MenuParams& params,
                           std::span<const GlyphCell> cells);

inline constexpr std::array<int, 5> kLabelIndices{125, 126, 120, 3, 43};  // READ 0x4C4214

}  // namespace webo::shell
