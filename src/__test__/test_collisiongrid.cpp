// Compare the stored .LVL collision mask byte-for-byte with engine grid dumps
// when available in build/live_dump/. Check corpus-wide invariants independently.

#include "assets/Box.h"
#include "assets/CollisionGrid.h"
#include "assets/LightMap.h"
#include "assets/LevelIndex.h"
#include "sim/Terrain.h"

#include <array>
#include <bit>
#include <chrono>
#include <cstring>
#include <cstdio>
#include <cstdlib>
#include <filesystem>
#include <format>
#include <fstream>
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

using Bytes = std::vector<std::byte>;

void put16(Bytes& out, std::uint16_t value) {
  for (int i = 0; i < 2; ++i)
    out.push_back(static_cast<std::byte>(value >> (8 * i)));
}

void put32(Bytes& out, std::uint32_t value) {
  for (int i = 0; i < 4; ++i)
    out.push_back(static_cast<std::byte>(value >> (8 * i)));
}

void put64(Bytes& out, std::uint64_t value) {
  for (int i = 0; i < 8; ++i)
    out.push_back(static_cast<std::byte>(value >> (8 * i)));
}

void putName(Bytes& out, std::string_view text) {
  put32(out, static_cast<std::uint32_t>(text.size()));
  for (char c : text)
    out.push_back(static_cast<std::byte>(c));
}

// Explicit cell words, independent of the reader and the intervening arrays.
std::vector<std::uint32_t> fixtureCells() {
  std::vector<std::uint32_t> cells{0, 0x80000000u, 0xffffffffu, 0x80000081u};
  for (std::uint32_t i = 4; i < 512; ++i)
    cells.push_back(i);
  return cells;
}

constexpr std::uint64_t kNegativeZero = 0x8000000000000000ull;

Bytes structuredGrid(bool decoy, std::uint32_t height = 2,
                     std::size_t cellCount = 512, std::string_view worldName = "Cylinder") {
  Bytes out;
  putName(out, "Level 23");
  for (int i = 0; i < 7; ++i) put32(out, 0); // six fields, four flag bytes
  put32(out, 1);
  putName(out, decoy ? "Cylinder" : "OtherName");
  // Five empty counts and a zero-length name make the decoy appear to have height 0.
  for (int i = 0; i < 5; ++i) put32(out, 0);
  putName(out, "");
  put32(out, 112);
  put64(out, 0x4024000000000000ull);
  put32(out, 1);
  for (std::uint64_t i = 1; i <= 192; ++i) put64(out, i);
  put32(out, 1);
  for (std::uint64_t i = 193; i <= 224; ++i) put64(out, i);
  put32(out, 1);
  // Supported class-one record with empty triples/references/polygons/children.
  for (int i = 0; i < 4; ++i) put32(out, 0);
  for (int i = 0; i < 12; ++i) put64(out, 0);
  putName(out, worldName);
  put64(out, 0); put64(out, 0); put32(out, 0);
  put32(out, height);
  put64(out, kNegativeZero);
  put32(out, 3);
  for (auto word : {11u, 22u, 33u}) put32(out, word);
  put32(out, 2);
  for (std::uint32_t i = 44; i < 50; ++i) put32(out, i);
  const auto cells = fixtureCells();
  put32(out, static_cast<std::uint32_t>(cellCount));
  for (std::size_t i = 0; i < cellCount; ++i) put32(out, cells[i % cells.size()]);
  put32(out, 3);
  for (auto byte : {7, 8, 9}) out.push_back(static_cast<std::byte>(byte));
  out.resize(out.size() + 6144, std::byte{0});
  putName(out, "INN_fixture"); putName(out, "OUT_fixture");
  for (int i = 0; i < 32; ++i) {
    put32(out, i == 0 || i == 31 ? 1 : 0);
    if (i == 0 || i == 31) {
      put32(out, 99); put16(out, 0x8000); put16(out, 0xffff);
      for (std::uint64_t j = 0; j < 5; ++j) put64(out, j);
    }
  }
  return out;
}

