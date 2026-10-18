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
//   * Header invariants across all 102 levels: word[3] == word[4], word[2] -
//     word[1] is always 0x1B0000, and word[0] repeats the manifest count.
//     word[3] is a multiple of 64 and ranges from 20480 to 111936 (30 distinct
//     values). It is **not** the lattice's byte length. That reading was a
//     window into the array below; see docs/scratch/phase2-lattice.md.
//   * The literal "Cylinder" chunk starts a fixed walk, the same one
//     CollisionGrid.h uses to reach the mask:
//
//         u32 8, "Cylinder"
//         u32[5]     zeros, in all 102 levels
//         u32        height 96, in all 102 levels
//         f64        0.36, in all 102 levels
//         u32 N      49858, in all 102 levels
//         u32[N]     one surface-index array
//
//     Every word is 0xFFFFFFFF or an index into CylinderMesh::surface. The
//     array is occupied through its last word in all 102 levels (last
//     non-empty at 49856 or 49857). The tube mesh in Cylinder.h is a
//     different, earlier object; this array is not that mesh, and its bytes
//     are not the same from level to level.
//   * There is no second index array of length word[3]. grid1 is empty.
//     The collision grid is the next counted array after u32 R and R*12
//     records (CollisionGrid.h). A counted array of 12288 words follows the
//     collision payload in all 102 levels; it is not this lattice.
//
// Keying — settled
// ----------------
//   The non-empty entries are indices into the cylinder's surface vertices
//   (see Cylinder.h). Asserted on the whole 49858-word array in all 102
//   levels: every value is in range, and (highest + 1) / vertex count is at
//   worst 0.993 (M18_MT_1). The earlier 87%..99.8% band was the header[3]
//   window, which drops most of the pool on the short levels.
//
//   The *slots* are a cylindrical lattice, not a hash and not a flat terrain
//   bitmap. Neighbouring keys name neighbouring vertices. Re-measured on the
//   49858-word array, all 102 levels:
//
//     key+1   inner <-> outer (median distance within 1 of thickness 10)
//     key+16  next angular step (median within 1 of the chord at r=50 or r=60)
//
//   The array length is not a multiple of 16: 49858 = 16*3116 + 2. The old
//   "multiple of 16" note was header[3] / 4, which is a different number.
//   `lrtool lvlkeys` reports the median distances; see PLAN.md 1.6 for the
//   quad reconstruction.
//
//   The ring stride Y is not a stored .LVL field. On MM_CLIMB_002, MM_FLOAT,
//   MM_BUILD and VH13_KF_1, SurfaceMesh::detectRingStride returns 8224 and
//   the median key+8224 distance is exactly one ring (16). MM_CLIMB_002 has
//   no aligned u32 equal to 8224 or 8224/16 (test_surface).
//
// Not yet determined
// ------------------
//   * What header word[3] and word[4] are. They match each other in all 102
//     levels, but neither equals N*4 (199432) and neither is stored as a
//     count beside the Cylinder chunk.
//   * Script / remaining non-world Object 09 subtypes (fish, teleport_fx, …).
//
// Skin tiles do **not** encode solidity: all 4,896 named INN/OUT tiles across
// the corpus are opaque bitmaps with no transparency sigil (ruled out in
// PLAN.md 1.6 / test_surface).
//
// PolyList 09 chunks are render attributes for surface vertices (UV + RGBA),
// not a second collision mesh -- see PolyList.h.
//
// Entrances, exits, other placed models, and skill counts: see LevelObjects.h.
//
// The string sweep is a *heuristic index*, not a parser: it finds
// length-prefixed strings rather than walking a known structure. The
// surface-index array is the exception. It is the counted u32[N] on the
// Cylinder walk above, not a scan for 0xFFFFFFFF.

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

  // The u32[N] surface-index lattice after the Cylinder preamble. N is 49858
  // in every retail level. 0xFFFFFFFF is an empty slot.
  std::vector<std::uint32_t> grid0;
  // Not present in the file. Kept so callers can still pass both spans;
  // indexLevel leaves it empty.
  std::vector<std::uint32_t> grid1;

  // Cells in grid1 that are not the empty marker (0xFFFFFFFF).
  std::size_t grid1SolidCells() const;
};

// Maps a reference as stored to its archive entry:
//   "Models\exit\basket" -> "TEXTURES\MODELS\EXIT\BASKET.BMP"
std::string levelTextureAssetName(std::string_view reference);

std::expected<LevelIndex, std::string> indexLevel(std::span<const std::byte>);
std::expected<LevelIndex, std::string> loadLevelIndex(const AssetLibrary&,
                                                      std::string_view name);

// Every "COMPILED\\*.LVL" in the archive.
std::vector<std::string> levelFileNames(const AssetLibrary&);

}  // namespace webo
