// Draws the "Paused" panel at [277,422,363,454) for a 640x480 frame ([0x58CB40/44], panel G+0x25C90).
// 0x45D2A8..0x45D35A uses x0 = half(P40) - half(W) - 8, y0 = P44 - 2H,
// x1 = half(P40) + half(W) + 8 and y1 = P44 - H + 3 (cdq; sub; sar).
// This implies half(W) = 35 and H = 29. 0x44DC50 stores the rect and 0x44DE60 draws it.
// At 0x45D35F..0x45D3BD, 0x44DCC0 stores 0.00225 at quad+0x18 (+0x68 per vertex),
// 0x44DBC0 copies three zero doubles from the +4*i dword store in 0x4017F0 to quad+0x40,
// and 0x44DD20 stores alpha 128.0 at quad+0x58. At 0x44FF46, 0x44DBC0 receives 0x406FE000 (255.0).
// The meanings of 0.00225 and the 1 at 0x45D3AF passed to 0x44DBA0 remain unresolved.
// 0x46D460 writes x,y,z,rhw (0x46D4F9..0x46D528), and packs diffuse (0x46D52E..0x46D586)
// from d[+0x50], c[0], c[1] and c[2] using int() at 0x49C5AC (fnstcw/or ah,0xC/fistp).
// The vertex base is quad+8, with vertex+0x38 RGB, vertex+0x50 alpha and vertex+0x10 for 0.00225.
// Packed RGB is zero and alpha is 0x80. Vertex lookup at 0x414A40 is unresolved.
// It adds [0x58D2A0/A8] to UV and submits (6, 0x1C4, verts, count) via vtable +0x70
// (0x46D5DE..0x46D5F3). FVF 0x1C4 is x,y,z,rhw, diffuse and tex1. Primitive 6 is unresolved.
// The clone blend is provisional. The D3D render states (0xF, 0x18, 0x19) at 0x472860..0x472975
// are from another path. This quad's device blend, rounding and texture state are unresolved.
// 0x45D262..0x45D28C calls 0x486847(0x54) for STR_PAUSED (0x4C3C18), slot 84 of 0x4C3494
// (pointer 0x4C35E4). 0x486819 uses [0x4C3434] (2, Uk). If idx >= table[lang][0],
// it returns 0x4C3450 "Unknown String". Otherwise it loads the relocated pointer
// at table[lang][idx + 1]. 0x486670 loads "Language\%s.ltx" (Uk/German/...).
// Index 0x54 is "Paused" in LANGUAGE\UK.LTX in lemmings.box and patch.box (offsets 1548 and 1956).
// White text starts at 285,422 (G+0x25C68 +0x14/+0x18). Its ^Font width at 0x4BF968 is 70 pixels.

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

// Rect formula at 0x45D2A8..0x45D340. halfW is half(W), and h is H.
CaptionRect captionRectFromInputs(int p40, int p44, int halfW, int h);

struct CaptionParams {
  CaptionRect rect{277, 422, 363, 454};   // Panel bounds at 640x480.
  std::array<std::uint8_t, 3> fill{0, 0, 0};  // Zero RGB doubles from 0x45D37A..0x45D384.
  int alpha = 128;                        // Alpha double stored by 0x44DD20.
  int alphaScale = 255;                   // Colour scale from 0x44FF46. Device blend unresolved.
  std::string text = "Paused";            // Localized value for STR_PAUSED.
  int textX = 285, textY = 422;           // Text origin at G+0x25C68 +0x14/+0x18.
  std::array<std::uint8_t, 3> textColour{255, 255, 255};  // Reference text colour. Original colour source unresolved.
};

// 0x46D460 packs truncated alpha, red, green and blue from high to low byte
// (0x46D52E..0x46D586, int() at 0x49C5AC).
std::uint32_t captionDiffuse(const CaptionParams& p = {});

// Blend `fill` over the rect: px = (px * (scale - alpha) + fill * alpha + scale / 2) / scale.
void drawCaptionPanel(webo::Image& target, const CaptionParams& p = {});

// Draw this HudLine over the panel with HudText's drawHud.
HudLine captionLine(const CaptionParams& p = {});

// Draw the quad (0x44DE60) before the cell list (0x44E150).
void drawCaption(webo::Image& target, const webo::Image& sheet, std::span<const GlyphCell> cells,
                 const CaptionParams& p = {});

// Clone addition. Centre both lines in the logical viewport before window scaling.
void drawExamineCaption(webo::Image& target, const webo::Image& sheet, std::span<const GlyphCell> cells);

}  // namespace webo::shell
