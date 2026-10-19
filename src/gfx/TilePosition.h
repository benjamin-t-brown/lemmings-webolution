// Where a cylinder skin tile sits, parsed out of its own name.
//
// A level names 48 skin tiles: 8 columns around the tube by 3 rows up it, for
// each of the inner and outer surfaces. The name carries its own position --
// "Cylinder\MM_CLIMB_002\OUT_MM_CLIMB_002_3_1" is the outer surface, column 3,
// row 1 -- so this parses the name rather than trusting the slot index. The
// slot layout is regular (stride 2 within a block) but the name is
// self-describing and cannot drift out of step with the data.
//
// Header-only so it can be tested without a GL context. That is not fussiness:
// the first version of this function computed the indices and forgot to write
// them to its out-parameters, so every tile came back as column 0, row 0 and
// the level stacked itself in one corner. This was retired with the flat
// renderer (old/src/gfx/CylinderSkin.h); A4 restores it as production code.

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

}  // namespace webo::gfx
