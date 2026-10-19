// Tests for the per-cell tube mesh rule (src/assets/CellMesh).
//
// Everything asserted here is derived from stored level data: the collision
// grid, the `(x, y, layer) -> pool index` key table (LevelIndex::grid0) and the
// surface pool (CylinderMesh::surface). No captured arena bytes are read.
//
// The two file-derived invariants that pin the rule:
//
//   1. The rule resolves exactly the key-table slots the level populates and no
//      others. On MM_CLIMB_002 that is 897 of the 49,858 slots, and a wrong
//      scan (for example reading a gap's start where its end belongs) asks for
//      keys the level does not hold, so this fails.
//   2. The mesh0 / mesh4 quad counts and the quads-per-cell histogram are
//      stable for the level.
//
// The retained A1 live arena match (389/389 mesh0 and 387/387 mesh4 quads by
// index, UV, colour and order) is an offline oracle check, not a production
// input: `tools/harness/generate_cellmesh.py --compare`.

#include "assets/Box.h"
#include "assets/CellMesh.h"
#include "assets/CollisionGrid.h"
#include "assets/Cylinder.h"
#include "assets/LevelIndex.h"

#include <array>
#include <cstdint>
#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <map>
#include <set>
#include <string_view>
#include <vector>

namespace fs = std::filesystem;

