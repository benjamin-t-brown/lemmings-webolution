// Tests for the partial .LVL reading.
//
// The load-bearing test is the last one: every asset reference recovered from
// every retail level must resolve to an entry that exists in the archive. A
// mis-scan would produce references that do not, so this is what justifies
// trusting the index while the format itself is still being worked out.

#include "assets/Box.h"
#include "assets/LevelIndex.h"

#include <algorithm>
#include <cstring>
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
void putF64(std::vector<std::byte>& v, double x) {
  unsigned char buf[8];
  std::memcpy(buf, &x, 8);
  for (unsigned char c : buf)
    v.push_back(static_cast<std::byte>(c));
}

// native-012 fixtures: serialized fields, independent of recovered C++ layouts.
void put16(std::vector<std::byte>& v, std::uint16_t x) {
  for (int i = 0; i < 2; ++i)
    v.push_back(static_cast<std::byte>((x >> (8 * i)) & 0xFF));
}
void put64(std::vector<std::byte>& v, std::uint64_t x) {
  for (int i = 0; i < 8; ++i)
    v.push_back(static_cast<std::byte>((x >> (8 * i)) & 0xFF));
}

struct StructuredFixture {
  std::vector<std::byte> bytes;
  std::size_t prefixEnd, integerCountAt, fixedBytesAt, worldEnd;
};

StructuredFixture structuredFixture(bool decoy, bool empty = false) {
  StructuredFixture f{};
  auto& d = f.bytes;
  putStr(d, "Level 23");
  for (std::uint32_t word : {1u, 0u, 0u, 0u, 0u, 0u}) put32(d, word);
  put32(d, 0);  // four flags
  put32(d, 1);  // first name table: the decoy is a valid resource name
  putStr(d, decoy ? "Cylinder" : "Resource");
  put32(d, 0);  // second and third name tables, then object list
  put32(d, 0);
  put32(d, 0);
  put32(d, 1);  // fourth name table
  putStr(d, "Models\\exit\\basket");
  put32(d, 1);  // fifth name table
  putStr(d, "Cylinder\\L\\INN_L_0_0");
  putStr(d, "world-prefix");
  f.prefixEnd = d.size();

  put32(d, 112);
  put64(d, 0x4024000000000000ull);  // binary64 bits, no conversion
  put32(d, 1);
  for (unsigned i = 0; i < 192; ++i) put64(d, 0x3FF0000000000000ull + i);
  put32(d, 2);
  for (unsigned i = 0; i < 64; ++i) put64(d, 0x4000000000000000ull + i);
  put32(d, 1);  // class-one leading value
  put32(d, 2);
  for (unsigned i = 0; i < 6; ++i) put64(d, 0x4010000000000000ull + i);
  put32(d, 1);
  for (unsigned i = 0; i < 3; ++i) put64(d, 0x4020000000000000ull + i);
  put32(d, 1);  // named reference
  putStr(d, "Models\\fixture\\skin");
  put32(d, 7);
  put32(d, 0);  // polygon count
  for (unsigned i = 0; i < 12; ++i) put64(d, 0x3FF0000000000000ull + i);
  putStr(d, empty ? "WorldTag" : "Cylinder");
  put64(d, 0);
  put64(d, 0);
  put32(d, 0);  // nested object count: completes the legacy five-zero preamble
  put32(d, 96);
  put64(d, 0x3FD70A3D70A3D70Aull);
  f.integerCountAt = d.size();
  put32(d, empty ? 0 : 6);
  if (!empty) {
    for (std::uint32_t word : {0xFFFFFFFFu, 0x80000000u, 0x01234567u,
                               0u, 0xFEDCBA98u, 0x7FFFFFFFu}) put32(d, word);
  }
  put32(d, 2);  // integer triples, distinct from the lattice
  for (unsigned i = 0; i < 6; ++i) put32(d, 0xABC00000u + i);
  put32(d, 3);
  for (unsigned i = 0; i < 3; ++i) put32(d, 0xDEF00000u + i);
  put32(d, 5);
  for (unsigned i = 0; i < 5; ++i) d.push_back(static_cast<std::byte>(0xD0 + i));
  f.fixedBytesAt = d.size();
  d.insert(d.end(), 6144, std::byte{0xA5});
  putStr(d, "INN_fixture");
  putStr(d, "OUT_fixture");
  for (unsigned list = 0; list < 32; ++list) {
    put32(d, list == 0 || list == 31 ? 1 : 0);
    if (list == 0 || list == 31) {
      put32(d, 0xA0000000u + list);
      put16(d, 0x8001);
      put16(d, 0xFEDC);
      for (unsigned i = 0; i < 5; ++i) put64(d, 0x4008000000000000ull + i);
    }
  }
  f.worldEnd = d.size();
  for (std::uint32_t word : {0xDEADBEEFu, 0xFFFFFFFFu, 0x12345678u}) put32(d, word);
  return f;
}
// End native-012 fixture construction.

