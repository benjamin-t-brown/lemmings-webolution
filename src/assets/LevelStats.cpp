#include "assets/LevelStats.h"

#include "assets/Box.h"
#include "assets/CollisionGrid.h"
#include "assets/Cylinder.h"
#include "assets/LevelIndex.h"
#include "assets/LevelObjects.h"
#include "assets/SurfaceMesh.h"
#include "sim/Terrain.h"

#include <algorithm>
#include <cstdint>

namespace webo {
namespace {

// Match the diagnostic mask resolution used by lrtool lvlmask.
constexpr int kPixelsPerUnit = 2;

double median(std::vector<double> v) {
  if (v.empty())
    return 0;
  std::ranges::sort(v);
  return v[v.size() / 2];
}

int median(std::vector<int> v) {
  if (v.empty())
    return -1;
  std::ranges::sort(v);
  return v[v.size() / 2];
}

std::string basename(const std::string& entry) {
  std::string s = entry;
  if (const auto slash = s.find_last_of("/\\"); slash != std::string::npos)
    s = s.substr(slash + 1);
  if (const auto dot = s.find_last_of('.'); dot != std::string::npos)
    s = s.substr(0, dot);
  return s;
}

}  // namespace

std::expected<std::vector<LevelStats>, std::string> surveyLevels(const AssetLibrary& lib,
                                                                 int splatDistanceCells) {
  std::vector<LevelStats> out;
  for (const auto& name : levelFileNames(lib)) {
    LevelStats s;
    s.name = name;
    s.shortName = basename(name);
    out.push_back(s);
    LevelStats& row = out.back();

    auto blob = lib.read(name);
    if (!blob)
      continue;
    auto mesh = findCylinderMesh(*blob);
    auto index = indexLevel(*blob);
    if (!mesh || !index || index->grid0.empty())
      continue;
    auto faces = buildSurfaceMesh(*mesh, index->grid0, index->grid1);
    if (!faces)
      continue;

    row.parsed = true;
    row.ringStride = faces->ringStride;
    row.quads = faces->quads.size();
    row.openEdges = faces->openEdges.size();

    const auto mask = rasteriseSurface(*faces, *mesh, kPixelsPerUnit);
    row.maskWidth = mask.width;
    row.maskHeight = mask.height;
    row.solidCells = mask.solidCells();
    const auto cells = static_cast<std::size_t>(mask.width) * mask.height;
    row.solidPercent = cells ? 100.0 * static_cast<double>(row.solidCells) / cells : 0.0;
    row.nonEmptyMask = !mask.empty() && row.solidCells > 0;

    auto objs = findLevelObjects(*blob);
    if (!objs)
      continue;
    row.entrances = objs->entrances.size();
    row.exits = objs->exits.size();

    // An empty lattice raster still permits a stored-grid survey.
    for (const auto& e : objs->entrances) {
      if (!row.nonEmptyMask)
        break;
      const auto [col, top] = worldToMaskPixel(
          e.position, mask.width, mask.height, mesh->height(), kPixelsPerUnit);
      // Scan to the first solid row in the mask column. Y grows downward.
      int drop = -1;
      for (int y = top; y < mask.height; ++y)
        if (mask.solid[static_cast<std::size_t>(y) * mask.width + col]) {
          drop = y - top;
          break;
        }
      if (drop < 0)
        continue;
      ++row.entrancesWithFloor;
      row.drops.push_back(drop);
      // Scale the world-unit splat threshold by kPixelsPerUnit to compare it with the raster drop.
      if (drop <= splatDistanceCells * kPixelsPerUnit)
        ++row.entrancesSurvivable;
    }
    std::ranges::sort(row.drops);
    row.medianDropRows = median(row.drops);

    // The stored collision grid has one cell per world unit.
    auto grid = findCollisionGrid(*blob);
    if (!grid)
      continue;
    row.hasGrid = true;
    row.gridHeight = grid->height;
    row.gridSolidCells = grid->solidCells();
    const auto gcells = static_cast<std::size_t>(grid->width) * grid->height;
    row.gridSolidPercent =
        gcells ? 100.0 * static_cast<double>(row.gridSolidCells) / gcells : 0.0;
    // Terrain::fromCollisionGrid flips the Y-up file grid to the Y-down mask used by worldToMaskPixel.
    const auto terrain = sim::Terrain::fromCollisionGrid(*grid);
    for (const auto& e : objs->entrances) {
      const auto [col, top] =
          worldToMaskPixel(e.position, grid->width, grid->height, mesh->height(), 1);
      int drop = -1;
      for (int y = top; y < terrain.height(); ++y)
        if (terrain.solid(col, y)) {
          drop = y - top;
          break;
        }
      if (drop < 0)
        continue;
      ++row.gridEntrancesWithFloor;
      row.gridDrops.push_back(drop);
      if (drop <= splatDistanceCells)
        ++row.gridEntrancesSurvivable;
    }
    std::ranges::sort(row.gridDrops);
    row.gridMedianDropRows = median(row.gridDrops);
  }
  return out;
}

CorpusStats summariseLevels(std::span<const LevelStats> rows, int splatDistanceCells) {
  CorpusStats c;
  c.levels = rows.size();
  std::vector<double> solids, gridSolids;
  std::vector<int> drops, gridDrops;
  for (const auto& r : rows) {
    if (!r.parsed)
      continue;
    ++c.parsed;

    // Accumulate stored-grid results before skipping empty rasters.
    if (r.hasGrid) {
      ++c.grids;
      gridSolids.push_back(r.gridSolidPercent);
      if (r.gridMedianDropRows >= 0)
        gridDrops.push_back(r.gridMedianDropRows);
      c.gridEntrancesWithFloor += r.gridEntrancesWithFloor;
      c.gridEntrancesSurvivable += r.gridEntrancesSurvivable;
      c.gridFloorlessEntrances += r.entrances - r.gridEntrancesWithFloor;
      if (r.gridEntrancesWithFloor > 0 &&
          r.gridEntrancesSurvivable == r.gridEntrancesWithFloor)
        ++c.gridSurvivableLevels;
    }

    if (!r.nonEmptyMask)
      continue;
    ++c.nonEmptyMasks;
    solids.push_back(r.solidPercent);
    if (r.medianDropRows >= 0)
      drops.push_back(r.medianDropRows);

    c.entrances += r.entrances;
    c.entrancesWithFloor += r.entrancesWithFloor;
    c.entrancesSurvivable += r.entrancesSurvivable;

    const auto floorless = r.entrances - r.entrancesWithFloor;
    if (floorless > 0) {
      ++c.levelsWithFloorlessEntrance;
      c.floorlessEntrances += floorless;
    }
    // Count levels with a floor and no fatal measured drops.
    if (r.entrancesWithFloor > 0 && r.entrancesSurvivable == r.entrancesWithFloor)
      ++c.survivableLevels;
    if (r.medianDropRows >= 0 && r.medianDropRows <= splatDistanceCells * kPixelsPerUnit)
      ++c.survivableByMedian;

  }
  c.medianSolidPercent = median(std::move(solids));
  c.medianDropRows = median(std::move(drops));
  c.gridMedianSolidPercent = median(std::move(gridSolids));
  c.gridMedianDropRows = median(std::move(gridDrops));
  return c;
}

}  // namespace webo
