// C2i/C2j: the "Paused" caption panel (the translucent quad behind the pause caption) and its text.
//
// Evidence tags as in MenuRender.h. Nothing here was tuned to the A7 score.
//   MEASURED   C2h, live read at G+0x25C90 in five paused snapshots: the quad is
//              [277,422,363,454), exact integer doubles; [0x58CB40/44] = 640, 480.
//   READ       C2f: 0x45D2A8..0x45D35A computes it as
//              x0 = half(P40) - half(W) - 8, y0 = P44 - 2H, x1 = half(P40) + half(W) + 8,
//              y1 = P44 - H + 3 (half = cdq; sub; sar). Stored by 0x44DC50, drawn by 0x44DE60.
//   DERIVED    half(W) = 35 and H = 29 from the measured quad and P40, P44 (W itself is not
//              recoverable: 70 or 71).
//   READ       C2i, caller 0x45D35F..0x45D3BD and the setters: 0x44DCC0 stores 0.00225 at
//              quad+0x18 (+0x68 per vertex); 0x44DBC0 passes six zero dwords to 0x4017F0 (which
//              stores dword i at +4*i, so three zero doubles) and copies 24 bytes to quad+0x40;
//              0x44DD20 stores the double 128.0 at quad+0x58, the double right after that colour
//              block; 0x44DE60 only registers the vertices and calls 0x46D460.
//   READ       C2j: the same setters have other callers; 0x44FF46 passes 0x406FE000 (255.0) three
//              times to 0x44DBC0, so colour is on a 0..255 double scale (INFERRED from the sibling
//              call sites; the reader of the colour and alpha doubles is not read).
//   INFERRED   fill (0,0,0) and alpha 128 of 255. The four-double r,g,b,a block layout is read
//              from the offsets; that the +0x58 double is read as alpha is not.
//   UNREAD     the blend in 0x46D460 (OURS: (px*(255-a)+fill*a+127)/255), what 0.00225 and the
//              dword stored by 0x44DBA0 (the 1 at 0x45D3AF) mean, and that the quad doubles are
//              final frame pixels (untested by C2h: identity transform at 640x480).
//   READ       C2j: STR_PAUSED (0x4C3C18) is slot 84 of the pointer table at 0x4C3494 (pointer
//              0x4C35E4); 0x45D262..0x45D28C pushes 0x54 and calls 0x486847 (lookup, UNREAD).
//   MEASURED   C2h: text placement 285,422 (G+0x25C68 +0x14/+0x18) = rect x0 + 8, y0.
//   READ       "Paused" in the ^Font glyph table (0x4BF968) is 14+11+12+11+11+11 = 70 wide,
//              which is the derived W (70 or 71).
//   MEASURED-FROM-REFERENCE  the display string "Paused" and white ink: the A7 baseline crop
//              shows them; the exe holds only the key, so neither is read from it.

#pragma once

#include "assets/Bitmap.h"
#include "shell/HudText.h"

#include <array>
#include <cstdint>
#include <string>
#include <span>

namespace webo::shell {

struct CaptionRect {
  int x0 = 0, y0 = 0, x1 = 0, y1 = 0;  // [x0,x1) x [y0,y1)
  bool operator==(const CaptionRect&) const = default;
};

// The 0x45D2A8..0x45D340 formula (READ). `halfW` is half(W), `h` is H.
CaptionRect captionRectFromInputs(int p40, int p44, int halfW, int h);

// Every render input in one place so a test can change one alone.
struct CaptionParams {
  CaptionRect rect{277, 422, 363, 454};   // MEASURED (C2h), formula READ (C2f)
  std::array<std::uint8_t, 3> fill{0, 0, 0};  // HYPOTHESIS: six zero pushes at 0x45D37A..0x45D384
  int alpha = 128;                        // HYPOTHESIS: the double 128.0 stored by 0x44DD20
  int alphaScale = 255;                   // INFERRED: 255.0 colour scale at 0x44FF46; blend UNREAD
  std::string text = "Paused";            // MEASURED-FROM-REFERENCE (exe holds only STR_PAUSED)
  int textX = 285, textY = 422;           // MEASURED live (C2h), G+0x25C68 +0x14/+0x18
  std::array<std::uint8_t, 3> textColour{255, 255, 255};  // MEASURED-FROM-REFERENCE; exe colour UNREAD
};

// Blend `fill` over the rect: px = (px * (scale - alpha) + fill * alpha + scale / 2) / scale.
void drawCaptionPanel(webo::Image& target, const CaptionParams& p = {});

// The caption text as one HudLine, drawn by HudText's drawHud over the panel (not forked).
HudLine captionLine(const CaptionParams& p = {});

// Panel then text (C2f draw order: quad 0x44DE60, then the cell list 0x44E150).
void drawCaption(webo::Image& target, const webo::Image& sheet, std::span<const GlyphCell> cells,
                 const CaptionParams& p = {});

}  // namespace webo::shell
