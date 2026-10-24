// C2g: layout checks for the Out / Saved counters and the timer (shell/HudText.h). Each
// assertion pins one input (digit string, right-end anchor, y row, timer split, colour) so a
// change to that input alone fails it. The ink check draws on a blank frame with a synthetic
// opaque sheet and reads back where the glyph quads landed.

#include "assets/Bitmap.h"
#include "shell/HudText.h"

#include <cstdint>
#include <iostream>
#include <string>
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

webo::Image blank(int w, int h, std::uint8_t v, std::uint8_t a) {
  webo::Image img;
  img.width = w;
  img.height = h;
  img.rgba.assign(static_cast<std::size_t>(w) * h * 4, v);
  for (std::size_t i = 3; i < img.rgba.size(); i += 4) img.rgba[i] = a;
  return img;
}

void testText() {
  std::cout << "hud text\n";
  check(counterText(4) == "4" && counterText(0) == "0" && counterText(12) == "12", "%d of the count");
  check(savedText(0) == "0", "Saved 0 draws '0'");
  check(savedText(0x10005) == "5", "Saved goes through movsx word (0x44D316)");
  check(savedText(-3) == "-3", "Saved is signed");
  check(timerText(286) == "04:46", "286 s is 04:46 (idiv 60)");
  check(timerText(300) == "05:00", "300 s is 05:00 (seconds < 10 uses the '0%d' format)");
  check(timerText(305) == "05:05", "305 s is 05:05");
  check(timerText(59) == "00:59", "59 s is 00:59");
  check(timerText(60) == "01:00", "60 s is 01:00");
  check(timerText(-5) == "00:00", "negative is stored back as 0");
  check(timerText(286, 100) != "04:46", "the split is 60, not 100");
}

void testLayout() {
  std::cout << "hud layout\n";
  const auto& c = fontCells();
  HudValues v;
  v.out = 4;
  v.saved = 0;
  v.saveTarget = 1;
  v.secondsRemaining = 286;
  const HudParams p;
  const auto lines = layoutHud(v, p, c);
  check(lines.size() == 3, "Out, Saved, timer");
  check(lines[0].text == "4" && lines[1].text == "0" && lines[2].text == "04:46", "digit strings");
  check(lines[0].y == 15, "Out row y 15 (0xF)");
  check(lines[1].y == 50, "Saved row y 50 (0x32)");
  check(lines[2].y == 430 && lines[2].x == 560, "timer at 560,430");
  check(lines[0].x + textWidth(lines[0].text, c) == 620, "Out right end 620");
  check(lines[1].x + textWidth(lines[1].text, c) == 620, "Saved right end 620");
  check(lines[0].x == 620 - c['4' - kFirstChar].width, "Out left = 620 - width('4')");

  // two-digit Out moves the LEFT edge, not the right end
  v.out = 12;
  const auto wide = layoutHud(v, p, c);
  check(wide[0].x + textWidth("12", c) == 620 && wide[0].x < lines[0].x, "right end holds as digits grow");

  // Saved colour: white below the target, 255,63,63 at or above (signed word compare)
  check(lines[1].colour == Rgb{255, 255, 255}, "Saved 0 < target 1 is white");
  v.saved = 1;
  check(layoutHud(v, p, c)[1].colour == (Rgb{255, 63, 63}), "Saved at target is 255,63,63");
  check(lines[0].colour == Rgb{255, 255, 255} && lines[2].colour == Rgb{255, 255, 255},
        "Out and timer are the UNREAD-default white");

  // each input alone moves the result
  HudParams q = p;
  q.counterRightEnd = 619;
  check(layoutHud(v, q, c)[0].x != layoutHud(v, p, c)[0].x, "anchor input moves the digits");
  q = p;
  q.savedY = 45;
  check(layoutHud(v, q, c)[1].y == 45, "Saved y input moves the row");
}

void testInk() {
  std::cout << "hud ink\n";
  const auto& c = fontCells();
  // An opaque sheet: every glyph cell is solid, so the drawn pixels are exactly the quads.
  const webo::Image sheet = blank(256, 240, 255, 255);
  webo::Image frame = blank(640, 480, 0, 255);
  HudValues v;
  v.out = 4;
  v.saved = 0;
  v.saveTarget = 1;
  v.secondsRemaining = 286;
  const auto lines = layoutHud(v, HudParams{}, c);
  drawHud(frame, sheet, lines, c);
  auto lit = [&](int x, int y) { return frame.rgba[(static_cast<std::size_t>(y) * 640 + x) * 4] != 0; };
  const HudBox out = lineBox(lines[0], c), saved = lineBox(lines[1], c), timer = lineBox(lines[2], c);
  check(lit(out.x0, 15) && lit(out.x1 - 1, 15 + 28) && !lit(out.x1, 15) && !lit(out.x0, 14),
        "Out quad is [x, x+w) x [15, 44)");
  check(out.x1 == 620, "Out box right edge 620");
  check(lit(saved.x0, 50) && !lit(saved.x0, 49), "Saved quad starts at y 50");
  check(lit(timer.x0, 430) && lit(timer.x1 - 1, 430 + 28) && !lit(timer.x1, 430) && !lit(559, 430),
        "timer quad starts at 560,430");
  check(timer.x1 - timer.x0 == textWidth("04:46", c), "timer width is the glyph sum");
}

}  // namespace

int main() {
  testText();
  testLayout();
  testInk();
  if (g_failures) {
    std::cerr << g_failures << " check(s) failed\n";
    return 1;
  }
  std::cout << "all hudtext checks passed\n";
  return 0;
}
