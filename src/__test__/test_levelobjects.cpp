// Tests for placed-model reads (entrance/exit + scenery/hazards) and skills.

#include "assets/Box.h"
#include "assets/Cylinder.h"
#include "assets/LevelIndex.h"
#include "assets/LevelObjects.h"
#include "assets/SurfaceMesh.h"

#include <array>
#include <bit>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <expected>
#include <filesystem>
#include <format>
#include <iostream>
#include <map>
#include <span>
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

void testWorldToMask() {
  std::cout << "worldToMaskPixel\n";
  // Forward axis (0, y, r) unrolls to ang 0; y=96 is the top of a height-96
  // tube, which maps to mask row 0 at pixelsPerUnit=2.
  const auto [x0, y0] = webo::worldToMaskPixel({0, 96, 60}, 256, 192, 96, 2);
  check(x0 == 0 && y0 == 0, "top of forward axis -> (0,0)");
  const auto [x1, y1] = webo::worldToMaskPixel({0, 0, 60}, 256, 192, 96, 2);
  check(x1 == 0 && y1 == 191, "bottom of forward axis clamps to last row");
  // -X is a quarter turn -> x = width/4.
  const auto [x2, y2] = webo::worldToMaskPixel({-60, 48, 0}, 256, 192, 96, 2);
  check(x2 == 64 && y2 == 96, "-X at mid height -> (64,96)");
}

void testRetail(const fs::path& dir) {
  std::cout << "retail placements\n";
  auto lib = webo::AssetLibrary::open(dir / "lemmings.box", dir / "patch.box");
  if (!lib) {
    check(false, lib.error());
    return;
  }

  std::size_t ok = 0, total = 0;
  for (const auto& name : webo::levelFileNames(*lib)) {
    auto blob = lib->read(name);
    if (!blob)
      continue;
    ++total;
    auto objs = webo::findLevelObjects(*blob);
    if (!objs) {
      std::cerr << std::format("  FAIL  {}: {}\n", name, objs.error());
      ++g_failures;
      continue;
    }
    check(!objs->entrances.empty(),
          std::format("{}: at least one entrance (got {})", name, objs->entrances.size()));
    check(!objs->exits.empty(),
          std::format("{}: at least one exit (got {})", name, objs->exits.size()));
    if (objs->entrances.empty() || objs->exits.empty())
      continue;

    bool entOk = true, exitOk = true;
    for (const auto& e : objs->entrances) {
      const double r = std::hypot(e.position.x, e.position.z);
      if (!(r > 45 && r < 55))
        entOk = false;
    }
    for (const auto& e : objs->exits) {
      const double r = std::hypot(e.position.x, e.position.z);
      if (!(r > 55 && r < 75))
        exitOk = false;
    }
    check(entOk, std::format("{}: entrances on inner tube", name));
    check(exitOk, std::format("{}: exits near outer tube", name));
    ++ok;
  }
  check(total == 102, std::format("102 retail levels (got {})", total));
  check(ok == total, std::format("every level yielded both placements ({}/{})", ok, total));

  // Spot-check MM_CLIMB_002 against the hand-read values.
  for (const auto& name : webo::levelFileNames(*lib)) {
    if (name.find("MM_CLIMB_002") == std::string::npos)
      continue;
    auto blob = lib->read(name);
    auto objs = webo::findLevelObjects(*blob);
    check(objs.has_value(), "MM_CLIMB_002 parses");
    if (!objs)
      break;
    const auto& e = objs->entrances[0].position;
    const auto& x = objs->exits[0].position;
    check(std::abs(e.x - (-35.54688)) < 0.01 && std::abs(e.y - 44.0) < 0.01 &&
              std::abs(e.z - 35.54687) < 0.01,
          "MM_CLIMB_002 entrance translation");
    check(std::abs(x.x - 49.062) < 0.01 && std::abs(x.y - 40.0) < 0.01 &&
              std::abs(x.z - 42.35388) < 0.01,
          "MM_CLIMB_002 exit translation");

    auto mesh = webo::findCylinderMesh(*blob);
    auto index = webo::indexLevel(*blob);
    auto faces = webo::buildSurfaceMesh(*mesh, index->grid0, index->grid1);
    const auto mask = webo::rasteriseSurface(*faces, *mesh, 2);
    const auto [ex, ey] =
        webo::worldToMaskPixel(e, mask.width, mask.height, mesh->height(), 2);
    const auto [xx, xy] =
        webo::worldToMaskPixel(x, mask.width, mask.height, mesh->height(), 2);
    check(ex >= 0 && ex < mask.width && ey >= 0 && ey < mask.height,
          "entrance lands on the mask");
    check(xx >= 0 && xx < mask.width && xy >= 0 && xy < mask.height, "exit lands on the mask");
    check(ex != xx || ey != xy, "entrance and exit are distinct pixels");
    break;
  }
}

