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

// Per-level grey light map and RGB palette stored immediately after the
// collision grid (`u32 12288, u8[12288]` then 256 x {f64 R, G, B} with no
// prefix). Indexed by actor world position; see actorLightMapIndex.
struct ActorLightMap {
  static constexpr std::size_t kMapBytes = 12288;
  static constexpr std::size_t kPaletteBytes = 6144;
  static constexpr int kPaletteEntries = 256;

  std::vector<std::uint8_t> map;
  // Palette channel bytes converted from the stored doubles (grey entries use
  // equal R, G, B).
  std::array<std::array<std::uint8_t, 3>, kPaletteEntries> palette{};

  bool empty() const { return map.empty(); }

  std::size_t histogram(std::uint8_t value) const;
};

// Native index at 0x403F40 / 0x403FC0: trunc(Y), trunc(X), then
// `(b << 8) / 2 + (a & 0xFF) / 2` with C signed truncating division.
int actorLightMapIndex(double worldX, double worldY);

std::expected<ActorLightMap, std::string> findActorLightMap(std::span<const std::byte> levelBytes);

std::expected<ActorLightMap, std::string> loadActorLightMap(const AssetLibrary&,
                                                            std::string_view levelName);

// Bounds-checked lookup; the native path does not guard idx.
std::expected<std::array<std::uint8_t, 3>, std::string> actorLightTintAt(
    const ActorLightMap& light, double worldX, double worldY);

// D3D-style MODULATE on RGB; alpha is copied unchanged. This is the RGB stage
// only; the alpha test below is separate (A4u).
std::array<std::uint8_t, 4> modulateActorSpritePixel(const std::array<std::uint8_t, 4>& tex,
                                                     const std::array<std::uint8_t, 3>& tint);

// Material row 3 (flags 0x2): ALPHAFUNC GREATER against the reference stored at
// 0x004C24E0. A4x: the actor submit 0x47A75D calls 0x470F29(0) at 0x47A776
// before its draw, and the writer 0x470F39 sets [0x4C24E0] = 0xEF; the reader is
// 0x472896. The 0xA0 in file .data and read live between draws is the LINEAR
// post-call value (A4u/A4t/A4w), not the in-draw one. A texel is drawn only when
// texture alpha is strictly greater than the reference. Row 3 alpha is
// SELECTARG1(TEXTURE) and selector 0 disables blending, so a drawn texel
// replaces the destination (see drawActorSprite).
inline constexpr std::uint8_t kActorAlphaTestReference = 0xEF;
bool actorSpriteAlphaTestPasses(std::uint8_t textureAlpha);

}  // namespace webo
