// Synthetic BMPs and archives exercise decoding, alpha sigils, PNG output and loadTexture.
// Retail checks require game data.

#include "assets/Bitmap.h"
#include "assets/Box.h"
#include "assets/Png.h"

#include <cstdlib>
#include <chrono>
#include <cstring>
#include <filesystem>
#include <format>
#include <fstream>
#include <iostream>
#include <stdexcept>
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
  check(alphaModeFor("TEXTURES\\A\\+X.BMP") == webo::AlphaMode::Embedded, "+ retains legacy embedded classification");
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
  // Store the top row first for negative BMP height.
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

using Bytes = std::vector<std::byte>;

Bytes bytes(std::initializer_list<std::uint8_t> values) {
  Bytes result;
  for (auto value : values)
    result.push_back(static_cast<std::byte>(value));
  return result;
}

// Literal bottom-up 3x2 BMP with nine pixel bytes and three nonzero padding bytes per row.
Bytes oddWidthBmp() {
  return bytes({
      0x42, 0x4d, 78, 0, 0, 0, 0, 0, 0, 0, 54, 0, 0, 0,
      40, 0, 0, 0, 3, 0, 0, 0, 2, 0, 0, 0, 1, 0, 24, 0,
      0, 0, 0, 0, 24, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 64, 64, 64, 255, 255, 255, 0xa1, 0xb2, 0xc3,
      0, 0, 255, 0, 255, 0, 255, 0, 0, 0xd4, 0xe5, 0xf6,
  });
}

// Literal 2x2 bottom-up BGRA pixels. Its BMP alpha is deliberately unrelated
// to the sibling ALF; direct decodeBmp must continue to preserve these bytes.
Bytes alpha32Bmp() {
  return bytes({
      0x42, 0x4d, 70, 0, 0, 0, 0, 0, 0, 0, 54, 0, 0, 0,
      40, 0, 0, 0, 2, 0, 0, 0, 2, 0, 0, 0, 1, 0, 32, 0,
      0, 0, 0, 0, 16, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0,
      90, 80, 70, 9, 120, 110, 100, 10,
      30, 20, 10, 7, 60, 50, 40, 8,
  });
}

// Encode a single literal run in codec 4's zero-table unary mode.
Bytes literalBlock(std::span<const std::byte> input) {
  Bytes result(3, std::byte{0});  // three unary tables
  unsigned bit = 0;
  const auto putBit = [&](unsigned value) {
    if (bit == 0)
      result.push_back(std::byte{0});
    if (value)
      result.back() |= static_cast<std::byte>(1u << (7 - bit));
    bit = (bit + 1) % 8;
  };
  const unsigned encoded = static_cast<unsigned>(input.size()) + 1;
  unsigned width = 0;
  for (unsigned n = encoded; n > 1; n >>= 1)
    ++width;
  for (unsigned i = 0; i < width; ++i)
    putBit(1);
  putBit(0);
  for (unsigned i = width; i > 0; --i)
    putBit((encoded >> (i - 1)) & 1u);
  for (auto value : input)
    for (unsigned i = 8; i > 0; --i)
      putBit((std::to_integer<unsigned>(value) >> (i - 1)) & 1u);
  return result;
}

void appendBlock(Bytes& envelope, std::span<const std::byte> input, unsigned codec) {
  const Bytes payload = codec == 4 ? literalBlock(input) : Bytes(input.begin(), input.end());
  put16(envelope, static_cast<std::uint16_t>(payload.size() + 5));
  put16(envelope, static_cast<std::uint16_t>(input.size()));
  envelope.push_back(static_cast<std::byte>(codec));
  envelope.insert(envelope.end(), payload.begin(), payload.end());
}

Bytes envelope(const Bytes& input, unsigned codec) {
  Bytes result{std::byte{1}};
  appendBlock(result, input, codec);
  return result;
}

