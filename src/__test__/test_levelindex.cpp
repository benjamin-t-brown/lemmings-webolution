// Tests for the partial .LVL reading.
//
// The load-bearing test is the last one: every asset reference recovered from
// every retail level must resolve to an entry that exists in the archive. A
// mis-scan would produce references that do not, so this is what justifies
// trusting the index while the format itself is still being worked out.

#include "assets/Box.h"
#include "assets/LevelIndex.h"

#include <algorithm>
#include <cstdlib>
#include <filesystem>
#include <format>
#include <iostream>
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
void putStr(std::vector<std::byte>& v, std::string_view s) {
  put32(v, static_cast<std::uint32_t>(s.size()));
  for (char c : s)
    v.push_back(static_cast<std::byte>(c));
}

void testAssetNames() {
  std::cout << "reference mapping\n";
  check(webo::levelTextureAssetName("Models\\exit\\basket") == "TEXTURES\\MODELS\\EXIT\\BASKET.BMP",
        "model reference");
  check(webo::levelTextureAssetName("Cylinder/A/INN_A_0_0") == "TEXTURES\\CYLINDER\\A\\INN_A_0_0.BMP",
        "cylinder reference, forward slashes");
}

void testSynthetic() {
  std::cout << "synthetic level\n";
  std::vector<std::byte> d;
  putStr(d, "Level 99");
  for (int i = 0; i < 12; ++i)
    put32(d, static_cast<std::uint32_t>(i == 3 || i == 4 ? 8 : i));
  putStr(d, "_curNoLem");
  putStr(d, "Cylinder\\L\\INN_L_0_0");
  putStr(d, "Models\\exit\\basket");
  putStr(d, "PolyList 09");
  putStr(d, "Splat");
  // the "Cylinder" chunk, then two 8-byte grids
  putStr(d, "Cylinder");
  put32(d, 0xFFFFFFFF);
  put32(d, 0x00000001);
  put32(d, 0x00000003);
  put32(d, 0xFFFFFFFF);
  // padding so the span is comfortably large
  for (int i = 0; i < 16; ++i)
    put32(d, 0);

  auto lvl = webo::indexLevel(d);
  check(lvl.has_value(), "indexes");
  if (!lvl) {
    std::cerr << "    " << lvl.error() << "\n";
    return;
  }
  // The sweep is heuristic, so assert that the planted strings are *present*
  // rather than that the counts are exact: a header word can itself look like
  // a short length-prefixed string, and that is expected, not a defect.
  const auto has = [](const std::vector<std::string>& v, std::string_view want) {
    return std::find(v.begin(), v.end(), want) != v.end();
  };

  check(lvl->title == "Level 99", "title");
  check(lvl->header.size() == 12, "header words");
  check(has(lvl->cylinderTextures, "Cylinder\\L\\INN_L_0_0"), "cylinder reference found");
  check(has(lvl->modelTextures, "Models\\exit\\basket"), "model reference found");
  check(lvl->polyListCount == 1, "PolyList counted, not stored");
  check(has(lvl->scriptVariables, "_curNoLem"), "script variable found");
  check(has(lvl->shortNames, "Splat"), "short name found");
  check(lvl->grid0.size() == 2 && lvl->grid1.size() == 2, "two grids of the stated size");
  if (lvl->grid1.size() == 2)
    check(lvl->grid1SolidCells() == 1, "solid cell count skips the empty marker");

  std::vector<std::byte> noTitle;
  put32(noTitle, 0xFFFFFF);
  for (int i = 0; i < 64; ++i)
    noTitle.push_back(std::byte{0});
  check(!webo::indexLevel(noTitle).has_value(), "rejects a file with no title");
}

void testRealLevels() {
  const char* dir = std::getenv("WEBOLUTION_GAME_DIR");
  if (!dir || !*dir) {
    std::cout << "real levels: SKIPPED (set WEBOLUTION_GAME_DIR to enable)\n";
    return;
  }
  std::cout << "real levels\n";
  auto lib = webo::AssetLibrary::open(fs::path(dir) / "lemmings.box", fs::path(dir) / "patch.box");
  if (!lib) {
    check(false, lib.error());
    return;
  }

  const auto names = webo::levelFileNames(*lib);
  check(names.size() == 102, std::format("102 level files (got {})", names.size()));

  std::size_t indexed = 0, refs = 0, unresolved = 0, withGrids = 0, titled = 0;
  for (const auto& n : names) {
    auto lvl = webo::loadLevelIndex(*lib, n);
    if (!lvl) {
      check(false, std::format("{}: {}", n, lvl.error()));
      continue;
    }
    ++indexed;
    if (!lvl->title.empty())
      ++titled;
    if (!lvl->grid0.empty() && lvl->grid0.size() == lvl->grid1.size())
      ++withGrids;

    for (const auto* list : {&lvl->cylinderTextures, &lvl->modelTextures})
      for (const auto& ref : *list) {
        ++refs;
        if (!lib->contains(webo::levelTextureAssetName(ref)))
          ++unresolved;
      }
  }

  check(indexed == names.size(), std::format("all levels index ({} of {})", indexed, names.size()));
  check(titled == names.size(), "every level has a title");
  check(refs > 20000, std::format("tens of thousands of references ({})", refs));
  check(unresolved == 0,
        std::format("every reference resolves in the archive ({} did not)", unresolved));
  check(withGrids == names.size(),
        std::format("every level yields two equal grids ({} did)", withGrids));

  std::cout << std::format("    {} levels, {} asset references, all resolving\n", indexed, refs);
}

}  // namespace

int main() {
  testAssetNames();
  testSynthetic();
  testRealLevels();
  std::cout << std::format("\n{} checks, {} failures\n", g_checks, g_failures);
  return g_failures == 0 ? 0 : 1;
}
