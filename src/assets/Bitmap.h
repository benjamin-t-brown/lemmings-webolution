// Turning a decoded texture entry into pixels.
//
// A texture entry decompresses (see Lrz.h) to an ordinary Windows BMP. Across
// the archive those are 24bpp (10208 of them), 8bpp palettised (160) and 4bpp
// palettised (15).
//
// Transparency is not in the bitmap: it is selected by a sigil on the file
// name, and the archive uses all of them:
//
//     ^   a sibling .ALF holds an 8-bit alpha plane      (82 entries)
//     _   colour 000000 is transparent                   (78 entries)
//     $   colour 404040 is transparent                   (161 entries)
//     +   the bitmap carries its own alpha               (1 entry)
//     ~   drawn with additive blending                   (38 entries)
//
// `~` is a blend mode rather than a mask, so it is reported but does not
// change the pixels.

#pragma once

#include <cstddef>
#include <cstdint>
#include <expected>
#include <span>
#include <string>
#include <string_view>
#include <vector>

namespace webo {

class AssetLibrary;

enum class AlphaMode {
  Opaque,       // no sigil
  AlfFile,      // '^'
  Embedded,     // '+'
  KeyBlack,     // '_'  colour key 000000
  KeyGrey,      // '$'  colour key 404040
  Additive,     // '~'  blend mode, pixels untouched
};

std::string_view alphaModeName(AlphaMode m);

// Which sigil, if any, leads the file name's last path component.
AlphaMode alphaModeFor(std::string_view assetName);

// Pixels are RGBA8, top-down, tightly packed.
struct Image {
  int width = 0;
  int height = 0;
  AlphaMode alpha = AlphaMode::Opaque;
  std::vector<std::uint8_t> rgba;

  std::size_t pixelCount() const { return static_cast<std::size_t>(width) * height; }
  bool empty() const { return rgba.empty(); }
};

// Decodes a Windows BMP. Bottom-up rows (the usual case) are flipped so the
// result is always top-down. Alpha comes out opaque.
std::expected<Image, std::string> decodeBmp(std::span<const std::byte> bmp);

// Applies an 8-bit alpha plane, as stored in a .ALF: one byte per pixel, left
// to right and top to bottom, same dimensions as the bitmap.
std::expected<void, std::string> applyAlfAlpha(Image& image, std::span<const std::byte> alf);

// Reads an entry, decompresses it, decodes the bitmap, and applies whatever
// transparency its name calls for -- fetching the sibling .ALF when needed.
std::expected<Image, std::string> loadTexture(const AssetLibrary& lib, std::string_view name);

}  // namespace webo