void testStructuredLattice() {
  std::cout << "structured surface-index lattice\n";
  const std::vector<std::uint32_t> expected = {
      0xFFFFFFFFu, 0x80000000u, 0x01234567u, 0u, 0xFEDCBA98u, 0x7FFFFFFFu};
  std::vector<std::uint32_t> plainGrid;
  for (bool decoy : {false, true}) {
    auto f = structuredFixture(decoy);
    const auto original = f.bytes;
    const auto lvl = webo::indexLevel(f.bytes);
    check(lvl.has_value(), "structured public index succeeds");
    check(f.bytes == original, "structured input preserved");
    if (!lvl) continue;
    check(lvl->gridFallbackReason.empty(), "structured result has no fallback diagnostic");
    check(lvl->grid0.size() == expected.size(), "lattice count is not a retail constant");
    for (std::size_t i = 0; i < expected.size(); ++i)
      check(i < lvl->grid0.size() && lvl->grid0[i] == expected[i],
            std::format("complete lattice word {} in order", i));
    check(lvl->grid1.empty(), "structured grid1 remains empty");
    if (!decoy) plainGrid = lvl->grid0;
    else check(lvl->grid0 == plainGrid, "earlier Cylinder name does not affect lattice");

    f.bytes.resize(f.worldEnd);
    auto exact = webo::indexLevel(f.bytes);
    check(exact && exact->grid0 == expected && exact->gridFallbackReason.empty(),
          "both exact-world and unrelated trailing input accepted");
  }

  // A later usable legacy chunk proves an empty structured result cannot fall back.
  auto empty = structuredFixture(false, true);
  putStr(empty.bytes, "Cylinder");
  for (int i = 0; i < 5; ++i) put32(empty.bytes, 0);
  put32(empty.bytes, 96);
  put64(empty.bytes, 0);
  put32(empty.bytes, 1);
  put32(empty.bytes, 77);
  auto lvl = webo::indexLevel(empty.bytes);
  check(lvl && lvl->grid0.empty() && lvl->grid1.empty() &&
            lvl->gridFallbackReason.empty(), "structured empty array succeeds without fallback");

  const auto rejected = [](const std::vector<std::byte>& d, std::size_t cursor,
                           std::string_view message, const std::vector<std::uint32_t>& grid) {
    // Any leaked invalid_argument terminates the test: only the public expected
    // interface is exercised here, with no catch hiding a decoder exception.
    auto result = webo::indexLevel(d);
    check(result.has_value(), "rejected structure retains partial-index success");
    if (result) {
      check(result->gridFallbackReason ==
                std::format("surface-index lattice: recovered reader at byte {}: {}", cursor, message),
            "decoder rejection and exact cursor retained");
      check(result->grid0 == grid && result->grid1.empty(), "legacy output preserved after rejection");
    }
  };
  auto prefix = structuredFixture(true);
  prefix.bytes.resize(prefix.prefixEnd - 1);
  rejected(prefix.bytes, prefix.prefixEnd - 12, "truncated level string", {});
  auto noLocator = structuredFixture(false);
  noLocator.bytes.resize(noLocator.prefixEnd - 1);
  rejected(noLocator.bytes, noLocator.prefixEnd - 12, "truncated level string", {});
  auto world = structuredFixture(false);
  world.bytes.resize(world.fixedBytesAt + 6143);
  rejected(world.bytes, world.fixedBytesAt, "truncated world fixed bytes", expected);
  auto tail = structuredFixture(false);
  tail.bytes.resize(tail.worldEnd - 1);
  rejected(tail.bytes, tail.worldEnd - 48, "invalid world tail count", expected);
  auto malformed = structuredFixture(true);
  // Impossible first integer count; the original locator still returns empty.
  for (int i = 0; i < 4; ++i) malformed.bytes[malformed.integerCountAt + i] = std::byte{0xFF};
  rejected(malformed.bytes, malformed.integerCountAt + 4, "invalid world integer count", {});
}

