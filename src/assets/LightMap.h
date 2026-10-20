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

// D3D-style MODULATE on RGB; alpha is copied unchanged (blend state unread).
std::array<std::uint8_t, 4> modulateActorSpritePixel(const std::array<std::uint8_t, 4>& tex,
                                                     const std::array<std::uint8_t, 3>& tint);

}  // namespace webo