// Minimal BOX writer: offsets are absolute, each table has its own count.
bool writeBox(const fs::path& path, const std::vector<std::pair<std::string, Bytes>>& entries) {
  Bytes names;
  for (const auto& [name, payload] : entries) {
    put32(names, static_cast<std::uint32_t>(name.size()));
    for (char c : name)
      names.push_back(static_cast<std::byte>(c));
  }
  const auto count = static_cast<std::uint32_t>(entries.size());
  Bytes output = bytes({'L', 'E', 'M', 'B', 'O', 'X'});
  put32(output, count);
  put32(output, static_cast<std::uint32_t>(names.size()));
  output.insert(output.end(), names.begin(), names.end());
  put32(output, count);
  auto offset = static_cast<std::uint32_t>(22 + names.size() + count * 8);
  for (const auto& [name, payload] : entries) {
    put32(output, offset);
    offset += static_cast<std::uint32_t>(payload.size());
  }
  put32(output, count);
  for (const auto& [name, payload] : entries)
    put32(output, static_cast<std::uint32_t>(payload.size()));
  for (const auto& [name, payload] : entries)
    output.insert(output.end(), payload.begin(), payload.end());
  std::ofstream file(path, std::ios::binary);
  file.write(reinterpret_cast<const char*>(output.data()),
             static_cast<std::streamsize>(output.size()));
  file.close();
  return static_cast<bool>(file);
}

fs::path freshBitmapScratch() {
  const fs::path parent = WEBOLUTION_BITMAP_TEST_DIR;
  fs::create_directories(parent);
  const auto stamp = std::chrono::steady_clock::now().time_since_epoch().count();
  for (unsigned attempt = 0; attempt < 100; ++attempt) {
    const auto path = parent / std::format("run-{}-{}", stamp, attempt);
    if (fs::create_directory(path))
      return path;
  }
  throw std::runtime_error("cannot create a fresh bitmap test directory");
}