Bytes legacyGridChunk() {
  Bytes out;
  putName(out, "Cylinder");
  for (int i = 0; i < 5; ++i) put32(out, 0);
  put32(out, 2); put64(out, kNegativeZero);
  put32(out, 1); put32(out, 123); // surface array
  put32(out, 1); // integer triple
  for (auto value : {4u, 5u, 6u}) put32(out, value);
  put32(out, 512);
  for (auto cell : fixtureCells()) put32(out, cell);
  return out;
}

void checkFixtureGrid(const std::expected<webo::CollisionGrid, std::string>& grid,
                      bool fallback, std::string_view label) {
  check(grid.has_value(), std::format("{} loads{}", label, grid ? "" : ": " + grid.error()));
  if (!grid) return;
  check(grid->width == 256 && grid->height == 2, std::format("{} dimensions", label));
  check(grid->cells == fixtureCells(), std::format("{} complete cell words and row order", label));
  check(std::bit_cast<std::uint64_t>(grid->cylinderExtent) == kNegativeZero,
        std::format("{} negative-zero extent bits", label));
  check(grid->diagnostic.empty() == !fallback, std::format("{} diagnostic policy", label));
}

void writeBytes(const fs::path& path, const Bytes& bytes) {
  std::ofstream out(path, std::ios::binary);
  out.write(reinterpret_cast<const char*>(bytes.data()), static_cast<std::streamsize>(bytes.size()));
  out.close();
  check(static_cast<bool>(out), std::format("write {}", path.string()));
}

Bytes singleEntryBox(std::string_view name, const Bytes& payload) {
  Bytes out;
  for (char c : std::string_view("LEMBOX")) out.push_back(static_cast<std::byte>(c));
  put32(out, 1); put32(out, static_cast<std::uint32_t>(4 + name.size()));
  putName(out, name);
  put32(out, 1); put32(out, static_cast<std::uint32_t>(14 + 4 + name.size() + 16));
  put32(out, 1); put32(out, static_cast<std::uint32_t>(payload.size()));
  out.insert(out.end(), payload.begin(), payload.end());
  return out;
}

