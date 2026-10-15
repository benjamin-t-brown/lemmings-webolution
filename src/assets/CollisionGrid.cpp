#include "CollisionGrid.h"

#include "Box.h"

#include <cstring>
#include <format>

namespace webo {
namespace {

// The chunk the walk starts from. Length-prefixed, like every string in the
// file.
constexpr char kCylinder[] = "Cylinder";
constexpr std::uint32_t kCylinderLen = 8;

// Fixed fields between "Cylinder" and the first array: five zero words, the
// height, and a double. Their values are not asserted -- only their size
// matters for the walk -- but they are stable across all 102 levels
// (0,0,0,0,0 and 0.36), so a mismatch downstream means the walk is wrong
// rather than the data being unusual.
constexpr std::size_t kZeroWords = 5;
constexpr std::size_t kPreambleBytes = kZeroWords * 4 + 4 /*height*/ + 8 /*f64*/;

// The 12-byte records the engine deserialises one at a time into `+0x43C`
// (FUN_0043B9C0, `imul ecx, ecx, 0xc` at 0x0043BA1D).
constexpr std::size_t kRecordBytes = 12;

// A sane bound on the grid, so a bad walk fails instead of allocating wildly.
// Retail is 96 rows; this leaves room without trusting the file.
constexpr std::uint32_t kMaxRows = 4096;

bool readU32(std::span<const std::byte> d, std::size_t offset, std::uint32_t& out) {
  if (offset + 4 > d.size())
    return false;
  std::memcpy(&out, d.data() + offset, 4);
  return true;
}

// The literal `u32 8, "Cylinder"` chunk. `LevelIndex` finds the same string
// while scanning for references; this looks for it directly because the walk
// below needs its exact offset.
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

std::expected<CollisionGrid, std::string> findCollisionGrid(
    std::span<const std::byte> d) {
  const std::size_t cyl = findCylinderChunk(d);
  if (cyl == std::string::npos)
    return std::unexpected("collision grid: no \"Cylinder\" chunk");

  // Past the length-prefixed string, then the fixed preamble.
  std::size_t p = cyl + 4 + kCylinderLen;
  std::uint32_t rows = 0;
  if (!readU32(d, p + kZeroWords * 4, rows))
    return std::unexpected("collision grid: truncated in the Cylinder preamble");
  if (rows == 0 || rows > kMaxRows)
    return std::unexpected(std::format("collision grid: implausible height {}", rows));
  p += kPreambleBytes;

  // The tube's own array. Its length is the same in every retail level, but
  // it is read from the file rather than assumed.
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

  // The grid itself.
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
  grid.cells.resize(want);
  std::memcpy(grid.cells.data(), d.data() + p, want * 4);
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
