// C2i: the pause caption panel (shell/PauseCaption.h). Each assertion pins one input
// (rectangle edge, fill colour, alpha) so a change to that input alone fails it.

#include "assets/Bitmap.h"
#include "shell/PauseCaption.h"

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

}  // namespace

int main() {
  testRect();
  testDraw();
  if (g_failures) {
    std::cerr << g_failures << " check(s) failed\n";
    return 1;
  }
  std::cout << "all pausecaption checks passed\n";
  return 0;
}
