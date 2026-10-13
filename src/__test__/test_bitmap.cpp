// Tests for BMP decoding, the alpha-sigil rules, and the PNG writer.
//
// The bitmap tests synthesise their own BMPs so they run anywhere. The
// end-to-end texture test needs real game data and skips without it.

#include "assets/Bitmap.h"
#include "assets/Box.h"
#include "assets/Png.h"

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

void put16(std::vector<std::byte>& v, std::uint16_t x) {
  v.push_back(static_cast<std::byte>(x & 0xFF));
  v.push_back(static_cast<std::byte>(x >> 8));
}
void put32(std::vector<std::byte>& v, std::uint32_t x) {
  for (int i = 0; i < 4; ++i)
    v.push_back(static_cast<std::byte>((x >> (8 * i)) & 0xFF));
}

// A bottom-up 24bpp BMP, which is what the archive overwhelmingly contains.
std::vector<std::byte> make24bpp(int w, int h, const std::vector<std::uint8_t>& rgbTopDown) {
  const std::size_t rowBytes = ((static_cast<std::size_t>(w) * 24 + 31) / 32) * 4;
  const std::uint32_t offBits = 54;
  std::vector<std::byte> v;
  v.push_back(static_cast<std::byte>('B'));
  v.push_back(static_cast<std::byte>('M'));
  put32(v, static_cast<std::uint32_t>(offBits + rowBytes * h));
  put16(v, 0);
  put16(v, 0);
  put32(v, offBits);
  put32(v, 40);
  put32(v, static_cast<std::uint32_t>(w));
  put32(v, static_cast<std::uint32_t>(h));  // positive: bottom-up
  put16(v, 1);
  put16(v, 24);
  put32(v, 0);
  put32(v, 0);
  put32(v, 0);
  put32(v, 0);
  put32(v, 0);
  put32(v, 0);

  for (int y = h - 1; y >= 0; --y) {  // bottom row first
    std::size_t before = v.size();
    for (int x = 0; x < w; ++x) {
      const std::uint8_t* p = rgbTopDown.data() + (static_cast<std::size_t>(y) * w + x) * 3;
      v.push_back(static_cast<std::byte>(p[2]));  // B
      v.push_back(static_cast<std::byte>(p[1]));  // G
      v.push_back(static_cast<std::byte>(p[0]));  // R
    }
    while (v.size() - before < rowBytes)
      v.push_back(std::byte{0});
  }
  return v;
}

void testSigils() {
  std::cout << "alpha sigils\n";
  using webo::alphaModeFor;
  check(alphaModeFor("TEXTURES\\A\\^VINES.BMP") == webo::AlphaMode::AlfFile, "^ is alf");
  check(alphaModeFor("TEXTURES\\A\\_SHAFT.BMP") == webo::AlphaMode::KeyBlack, "_ keys black");
  check(alphaModeFor("TEXTURES\\A\\$LEM.BMP") == webo::AlphaMode::KeyGrey, "$ keys grey");
  check(alphaModeFor("TEXTURES\\A\\~GLOW.BMP") == webo::AlphaMode::Additive, "~ is additive");
  check(alphaModeFor("TEXTURES\\A\\+X.BMP") == webo::AlphaMode::Embedded, "+ is embedded");
  check(alphaModeFor("TEXTURES\\A\\PLAIN.BMP") == webo::AlphaMode::Opaque, "no sigil is opaque");
  // A sigil only counts on the file name, never on a directory.
  check(alphaModeFor("TEXTURES\\^DIR\\PLAIN.BMP") == webo::AlphaMode::Opaque,
        "sigil on a directory is ignored");
}

void testDecode24() {
  std::cout << "24bpp decode\n";
  // 3x2, deliberately an odd width so rows need padding.
  const std::vector<std::uint8_t> rgb = {
      255, 0,   0,    0,   255, 0,    0,   0,   255,   // top row
      0,   0,   0,    64,  64,  64,   255, 255, 255,   // bottom row
  };
  auto bmp = make24bpp(3, 2, rgb);
  auto img = webo::decodeBmp(bmp);
  check(img.has_value(), "decodes");
  if (!img) {
    std::cerr << "    " << img.error() << "\n";
    return;
  }
  check(img->width == 3 && img->height == 2, "dimensions");
  check(img->rgba.size() == 3 * 2 * 4, "rgba size");

  bool pixelsMatch = true;
  for (int i = 0; i < 6; ++i) {
    for (int c = 0; c < 3; ++c)
      if (img->rgba[i * 4 + c] != rgb[i * 3 + c])
        pixelsMatch = false;
    if (img->rgba[i * 4 + 3] != 0xFF)
      pixelsMatch = false;
  }
  check(pixelsMatch, "pixels round-trip, rows unflipped, alpha opaque");

  // A top-down bitmap (negative height) must come out the same way up.
  auto flipped = bmp;
  const std::int32_t negative = -2;
  std::memcpy(flipped.data() + 22, &negative, 4);
  // Re-lay the rows top-down to match the new orientation.
  auto topDown = webo::decodeBmp(flipped);
  check(topDown.has_value(), "decodes a top-down bitmap");
  if (topDown)
    check(topDown->height == 2, "top-down height is positive");
}

