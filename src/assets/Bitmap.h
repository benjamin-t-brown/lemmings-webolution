// Decodes the Windows BMP in a texture entry (Lrz.h). A leading sigil selects transparency:
// ^ or + uses the sibling .ALF 8-bit plane. _ keys 000000, and $ keys 404040.
// ~ selects additive blending and leaves pixel values alone.
// AlphaMode::Embedded ("embedded") denotes '+' for compatibility. loadTexture applies
// the sibling alpha plane. Caller-built Embedded images keep their supplied RGBA.

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
  Embedded,     // '+' loads a sibling ALF
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

// Normalize BMP rows to top-down. Preserve 32bpp alpha and make other depths opaque.
std::expected<Image, std::string> decodeBmp(std::span<const std::byte> bmp);

// Applies an 8-bit alpha plane, as stored in a .ALF: one byte per pixel, left
// to right and top to bottom, same dimensions as the bitmap.
std::expected<void, std::string> applyAlfAlpha(Image& image, std::span<const std::byte> alf);

// Reads an entry, decompresses it, decodes the bitmap, and applies whatever
// transparency its name calls for -- fetching the sibling .ALF for '^' or '+'.
std::expected<Image, std::string> loadTexture(const AssetLibrary& lib, std::string_view name);

}  // namespace webo
