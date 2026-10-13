// Tests for PolyList 09 parsing: size formula, UV/RGBA doubles, surface indices.

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

  std::size_t levels = 0, matchedBlocks = 0, verts = 0, inSurface = 0;
  std::size_t badKind = 0, badCount = 0, badUv = 0;
  for (const auto& name : webo::levelFileNames(*lib)) {
    auto blob = lib->read(name);
    if (!blob)
      continue;
    auto mesh = webo::findCylinderMesh(*blob);
    auto polys = webo::findPolyLists(*blob);
    if (!mesh || !polys)
      continue;
    ++levels;
    matchedBlocks += polys->matchedCount;
    const auto sc = mesh->surface.size();
    for (const auto& block : polys->blocks) {
      if (block.kind != 5 && block.kind != 7)
        ++badKind;
      if (block.vertices.size() != block.count)
        ++badCount;
      for (const auto& v : block.vertices) {
        ++verts;
        if (v.index < sc)
          ++inSurface;
        if (!std::isfinite(v.u) || !std::isfinite(v.v) || v.u < -0.01 || v.u > 1.01 ||
            v.v < -0.01 || v.v > 1.01)
          ++badUv;
      }
    }
  }
  check(badKind == 0, "every matched block is kind 5 or 7");
  check(badCount == 0, "vertex vectors match header counts");
  // A few percent sit outside [0,1] (clamped by the GPU); require ≥90% inside.
  check(badUv * 10 < verts, std::format("UVs mostly in unit square (bad {}/{})", badUv, verts));
  check(levels == 102, std::format("102 levels (got {})", levels));
  check(matchedBlocks > 20000, std::format("many matched blocks (got {})", matchedBlocks));
  const double frac = verts ? 100.0 * static_cast<double>(inSurface) / verts : 0;
  check(frac > 80.0, std::format("most indices in surface pool ({:.1f}%)", frac));
  std::cout << std::format("  {} levels, {} blocks, {} verts, {:.1f}% in surface\n", levels,
                           matchedBlocks, verts, frac);

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
      for (const auto& v : block.vertices) {
        if (seen.insert(v.index).second) {
          ++unique;
          if (grid.count(v.index))
            ++inGrid;
        }
        if (v.r == 255 && v.g == 255 && v.b == 255 && v.a == 255)
          sawWhite = true;
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
