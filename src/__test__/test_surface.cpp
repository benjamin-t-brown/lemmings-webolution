// Tests for surface face reconstruction and rasterisation.
//
// Synthetic cases lock the lattice → quad → mask path without game data.
// The retail pass asserts that every level yields at least some faces and
// that MM_CLIMB_002 still rebuilds the quads the RE notes counted by hand.

#include "assets/Bitmap.h"
#include "assets/Box.h"
#include "assets/Cylinder.h"
#include "assets/LevelIndex.h"
#include "assets/SurfaceMesh.h"

#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <cstring>
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
      // Negative RE result: Y is derived, not read. Climb's stride (and Y/16)
      // never appear as aligned u32s in the file, and no header word holds Y.
      std::size_t asU32 = 0, asY16 = 0;
      for (std::size_t o = 0; o + 4 <= blob->size(); o += 4) {
        std::uint32_t w;
        std::memcpy(&w, blob->data() + o, 4);
        if (w == 8224)
          ++asU32;
        if (w == 514)
          ++asY16;
      }
      check(asU32 == 0, "climb has no aligned u32 == ring stride");
      check(asY16 == 0, "climb has no aligned u32 == Y/16");
      bool headerHoldsY = false;
      for (std::uint32_t w : index->header)
        if (w == 8224 || w == 514)
          headerHoldsY = true;
      check(!headerHoldsY, "climb header does not store the ring stride");
      std::cout << std::format(
          "  MM_CLIMB_002: {} quads (ang {}, ring {}), stride {}, mask {}x{} solid {}\n",
          faces->quads.size(), faces->angularQuads, faces->ringQuads, faces->ringStride,
          mask.width, mask.height, mask.solidCells());
    }

    // Corpus: when a stride is found, it is never one of the level header words.
    if (faces->ringStride > 0) {
      bool headerHoldsY = false;
      for (std::uint32_t w : index->header)
        if (w == static_cast<std::uint32_t>(faces->ringStride))
          headerHoldsY = true;
      check(!headerHoldsY,
            std::format("{}: header must not equal derived ring stride {}", name,
                        faces->ringStride));
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

// Skin tiles wrap the whole tube and are fully opaque. They cannot be the
// walkable mask: surface rasterisation stays sparse while every named tile
// has AlphaMode::Opaque and essentially no translucent or colour-keyed holes.
void testSkinNotSolidity(const fs::path& dir) {
  std::cout << "skin is not solidity\n";
  auto lib = webo::AssetLibrary::open(dir / "lemmings.box", dir / "patch.box");
  if (!lib) {
    check(false, lib.error());
    return;
  }

  std::size_t tiles = 0, translucent = 0, totalPx = 0, nonOpaqueMode = 0;
  for (const auto& name : webo::levelFileNames(*lib)) {
    auto blob = lib->read(name);
    if (!blob)
      continue;
    auto mesh = webo::findCylinderMesh(*blob);
    if (!mesh)
      continue;
    for (const auto* block : {&mesh->tileTextures, &mesh->innerTileTextures}) {
      for (const auto& tile : *block) {
        if (tile.empty())
          continue;
        auto img = webo::loadTexture(*lib, webo::levelTextureAssetName(tile));
        if (!img)
          continue;
        ++tiles;
        if (img->alpha != webo::AlphaMode::Opaque)
          ++nonOpaqueMode;
        for (std::size_t i = 0; i < img->pixelCount(); ++i) {
          ++totalPx;
          if (img->rgba[i * 4 + 3] < 250)
            ++translucent;
        }
      }
    }
  }
  check(tiles == 4896, std::format("every named skin tile loads (got {})", tiles));
  check(nonOpaqueMode == 0, "no skin tile uses a transparency sigil");
  check(translucent == 0, "no translucent skin texels");
  check(totalPx > 0, "counted skin pixels");

  // Spot-check: opaque outer skin covers the full unroll; surface does not.
  for (const auto& name : webo::levelFileNames(*lib)) {
    if (name.find("MM_CLIMB_002") == std::string::npos)
      continue;
    auto blob = lib->read(name);
    auto mesh = webo::findCylinderMesh(*blob);
    auto index = webo::indexLevel(*blob);
    auto faces = webo::buildSurfaceMesh(*mesh, index->grid0, index->grid1);
    const auto mask = webo::rasteriseSurface(*faces, *mesh, 2);
    const double surfPct =
        100.0 * static_cast<double>(mask.solidCells()) /
        (static_cast<double>(mask.width) * mask.height);
    check(surfPct < 20.0, std::format("climb surface stays sparse ({:.1f}%)", surfPct));
    check(mesh->tileTextures.size() == 48, "outer skin slot table is 48");
    std::size_t named = 0;
    for (const auto& t : mesh->tileTextures)
      if (!t.empty())
        ++named;
    check(named == 24, std::format("climb has a full 8x3 outer skin (got {})", named));
    break;
  }
}

}  // namespace

int main() {
  testUnroll();

  if (const char* dir = std::getenv("WEBOLUTION_GAME_DIR")) {
    if (fs::exists(fs::path(dir) / "lemmings.box")) {
      testRetail(dir);
      testSkinNotSolidity(dir);
    } else
      std::cout << "retail surfaces: skipped, no lemmings.box\n";
  } else {
    std::cout << "retail surfaces: skipped, WEBOLUTION_GAME_DIR is not set\n";
  }

  std::cout << std::format("\n{} checks, {} failures\n", g_checks, g_failures);
  return g_failures == 0 ? 0 : 1;
}
