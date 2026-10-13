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
// Not yet determined
// ------------------
//   * What the two arrays actually are. They are **not** a terrain grid, which
//     is worth stating because it looked like one. Across the 102 levels they
//     take 30 distinct sizes whose entry counts have a greatest common divisor
//     of 16, so no constant width above 16 divides them all, and individual
//     counts factor badly: 5,392 is 16 x 337 with 337 prime, which admits no
//     sensible rectangle. Only 4 levels have the 27,648 entries a 192 x 72 x 2
//     reading would want. Given the 0xFFFFFFFF sentinels and the small value
//     set, per-face or per-polygon index data fits better -- but that is a
//     guess, and nothing here asserts it.
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
