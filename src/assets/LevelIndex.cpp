#include "LevelIndex.h"

#include "Box.h"

#include "lemmings/level_name_tables.hpp"
#include "lemmings/world_geometry_prefix.hpp"

#include <cstring>
#include <format>
#include <stdexcept>
#include <utility>

namespace webo {
namespace {

constexpr std::uint32_t kEmptyCell = 0xFFFFFFFFu;
constexpr std::size_t kMaxStringLen = 200;
constexpr std::size_t kHeaderWords = 12;

std::uint32_t u32At(std::span<const std::byte> d, std::size_t o) {
  std::uint32_t v;
  std::memcpy(&v, d.data() + o, 4);
  return v;
}

bool printableAscii(std::span<const std::byte> d, std::size_t o, std::size_t n) {
  for (std::size_t i = 0; i < n; ++i) {
    const auto c = std::to_integer<unsigned char>(d[o + i]);
    if (c < 32 || c >= 127)
      return false;
  }
  return true;
}

// Does `s` start with `prefix`, ignoring case and treating / and \ alike?
bool startsWithFolder(std::string_view s, std::string_view folder) {
  if (s.size() <= folder.size())
    return false;
  for (std::size_t i = 0; i < folder.size(); ++i) {
    char a = s[i], bch = folder[i];
    if (a >= 'a' && a <= 'z')
      a = static_cast<char>(a - 'a' + 'A');
    if (bch >= 'a' && bch <= 'z')
      bch = static_cast<char>(bch - 'a' + 'A');
    if (a != bch)
      return false;
  }
  const char sep = s[folder.size()];
  return sep == '\\' || sep == '/';
}

}  // namespace

std::size_t LevelIndex::grid1SolidCells() const {
  std::size_t n = 0;
  for (std::uint32_t v : grid1)
    if (v != kEmptyCell)
      ++n;
  return n;
}

std::string levelTextureAssetName(std::string_view reference) {
  std::string out = "TEXTURES\\";
  out += normaliseAssetName(reference);
  out += ".BMP";
  return out;
}

std::string levelTextureAssetNameHi(std::string_view reference) {
  std::string out = "TEXTURES\\";
  out += normaliseAssetName(reference);
  out += "HI.BMP";
  return out;
}

std::expected<LevelIndex, std::string> indexLevel(std::span<const std::byte> d) {
  if (d.size() < 64)
    return std::unexpected("too small to be a level");

  LevelIndex lvl;

  const std::uint32_t titleLen = u32At(d, 0);
  if (titleLen == 0 || titleLen > kMaxStringLen || 4 + titleLen > d.size() ||
      !printableAscii(d, 4, titleLen))
    return std::unexpected("no length-prefixed title at offset 0");
  lvl.title.assign(reinterpret_cast<const char*>(d.data() + 4), titleLen);

  std::size_t pos = 4 + titleLen;
  for (std::size_t i = 0; i < kHeaderWords && pos + 4 <= d.size(); ++i, pos += 4)
    lvl.header.push_back(u32At(d, pos));

  // Heuristically index length-prefixed printable strings.
  std::size_t cylinderChunk = std::string::npos;
  std::size_t i = 0;
  while (i + 4 <= d.size()) {
    const std::uint32_t len = u32At(d, i);
    if (len >= 1 && len <= kMaxStringLen && i + 4 + len <= d.size() &&
        printableAscii(d, i + 4, len)) {
      std::string s(reinterpret_cast<const char*>(d.data() + i + 4), len);

      if (startsWithFolder(s, "Cylinder")) {
        lvl.cylinderTextures.push_back(s);
      } else if (startsWithFolder(s, "Models")) {
        lvl.modelTextures.push_back(s);
      } else if (s.starts_with("PolyList")) {
        ++lvl.polyListCount;
      } else if (s == "Cylinder") {
        if (cylinderChunk == std::string::npos)
          cylinderChunk = i + 4 + len;
      } else {
        const char c = s[0];
        const bool alnum =
            (c >= '0' && c <= '9') || (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z');
        if (alnum)
          lvl.shortNames.push_back(std::move(s));
        else
          lvl.scriptVariables.push_back(std::move(s));
      }
      i += 4 + len;
      continue;
    }
    ++i;
  }

  // Keep the borrowed storage alive through the two ordered reads. Stop at the world boundary.
  std::vector<std::uint8_t> bytes;
  bytes.reserve(d.size());
  for (const auto byte : d)
    bytes.push_back(std::to_integer<std::uint8_t>(byte));
  lemmings::LevelByteStream stream{bytes};
  lemmings::WorldGeometryPrefix world;
  try {
    (void)lemmings::read_compiled_level_prefix(stream);
    world = lemmings::read_world_geometry_prefix(stream);
  } catch (const std::invalid_argument& error) {
    lvl.gridFallbackReason = std::format(
        "surface-index lattice: recovered reader at byte {}: {}", stream.cursor, error.what());
  }

  if (lvl.gridFallbackReason.empty()) {
    // Preserve every word and its position, including an empty array.
    lvl.grid0 = std::move(world.first_integer_array);
  } else if (cylinderChunk != std::string::npos) {
    // On reader rejection, try the legacy locator. A missing array still permits a partial index.
    constexpr std::size_t kPreambleBytes = 5 * 4 + 4 + 8;
    const std::size_t countAt = cylinderChunk + kPreambleBytes;
    if (countAt + 4 <= d.size()) {
      const std::uint32_t n = u32At(d, countAt);
      const std::size_t payload = countAt + 4;
      if (n > 0 && payload + static_cast<std::size_t>(n) * 4 <= d.size()) {
        lvl.grid0.resize(n);
        std::memcpy(lvl.grid0.data(), d.data() + payload,
                    static_cast<std::size_t>(n) * 4);
      }
    }
  }

  return lvl;
}

std::expected<LevelIndex, std::string> loadLevelIndex(const AssetLibrary& lib,
                                                      std::string_view name) {
  auto bytes = lib.read(name);
  if (!bytes)
    return std::unexpected(bytes.error());
  auto parsed = indexLevel(*bytes);
  if (!parsed)
    return std::unexpected(std::format("{}: {}", name, parsed.error()));
  return parsed;
}

std::vector<std::string> levelFileNames(const AssetLibrary& lib) {
  std::vector<std::string> out;
  for (const auto& name : lib.names()) {
    const auto upper = normaliseAssetName(name);
    if (upper.starts_with("COMPILED\\") && upper.ends_with(".LVL"))
      out.push_back(name);
  }
  return out;
}

}  // namespace webo
