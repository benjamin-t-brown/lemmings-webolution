#include "CollisionGrid.h"

#include "Box.h"

#include "lemmings/level_name_tables.hpp"
#include "lemmings/world_geometry_prefix.hpp"

#include <bit>
#include <cstring>
#include <format>
#include <stdexcept>
#include <utility>

namespace webo {
namespace {

// Begin at the length-prefixed "Cylinder" chunk name.
constexpr char kCylinder[] = "Cylinder";
constexpr std::uint32_t kCylinderLen = 8;

// Preamble after "Cylinder": five zero words, height and a double (0.36 in retail).
// The locator uses field sizes and does not validate these values.
constexpr std::size_t kZeroWords = 5;
constexpr std::size_t kPreambleBytes = kZeroWords * 4 + 4 /*height*/ + 8 /*f64*/;

// The 12-byte records the engine deserialises one at a time into `+0x43C`
// (FUN_0043B9C0, `imul ecx, ecx, 0xc` at 0x0043BA1D).
constexpr std::size_t kRecordBytes = 12;

// Allocation bound for malformed grids. Retail height is 96.
constexpr std::uint32_t kMaxRows = 4096;

bool readU32(std::span<const std::byte> d, std::size_t offset, std::uint32_t& out) {
  if (offset + 4 > d.size())
    return false;
  std::memcpy(&out, d.data() + offset, 4);
  return true;
}

bool readF64(std::span<const std::byte> d, std::size_t offset, double& out) {
  if (offset + 8 > d.size())
    return false;
  std::memcpy(&out, d.data() + offset, 8);
  return true;
}

// Locate the literal u32 8, "Cylinder" chunk. LevelIndex scans its other references separately.
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

}  // namespace

std::uint32_t CollisionGrid::at(int x, int y) const {
  if (cells.empty() || y < 0 || y >= height)
    return 0;
  int cx = x % width;
  if (cx < 0)
    cx += width;
  return cells[static_cast<std::size_t>(y) * width + cx];
}

bool CollisionGrid::solid(int x, int y) const {
  const std::uint32_t c = at(x, y);
  return c != 0 && !(c & kCellNotLandable);
}

std::size_t CollisionGrid::solidCells() const {
  std::size_t n = 0;
  for (int y = 0; y < height; ++y)
    for (int x = 0; x < width; ++x)
      if (solid(x, y))
        ++n;
  return n;
}

namespace {

std::expected<CollisionGrid, std::string> findCollisionGridLegacy(
    std::span<const std::byte> d) {
  const std::size_t cyl = findCylinderChunk(d);
  if (cyl == std::string::npos)
    return std::unexpected("collision grid: no \"Cylinder\" chunk");

  // Skip the length-prefixed name and preamble.
  std::size_t p = cyl + 4 + kCylinderLen;
  std::uint32_t rows = 0;
  if (!readU32(d, p + kZeroWords * 4, rows))
    return std::unexpected("collision grid: truncated in the Cylinder preamble");
  if (rows == 0 || rows > kMaxRows)
    return std::unexpected(std::format("collision grid: implausible height {}", rows));
  double extent = 0.0;
  const bool haveExtent = readF64(d, p + kZeroWords * 4 + 4, extent);
  p += kPreambleBytes;

  // Use the serialized array length.
  std::uint32_t tubeCount = 0;
  if (!readU32(d, p, tubeCount))
    return std::unexpected("collision grid: truncated before the surface array");
  p += 4;
  if (tubeCount > d.size() / 4)
    return std::unexpected("collision grid: surface array runs past the file");
  p += static_cast<std::size_t>(tubeCount) * 4;

  // The 12-byte record array. Usually empty.
  std::uint32_t recordCount = 0;
  if (!readU32(d, p, recordCount))
    return std::unexpected("collision grid: truncated before the record array");
  p += 4;
  if (recordCount > d.size() / kRecordBytes)
    return std::unexpected("collision grid: record array runs past the file");
  p += static_cast<std::size_t>(recordCount) * kRecordBytes;

  std::uint32_t count = 0;
  if (!readU32(d, p, count))
    return std::unexpected("collision grid: truncated before the grid");
  p += 4;
  const std::size_t want = static_cast<std::size_t>(CollisionGrid::kWidth) * rows;
  if (count != want)
    return std::unexpected(
        std::format("collision grid: expected {} cells for {} rows, the file says {}",
                    want, rows, count));
  if (p + want * 4 > d.size())
    return std::unexpected("collision grid: grid runs past the end of the file");

  CollisionGrid grid;
  grid.height = static_cast<int>(rows);
  grid.cylinderExtent = haveExtent ? extent : 0.0;
  grid.cells.resize(want);
  std::memcpy(grid.cells.data(), d.data() + p, want * 4);
  return grid;
}

}  // namespace

std::expected<CollisionGrid, std::string> findCollisionGrid(
    std::span<const std::byte> d) {
  // Keep byte storage alive for the borrowing stream and read in serialized order.
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
    auto fallback = findCollisionGridLegacy(d);
    const auto rejection = std::format("collision grid: recovered reader at byte {}: {}",
                                       stream.cursor, error.what());
    if (!fallback)
      return std::unexpected(rejection + "; legacy locator: " + fallback.error());
    fallback->diagnostic = rejection;
    return fallback;
  }

  // Clone validation: reject invalid decoded dimensions before attempting any fallback.
  const auto rows = world.following_value;
  if (rows == 0 || rows > kMaxRows)
    return std::unexpected(std::format("collision grid: implausible height {}", rows));
  const auto want = static_cast<std::size_t>(CollisionGrid::kWidth) * rows;
  if (world.second_integer_array.size() != want)
    return std::unexpected(
        std::format("collision grid: expected {} cells for {} rows, the file says {}",
                    want, rows, world.second_integer_array.size()));

  CollisionGrid grid;
  grid.height = static_cast<int>(rows);
  grid.cells = std::move(world.second_integer_array);
  // Mapping this field to runtime world+0x408 is inferred.
  grid.cylinderExtent = std::bit_cast<double>(world.following_double_bits_2);
  return grid;
}

std::expected<CollisionGrid, std::string> loadCollisionGrid(const AssetLibrary& lib,
                                                            std::string_view name) {
  auto bytes = lib.read(name);
  if (!bytes)
    return std::unexpected(bytes.error());
  return findCollisionGrid(*bytes);
}

}  // namespace webo
