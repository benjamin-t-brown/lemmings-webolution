#include "assets/LevelStats.h"

#include "assets/Box.h"
#include "assets/Cylinder.h"
#include "assets/LevelIndex.h"
#include "assets/LevelObjects.h"
#include "assets/SurfaceMesh.h"

#include <algorithm>
#include <cstdint>

namespace webo {
namespace {

// The mask resolution the simulation loads a level at, and the same value
// `lrtool lvlmask` draws. Changing it here without changing the caller would
// make the survey measure a mask nothing plays on.
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
                                                                 int splatDistanceRows) {
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
    if (!row.nonEmptyMask)
      continue;

    auto objs = findLevelObjects(*blob);
    if (!objs)
      continue;
    row.entrances = objs->entrances.size();
    row.exits = objs->exits.size();

    for (const auto& e : objs->entrances) {
      const auto [col, top] = worldToMaskPixel(
          e.position, mask.width, mask.height, mesh->height(), kPixelsPerUnit);
      // Straight down the mask column. py grows downward, so this is the
      // first thing the lemming would meet.
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
      if (drop <= splatDistanceRows)
        ++row.entrancesSurvivable;
    }
    std::ranges::sort(row.drops);
    row.medianDropRows = median(row.drops);
  }
  return out;
}

CorpusStats summariseLevels(std::span<const LevelStats> rows, int splatDistanceRows) {
  CorpusStats c;
  c.levels = rows.size();
  std::vector<double> solids;
  std::vector<int> drops;
  for (const auto& r : rows) {
    if (!r.parsed)
      continue;
    ++c.parsed;
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
    // Releasable: something to land on, and nothing that lands fatally.
    if (r.entrancesWithFloor > 0 && r.entrancesSurvivable == r.entrancesWithFloor)
      ++c.survivableLevels;
    if (r.medianDropRows >= 0 && r.medianDropRows <= splatDistanceRows)
      ++c.survivableByMedian;
  }
  c.medianSolidPercent = median(std::move(solids));
  c.medianDropRows = median(std::move(drops));
  return c;
}

}  // namespace webo