void testStructuredLoading() {
  std::cout << "asset-free structured collision loading\n";
  auto plain = structuredGrid(false);
  auto decoy = structuredGrid(true);
  const auto plainOriginal = plain, decoyOriginal = decoy;
  const auto a = webo::findCollisionGrid(plain);
  const auto b = webo::findCollisionGrid(decoy);
  checkFixtureGrid(a, false, "without decoy");
  checkFixtureGrid(b, false, "with earlier Cylinder name");
  check(plain == plainOriginal && decoy == decoyOriginal, "caller input unchanged");
  if (a && b)
    check(a->cells == b->cells && a->width == b->width && a->height == b->height &&
              std::bit_cast<std::uint64_t>(a->cylinderExtent) ==
                  std::bit_cast<std::uint64_t>(b->cylinderExtent), "decoy-independent result");
  decoy.insert(decoy.end(), {std::byte{0xff}, std::byte{0x13}, std::byte{0x37}});
  checkFixtureGrid(webo::findCollisionGrid(decoy), false, "unrelated trailing bytes");

  Bytes fallback;
  putName(fallback, "junk");
  for (int i = 0; i < 7; ++i) put32(fallback, 0);
  put32(fallback, 0xffffffffu); // forces an invalid level name count
  const auto chunk = legacyGridChunk();
  fallback.insert(fallback.end(), chunk.begin(), chunk.end());
  const auto f = webo::findCollisionGrid(fallback);
  checkFixtureGrid(f, true, "non-level legacy chunk");
  if (f) check(f->diagnostic.find("invalid level name count") != std::string::npos,
               "successful fallback retains recovered decoder message");

  auto truncatedWorld = structuredGrid(false);
  truncatedWorld.pop_back(); // complete grid, incomplete final world tail record
  const auto worldFallback = webo::findCollisionGrid(truncatedWorld);
  checkFixtureGrid(worldFallback, true, "world-reader rejection fallback");
  if (worldFallback)
    check(worldFallback->diagnostic.find("invalid world tail count") != std::string::npos,
          "fallback retains world-reader rejection after a valid prefix");

  // A valid locator chunk checks that dimension or count failures cannot trigger fallback.
  for (const auto height : {0u, 4097u, 0xffffffffu}) {
    auto invalid = structuredGrid(false, height, 512, "World");
    invalid.insert(invalid.end(), chunk.begin(), chunk.end());
    const auto result = webo::findCollisionGrid(invalid);
    check(!result && result.error() == std::format("collision grid: implausible height {}", height),
          "decoded invalid height returns expected error without fallback");
  }
  for (const auto count : {0u, 511u, 513u}) {
    auto invalid = structuredGrid(false, 2, count, "World");
    invalid.insert(invalid.end(), chunk.begin(), chunk.end());
    const auto result = webo::findCollisionGrid(invalid);
    check(!result && result.error() == std::format(
              "collision grid: expected 512 cells for 2 rows, the file says {}", count),
          "decoded inconsistent count returns expected error without fallback");
  }
  // Prefix, world arrays, grid and final tail truncation: no exception may
  // escape the public expected interface. Avoid a locator-readable world name.
  const auto bounded = structuredGrid(false, 2, 512, "World");
  for (auto length : {std::size_t{0}, std::size_t{1}, std::size_t{40},
                      std::size_t{100}, std::size_t{2200}, bounded.size() - 1}) {
    const auto result = webo::findCollisionGrid(std::span(bounded).first(length));
    check(!result && result.error().find("recovered reader") != std::string::npos &&
              result.error().find("legacy locator: collision grid: no") != std::string::npos,
          "truncated structured data retains both rejection causes");
  }
  Bytes brokenChunk = chunk;
  brokenChunk.pop_back();
  const auto broken = webo::findCollisionGrid(brokenChunk);
  check(!broken && broken.error().find("recovered reader") != std::string::npos &&
            broken.error().find("grid runs past the end") != std::string::npos,
        "failed fallback retains decoder and locator errors");

  // Fresh test-owned directory below ignored build, independent of CWD/game.
  const auto root = fs::path(WEBOLUTION_SOURCE_DIR) / "build" / "native-010";
  fs::create_directories(root);
  fs::path dir;
  for (std::uint64_t attempt = 0;; ++attempt) {
    const auto stamp = std::chrono::steady_clock::now().time_since_epoch().count();
    dir = root / std::format("archive-test-{}-{}", stamp, attempt);
    if (fs::create_directory(dir)) break;
  }
  constexpr std::string_view asset = "COMPILED\\SYNTHETIC.LVL";
  writeBytes(dir / "base.box", singleEntryBox(asset, fallback));
  writeBytes(dir / "patch.box", singleEntryBox(asset, decoy));
  auto base = webo::AssetLibrary::open(dir / "base.box");
  check(base.has_value(), "synthetic base archive opens");
  if (base) {
    checkFixtureGrid(webo::loadCollisionGrid(*base, asset), true, "archive fallback");
    check(!webo::loadCollisionGrid(*base, "missing"), "archive missing asset returns error");
  }
  auto overlay = webo::AssetLibrary::open(dir / "base.box", dir / "patch.box");
  check(overlay.has_value(), "synthetic patch overlay opens");
  if (overlay)
    checkFixtureGrid(webo::loadCollisionGrid(*overlay, "compiled/synthetic.lvl"), false,
                     "archive structured patch override");
}

