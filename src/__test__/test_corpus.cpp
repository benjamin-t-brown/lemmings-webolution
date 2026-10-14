// Corpus baseline for the terrain the surface lattice rebuilds.
//
// This is a change-detector, not a statement that the numbers are right.
// The current state is bad: on most of the 102 retail levels the parsed
// entrance sits a fatal drop above anything solid in our mask -- the median
// is 132 mask rows, which is 66 world units against a splat distance of 60
// rows. Only 22 of the 101 levels that rasterise to anything could have a
// lemming released into them and survive the landing.
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

// Measured 2026-10-14 by `lrtool lvlstats`, which shares this survey code.
constexpr std::size_t kLevels = 102;
constexpr std::size_t kNonEmptyMasks = 101;  // VH13_KF_1 rasterises to nothing
constexpr std::size_t kSurvivableLevels = 22;
constexpr std::size_t kEntrances = 143;
constexpr std::size_t kEntrancesWithFloor = 132;
constexpr std::size_t kFloorlessEntrances = 11;
constexpr int kMedianDropRows = 132;

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
      "  {} levels, {} non-empty masks, median solid {:.1f}%, median drop {} rows\n"
      "  {} releasable, {} entrances ({} with floor, {} survivable, {} with nothing below)\n",
      c.levels, c.nonEmptyMasks, c.medianSolidPercent, c.medianDropRows, c.survivableLevels,
      c.entrances, c.entrancesWithFloor, c.entrancesSurvivable, c.floorlessEntrances);
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
