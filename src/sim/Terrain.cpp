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

std::size_t Terrain::clearCircle(int cx, int cy, int radius) {
  if (radius <= 0)
    return 0;
  std::size_t removed = 0;
  const int r2 = radius * radius;
  for (int dy = -radius; dy <= radius; ++dy) {
    const int y = cy + dy;
    if (y < 0 || y >= height_)
      continue;
    for (int dx = -radius; dx <= radius; ++dx) {
      if (dx * dx + dy * dy > r2)
        continue;
      const int x = cx + dx;
      // Steel survives an explosion; everything else does not.
      const Material m = at(x, y);
      if (m == Material::Empty || m == Material::Steel)
        continue;
      set(x, y, Material::Empty);
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
  Terrain t(grid.width, grid.height);
  for (int y = 0; y < grid.height; ++y) {
    for (int x = 0; x < grid.width; ++x) {
      if (!grid.solid(x, y))
        continue;
      t.cells_[t.index(x, y)] = (grid.at(x, y) & kCellIndestructible) ? Material::Steel
                                                                     : Material::Earth;
    }
  }
  return t;
}

}  // namespace webo::sim
