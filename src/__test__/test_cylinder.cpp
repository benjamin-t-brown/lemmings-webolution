// Tests for the cylinder mesh read out of .LVL.
//
// The load-bearing test is the last one: the mesh must be found and fully
// validated in all 102 retail levels, and every one must agree on the geometry
// -- 7 rings, 32 angular steps, thickness 10, radii 50 and 60, rings 16 apart,
// and angular steps of exactly 11.25 degrees. That uniformity is the finding:
// the tube is a constant of the game that happens to be stored per level.
//
// A mis-locate would not survive it. The search accepts a run only if it is a
// whole number of equal-width rings, so a false positive would have to fake
// the entire structure, and the per-level agreement would then have to hold by
// accident 102 times over.

#include "assets/Box.h"
#include "assets/Cylinder.h"
#include "assets/LevelIndex.h"

#include <cmath>
#include <cstdlib>
#include <cstring>
#include <string_view>
#include <filesystem>
#include <format>
#include <iostream>
#include <numbers>
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
  check(mesh->endOffset == v.size(), "end offset is the end of the vertex run");
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
  for (const auto& name : levels) {
    auto blob = lib.read(name);
    if (!blob)
      continue;
    auto mesh = webo::findCylinderMesh(*blob);
    if (!mesh)
      continue;
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
    ++agreed;
  }

  std::cout << std::format("  mesh located in {} of {} levels; {} agree on the geometry\n", found,
                           levels.size(), agreed);
  check(found == levels.size(), "the cylinder mesh is found in every level");
  check(agreed == levels.size(), "every level agrees on the cylinder geometry");
}

}  // namespace

int main() {
  testSynthetic();

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
