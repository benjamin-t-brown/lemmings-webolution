// Corpus-wide measurement of the terrain the surface lattice rebuilds.
//
// This is an *offline oracle*, not a loader. It asks one question of every
// level, and the question is chosen because it needs no running original: a
// lemming released from a parsed entrance has to land on something, and a
// drop past `splatDistance` kills it. So for each entrance, is there a solid
// mask cell below it, and is the drop survivable?
//
// The answer is a measurement of where the lattice reading stands, not a
// statement that it is right or wrong. PLAN.md 1.6 records the numbers and
// what they do and do not support. `lrtool lvlstats` prints them and
// `test_corpus` holds them as a baseline, so a change to the lattice rule
// moves a number instead of going unnoticed.
//
// Deliberately free of any `src/sim` dependency: the splat distance is
// passed in by the caller, which reads it off `World::Rules`.
#pragma once

#include <cstddef>
#include <expected>
#include <span>
#include <string>
#include <vector>

namespace webo {

class AssetLibrary;

// One level's survey. Every count is of this level alone.
struct LevelStats {
  std::string name;      // archive entry, e.g. "COMPILED\\MM_CLIMB_002.LVL"
  std::string shortName; // basename without directory or extension

  // False when the cylinder, the index arrays or the face build did not
  // produce a mesh. Such a level contributes to no other count here.
  bool parsed = false;
  // Parsed, and the raster produced a mask with at least one solid cell.
  // A level can parse and still rasterise to nothing: VH13_KF_1 does.
  bool nonEmptyMask = false;

  int ringStride = 0; // 0 when no multiple of 16 landed near one ring spacing
  std::size_t quads = 0;
  std::size_t openEdges = 0;

  int maskWidth = 0;
  int maskHeight = 0;
  std::size_t solidCells = 0;
  double solidPercent = 0;

  std::size_t entrances = 0;
  std::size_t exits = 0;
  // Entrances with any solid cell somewhere below them in the mask column.
  std::size_t entrancesWithFloor = 0;
  // Of those, the ones whose drop is at most the splat distance.
  std::size_t entrancesSurvivable = 0;

  // Mask rows from the entrance down to the first solid cell, one per
  // entrance that has a floor at all, ascending. Empty when none does.
  std::vector<int> drops;
  // Median of `drops`, or -1 when it is empty.
  int medianDropRows = -1;

  // The same survey against the mask the original actually collides against,
  // read out of the .LVL (CollisionGrid.h). This is the real terrain; the
  // fields above describe our lattice raster, which is a different and much
  // sparser thing. Rows here are world units, because the stored grid is one
  // cell per unit.
  bool hasGrid = false;
  int gridHeight = 0;
  std::size_t gridSolidCells = 0;
  double gridSolidPercent = 0;
  std::size_t gridEntrancesWithFloor = 0;
  std::size_t gridEntrancesSurvivable = 0;
  std::vector<int> gridDrops;
  int gridMedianDropRows = -1;
};

// What the corpus looks like taken together. Medians are over levels that
// parsed; the entrance counts are over entrances, not levels.
struct CorpusStats {
  std::size_t levels = 0;        // entries offered by levelFileNames
  std::size_t parsed = 0;        // cylinder + index + faces all built
  std::size_t nonEmptyMasks = 0; // of those, the ones that rasterised to something

  double medianSolidPercent = 0;
  int medianDropRows = -1;

  std::size_t entrances = 0;
  std::size_t entrancesWithFloor = 0;
  std::size_t entrancesSurvivable = 0;
  // Levels on which every entrance that has a floor has a survivable one,
  // and at least one entrance has a floor. This is the headline number: how
  // many levels a lemming could actually be released into.
  std::size_t survivableLevels = 0;
  // The looser reading of the same thing: levels whose *median* entrance
  // drop is survivable. Reported alongside because a level with several
  // entrances can be half playable, and the two counts diverging is itself
  // worth seeing.
  std::size_t survivableByMedian = 0;
  // Levels with at least one entrance that has nothing solid below it.
  std::size_t levelsWithFloorlessEntrance = 0;
  std::size_t floorlessEntrances = 0;

  // The same headline counts against the stored collision grid -- the real
  // terrain. Where the lattice numbers measure our raster, these measure the
  // game.
  std::size_t grids = 0;
  double gridMedianSolidPercent = 0;
  int gridMedianDropRows = -1;
  std::size_t gridEntrancesWithFloor = 0;
  std::size_t gridEntrancesSurvivable = 0;
  std::size_t gridSurvivableLevels = 0;
  std::size_t gridFloorlessEntrances = 0;
};

// Survey every level in the archive. `splatDistanceCells` is the kill
// distance in terrain cells, which are world units; pass
// `World::Rules{}.splatDistance` rather than a literal, so the oracle tracks
// the simulation. The lattice raster is two rows per world unit, so its
// drops are scaled before the comparison; the stored grid is one cell per
// unit and needs none.
std::expected<std::vector<LevelStats>, std::string> surveyLevels(const AssetLibrary&,
                                                                 int splatDistanceCells);

// `splatDistanceCells` must be the value passed to surveyLevels; it is
// needed again only for the median-based counts.
CorpusStats summariseLevels(std::span<const LevelStats>, int splatDistanceCells);

}  // namespace webo
