// A first, deliberately partial reading of .LVL.
//
// The level format is large (700 KB - 1.9 MB each), undocumented, and carries
// several things at once: cylinder geometry, the terrain, placed models, a
// script, and sound and sprite references. It is not solved yet. This header
// exposes only what has been *verified* across all 102 retail levels, so that
// the rest of the engine has something real to build against while the format
// is finished.
//
// Verified
// --------
//   * The file opens with a u32-length-prefixed title ("Level 23").
//   * Asset references appear as u32-length-prefixed ASCII, and every one of
//     the 26,021 references across the 102 levels resolves to an entry that
//     exists in the archive, under TEXTURES\<reference>.BMP. That is what
//     makes the scan below trustworthy rather than a guess: a mis-scan would
//     produce references that do not resolve.
//   * A chunk named "Cylinder" is followed by two equal-sized arrays of u32
//     whose byte length is given twice in the file header. In the retail data
//     both are 110592 bytes, i.e. 27648 cells.
//
// Not yet determined
// ------------------
//   * The grids' width and height. 27648 factors many ways and rendering the
//     candidates was not conclusive, so no dimensions are asserted here.
//   * Which grid is the terrain. The second is far denser (roughly 6k of 27648
//     cells set, against 194 in the first), so it is the better candidate, but
//     that is an observation rather than a conclusion.
//   * The polygon lists, the placed-model transforms, and the script.
//
// The scan is a *heuristic index*, not a parser: it finds length-prefixed
// strings rather than walking a known structure. It is named accordingly.

#pragma once

#include <cstddef>
#include <cstdint>
#include <expected>
#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <vector>

namespace webo {

class AssetLibrary;

struct LevelIndex {
  std::string title;
  std::vector<std::uint32_t> header;  // the u32 words following the title

  // "Cylinder\MM_CLIMB_002\INN_MM_CLIMB_002_0_0" and friends: the inner and
  // outer skins of the tube, as a grid of tiles.
  std::vector<std::string> cylinderTextures;
  // "Models\bg_sand_medium\sand_1", "Models\exit\basket", ...
  std::vector<std::string> modelTextures;
  // Sigil-prefixed script variables: "_curNoLem", "^tab", "~splat".
  std::vector<std::string> scriptVariables;
  // Bare names: sound cues ("Splat", "Yippee") and sprite sheets ("Axe").
  std::vector<std::string> shortNames;

  std::size_t polyListCount = 0;

  // The two equal arrays after the "Cylinder" chunk. Dimensions unknown.
  std::vector<std::uint32_t> grid0;
  std::vector<std::uint32_t> grid1;

  // Cells in grid1 that are not the empty marker (0xFFFFFFFF).
  std::size_t grid1SolidCells() const;
};

// Maps a reference as stored to its archive entry:
//   "Models\exit\basket" -> "TEXTURES\MODELS\EXIT\BASKET.BMP"
std::string levelTextureAssetName(std::string_view reference);

std::expected<LevelIndex, std::string> indexLevel(std::span<const std::byte>);
std::expected<LevelIndex, std::string> loadLevelIndex(const AssetLibrary&, std::string_view name);

// Every "COMPILED\\*.LVL" in the archive.
std::vector<std::string> levelFileNames(const AssetLibrary&);

}  // namespace webo
