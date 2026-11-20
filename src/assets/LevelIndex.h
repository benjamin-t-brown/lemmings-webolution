// Partial .LVL index. indexLevel scans for strings and reads world.first_integer_array
// as grid0 in section order. A recovered reader's invalid_argument enables the legacy
// "Cylinder" locator. An absent fallback array still yields a partial index.
// A u32-length-prefixed title ("Level 23") precedes seven header words and a counted name
// list starting with "_curNoLem". TEXTURES\<reference>.BMP resolves asset names.
// Retail word[0] repeats the name count, word[2] - word[1] == 0x1B0000, and word[3] == word[4].
// word[3] (header[3]) and word[4] are multiples of 64 with unresolved meaning.
// The "Cylinder" preamble (CollisionGrid.h) has u32[5] zeros, height 96, f64 0.36,
// then one counted u32[N] surface-index array (49858 in retail). It maps keys to
// CylinderMesh::surface indices or 0xFFFFFFFF. The ring geometry in Cylinder.h is separate.
// On M18_MT_1 the highest index plus one is 99.3% of the surface pool.
// The collision grid follows R 12-byte records. grid1 stays empty. The grid is followed by 12288 light-map bytes.
// key+1 crosses shells and key+16 advances an angular step. SurfaceMesh::detectRingStride
// derives the ring stride, e.g. 8224 on MM_CLIMB_002, MM_FLOAT, MM_BUILD and VH13_KF_1.
// The measured displacement for key+8224 is 16 world units. lrtool lvlkeys reports these distances.
// Scripts and some Object 09 subtypes (fish, teleport_fx) remain unresolved here.
// Retail INN/OUT tiles are opaque. PolyList 09 describes render UV and RGBA (PolyList.h).
// LevelObjects.h covers placed models and skill counts.

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

  // Tube skin tile, e.g. "Cylinder\MM_CLIMB_002\INN_MM_CLIMB_002_0_0".
  std::vector<std::string> cylinderTextures;
  // "Models\bg_sand_medium\sand_1", "Models\exit\basket", ...
  std::vector<std::string> modelTextures;
  // Sigil-prefixed script variables: "_curNoLem", "^tab", "~splat".
  std::vector<std::string> scriptVariables;
  // Bare names: sound cues ("Splat", "Yippee") and sprite sheets ("Axe").
  std::vector<std::string> shortNames;

  std::size_t polyListCount = 0;

  // Preserve the world's first_integer_array words and positions. Consumers use 0xFFFFFFFF for empty slots.
  std::vector<std::uint32_t> grid0;
  // Kept for the two-span interface. indexLevel leaves it empty.
  std::vector<std::uint32_t> grid1;

  // Empty on ordered success. Otherwise records the reader rejection and cursor even if no fallback array is found.
  std::string gridFallbackReason;

  // Cells in grid1 that are not the empty marker (0xFFFFFFFF).
  std::size_t grid1SolidCells() const;
};

// Maps a reference as stored to its archive entry:
//   "Models\exit\basket" -> "TEXTURES\MODELS\EXIT\BASKET.BMP"
std::string levelTextureAssetName(std::string_view reference);

// High-detail cylinder variant:
//   "Cylinder\MM_CLIMB_002\OUT_MM_CLIMB_002_3_1"
//     -> "TEXTURES\CYLINDER\MM_CLIMB_002\OUT_MM_CLIMB_002_3_1HI.BMP"
// The .LVL stores an unsuffixed name for both "<name>.BMP" and "<name>HI.BMP".
// parseTilePosition reads its trailing column/row. Clone selection of HI follows the original image match.
std::string levelTextureAssetNameHi(std::string_view reference);

std::expected<LevelIndex, std::string> indexLevel(std::span<const std::byte>);
std::expected<LevelIndex, std::string> loadLevelIndex(const AssetLibrary&,
                                                      std::string_view name);

// Every "COMPILED\\*.LVL" in the archive.
std::vector<std::string> levelFileNames(const AssetLibrary&);

}  // namespace webo