// Check literal output bytes for the portable binary64 conversion.
// Original x87 equivalence is unestablished.
struct LightChannelCase { std::uint64_t bits; std::uint8_t expected; };
constexpr std::array<LightChannelCase, 16> kLightChannels{{
    {0x405ff00000000000ull, 127}, // 127.75
    {0x4050200000000000ull, 64},  // 64.5
    {0x4072c00000000000ull, 255}, // 300
    {0xbff0000000000000ull, 0},   // -1
    {0x8000000000000000ull, 0},   // -0
    {0x3fe8000000000000ull, 0},   // 0.75
    {0x3ff8000000000000ull, 1},   // 1.5
    {0x406fd80000000000ull, 254}, // 254.75
    {0x406fe00000000000ull, 255}, // 255
    {0x7ff0000000000000ull, 0},   // +infinity
    {0xfff0000000000000ull, 0},   // -infinity
    {0x7ff8000000001234ull, 0},   // quiet NaN
    {0x4027000000000000ull, 11},  // 11.5
    {0x4036800000000000ull, 22},  // 22.5
    {0x4040c00000000000ull, 33},  // 33.5
    {0x4004000000000000ull, 2},   // 2.5
}};
constexpr std::array<std::uint8_t, 16> kLightMapPattern{
    0, 255, 2, 1, 7, 12, 19, 31, 63, 127, 128, 192, 254, 3, 4, 5};

void putLightPayload(Bytes& out, std::size_t mapLength) {
  put32(out, static_cast<std::uint32_t>(mapLength));
  for (std::size_t i = 0; i < mapLength; ++i)
    out.push_back(static_cast<std::byte>(kLightMapPattern[i % 16]));
  for (std::size_t i = 0; i < 768; ++i) put64(out, kLightChannels[i % 16].bits);
}

struct LightFixture {
  Bytes bytes;
  std::size_t worldStart;
  std::size_t mapStart;
  std::size_t paletteStart;
};

LightFixture structuredLight(bool decoy, std::uint32_t height = 96,
                             std::size_t mapLength = 12288,
                             std::string_view worldName = "Cylinder") {
  LightFixture fixture{};
  auto& out = fixture.bytes;
  putName(out, "Level 23");
  for (int i = 0; i < 7; ++i) put32(out, 0);
  put32(out, 1); putName(out, decoy ? "Cylinder" : "OtherName");
  // Name tables, object count and short string complete the zero-height decoy before the world.
  for (int i = 0; i < 5; ++i) put32(out, 0);
  putName(out, "");
  fixture.worldStart = out.size();
  put32(out, 112); put64(out, 0x4024000000000000ull);
  put32(out, 1);
  for (std::uint64_t i = 1; i <= 192; ++i) put64(out, i);
  put32(out, 1);
  for (std::uint64_t i = 193; i <= 224; ++i) put64(out, i);
  put32(out, 1); // class-leading value
  // Class-one triples and named references are populated as well.
  for (int list = 0; list < 2; ++list) {
    put32(out, 1);
    for (std::uint64_t i = 0; i < 3; ++i) put64(out, 0x3ff0000000000000ull + i);
  }
  put32(out, 1); putName(out, "fixture-reference"); put32(out, 91);
  put32(out, 0); // polygons
  for (int i = 0; i < 12; ++i) put64(out, 0);
  putName(out, worldName);
  put64(out, 0); put64(out, 0); put32(out, 0); // final doubles, children
  put32(out, height); put64(out, 0x3fd70a3d70a3d70aull);
  put32(out, 3);
  for (auto value : {11u, 22u, 33u}) put32(out, value);
  put32(out, 2);
  for (auto value : {44u, 45u, 46u, 47u, 48u, 49u}) put32(out, value);
  // Deliberately unrelated to height: light loading has no collision-count
  // policy. A scene's other prerequisites are outside this fixture's scope.
  put32(out, 3);
  for (auto value : {0u, 0xffffffffu, 0x80000081u}) put32(out, value);
  fixture.mapStart = out.size() + 4;
  fixture.paletteStart = fixture.mapStart + mapLength;
  putLightPayload(out, mapLength);
  putName(out, "INN_light"); putName(out, "OUT_light");
  for (int list = 0; list < 32; ++list) {
    put32(out, list == 0 || list == 31 ? 1 : 0);
    if (list == 0 || list == 31) {
      put32(out, 99); put16(out, 0x8000); put16(out, 0xffff);
      for (std::uint64_t i = 0; i < 5; ++i) put64(out, i);
    }
  }
  return fixture;
}

