// Draws the Out / Saved counters and timer. 0x44D22C uses counter X 0x26C (620), Out Y 0xF
// and Saved Y 0x32 (0x44D2B7). 0x44D316 uses "%d" (0x4C01F4 / 0x4C01F8) and
// movsx word [0x4CBAA4] for Saved. 0x44D395..0x44D3CC compares to word [0x4CBAB0]
// for colour 255,255,255 or 255,63,63. Channel order at 0x403820 remains unresolved.
// Timer [0x4CBAAC] is split by idiv 60, clamped at zero and drawn at 560,430 by 0x44DF40
// with "%02d:0%d" (0x4C01DC) or "%02d:%02d" (0x4C01E8). Original [0x4E33C8] == 1 skips submission.
// Right anchoring is inferred from 0x26C - [last glyph + 0xF0] + [first glyph + 0x20].
// 0x414770 / 0x44D860 truncate [glyph+0x20] and [glyph+0xF0]. Their meanings are unresolved.
// Provisional clone values use white for Out/timer, saveTarget for [0x4CBAB0], and
// spawned minus saved minus dead for Out (original sums words at this+0x5070).

#pragma once

#include "assets/Bitmap.h"
#include "shell/MenuRender.h"

#include <array>
#include <cstdint>
#include <span>
#include <string>
#include <vector>

namespace webo::shell {

using Rgb = std::array<std::uint8_t, 3>;

struct HudParams {
  int counterRightEnd = 620;  // X operand at 0x44D288 (0x26C)
  int outY = 15;              // Y operand at 0x44D22C (push 0xF)
  int savedY = 50;            // Y operand at 0x44D2B7 (push 0x32)
  int timerX = 560;           // X operand for 0x44DF40
  int timerY = 430;
  int secondsPerMinute = 60;  // idiv 60
  Rgb white{255, 255, 255};   // Provisional for Out and timer. Original below-target Saved colour.
  Rgb atTarget{255, 63, 63};  // Original doubles in memory order. Channel order unresolved.
};

struct HudValues {
  int out = 0;
  int saved = 0;
  int saveTarget = 0;         // Provisional stand-in for word [0x4CBAB0].
  int secondsRemaining = 0;   // [0x4CBAAC]
};

struct HudLine {
  std::string text;
  int x = 0;
  int y = 0;
  Rgb colour{255, 255, 255};
};

// "%d" of the argument; Saved goes through `movsx word` first (0x44D316).
std::string counterText(int value);
std::string savedText(int saved);
// minutes = s / 60, seconds = s % 60, negative stored back as 0; "%02d:0%d" below ten seconds.
std::string timerText(int secondsRemaining, int secondsPerMinute = 60);
// Saved colour: white below the target (signed word compare), 255,63,63 at or above it.
Rgb savedColour(int saved, int target, const HudParams& p = {});

// Out, Saved, timer, in that order.
std::vector<HudLine> layoutHud(const HudValues& v, const HudParams& p, std::span<const GlyphCell> cells);

// Glyph quad box of a line: [x, x + width) x [y, y + kLineHeight).
struct HudBox {
  int x0 = 0, y0 = 0, x1 = 0, y1 = 0;
};
HudBox lineBox(const HudLine& line, std::span<const GlyphCell> cells);

// Alpha-only blit of ^Font glyphs over `target`, same blend as renderMainMenu.
void drawHud(webo::Image& target, const webo::Image& sheet, std::span<const HudLine> lines,
             std::span<const GlyphCell> cells);

}  // namespace webo::shell
