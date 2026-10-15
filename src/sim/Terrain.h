// The terrain a lemming walks on, digs through and builds across.
//
// Revolution wraps its level around a cylinder, but the simulation does not
// need to know that: walking far enough to the right brings you back to where
// you started, which is exactly a 2D grid whose X axis wraps. So terrain here
// is a flat mask with horizontal wraparound and a hard top and bottom. The
// cylinder is the renderer's problem.
//
// Cells carry a material rather than a bool, because the original stores one
// (its grid holds 0xFFFFFFFF for empty and small indices otherwise) and
// because some materials resist digging.

#pragma once

#include <cstddef>
#include <cstdint>
#include <span>
#include <vector>

namespace webo {
struct CollisionGrid;
}

namespace webo::sim {

enum class Material : std::uint8_t {
  Empty = 0,
  Earth,      // diggable
  Rock,       // blocks bashers and miners
  Steel,      // blocks everything
  Built,      // placed by a builder; diggable like earth
};

inline bool isSolid(Material m) { return m != Material::Empty; }
inline bool isDiggable(Material m) {
  return m == Material::Earth || m == Material::Built;
}

class Terrain {
public:
  Terrain() = default;
  Terrain(int width, int height, Material fill = Material::Empty);

  int width() const { return width_; }
  int height() const { return height_; }
  bool empty() const { return cells_.empty(); }

  // X wraps; Y does not. Reads outside the top are Empty, below the bottom
  // Steel, so a lemming cannot fall out of the world or climb past the sky.
  Material at(int x, int y) const;
  bool solid(int x, int y) const { return isSolid(at(x, y)); }

  void set(int x, int y, Material m);

  // Clears a filled circle, the shape a bomber leaves. Returns cells removed.
  std::size_t clearCircle(int cx, int cy, int radius);
  // Clears an axis-aligned box, the shape bashers and diggers carve.
  std::size_t clearBox(int x, int y, int w, int h);
  // Fills a box, which is what a builder's brick is.
  std::size_t fillBox(int x, int y, int w, int h, Material m);

  // Normalises an X to [0, width).
  int wrapX(int x) const;

  std::size_t solidCells() const;
  std::span<const Material> cells() const { return cells_; }

  // Builds terrain from the surface lattice's index array: 0xFFFFFFFF means
  // empty and anything else is solid. That array is the engine's *render*
  // input, so this is not how a level's terrain is built -- see
  // fromCollisionGrid. Kept because the lattice tooling still uses it.
  static Terrain fromLevelGrid(std::span<const std::uint32_t> grid, int width, int height);

  // Builds terrain from the collision mask stored in the .LVL -- the one the
  // original itself collides against (assets/CollisionGrid.h). Solidity is
  // the engine's own test, and the material comes from the one flag bit that
  // carries a material distinction:
  //
  //   not solid            -> Empty   (zero, or 0x80 not-landable)
  //   0x2 indestructible   -> Steel   (tested first at 0x004248E6, never cleared)
  //   otherwise            -> Earth
  //
  // Rock has no counterpart in the grid. The engine draws one line, between
  // destructible and not, so inventing a second here would be our rule and
  // not the original's; Rock stays unused by this path.
  static Terrain fromCollisionGrid(const CollisionGrid& grid);

private:
  std::size_t index(int x, int y) const;

  int width_ = 0;
  int height_ = 0;
  std::vector<Material> cells_;
};

}  // namespace webo::sim