Bytes legacyLightChunk() {
  Bytes out;
  putName(out, "Cylinder");
  for (int i = 0; i < 5; ++i) put32(out, 0);
  put32(out, 96); put64(out, 0);
  put32(out, 2); put32(out, 123); put32(out, 456);
  put32(out, 1); put32(out, 4); put32(out, 5); put32(out, 6);
  put32(out, 1); put32(out, 0x80000081u);
  putLightPayload(out, 12288);
  return out;
}

void checkFixtureLight(const std::expected<webo::ActorLightMap, std::string>& light,
                       bool fallback, std::string_view label) {
  check(light.has_value(), std::format("{} loads{}", label, light ? "" : ": " + light.error()));
  if (!light) return;
  check(light->diagnostic.empty() == !fallback, std::format("{} diagnostic policy", label));
  check(light->map.size() == 12288, std::format("{} literal map length", label));
  std::size_t wrongBytes = 0, wrongChannels = 0;
  for (std::size_t i = 0; i < light->map.size(); ++i)
    wrongBytes += light->map[i] != kLightMapPattern[i % 16];
  for (std::size_t i = 0; i < 768; ++i)
    wrongChannels += light->palette[i / 3][i % 3] != kLightChannels[i % 16].expected;
  check(wrongBytes == 0, std::format("{} every map byte in serialized order", label));
  check(wrongChannels == 0, std::format("{} all 768 converted RGB channels", label));
  check(light->histogram(0) == 768 && light->histogram(255) == 768,
        std::format("{} endpoint map indices", label));
}

