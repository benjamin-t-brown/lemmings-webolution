// The per-cell tube mesh the engine builds at level load.
//
// `Lemmings Revolution` does not store the tube's per-cell topology. It stores
// a 256 x 96 collision grid (`CollisionGrid.h`) and a 897..1558 vertex surface
// pool (`CylinderMesh::surface`), and derives the quads from them at load. This
// module is that derivation, read out of the shipped executable:
//
//   FUN_004608BB  0x4608BB  per-bucket sweep: 32 buckets x 6 bands = 192 cells
//   FUN_00461E1D  0x461E1D  the eight 0x10-byte part lists, merged into the
//                           0x4ECA80 records (one solid rectangle per entry)
//   FUN_004615E9  0x4615E9  the gap lists at the rectangle's left / right / top
//                           boundary (0x4ECA68 / 0x4ECA58 / 0x4ECA48)
//   FUN_00463100  0x463100  the whole-block "wall" quad at entry+8
//   FUN_00463410  0x463410  mesh0 (entry+0) and mesh4 (entry+4)
//   FUN_0043A31D  0x43A31D  get-or-create a vertex keyed `layer + (x + y*0x101)*2`
//   FUN_0043A1C6  0x43A1C6  the door path: a separate (x, y) record list
//   FUN_004136E0  0x4136E0  U = (x & 31) / 32     (the table is T[i] = i/64)
//   FUN_00413710  0x413710  V = 1 - (y & 31) / 32
//
// The `layer` is the cylinder template's inner (0) / outer (1) selector.
//
// Two further derivations the draw passes need, both read out of the shipped
// bytes and both matched against the retained arena:
//
//   * `CellMeshQuad::coord` -- the per-quad child record at entry+0x88 /
//     entry+0x98, whose signed low short `0x462465` compares against the
//     camera's own column. It is the `x0` of the rectangle the quad came from.
//   * `CellMeshCell::inner` -- FUN_00463100's whole-block quad at entry+8, the
//     INN pass's inline quad. Four layer-0 vertices over the block, open-top
//     UVs, colour 255, gated by the entry+0xF8 byte (the `wall` flag).
//
// What the vertex indices mean
// ----------------------------
// The `u32[N]` array on the `Cylinder` chunk walk (LevelIndex::grid0, 49858
// words in every retail level) is **not** a face list: it is the engine's
// `(x, y, layer) -> pool index` table at `world+0x42C`. Key `layer + (x +
// y*0x101)*2` (`FUN_0043C930`), value the index into `CylinderMesh::surface`,
// or 0xFFFFFFFF for "not present". On `MM_CLIMB_002` exactly 897 of the 49,858
// slots are populated, values 0..896, a permutation onto the whole surface
// pool. The rule below touches exactly those 897 slots and no others, which is
// the file-derived check `test_cellmesh` asserts.
//
// The companion `R x 12` record array (`world+0x43C`, `FUN_0043A1C6`) is empty
// in `MM_CLIMB_002` and in every retail level measured so far.
//
// Evidence: `tools/harness/generate_cellmesh.py` (the same rule in Python,
// with `--compare` against the retained A1 live arena and static pin
// assertions) and `docs/scratch/phase-a4-cell-mesh-source.md`.

#pragma once

#include "CollisionGrid.h"

#include <array>
#include <cstddef>
#include <cstdint>
#include <expected>
#include <span>
#include <string>
#include <vector>

namespace webo {

// One quad in a mesh bank. `index` are pool indices into
// `CylinderMesh::surface`; `uv` is per corner; `colour` is the single double
// the engine writes into all three channels (`FUN_004135C0` called with the
// entry's two colour dwords repeated three times).
struct CellMeshQuad {
  std::array<std::uint32_t, 4> index{};
  std::array<std::array<double, 2>, 4> uv{};
  double colour = 255.0;

  // The centre-pass filter key. Each bank has a parallel 4-byte child record
  // per quad at entry+0x88 (mesh0) / entry+0x98 (mesh4), whose signed low
  // short is this value -- the `x0` of the solid rectangle the quad was built
  // from. `0x462465` submits mesh0 quads with `coord < cameraCentre*8` and
  // mesh4 quads with `coord >= cameraCentre*8`, which is how one record is
  // drawn by exactly one bank at the camera's own bucket.
  int coord = 0;
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

  // The whole-block "wall" quad at entry+8, emitted by FUN_00463100 exactly
  // when `wall` is true and submitted as the INN pass's inline quad by
  // `0x4622FD` (which tests the byte at entry+0xF8, the same flag). Four
  // layer-0 (inner shell, radius ~50) vertices over the whole 8x16 block with
  // the open-top UV formula and a white modulate. In the retained arena this
  // is active on 98 of the 192 cells and on exactly the `wall` set.
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

  // Cells are stored bucket-major: `band + bucket * bands`.
  const CellMeshCell* at(int bucket, int band) const {
    if (bucket < 0 || band < 0 || static_cast<std::uint32_t>(bucket) >= buckets ||
        static_cast<std::uint32_t>(band) >= bands)
      return nullptr;
    return &cells[static_cast<std::size_t>(bucket) * bands + static_cast<std::size_t>(band)];
  }
};

// `keyTable` is LevelIndex::grid0; `doorRecords` is the `R x 12` array on the
// same walk (the `FUN_0043A1C6` (x, y, index) list, empty in every retail level
// measured); `surfaceVertexCount` is CylinderMesh::surface.size(), used only to
// bound-check the indices.
std::expected<CellMeshes, std::string> buildCellMeshes(
    std::span<const std::uint32_t> keyTable,
    std::span<const std::array<std::int32_t, 3>> doorRecords,
    const CollisionGrid& grid, std::size_t surfaceVertexCount);

}  // namespace webo
