// Tests for the cylinder mesh read out of .LVL.
//
// The load-bearing test is the last one: the mesh must be found and fully
// validated in all 102 retail levels, and every one must agree on the geometry
// -- 7 rings, 32 angular steps, thickness 10, radii 50 and 60, rings 16 apart,
// and angular steps of exactly 11.25 degrees. That uniformity is the finding:
// the tube is a constant of the game that happens to be stored per level.
//
// A mis-locate would not survive it. The search accepts a run only if it is a
// whole number of equal-width rings and the whole record chain behind it
// parses, so a false positive would have to fake the entire structure, and the
// per-level agreement would then have to hold by accident 102 times over.
//
// The complement matters just as much: the surface geometry that follows the
// tube must *vary*. It does -- 100 distinct vertex counts across the 102
// levels, from 505 to 2772 -- which is what identifies it as the per-level
// terrain. Were it constant too, this reading would be suspect.

#include "assets/Box.h"
#include "assets/Cylinder.h"
#include "assets/LevelIndex.h"
#include "gfx/CylinderSkin.h"

#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <cstring>
#include <string_view>
#include <filesystem>
#include <format>
#include <iostream>
#include <numbers>
#include <set>
#include <string>
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

void put32(std::vector<std::byte>& v, std::uint32_t x) {
  for (int i = 0; i < 4; ++i)
    v.push_back(static_cast<std::byte>((x >> (8 * i)) & 0xFF));
}

void putF64(std::vector<std::byte>& v, double d) {
  std::uint64_t bits = 0;
  std::memcpy(&bits, &d, sizeof bits);
  for (int i = 0; i < 8; ++i)
    v.push_back(static_cast<std::byte>((bits >> (8 * i)) & 0xFF));
}

// A synthetic mesh, so the parser is tested without needing the retail data.
void testSynthetic() {
  std::cout << "synthetic mesh\n";

  constexpr std::uint32_t kRings = 3;
  constexpr std::uint32_t kSteps = 8;
  constexpr double kInner = 50.0;
  constexpr double kThickness = 10.0;

  std::vector<std::byte> v;
  // Leading filler, so the parser has to search rather than start at zero.
  for (int i = 0; i < 7; ++i)
    v.push_back(static_cast<std::byte>(0xAB));

  put32(v, 112);
  putF64(v, kThickness);
  put32(v, kRings);
  for (std::uint32_t ring = 0; ring < kRings; ++ring)
    for (std::uint32_t step = 0; step < kSteps; ++step) {
      const double a = 2.0 * std::numbers::pi * step / kSteps;
      for (int outer = 0; outer < 2; ++outer) {
        const double r = kInner + (outer ? kThickness : 0.0);
        putF64(v, -std::sin(a) * r);
        putF64(v, 16.0 * ring);
        putF64(v, std::cos(a) * r);
      }
    }

  // u32 ringCount again, then the per-vertex inner radii.
  put32(v, kRings);
  for (std::uint32_t i = 0; i < kRings * kSteps; ++i)
    putF64(v, kInner);

  // u32 one, u32 surfaceCount, then the surface vertices.
  put32(v, 1);
  put32(v, 5);
  for (int i = 0; i < 5; ++i) {
    putF64(v, 1.0 * i);
    putF64(v, 2.0 * i);
    putF64(v, 60.0);
  }

  // u32 zero, u32 tileCount, then the names.
  put32(v, 0);
  put32(v, 2);
  for (std::string_view name : {"INN_0_0", "OUT_0_0"}) {
    put32(v, static_cast<std::uint32_t>(name.size()));
    for (char c : name)
      v.push_back(static_cast<std::byte>(c));
  }

  auto mesh = webo::findCylinderMesh(v);
  check(mesh.has_value(), "synthetic mesh is found");
  if (!mesh)
    return;

  check(mesh->ringCount == kRings, "ring count");
  check(mesh->angularSteps == kSteps, "angular steps");
  check(mesh->faceCount == 112, "face count is recorded verbatim");
  check(mesh->thickness == kThickness, "thickness");
  check(mesh->vertices.size() == kRings * kSteps * 2, "vertex count");
  check(mesh->offset == 7, "offset points at the header, past the filler");
  check(mesh->endOffset == v.size(), "end offset is the end of the whole chain");
  check(mesh->radii.size() == kRings * kSteps, "one radius per inner vertex");
  check(mesh->one == 1, "the unidentified word is recorded verbatim");
  check(mesh->surface.size() == 5, "surface vertex count");
  check(mesh->surface[4].y == 8.0, "surface vertices are read in order");
  check(mesh->tileTextures.size() == 2, "tile texture count");
  check(mesh->tileTextures[1] == "OUT_0_0", "tile texture names");
  check(std::abs(mesh->innerRadius() - kInner) < 1e-9, "inner radius");
  check(std::abs(mesh->outerRadius() - (kInner + kThickness)) < 1e-9, "outer radius");
  check(std::abs(mesh->ringSpacing() - 16.0) < 1e-9, "ring spacing");
  check(std::abs(mesh->height() - 32.0) < 1e-9, "height spans the outer rings");
  check(mesh->bandCount() == kRings - 1, "bands sit between rings");

  // Inner precedes outer at each angular step.
  const double ri = std::hypot(mesh->vertex(0, 3, false).x, mesh->vertex(0, 3, false).z);
  const double ro = std::hypot(mesh->vertex(0, 3, true).x, mesh->vertex(0, 3, true).z);
  check(ro > ri, "the outer vertex of a step is further out than the inner one");
}