void testRetailPlaced(const fs::path& dir) {
  std::cout << "retail placed models\n";
  auto lib = webo::AssetLibrary::open(dir / "lemmings.box", dir / "patch.box");
  if (!lib) {
    check(false, lib.error());
    return;
  }

  std::map<std::string, std::size_t> counts;
  std::size_t levelsWithPlaced = 0;
  std::size_t totalPlaced = 0;
  for (const auto& name : webo::levelFileNames(*lib)) {
    auto blob = lib->read(name);
    if (!blob)
      continue;
    auto objs = webo::findLevelObjects(*blob);
    check(objs.has_value(), std::format("{} placed parse", name));
    if (!objs)
      continue;
    if (!objs->placed.empty())
      ++levelsWithPlaced;
    for (const auto& p : objs->placed) {
      ++totalPlaced;
      ++counts[p.name];
      const double r = std::hypot(p.pose.position.x, p.pose.position.z);
      check(r > 20 && r < 100, std::format("{} {} on tube", name, p.name));
      check(p.kind == webo::classifyPlacedModel(p.name),
            std::format("{} kind matches classifier", p.name));
      check(p.kind != webo::PlacedKind::Other,
            std::format("{}: expected a known PlacedKind", p.name));
    }
  }

  // Corpus totals from a full Object 09 + pad0 + tube scan of all 102 levels.
  check(totalPlaced == 417, std::format("417 non-entrance/exit placements (got {})", totalPlaced));
  check(levelsWithPlaced == 73, std::format("73 levels carry placed models (got {})", levelsWithPlaced));
  check(counts["lever.a3d"] == 130, "130 levers");
  check(counts["teleport.a3d"] == 54, "54 teleports");
  check(counts["anti_grav.a3d"] == 50, "50 anti-grav");
  check(counts["pressure_pad.a3d"] == 45, "45 pressure pads");
  check(counts["pipe_drainage.a3d"] == 37, "37 pipe drains");
  check(counts["pipe_supply.a3d"] == 34, "34 pipe supplies");
  check(counts["deathroller.a3d"] == 22, "22 deathrollers");
  check(counts["speedup_two_way.a3d"] == 16, "16 speedups");
  check(counts["Crab.a3d"] == 14, "14 crabs");
  check(counts["PlaceHolder.a3d"] == 7, "7 placeholders");
  check(counts["robin.a3d"] == 5, "5 robins");
  check(counts["Scorp.a3d"] == 1, "1 scorpion");
  check(counts["buzzsaw.a3d"] == 1, "1 buzzsaw");
  check(counts["light_green.a3d"] == 1, "1 light_green");
  check(!counts.contains("fish.a3d"), "fish.a3d is a different Object 09 subtype");
  check(!counts.contains("teleport_fx.a3d"), "teleport_fx is identity-at-origin FX");

  check(webo::classifyPlacedModel("anti_grav.a3d") == webo::PlacedKind::AntiGrav, "classify anti_grav");
  check(webo::classifyPlacedModel("deathroller.a3d") == webo::PlacedKind::Deathroller,
        "classify deathroller");
  check(webo::classifyPlacedModel("Crab.a3d") == webo::PlacedKind::Creature, "classify crab");
  check(webo::classifyPlacedModel("light_green.a3d") == webo::PlacedKind::Scenery,
        "classify light");

  // Spot-check a teleporter tutorial and a deathroller level.
  for (const auto& name : webo::levelFileNames(*lib)) {
    if (name.find("MM_TELE_003") == std::string::npos)
      continue;
    auto blob = lib->read(name);
    auto objs = webo::findLevelObjects(*blob);
    check(objs.has_value(), "MM_TELE_003 parses");
    if (!objs)
      break;
    std::size_t teles = 0;
    for (const auto& p : objs->placed) {
      if (p.name == "teleport.a3d") {
        ++teles;
        check(p.kind == webo::PlacedKind::Teleport, "teleport kind");
      }
    }
    check(teles >= 2, std::format("MM_TELE_003 has teleports (got {})", teles));
    break;
  }
}

void testRetailSkills(const fs::path& dir) {
  std::cout << "retail skills\n";
  auto lib = webo::AssetLibrary::open(dir / "lemmings.box", dir / "patch.box");
  if (!lib) {
    check(false, lib.error());
    return;
  }

  std::size_t ok = 0, total = 0;
  for (const auto& name : webo::levelFileNames(*lib)) {
    auto blob = lib->read(name);
    if (!blob)
      continue;
    ++total;
    auto skills = webo::findLevelSkills(*blob);
    if (!skills) {
      std::cerr << std::format("  FAIL  {}: {}\n", name, skills.error());
      ++g_failures;
      continue;
    }
    std::uint32_t sum = 0;
    for (auto n : skills->counts)
      sum += n;
    check(sum > 0, std::format("{}: at least one skill", name));
    ++ok;
  }
  check(total == 102, std::format("102 retail levels for skills (got {})", total));
  check(ok == total, std::format("every level yielded skill counts ({}/{})", ok, total));

  struct Spot {
    const char* id;
    std::array<std::uint32_t, 8> want;
  };
  // Tutorial singles and the four-skill seafood level.
  const Spot spots[] = {
      {"MM_CLIMB_002", {10, 0, 0, 0, 0, 0, 0, 0}},
      {"MM_FLOAT", {0, 10, 0, 0, 0, 0, 0, 0}},
      {"MM_BOMB", {0, 0, 20, 0, 0, 0, 0, 0}},
      {"MM_BLOCK_001_A", {0, 0, 0, 10, 0, 0, 0, 0}},
      {"MM_BUILD", {0, 0, 0, 0, 10, 0, 0, 0}},
      {"MM_BASH_002", {0, 0, 0, 0, 0, 10, 0, 0}},
      {"MM_DIG_000", {0, 0, 0, 0, 0, 0, 0, 10}},
      {"MM_E4_4_SKILLS", {0, 0, 20, 20, 20, 20, 0, 0}},
  };
  for (const auto& spot : spots) {
    bool found = false;
    for (const auto& name : webo::levelFileNames(*lib)) {
      if (name.find(spot.id) == std::string::npos)
        continue;
      found = true;
      auto blob = lib->read(name);
      auto skills = webo::findLevelSkills(*blob);
      check(skills.has_value(), std::format("{} parses skills", spot.id));
      if (!skills)
        break;
      check(skills->counts == spot.want, std::format("{} skill vector", spot.id));
      break;
    }
    check(found, std::format("{} present in archive", spot.id));
  }
}

// ---------------------------------------------------------------------------
// native-009: hatch records through the recovered structured level reader.
//
// Read order of a serialized compiled level, with the original readers:
//   prefix          0x004432C1
//   world geometry  0x00439FD3
//   paired objects  0x00437D48
//   entity tables   0x00447960
//   gadget records  0x00448650 … 0x0044D429
// The hatch list is the third gadget call, 0x004487F0, whose records are read
// by 0x00425E50: the shared spatial base 0x0043130E (two triples into
// hatch+0x48 and +0x60, then two *counted* triple arrays) followed by the
// hatch's own type/count/x/y and rate words.
//
// These buffers are written out here from explicit little-endian primitives
// with empty collections. They are a fixture for that read layout -- not a
// claim about the original in-memory hatch object, its allocation, its
// resource resolution or any timing -- and nothing is round-tripped through a
// production writer.

struct Blob {
  std::vector<std::uint8_t> bytes;

  void u8(std::uint8_t v) { bytes.push_back(v); }
  void le(std::uint64_t v, unsigned width) {
    for (unsigned i = 0; i < width; ++i)
      u8(static_cast<std::uint8_t>(v >> (8 * i)));
  }
  void u16(std::uint16_t v) { le(v, 2); }
  void u32(std::uint32_t v) { le(v, 4); }
  void u64(std::uint64_t bits) { le(bits, 8); }
  void text(std::string_view s) {
    u32(static_cast<std::uint32_t>(s.size()));
    bytes.insert(bytes.end(), s.begin(), s.end());
  }
  void u32s(int count) {
    for (int i = 0; i < count; ++i) u32(0);
  }
  void u64s(int count) {
    for (int i = 0; i < count; ++i) u64(0);
  }
};