namespace {

int g_failures = 0;
int g_checks = 0;

void check(bool cond, std::string_view what) {
  ++g_checks;
  if (!cond) {
    ++g_failures;
    std::cerr << "  FAIL  " << what << "\n";
  }
}

void testLevel(const fs::path& dir, std::string_view level) {
  std::cout << "cell mesh: " << level << "\n";
  auto lib = webo::AssetLibrary::open(dir / "lemmings.box", dir / "patch.box");
  if (!lib) {
    check(false, lib.error());
    return;
  }
  std::string name;
  for (const auto& candidate : webo::levelFileNames(*lib)) {
    if (candidate.find(std::string(level)) != std::string::npos) {
      name = candidate;
      break;
    }
  }
  check(!name.empty(), "the level is in the archive");
  if (name.empty())
    return;
  auto blob = lib->read(name);
  if (!blob) {
    check(false, "level bytes");
    return;
  }
  auto mesh = webo::findCylinderMesh(*blob);
  auto grid = webo::findCollisionGrid(*blob);
  auto index = webo::indexLevel(*blob);
  check(mesh.has_value(), "cylinder mesh parses");
  check(grid.has_value(), "collision grid parses");
  check(index.has_value(), "level index parses");
  if (!mesh || !grid || !index)
    return;

  auto built = webo::buildCellMeshes(index->grid0, {}, *grid, mesh->surface.size());
  check(built.has_value(), built ? "cell mesh builds" : built.error().c_str());
  if (!built)
    return;
  const auto& cells = *built;

  std::cout << "  key slots " << cells.keySlots << " touched " << cells.keySlotsTouched
            << " unresolved " << cells.unresolved << "\n";
  std::cout << "  mesh0 " << cells.mesh0Quads << " mesh4 " << cells.mesh4Quads << "\n";

  check(cells.unresolved == 0, "every vertex lookup is answered by the stored table");
  check(cells.keySupportMatches,
        "the rule touches exactly the key-table slots the level populates");
  check(cells.keySlotsTouched == cells.keySlots, "all populated slots are touched");

  if (level == "MM_CLIMB_002") {
    check(cells.keySlots == 897, "MM_CLIMB_002 populates 897 key slots");
    check(cells.mesh0Quads == 389, "MM_CLIMB_002 mesh0 has 389 quads");
    check(cells.mesh4Quads == 387, "MM_CLIMB_002 mesh4 has 387 quads");
    check(cells.buckets == 32, "32 angular buckets");
    check(cells.bands == 6, "6 stored bands");
    check(cells.innerQuads == 98, "MM_CLIMB_002 emits 98 whole-block inner quads");

    std::map<std::size_t, std::size_t> histogram;
    std::set<std::uint32_t> used;
    std::size_t both = 0;
    for (const auto& cell : cells.cells) {
      histogram[cell.mesh0.size()] += 1;
      if (!cell.mesh0.empty() && !cell.mesh4.empty())
        ++both;
      for (const auto& q : cell.mesh0)
        for (std::uint32_t i : q.index)
          used.insert(i);
      for (const auto& q : cell.mesh4)
        for (std::uint32_t i : q.index)
          used.insert(i);
    }
    const std::map<std::size_t, std::size_t> want = {
        {0, 29}, {1, 96}, {2, 36}, {3, 7}, {4, 3}, {5, 4},
        {6, 6}, {8, 3}, {11, 2}, {12, 1}, {14, 1}, {15, 4}};
    check(histogram == want, "MM_CLIMB_002 mesh0 quads/cell histogram");
    check(both == 163, "MM_CLIMB_002 cells carrying both banks (163)");
    // The mesh banks reach 802 of the 897 pool vertices; the whole-block inner
    // quads reach 131, of which 36 are shared with the banks, leaving the 95 the
    // A4c check measured as reachable only through FUN_00463100's wall path.
    std::set<std::uint32_t> innerUsed;
    for (const auto& cell : cells.cells)
      if (cell.hasInner)
        for (std::uint32_t i : cell.inner.index)
          innerUsed.insert(i);
    std::set<std::uint32_t> innerOnly;
    for (std::uint32_t i : innerUsed)
      if (used.count(i) == 0)
        innerOnly.insert(i);
    std::set<std::uint32_t> together = used;
    together.insert(innerUsed.begin(), innerUsed.end());
    std::cout << "  distinct pool indices referenced by the meshes " << used.size() << "\n";
    std::cout << "  distinct pool indices referenced by the inner quads " << innerUsed.size()
              << " (" << innerOnly.size() << " of them not in the banks)\n";
    check(used.size() == 802, "the mesh banks reference 802 of the 897 pool vertices");
    check(used.size() + 95 == cells.keySlots,
          "the other 95 pool vertices belong to the whole-block wall quads");
    check(innerUsed.size() == 131, "the 98 inner quads reference 131 pool vertices");
    check(innerOnly.size() == 95, "95 of those are reachable only through the inner quads");
    check(together.size() == cells.keySlots,
          "the banks and the inner quads together cover every populated key");

    // The two draw-route derivations, checked against the stored data.
    //
    // `coord` is the value the engine keeps in a parallel 4-byte child record
    // per quad (entry+0x88 / entry+0x98) and compares against the camera's own
    // column in `0x462465`. It is the x0 of the rectangle the quad came from, so
    // it must lie inside the cell's own 8 columns, and the assembly order puts
    // mesh0's in ascending and mesh4's in descending order. A wrong rule (for
    // instance taking the record's x1, or the gap's own column) breaks both.
    for (const auto& cell : cells.cells) {
      for (const auto& q : cell.mesh0) {
        check(q.coord >= cell.x0 && q.coord < cell.x0 + 8,
              "mesh0 child coord lies in the cell's own columns");
      }
      for (const auto& q : cell.mesh4) {
        check(q.coord >= cell.x0 && q.coord < cell.x0 + 8,
              "mesh4 child coord lies in the cell's own columns");
      }
      for (std::size_t i = 1; i < cell.mesh0.size(); ++i)
        check(cell.mesh0[i - 1].coord <= cell.mesh0[i].coord,
              "mesh0 child coords are non-decreasing");
      for (std::size_t i = 1; i < cell.mesh4.size(); ++i)
        check(cell.mesh4[i - 1].coord >= cell.mesh4[i].coord,
              "mesh4 child coords are non-increasing");
    }

    // `CellMeshCell::inner` is FUN_00463100's whole-block quad at entry+8: four
    // LAYER-0 (inner shell) vertices over the full 8x16 block, in the open-top
    // order and with the open-top UV formula. Resolve every index back through
    // the stored key table and check the geometry and the UVs; a layer-1
    // (outer shell) quad, a per-record extent instead of the block's, or the
    // closed-top UV order all fail here.
    std::vector<std::int64_t> indexToKey(mesh->surface.size(), -1);
    for (std::size_t k = 0; k < index->grid0.size(); ++k)
      if (index->grid0[k] != 0xFFFFFFFFu && index->grid0[k] < indexToKey.size())
        indexToKey[index->grid0[k]] = static_cast<std::int64_t>(k);
    const auto decode = [&](std::uint32_t poolIndex, int& x, int& y, int& layer) {
      const std::int64_t key =
          poolIndex < indexToKey.size() ? indexToKey[poolIndex] : std::int64_t{-1};
      if (key < 0)
        return false;
      layer = static_cast<int>(key % 2);
      const std::int64_t rest = (key - layer) / 2;
      y = static_cast<int>(rest / 0x101);
      x = static_cast<int>(rest % 0x101);
      return true;
    };
    std::size_t innerChecked = 0;
    for (const auto& cell : cells.cells) {
      if (!cell.hasInner)
        continue;
      ++innerChecked;
      const int x1 = cell.x0 + 8;
      const int y1 = cell.y0 + 16;
      const std::array<std::array<int, 3>, 4> want = {
          {{cell.x0, cell.y0, 0}, {cell.x0, y1, 0}, {x1, y1, 0}, {x1, cell.y0, 0}}};
      for (int i = 0; i < 4; ++i) {
        int x = 0, y = 0, l = 0;
        const bool ok = decode(cell.inner.index[i], x, y, l);
        check(ok, "inner quad index resolves through the stored key table");
        check(ok && x == want[i][0] && y == want[i][1] && l == want[i][2],
              "inner quad corner is the block's layer-0 rectangle");
      }
      const double u0 = (cell.x0 & 31) / 32.0;
      const double u1raw = (x1 & 31) / 32.0;
      const double u1 = u1raw == 0.0 ? 1.0 : u1raw;
      const double v0 = 1.0 - (cell.y0 & 31) / 32.0;
      double v1 = 1.0 - (y1 & 31) / 32.0;
      if (v1 == 1.0)
        v1 = 0.0;
      const std::array<std::array<double, 2>, 4> wuv = {
          {{u0, v0}, {u0, v1}, {u1, v1}, {u1, v0}}};
      for (int i = 0; i < 4; ++i)
        check(cell.inner.uv[i][0] == wuv[i][0] && cell.inner.uv[i][1] == wuv[i][1],
              "inner quad UV is the open-top formula over the whole block");
      check(cell.inner.colour == 255.0, "inner quad modulate is white");
      check(cell.inner.coord == cell.x0, "inner quad child coord is the block's x0");
    }
    check(innerChecked == 98, "all 98 wall-flagged cells carry an inner quad");

    // The discriminating check on `coord`: it is at or below the x of the
    // quad's first corner. A record's top quad starts at the record's x0, so
    // that one is equality; a mesh0 right-side gap starts at x1+1 and a top gap
    // at its own start column, both strictly above. Putting x1 in `coord`
    // instead of x0 pushes every top quad above its own first corner and fails
    // here, which the column-range and monotonicity checks above do not catch.
    std::size_t coordAtFirstCorner = 0;
    for (const auto& cell : cells.cells) {
      for (const auto* bank : {&cell.mesh0, &cell.mesh4}) {
        for (const auto& q : *bank) {
          int x = 0, y = 0, l = 0;
          if (!decode(q.index[0], x, y, l))
            continue;
          check(q.coord <= x, "child coord is at or below the quad's first corner");
          if (q.coord == x)
            ++coordAtFirstCorner;
        }
      }
    }
    check(coordAtFirstCorner == 709,
          "709 of the 776 bank quads begin on the column their coord names");
  }

  for (const auto& cell : cells.cells) {
    check(cell.hasInner == cell.wall,
          "the inner whole-block quad is emitted exactly on the wall flag");
    for (const auto& q : cell.mesh0)
      for (std::uint32_t i : q.index)
        check(i < mesh->surface.size(), "mesh0 index inside the pool");
    for (const auto& q : cell.mesh4)
      for (std::uint32_t i : q.index)
        check(i < mesh->surface.size(), "mesh4 index inside the pool");
  }
}

}  // namespace

int main() {
  const char* env = std::getenv("WEBOLUTION_GAME_DIR");
  if (env == nullptr || *env == '\0') {
    std::cout << "cellmesh: skipped (set WEBOLUTION_GAME_DIR to the install)\n";
    return 0;
  }
  const fs::path dir(env);
  if (!fs::exists(dir / "lemmings.box")) {
    std::cout << "cellmesh: skipped (no lemmings.box under " << dir.string() << ")\n";
    return 0;
  }
  testLevel(dir, "MM_CLIMB_002");
  std::cout << (g_failures == 0 ? "cellmesh: OK" : "cellmesh: FAILED") << " (" << g_checks
            << " checks)\n";
  return g_failures == 0 ? 0 : 1;
}