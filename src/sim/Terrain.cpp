#include "Terrain.h"

#include "assets/CollisionGrid.h"

#include <algorithm>

namespace webo::sim {

Terrain::Terrain(int width, int height, Material fill)
    : width_(std::max(0, width)), height_(std::max(0, height)) {
  cells_.assign(static_cast<std::size_t>(width_) * height_, fill);
}

int Terrain::wrapX(int x) const {
  if (width_ <= 0)
    return 0;
  x %= width_;
  if (x < 0)
    x += width_;
  return x;
}

std::size_t Terrain::index(int x, int y) const {
  return static_cast<std::size_t>(y) * width_ + wrapX(x);
}

Material Terrain::at(int x, int y) const {
  if (cells_.empty())
    return Material::Empty;
  // Above the world is open sky; below it is solid, so nothing falls out.
  if (y < 0)
    return Material::Empty;
  if (y >= height_)
    return Material::Steel;
  return cells_[index(x, y)];
}

void Terrain::set(int x, int y, Material m) {
  if (cells_.empty() || y < 0 || y >= height_)
    return;
  cells_[index(x, y)] = m;
}

bool Terrain::blockerField(int x, int y) const {
  if (blocker_.empty() || y < 0 || y >= height_)
    return false;
  return blocker_[index(x, y)] != 0;
}

void Terrain::setBlockerField(int x, int y, bool on) {
  if (cells_.empty() || y < 0 || y >= height_)
    return;
  if (blocker_.empty()) {
    if (!on)
      return;
    blocker_.assign(cells_.size(), 0);
  }
  blocker_[index(x, y)] = on ? 1 : 0;
}

std::size_t Terrain::blockerFieldCells() const {
  std::size_t n = 0;
  for (std::uint8_t b : blocker_)
    if (b)
      ++n;
  return n;
}

bool Terrain::antiGrav(int x, int y) const {
  if (antiGrav_.empty() || y < 0 || y >= height_)
    return false;
  return antiGrav_[index(x, y)] != 0;
}

void Terrain::setAntiGrav(int x, int y, bool on) {
  if (cells_.empty() || y < 0 || y >= height_)
    return;
  if (antiGrav_.empty()) {
    if (!on)
      return;
    antiGrav_.assign(cells_.size(), 0);
  }
  antiGrav_[index(x, y)] = on ? 1 : 0;
}

std::size_t Terrain::antiGravCells() const {
  std::size_t n = 0;
  for (std::uint8_t b : antiGrav_)
    if (b)
      ++n;
  return n;
}

bool Terrain::noDigUpright(int x, int y) const {
  if (noDigUpright_.empty() || y < 0 || y >= height_)
    return false;
  return noDigUpright_[index(x, y)] != 0;
}

void Terrain::setNoDigUpright(int x, int y, bool on) {
  if (cells_.empty() || y < 0 || y >= height_)
    return;
  if (noDigUpright_.empty()) {
    if (!on)
      return;
    noDigUpright_.assign(cells_.size(), 0);
  }
  noDigUpright_[index(x, y)] = on ? 1 : 0;
}

bool Terrain::noDigInverted(int x, int y) const {
  if (noDigInverted_.empty() || y < 0 || y >= height_)
    return false;
  return noDigInverted_[index(x, y)] != 0;
}

void Terrain::setNoDigInverted(int x, int y, bool on) {
  if (cells_.empty() || y < 0 || y >= height_)
    return;
  if (noDigInverted_.empty()) {
    if (!on)
      return;
    noDigInverted_.assign(cells_.size(), 0);
  }
  noDigInverted_[index(x, y)] = on ? 1 : 0;
}

bool Terrain::digRefused(int x, int y, int verticalDir) const {
  // 0x00415A86 stores 0x10; 0x00415A96 stores 0x20 when [+0x44] == -1.
  if (verticalDir < 0)
    return noDigInverted(x, y);
  return noDigUpright(x, y);
}

bool Terrain::trampoline(int x, int y) const {
  if (trampoline_.empty() || y < 0 || y >= height_)
    return false;
  return trampoline_[index(x, y)] != 0;
}

void Terrain::setTrampoline(int x, int y, bool on) {
  if (cells_.empty() || y < 0 || y >= height_)
    return;
  if (trampoline_.empty()) {
    if (!on)
      return;
    trampoline_.assign(cells_.size(), 0);
  }
  trampoline_[index(x, y)] = on ? 1 : 0;
}

std::size_t Terrain::trampolineCells() const {
  std::size_t n = 0;
  for (std::uint8_t b : trampoline_)
    if (b)
      ++n;
  return n;
}

bool Terrain::waterHazard(int x, int y) const {
  if (waterHazard_.empty() || y < 0 || y >= height_)
    return false;
  return waterHazard_[index(x, y)] != 0;
}

void Terrain::setWaterHazard(int x, int y, bool on) {
  if (cells_.empty() || y < 0 || y >= height_)
    return;
  if (waterHazard_.empty()) {
    if (!on)
      return;
    waterHazard_.assign(cells_.size(), 0);
  }
  waterHazard_[index(x, y)] = on ? 1 : 0;
}

std::size_t Terrain::waterHazardCells() const {
  std::size_t n = 0;
  for (std::uint8_t b : waterHazard_)
    if (b)
      ++n;
  return n;
}

bool Terrain::zoneWater(int x, int y) const {
  if (zoneWater_.empty() || y < 0 || y >= height_)
    return false;
  return zoneWater_[index(x, y)] != 0;
}

void Terrain::setZoneWater(int x, int y, bool on) {
  if (cells_.empty() || y < 0 || y >= height_)
    return;
  if (zoneWater_.empty()) {
    if (!on)
      return;
    zoneWater_.assign(cells_.size(), 0);
  }
  zoneWater_[index(x, y)] = on ? 1 : 0;
}

std::size_t Terrain::zoneWaterCells() const {
  std::size_t n = 0;
  for (std::uint8_t b : zoneWater_)
    if (b)
      ++n;
  return n;
}

bool Terrain::zoneAcid(int x, int y) const {
  if (zoneAcid_.empty() || y < 0 || y >= height_)
    return false;
  return zoneAcid_[index(x, y)] != 0;
}

void Terrain::setZoneAcid(int x, int y, bool on) {
  if (cells_.empty() || y < 0 || y >= height_)
    return;
  if (zoneAcid_.empty()) {
    if (!on)
      return;
    zoneAcid_.assign(cells_.size(), 0);
  }
  zoneAcid_[index(x, y)] = on ? 1 : 0;
}

std::size_t Terrain::zoneAcidCells() const {
  std::size_t n = 0;
  for (std::uint8_t b : zoneAcid_)
    if (b)
      ++n;
  return n;
}

bool Terrain::splatExempt(int x, int y, int sheetType) const {
  // FUN_00416433: (0x200 && sheet==1) || (0x4000 && sheet==2).
  if (sheetType == 1)
    return zoneWater(x, y);
  if (sheetType == 2)
    return zoneAcid(x, y);
  return false;
}

// The bytes at 0x004B1528, read out of the stock exe. Written out here rather
// than computed, because the original stores them: there is no formula behind
// the shape, and `tools/harness/measure_bomb.py --stencil` prints the same 49
// bytes straight out of .rdata.
namespace {
constexpr int kExplosionSize = 7;
constexpr std::uint8_t kExplosionStencil[kExplosionSize * kExplosionSize] = {
    0, 0, 1, 1, 1, 0, 0,
    0, 1, 1, 1, 1, 1, 0,
    1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1,
    0, 1, 1, 1, 1, 1, 0,
    0, 0, 1, 1, 1, 0, 0,
};
}  // namespace

std::size_t Terrain::clearExplosion(int x, int y) {
  std::size_t removed = 0;
  const int col0 = x - 4;
  const int row0 = y - 4;
  for (int i = 0; i < kExplosionSize; ++i) {
    const int cy = row0 + i;
    if (cy < 0 || cy >= height_)
      continue;
    for (int j = 0; j < kExplosionSize; ++j) {
      if (!kExplosionStencil[i * kExplosionSize + j])
        continue;
      const int cx = wrapX(col0 + j);
      // 0x0043F61B: an indestructible cell is skipped. 0x0043F63E: so is a
      // cell carrying a blocker's field -- a dying blocker's own four cells
      // are already gone by now (the exit hook runs when the fuse ends), but
      // a second blocker standing inside the window keeps its field.
      if (blockerField(cx, cy))
        continue;
      const Material m = at(cx, cy);
      if (m == Material::Empty || m == Material::Steel)
        continue;
      set(cx, cy, Material::Empty);
      ++removed;
    }
  }
  return removed;
}

std::size_t Terrain::clearBox(int x0, int y0, int w, int h) {
  std::size_t removed = 0;
  for (int y = y0; y < y0 + h; ++y) {
    if (y < 0 || y >= height_)
      continue;
    for (int x = x0; x < x0 + w; ++x) {
      const Material m = at(x, y);
      if (m == Material::Empty || m == Material::Steel)
        continue;
      set(x, y, Material::Empty);
      ++removed;
    }
  }
  return removed;
}

std::size_t Terrain::fillBox(int x0, int y0, int w, int h, Material m) {
  std::size_t filled = 0;
  for (int y = y0; y < y0 + h; ++y) {
    if (y < 0 || y >= height_)
      continue;
    for (int x = x0; x < x0 + w; ++x) {
      if (at(x, y) != Material::Empty)
        continue;
      set(x, y, m);
      ++filled;
    }
  }
  return filled;
}

std::size_t Terrain::solidCells() const {
  std::size_t n = 0;
  for (Material m : cells_)
    if (isSolid(m))
      ++n;
  return n;
}

Terrain Terrain::fromLevelGrid(std::span<const std::uint32_t> grid, int width, int height) {
  Terrain t(width, height);
  const std::size_t want = static_cast<std::size_t>(width) * height;
  const std::size_t n = std::min(want, grid.size());
  for (std::size_t i = 0; i < n; ++i)
    t.cells_[i] = (grid[i] == 0xFFFFFFFFu) ? Material::Empty : Material::Earth;
  return t;
}

Terrain Terrain::fromCollisionGrid(const CollisionGrid& grid) {
  // The file grid uses the engine's own Y: 0 at the bottom of the tube,
  // increasing upward. FUN_0040DE60 indexes row Y directly; a walker at
  // world Y 32.190 on MM_CLIMB_002 has solid at file row 31 and empty at
  // 32 (tools/harness/compare_tick.py, docs/scratch/phase3-round1.md).
  // Fall subtracts from Y; climb adds. Our Terrain and steppers are the
  // other way: y increases downward, ground is at y+1, fall does ++y.
  // worldToMaskPixel already maps with floor(worldH - y). Flip here so a
  // retail mask and those coordinates share one axis. Hand-built test
  // terrains are untouched.
  Terrain t(grid.width, grid.height);
  for (int y = 0; y < grid.height; ++y) {
    const int destY = grid.height - 1 - y;
    for (int x = 0; x < grid.width; ++x) {
      const std::uint32_t word = grid.at(x, y);
      // Round 9: 0x8000 is the anti-grav pad bit (kCellFlag0x8000). It can
      // sit on a solid cell (typical pad word 0x8003) or, rarely, alone; the
      // walk probe tests the bit, not solidity of that word beyond the
      // ground sample it already took.
      if (word & kCellFlag0x8000)
        t.setAntiGrav(x, destY, true);
      // Round 30: dig refuse 0x10 / 0x20 (kCellNoDigUpright / Inverted).
      if (word & kCellNoDigUpright)
        t.setNoDigUpright(x, destY, true);
      if (word & kCellNoDigInverted)
        t.setNoDigInverted(x, destY, true);
      // Round 14: 0x40 is the drown / water-hazard bit (kCellFlag0x40). It
      // usually rides a solid cell (word 0x41 on H18_mg_1 / M19_MT_1).
      if (word & kCellFlag0x40)
        t.setWaterHazard(x, destY, true);
      // Round 29: splat-exemption zones (kCellZoneType0 / Type2). Can sit on
      // air (word 0x200 alone) or ride solid; the fall land tests the bit,
      // not Material::Earth.
      if (word & kCellZoneType0)
        t.setZoneWater(x, destY, true);
      if (word & kCellZoneType2)
        t.setZoneAcid(x, destY, true);
      if (!grid.solid(x, y))
        continue;
      t.cells_[t.index(x, destY)] = (word & kCellIndestructible)
                                        ? Material::Steel
                                        : Material::Earth;
    }
  }
  return t;
}

}  // namespace webo::sim