void testArchiveTextures() {
  std::cout << "synthetic BOX-backed textures\n";
  const auto scratch = freshBitmapScratch();
  std::cout << "    scratch: " << scratch.string() << '\n';
  const auto bmp = oddWidthBmp();
  const std::vector<std::uint8_t> expected = {
      255, 0, 0, 255, 0, 255, 0, 255, 0, 0, 255, 255,
      0, 0, 0, 255, 64, 64, 64, 255, 255, 255, 255, 255,
  };
  Bytes multiple{std::byte{2}};
  appendBlock(multiple, std::span(bmp).first(35), 3);
  appendBlock(multiple, std::span(bmp).subspan(35), 4);
  // Looks like another invalid block, but lies beyond the declared count.
  const auto trailing = bytes({255, 0, 0, 0, 255, 123});
  multiple.insert(multiple.end(), trailing.begin(), trailing.end());
  auto truncatedBmp = bmp;
  truncatedBmp.pop_back();  // BM/header intact, final padded row incomplete
  const auto basePath = scratch / "base.box";
  const auto patchPath = scratch / "patch.box";
  const auto baseBmp = make24bpp(1, 1, {11, 22, 33});
  check(writeBox(basePath, {
      {"TEXTURES\\RAW.BMP", bmp},
      {"TEXTURES\\CODEC3.BMP", envelope(bmp, 3)},
      {"TEXTURES\\CODEC4.BMP", envelope(bmp, 4)},
      {"TEXTURES\\MULTI.BMP", multiple},
      {"TEXTURES\\OVERRIDE.BMP", envelope(baseBmp, 3)},
      {"TEXTURES\\^CARET.BMP", envelope(bmp, 4)},
      {"TEXTURES\\^CARET.ALF", bytes({0, 17, 64, 128, 200, 255})},
      {"TEXTURES\\BAD_HEADER.BMP", bytes({1, 5})},
      {"TEXTURES\\BAD_CODEC.BMP", bytes({1, 5, 0, 1, 0, 9})},
      {"TEXTURES\\BAD_PACKED.BMP", bytes({1, 8, 0, 1, 0, 4, 0, 0, 0})},
      {"TEXTURES\\TRUNCATED.BMP", truncatedBmp},
      {"TEXTURES\\PACKED_TRUNCATED.BMP", envelope(truncatedBmp, 3)},
  }), "writes synthetic base BOX");
  check(writeBox(patchPath, {{"TEXTURES\\OVERRIDE.BMP", envelope(bmp, 4)}}),
        "writes synthetic patch BOX");
  auto lib = webo::AssetLibrary::open(basePath, patchPath);
  check(lib.has_value(), "opens synthetic texture library");
  if (!lib)
    return;
  check(lib->base().validate().ok && lib->patch() && lib->patch()->validate().ok,
        "synthetic BOX tables and payloads validate");
  for (const auto name : {"TEXTURES\\RAW.BMP", "TEXTURES\\CODEC3.BMP",
                          "TEXTURES\\CODEC4.BMP", "TEXTURES\\MULTI.BMP",
                          "TEXTURES\\OVERRIDE.BMP"}) {
    auto img = webo::loadTexture(*lib, name);
    check(img.has_value(), std::format("{}: loadTexture succeeds", name));
    if (!img) {
      std::cerr << "    " << img.error() << '\n';
      continue;
    }
    check(img->width == 3 && img->height == 2, std::format("{}: literal 3x2 dimensions", name));
    check(img->rgba == expected, std::format("{}: all RGBA pixels, row order and padding", name));
    check(img->alpha == webo::AlphaMode::Opaque, std::format("{}: opaque mode", name));
  }
  auto baseOnly = webo::AssetLibrary::open(basePath);
  check(baseOnly.has_value(), "opens base without patch");
  if (baseOnly) {
    auto img = webo::loadTexture(*baseOnly, "TEXTURES\\OVERRIDE.BMP");
    check(img && img->width == 1 && img->height == 1 &&
              img->rgba == std::vector<std::uint8_t>{11, 22, 33, 255},
          "base-only override has distinct literal dimensions and pixels");
  }
  auto caret = webo::loadTexture(*lib, "TEXTURES\\^CARET.BMP");
  check(caret.has_value(), "caret texture loads through recovered envelope boundary");
  if (caret) {
    check(caret->width == 3 && caret->height == 2, "caret literal 3x2 dimensions");
    check(caret->alpha == webo::AlphaMode::AlfFile, "caret selects ALF mode");
    check(caret->rgba == std::vector<std::uint8_t>{
        255, 0, 0, 0, 0, 255, 0, 17, 0, 0, 255, 64,
        0, 0, 0, 128, 64, 64, 64, 200, 255, 255, 255, 255,
    }, "caret complete RGBA includes literal top-down ALF plane");
  }
  for (const auto name : {"TEXTURES\\BAD_HEADER.BMP", "TEXTURES\\BAD_CODEC.BMP",
                          "TEXTURES\\BAD_PACKED.BMP", "TEXTURES\\TRUNCATED.BMP",
                          "TEXTURES\\PACKED_TRUNCATED.BMP"}) {
    try {
      const auto result = webo::loadTexture(*lib, name);
      check(!result, std::format("{}: public API rejects malformed data", name));
      check(!result && result.error().starts_with(std::format("{}: ", name)),
            std::format("{}: expected error includes asset name", name));
    } catch (const std::invalid_argument& error) {
      check(false, std::format("{}: decoder exception escaped: {}", name, error.what()));
    }
  }
}

