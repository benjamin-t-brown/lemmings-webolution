// C2g: the in-level Out / Saved counters and the countdown timer, drawn from the C2c/C2d reads.
//
// Evidence tags as in MenuRender.h. Nothing here was tuned to the A7 score.
//   READ       0x44D22C: counter right end 0x26C = 620, Out y 0xF, Saved y 0x32 (push at 0x44D2B7);
//              0x44D316 formats %d (0x4C01F4 / 0x4C01F8) and replaces the Out / Saved words
//              (C2d); Saved is `movsx word [0x4CBAA4]`; 0x44D395..0x44D3CC colour: 255,255,255
//              below word [0x4CBAB0], 255,63,63 at or above it (R,G,B order UNREAD, 0x403820).
//              Timer: [0x4CBAAC] split by `idiv 60`, formats "%02d:0%d" (0x4C01DC) /
//              "%02d:%02d" (0x4C01E8), placed at 560,430 by 0x44DF40; negative stored back as 0.
//   INFERRED   the counter anchor is the RIGHT end of the quad run: 0x44D22C passes
//              0x26C - [last glyph + 0xF0] + [first glyph + 0x20] to 0x44DF40, which shifts every
//              glyph by (x - list x); 0x414770 / 0x44D860 truncate doubles at [glyph+0x20] and
//              [glyph+0xF0]. What those two fields hold is NOT read. IF they are the first quad's
//              left and the last quad's right x, the right end lands on 620, so left =
//              620 - textWidth(digits). Right end versus centre is therefore not settled.
//   UNREAD     the default glyph colour (Out, timer): drawn white. The relation of word [0x4CBAB0]
//              to the level's save target (it has no direct-address writer): the sim's saveTarget
//              is used. What `[0x4E33C8] == 1` means (the timer submit is skipped then; not modelled).
//              The original's Out definition (sum of words at this+0x5070); the sim's
//              spawned - saved - dead is used.

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

// Every layout input in one place so a test can change one alone.
struct HudParams {
  int counterRightEnd = 620;  // READ 0x44D288 (0x26C)
  int outY = 15;              // READ 0x44D22C (push 0xF)
  int savedY = 50;            // READ 0x44D2B7 (push 0x32)
  int timerX = 560;           // READ C2a / 0x44DF40 operands
  int timerY = 430;
  int secondsPerMinute = 60;  // READ idiv 60
  Rgb white{255, 255, 255};   // Out / timer: UNREAD default; Saved below target: READ
  Rgb atTarget{255, 63, 63};  // READ doubles 255, 63, 63 in memory order; R,G,B order UNREAD
};

struct HudValues {
  int out = 0;
  int saved = 0;
  int saveTarget = 0;         // stands in for word [0x4CBAB0] (UNREAD relation)
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
