// Tests for surface face reconstruction and rasterisation.
//
// Synthetic cases lock the lattice → quad → mask path without game data.
// The retail pass asserts that every level yields at least some faces and
// that MM_CLIMB_002 still rebuilds the quads the RE notes counted by hand.

#include "assets/Box.h"
#include "assets/Cylinder.h"
#include "assets/LevelIndex.h"
#include "assets/SurfaceMesh.h"

#include <cmath>
#include <cstdlib>
#include <filesystem>
#include <format>
#include <iostream>
#include <numbers>
#include <string>
#include <string_view>
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

void testUnroll() {
  std::cout << "unroll\n";
  const auto [a0, y0] = webo::unrollCylinder({0, 16, 60});
  check(std::abs(a0) < 1e-9 && y0 == 16, "forward axis unrolls to ang 0");
  const auto [a1, y1] = webo::unrollCylinder({-60, 0, 0});
  check(std::abs(a1 - 0.25) < 1e-9 && y1 == 0, "-X is a quarter turn");
}

void testRetail(const fs::path& dir) {
  std::cout << "retail surfaces\n";
  auto lib = webo::AssetLibrary::open(dir / "lemmings.box", dir / "patch.box");
  if (!lib) {
    check(false, lib.error());
    return;
  }

  std::size_t built = 0, withRing = 0, withQuads = 0, totalQuads = 0, totalSolid = 0;
  std::size_t pixels = 0;
  for (const auto& name : webo::levelFileNames(*lib)) {
    auto blob = lib->read(name);
    if (!blob)
      continue;
    auto mesh = webo::findCylinderMesh(*blob);
    auto index = webo::indexLevel(*blob);
    if (!mesh || !index || index->grid0.empty())
      continue;
    auto faces = webo::buildSurfaceMesh(*mesh, index->grid0, index->grid1);
    if (!faces) {
      std::cerr << std::format("  FAIL  {}: {}\n", name, faces.error());
      ++g_failures;
      continue;
    }
    ++built;
    if (faces->ringStride > 0)
      ++withRing;
    if (!faces->quads.empty())
      ++withQuads;
    totalQuads += faces->quads.size();

    const auto mask = webo::rasteriseSurface(*faces, *mesh, 2);
    totalSolid += mask.solidCells();
    pixels += static_cast<std::size_t>(mask.width) * mask.height;

    if (name.find("MM_CLIMB_002") != std::string::npos) {
      // Hand-counted on this level during RE: 28 angular + 12 vertical on
      // grid0 alone; grid1 adds more. Accept a floor, not an exact total.
      check(faces->angularQuads >= 28, std::format("climb angular quads (>=28, got {})",
                                                   faces->angularQuads));
      check(faces->ringStride == 8224, std::format("climb ring stride 8224 (got {})",
                                                   faces->ringStride));
      check(faces->ringQuads >= 12,
            std::format("climb ring quads (>=12, got {})", faces->ringQuads));
      check(mask.solidCells() > 100, "climb rasterises to a visible mask");
      std::cout << std::format(
          "  MM_CLIMB_002: {} quads (ang {}, ring {}), stride {}, mask {}x{} solid {}\n",
          faces->quads.size(), faces->angularQuads, faces->ringQuads, faces->ringStride,
          mask.width, mask.height, mask.solidCells());
    }
  }

  const double coverage =
      pixels ? 100.0 * static_cast<double>(totalSolid) / static_cast<double>(pixels) : 0;
  std::cout << std::format(
      "  {} levels parsed, {} with quads, {} with a ring stride, {} quads total, "
      "corpus mask coverage {:.2f}%\n",
      built, withQuads, withRing, totalQuads, coverage);
  check(built == 102, std::format("every level parses (got {})", built));
  check(withQuads > 90, std::format("most levels produce quads (got {})", withQuads));
  check(withRing > 10, "several levels expose a ring stride");
  // Coverage is expected to be low: the surface is sparse platforms, not a
  // filled tube. Record it so a sudden fill would be noticed.
  check(coverage < 40.0, "corpus coverage stays sparse (<40%)");
}

}  // namespace

int main() {
  testUnroll();

  if (const char* dir = std::getenv("WEBOLUTION_GAME_DIR")) {
    if (fs::exists(fs::path(dir) / "lemmings.box"))
      testRetail(dir);
    else
      std::cout << "retail surfaces: skipped, no lemmings.box\n";
  } else {
    std::cout << "retail surfaces: skipped, WEBOLUTION_GAME_DIR is not set\n";
  }

  std::cout << std::format("\n{} checks, {} failures\n", g_checks, g_failures);
  return g_failures == 0 ? 0 : 1;
}
