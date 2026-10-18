// Tests for PolyList 09 parsing: size formula, repeated vertices, surface indices.
//
// Kind 5 is 3 vertices of 52 bytes plus a trailing u32 (160). Kind 7 is 4
// (212). The ranges pinned here were measured on all 102 retail levels; see
// tools/harness/read_polylist_tail.py and docs/scratch/phase2-polylist.md.

#include "assets/Box.h"
#include "assets/Cylinder.h"
#include "assets/LevelIndex.h"
#include "assets/PolyList.h"

#include <cmath>
#include <cstdlib>
#include <filesystem>
#include <format>
#include <iostream>
#include <set>
#include <string>
#include <string_view>

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

void testRetail(const fs::path& dir) {
  std::cout << "retail polylists\n";
  auto lib = webo::AssetLibrary::open(dir / "lemmings.box", dir / "patch.box");
  if (!lib) {
    check(false, lib.error());
    return;
  }

  std::size_t levels = 0, matchedBlocks = 0, verts = 0;
  std::size_t badKind = 0, badCount = 0, badCorners = 0, badUv = 0;
  std::size_t badFinite = 0, badRgb = 0, badA = 0, records = 0, alphaShared = 0;
  std::size_t cornerVerts[4] = {};
  std::size_t cornerInSurface[4] = {};
  std::uint32_t endMin = 0xffffffffu, endMax = 0;
  bool sawClimb = false, sawFloat = false, sawSmall = false;
  auto wholeByte = [](double x) {
    return std::isfinite(x) && x >= 0.0 && x <= 255.0 && x == std::trunc(x);
  };
  for (const auto& name : webo::levelFileNames(*lib)) {
    auto blob = lib->read(name);
    if (!blob)
      continue;
    auto mesh = webo::findCylinderMesh(*blob);
    auto polys = webo::findPolyLists(*blob);
    if (!mesh || !polys)
      continue;
    ++levels;
    // Stem match, so MM_FLOAT does not also count MM_FLOAT_CLIMB2.
    const auto isLevel = [&](std::string_view id) {
      const auto pos = name.find(id);
      return pos != std::string::npos && pos + id.size() < name.size() &&
             name[pos + id.size()] == '.';
    };
    if (isLevel("MM_CLIMB_002"))
      sawClimb = true;
    if (isLevel("MM_FLOAT"))
      sawFloat = true;
    if (isLevel("VH13_KF_1"))
      sawSmall = true;
    matchedBlocks += polys->matchedCount;
    const auto sc = mesh->surface.size();
    for (const auto& block : polys->blocks) {
      if (block.kind != 5 && block.kind != 7)
        ++badKind;
      if (block.records.size() != block.count)
        ++badCount;
      const std::uint32_t expect = block.kind == 5 ? 3u : block.kind == 7 ? 4u : 0u;
      for (const auto& rec : block.records) {
        ++records;
        if (rec.cornerCount != expect)
          ++badCorners;
        if (rec.endWord < endMin)
          endMin = rec.endWord;
        if (rec.endWord > endMax)
          endMax = rec.endWord;
        const double a0 = rec.corners[0].a;
        bool sameA = true;
        for (std::uint32_t c = 0; c < rec.cornerCount && c < 4; ++c) {
          const auto& v = rec.corners[c];
          ++verts;
          ++cornerVerts[c];
          if (v.index < sc)
            ++cornerInSurface[c];
          if (!std::isfinite(v.u) || !std::isfinite(v.v) || !std::isfinite(v.r) ||
              !std::isfinite(v.g) || !std::isfinite(v.b) || !std::isfinite(v.a))
            ++badFinite;
          if (!wholeByte(v.r) || !wholeByte(v.g) || !wholeByte(v.b))
            ++badRgb;
          if (!std::isfinite(v.a) || v.a < 0.0 || v.a > 255.0)
            ++badA;
          if (v.a != a0)
            sameA = false;
          if (!std::isfinite(v.u) || !std::isfinite(v.v) || v.u < -0.01 || v.u > 1.01 ||
              v.v < -0.01 || v.v > 1.01)
            ++badUv;
        }
        if (sameA)
          ++alphaShared;
      }
    }
  }
  check(badKind == 0, "every matched block is kind 5 or 7");
  check(badCount == 0, "record vectors match header counts");
  check(badCorners == 0, "kind 5 has 3 vertices and kind 7 has 4");
  check(badFinite == 0, std::format("every vertex double is finite (bad {})", badFinite));
  check(badRgb == 0, std::format("r, g, b are integers in [0, 255] (bad {})", badRgb));
  check(badA == 0, std::format("a is in [0, 255] (bad {})", badA));
  check(alphaShared * 100 >= records * 99,
        std::format("alpha shared across a record's vertices ({}/{})", alphaShared, records));
  check(endMax > 1000 && endMin != endMax,
        std::format("trailing u32 is a wide non-constant word [{}, {}]", endMin, endMax));
  // A few percent sit outside [0,1] (clamped by the GPU); require ≥90% inside.
  check(badUv * 10 < verts, std::format("UVs mostly in unit square (bad {}/{})", badUv, verts));
  check(levels == 102, std::format("102 levels (got {})", levels));
  check(sawClimb && sawFloat && sawSmall, "MM_CLIMB_002, MM_FLOAT, and VH13_KF_1 were in the 102");
  check(matchedBlocks > 20000, std::format("many matched blocks (got {})", matchedBlocks));
  for (int c = 0; c < 4; ++c) {
    const double frac =
        cornerVerts[c] ? 100.0 * static_cast<double>(cornerInSurface[c]) / cornerVerts[c] : 0;
    check(frac > 80.0, std::format("corner {} indices in surface pool ({:.1f}%)", c, frac));
    std::cout << std::format("  corner {}  {} verts, {:.1f}% in surface\n", c, cornerVerts[c],
                             frac);
  }
  std::cout << std::format("  {} levels, {} blocks, {} verts, trailing u32 [{}, {}]\n", levels,
                           matchedBlocks, verts, endMin, endMax);

  for (const auto& name : webo::levelFileNames(*lib)) {
    if (name.find("MM_CLIMB_002") == std::string::npos)
      continue;
    auto blob = lib->read(name);
    auto mesh = webo::findCylinderMesh(*blob);
    auto index = webo::indexLevel(*blob);
    auto polys = webo::findPolyLists(*blob);
    check(polys.has_value(), "climb polylists parse");
    check(polys->rawCount == 379, std::format("climb raw count 379 (got {})", polys->rawCount));
    check(polys->matchedCount > 300,
          std::format("climb matched >300 (got {})", polys->matchedCount));

    std::set<std::uint32_t> grid;
    for (auto v : index->grid0)
      if (v != 0xFFFFFFFFu)
        grid.insert(v);
    for (auto v : index->grid1)
      if (v != 0xFFFFFFFFu)
        grid.insert(v);

    std::size_t inGrid = 0, unique = 0;
    std::set<std::uint32_t> seen;
    bool sawWhite = false;
    for (const auto& block : polys->blocks) {
      for (const auto& rec : block.records) {
        for (std::uint32_t c = 0; c < rec.cornerCount; ++c) {
          const auto& v = rec.corners[c];
          if (seen.insert(v.index).second) {
            ++unique;
            if (grid.count(v.index))
              ++inGrid;
          }
          if (v.r == 255 && v.g == 255 && v.b == 255 && v.a == 255)
            sawWhite = true;
        }
      }
    }
    check(sawWhite, "climb has opaque white vertex colours");
    check(inGrid * 100 >= unique * 90,
          std::format("climb poly idxs mostly in grids ({}/{})", inGrid, unique));
    check(mesh->surface.size() == 897, "climb surface count");
    break;
  }
}

}  // namespace

int main() {
  const char* dir = std::getenv("WEBOLUTION_GAME_DIR");
  if (!dir) {
    std::cout << "retail polylists: SKIP (WEBOLUTION_GAME_DIR unset)\n";
  } else if (!fs::exists(fs::path(dir) / "lemmings.box")) {
    std::cout << "retail polylists: SKIP (no lemmings.box)\n";
  } else {
    testRetail(dir);
  }

  std::cout << std::format("{} checks, {} failures\n", g_checks, g_failures);
  return g_failures == 0 ? 0 : 1;
}
