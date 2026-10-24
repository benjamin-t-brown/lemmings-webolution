// C2i: the "Paused" caption panel, the translucent quad behind the pause caption.
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
//              quad+0x18 (+0x68 per vertex); 0x44DBC0 passes six zero dwords and copies 24 bytes
//              to quad+0x40; 0x44DD20 stores the double 128.0 at quad+0x58, right after that
//              colour block; 0x44DE60 only registers the vertices and calls 0x46D460.
//   HYPOTHESIS the fill is colour (0,0,0) and the 128.0 is its alpha on a 0..255 scale. The
//              callee 0x4017f0 (that six zero dwords are three zero doubles), 0x44DBA0, and the
//              blend in 0x46D460 are UNREAD, so colour, alpha scale and blend are not read.
//   UNREAD     the caption text (STR_PAUSED at 0x4C3C18; its string-table consumer) and its
//              layout. It is not drawn. That the quad doubles are final frame pixels is
//              untested by C2h (identity transform at 640x480).

#pragma once

#include "assets/Bitmap.h"

#include <array>
#include <cstdint>

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
  int alphaScale = 255;                   // UNREAD: blend in 0x46D460 not read
};

// Blend `fill` over the rect: px = (px * (scale - alpha) + fill * alpha + scale / 2) / scale.
void drawCaptionPanel(webo::Image& target, const CaptionParams& p = {});

}  // namespace webo::shell
