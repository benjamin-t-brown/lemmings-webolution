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
  // Above the world is open sky; below it is a solid probe sentinel.
  // World::stepFaller loses open-bottom falls before consulting that sentinel.
  if (y < 0)
    return Material::Empty;
  if (y >= height_)
    return Material::Steel;
  return cells_[index(x, y)];
}

void Terrain::set(int x, int y, Material m) {
  if (cells_.empty() || y < 0 || y >= height_)
    return;
  const auto cell = index(x, y);
  cells_[cell] = m;
  if (!bashRefusalFlags_.empty())
    bashRefusalFlags_[cell] = 0;
  ++version_;
}

void Terrain::setGateCollision(int x, int y, bool closed) {
  if (cells_.empty() || y < 0 || y >= height_) return;
  const auto cell = index(x, y);
  const auto word = gateResidualWords_.empty() ? 0u : gateResidualWords_[cell];
  // 0x80 inhibits landability even after OR 0x403, as in CollisionGrid::solid.
  const auto material = (word & kCellNotLandable) ? Material::Empty :
      closed ? Material::Steel : word ? Material::Earth : Material::Empty;
  if (cells_[cell] != material) {
    cells_[cell] = material;
    ++version_;
  }
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

bool Terrain::bashRefused(int x, int y, int facing) const {
  if (bashRefusalFlags_.empty() || y < 0 || y >= height_)
    return false;
  const auto mask = facing < 0 ? kCellNoBashLeft : kCellNoBashRight;
  return (bashRefusalFlags_[index(x, y)] & mask) != 0;
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

void Terrain::paintZoneWaterRect(double minX, double minY, double maxX, double maxY, bool set) {
  if (cells_.empty() || width_ <= 0 || height_ <= 0)
    return;
  const int x0 = static_cast<int>(minX);  // ftol 0x0049C5AC: truncation toward zero
  int x1 = static_cast<int>(maxX);
  // 0x004311EE..0x00431207 extends seam-crossing set rectangles by one
  // width. Clear painter 0x0043127E uses the raw bounds.
  if (set && static_cast<double>(x1) < minX)
    x1 += width_;
  const int y0 = static_cast<int>(minY);
  const int y1 = static_cast<int>(maxY);  // the bound is exclusive, as is x1
  for (int x = x0; x < x1; ++x) {
    for (int y = y0; y < y1; ++y) {
      // 0x0040DE60: rows past the bottom read height-1, negative rows read 0.
      const int original = y >= height_ ? height_ - 1 : (y < 0 ? 0 : y);
      const int destY = height_ - 1 - original;  // original-to-clone Y conversion
      if (zoneWater_.empty()) {
        if (!set)
          return;  // nothing can be cleared on an unallocated plane
        zoneWater_.assign(cells_.size(), 0);
      }
      zoneWater_[index(x, destY)] = set ? 1 : 0;
    }
  }
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

// The original stores the 49 crater stencil bytes at 0x004B1528.
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
      // Skip indestructible cells (0x0043F61B) and blocker fields (0x0043F63E).
      // The exploding blocker's exit hook clears its own field before the blast.
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
  // FUN_0040DE60 indexes the original's Y-up rows from the tube floor.
  // MM_CLIMB_002's file row 31 supports a walker at world Y 32.190.
  // Flip rows to match the Y-down steppers and worldToMaskPixel.
  Terrain t(grid.width, grid.height);
  t.gateResidualWords_.resize(static_cast<std::size_t>(grid.width) * grid.height);
  for (int y = 0; y < grid.height; ++y) {
    const int destY = grid.height - 1 - y;
    for (int x = 0; x < grid.width; ++x) {
      const std::uint32_t word = grid.at(x, y);
      const auto cell = t.index(x, destY);
      t.gateResidualWords_[cell] = word & ~0x403u;
      const auto bashFlags = word & (kCellNoBashRight | kCellNoBashLeft);
      if (bashFlags != 0) {
        if (t.bashRefusalFlags_.empty())
          t.bashRefusalFlags_.resize(t.cells_.size());
        t.bashRefusalFlags_[cell] = static_cast<std::uint8_t>(bashFlags);
      }
      // Import anti-grav bit 0x8000 (kCellFlag0x8000) independently of material.
      if (word & kCellFlag0x8000)
        t.setAntiGrav(x, destY, true);
      // Dig refusal bits 0x10 / 0x20 (kCellNoDigUpright / Inverted).
      if (word & kCellNoDigUpright)
        t.setNoDigUpright(x, destY, true);
      if (word & kCellNoDigInverted)
        t.setNoDigInverted(x, destY, true);
      // Import water-hazard bit 0x40 (kCellFlag0x40) independently of material.
      // H18_mg_1 / M19_MT_1 carry it on solid cells (word 0x41).
      if (word & kCellFlag0x40)
        t.setWaterHazard(x, destY, true);
      // Import splat-exemption bits (kCellZoneType0 / Type2), including on air cells.
      if (word & kCellZoneType0)
        t.setZoneWater(x, destY, true);
      if (word & kCellZoneType2)
        t.setZoneAcid(x, destY, true);
      if (!grid.solid(x, y))
        continue;
      t.cells_[cell] = (word & kCellIndestructible) ? Material::Steel : Material::Earth;
    }
  }
  return t;
}

}  // namespace webo::sim