std::span<const std::byte> view(const std::vector<std::uint8_t>& v) {
  return {reinterpret_cast<const std::byte*>(v.data()), v.size()};
}

// Binary64 bit patterns, written as such. Neither the fixture nor the
// production projection converts or scales them.
constexpr std::uint64_t kB0 = 0x0000000000000000ULL;   // 0.0
constexpr std::uint64_t kB1 = 0x3FF0000000000000ULL;   // 1.0
constexpr std::uint64_t kB10 = 0x4024000000000000ULL;  // 10.0
constexpr std::uint64_t kB18 = 0x4032000000000000ULL;  // 18.0
constexpr std::uint64_t kB20 = 0x4034000000000000ULL;  // 20.0
constexpr std::uint64_t kB25 = 0x4039000000000000ULL;  // 25.0
constexpr std::uint64_t kB30 = 0x403E000000000000ULL;  // 30.0
constexpr std::uint64_t kB40 = 0x4044000000000000ULL;  // 40.0
constexpr std::uint64_t kB41 = 0x4044800000000000ULL;  // 41.0
constexpr std::uint64_t kB44 = 0x4046000000000000ULL;  // 44.0
constexpr std::uint64_t kB47 = 0x4047800000000000ULL;  // 47.0
constexpr std::uint64_t kB50 = 0x4049000000000000ULL;  // 50.0

struct SynthHatch {
  std::uint64_t baseYawBits = 0;
  std::string model;
  std::array<std::uint64_t, 6> boxBits{};  // hatch+0x48 xyz, then +0x60 xyz
  std::uint32_t triples0 = 0;              // records in the first counted array
  std::uint32_t type = 0;                  // hatch+0xB0
  std::uint16_t count = 0;                 // hatch+0x98
  std::uint16_t x = 0;                     // hatch+0x9A
  std::uint16_t y = 0;                     // hatch+0x9C
  std::uint32_t rate = 0;                  // hatch+0xA0
  std::uint32_t rateMin = 0;               // hatch+0xA4
  std::uint32_t rateMax = 0;               // hatch+0xA8
};

// Model read variant zero (0x00465924 tag 0 -> 0x00450F57), carrying the
// shared serialized object 0x004749BF with one class-zero child.
void levelModel(Blob& out, std::string_view name) {
  out.u32(0);
  out.text(name);
  out.u64s(3);
  out.text("Object 09");
  out.u32(0);  // class tag zero
  out.u64s(12);
  out.text("node");
  out.u64s(2);
  out.u32(0);  // no nested children
  out.u8(0);   // first flag
  out.u8(0);   // no auxiliary list
  out.u8(0);   // no final name
}

void hatchRecord(Blob& out, const SynthHatch& h) {
  // 0x00423291 actor base: five doubles, two flags, the stream id.
  out.u64(h.baseYawBits);
  out.u64s(4);
  out.u8(0);
  out.u8(0);
  out.u16(0);
  // 0x00432283: flag +0x40, model-present byte, flag +0x41, optional model.
  out.u8(0);
  out.u8(1);
  out.u8(0);
  levelModel(out, h.model);
  // 0x0043130E: the two triples, then two counted triple arrays.
  for (const auto bits : h.boxBits) out.u64(bits);
  out.u32(h.triples0);
  for (std::uint32_t i = 0; i < h.triples0; ++i) {
    out.u64(0x3FF8000000000000ULL);  // 1.5; an array entry, meaning unknown
    out.u64(kB0);
    out.u64(kB0);
  }
  out.u32(0);  // second counted array, empty
  // 0x00425E50's own fields.
  out.u32(h.type);
  out.u16(h.count);
  out.u16(h.x);
  out.u16(h.y);
  out.u32(h.rate);
  out.u32(h.rateMin);
  out.u32(h.rateMax);
  out.u8(0);   // +0xAC
  out.u16(0);  // +0xB4
  out.u8(0);   // +0xB6
  out.u8(0);   // +0xAD
}

// One Entrance.a3d world placement in the layout poseAfterA3d matches, so
// findLevelObjects accepts the fixture at all. Unrelated to the hatch list.
void entrancePlacement(Blob& out) {
  out.text("Entrance.a3d");
  out.text("Object 09");
  out.u32(0);  // zero pad: the world-placement subtype
  out.u64(kB1);
  out.u64(kB0);
  out.u64(kB0);  // cos / sin pair -> yaw 0
  out.u64s(6);
  out.u64(kB0);
  out.u64(kB44);
  out.u64(kB50);  // translation, r = 50 on the tube
}

std::vector<std::uint8_t> syntheticLevel(const std::vector<SynthHatch>& hatches,
                                         std::uint32_t selector = 0) {
  Blob out;

  // Prefix, 0x004432C1: marker, six fields, four flags, five name tables with
  // no top-level object, then the bounded string.
  out.text("Level 23");
  out.u32(3);
  out.u32s(5);
  for (int i = 0; i < 4; ++i) out.u8(0);
  out.u32s(3);
  out.u32(0);  // no prefix object
  out.u32s(2);
  out.text("prefix");

  // World geometry, 0x00439FD3.
  out.u32(112);
  out.u64(kB0);
  out.u32(0);  // triple blocks
  out.u32(0);  // double blocks
  out.u32(0);  // class-one leading value
  out.u32s(4); // triples, triples, named references, polygons
  out.u64s(12);
  out.text("world");
  out.u64s(2);
  out.u32(0);   // no nested class-one
  out.u32(96);  // following value
  out.u64(kB0);
  out.u32s(4);  // integer, triple, integer and byte arrays
  {
    const std::size_t fixedAt = out.bytes.size();
    entrancePlacement(out);
    while (out.bytes.size() - fixedAt < 6144) out.u8(0x6D);
  }
  out.text("in");
  out.text("out");
  out.u32s(32);  // tail lists

  // Paired objects, 0x00437D48 -> 0x004749BF, both objects present as in the
  // installed data.
  out.text("Object 09");
  out.u32(0);
  out.u64s(12);
  out.text("first");
  out.u64s(2);
  out.u32(0);  // no children
  out.u8(1);   // first flag
  out.u8(1);   // auxiliary list present
  out.u32(0);  // and empty
  out.u8(1);   // second object present
  out.text("Object 09");
  out.u32(0);
  out.u64s(12);
  out.text("second");
  out.u64s(2);
  out.u32(0);
  out.u8(0);
  out.u8(1);
  out.u32(0);

  // Entity tables, 0x00447960.
  out.u32(0);   // registry size
  out.u32s(4);  // groups, name pairs, both actor lists

  // Gadget records. The third list is the hatches.
  out.u32(0);  // 0x00448650
  out.u32(0);  // 0x00448710
  out.u32(static_cast<std::uint32_t>(hatches.size()));
  for (const auto& h : hatches) hatchRecord(out, h);
  out.u32(selector);   // value_5568
  out.u32s(9);  // the remaining entity lists, including tagged and named
  out.u32s(11); // 0x0045B86D
  out.u32(0);   // 0x00447880 integer array
  for (int i = 0; i < 5; ++i) out.u8(0);
  out.u16(0);
  out.u32(0);
  out.u32(0);  // 0x0044D429 final records
  return out.bytes;
}

