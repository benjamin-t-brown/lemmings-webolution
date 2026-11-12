// Restored asset-side coverage (PLAN.md Phase A, debt 2).
//
// Which .SPR sprite each simulation state is drawn with. The mapping lives in
// a renderer-independent header so it can be checked without a GL context or
// an SDL window; sprite drawing itself is a later presentation task.
//
// Most states name a sprite directly. The two that do not are deliberate and
// marked as such in the header: Splatter ("cartwheel") is an interpretation,
// and Exited has no artwork. The retail sweep confirms every non-empty name
// actually exists in the shipped LEMMINGSLO.SPR, which is the part that cannot
// be inferred from the state enum alone.

#include "assets/Box.h"
#include "assets/Sprite.h"
#include "gfx/SpriteState.h"
#include "sim/World.h"

#include <array>
#include <bit>
#include <chrono>
#include <cstdlib>
#include <filesystem>
#include <format>
#include <fstream>
#include <iostream>
#include <stdexcept>
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

void put32(Bytes& out, std::uint32_t value) {
  for (unsigned i = 0; i < 4; ++i)
    out.push_back(static_cast<std::byte>((value >> (8 * i)) & 0xff));
}

void put64(Bytes& out, std::uint64_t bits) {
  for (unsigned i = 0; i < 8; ++i)
    out.push_back(static_cast<std::byte>((bits >> (8 * i)) & 0xff));
}

void putName(Bytes& out, std::string_view name) {
  put32(out, static_cast<std::uint32_t>(name.size()));
  for (unsigned char c : name)
    out.push_back(static_cast<std::byte>(c));
}

void putPiece(Bytes& out, std::array<std::uint32_t, 4> first,
              std::array<std::uint64_t, 4> bits, std::array<std::uint32_t, 4> last,
              std::string_view name) {
  for (auto value : first) put32(out, value);
  for (auto value : bits) put64(out, value);
  for (auto value : last) put32(out, value);
  putName(out, name);
}

// Explicit serialized input, independent of both parser implementations.
Bytes clips() {
  Bytes out;
  put32(out, 3);  // clips
  put32(out, 3);  // clip 0: three temporal entries
  put32(out, 2);  // first entry: two pieces
  putPiece(out, {0xffffffff, 0, 0x80000000, 17},
           {0x8000000000000000ULL, 0x7ff8123456789abcULL,
            0x3ff0000000000000ULL, 0xbfe0000000000000ULL},
           {0x87654321, 0xffffffff, 23, 31}, std::string_view("Sheets\\A\0tail", 13));
  putPiece(out, {19, 2, 3, 47},
           {0x3fe0000000000000ULL, 0x3fd0000000000000ULL,
            0x3fc0000000000000ULL, 0x3fb0000000000000ULL},
           {5, 6, 44, 17}, "Sheets\\B");
  put32(out, 0);  // second entry: empty pieces
  put32(out, 1);  // third entry
  putPiece(out, {101, 102, 103, 104},
           {0xfff8abcdef012345ULL, 0x8000000000000000ULL,
            0x4000000000000000ULL, 0xc008000000000000ULL},
           {105, 106, 107, 108}, "");
  putName(out, std::string_view("walk\0tail", 9));
  put64(out, 0x8000000000000000ULL);
  put32(out, 0);  // clip 1: no temporal entries
  putName(out, "empty");
  put64(out, 0x7ff8fedcba987654ULL);
  put32(out, 1);  // clip 2
  put32(out, 1);
  putPiece(out, {201, 202, 203, 204},
           {0x4010000000000000ULL, 0x4014000000000000ULL,
            0x4018000000000000ULL, 0x401c000000000000ULL},
           {205, 206, 207, 208}, std::string(259, 'T') + 'Z');
  putName(out, std::string(127, 'C') + 'Z');
  put64(out, 0x4020000000000000ULL);
  return out;
}

void putTriples(Bytes& out, unsigned count, std::uint64_t seed) {
  for (unsigned i = 0; i < count * 3; ++i)
    put64(out, seed + i);
}

