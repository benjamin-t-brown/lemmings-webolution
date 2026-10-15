// The stored collision mask, checked against the original itself.
//
// This is the strongest oracle the project has: the grid in the `.LVL` is the
// grid the running engine collides against, so the parser is correct exactly
// when it reproduces a live dump byte for byte. Five dumps were taken from
// five independently entered levels (PLAN.md 1.6, `tools/harness/dump_grids.py`)
// and live in `build/live_dump/`. When they are present this test diffs
// against them; when they are not it still checks the corpus-wide invariants,
// so the suite stays useful on a machine that has never run the original.
//
// Unlike `test_corpus`, the numbers here are not a baseline to be improved.
// A disagreement with a live dump is a bug in the parser.

#include "assets/Box.h"
#include "assets/CollisionGrid.h"
#include "assets/LevelIndex.h"
#include "sim/Terrain.h"

#include <cstdio>
#include <cstdlib>
#include <filesystem>
#include <format>
#include <iostream>
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

// Every retail level: 256 cells around the tube, one row per world unit of
// its 96-unit height.
constexpr int kWidth = 256;
constexpr int kHeight = 96;
constexpr std::size_t kLevels = 102;

// Live dumps, and the solid percentage each shows. Measured off the running
// original, so these are the engine's own numbers, not ours.
struct LiveCase {
  const char* level;
  double solidPercent;
};
constexpr LiveCase kLiveCases[] = {
    {"MM_CLIMB_002", 63.33}, {"MM_BUILD", 55.66},   {"MM_FLOAT", 45.40},
    {"MM_BOMB", 40.79},      {"MM_DIG_000", 35.03},
};

std::vector<std::uint32_t> readDump(const fs::path& p) {
  std::vector<std::uint32_t> out;
  std::FILE* f = std::fopen(p.string().c_str(), "rb");
  if (!f)
    return out;
  std::fseek(f, 0, SEEK_END);
  const long bytes = std::ftell(f);
  std::fseek(f, 0, SEEK_SET);
  if (bytes > 0 && bytes % 4 == 0) {
    out.resize(static_cast<std::size_t>(bytes) / 4);
    if (std::fread(out.data(), 1, static_cast<std::size_t>(bytes), f) !=
        static_cast<std::size_t>(bytes))
      out.clear();
  }
  std::fclose(f);
  return out;
}

void testAgainstLiveDumps(const webo::AssetLibrary& lib, const fs::path& dumpDir) {
  std::cout << "collision grid against the running original\n";
  int compared = 0;
  for (const auto& c : kLiveCases) {
    const fs::path dump = dumpDir / std::format("grid_{}.bin", c.level);
    const auto live = readDump(dump);
    if (live.empty()) {
      std::cout << std::format("  {}: no live dump at {}, skipped\n", c.level,
                               dump.string());
      continue;
    }
    ++compared;
    auto grid = webo::loadCollisionGrid(lib, std::format("COMPILED\\{}.LVL", c.level));
    if (!grid) {
      check(false, std::format("{}: {}", c.level, grid.error()));
      continue;
    }
    check(grid->cells.size() == live.size(),
          std::format("{}: {} cells, live dump has {}", c.level, grid->cells.size(),
                      live.size()));
    if (grid->cells.size() != live.size())
      continue;

    std::size_t differing = 0;
    for (std::size_t i = 0; i < live.size(); ++i)
      if (grid->cells[i] != live[i])
        ++differing;
    check(differing == 0,
          std::format("{}: byte-identical to the live grid ({} cells differ)", c.level,
                      differing));

    const double pct = 100.0 * grid->solidCells() / grid->cells.size();
    check(std::abs(pct - c.solidPercent) < 0.01,
          std::format("{}: solid {:.2f}% (engine measured {:.2f}%)", c.level, pct,
                      c.solidPercent));
  }
  if (compared == 0)
    std::cout
        << "  no live dumps present; run tools/harness/dump_grids.py to make them\n";
}