// native-013: reuse the complete empty Level 23 sections above, replacing only
// the empty tail from 0x0045B86D onward with explicit ordered little-endian
// fields. No production writer or parser supplies the stock expectations.
struct SynthSkills {
  std::vector<std::uint8_t> bytes;
  std::size_t panelAt = 0;
  std::size_t integersAt = 0;
  std::size_t finalAt = 0;
};

SynthSkills syntheticSkillLevel(bool laterPair) {
  Blob out{syntheticLevel({})};
  constexpr std::size_t kEmptyTailBytes = 11 * 4 + 4 + 5 + 2 + 4 + 4;
  out.bytes.resize(out.bytes.size() - kEmptyTailBytes);
  SynthSkills fixture;
  fixture.panelAt = out.bytes.size();
  out.u32(0);    // climber
  out.u32(200);  // floater
  out.u32(2);    // bomber
  out.u32(17);   // blocker
  out.u32(33);   // builder
  out.u32(41);   // basher
  out.u32(59);   // miner
  out.u32(83);   // digger
  // The other three serialized words are deliberately distinct and receive
  // no portable skill meaning. In particular, no 1,8 signature is required.
  out.u32(0x12345678);
  out.u32(0x89ABCDEF);
  out.u32(0xFEDCBA98);
  fixture.integersAt = out.bytes.size();
  out.u32(3);  // following counted integer array, 0x00447880
  out.u32(301);
  out.u32(0xFFFFFFFF);
  out.u32(403);
  for (int i = 0; i < 5; ++i) out.u8(0);
  out.u16(0);
  out.u32(0);
  fixture.finalAt = out.bytes.size();
  out.u32(4);  // counted three-integer records, 0x0044D429
  out.u32(101); out.u32(102); out.u32(103);
  out.u32(104); out.u32(105); out.u32(106);
  out.u32(107); out.u32(108); out.u32(109);
  out.u32(110); out.u32(laterPair ? 1 : 111); out.u32(laterPair ? 8 : 112);
  fixture.bytes = std::move(out.bytes);
  return fixture;
}

void legacySkillTrailer(Blob& out) {
  out.u32(13); out.u32(23); out.u32(31); out.u32(41);
  out.u32(53); out.u32(61); out.u32(71); out.u32(83);
  out.u32(91);  // unexplained word, retained only for the old scan's spacing
  out.u32(1); out.u32(8);
}

void testStructuredSkills() {
  std::cout << "structured skill stocks\n";
  const std::array<std::uint32_t, 8> expected = {0, 200, 2, 17, 33, 41, 59, 83};
  for (bool laterPair : {false, true}) {
    auto fixture = syntheticSkillLevel(laterPair);
    const auto original = fixture.bytes;
    auto skills = webo::findLevelSkills(view(fixture.bytes));
    check(skills.has_value(), std::format("complete Level 23 skills, later pair {}",
                                         laterPair));
    if (skills) {
      check(skills->counts == expected, "all eight ordered stocks, including 0 and 200");
      check(skills->skillFallbackReason.empty(), "structured success has no diagnostic");
    }
    check(fixture.bytes == original, "structured skill loading leaves input unchanged");
    // A different usable legacy trailer after a complete level must not
    // override the decoded panel words; EOF validation is intentionally off.
    Blob extended{fixture.bytes};
    legacySkillTrailer(extended);
    const auto extendedOriginal = extended.bytes;
    auto trailing = webo::findLevelSkills(view(extended.bytes));
    check(trailing.has_value(), "complete level accepts trailing input");
    if (trailing) {
      check(trailing->counts == expected, "trailing legacy words do not replace decoded stocks");
      check(trailing->skillFallbackReason.empty(), "trailing bytes do not cause fallback");
    }
    check(extended.bytes == extendedOriginal, "trailing-input loading leaves input unchanged");
  }
}

void testSkillFallbackAndErrors() {
  std::cout << "skill fallback and expected errors\n";
  Blob legacy;
  legacySkillTrailer(legacy);
  const auto original = legacy.bytes;
  auto fallback = webo::findLevelSkills(view(legacy.bytes));
  check(fallback.has_value(), "short non-level input retains legacy success");
  if (fallback) {
    const std::array<std::uint32_t, 8> expected = {13, 23, 31, 41, 53, 61, 71, 83};
    check(fallback->counts == expected, "all eight legacy counts retained");
    check(fallback->skillFallbackReason == "prefix at byte 44: truncated level flags",
          "fallback retains the recovered reader's decorated rejection");
  }
  check(legacy.bytes == original, "legacy fallback leaves input unchanged");

  auto empty = webo::findLevelSkills({});
  check(!empty, "both readers reject empty input through expected");
  if (!empty) {
    check(empty.error().find("prefix at byte 0: invalid level string read") !=
              std::string::npos &&
              empty.error().find("level too short for skill trailer") != std::string::npos,
          "combined failure retains both exact causes");
  }

  auto fixture = syntheticSkillLevel(false);
  // Truncation in each part of the complete tail must not escape as an
  // invalid_argument. No incidental marker is available to salvage these.
  const std::array<std::size_t, 5> ends = {
      2, fixture.panelAt + 7 * 4, fixture.integersAt + 4,
      fixture.finalAt + 4, fixture.bytes.size() - 1};
  for (auto end : ends) {
    auto truncated = webo::findLevelSkills(view(fixture.bytes).first(end));
    check(!truncated, std::format("truncated complete level at {} returns expected error", end));
    if (!truncated) {
      check(truncated.error().find(end == 2 ? "prefix at byte" : "gadget records at byte") !=
                std::string::npos &&
                truncated.error().find(end == 2 ? "level too short for skill trailer" :
                                                                "skill trailer marker not found") !=
                    std::string::npos,
            "truncated failure retains decorated section/cursor and legacy cause");
    }
  }

  auto malformed = fixture.bytes;
  for (std::size_t i = 0; i < 4; ++i)
    malformed[fixture.integersAt + i] = 0xFF;
  auto badCount = webo::findLevelSkills(view(malformed));
  check(!badCount, "malformed following-array count returns expected error");
  if (!badCount) {
    check(badCount.error().find("gadget records at byte ") != std::string::npos &&
              badCount.error().find("invalid gadget record count") != std::string::npos &&
              badCount.error().find("skill trailer marker not found") != std::string::npos,
          "malformed failure retains both recovered and legacy causes");
  }

  // Decoding succeeds, but portable adapter validation fails. The later
  // trailer is usable by the unchanged scanner and must not enable fallback.
  fixture.bytes[fixture.panelAt + 5 * 4] = 201;
  Blob invalid{fixture.bytes};
  legacySkillTrailer(invalid);
  auto rejected = webo::findLevelSkills(view(invalid.bytes));
  check(!rejected, "decoded 201 is rejected despite a usable legacy trailer");
  if (!rejected)
    check(rejected.error() == "implausible skill count 201 at slot 5",
          "decoded count failure is solely the adapter validation error");
}

