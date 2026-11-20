// .SPR projection of the serialized reader (0x00483EF0): clips become Sprite objects,
// temporal frames become SpriteAnimation entries, and pieces become SpriteFrame entries.
// values_00 supplies bottom/top/left/right, double_bits_10 the u* fields, and values_30
// unknown0/unknown1/width/height. Keep original order and numeric bits, without x87 conversions.
// Names use bounded C-string behavior with capacities 128 (clip) and 260 (texture).
// Embedded NULs terminate names and exact-capacity names are shortened.
// The palette vector flattens tagged triples in block order (tag 0: 16, tag 1: 256,
// others: none). Runtime meaning of all tags remains unresolved.

#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <expected>
#include <span>
#include <string>
#include <string_view>
#include <vector>

namespace webo {

class AssetLibrary;

struct SpriteFrame {
  std::uint32_t left = 0, top = 0, right = 0, bottom = 0;
  double uLeft = 0, uTop = 0, uRight = 0, uBottom = 0;
  std::uint32_t unknown0 = 0, unknown1 = 0;
  std::uint32_t width = 0, height = 0;
  std::string sheet;  // bounded texture name, e.g. "Sheets\\$LemSheet8"

  // Infer sheet size from the pixel and normalized rects. Degenerate frames yield zero.
  std::uint32_t sheetWidth() const;
  std::uint32_t sheetHeight() const;
};

struct SpriteAnimation {
  // A temporal entry holds its directional images in order.
  std::vector<SpriteFrame> frames;
};

struct Sprite {
  std::string name;  // "walk", "bash", "explode", ...
  // Indexes temporal entries. SpriteAnimation::frames holds the directional images.
  std::vector<SpriteAnimation> animations;
  double rate = 0;

  std::size_t frameCount() const;
};

using PaletteColour = std::array<double, 3>;  // compatibility name for serialized triples

struct SpriteSheet {
  std::vector<Sprite> sprites;
  std::vector<PaletteColour> palette;  // populated tagged triples, flattened in block order

  const Sprite* find(std::string_view name) const;
  std::size_t frameCount() const;
};

// "Sheets\\$LemSheet8" -> "TEXTURES\\SHEETS\\$LEMSHEET8.BMP"
std::string sheetAssetName(std::string_view sheet);

std::expected<SpriteSheet, std::string> parseSpriteSheet(std::span<const std::byte>);
std::expected<SpriteSheet, std::string> loadSpriteSheet(const AssetLibrary&, std::string_view name);

// Every "SPRITES\\*.SPR" in the archive.
std::vector<std::string> spriteSheetNames(const AssetLibrary&);

}  // namespace webo