void testPlusTextures() {
  std::cout << "plus textures through public archive loader\n";
  const auto scratch = freshBitmapScratch();
  std::cout << "    scratch: " << scratch.string() << '\n';
  const auto bmp = oddWidthBmp();
  const auto bmp32 = alpha32Bmp();
  const auto alf = bytes({0, 17, 64, 128, 200, 255});
  const auto alf32 = bytes({255, 0, 128, 64});
  const std::vector<std::uint8_t> expected = {
      255, 0, 0, 0, 0, 255, 0, 17, 0, 0, 255, 64,
      0, 0, 0, 128, 64, 64, 64, 200, 255, 255, 255, 255,
  };
  const std::vector<std::uint8_t> expected32 = {
      10, 20, 30, 255, 40, 50, 60, 0,
      70, 80, 90, 128, 100, 110, 120, 64,
  };
  const std::vector<std::uint8_t> bmpAlpha = {
      10, 20, 30, 7, 40, 50, 60, 8,
      70, 80, 90, 9, 100, 110, 120, 10,
  };
  const auto basePath = scratch / "base.box";
  const auto patchPath = scratch / "patch.box";
  check(writeBox(basePath, {
      {"TEXTURES\\+RAW.BMP", bmp}, {"TEXTURES\\+RAW.ALF", alf},
      {"TEXTURES\\+CODEC3.BMP", envelope(bmp, 3)}, {"TEXTURES\\+CODEC3.ALF", alf},
      {"TEXTURES\\+CODEC4.BMP", envelope(bmp, 4)}, {"TEXTURES\\+CODEC4.ALF", alf},
      {"TEXTURES\\+32.BMP", bmp32}, {"TEXTURES\\+32.ALF", alf32},
      {"TEXTURES\\+PACKED32.BMP", envelope(bmp32, 4)}, {"TEXTURES\\+PACKED32.ALF", alf32},
      {"TEXTURES\\+MISSING.BMP", bmp},
      {"TEXTURES\\+SHORT.BMP", envelope(bmp, 3)}, {"TEXTURES\\+SHORT.ALF", bytes({0, 17})},
      {"TEXTURES\\+EXTRA.BMP", bmp}, {"TEXTURES\\+EXTRA.ALF", bytes({0, 17, 64, 128, 200, 255, 99, 88})},
      {"TEXTURES\\+PATCH_BMP.BMP", make24bpp(1, 1, {11, 22, 33})},
      {"TEXTURES\\+PATCH_BMP.ALF", alf},
      {"TEXTURES\\+PATCH_ALF.BMP", envelope(bmp, 4)},
      {"TEXTURES\\+PATCH_ALF.ALF", bytes({255, 255, 255, 255, 255, 255})},
      {"TEXTURES\\^32.BMP", bmp32}, {"TEXTURES\\^32.ALF", alf32},
      {"TEXTURES\\PLAIN32.BMP", bmp32},
      {"TEXTURES\\_KEY.BMP", bmp}, {"TEXTURES\\$KEY.BMP", envelope(bmp, 3)},
      {"TEXTURES\\~ADD.BMP", envelope(bmp, 4)},
  }), "writes plus texture base BOX");
  check(writeBox(patchPath, {
      {"TEXTURES\\+PATCH_BMP.BMP", envelope(bmp, 3)},
      {"TEXTURES\\+PATCH_ALF.ALF", alf},
  }), "writes independently selected patch BMP and ALF entries");
  auto lib = webo::AssetLibrary::open(basePath, patchPath);
  check(lib.has_value(), "opens plus texture library");
  if (!lib) return;
  check(lib->base().validate().ok && lib->patch() && lib->patch()->validate().ok,
        "plus BOX tables and payloads validate");
  for (const auto name : {"TEXTURES\\+RAW.BMP", "TEXTURES\\+CODEC3.BMP", "TEXTURES\\+CODEC4.BMP",
                          "TEXTURES\\+EXTRA.BMP", "TEXTURES\\+PATCH_BMP.BMP", "TEXTURES\\+PATCH_ALF.BMP"}) {
    const auto image = webo::loadTexture(*lib, name);
    check(image.has_value(), std::format("{}: loads", name));
    if (!image) continue;
    check(image->width == 3 && image->height == 2 && image->rgba == expected,
          std::format("{}: complete literal RGBA, padded bottom-up BMP / top-down ALF", name));
    check(image->alpha == webo::AlphaMode::Embedded && webo::alphaModeName(image->alpha) == "embedded",
          std::format("{}: legacy public mode and string preserved", name));
  }
  for (const auto name : {"TEXTURES\\+32.BMP", "TEXTURES\\+PACKED32.BMP", "TEXTURES\\^32.BMP"}) {
    const auto image = webo::loadTexture(*lib, name);
    check(image && image->width == 2 && image->height == 2 && image->rgba == expected32,
          std::format("{}: sibling ALF replaces all BMP alpha", name));
    check(image && image->alpha == (name[9] == '^' ? webo::AlphaMode::AlfFile : webo::AlphaMode::Embedded),
          std::format("{}: public mode preserved", name));
  }
  const auto missing = webo::loadTexture(*lib, "TEXTURES\\+MISSING.BMP");
  check(!missing && missing.error().starts_with("TEXTURES\\+MISSING.BMP: no alpha plane (") &&
        missing.error().find("TEXTURES\\+MISSING.ALF") != std::string::npos,
        "missing plus ALF returns existing asset-qualified expected error");
  const auto shortAlf = webo::loadTexture(*lib, "TEXTURES\\+SHORT.BMP");
  check(!shortAlf && shortAlf.error() == "TEXTURES\\+SHORT.BMP: alpha plane has 2 bytes for 6 pixels",
        "short plus ALF returns exact existing expected error");
  const auto direct = webo::decodeBmp(bmp32);
  const auto plain = webo::loadTexture(*lib, "TEXTURES\\PLAIN32.BMP");
  check(direct && direct->width == 2 && direct->height == 2 && direct->rgba == bmpAlpha,
        "direct 32-bit BMP decoding retains complete BMP alpha");
  check(plain && plain->width == 2 && plain->height == 2 && plain->rgba == bmpAlpha &&
        plain->alpha == webo::AlphaMode::Opaque, "plain 32-bit loader retains BMP alpha and opaque classification");
  for (const auto sigil : {'_', '$', '~'}) {
    const auto name = std::format("TEXTURES\\{}{}.BMP", sigil, sigil == '~' ? "ADD" : "KEY");
    const auto image = webo::loadTexture(*lib, name);
    // Independent expectations for keying and additive alpha.
    const std::vector<std::uint8_t> control = {
        255, 0, 0, 255, 0, 255, 0, 255, 0, 0, 255, 255,
        0, 0, 0, static_cast<std::uint8_t>(sigil == '_' ? 0 : 255),
        64, 64, 64, static_cast<std::uint8_t>(sigil == '$' ? 0 : 255), 255, 255, 255, 255,
    };
    check(image && image->width == 3 && image->height == 2 && image->rgba == control &&
          image->alpha == webo::alphaModeFor(name), std::format("{}: unchanged complete pixels and mode", name));
  }
  // The loader's ALF adaptation must not mutate selected archive bytes.
  check(lib->read("TEXTURES\\+RAW.BMP") == std::expected<Bytes, std::string>(bmp) &&
        lib->read("TEXTURES\\+RAW.ALF") == std::expected<Bytes, std::string>(alf) &&
        lib->read("TEXTURES\\+32.BMP") == std::expected<Bytes, std::string>(bmp32) &&
        lib->read("TEXTURES\\+32.ALF") == std::expected<Bytes, std::string>(alf32),
        "public loading preserves BMP and ALF inputs");
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

  std::size_t plusCount = 0;
  for (const auto& name : lib->names()) {
    if (!name.ends_with(".BMP") || webo::alphaModeFor(name) != webo::AlphaMode::Embedded)
      continue;
    ++plusCount;
    auto alfName = name;
    alfName.replace(alfName.size() - 4, 4, ".ALF");
    const auto alf = lib->read(alfName);
    const auto image = webo::loadTexture(*lib, name);
    check(image.has_value(), std::format("{}: retail plus loads", name));
    if (!image) { std::cerr << image.error() << '\n'; continue; }
    check(alf && alf->size() >= image->pixelCount(), std::format("{}: selected sibling plane covers pixels", name));
    if (!alf || alf->size() < image->pixelCount()) continue;
    std::size_t differences = 0;
    for (std::size_t i = 0; i < image->pixelCount(); ++i)
      differences += image->rgba[i * 4 + 3] != std::to_integer<std::uint8_t>((*alf)[i]);
    check(differences == 0 && image->alpha == webo::AlphaMode::Embedded,
          std::format("{}: every retail alpha byte equals selected ALF; legacy mode retained", name));
    std::cout << std::format("    {}: {}x{}, {} pixels, ALF {} bytes, alpha differences {}\n",
                             name, image->width, image->height, image->pixelCount(), alf->size(), differences);
  }
  std::cout << std::format("    retail plus BMP corpus: {}\n", plusCount);
}

}  // namespace

int main() {
  testSigils();
  testDecode24();
  testAlf();
  testPng();
  testArchiveTextures();
  testPlusTextures();
  testRealTextures();

  std::cout << std::format("\n{} checks, {} failures\n", g_checks, g_failures);
  return g_failures == 0 ? 0 : 1;
}