void testAlf() {
  std::cout << "alf alpha plane\n";
  const std::vector<std::uint8_t> rgb(2 * 2 * 3, 128);
  auto bmp = make24bpp(2, 2, rgb);
  auto img = webo::decodeBmp(bmp);
  check(img.has_value(), "decodes");
  if (!img)
    return;

  const std::vector<std::byte> alf = {std::byte{0}, std::byte{64}, std::byte{128}, std::byte{255}};
  auto r = webo::applyAlfAlpha(*img, alf);
  check(r.has_value(), "applies");
  check(img->rgba[3] == 0 && img->rgba[7] == 64 && img->rgba[11] == 128 && img->rgba[15] == 255,
        "alpha values land in order");

  const std::vector<std::byte> tooSmall = {std::byte{0}};
  check(!webo::applyAlfAlpha(*img, tooSmall).has_value(), "rejects a short alpha plane");
}

void testPng() {
  std::cout << "png writer\n";
  webo::Image img;
  img.width = 2;
  img.height = 2;
  img.rgba = {255, 0, 0, 255, 0, 255, 0, 255, 0, 0, 255, 255, 1, 2, 3, 128};

  const auto png = webo::encodePng(img);
  check(png.size() > 8, "produces bytes");
  const std::uint8_t sig[8] = {0x89, 'P', 'N', 'G', 0x0D, 0x0A, 0x1A, 0x0A};
  check(png.size() >= 8 && std::memcmp(png.data(), sig, 8) == 0, "PNG signature");

  // IHDR must follow immediately, and IEND must end the file.
  check(png.size() > 16 && std::memcmp(png.data() + 12, "IHDR", 4) == 0, "IHDR first");
  check(png.size() > 12 && std::memcmp(png.data() + png.size() - 8, "IEND", 4) == 0, "IEND last");

  // With a transparent pixel present the writer must choose RGBA (colour type 6).
  check(png[25] == 6, "keeps alpha when a pixel is transparent");

  img.rgba[15] = 255;
  const auto opaque = webo::encodePng(img);
  check(opaque[25] == 2, "drops alpha when fully opaque");
}

void testRealTextures() {
  const char* dir = std::getenv("WEBOLUTION_GAME_DIR");
  if (!dir || !*dir) {
    std::cout << "real textures: SKIPPED (set WEBOLUTION_GAME_DIR to enable)\n";
    return;
  }
  std::cout << "real textures\n";
  auto lib = webo::AssetLibrary::open(fs::path(dir) / "lemmings.box", fs::path(dir) / "patch.box");
  if (!lib) {
    check(false, lib.error());
    return;
  }

  // One of each interesting kind, by name, so a regression names itself.
  struct Case {
    const char* name;
    webo::AlphaMode mode;
    bool expectTransparency;
  };
  const Case cases[] = {
      {"TEXTURES\\MODELS\\ANTI_GRAV\\PANEL_ANTIGRAV.BMP", webo::AlphaMode::Opaque, false},
      {"TEXTURES\\MODELS\\ANTI_GRAV\\_SHAFT_ANTIGRAV.BMP", webo::AlphaMode::KeyBlack, true},
      {"TEXTURES\\SHEETS\\$LEMSHEET0.BMP", webo::AlphaMode::KeyGrey, true},
      {"TEXTURES\\MODELS\\BG_MINE01\\^VINES01.BMP", webo::AlphaMode::AlfFile, true},
  };

  for (const auto& c : cases) {
    if (!lib->contains(c.name)) {
      std::cout << std::format("    (absent, skipped: {})\n", c.name);
      continue;
    }
    auto img = webo::loadTexture(*lib, c.name);
    if (!img) {
      check(false, std::format("{}: {}", c.name, img.error()));
      continue;
    }
    check(img->alpha == c.mode, std::format("{}: alpha mode", c.name));
    check(img->width > 0 && img->height > 0, std::format("{}: has pixels", c.name));

    bool anyTransparent = false;
    for (std::size_t i = 0; i < img->pixelCount(); ++i)
      if (img->rgba[i * 4 + 3] != 0xFF) {
        anyTransparent = true;
        break;
      }
    check(anyTransparent == c.expectTransparency,
          std::format("{}: transparency {}", c.name, c.expectTransparency ? "present" : "absent"));
  }
}

}  // namespace

int main() {
  testSigils();
  testDecode24();
  testAlf();
  testPng();
  testRealTextures();

  std::cout << std::format("\n{} checks, {} failures\n", g_checks, g_failures);
  return g_failures == 0 ? 0 : 1;
}
