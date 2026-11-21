// Surveys the rebuilt lattice and stored collision grid for floors below each entrance
// and drops exceeding splatDistance. The caller supplies World::Rules values without a src/sim dependency.
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

  // False if the cylinder, index arrays or face build failed.
  bool parsed = false;
  // At least one solid raster cell. VH13_KF_1 uses the 49858-word index counted separately from header[3].
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

  // Survey the stored collision grid from the .LVL (CollisionGrid.h), one row per world unit.
  bool hasGrid = false;
  int gridHeight = 0;
  std::size_t gridSolidCells = 0;
  double gridSolidPercent = 0;
  std::size_t gridEntrancesWithFloor = 0;
  std::size_t gridEntrancesSurvivable = 0;
  std::vector<int> gridDrops;
  int gridMedianDropRows = -1;
};

// Medians use parsed levels. Entrance totals count individual entrances.
struct CorpusStats {
  std::size_t levels = 0;        // entries offered by levelFileNames
  std::size_t parsed = 0;        // cylinder + index + faces all built
  std::size_t nonEmptyMasks = 0; // of those, the ones that rasterised to something

  double medianSolidPercent = 0;
  int medianDropRows = -1;

  std::size_t entrances = 0;
  std::size_t entrancesWithFloor = 0;
  std::size_t entrancesSurvivable = 0;
  // At least one entrance has a floor and all measured drops are survivable.
  std::size_t survivableLevels = 0;
  // Levels with a survivable median entrance drop.
  std::size_t survivableByMedian = 0;
  // Levels with at least one entrance that has nothing solid below it.
  std::size_t levelsWithFloorlessEntrance = 0;
  std::size_t floorlessEntrances = 0;

  // Equivalent totals for the stored collision grid.
  std::size_t grids = 0;
  double gridMedianSolidPercent = 0;
  int gridMedianDropRows = -1;
  std::size_t gridEntrancesWithFloor = 0;
  std::size_t gridEntrancesSurvivable = 0;
  std::size_t gridSurvivableLevels = 0;
  std::size_t gridFloorlessEntrances = 0;
};

// Pass World::Rules{}.splatDistance in terrain cells (world units).
// Compare raster drops at two rows per unit and stored-grid drops at one cell per unit.
std::expected<std::vector<LevelStats>, std::string> surveyLevels(const AssetLibrary&,
                                                                 int splatDistanceCells);

// Use the same splatDistanceCells as surveyLevels for the median comparisons.
CorpusStats summariseLevels(std::span<const LevelStats>, int splatDistanceCells);

}  // namespace webo