Bytes mixedResource() {
  auto out = clips();
  put32(out, 5);
  put32(out, 0);
  put64(out, 0x8000000000000000ULL);
  put64(out, 0x7ff8123456789abcULL);
  put64(out, 0x3ff0000000000000ULL);
  putTriples(out, 15, 0x4000000000000000ULL);
  put32(out, 2);
  put32(out, 1);
  putTriples(out, 256, 0x4010000000000000ULL);
  put32(out, 0xffffffff);
  put32(out, 0);
  putTriples(out, 16, 0x4020000000000000ULL);
  return out;
}

Bytes singleTag(std::uint32_t tag) {
  Bytes out;
  put32(out, 0);
  put32(out, 1);
  put32(out, tag);
  if (tag == 1) putTriples(out, 256, 0x4030000000000000ULL);
  return out;
}

void checkPiece(const webo::SpriteFrame& p, std::array<std::uint32_t, 4> rect,
                std::array<std::uint64_t, 4> uv, std::array<std::uint32_t, 4> rest,
                std::string_view name) {
  check(std::array{p.bottom, p.top, p.left, p.right} == rect, "complete integer rectangle");
  check(std::array{std::bit_cast<std::uint64_t>(p.uBottom), std::bit_cast<std::uint64_t>(p.uTop),
                   std::bit_cast<std::uint64_t>(p.uLeft), std::bit_cast<std::uint64_t>(p.uRight)}
            == uv, "complete binary64 bits in UV order");
  check(std::array{p.unknown0, p.unknown1, p.width, p.height} == rest,
        "complete unknown and size integers");
  check(p.sheet == name, "bounded texture name");
}

void checkClips(const webo::SpriteSheet& sheet) {
  check(sheet.sprites.size() == 3, "three clips, in order");
  check(sheet.frameCount() == 4, "four total pieces");
  if (sheet.sprites.size() != 3) return;
  const auto& a = sheet.sprites[0];
  const auto& b = sheet.sprites[1];
  const auto& c = sheet.sprites[2];
  check(a.name == "walk" && b.name == "empty" && c.name == std::string(127, 'C'),
        "clip names: embedded NUL and exact capacity (128 -> 127)");
  check(sheet.find("walk") == &a && sheet.find("missing") == nullptr, "find projected names");
  check(std::bit_cast<std::uint64_t>(a.rate) == 0x8000000000000000ULL &&
        std::bit_cast<std::uint64_t>(b.rate) == 0x7ff8fedcba987654ULL &&
        std::bit_cast<std::uint64_t>(c.rate) == 0x4020000000000000ULL,
        "clip speeds retain negative zero and quiet-NaN payload");
  check(a.animations.size() == 3 && b.animations.empty() && c.animations.size() == 1,
        "temporal counts including empty clip");
  if (a.animations.size() != 3 || c.animations.size() != 1) return;
  check(a.animations[0].frames.size() == 2 && a.animations[1].frames.empty() &&
        a.animations[2].frames.size() == 1 && c.animations[0].frames.size() == 1,
        "piece counts including empty temporal entry");
  if (a.animations[0].frames.size() != 2 || a.animations[2].frames.size() != 1 ||
      c.animations[0].frames.size() != 1) return;
  checkPiece(a.animations[0].frames[0], {0xffffffff, 0, 0x80000000, 17},
             {0x8000000000000000ULL, 0x7ff8123456789abcULL,
              0x3ff0000000000000ULL, 0xbfe0000000000000ULL},
             {0x87654321, 0xffffffff, 23, 31}, "Sheets\\A");
  checkPiece(a.animations[0].frames[1], {19, 2, 3, 47},
             {0x3fe0000000000000ULL, 0x3fd0000000000000ULL,
              0x3fc0000000000000ULL, 0x3fb0000000000000ULL},
             {5, 6, 44, 17}, "Sheets\\B");
  checkPiece(a.animations[2].frames[0], {101, 102, 103, 104},
             {0xfff8abcdef012345ULL, 0x8000000000000000ULL,
              0x4000000000000000ULL, 0xc008000000000000ULL},
             {105, 106, 107, 108}, "");
  checkPiece(c.animations[0].frames[0], {201, 202, 203, 204},
             {0x4010000000000000ULL, 0x4014000000000000ULL,
              0x4018000000000000ULL, 0x401c000000000000ULL},
             {205, 206, 207, 208}, std::string(259, 'T'));
}

