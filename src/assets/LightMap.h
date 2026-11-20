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

// Light map after the collision grid: u32 count, u8[12288], then 256 uncounted f64 RGB triples.
// Index by actor terrain X and file-row Y (actorLightMapIndex).
struct ActorLightMap {
  static constexpr std::size_t kMapBytes = 12288;
  static constexpr std::size_t kPaletteBytes = 6144;
  static constexpr int kPaletteEntries = 256;

  std::vector<std::uint8_t> map;
  // Convert the stored RGB doubles to bytes.
  std::array<std::array<std::uint8_t, 3>, kPaletteEntries> palette{};

  // Empty on structured success; recovered rejection and cursor when the
  // legacy locator supplied the data instead.
  std::string diagnostic;

  bool empty() const { return map.empty(); }

  std::size_t histogram(std::uint8_t value) const;
};

// 0x403F40 / 0x403FC0 take terrainX from actor+0x08 and fileRowY from actor+0x10.
// With a = trunc(terrainX), b = trunc(fileRowY), the index is
// `(b << 8) / 2 + (a & 0xFF) / 2` with C signed truncating division.
int actorLightMapIndex(double terrainX, double fileRowY);

std::expected<ActorLightMap, std::string> findActorLightMap(std::span<const std::byte> levelBytes);

std::expected<ActorLightMap, std::string> loadActorLightMap(const AssetLibrary&,
                                                            std::string_view levelName);

// Bounds-checked lookup. Native lookup leaves idx unchecked.
std::expected<std::array<std::uint8_t, 3>, std::string> actorLightTintAt(
    const ActorLightMap& light, double terrainX, double fileRowY);

// Apply D3D MODULATE to RGB, preserving alpha for the separate test.
std::array<std::uint8_t, 4> modulateActorSpritePixel(const std::array<std::uint8_t, 4>& tex,
                                                     const std::array<std::uint8_t, 3>& tint);

// Material row 3 (flags 0x2) uses ALPHAFUNC GREATER against 0x004C24E0.
// Actor submit 0x47A75D calls 0x470F29(0) at 0x47A776. Writer 0x470F39 sets
// [0x4C24E0] to 0xEF for reader 0x472896. The LINEAR post-call value is 0xA0, also stored in .data.
// SELECTARG1(TEXTURE) takes texture alpha. Selector 0 disables blending, so drawActorSprite replaces the destination.
inline constexpr std::uint8_t kActorAlphaTestReference = 0xEF;
bool actorSpriteAlphaTestPasses(std::uint8_t textureAlpha);

}  // namespace webo