void testCorpus(const webo::AssetLibrary& lib) {
  std::cout << "collision grid across the corpus\n";
  const auto names = webo::levelFileNames(lib);
  check(names.size() == kLevels,
        std::format("{} level files (got {})", kLevels, names.size()));

  std::size_t parsed = 0, wrongShape = 0, empty = 0;
  std::string firstError;
  double minPct = 100.0, maxPct = 0.0;
  for (const auto& name : names) {
    auto grid = webo::loadCollisionGrid(lib, name);
    if (!grid) {
      if (firstError.empty())
        firstError = std::format("{}: {}", name, grid.error());
      continue;
    }
    ++parsed;
    if (grid->width != kWidth || grid->height != kHeight)
      ++wrongShape;
    const double pct = 100.0 * grid->solidCells() / grid->cells.size();
    if (grid->solidCells() == 0)
      ++empty;
    minPct = std::min(minPct, pct);
    maxPct = std::max(maxPct, pct);
  }

  check(parsed == kLevels,
        std::format("every level yields a grid ({}/{}){}", parsed, kLevels,
                    firstError.empty() ? "" : " -- first error: " + firstError));
  check(wrongShape == 0,
        std::format("every grid is {}x{} ({} are not)", kWidth, kHeight, wrongShape));
  // The engine has no empty levels: the sparsest retail mask is 5.5% solid.
  // This is what separates the stored mask from the lattice raster, where one
  // level rasterises to nothing at all.
  check(empty == 0, std::format("no level has an empty mask ({} do)", empty));
  check(minPct > 1.0,
        std::format("sparsest level is still real terrain ({:.2f}%)", minPct));

  std::cout << std::format("  {} grids, {}x{}, solid {:.1f}%..{:.1f}%\n", parsed, kWidth,
                           kHeight, minPct, maxPct);
}

void testTerrainMapping(const webo::AssetLibrary& lib) {
  std::cout << "collision grid -> Terrain\n";
  auto grid = webo::loadCollisionGrid(lib, "COMPILED\\MM_CLIMB_002.LVL");
  if (!grid) {
    check(false, grid.error());
    return;
  }
  const auto terrain = webo::sim::Terrain::fromCollisionGrid(*grid);
  check(terrain.width() == grid->width, "terrain keeps the grid's width");
  check(terrain.height() == grid->height, "terrain keeps the grid's height");
  check(terrain.solidCells() == grid->solidCells(),
        std::format("solid cells carry over ({} vs {})", terrain.solidCells(),
                    grid->solidCells()));

  // Every cell must agree with the engine's own test, not just the total.
  std::size_t mismatched = 0;
  for (int y = 0; y < grid->height; ++y)
    for (int x = 0; x < grid->width; ++x)
      if (terrain.solid(x, y) != grid->solid(x, y))
        ++mismatched;
  check(mismatched == 0, std::format("every cell agrees ({} do not)", mismatched));

  // 0x2 is the engine's indestructible bit, and Steel is the material that
  // already means that here.
  bool sawSteel = false;
  for (int y = 0; y < grid->height && !sawSteel; ++y)
    for (int x = 0; x < grid->width; ++x)
      if (grid->at(x, y) & webo::kCellIndestructible && grid->solid(x, y)) {
        sawSteel = terrain.at(x, y) == webo::sim::Material::Steel;
        break;
      }
  check(sawSteel, "indestructible cells become Steel");
}

}  // namespace

int main() {
  const char* dir = std::getenv("WEBOLUTION_GAME_DIR");
  if (!dir) {
    std::cout << "collision grid: skipped, WEBOLUTION_GAME_DIR is not set\n";
    std::cout << std::format("\n{} checks, {} failures\n", g_checks, g_failures);
    return 0;
  }
  if (!fs::exists(fs::path(dir) / "lemmings.box")) {
    std::cout << "collision grid: skipped, no lemmings.box\n";
    std::cout << std::format("\n{} checks, {} failures\n", g_checks, g_failures);
    return 0;
  }

  auto lib = webo::AssetLibrary::open(fs::path(dir) / "lemmings.box",
                                      fs::path(dir) / "patch.box");
  if (!lib) {
    std::cerr << lib.error() << "\n";
    return 1;
  }

  // The dumps sit beside the build, not in the game directory.
  testAgainstLiveDumps(*lib, fs::path(WEBOLUTION_SOURCE_DIR) / "build" / "live_dump");
  testCorpus(*lib);
  testTerrainMapping(*lib);

  std::cout << std::format("\n{} checks, {} failures\n", g_checks, g_failures);
  return g_failures == 0 ? 0 : 1;
}