void checkTriples(const webo::SpriteSheet& sheet) {
  check(sheet.palette.size() == 288, "mixed 0/2/1/FFFFFFFF/0 blocks flatten to 16+256+16");
  if (sheet.palette.size() != 288) return;
  const std::array<std::uint64_t, 3> first = {
      0x8000000000000000ULL, 0x7ff8123456789abcULL, 0x3ff0000000000000ULL};
  for (std::size_t i = 0; i < 288; ++i) {
    for (std::size_t component = 0; component < 3; ++component) {
      const auto expected = i == 0 ? first[component] :
          i < 16 ? 0x4000000000000000ULL + (i - 1) * 3 + component :
          i < 272 ? 0x4010000000000000ULL + (i - 16) * 3 + component :
                    0x4020000000000000ULL + (i - 272) * 3 + component;
      check(std::bit_cast<std::uint64_t>(sheet.palette[i][component]) == expected,
            std::format("flattened triple {} component {} bits", i, component));
    }
  }
}

void testSerializedProjection() {
  std::cout << "asset-free serialized SPR projection\n";
  const auto bytes = mixedResource();
  try {
    auto sheet = webo::parseSpriteSheet(bytes);
    check(sheet.has_value(), "mixed resource parses through production API");
    if (sheet) {
      checkClips(*sheet);
      checkTriples(*sheet);
    } else {
      std::cerr << "    " << sheet.error() << '\n';
    }
    auto noBlocks = clips();
    put32(noBlocks, 0);
    auto onlyClips = webo::parseSpriteSheet(noBlocks);
    check(onlyClips.has_value(), "clips without tagged arrays");
    if (onlyClips) {
      checkClips(*onlyClips);
      check(onlyClips->palette.empty(), "no triples without blocks");
    }
    Bytes empty;
    put32(empty, 0);
    put32(empty, 0);
    auto none = webo::parseSpriteSheet(empty);
    check(none && none->sprites.empty() && none->palette.empty() && none->frameCount() == 0,
          "empty root collections");
    // Every strict prefix is incomplete, covering primitive, string, count and
    // triple boundaries, with exact EOF required by the public entry point.
    for (std::size_t n = 0; n < bytes.size(); ++n) {
      auto truncated = webo::parseSpriteSheet(std::span(bytes).first(n));
      check(!truncated && !truncated.error().empty(), std::format("truncated prefix {}", n));
    }
    auto trailing = bytes;
    trailing.push_back(std::byte{0xa5});
    auto extra = webo::parseSpriteSheet(trailing);
    check(!extra && !extra.error().empty(), "trailing byte is an expected error");
  } catch (const std::invalid_argument& error) {
    check(false, std::format("parseSpriteSheet leaked invalid_argument: {}", error.what()));
  }
}

struct BoxEntry {
  std::string name;
  Bytes payload;
};

bool writeBox(const fs::path& path, const std::vector<BoxEntry>& entries) {
  Bytes names;
  for (const auto& e : entries) putName(names, e.name);
  Bytes out;
  for (char c : std::string_view("LEMBOX")) out.push_back(static_cast<std::byte>(c));
  const auto count = static_cast<std::uint32_t>(entries.size());
  put32(out, count);
  put32(out, static_cast<std::uint32_t>(names.size()));
  out.insert(out.end(), names.begin(), names.end());
  put32(out, count);
  auto offset = static_cast<std::uint32_t>(14 + names.size() + 8 + count * 8);
  for (const auto& e : entries) {
    put32(out, offset);
    offset += static_cast<std::uint32_t>(e.payload.size());
  }
  put32(out, count);
  for (const auto& e : entries) put32(out, static_cast<std::uint32_t>(e.payload.size()));
  for (const auto& e : entries) out.insert(out.end(), e.payload.begin(), e.payload.end());
  std::ofstream file(path, std::ios::binary);
  file.write(reinterpret_cast<const char*>(out.data()), static_cast<std::streamsize>(out.size()));
  file.close();
  return bool(file);
}