void testStructuredActorLightLoading() {
  std::cout << "asset-free structured actor light loading\n";
  const auto plain = structuredLight(false), decoy = structuredLight(true);
  const auto plainOriginal = plain.bytes, decoyOriginal = decoy.bytes;
  const auto a = webo::findActorLightMap(plain.bytes);
  const auto b = webo::findActorLightMap(decoy.bytes);
  checkFixtureLight(a, false, "light without decoy");
  checkFixtureLight(b, false, "light with earlier Cylinder name");
  check(plain.bytes == plainOriginal && decoy.bytes == decoyOriginal, "light caller input preserved");
  if (a && b) {
    check(a->map == b->map && a->palette == b->palette, "light decoy independence");
    const auto zero = webo::actorLightTintAt(*b, 0.0, 0.0);
    check(zero && *zero == std::array<std::uint8_t, 3>{127, 64, 255}, "map index 0 non-grey RGB");
    const auto tint = webo::actorLightTintAt(*b, 2.0, 0.0);
    check(webo::actorLightMapIndex(2.0, 0.0) == 1 && tint &&
              *tint == std::array<std::uint8_t, 3>{22, 33, 2}, "known coordinate selects palette 255");
    if (tint)
      check(webo::modulateActorSpritePixel({128, 255, 200, 197}, *tint) ==
                std::array<std::uint8_t, 4>{11, 33, 1, 197}, "known tint modulation and retained alpha");
  }
  auto trailing = decoy.bytes;
  trailing.insert(trailing.end(), {std::byte{0xff}, std::byte{0x13}, std::byte{0x37}});
  checkFixtureLight(webo::findActorLightMap(trailing), false, "light world followed by unrelated bytes");
  for (const auto height : {1u, 4096u})
    checkFixtureLight(webo::findActorLightMap(structuredLight(false, height).bytes), false,
                      "light accepted height boundary independent of map and collision lengths");

  Bytes fallback;
  putName(fallback, "junk");
  for (int i = 0; i < 7; ++i) put32(fallback, 0);
  put32(fallback, 0xffffffffu);
  const auto chunk = legacyLightChunk();
  fallback.insert(fallback.end(), chunk.begin(), chunk.end());
  const auto f = webo::findActorLightMap(fallback);
  checkFixtureLight(f, true, "light diagnosed legacy fallback");
  if (f) check(f->diagnostic == "actor light map: recovered reader at byte 40: invalid level name count",
               "light fallback retains exact rejection and cursor");
  auto tailTruncated = plain.bytes;
  tailTruncated.pop_back();
  const auto tailFallback = webo::findActorLightMap(tailTruncated);
  checkFixtureLight(tailFallback, true, "light world-tail rejection fallback");
  if (tailFallback)
    check(tailFallback->diagnostic.find("invalid world tail count") != std::string::npos &&
              tailFallback->diagnostic.find("at byte ") != std::string::npos,
          "light fallback retains world reader rejection and cursor");

  // The real world uses another name; a later locator-readable chunk is
  // usable in isolation. Projection failures must never select that chunk.
  checkFixtureLight(webo::findActorLightMap(chunk), true, "later legacy light chunk is usable");
  for (const auto height : {0u, 4097u, 0xffffffffu}) {
    auto invalid = structuredLight(false, height, 12288, "World").bytes;
    invalid.insert(invalid.end(), chunk.begin(), chunk.end());
    const auto result = webo::findActorLightMap(invalid);
    check(!result && result.error() == std::format("actor light map: implausible height {}", height),
          "light decoded invalid height does not fall back");
  }
  for (const auto count : {0u, 12287u, 12289u}) {
    auto invalid = structuredLight(false, 96, count, "World").bytes;
    invalid.insert(invalid.end(), chunk.begin(), chunk.end());
    const auto result = webo::findActorLightMap(invalid);
    check(!result && result.error() == std::format(
              "actor light map: expected 12288 map bytes, the file says {}", count),
          "light decoded wrong map length does not fall back");
  }
  const auto bounded = structuredLight(false, 96, 12288, "World");
  for (auto length : {std::size_t{0}, std::size_t{1}, std::size_t{40}, bounded.worldStart,
                      bounded.worldStart + 20, bounded.mapStart - 1, bounded.mapStart + 12,
                      bounded.paletteStart, bounded.paletteStart + 6143, bounded.bytes.size() - 1}) {
    const auto result = webo::findActorLightMap(std::span(bounded.bytes).first(length));
    check(!result && result.error().find("recovered reader at byte ") != std::string::npos &&
              result.error().find("legacy locator: actor light map: no") != std::string::npos,
          "light truncated prefix/world/map/palette/tail returns both causes");
  }
  auto brokenChunk = chunk;
  brokenChunk.pop_back();
  const auto broken = webo::findActorLightMap(brokenChunk);
  check(!broken && broken.error().find("recovered reader at byte ") != std::string::npos &&
            broken.error().find("legacy locator: actor light map: map or palette runs past") != std::string::npos,
        "light failed fallback retains both causes");

  const auto root = fs::path(WEBOLUTION_SOURCE_DIR) / "build" / "native-011";
  fs::create_directories(root);
  fs::path dir;
  for (std::uint64_t attempt = 0;; ++attempt) {
    const auto stamp = std::chrono::steady_clock::now().time_since_epoch().count();
    dir = root / std::format("archive-test-{}-{}", stamp, attempt);
    if (fs::create_directory(dir)) break;
  }
  // Export the same synthetic inputs for the public loader comparisons.
  writeBytes(dir / "plain.lvl", plain.bytes);
  writeBytes(dir / "decoy.lvl", decoy.bytes);
  constexpr std::string_view asset = "COMPILED\\SYNTHETIC.LVL";
  writeBytes(dir / "base.box", singleEntryBox(asset, fallback));
  writeBytes(dir / "patch.box", singleEntryBox(asset, trailing));
  auto base = webo::AssetLibrary::open(dir / "base.box");
  check(base.has_value(), "light synthetic base opens");
  if (base) {
    checkFixtureLight(webo::loadActorLightMap(*base, asset), true, "light archive base fallback");
    check(!webo::loadActorLightMap(*base, "missing"), "light archive missing asset returns error");
  }
  auto overlay = webo::AssetLibrary::open(dir / "base.box", dir / "patch.box");
  check(overlay.has_value(), "light synthetic patch overlay opens");
  if (overlay)
    checkFixtureLight(webo::loadActorLightMap(*overlay, "compiled/synthetic.lvl"), false,
                      "light archive patch precedence");
  std::cout << "  light regression fixtures and archives: " << dir.string() << '\n';
}

