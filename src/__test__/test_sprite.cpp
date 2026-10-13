// Tests for the .SPR sprite sheet format.
//
// The format is undocumented, so the strongest checks are the internal
// consistencies that pin down the field order: every frame's width and height
// must equal its rectangle's extents, and dividing each integer edge by its
// normalised twin must recover the same sheet size for all four edges. Those
// hold only for the correct reading, so they catch a transposed or shifted
// layout immediately.

#include "assets/Bitmap.h"
#include "assets/Box.h"
#include "assets/Sprite.h"

#include <cmath>
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <format>
#include <iostream>
#include <string>
#include <vector>

namespace fs = std::filesystem;

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

void put32(std::vector<std::byte>& v, std::uint32_t x) {
  for (int i = 0; i < 4; ++i)
    v.push_back(static_cast<std::byte>((x >> (8 * i)) & 0xFF));
}
void putF64(std::vector<std::byte>& v, double d) {
  const auto* p = reinterpret_cast<const std::byte*>(&d);
  v.insert(v.end(), p, p + 8);
}
void putStr(std::vector<std::byte>& v, std::string_view s) {
  put32(v, static_cast<std::uint32_t>(s.size()));
  for (char c : s)
    v.push_back(static_cast<std::byte>(c));
}

void testSheetAssetName() {
  std::cout << "sheet asset names\n";
  check(webo::sheetAssetName("Sheets\\$LemSheet8") == "TEXTURES\\SHEETS\\$LEMSHEET8.BMP",
        "prefix, upper-case and extension");
  check(webo::sheetAssetName("Sheets/_Bubbles0") == "TEXTURES\\SHEETS\\_BUBBLES0.BMP",
        "forward slashes normalise too");
}

void testSynthetic() {
  std::cout << "synthetic sheet\n";
  // One sprite, one animation, one frame, on a 256x251 sheet.
  const std::uint32_t bottom = 144, top = 108, left = 41, right = 64;
  const double sheetW = 256.0, sheetH = 251.0;

  std::vector<std::byte> d;
  put32(d, 1);  // sprites
  put32(d, 1);  // animations
  put32(d, 1);  // frames
  put32(d, bottom);
  put32(d, top);
  put32(d, left);
  put32(d, right);
  putF64(d, bottom / sheetH);
  putF64(d, top / sheetH);
  putF64(d, left / sheetW);
  putF64(d, right / sheetW);
  put32(d, 11);
  put32(d, 1);
  put32(d, right - left);
  put32(d, bottom - top);
  putStr(d, "Sheets\\$LemSheet8");
  putStr(d, "walk");
  putF64(d, 0.5);
  put32(d, 0);  // no palette

  auto sheet = webo::parseSpriteSheet(d);
  check(sheet.has_value(), "parses");
  if (!sheet) {
    std::cerr << "    " << sheet.error() << "\n";
    return;
  }
  check(sheet->sprites.size() == 1, "one sprite");
  check(sheet->sprites[0].name == "walk", "sprite name");
  check(sheet->sprites[0].rate == 0.5, "rate");
  check(sheet->frameCount() == 1, "one frame");
  check(sheet->palette.empty(), "no palette");
  check(sheet->find("walk") != nullptr, "lookup by name");
  check(sheet->find("nope") == nullptr, "missing lookup is null");

  const auto& f = sheet->sprites[0].animations[0].frames[0];
  check(f.left == left && f.right == right && f.top == top && f.bottom == bottom, "rectangle");
  check(f.width == right - left, "width matches the rectangle");
  check(f.height == bottom - top, "height matches the rectangle");
  check(f.sheetWidth() == 256, std::format("recovers sheet width (got {})", f.sheetWidth()));
  check(f.sheetHeight() == 251, std::format("recovers sheet height (got {})", f.sheetHeight()));

  // Trailing bytes mean the layout is wrong; that must be reported.
  auto extra = d;
  extra.push_back(std::byte{0});
  check(!webo::parseSpriteSheet(extra).has_value(), "rejects leftover bytes");

  std::vector<std::byte> truncated(d.begin(), d.begin() + 20);
  check(!webo::parseSpriteSheet(truncated).has_value(), "rejects a truncated file");
}