void testArchiveLoading() {
  std::cout << "asset-free BOX-backed sprite loading\n";
  // Atomically claim a fresh directory; never reuse or remove another run's files.
  const fs::path root(WEBOLUTION_SPRITE_SCRATCH_ROOT);
  fs::create_directories(root);
  const auto stamp = std::chrono::steady_clock::now().time_since_epoch().count();
  fs::path scratch;
  for (unsigned attempt = 0; attempt < 100; ++attempt) {
    auto candidate = root / std::format("run-{}-{}", stamp, attempt);
    if (fs::create_directory(candidate)) {
      scratch = std::move(candidate);
      break;
    }
  }
  check(!scratch.empty(), "fresh owned scratch directory");
  if (scratch.empty()) return;
  auto truncated = mixedResource();
  truncated.pop_back();
  auto trailing = mixedResource();
  trailing.push_back(std::byte{0xa5});
  const auto archive = scratch / "synthetic.box";
  check(writeBox(archive, {{"SPRITES\\TAG1.SPR", singleTag(1)},
                           {"SPRITES\\TAG2.SPR", singleTag(2)},
                           {"SPRITES\\MIXED.SPR", mixedResource()},
                           {"SPRITES\\TRUNCATED.SPR", truncated},
                           {"SPRITES\\TRAILING.SPR", trailing}}), "write synthetic BOX");
  auto lib = webo::AssetLibrary::open(archive);
  check(lib.has_value(), "open synthetic BOX");
  if (!lib) return;
  check(lib->base().validate().ok && lib->base().validate().gaps == 0, "synthetic BOX covers payload");
  try {
    for (unsigned tag : {1u, 2u}) {
      const auto name = std::format("SPRITES\\TAG{}.SPR", tag);
      auto loaded = webo::loadSpriteSheet(*lib, name);
      check(loaded.has_value(), std::format("archive-backed tag-{} regression", tag));
      if (!loaded) {
        std::cout << "    tag-" << tag << " error: " << loaded.error() << '\n';
        continue;
      }
      check(loaded->sprites.empty() && loaded->palette.size() == (tag == 1 ? 256 : 0),
            "single-tag complete counts");
      for (std::size_t i = 0; i < loaded->palette.size(); ++i)
        for (std::size_t j = 0; j < 3; ++j)
          check(std::bit_cast<std::uint64_t>(loaded->palette[i][j]) ==
                    0x4030000000000000ULL + i * 3 + j, "tag-1 ordered triple bits");
      std::cout << "    tag-" << tag << " loaded, " << loaded->palette.size() << " triples\n";
    }
    auto mixed = webo::loadSpriteSheet(*lib, "SPRITES\\MIXED.SPR");
    check(mixed.has_value(), "mixed archive resource loads");
    if (mixed) {
      checkClips(*mixed);
      checkTriples(*mixed);
    }
    for (auto name : {"SPRITES\\TRUNCATED.SPR", "SPRITES\\TRAILING.SPR"}) {
      auto result = webo::loadSpriteSheet(*lib, name);
      check(!result && result.error().starts_with(std::string(name) + ": ") &&
                result.error().size() > std::string_view(name).size() + 2,
            std::format("{} parse error is asset-qualified", name));
    }
  } catch (const std::invalid_argument& error) {
    check(false, std::format("loadSpriteSheet leaked invalid_argument: {}", error.what()));
  }
  std::cout << "    scratch: " << scratch.string() << '\n';
}