void testAssetNames() {
  std::cout << "reference mapping\n";
  check(webo::levelTextureAssetName("Models\\exit\\basket") ==
            "TEXTURES\\MODELS\\EXIT\\BASKET.BMP",
        "model reference");
  check(webo::levelTextureAssetName("Cylinder/A/INN_A_0_0") ==
            "TEXTURES\\CYLINDER\\A\\INN_A_0_0.BMP",
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
  // the "Cylinder" chunk: five zeros, height, an f64, then one counted
  // index array. header[3] is deliberately not that count.
  putStr(d, "Cylinder");
  for (int i = 0; i < 5; ++i)
    put32(d, 0);
  put32(d, 96);
  putF64(d, 0.36);
  put32(d, 4);
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
  check(lvl->gridFallbackReason ==
            "surface-index lattice: recovered reader at byte 95: invalid level name count",
        std::format("non-level synthetic input retains diagnosed legacy extraction: {}",
                    lvl->gridFallbackReason));
  check(lvl->header.size() == 12, "header words");
  check(has(lvl->cylinderTextures, "Cylinder\\L\\INN_L_0_0"), "cylinder reference found");
  check(has(lvl->modelTextures, "Models\\exit\\basket"), "model reference found");
  check(lvl->polyListCount == 1, "PolyList counted, not stored");
  check(has(lvl->scriptVariables, "_curNoLem"), "script variable found");
  check(has(lvl->shortNames, "Splat"), "short name found");
  check(lvl->grid0.size() == 4 && lvl->grid1.empty(),
        "one index array of the counted length");
  if (lvl->grid0.size() == 4) {
    check(lvl->grid0[0] == 0xFFFFFFFFu && lvl->grid0[1] == 1 && lvl->grid0[2] == 3 &&
              lvl->grid0[3] == 0xFFFFFFFFu,
          "index words sit after the Cylinder preamble");
    check(lvl->grid1SolidCells() == 0, "grid1 is absent");
  }

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
  auto lib = webo::AssetLibrary::open(fs::path(dir) / "lemmings.box",
                                      fs::path(dir) / "patch.box");
  if (!lib) {
    check(false, lib.error());
    return;
  }

  const auto names = webo::levelFileNames(*lib);
  check(names.size() == 102, std::format("102 level files (got {})", names.size()));

  std::size_t indexed = 0, refs = 0, unresolved = 0, withLattice = 0, titled = 0;
  for (const auto& n : names) {
    auto lvl = webo::loadLevelIndex(*lib, n);
    if (!lvl) {
      check(false, std::format("{}: {}", n, lvl.error()));
      continue;
    }
    ++indexed;
    if (!lvl->title.empty())
      ++titled;
    if (lvl->grid0.size() == 49858 && lvl->grid1.empty())
      ++withLattice;

    for (const auto* list : {&lvl->cylinderTextures, &lvl->modelTextures})
      for (const auto& ref : *list) {
        ++refs;
        if (!lib->contains(webo::levelTextureAssetName(ref)))
          ++unresolved;
      }
  }

  check(indexed == names.size(),
        std::format("all levels index ({} of {})", indexed, names.size()));
  check(titled == names.size(), "every level has a title");
  check(refs > 20000, std::format("tens of thousands of references ({})", refs));
  check(unresolved == 0,
        std::format("every reference resolves in the archive ({} did not)", unresolved));
  check(
      withLattice == names.size(),
      std::format("every level yields the 49858-word index array ({} did)", withLattice));

  std::cout << std::format("    {} levels, {} asset references, all resolving\n", indexed,
                           refs);
}

}  // namespace

int main() {
  testAssetNames();
  testSynthetic();
  testStructuredLattice();
  testRealLevels();
  std::cout << std::format("\n{} checks, {} failures\n", g_checks, g_failures);
  return g_failures == 0 ? 0 : 1;
}