void testPalette() {
  std::cout << "palette block\n";
  std::vector<std::byte> d;
  put32(d, 0);  // no sprites
  put32(d, 1);  // one palette block
  put32(d, 0);  // block header
  for (int i = 0; i < 48; ++i)
    putF64(d, static_cast<double>(i));

  auto sheet = webo::parseSpriteSheet(d);
  check(sheet.has_value(), "parses");
  if (!sheet)
    return;
  check(sheet->palette.size() == 16, "16 colours");
  check(sheet->palette[0][0] == 0 && sheet->palette[0][2] == 2, "first colour");
  check(sheet->palette[15][2] == 47, "last colour");
}

void testRealSheets() {
  const char* dir = std::getenv("WEBOLUTION_GAME_DIR");
  if (!dir || !*dir) {
    std::cout << "real sheets: SKIPPED (set WEBOLUTION_GAME_DIR to enable)\n";
    return;
  }
  std::cout << "real sheets\n";
  auto lib = webo::AssetLibrary::open(fs::path(dir) / "lemmings.box", fs::path(dir) / "patch.box");
  if (!lib) {
    check(false, lib.error());
    return;
  }

  const auto names = webo::spriteSheetNames(*lib);
  check(names.size() == 19, std::format("19 sprite sheets (got {})", names.size()));

  std::size_t frames = 0, dimMismatch = 0, sheetMismatch = 0, missing = 0;
  for (const auto& n : names) {
    auto sheet = webo::loadSpriteSheet(*lib, n);
    if (!sheet) {
      check(false, std::format("{}: {}", n, sheet.error()));
      continue;
    }
    for (const auto& s : sheet->sprites)
      for (const auto& a : s.animations)
        for (const auto& f : a.frames) {
          ++frames;
          if (f.width != f.right - f.left || f.height != f.bottom - f.top)
            ++dimMismatch;
          // All four edges must imply the same sheet size.
          const std::uint32_t w1 = f.sheetWidth();
          const std::uint32_t h1 = f.sheetHeight();
          if (f.uLeft > 0 && w1 &&
              std::llabs(static_cast<long long>(std::lround(f.left / f.uLeft)) - w1) > 1)
            ++sheetMismatch;
          if (f.uTop > 0 && h1 &&
              std::llabs(static_cast<long long>(std::lround(f.top / f.uTop)) - h1) > 1)
            ++sheetMismatch;
          if (!lib->contains(webo::sheetAssetName(f.sheet)))
            ++missing;
        }
  }

  check(frames > 7000, std::format("thousands of frames ({})", frames));
  check(dimMismatch == 0, std::format("width/height match the rectangle ({} did not)", dimMismatch));
  check(sheetMismatch == 0,
        std::format("all four edges imply one sheet size ({} did not)", sheetMismatch));
  check(missing == 0, std::format("every referenced sheet exists ({} did not)", missing));

  // The lemming sheet is the one the simulation will lean on most.
  auto lem = webo::loadSpriteSheet(*lib, "SPRITES\\LEMMINGSLO.SPR");
  check(lem.has_value(), "LEMMINGSLO loads");
  if (lem) {
    check(lem->sprites.size() == 15, std::format("15 sprites (got {})", lem->sprites.size()));
    for (const char* skill : {"walk", "bash", "build", "climb", "dig", "mine", "float", "explode"})
      check(lem->find(skill) != nullptr, std::format("has '{}'", skill));
    check(lem->palette.size() == 16, "carries the 16-colour palette");
    if (lem->palette.size() == 16)
      check(lem->palette[0][0] == 64 && lem->palette[0][1] == 64 && lem->palette[0][2] == 64,
            "first palette entry is 404040, the '$' key colour");

    // Frame rectangles must lie inside the sheet they name.
    std::size_t outside = 0;
    const webo::Sprite* walk = lem->find("walk");
    if (walk)
      for (const auto& a : walk->animations)
        for (const auto& f : a.frames) {
          auto img = webo::loadTexture(*lib, webo::sheetAssetName(f.sheet));
          if (!img)
            continue;
          if (static_cast<int>(f.right) > img->width || static_cast<int>(f.bottom) > img->height)
            ++outside;
        }
    check(outside == 0, std::format("walk frames fit inside their sheets ({} did not)", outside));
  }

  std::cout << std::format("    {} frames across {} files\n", frames, names.size());
}

}  // namespace

int main() {
  testSheetAssetName();
  testSynthetic();
  testPalette();
  testRealSheets();

  std::cout << std::format("\n{} checks, {} failures\n", g_checks, g_failures);
  return g_failures == 0 ? 0 : 1;
}
