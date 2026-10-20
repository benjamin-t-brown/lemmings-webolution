#include "LightMap.h"

#include "Box.h"

#include <cmath>
#include <cstring>
#include <format>

namespace webo {
namespace {

constexpr char kCylinder[] = "Cylinder";
constexpr std::uint32_t kCylinderLen = 8;
constexpr std::size_t kZeroWords = 5;
constexpr std::size_t kPreambleBytes = kZeroWords * 4 + 4 + 8;
constexpr std::size_t kRecordBytes = 12;
constexpr std::uint32_t kMaxRows = 4096;

bool readU32(std::span<const std::byte> d, std::size_t offset, std::uint32_t& out) {
  if (offset + 4 > d.size())
    return false;
  std::memcpy(&out, d.data() + offset, 4);
  return true;
}

std::size_t findCylinderChunk(std::span<const std::byte> d) {
  std::byte needle[4 + kCylinderLen];
  std::memcpy(needle, &kCylinderLen, 4);
  std::memcpy(needle + 4, kCylinder, kCylinderLen);
  if (d.size() < sizeof(needle))
    return std::string::npos;
  for (std::size_t i = 0; i + sizeof(needle) <= d.size(); ++i)
    if (std::memcmp(d.data() + i, needle, sizeof(needle)) == 0)
      return i;
  return std::string::npos;
}

std::uint8_t paletteChannel(double value) {
  if (!std::isfinite(value) || value <= 0.0)
    return 0;
  if (value >= 255.0)
    return 255;
  return static_cast<std::uint8_t>(value);
}

}  // namespace

int actorLightMapIndex(double worldX, double worldY) {
  const int a = static_cast<int>(std::trunc(worldX));
  const int b = static_cast<int>(std::trunc(worldY));
  return (b << 8) / 2 + (a & 0xFF) / 2;
}

std::size_t ActorLightMap::histogram(std::uint8_t value) const {
  std::size_t count = 0;
  for (std::uint8_t byte : map)
    if (byte == value)
      ++count;
  return count;
}

std::expected<ActorLightMap, std::string> findActorLightMap(
    std::span<const std::byte> d) {
  const std::size_t cyl = findCylinderChunk(d);
  if (cyl == std::string::npos)
    return std::unexpected("actor light map: no \"Cylinder\" chunk");

  std::size_t p = cyl + 4 + kCylinderLen;
  std::uint32_t rows = 0;
  if (!readU32(d, p + kZeroWords * 4, rows))
    return std::unexpected("actor light map: truncated in the Cylinder preamble");
  if (rows == 0 || rows > kMaxRows)
    return std::unexpected(std::format("actor light map: implausible height {}", rows));
  p += kPreambleBytes;

  std::uint32_t tubeCount = 0;
  if (!readU32(d, p, tubeCount))
    return std::unexpected("actor light map: truncated before the surface array");
  p += 4;
  if (tubeCount > d.size() / 4)
    return std::unexpected("actor light map: surface array runs past the file");
  p += static_cast<std::size_t>(tubeCount) * 4;

  std::uint32_t recordCount = 0;
  if (!readU32(d, p, recordCount))
    return std::unexpected("actor light map: truncated before the record array");
  p += 4;
  if (recordCount > d.size() / kRecordBytes)
    return std::unexpected("actor light map: record array runs past the file");
  p += static_cast<std::size_t>(recordCount) * kRecordBytes;

  std::uint32_t gridCount = 0;
  if (!readU32(d, p, gridCount))
    return std::unexpected("actor light map: truncated before the grid");
  p += 4;
  const std::size_t gridBytes = static_cast<std::size_t>(gridCount) * 4;
  if (p + gridBytes > d.size())
    return std::unexpected("actor light map: grid runs past the end of the file");
  p += gridBytes;

  std::uint32_t mapCount = 0;
  if (!readU32(d, p, mapCount))
    return std::unexpected("actor light map: truncated before the byte map count");
  p += 4;
  if (mapCount != ActorLightMap::kMapBytes)
    return std::unexpected(
        std::format("actor light map: expected {} map bytes, the file says {}", ActorLightMap::kMapBytes,
                    mapCount));
  if (p + mapCount + ActorLightMap::kPaletteBytes > d.size())
    return std::unexpected("actor light map: map or palette runs past the end of the file");

  ActorLightMap light;
  light.map.resize(mapCount);
  std::memcpy(light.map.data(), d.data() + p, mapCount);
  p += mapCount;

  for (int i = 0; i < ActorLightMap::kPaletteEntries; ++i) {
    double rgb[3]{};
    std::memcpy(rgb, d.data() + p, sizeof(rgb));
    p += sizeof(rgb);
    light.palette[static_cast<std::size_t>(i)] = {paletteChannel(rgb[0]), paletteChannel(rgb[1]),
                                                  paletteChannel(rgb[2])};
  }
  return light;
}

std::expected<ActorLightMap, std::string> loadActorLightMap(const AssetLibrary& lib,
                                                            std::string_view levelName) {
  auto bytes = lib.read(levelName);
  if (!bytes)
    return std::unexpected(bytes.error());
  return findActorLightMap(*bytes);
}

std::expected<std::array<std::uint8_t, 3>, std::string> actorLightTintAt(
    const ActorLightMap& light, double worldX, double worldY) {
  if (light.map.size() != ActorLightMap::kMapBytes)
    return std::unexpected("actor light map: map storage is not loaded");
  const int idx = actorLightMapIndex(worldX, worldY);
  if (idx < 0 || static_cast<std::size_t>(idx) >= light.map.size())
    return std::unexpected(std::format("actor light map: index {} out of range", idx));
  const std::uint8_t entry = light.map[static_cast<std::size_t>(idx)];
  return light.palette[entry];
}

std::array<std::uint8_t, 4> modulateActorSpritePixel(const std::array<std::uint8_t, 4>& tex,
                                                     const std::array<std::uint8_t, 3>& tint) {
  return {static_cast<std::uint8_t>((tex[0] * tint[0]) / 255),
          static_cast<std::uint8_t>((tex[1] * tint[1]) / 255),
          static_cast<std::uint8_t>((tex[2] * tint[2]) / 255), tex[3]};
}

bool actorSpriteAlphaTestPasses(std::uint8_t textureAlpha) {
  return textureAlpha > kActorAlphaTestReference;
}

}  // namespace webo