// Every retail level: 256 cells around the tube, one row per world unit of
// its 96-unit height.
constexpr int kWidth = 256;
constexpr int kHeight = 96;
constexpr std::size_t kLevels = 102;

// Original grid dump names and solid fractions.
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
  std::cout << "collision grid against retained historical live dumps\n";
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
        << "  no live dumps present; historical comparison skipped (no capture authorized)\n";
  else
    std::cout << std::format("  compared {} retained historical dumps\n", compared);
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
  // The sparsest stored retail mask is 5.5% solid. The lattice raster can be empty.
  check(empty == 0, std::format("no level has an empty mask ({} do)", empty));
  check(minPct > 1.0,
        std::format("sparsest level is still real terrain ({:.2f}%)", minPct));

  std::cout << std::format("  {} grids, {}x{}, solid {:.1f}%..{:.1f}%\n", parsed, kWidth,
                           kHeight, minPct, maxPct);
}

void testActorLightMap(const webo::AssetLibrary& lib) {
  std::cout << "actor light map on MM_CLIMB_002\n";
  auto blob = lib.read("COMPILED\\MM_CLIMB_002.LVL");
  if (!blob) {
    check(false, blob.error());
    return;
  }
  const auto light = webo::findActorLightMap(*blob);
  if (!light) {
    check(false, light.error());
    return;
  }
  check(light->map.size() == webo::ActorLightMap::kMapBytes, "map length is 12288");
  check(light->histogram(0) == 9381, std::format("byte 0 count is 9381 (got {})", light->histogram(0)));
  check(light->histogram(12) == 641, std::format("byte 12 count is 641 (got {})", light->histogram(12)));

  const std::array<std::uint8_t, 13> expected{152, 147, 140, 131, 123, 116, 107, 98, 92, 82, 74, 68, 61};
  for (int i = 0; i < 13; ++i) {
    const auto& rgb = light->palette[static_cast<std::size_t>(i)];
    check(rgb[0] == expected[static_cast<std::size_t>(i)] &&
              rgb[1] == expected[static_cast<std::size_t>(i)] &&
              rgb[2] == expected[static_cast<std::size_t>(i)],
          std::format("palette[{}] grey {}", i, expected[static_cast<std::size_t>(i)]));
  }

  // File offsets for the map count and palette start.
  constexpr std::size_t kMapCountOffset = 338315;
  constexpr std::size_t kPaletteOffset = 350607;
  std::uint32_t mapCount = 0;
  std::memcpy(&mapCount, blob->data() + kMapCountOffset, 4);
  check(mapCount == 12288, "map count u32 at file offset 338315");
  check(blob->size() >= kPaletteOffset + webo::ActorLightMap::kPaletteBytes,
        "palette fits after offset 350607");

  // Original actor+0x08/+0x10 before cylinder projection.
  constexpr double fixtureTerrainX = 59.68486785888672;
  constexpr double fixtureFileRowY = 32.0;
  const int idxFixture = webo::actorLightMapIndex(fixtureTerrainX, fixtureFileRowY);
  const int idxMinus57 = webo::actorLightMapIndex(-57.0, 32.0);
  check(idxFixture == 4125, std::format("A4f fixture index is 4125 (got {})", idxFixture));
  check(idxMinus57 == 4195, std::format("X=-57 index is 4195 (got {})", idxMinus57));
  check(light->map[static_cast<std::size_t>(idxFixture)] == 0, "fixture map byte is 0");
  check(light->map[static_cast<std::size_t>(idxMinus57)] == 0, "X=-57 map byte is 0");
  const auto fixtureTint = webo::actorLightTintAt(*light, fixtureTerrainX, fixtureFileRowY);
  check(fixtureTint.has_value() && (*fixtureTint)[0] == 152 && (*fixtureTint)[1] == 152 &&
            (*fixtureTint)[2] == 152,
        "fixture lookup RGB is 152");
  check(webo::actorLightMapIndex(-0.25, 32.0) == webo::actorLightMapIndex(0.0, 32.0),
        "X in (-1,0) uses trunc toward zero");

  std::size_t nonzeroIndex = light->map.size();
  for (std::size_t i = 0; i < light->map.size(); ++i)
    if (light->map[i] == 12) {
      nonzeroIndex = i;
      break;
    }
  check(nonzeroIndex < light->map.size(), "found a map cell with byte 12");
  if (nonzeroIndex < light->map.size()) {
    int row = -1;
    int xLow = -1;
    for (int candidateRow = 0; candidateRow < 96; ++candidateRow) {
      const int rem = static_cast<int>(nonzeroIndex) - ((candidateRow << 8) / 2);
      if (rem < 0 || rem > 127)
        continue;
      xLow = rem * 2;
      row = candidateRow;
      break;
    }
    check(row >= 0 && xLow >= 0 && xLow <= 255, "derived row/X for byte-12 cell");
    const double terrainX = static_cast<double>(xLow);
    const double fileRowY = static_cast<double>(row);
    check(webo::actorLightMapIndex(terrainX, fileRowY) ==
              static_cast<int>(nonzeroIndex),
          "constructed X/Y hits the byte-12 map cell");
    const auto tint = webo::actorLightTintAt(*light, terrainX, fileRowY);
    check(tint.has_value(), "non-zero map cell resolves through the palette");
    if (tint)
      check((*tint)[0] == 61 && (*tint)[1] == 61 && (*tint)[2] == 61,
            "palette entry 12 is grey 61");
  }
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

  // Compare each cell across the vertical flip. Terrain is Y-down and the file grid
  // is Y-up, so fromCollisionGrid maps file row r to terrain row height - 1 - r.
  // A walker at world Y 32.190 has solid file row 31 and empty row 32.
  std::size_t mismatched = 0;
  for (int y = 0; y < grid->height; ++y)
    for (int x = 0; x < grid->width; ++x)
      if (terrain.solid(x, grid->height - 1 - y) != grid->solid(x, y))
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
  testStructuredLoading();
  testStructuredActorLightLoading();
  const char* dir = std::getenv("WEBOLUTION_GAME_DIR");
  if (!dir) {
    std::cout << "collision grid: skipped, WEBOLUTION_GAME_DIR is not set\n";
    std::cout << std::format("\n{} checks, {} failures\n", g_checks, g_failures);
    return g_failures == 0 ? 0 : 1;
  }
  if (!fs::exists(fs::path(dir) / "lemmings.box")) {
    std::cout << "collision grid: skipped, no lemmings.box\n";
    std::cout << std::format("\n{} checks, {} failures\n", g_checks, g_failures);
    return g_failures == 0 ? 0 : 1;
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
  testActorLightMap(*lib);
  testTerrainMapping(*lib);

  std::cout << std::format("\n{} checks, {} failures\n", g_checks, g_failures);
  return g_failures == 0 ? 0 : 1;
}