void testRetail(const fs::path& gameDir) {
  std::cout << "retail levels\n";

  auto opened = webo::AssetLibrary::open(gameDir / "lemmings.box", gameDir / "patch.box");
  if (!opened) {
    std::cerr << "  FAIL  could not open the installation: " << opened.error() << "\n";
    ++g_failures;
    return;
  }
  const webo::AssetLibrary& lib = *opened;

  const auto levels = webo::levelFileNames(lib);
  check(levels.size() == 102, "102 levels in the archive");

  std::size_t found = 0;
  std::size_t agreed = 0;
  std::set<std::size_t> surfaceSizes;
  for (const auto& name : levels) {
    auto blob = lib.read(name);
    if (!blob)
      continue;
    auto mesh = webo::findCylinderMesh(*blob);
    if (!mesh) {
      std::cerr << std::format("  FAIL  {}: {}\n", name, mesh.error());
      ++g_failures;
      continue;
    }
    ++found;

    // Every retail level agrees on all of this.
    const bool uniform = mesh->ringCount == 7 && mesh->angularSteps == 32 &&
                         mesh->faceCount == 112 && mesh->thickness == 10.0 &&
                         mesh->vertices.size() == 7 * 32 * 2 &&
                         std::abs(mesh->innerRadius() - 50.0) < 1e-6 &&
                         std::abs(mesh->outerRadius() - 60.0) < 1e-6 &&
                         std::abs(mesh->ringSpacing() - 16.0) < 1e-6 &&
                         std::abs(mesh->height() - 96.0) < 1e-6;
    if (!uniform) {
      std::cerr << std::format("  FAIL  {} disagrees: rings={} steps={} thick={} r={}..{}\n", name,
                               mesh->ringCount, mesh->angularSteps, mesh->thickness,
                               mesh->innerRadius(), mesh->outerRadius());
      continue;
    }

    // The angular step is 360/32 = 11.25 degrees, measured off the vertices.
    const auto& v1 = mesh->vertex(0, 1, false);
    const double deg = std::atan2(-v1.x, v1.z) * 180.0 / std::numbers::pi;
    if (std::abs(deg - 11.25) > 1e-6) {
      std::cerr << std::format("  FAIL  {} angular step is {} degrees\n", name, deg);
      continue;
    }

    // The tile grid must divide the mesh evenly, or the skin cannot be
    // addressed as 8 columns by 3 rows.
    if (mesh->stepsPerTileColumn() != 4 || mesh->bandsPerTileRow() != 2) {
      std::cerr << std::format("  FAIL  {} tile grid does not divide: {} steps/col {} bands/row\n",
                               name, mesh->stepsPerTileColumn(), mesh->bandsPerTileRow());
      continue;
    }

    // The rest of the chain: radii parallel to the inner vertices, the
    // per-level surface, and exactly 48 tile names (8 columns x 3 rows,
    // inner and outer).
    if (mesh->radii.size() != 7u * 32u || mesh->one != 1 ||
        mesh->tileTextures.size() != 48) {
      std::cerr << std::format("  FAIL  {} tail disagrees: radii={} one={} tiles={}\n", name,
                               mesh->radii.size(), mesh->one, mesh->tileTextures.size());
      continue;
    }
    // Every non-empty tile name must resolve to a texture that exists in the
    // archive. Empty slots are normal -- most levels skin one surface only.
    bool resolved = true;
    for (const auto& tile : mesh->tileTextures)
      if (!tile.empty() && !lib.contains(webo::levelTextureAssetName(tile))) {
        std::cerr << std::format("  FAIL  {} tile {} does not resolve\n", name, tile);
        resolved = false;
        break;
      }
    if (!resolved)
      continue;

    // The surface is the part that varies, so it is checked for being
    // present and level-specific rather than for a fixed size.
    if (mesh->surface.empty()) {
      std::cerr << std::format("  FAIL  {} has no surface geometry\n", name);
      continue;
    }
    surfaceSizes.insert(mesh->surface.size());
    ++agreed;
  }

  std::cout << std::format("  mesh located in {} of {} levels; {} agree on the geometry\n", found,
                           levels.size(), agreed);
  check(found == levels.size(), "the cylinder mesh is found in every level");
  check(agreed == levels.size(), "every level agrees on the cylinder geometry");

  // Cross-check against LevelIndex: the first of the two u32 arrays holds
  // indices into this surface. Every non-empty entry must be in range, and
  // the largest must come close to the vertex count -- an index array covers
  // nearly all of its pool, where unrelated data would not land just under
  // the bound in all 102 levels.
  std::size_t indexed = 0;
  double worstCoverage = 1.0;
  for (const auto& name : levels) {
    auto blob = lib.read(name);
    if (!blob)
      continue;
    auto mesh = webo::findCylinderMesh(*blob);
    auto index = webo::indexLevel(*blob);
    if (!mesh || !index || index->grid0.empty())
      continue;

    std::uint32_t highest = 0;
    bool inRange = true;
    for (std::uint32_t v : index->grid0) {
      if (v == 0xFFFFFFFFu)
        continue;
      if (v >= mesh->surface.size()) {
        inRange = false;
        break;
      }
      highest = std::max(highest, v);
    }
    if (!inRange) {
      std::cerr << std::format("  FAIL  {} has an out-of-range surface index\n", name);
      continue;
    }
    worstCoverage =
        std::min(worstCoverage, static_cast<double>(highest + 1) / mesh->surface.size());
    ++indexed;
  }
  std::cout << std::format("  {} of {} levels index the surface in range; worst coverage {:.3f}\n",
                           indexed, levels.size(), worstCoverage);
  check(indexed == levels.size(), "the first u32 array indexes the surface in every level");
  check(worstCoverage > 0.8, "the highest index comes close to the vertex count");

  // The point of the exercise: the tube is constant, the surface is not. If
  // the surface were also constant it would not be the terrain, and this
  // whole reading would be suspect.
  std::cout << std::format("  surface vertex counts: {} distinct, {}..{}\n", surfaceSizes.size(),
                           surfaceSizes.empty() ? 0 : *surfaceSizes.begin(),
                           surfaceSizes.empty() ? 0 : *surfaceSizes.rbegin());
  check(surfaceSizes.size() > 50, "the surface geometry varies from level to level");
}