constexpr std::array kStates = {
    webo::sim::State::Faller,   webo::sim::State::Walker,   webo::sim::State::Climber,
    webo::sim::State::Hoister,  webo::sim::State::Floater,  webo::sim::State::Blocker,
    webo::sim::State::Builder,  webo::sim::State::Shrugger, webo::sim::State::Basher,
    webo::sim::State::Miner,    webo::sim::State::Digger,   webo::sim::State::Exploder,
    webo::sim::State::Splatter, webo::sim::State::Drown,    webo::sim::State::Bounce,
    webo::sim::State::Teleporting, webo::sim::State::Exited, webo::sim::State::Dead,
};

void testNames() {
  std::cout << "state -> sprite names\n";

  check(webo::gfx::spriteNameForState(webo::sim::State::Exited).empty(), "Exited has no sprite");
  check(webo::gfx::spriteNameForState(webo::sim::State::Dead).empty(), "Dead has no sprite");
  check(webo::gfx::spriteNameForState(webo::sim::State::Teleporting).empty(),
        "Teleporting has no lemming sprite");
  check(webo::gfx::spriteNameForState(webo::sim::State::Walker) == "walk", "Walker -> walk");
  check(webo::gfx::spriteNameForState(webo::sim::State::Hoister) == "flip", "Hoister -> flip");
  check(webo::gfx::spriteNameForState(webo::sim::State::Drown) == "drown", "Drown -> drown");
  check(webo::gfx::spriteNameForState(webo::sim::State::Bounce) == "fall", "Bounce -> fall");

  int named = 0;
  for (auto s : kStates)
    if (!webo::gfx::spriteNameForState(s).empty())
      ++named;
  check(named == 15, std::format("15 of the 18 states are drawn ({} are)", named));
}

void testRetail(const fs::path& gameDir) {
  std::cout << "state -> sprite names (retail)\n";

  auto lib = webo::AssetLibrary::open(gameDir / "lemmings.box", gameDir / "patch.box");
  if (!lib) {
    std::cerr << "  FAIL  could not open the installation: " << lib.error() << "\n";
    ++g_failures;
    return;
  }
  const auto names = webo::spriteSheetNames(*lib);
  std::size_t loadedCount = 0;
  for (const auto& name : names) {
    auto resource = webo::loadSpriteSheet(*lib, name);
    check(resource.has_value(), std::format("retail SPR {}", name));
    if (resource) ++loadedCount;
    else std::cerr << "    " << resource.error() << '\n';
  }
  check(!names.empty(), "retail SPR corpus is nonempty");
  std::cout << std::format("    retail SPR corpus: {} / {} entries loaded\n", loadedCount, names.size());
  auto sheet = webo::loadSpriteSheet(*lib, "SPRITES\\LEMMINGSLO.SPR");
  if (!sheet) {
    std::cerr << "  FAIL  LEMMINGSLO.SPR: " << sheet.error() << "\n";
    ++g_failures;
    return;
  }

  int matched = 0;
  for (auto s : kStates) {
    const auto name = webo::gfx::spriteNameForState(s);
    if (name.empty())
      continue;
    const webo::Sprite* sprite = sheet->find(name);
    check(sprite != nullptr, std::format("sprite '{}' exists in LEMMINGSLO.SPR", name));
    if (sprite) {
      ++matched;
      check(!sprite->animations.empty() && !sprite->animations.front().frames.empty(),
            std::format("sprite '{}' has at least one frame", name));
    }
  }
  std::cout << std::format("    {} states mapped onto {} sprites\n", matched, sheet->sprites.size());
}

}  // namespace

int main() {
  testNames();
  testSerializedProjection();
  testArchiveLoading();

  if (const char* dir = std::getenv("WEBOLUTION_GAME_DIR")) {
    if (fs::exists(fs::path(dir) / "lemmings.box"))
      testRetail(dir);
    else
      std::cout << "state -> sprite names (retail): skipped, no lemmings.box in " << dir << "\n";
  } else {
    std::cout << "state -> sprite names (retail): skipped, WEBOLUTION_GAME_DIR is not set\n";
  }

  std::cout << std::format("{} checks, {} failures\n", g_checks, g_failures);
  return g_failures == 0 ? 0 : 1;
}
