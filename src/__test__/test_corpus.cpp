// Corpus baseline for the terrain, both readings of it.
//
// This is a change-detector, not a statement that the numbers are right.
//
// Two masks are surveyed. The **stored collision grid** is the one the
// original itself collides against, read out of the .LVL, and it is what the
// simulation now runs on: 39.7% solid at the median, and 99 of 102 levels
// take a lemming released from a parsed entrance without killing it once the
// grid is flipped onto the sim's y-down axis. The **lattice raster** is our
// own reconstruction from the surface vertices, which turns out to be the
// engine's render input rather than its terrain. On the 49858-word index
// array it is 27.5% solid at the median and 93 of 102 releasable. The
// numbers are a change detector, not a quality target.
//
// The invariant is chosen because it needs no running original: a lemming
// released from an entrance has to land on something, and past
// `Rules::splatDistance` the landing kills it. So every number here is a
// measurement of where the lattice reading stands, and the point of pinning
// them is that the next change to the face rule in `SurfaceMesh.cpp` moves
// one visibly instead of going unnoticed. PLAN.md 1.6 records what the
// survey does and does not support -- in particular, a tall hatch is
// legitimate on MM_FLOAT, where a floater survives any drop.
//
// Bounds are directional: a floor where an improvement should push the
// number up, a ceiling where it should push the number down. The parse
// counts are exact, because a level that stops parsing is a bug whichever
// way it moves.

#include "assets/Box.h"
#include "assets/LevelStats.h"
#include "sim/World.h"

#include <cstdlib>
#include <filesystem>
#include <format>
#include <iostream>
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

// The stored-grid figures were re-measured 2026-10-14 after
// Terrain::fromCollisionGrid started flipping file rows onto the sim's
// y-down axis (same map as worldToMaskPixel). The earlier pin (median
// drop 9, 81 releasable) searched +y on the unflipped file grid from a
// flipped entrance row, so it often hit a different solid band. Lattice
// figures are unchanged.
constexpr std::size_t kLevels = 102;
// The stored grid: every level has one, and it is real terrain everywhere.
constexpr std::size_t kGrids = 102;
constexpr std::size_t kGridSurvivableLevels = 99;
constexpr std::size_t kGridEntrancesWithFloor = 144;
constexpr std::size_t kGridEntrancesSurvivable = 138;
constexpr int kGridMedianDropCells = 13;
// Re-measured 2026-10-14 after indexLevel started reading the whole
// 49858-word array. VH13_KF_1 rasterises now; it was the empty mask.
constexpr std::size_t kNonEmptyMasks = 102;
constexpr std::size_t kSurvivableLevels = 93;
constexpr std::size_t kEntrances = 144;
constexpr std::size_t kEntrancesWithFloor = 144;
constexpr std::size_t kFloorlessEntrances = 0;
constexpr int kMedianDropRows = 26;

