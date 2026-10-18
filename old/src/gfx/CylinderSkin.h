// A level's actual artwork, wrapped on the tube.
//
// Each level names 48 skin tiles: 8 columns around the cylinder by 3 rows up
// it, for each of the inner and outer surfaces. The names carry their own
// position -- "Cylinder\MM_CLIMB_002\OUT_MM_CLIMB_002_3_1" is the outer
// surface, column 3, row 1 -- so this parses the name rather than relying on
// the slot index. The slot layout is regular (stride 2 within a block), but
// the name is self-describing and cannot drift out of step with the data.
//
// This draws the level as it looks. It does **not** know what is solid: every
// retail skin tile is fully opaque (`AlphaMode::Opaque`, no translucent
// texels), so alpha cannot be a collision mask. Walkable cells come from the
// surface lattice (see PLAN.md 1.6). The skin is scenery, drawn behind the
// simulation.

#pragma once

#include "assets/Cylinder.h"

#include <charconv>
#include <cstdint>
#include <expected>
#include <string>
#include <string_view>
#include <vector>

namespace webo {
class AssetLibrary;
}

namespace webo::gfx {

class Renderer;

// Where a tile sits, parsed out of its name.
struct TilePosition {
  int column = 0;
  int row = 0;
  bool outer = true;
  bool valid = false;
};

// "Cylinder\LEVEL\OUT_LEVEL_3_1" -> column 3, row 1, outer.
//
// Defined inline so it can be tested without a GL context. That is not
// fussiness: the first version of this silently returned column 0, row 0 for
// every tile -- it computed the indices and forgot to write them to the
// out-parameters -- and the result was a level that loaded all 24 tiles and
// stacked them in one corner. A test would have caught it immediately; the
// screenshot took longer.
inline TilePosition parseTilePosition(std::string_view tileName) {
  const auto index = [](std::string_view text, int& out) {
    if (text.empty())
      return false;
    int value = 0;
    const char* const first = text.data();
    const char* const last = text.data() + text.size();
    const auto result = std::from_chars(first, last, value);
    if (result.ec != std::errc{} || result.ptr != last || value < 0)
      return false;
    out = value;
    return true;
  };

  TilePosition pos;
  if (tileName.empty())
    return pos;

  // Keep only the last path component: the folder names repeat the level id
  // and would otherwise confuse the underscore split.
  const auto slash = tileName.find_last_of("\\/");
  const std::string_view leaf =
      slash == std::string_view::npos ? tileName : tileName.substr(slash + 1);

  // The leading "INN_" or "OUT_" says which surface.
  if (leaf.starts_with("OUT_"))
    pos.outer = true;
  else if (leaf.starts_with("INN_"))
    pos.outer = false;
  else
    return pos;

  // The trailing "_<column>_<row>".
  const auto lastUnderscore = leaf.find_last_of('_');
  if (lastUnderscore == std::string_view::npos || lastUnderscore == 0)
    return pos;
  const auto prevUnderscore = leaf.find_last_of('_', lastUnderscore - 1);
  if (prevUnderscore == std::string_view::npos)
    return pos;

  if (!index(leaf.substr(prevUnderscore + 1, lastUnderscore - prevUnderscore - 1), pos.column))
    return pos;
  if (!index(leaf.substr(lastUnderscore + 1), pos.row))
    return pos;

  pos.valid = true;
  return pos;
}

class CylinderSkin {
public:
  // Loads every non-empty tile of a level, both surfaces, and uploads it.
  std::expected<void, std::string> load(const AssetLibrary& lib, Renderer& renderer,
                                        const CylinderMesh& mesh);

  bool ready() const { return !tiles_.empty(); }
  std::size_t tileCount() const { return tiles_.size(); }
  std::size_t outerCount() const;
  std::size_t innerCount() const;

  // Draws the skin across the full circumference and the given height, inner
  // surface first so the outer one covers it where it is opaque. `segments` is
  // the subdivision per tile, since a quad cannot bend.
  void draw(Renderer& renderer, float worldWidth, float worldHeight, int segments = 16) const;

  const std::vector<std::string>& missing() const { return missing_; }

private:
  struct Tile {
    unsigned texture = 0;
    int column = 0;
    int row = 0;
    bool outer = true;
  };

  std::vector<Tile> tiles_;
  std::vector<std::string> missing_;
  int columns_ = CylinderMesh::kTileColumns;
  int rows_ = CylinderMesh::kTileRows;
};

}  // namespace webo::gfx