// A hatch the locator cannot see: an exactly 8 x 5 box, but a populated
// counted triple array, so the eight bytes after the box are not the +0.0
// the byte pattern needs for its seventh double.
SynthHatch hatchWithTripleArray() {
  SynthHatch h;
  h.model = "entrance_synth_a";
  h.boxBits = {kB10, kB20, kB0, kB18, kB25, kB1};
  h.triples0 = 1;
  h.type = 3;
  h.count = 7;
  h.x = 41;
  h.y = 33;
  h.rate = 12;
  h.rateMin = 2;
  h.rateMax = 90;
  return h;
}

// A hatch the locator cannot see for the other reason: empty arrays, but an
// 11 x 7 box instead of the 8 x 5 the byte pattern insists on.
SynthHatch hatchWithOtherBox() {
  SynthHatch h;
  h.model = "entrance_synth_b";
  h.boxBits = {kB30, kB40, kB0, kB41, kB47, kB1};
  h.triples0 = 0;
  h.type = 5;
  h.count = 11;
  h.x = 77;
  h.y = 64;
  h.rate = 20;
  h.rateMin = 1;
  h.rateMax = 99;
  return h;
}

void checkProjected(const webo::HatchRecord& got, const SynthHatch& want,
                    std::string_view what) {
  check(got.count == want.count,
        std::format("{}: count +0x98 is {} (got {})", what, want.count, got.count));
  check(got.x == want.x, std::format("{}: x +0x9A is {} (got {})", what, want.x, got.x));
  check(got.y == want.y, std::format("{}: y +0x9C is {} (got {})", what, want.y, got.y));
  check(got.type == static_cast<int>(want.type),
        std::format("{}: type +0xB0 is {} (got {})", what, want.type, got.type));
  check(got.rate == static_cast<int>(want.rate),
        std::format("{}: rate +0xA0 is {} (got {})", what, want.rate, got.rate));
  check(got.rateMin == static_cast<int>(want.rateMin),
        std::format("{}: rateMin +0xA4 is {} (got {})", what, want.rateMin, got.rateMin));
  check(got.rateMax == static_cast<int>(want.rateMax),
        std::format("{}: rateMax +0xA8 is {} (got {})", what, want.rateMax, got.rateMax));
  const std::array<std::uint64_t, 6> bits{
      std::bit_cast<std::uint64_t>(got.boxMin.x), std::bit_cast<std::uint64_t>(got.boxMin.y),
      std::bit_cast<std::uint64_t>(got.boxMin.z), std::bit_cast<std::uint64_t>(got.boxMax.x),
      std::bit_cast<std::uint64_t>(got.boxMax.y), std::bit_cast<std::uint64_t>(got.boxMax.z)};
  check(bits == want.boxBits,
        std::format("{}: box +0x48/+0x60 bit patterns transferred unchanged "
                    "(got {:#018x} {:#018x} {:#018x} {:#018x} {:#018x} {:#018x})",
                    what, bits[0], bits[1], bits[2], bits[3], bits[4], bits[5]));
  check(got.model == want.model,
        std::format("{}: model name is \"{}\" (got \"{}\")", what, want.model, got.model));
  check(got.at == 0, std::format("{}: no field offset from the structured reader", what));
}

void testSyntheticStructuredHatches() {
  std::cout << "structured hatch records (synthetic Level 23)\n";
  const SynthHatch wantA = hatchWithTripleArray();
  const SynthHatch wantB = hatchWithOtherBox();
  const auto bytes = syntheticLevel({wantA, wantB});

  // Both records are deliberately invisible to the byte-pattern locator: this
  // is the before/after. Before the production change findLevelObjects saw
  // only findHatchRecords, so both were lost and loadRetailLevelReport would
  // reject such a level with "no hatch record".
  const auto located = webo::findHatchRecords(view(bytes));
  check(located.empty(),
        std::format("the locator cannot see either record (it found {})", located.size()));

  auto objs = webo::findLevelObjects(view(bytes));
  if (!objs) {
    check(false, std::format("synthetic Level 23 parses: {}", objs.error()));
    return;
  }
  check(objs->hatchFallbackReason.empty(),
        std::format("the structured reader accepted the fixture (reason \"{}\")",
                    objs->hatchFallbackReason));
  check(objs->hatches.size() == 2,
        std::format("two structured hatch records (got {})", objs->hatches.size()));
  if (objs->hatches.size() != 2)
    return;
  checkProjected(objs->hatches[0], wantA, "populated-triple-array hatch");
  checkProjected(objs->hatches[1], wantB, "non-8x5-box hatch");
}

