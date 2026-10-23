// B1w: unit checks for the main-menu renderer, plus a real-archive check (gated on
// WEBOLUTION_GAME_DIR) that the transcribed glyph table equals the exe bytes at 0x4BF968
// and that the sheet and strings are what the renderer assumes.

#include "assets/Bitmap.h"
#include "assets/Box.h"
#include "assets/GameData.h"
#include "shell/MenuRender.h"

#include <cstdint>
#include <cstdlib>
#include <filesystem>
#include <format>
#include <fstream>
#include <iostream>
#include <iterator>
#include <string>
#include <vector>

namespace fs = std::filesystem;
using namespace webo::shell;

namespace {

int g_failures = 0;
int g_checks = 0;

void check(bool cond, std::string_view what) {
  ++g_checks;
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
  for (std::size_t i = 3; i < img.rgba.size(); i += 4) img.rgba[i] = 255;
  return img;
}

void testRules() {
  std::cout << "menu render rules\n";
  const auto& c = fontCells();
  check(halveSigned(640) == 320, "halved X word is 320");
  check(halveSigned(-3) == -1, "signed halving truncates toward zero (cdq; sub; sar)");
  check(halveSigned(3) == 1, "positive odd halves down");
  check(textWidth("Quit", c) == 41, "Quit advances 41 px (B1i replay: 15+12+6+8)");
  check(textWidth("AB", c) == textWidth("A&B", c), "& is zero-width");
  check(textWidth("Quit^rest", c) == 41, "^ ends the line");
  check(labelLeft("Quit", c, 640) == 320 - 20, "centre anchor: left = 320 - 41/2");
  check(labelLeft("Quit", c, 642) == 321 - 20, "left edge follows the X word");

  // A one-glyph sheet: 'Q' only, fully opaque; the rendered pixels land at the rule's place.
  webo::Image sheet = solid(256, 240, 255);
  webo::Image back = solid(640, 480, 128);
  MenuParams p;
  const std::vector<std::string> labels{"Q", "Q", "Q", "Q", "Q"};
  const auto out = renderMainMenu(back, sheet, labels, p, c);
  const int left = labelLeft("Q", c, p.xWord);
  auto px = [&](int x, int y) { return out.rgba[(static_cast<std::size_t>(y) * 640 + x) * 4]; };
  check(px(left + 1, 320 + 5) == 0, "row 0 glyph is black at (left, 320)");
  check(px(left + 1, 349 + 5) == 0, "row 1 glyph is one lineHeight lower (349)");
  check(px(left - 1 - p.panelPadX, 321 + 5) == 128, "pixel left of the panel is backdrop");
  check(px(left - 5, 321) == 64, "panel border is grey");
  check(px(left - p.panelPadX + 3, 321 + 5) == 224, "row 0 fill is the highlight colour");
}

std::vector<std::uint8_t> slurp(const fs::path& p) {
  std::ifstream f(p, std::ios::binary);
  return {std::istreambuf_iterator<char>(f), std::istreambuf_iterator<char>()};
}

void testRealData() {
  const char* dir = std::getenv("WEBOLUTION_GAME_DIR");
  if (!dir || !*dir) {
    std::cout << "menu render real data: SKIPPED (set WEBOLUTION_GAME_DIR to enable)\n";
    return;
  }
  std::cout << "menu render real data\n";
  const fs::path game(dir);

  // The glyph table equals the exe bytes: PE .data is raw 0xBF000 for VA 0x4BF000.
  const auto exe = slurp(game / "Lemmings Revolution.exe");
  const std::size_t off = 0xBF000 + (kFontTableVa - 0x4BF000);
  if (exe.size() < off + 4 + 3 * kGlyphCount) {
    check(false, "exe too small to hold the ^Font table");
  } else {
    const std::int32_t lh = exe[off] | exe[off + 1] << 8 | exe[off + 2] << 16 | exe[off + 3] << 24;
    check(lh == kLineHeight, "exe lineHeight at 0x4BF968 is 29");
    bool same = true;
    for (int i = 0; i < kGlyphCount; ++i) {
      const auto& c = fontCells()[static_cast<std::size_t>(i)];
      const std::size_t o = off + 4 + 3 * static_cast<std::size_t>(i);
      same = same && c.x == exe[o] && c.y == exe[o + 1] && c.width == exe[o + 2];
    }
    check(same, "transcribed cells equal the exe records for 0x20..0x7E");
  }

  auto lib = webo::AssetLibrary::open(game / "lemmings.box", game / "patch.box");
  if (!lib) {
    check(false, lib.error());
    return;
  }
  auto sheet = webo::loadTexture(*lib, "TEXTURES\\^FONT.BMP");
  check(sheet && sheet->width == 256 && sheet->height == 240 && sheet->alpha == webo::AlphaMode::AlfFile,
        "^FONT.BMP is 256x240 with an ALF plane");
  auto strings = webo::loadStrings(*lib, "UK");
  if (!strings) {
    check(false, strings.error());
    return;
  }
  check((*strings)[125] == "Start New Game" && (*strings)[126] == "Play Current Game" &&
            (*strings)[120] == "Language Selection" && (*strings)[3] == "Options" &&
            (*strings)[43] == "Quit",
        "UK strings at 125 126 120 3 43 are the five labels");
}

}  // namespace

int main() {
  testRules();
  testRealData();
  std::cout << std::format("\n{} checks, {} failures\n", g_checks, g_failures);
  return g_failures == 0 ? 0 : 1;
}