void testCorpusBaseline(const fs::path& dir) {
  std::cout << "corpus terrain baseline\n";
  auto lib = webo::AssetLibrary::open(dir / "lemmings.box", dir / "patch.box");
  if (!lib) {
    check(false, lib.error());
    return;
  }

  const int splat = webo::sim::World::Rules{}.splatDistance;
  auto rows = webo::surveyLevels(*lib, splat);
  if (!rows) {
    check(false, rows.error());
    return;
  }
  const auto c = webo::summariseLevels(*rows, splat);

  // Exact: the archive holds these levels and they all parse.
  check(c.levels == kLevels, std::format("{} level entries (got {})", kLevels, c.levels));
  check(c.parsed == kLevels, std::format("all {} parse (got {})", kLevels, c.parsed));
  check(c.nonEmptyMasks == kNonEmptyMasks,
        std::format("{} rasterise to a non-empty mask (got {})", kNonEmptyMasks,
                    c.nonEmptyMasks));

  // Floors: a better face rule should put more entrances on survivable
  // ground, never fewer.
  check(c.survivableLevels >= kSurvivableLevels,
        std::format("releasable levels >= {} (got {})", kSurvivableLevels,
                    c.survivableLevels));
  check(c.entrances >= kEntrances,
        std::format("entrances found >= {} (got {})", kEntrances, c.entrances));
  check(c.entrancesWithFloor >= kEntrancesWithFloor,
        std::format("entrances with floor >= {} (got {})", kEntrancesWithFloor,
                    c.entrancesWithFloor));

  // Ceilings: these should only ever shrink.
  check(c.floorlessEntrances <= kFloorlessEntrances,
        std::format("entrances with nothing below <= {} (got {})", kFloorlessEntrances,
                    c.floorlessEntrances));
  check(c.medianDropRows <= kMedianDropRows,
        std::format("median entrance drop <= {} rows (got {})", kMedianDropRows,
                    c.medianDropRows));

  // The stored grid. Exact where the data is the game's own, directional
  // where our reading of entrances could still improve.
  check(c.grids == kGrids, std::format("{} levels carry a grid (got {})", kGrids, c.grids));
  check(c.gridSurvivableLevels >= kGridSurvivableLevels,
        std::format("releasable on the real grid >= {} (got {})", kGridSurvivableLevels,
                    c.gridSurvivableLevels));
  check(c.gridEntrancesWithFloor >= kGridEntrancesWithFloor,
        std::format("real-grid entrances with floor >= {} (got {})", kGridEntrancesWithFloor,
                    c.gridEntrancesWithFloor));
  check(c.gridEntrancesSurvivable >= kGridEntrancesSurvivable,
        std::format("real-grid entrances survivable >= {} (got {})",
                    kGridEntrancesSurvivable, c.gridEntrancesSurvivable));
  check(c.gridMedianDropRows <= kGridMedianDropCells,
        std::format("real-grid median drop <= {} cells (got {})", kGridMedianDropCells,
                    c.gridMedianDropRows));
  // The stored grid is still the denser of the two (39.7% vs 27.5% at the
  // median). The old factor-of-five gap was the header[3] window, not a
  // property of the index array. This is a change detector, not a target.
  check(c.gridMedianSolidPercent > c.medianSolidPercent,
        std::format("the stored grid stays denser than the raster ({:.1f}% vs {:.1f}%)",
                    c.gridMedianSolidPercent, c.medianSolidPercent));

  // The one level whose ledge is confirmed against the running original
  // (PLAN.md 1.6, 1,231/1,231 foot cells). It must stay releasable, and it
  // must keep the ring stride the live check was made against.
  bool sawClimb = false;
  for (const auto& r : *rows) {
    if (r.shortName != "MM_CLIMB_002")
      continue;
    sawClimb = true;
    check(r.ringStride == 8224, std::format("climb ring stride 8224 (got {})", r.ringStride));
    check(r.entrancesWithFloor == r.entrances, "climb: every entrance has floor below it");
    check(r.medianDropRows >= 0 && r.medianDropRows <= splat,
          std::format("climb: entrance drop survivable (got {} rows, splat {})",
                      r.medianDropRows, splat));
  }
  check(sawClimb, "MM_CLIMB_002 is in the corpus");

  std::cout << std::format(
      "  raster: {} levels, {} non-empty, median solid {:.1f}%, median drop {} rows\n"
      "          {} releasable, {} entrances ({} with floor, {} survivable)\n"
      "  grid:   {} levels, median solid {:.1f}%, median drop {} cells\n"
      "          {} releasable, {} with floor, {} survivable\n",
      c.levels, c.nonEmptyMasks, c.medianSolidPercent, c.medianDropRows,
      c.survivableLevels, c.entrances, c.entrancesWithFloor, c.entrancesSurvivable,
      c.grids, c.gridMedianSolidPercent, c.gridMedianDropRows,
      c.gridSurvivableLevels, c.gridEntrancesWithFloor, c.gridEntrancesSurvivable);
}

}  // namespace

int main() {
  if (const char* dir = std::getenv("WEBOLUTION_GAME_DIR")) {
    if (fs::exists(fs::path(dir) / "lemmings.box"))
      testCorpusBaseline(dir);
    else
      std::cout << "corpus terrain baseline: skipped, no lemmings.box\n";
  } else {
    std::cout << "corpus terrain baseline: skipped, WEBOLUTION_GAME_DIR is not set\n";
  }

  std::cout << std::format("\n{} checks, {} failures\n", g_checks, g_failures);
  return g_failures == 0 ? 0 : 1;
}
