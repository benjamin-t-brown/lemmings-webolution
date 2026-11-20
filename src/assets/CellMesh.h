// Derives tube quads from the 256 x 96 collision grid (CollisionGrid.h) and CylinderMesh::surface.
// Original build:
//   FUN_004608BB  0x4608BB  32 buckets x 6 bands = 192 cells
//   FUN_00461E1D  0x461E1D  eight 0x10-byte part lists merged to 0x4ECA80 rectangles
//   FUN_004615E9  0x4615E9  left, right and top gap lists (0x4ECA68 / 0x4ECA58 / 0x4ECA48)
//   FUN_00463100  0x463100  whole-block "wall" quad at entry+8
//   FUN_00463410  0x463410  mesh0 (entry+0) and mesh4 (entry+4)
//   FUN_0043A31D  0x43A31D  get or create vertex by layer + (x + y*0x101)*2
//   FUN_0043A1C6  0x43A1C6  separate door records (x, y)
//   FUN_004136E0  0x4136E0  U = (x & 31) / 32, from T[i] = i/64
//   FUN_00413710  0x413710  V = 1 - (y & 31) / 32
// Layer 0 is inner, layer 1 outer. CellMeshQuad::coord is rectangle x0 at entry+0x88 / entry+0x98,
// compared with the camera column by 0x462465. CellMeshCell::inner is the INN pass's layer-0
// quad at entry+8, gated by entry+0xF8, with open-top UVs and colour 255.
// Cylinder chunk LevelIndex::grid0 is the 49,858-word u32[N] at world+0x42C (FUN_0043C930).
// It maps (x, y, layer) to a surface index or 0xFFFFFFFF. MM_CLIMB_002 has 897 populated slots.
// The R x 12 companion at world+0x43C (FUN_0043A1C6) is empty in retail levels.

#pragma once

#include "CollisionGrid.h"

#include <array>
#include <cstddef>
#include <cstdint>
#include <expected>
#include <span>
#include <string>
#include <utility>
#include <vector>

namespace webo {

// Per-corner indices into CylinderMesh::surface and UVs. FUN_004135C0 uses the
// same double for each colour channel by repeating the entry's two colour dwords.
struct CellMeshQuad {
  std::array<std::uint32_t, 4> index{};
  std::array<std::array<double, 2>, 4> uv{};
  double colour = 255.0;

  // Rectangle x0 is the signed low short of the mesh0 child at entry+0x88 or mesh4 child
  // at entry+0x98. At 0x462465, mesh0 draws coord < cameraCentre*8 and mesh4 draws coord >= cameraCentre*8.
  int coord = 0;
  bool gate = false;  // Face uses the movable door vertex records.
};

// One of the 32 x 6 cells. `x0`/`y0` are its collision-grid origin; `material`
// is `col + row*8` with `col = bucket >> 2` and `row = (bands - 1 - band) / 2`
// (FUN_004608BB 0x460A1B, stored at entry+0xC8).
struct CellMeshCell {
  int bucket = 0;
  int band = 0;
  int x0 = 0;
  int y0 = 0;
  std::uint32_t material = 0;
  // FUN_00463100's flag: false when the whole 8x16 block is one solid
  // rectangle, in which case no wall quad is emitted.
  bool wall = false;
  std::vector<CellMeshQuad> mesh0;
  std::vector<CellMeshQuad> mesh4;

  // FUN_00463100 emits the whole-block "wall" quad at entry+8. 0x4622FD submits it for INN
  // when entry+0xF8 is set. Four inner layer-0 vertices use open-top UVs and white modulation.
  bool hasInner = false;
  CellMeshQuad inner;
};

struct CellMeshes {
  std::vector<CellMeshCell> cells;
  std::uint32_t buckets = 0;
  std::uint32_t bands = 0;          // stored vertical bands (96 / 16 = 6)
  std::size_t mesh0Quads = 0;
  std::size_t mesh4Quads = 0;
  std::size_t innerQuads = 0;       // cells with the entry+8 whole-block quad
  std::size_t keySlots = 0;         // populated slots in the stored key table
  std::size_t keySlotsTouched = 0;  // distinct keys the rule resolves
  std::size_t unresolved = 0;       // lookups the stored table does not answer
  bool keySupportMatches = false;   // touched keys == populated keys
  // Unanswered key lookups across rebuildCellMeshCells calls. Zero when all required vertices exist.
  std::size_t rebuildUnresolved = 0;

  // Cells are stored bucket-major: `band + bucket * bands`.
  const CellMeshCell* at(int bucket, int band) const {
    if (bucket < 0 || band < 0 || static_cast<std::uint32_t>(bucket) >= buckets ||
        static_cast<std::uint32_t>(band) >= bands)
      return nullptr;
    return &cells[static_cast<std::size_t>(bucket) * bands + static_cast<std::size_t>(band)];
  }
};

// keyTable is LevelIndex::grid0. doorRecords is the FUN_0043A1C6 R x 12 (x, y, index) list,
// empty in retail. surfaceVertexCount bounds the indices into CylinderMesh::surface.
std::expected<CellMeshes, std::string> buildCellMeshes(
    std::span<const std::uint32_t> keyTable,
    std::span<const std::array<std::int32_t, 3>> doorRecords,
    const CollisionGrid& grid, std::size_t surfaceVertexCount);

// Regenerate selected (bucket, band) cells with the per-cell rule in buildCellMeshes and recount quads.
// Doors and gates invoke the original scan at 0x4615E9 via 0x462264.
// The caller chooses cells and must provide every needed key within surfaceVertexCount.
// Provisional runtime meshes split solid spans at steel boundaries to protect adjacent steel faces.
std::expected<void, std::string> rebuildCellMeshCells(
    CellMeshes& meshes, std::span<const std::uint32_t> keyTable,
    std::span<const std::array<std::int32_t, 3>> doorRecords, const CollisionGrid& grid,
    std::size_t surfaceVertexCount, std::span<const std::pair<int, int>> cells);

}  // namespace webo
