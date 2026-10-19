// Restored asset-side coverage (PLAN.md Phase A, debt 2).
//
// A cylinder skin tile carries its own position in its name, and that is the
// whole of the skin's layout logic. The flat renderer's version of this
// function shipped with a bug that returned column 0, row 0 for every tile, so
// the level stacked itself in one corner; these are the checks that would have
// caught it in a second. The function was retired with old/src/gfx; this
// re-establishes the coverage against the production header src/gfx/TilePosition.h.
//
// Synthetic checks always run. The retail sweep runs only when
// WEBOLUTION_GAME_DIR points at an installation.

#include "assets/Box.h"
#include "assets/Cylinder.h"
#include "assets/LevelIndex.h"
#include "gfx/TilePosition.h"

#include <cstdlib>
#include <filesystem>
#include <format>
#include <iostream>
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

void testSynthetic() {
  std::cout << "tile positions (synthetic)\n";

  const auto outer = webo::gfx::parseTilePosition("Cylinder\\MM_CLIMB_002\\OUT_MM_CLIMB_002_3_1");
  check(outer.valid, "a well-formed outer tile parses");
  check(outer.column == 3, std::format("column is 3 (got {})", outer.column));
  check(outer.row == 1, std::format("row is 1 (got {})", outer.row));
  check(outer.outer, "and it is the outer surface");

  const auto inner = webo::gfx::parseTilePosition("Cylinder\\E06_KF_1\\INN_e06_kf_1_7_2");
  check(inner.valid, "an inner tile parses");
  check(inner.column == 7 && inner.row == 2, "with its own position");
  check(!inner.outer, "and is marked inner");

  // Column 0 row 0 is a real position, so it must be distinguishable from the
  // failure case -- which is exactly what the original bug blurred.
  const auto origin = webo::gfx::parseTilePosition("Cylinder\\L\\OUT_L_0_0");
  check(origin.valid && origin.column == 0 && origin.row == 0, "0,0 is a valid position");

  for (const char* bad : {"", "OUT_L", "OUT_L_1", "Cylinder\\L\\MID_L_1_1", "OUT_L_x_1",
                          "OUT_L_1_y", "OUT_L_-1_1", "L_1_1"}) {
    const auto pos = webo::gfx::parseTilePosition(bad);
    check(!pos.valid, std::format("'{}' is rejected", bad));
  }

  const auto slashes = webo::gfx::parseTilePosition("Cylinder/L/OUT_L_2_2");
  check(slashes.valid && slashes.column == 2 && slashes.row == 2, "forward slashes parse");
}

void testRetail(const fs::path& gameDir) {
  std::cout << "tile positions (retail)\n";

  auto opened = webo::AssetLibrary::open(gameDir / "lemmings.box", gameDir / "patch.box");
  if (!opened) {
    std::cerr << "  FAIL  could not open the installation: " << opened.error() << "\n";
    ++g_failures;
    return;
  }
  const webo::AssetLibrary& lib = *opened;

  std::size_t parsed = 0;
  std::size_t outOfGrid = 0;
  std::size_t unparsed = 0;
  std::size_t levelsSwept = 0;
  for (const auto& name : webo::levelFileNames(lib)) {
    auto blob = lib.read(name);
    if (!blob)
      continue;
    auto mesh = webo::findCylinderMesh(*blob);
    if (!mesh)
      continue;
    ++levelsSwept;
    const std::vector<std::string>* blocks[2] = {&mesh->tileTextures, &mesh->innerTileTextures};
    for (const auto* block : blocks) {
      for (const auto& tile : *block) {
        if (tile.empty())
          continue;
        const auto pos = webo::gfx::parseTilePosition(tile);
        if (!pos.valid) {
          ++unparsed;
          continue;
        }
        ++parsed;
        if (pos.column >= static_cast<int>(webo::CylinderMesh::kTileColumns) ||
            pos.row >= static_cast<int>(webo::CylinderMesh::kTileRows))
          ++outOfGrid;
      }
    }
  }
  std::cout << std::format("  {} levels swept, {} tile names parsed\n", levelsSwept, parsed);
  check(unparsed == 0, std::format("every retail tile name parses ({} did not)", unparsed));
  check(outOfGrid == 0,
        std::format("every position is inside the 8x3 grid ({} were not)", outOfGrid));
  check(parsed == 102 * 48, std::format("48 tiles per level across 102 levels (got {})", parsed));
}

}  // namespace

int main() {
  testSynthetic();

  if (const char* dir = std::getenv("WEBOLUTION_GAME_DIR")) {
    if (fs::exists(fs::path(dir) / "lemmings.box"))
      testRetail(dir);
    else
      std::cout << "tile positions (retail): skipped, no lemmings.box in " << dir << "\n";
  } else {
    std::cout << "tile positions (retail): skipped, WEBOLUTION_GAME_DIR is not set\n";
  }

  std::cout << std::format("{} checks, {} failures\n", g_checks, g_failures);
  return g_failures == 0 ? 0 : 1;
}