void testStructuredFallback() {
  std::cout << "locator fallback when the structured reader rejects the input\n";
  // Not a compiled level: a declared string longer than the buffer. One
  // locator-visible record follows, then the entrance placement so
  // findLevelObjects still returns a value.
  Blob out;
  out.u32(254);  // 0x0047D940 string length, with far fewer bytes left
  out.u64(kB10);
  out.u64(kB20);
  out.u64(kB0);
  out.u64(kB18);
  out.u64(kB25);
  out.u64(kB1);
  out.u64(kB0);  // the locator's seventh double: two empty array counts
  out.u32(3);    // type
  out.u16(5);
  out.u16(40);
  out.u16(30);  // count, x, y
  out.u32(11);
  out.u32(1);
  out.u32(99);  // rate, rateMin, rateMax
  out.u8(0);
  entrancePlacement(out);
  for (int i = 0; i < 16; ++i) out.u8(0);  // slack past the transform run

  const auto located = webo::findHatchRecords(view(out.bytes));
  auto objs = webo::findLevelObjects(view(out.bytes));
  if (!objs) {
    check(false, std::format("fallback fixture parses: {}", objs.error()));
    return;
  }
  check(objs->hatchFallbackReason == "prefix at byte 4: truncated level string",
        std::format("the decoder's own message is recorded (got \"{}\")",
                    objs->hatchFallbackReason));
  check(!located.empty(),
        std::format("the locator sees its own record (found {})", located.size()));
  check(objs->hatches.size() == located.size(),
        std::format("fallback yields the locator's records ({} vs {})",
                    objs->hatches.size(), located.size()));
  if (objs->hatches.empty() || located.empty())
    return;
  bool same = objs->hatches.size() == located.size();
  for (std::size_t i = 0; same && i < located.size(); ++i) {
    const auto& a = objs->hatches[i];
    const auto& b = located[i];
    same = a.count == b.count && a.x == b.x && a.y == b.y && a.type == b.type &&
           a.rate == b.rate && a.rateMin == b.rateMin && a.rateMax == b.rateMax &&
           a.at == b.at && a.model == b.model;
  }
  check(same, "fallback records are findHatchRecords' own output, field for field");
  const auto& h = objs->hatches.front();
  check(h.count == 5 && h.x == 40 && h.y == 30 && h.type == 3 && h.rate == 11 &&
            h.rateMin == 1 && h.rateMax == 99,
        std::format("the located record carries the written words (got {}, {}, {}, {}, "
                    "{}, {}, {})",
                    h.count, h.x, h.y, h.type, h.rate, h.rateMin, h.rateMax));
  check(h.at != 0, "the locator still reports its file offset");
}

// Every locator-found record must also appear in the structured list with the
// same words. Structured-only records are reported, not asserted away: this
// sweep measures the installed corpus, it does not define it.
void testRetailStructuredSweep(const fs::path& dir) {
  std::cout << "retail structured vs located hatch records\n";
  auto lib = webo::AssetLibrary::open(dir / "lemmings.box", dir / "patch.box");
  if (!lib) {
    check(false, lib.error());
    return;
  }

  const auto started = std::chrono::steady_clock::now();
  double locatorSeconds = 0, structuredSeconds = 0;
  std::size_t levels = 0, locatedTotal = 0, structuredTotal = 0, rejected = 0;
  std::size_t structuredOnly = 0;
  for (const auto& name : webo::levelFileNames(*lib)) {
    auto blob = lib->read(name);
    if (!blob)
      continue;
    ++levels;

    const auto t0 = std::chrono::steady_clock::now();
    const auto located = webo::findHatchRecords(*blob);
    const auto t1 = std::chrono::steady_clock::now();
    std::vector<webo::HatchRecord> structured;
    std::string reason;
    try {
      structured = webo::readHatchRecords(*blob);
    } catch (const std::invalid_argument& error) {
      reason = error.what();
    }
    const auto t2 = std::chrono::steady_clock::now();
    locatorSeconds += std::chrono::duration<double>(t1 - t0).count();
    structuredSeconds += std::chrono::duration<double>(t2 - t1).count();

    if (!reason.empty()) {
      ++rejected;
      std::cerr << std::format("  FAIL  {}: structured reader rejected an installed "
                               "level: {}\n", name, reason);
      ++g_failures;
      continue;
    }
    locatedTotal += located.size();
    structuredTotal += structured.size();

    std::vector<bool> taken(structured.size(), false);
    for (const auto& l : located) {
      bool matched = false;
      for (std::size_t i = 0; i < structured.size() && !matched; ++i) {
        if (taken[i])
          continue;
        const auto& s = structured[i];
        if (s.count == l.count && s.x == l.x && s.y == l.y && s.type == l.type &&
            s.rate == l.rate && s.rateMin == l.rateMin && s.rateMax == l.rateMax) {
          taken[i] = true;
          matched = true;
        }
      }
      check(matched,
            std::format("{}: located record (count {}, x {}, y {}, type {}, rate "
                        "{}/{}/{}) is in the structured list",
                        name, l.count, l.x, l.y, l.type, l.rate, l.rateMin, l.rateMax));
    }
    for (std::size_t i = 0; i < structured.size(); ++i) {
      if (taken[i])
        continue;
      ++structuredOnly;
      const auto& s = structured[i];
      std::cout << std::format(
          "  structured-only  {} [{}]: count {}, x {}, y {}, type {}, rate {}/{}/{}, "
          "box ({:g}, {:g}, {:g})-({:g}, {:g}, {:g}), model \"{}\"\n",
          name, i, s.count, s.x, s.y, s.type, s.rate, s.rateMin, s.rateMax, s.boxMin.x,
          s.boxMin.y, s.boxMin.z, s.boxMax.x, s.boxMax.y, s.boxMax.z, s.model);
    }
  }

  const double wall = std::chrono::duration<double>(
                          std::chrono::steady_clock::now() - started).count();
  std::cout << std::format(
      "  {} levels: {} structured records, {} located records, {} structured-only, "
      "{} rejected\n", levels, structuredTotal, locatedTotal, structuredOnly, rejected);
  std::cout << std::format(
      "  sweep {:.3f} s wall: {:.3f} s structured parse, {:.3f} s locator scan\n", wall,
      structuredSeconds, locatorSeconds);
  check(levels == 102, std::format("102 retail levels swept (got {})", levels));
  check(rejected == 0,
        std::format("every installed level reads through the structured reader ({} "
                    "rejected)", rejected));
}

// camera-entry-001: selected-hatch camera metadata. The hatch's base yaw is the
// first double the base reader (0x00423291) stores at hatch+8; it is *not* one of
// the spawn words (+0x98/+0x9A/+0x9C). The selector is gadgets.value_5568.
constexpr std::uint64_t dbits(double v) { return std::bit_cast<std::uint64_t>(v); }

