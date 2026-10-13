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
//   * The title is followed by seven u32 header words, then a u32 count and
//     that many length-prefixed asset names -- the level's manifest. That
//     parse consumes exactly, in all 102 levels, and the manifest always
//     begins with "_curNoLem". An earlier reading of this header had eight
//     words: the eighth was really the manifest's count, and the ninth word
//     that "was always 9" was the length prefix of "_curNoLem".
//   * Header invariants across all 102 levels: word[3] == word[4] (the two
//     array byte lengths), word[2] - word[1] is always 0x1B0000, and word[0]
//     repeats the manifest count.
//   * A chunk named "Cylinder" is followed by two equal-sized arrays of u32
//     whose byte length is word[3]. The tube's own geometry *is* solved --
//     see Cylinder.h.
//
// Keying — settled
// ----------------
//   The non-empty entries of both arrays are indices into the cylinder's
//   surface vertices (see Cylinder.h). `grid0` is asserted in all 102 levels:
//   every value is in range and the largest lands just under the vertex count
//   (87%..99.8%, median 98.6%).
//
//   The *slots* are a cylindrical lattice, not a hash and not a flat terrain
//   bitmap. Neighbouring keys name neighbouring vertices:
//
//     key+1   inner <-> outer (distance ≈ thickness 10)
//     key+16  next angular step (distance ≈ chord at r=50 or r=60)
//     key+Y   next ring; Y is per-level (8224 on the ~27k arrays)
//
//   That is why every array length is a multiple of 16. `lrtool lvlkeys`
//   reports the median distances; see PLAN.md 1.6 for the quad reconstruction.
//
// Not yet determined
// ------------------
//   * The per-level ring stride Y, as a field rather than a search.
//   * Whether the walkable terrain is these faces, the skin tiles, or both.
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
