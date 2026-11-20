// Parse the tile's surface, column and row from its name. Each surface has 8 columns and 3 rows.
// "Cylinder\MM_CLIMB_002\OUT_MM_CLIMB_002_3_1" is outer column 3, row 1.

#pragma once

#include <charconv>
#include <string_view>

namespace webo::gfx {

struct TilePosition {
  int column = 0;
  int row = 0;
  bool outer = true;
  bool valid = false;
};

// "Cylinder\LEVEL\OUT_LEVEL_3_1" -> column 3, row 1, outer.
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

  // Ignore directories because the level ID can appear in their names too.
  const auto slash = tileName.find_last_of("\\/");
  const std::string_view leaf =
      slash == std::string_view::npos ? tileName : tileName.substr(slash + 1);

  // "INN_" and "OUT_" select the surface.
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

}  // namespace webo::gfx