std::string sha256Hex(const std::vector<std::uint8_t>& data) {
  static constexpr std::uint32_t k[64] = {
      0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5, 0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5,
      0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3, 0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174,
      0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc, 0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
      0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7, 0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967,
      0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13, 0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
      0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3, 0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
      0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5, 0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3,
      0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208, 0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2};
  auto rotr = [](std::uint32_t x, int n) { return (x >> n) | (x << (32 - n)); };
  std::uint32_t h[8] = {0x6a09e667, 0xbb67ae85, 0x3c6ef372, 0xa54ff53a,
                        0x510e527f, 0x9b05688c, 0x1f83d9ab, 0x5be0cd19};
  std::vector<std::uint8_t> msg(data);
  const std::uint64_t bitLen = static_cast<std::uint64_t>(msg.size()) * 8;
  msg.push_back(0x80);
  while ((msg.size() % 64) != 56) msg.push_back(0);
  for (int i = 7; i >= 0; --i) msg.push_back(static_cast<std::uint8_t>((bitLen >> (i * 8)) & 0xFF));
  for (std::size_t off = 0; off < msg.size(); off += 64) {
    std::uint32_t w[64];
    for (std::size_t i = 0; i < 16; ++i)
      w[i] = (static_cast<std::uint32_t>(msg[off + i * 4]) << 24) |
             (static_cast<std::uint32_t>(msg[off + i * 4 + 1]) << 16) |
             (static_cast<std::uint32_t>(msg[off + i * 4 + 2]) << 8) | msg[off + i * 4 + 3];
    for (std::size_t i = 16; i < 64; ++i) {
      const std::uint32_t s0 = rotr(w[i - 15], 7) ^ rotr(w[i - 15], 18) ^ (w[i - 15] >> 3);
      const std::uint32_t s1 = rotr(w[i - 2], 17) ^ rotr(w[i - 2], 19) ^ (w[i - 2] >> 10);
      w[i] = w[i - 16] + s0 + w[i - 7] + s1;
    }
    std::uint32_t a = h[0], b = h[1], c = h[2], d = h[3], e = h[4], f = h[5], g = h[6], hh = h[7];
    for (std::size_t i = 0; i < 64; ++i) {
      const std::uint32_t t1 = hh + (rotr(e, 6) ^ rotr(e, 11) ^ rotr(e, 25)) + ((e & f) ^ (~e & g)) + k[i] + w[i];
      const std::uint32_t t2 = (rotr(a, 2) ^ rotr(a, 13) ^ rotr(a, 22)) + ((a & b) ^ (a & c) ^ (b & c));
      hh = g; g = f; f = e; e = d + t1; d = c; c = b; b = a; a = t1 + t2;
    }
    h[0] += a; h[1] += b; h[2] += c; h[3] += d; h[4] += e; h[5] += f; h[6] += g; h[7] += hh;
  }
  std::string out;
  for (const auto word : h) out += std::format("{:08x}", word);
  return out;
}

void checkYaw(const std::expected<webo::SelectedHatchCamera, std::string>& got,
              std::int32_t selector, std::size_t count, double yaw, std::string_view what) {
  check(got.has_value(), std::format("{}: accepted (error \"{}\")", what,
                                     got ? std::string() : got.error()));
  if (!got)
    return;
  check(got->selector == selector && got->hatchCount == count,
        std::format("{}: selector {} / count {} (got {} / {})", what, selector, count,
                    got->selector, got->hatchCount));
  check(dbits(got->yawUnits) == dbits(yaw),
        std::format("{}: yaw bit pattern is {:#018x} (got {:#018x})", what, dbits(yaw),
                    dbits(got->yawUnits)));
}

void checkRejected(const std::expected<webo::SelectedHatchCamera, std::string>& got,
                   std::string_view needle, std::string_view what) {
  check(!got.has_value(), std::format("{}: rejected", what));
  if (got)
    return;
  check(got.error().starts_with("selected-hatch camera: ") &&
            got.error().find(needle) != std::string::npos,
        std::format("{}: contextual error mentions \"{}\" (got \"{}\")", what, needle,
                    got.error()));
}

void testSelectedHatchCamera() {
  std::cout << "selected-hatch camera metadata (synthetic Level 23)\n";
  SynthHatch first = hatchWithTripleArray();   // spawn words 7, 41, 33
  SynthHatch second = hatchWithOtherBox();     // spawn words 11, 77, 64
  first.baseYawBits = dbits(17.5);
  second.baseYawBits = dbits(229.0);
  const auto read = [](const std::vector<std::uint8_t>& bytes) {
    return webo::readSelectedHatchCamera(view(bytes));
  };

  // A selector chooses the non-first hatch, whose base yaw shares nothing with
  // its spawn words or either hatch's model name; the first hatch carries a
  // populated counted triple array and a model between the records.
  const auto bytes = syntheticLevel({first, second}, 1);
  const auto before = bytes;
  const auto chosen = read(bytes);
  checkYaw(chosen, 1, 2, 229.0, "selector 1 chooses the second hatch");
  check(bytes == before, "reading leaves the caller's bytes unchanged");
  if (chosen) {
    for (const double word : {7.0, 41.0, 33.0, 11.0, 77.0, 64.0})
      check(chosen->yawUnits != word, "yaw is not a spawn word");
  }
  checkYaw(read(syntheticLevel({first, second}, 0)), 0, 2, 17.5, "selector 0 chooses the first hatch");

  // Nonnegative selectors beyond the list, including an empty list, are the
  // evidenced yaw128 default (0x00444FA2 / 0x00444FA9).
  checkYaw(read(syntheticLevel({first, second}, 2)), 2, 2, 128.0, "selector equal to count");
  checkYaw(read(syntheticLevel({first, second}, 0x7FFFFFFFu)), 0x7FFFFFFF, 2, 128.0,
           "selector INT32_MAX");
  checkYaw(read(syntheticLevel({}, 0)), 0, 0, 128.0, "empty list, selector 0");
  checkYaw(read(syntheticLevel({}, 9)), 9, 0, 128.0, "empty list, selector 9");

  // Negative selectors would index out of bounds in the original: portable rejection.
  checkRejected(read(syntheticLevel({first, second}, 0xFFFFFFFFu)), "negative selector -1",
                "selector -1");
  checkRejected(read(syntheticLevel({first, second}, 0x80000000u)), "negative selector",
                "selector INT32_MIN");

  // The source bits are preserved without arithmetic.
  second.baseYawBits = dbits(-300.5);
  checkYaw(read(syntheticLevel({first, second}, 1)), 1, 2, -300.5, "negative yaw kept verbatim");
  second.baseYawBits = dbits(1.0e300);
  checkYaw(read(syntheticLevel({first, second}, 1)), 1, 2, 1.0e300, "large yaw kept verbatim");

  // Nonfinite selected yaw is rejected; an unselected one is never examined.
  for (const std::uint64_t bits : {0x7FF8000000000000ULL, 0x7FF0000000000000ULL,
                                   0xFFF0000000000000ULL}) {
    SynthHatch bad = first;
    bad.baseYawBits = bits;
    checkRejected(read(syntheticLevel({bad, second}, 0)), "nonfinite", "nonfinite selected yaw");
    second.baseYawBits = dbits(229.0);
    checkYaw(read(syntheticLevel({bad, second}, 1)), 1, 2, 229.0, "nonfinite unselected yaw");
  }

  // Truncated or foreign input becomes a contextual expected error.
  second.baseYawBits = dbits(229.0);
  const auto full = syntheticLevel({first, second}, 1);
  for (const std::size_t cut : {std::size_t{0}, std::size_t{4}, full.size() / 2}) {
    std::vector<std::uint8_t> shortened(full.begin(), full.begin() + static_cast<std::ptrdiff_t>(cut));
    checkRejected(read(shortened), "", std::format("truncated to {} bytes", cut));
  }
  checkRejected(read({0x01, 0x02, 0x03}), "", "not a compiled level");

  // require_eof=false: bytes after the last section are accepted and ignored.
  auto trailing = full;
  trailing.insert(trailing.end(), 64, 0xA5);
  checkYaw(read(trailing), 1, 2, 229.0, "trailing input accepted");
}

