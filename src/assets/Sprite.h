// .SPR -- sprite sheets: which rectangle of which texture each animation frame
// uses.
//
// The format is not publicly documented; this was derived by parsing the 19
// retail files until every one was consumed exactly. Layout:
//
//     u32 spriteCount
//     spriteCount x {
//         u32 animationCount
//         animationCount x {
//             u32 frameCount
//             frameCount x {
//                 u32    bottom, top, left, right    source rect, pixels
//                 double bottom, top, left, right    the same, normalised
//                 u32    unknown0, unknown1
//                 u32    width, height                == right-left, bottom-top
//                 u32 len; char[len] sheet            e.g. "Sheets\$LemSheet8"
//             }
//         }
//         u32 len; char[len] name                     e.g. "bash"
//         double rate
//     }
//     u32 paletteCount
//     paletteCount x { u32 unknown; double[48] }       16 RGB colours, 0..255
//
// The integer rectangle and the normalised one always agree, which is what
// pins down the field order: dividing one by the other recovers the sheet's
// pixel dimensions, and only the Y-then-X reading gives a sheet's real size
// rather than its dimensions transposed. `width`/`height` likewise always
// equal the rectangle's extents.
//
// Only the six lemming sheets carry a palette, and all six carry the same one:
// 16 colours whose first entry is 404040, the shade the '$' name sigil keys
// out. The two unknown per-frame integers are small and non-negative; they
// look like a per-frame anchor but that is not confirmed, so they are not
// given a name that asserts it.

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
  std::string sheet;  // as stored, e.g. "Sheets\\$LemSheet8"

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

using PaletteColour = std::array<double, 3>;  // RGB, 0..255

struct SpriteSheet {
  std::vector<Sprite> sprites;
  std::vector<PaletteColour> palette;  // empty unless this is a lemming sheet

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