// Tile names carry their own position, and reading it out is the whole of the
// skin's layout logic. The first version of parseTilePosition computed the
// indices and forgot to write them to its out-parameters, so every tile came
// back as column 0, row 0 and the level stacked itself in one corner. These
// are the checks that would have caught it in a second.
void testTilePositions() {
  std::cout << "tile positions\n";

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

  // Rejections, all of which must not silently read as 0,0.
  for (const char* bad : {"", "OUT_L", "OUT_L_1", "Cylinder\\L\\MID_L_1_1", "OUT_L_x_1",
                          "OUT_L_1_y", "OUT_L_-1_1", "L_1_1"}) {
    const auto pos = webo::gfx::parseTilePosition(bad);
    check(!pos.valid, std::format("'{}' is rejected", bad));
  }

  // Forward slashes work too; the archive uses both separators.
  const auto slashes = webo::gfx::parseTilePosition("Cylinder/L/OUT_L_2_2");
  check(slashes.valid && slashes.column == 2 && slashes.row == 2, "forward slashes parse");

  // Every non-empty tile of every retail level must give a position inside
  // the 8x3 grid, and the two surfaces must each cover all 24 cells.
  const char* dir = std::getenv("WEBOLUTION_GAME_DIR");
  if (!dir)
    return;
  auto opened = webo::AssetLibrary::open(fs::path(dir) / "lemmings.box",
                                         fs::path(dir) / "patch.box");
  if (!opened)
    return;

  std::size_t parsed = 0;
  std::size_t outOfGrid = 0;
  std::size_t unparsed = 0;
  for (const auto& name : webo::levelFileNames(*opened)) {
    auto blob = opened->read(name);
    if (!blob)
      continue;
    auto mesh = webo::findCylinderMesh(*blob);
    if (!mesh)
      continue;
    for (const auto& tile : mesh->tileTextures) {
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
  std::cout << std::format("  {} retail tile names parsed\n", parsed);
  check(unparsed == 0, std::format("every retail tile name parses ({} did not)", unparsed));
  check(outOfGrid == 0,
        std::format("every position is inside the 8x3 grid ({} were not)", outOfGrid));
  check(parsed == 102 * 24, std::format("24 tiles per level across 102 levels (got {})", parsed));
}

}  // namespace

int main() {
  testSynthetic();
  testTilePositions();

  if (const char* dir = std::getenv("WEBOLUTION_GAME_DIR")) {
    if (fs::exists(fs::path(dir) / "lemmings.box"))
      testRetail(dir);
    else
      std::cout << "retail levels: skipped, no lemmings.box in " << dir << "\n";
  } else {
    std::cout << "retail levels: skipped, WEBOLUTION_GAME_DIR is not set\n";
  }

  std::cout << std::format("{} checks, {} failures\n", g_checks, g_failures);
  return g_failures == 0 ? 0 : 1;
}
