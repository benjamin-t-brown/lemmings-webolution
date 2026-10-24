// C2i/C2j: the pause caption panel and text (shell/PauseCaption.h). Each assertion pins one input
// (rectangle edge, fill colour, alpha, text origin, text string) so a change to that input alone fails it.

#include "assets/Bitmap.h"
#include "shell/PauseCaption.h"

// The test target's CMake line lists only PauseCaption.cpp, and CMake files are outside this
// round's owned paths. drawCaption uses HudText's drawHud (and MenuRender's glyph table), so the
// two translation units are pulled in here. The clean fix is adding them to the add_executable line.
#include "shell/HudText.cpp"
#include "shell/MenuRender.cpp"

#include <cstdint>
#include <iostream>
#include <string_view>

using namespace webo::shell;

namespace {

int g_failures = 0;

void check(bool cond, std::string_view what) {
  if (!cond) {
    ++g_failures;
    std::cerr << "  FAIL  " << what << "\n";
  }
}

webo::Image solid(int w, int h, std::uint8_t v) {
  webo::Image img;
  img.width = w;
  img.height = h;
  img.rgba.assign(static_cast<std::size_t>(w) * h * 4, v);
  return img;
}

std::uint8_t at(const webo::Image& img, int x, int y, int k = 0) {
  return img.rgba[(static_cast<std::size_t>(y) * img.width + x) * 4 + k];
}

void testRect() {
  std::cout << "caption rect\n";
  const CaptionRect want{277, 422, 363, 454};
  check((CaptionParams{}.rect == want), "default rect is the C2h measurement [277,422,363,454)");
  check(captionRectFromInputs(640, 480, 35, 29) == want, "formula 0x45D2A8 with P40 640, P44 480, half(W) 35, H 29");
  check(!(captionRectFromInputs(640, 480, 36, 29) == want), "half(W) 36 misses");
  check(!(captionRectFromInputs(640, 480, 35, 30) == want), "H 30 misses");
}

void testDiffuse() {
  std::cout << "caption diffuse\n";
  const CaptionParams p;
  check(captionDiffuse(p) == 0x80000000u, "alpha 128 in the top byte, black fill: 0x80000000 (0x46D52E..0x46D586)");
  CaptionParams q = p;
  q.alpha = 127;
  check(captionDiffuse(q) == 0x7F000000u, "alpha alone moves byte 3");
  q = p;
  q.fill = {1, 2, 3};
  check(captionDiffuse(q) == 0x80010203u, "fill r,g,b land in bytes 2,1,0");
}

void testDraw() {
  std::cout << "caption draw\n";
  const CaptionParams p;
  webo::Image f = solid(640, 480, 255);
  drawCaptionPanel(f, p);
  check(at(f, 277, 422) == 127 && at(f, 362, 453) == 127, "inside corners: 255 * 127/255 + 0 -> 127 (black, alpha 128/255)");
  check(at(f, 276, 422) == 255 && at(f, 277, 421) == 255, "left and top edge stay outside");
  check(at(f, 363, 453) == 255 && at(f, 362, 454) == 255, "right and bottom edge are exclusive");
  check(at(f, 320, 440, 1) == 127 && at(f, 320, 440, 2) == 127, "all three channels blended");
  check(at(f, 320, 440, 3) == 255, "alpha channel untouched");
  webo::Image z = solid(640, 480, 0);
  drawCaptionPanel(z, p);
  check(at(z, 320, 440, 0) == 0 && at(z, 320, 440, 1) == 0 && at(z, 320, 440, 2) == 0,
        "black over black stays 0 in every channel (each fill channel is 0)");

  // each input alone changes the pixels
  CaptionParams q = p;
  q.rect.x0 = 278;
  webo::Image g = solid(640, 480, 200);
  drawCaptionPanel(g, q);
  check(at(g, 277, 430) == 200, "left edge input moves the panel");
  q = p;
  q.rect.y1 = 455;
  g = solid(640, 480, 200);
  drawCaptionPanel(g, q);
  check(at(g, 300, 454) != 200, "bottom edge input moves the panel");
  q = p;
  q.fill = {255, 255, 255};
  g = solid(640, 480, 200);
  drawCaptionPanel(g, q);
  check(at(g, 300, 430) == 228, "white fill: (200*127 + 255*128 + 127)/255 = 228");
  q = p;
  q.alpha = 255;
  g = solid(640, 480, 200);
  drawCaptionPanel(g, q);
  check(at(g, 300, 430) == 0, "alpha 255 black is opaque");
  q = p;
  q.alpha = 0;
  g = solid(640, 480, 200);
  drawCaptionPanel(g, q);
  check(at(g, 300, 430) == 200, "alpha 0 leaves the frame");
}

// A ^Font stand-in: alpha 255 only inside one glyph cell, 0 elsewhere. With the real glyph
// table (0x4BF968) the cell of 'P' is {78, 60, 14}, so only a leading 'P' inks.
webo::Image sheetWithOnly(char ch) {
  webo::Image sheet = solid(256, 256, 255);
  const GlyphCell& g = fontCells()[static_cast<unsigned char>(ch) - kFirstChar];
  for (int y = 0; y < 256; ++y)
    for (int x = 0; x < 256; ++x) {
      const bool in = x >= g.x && x < g.x + g.width && y >= g.y && y < g.y + kLineHeight;
      sheet.rgba[(static_cast<std::size_t>(y) * 256 + x) * 4 + 3] = in ? 255 : 0;
    }
  return sheet;
}

void testText() {
  std::cout << "caption text\n";
  const CaptionParams p;
  check(p.text == "Paused", "text is Paused (display text MEASURED-FROM-REFERENCE)");
  check(p.textX == 285 && p.textY == 422, "text origin 285,422 (C2h)");
  check(textWidth(p.text, fontCells()) == 70, "Paused is 70 wide in the exe glyph table (READ 0x4BF968)");

  // Full-alpha sheet: every glyph pixel inks, so the text box is [285,355) x [422,451).
  webo::Image full = solid(256, 256, 255);
  webo::Image f = solid(640, 480, 255);
  drawCaption(f, full, fontCells(), p);
  check(at(f, 285, 422) == 255 && at(f, 354, 450) == 255, "white ink at the text box corners");
  check(at(f, 284, 430) == 127, "left of the text origin stays panel");
  check(at(f, 355, 430) == 127, "right of the 70 px run stays panel");
  check(at(f, 300, 421) == 255, "row above the panel is untouched");
  check(at(f, 300, 451) == 127, "row below the 29 px line stays panel");

  // Only 'P' inks: the string decides which cell is drawn.
  webo::Image g = solid(640, 480, 255);
  drawCaption(g, sheetWithOnly('P'), fontCells(), p);
  check(at(g, 290, 430) == 255 && at(g, 298, 450) == 255, "P cell inks at the origin (14 wide: 285..298)");
  check(at(g, 299, 430) == 127, "the next glyph (a) does not ink on a P-only sheet");
  CaptionParams q = p;
  q.text = "Xaused";
  g = solid(640, 480, 255);
  drawCaption(g, sheetWithOnly('P'), fontCells(), q);
  check(at(g, 290, 430) == 127, "a different first letter changes the ink");

  q = p;
  q.textX = 286;
  g = solid(640, 480, 255);
  drawCaption(g, sheetWithOnly('P'), fontCells(), q);
  check(at(g, 285, 430) == 127 && at(g, 299, 430) == 255, "text x moves the ink");
  q = p;
  q.textY = 423;
  g = solid(640, 480, 255);
  drawCaption(g, sheetWithOnly('P'), fontCells(), q);
  check(at(g, 290, 422) == 127 && at(g, 290, 451) == 255, "text y moves the ink");
  q = p;
  q.textColour = {0, 0, 0};
  g = solid(640, 480, 255);
  drawCaption(g, sheetWithOnly('P'), fontCells(), q);
  check(at(g, 290, 430) == 0, "text colour is its own input (black over 255 -> 0)");

  // The panel is drawn first: ink is not dimmed by it.
  webo::Image h = solid(640, 480, 0);
  drawCaption(h, sheetWithOnly('P'), fontCells(), p);
  check(at(h, 290, 430) == 255 && at(h, 310, 430) == 0, "ink over the panel, panel under it");
}

}  // namespace

int main() {
  testRect();
  testDiffuse();
  testDraw();
  testText();
  if (g_failures) {
    std::cerr << g_failures << " check(s) failed\n";
    return 1;
  }
  std::cout << "all pausecaption checks passed\n";
  return 0;
}