void testRetailSelectedCamera(const fs::path& dir) {
  std::cout << "retail selected-hatch camera metadata\n";
  auto lib = webo::AssetLibrary::open(dir / "lemmings.box", dir / "patch.box");
  if (!lib) {
    check(false, lib.error());
    return;
  }
  const auto doubleAt = [](const std::vector<std::byte>& b, std::size_t at) {
    std::uint64_t bits = 0;
    std::memcpy(&bits, b.data() + at, 8);
    return std::bit_cast<double>(bits);
  };
  struct Expected { const char* id; double yaw; };
  for (const Expected& want : {Expected{"MM_CLIMB_002", 32.0}, Expected{"MM_BASH_002", 229.0}}) {
    auto blob = lib->read(std::string("COMPILED\\") + want.id + ".LVL");
    check(blob.has_value(), std::format("{}: installed level read", want.id));
    if (!blob)
      continue;
    const auto got = webo::readSelectedHatchCamera(*blob);
    checkYaw(got, 0, got ? got->hatchCount : 0, want.yaw,
             std::format("{} installed selector 0", want.id));
    check(got && got->hatchCount >= 1, std::format("{}: selected hatch is in the list", want.id));
    std::cout << std::format("  {}: {} bytes, selector {}, hatches {}, yaw {}\n", want.id,
                             blob->size(), got ? got->selector : -1, got ? got->hatchCount : 0,
                             got ? got->yawUnits : 0.0);
    if (std::string_view(want.id) == "MM_BASH_002") {
      // Planning-time measurements, reproduced here against the installed bytes.
      std::vector<std::uint8_t> raw(blob->size());
      std::memcpy(raw.data(), blob->data(), blob->size());
      check(blob->size() == 709367, "MM_BASH_002 is 709,367 bytes");
      check(sha256Hex(raw) == "65455693fa887f4474c66c2795526cb3b2bef2616449a5e1dc55debb9d7e5cd2",
            "MM_BASH_002 payload SHA-256");
      check(blob->size() > 0xA347F, "MM_BASH_002 holds the selector word");
      if (blob->size() > 0xA347F) {
        std::uint32_t selector = 0xFFFFFFFFu;
        std::memcpy(&selector, blob->data() + 0xA347B, 4);
        check(selector == 0, "MM_BASH_002 selector word at 0xA347B is 0");
        check(doubleAt(*blob, 0x9F473) == 229.0 && doubleAt(*blob, 0x9F473 + 8) == 75.0 &&
                  doubleAt(*blob, 0x9F473 + 16) == 0.0,
              "MM_BASH_002 hatch base triple at 0x9F473 is (229, 75, 0)");
      }
    }
  }

  // Corpus census, reported rather than asserted: it is the evidence for which
  // other levels already take the same path.
  std::size_t levels = 0, inRange = 0, defaulted = 0, rejected = 0, nonzero = 0;
  for (const auto& name : webo::levelFileNames(*lib)) {
    auto blob = lib->read(name);
    if (!blob)
      continue;
    ++levels;
    const auto got = webo::readSelectedHatchCamera(*blob);
    if (!got) {
      ++rejected;
      std::cout << std::format("  rejected {}: {}\n", name, got.error());
      continue;
    }
    if (got->selector != 0) ++nonzero;
    if (static_cast<std::size_t>(got->selector) < got->hatchCount) ++inRange; else ++defaulted;
  }
  std::cout << std::format("  {} levels: {} selected in range, {} default yaw128, {} nonzero "
                           "selectors, {} rejected\n", levels, inRange, defaulted, nonzero,
                           rejected);
  check(levels == 102, std::format("102 levels read for the census (got {})", levels));
}

}  // namespace

int main() {
  testWorldToMask();
  testSelectedHatchCamera();
  testSyntheticStructuredHatches();
  testStructuredFallback();
  testStructuredSkills();
  testSkillFallbackAndErrors();

  const char* dir = std::getenv("WEBOLUTION_GAME_DIR");
  if (!dir) {
    std::cout << "retail placements: SKIP (WEBOLUTION_GAME_DIR unset)\n";
    std::cout << "retail placed models: SKIP (WEBOLUTION_GAME_DIR unset)\n";
    std::cout << "retail skills: SKIP (WEBOLUTION_GAME_DIR unset)\n";
    std::cout << "retail structured vs located hatch records: SKIP "
                 "(WEBOLUTION_GAME_DIR unset)\n";
  } else {
    testRetail(dir);
    testRetailPlaced(dir);
    testRetailSkills(dir);
    testRetailStructuredSweep(dir);
    testRetailSelectedCamera(dir);
  }

  std::cout << std::format("{} checks, {} failures\n", g_checks, g_failures);
  return g_failures == 0 ? 0 : 1;
}
