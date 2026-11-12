// .SPR compatibility view used by the existing renderer. Parsing delegates to
// the recovered serialized reader (0x00483EF0 and nested helpers): clips map
// to Sprite, temporal frames to SpriteAnimation, and pieces to SpriteFrame,
// preserving all three levels of ordering.
//
// Piece values_00 map to bottom/top/left/right, double_bits_10 to the matching
// u* fields, and values_30 to unknown0/unknown1/width/height. Names use the
// recovered bounded C-string behavior (capacities 128 for clips, 260 for
// textures), including embedded-NUL termination and exact-capacity shortening.
// Numeric fields and clip speeds retain their serialized bits without arithmetic.
// This is separate from runtime allocation/layout, texture IDs and the original
// x87 loads/stores that may quiet signaling NaNs while reading runtime objects.
//
// The public palette vector flattens populated tagged triple arrays in block
// order: tag 0 supplies 16 triples, tag 1 supplies 256, other tags supply none.
// This API name does not establish that every tagged array is a rendering
// palette. Historical retail observations found six lemming sheets with the
// same 16-colour type-0 array; that is not a format restriction. Unknown piece
// integers remain unnamed; rectangle/UV interpretation is retained for consumers.

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

  // The sheet's pixel size, recovered from the rectangle and its normalised
  // form. Zero when the frame is degenerate.
  std::uint32_t sheetWidth() const;
  std::uint32_t sheetHeight() const;
};

struct SpriteAnimation {
  // The native selector treats this as one temporal entry. Its `frames`
  // vector contains the ordered directional images available at that time.
  std::vector<SpriteFrame> frames;
};

struct Sprite {
  std::string name;  // "walk", "bash", "explode", ...
  // Legacy API name: this indexes temporal animation entries; each entry's
  // SpriteAnimation::frames is the directional-image list.
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
