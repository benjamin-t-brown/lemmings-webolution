// Match each pad's +0x98 word to the partner's base ID. MM_TELE_003 pairs
// 4<->6 and 5<->3. Literal partner indices follow locator order.
#include "assets/Box.h"
#include "assets/CollisionGrid.h"
#include "assets/LevelIndex.h"
#include "assets/LevelObjects.h"
#include "sim/RetailLevel.h"

#include <bit>
#include <cstdint>
#include <cstdlib>
#include <filesystem>
#include <format>
#include <iostream>
#include <span>
#include <string>
#include <string_view>
#include <vector>

namespace fs = std::filesystem;

namespace {
int g_failures = 0, g_checks = 0;
void check(bool c, const std::string& what) {
  ++g_checks;
  if (!c) {
    ++g_failures;
    std::cerr << "  FAIL  " << what << "\n";
  }
}

void testResolver() {
  std::cout << "resolveTeleportPartners (synthetic)\n";
  using webo::TeleportLink;
  std::string diag;
  // Non-adjacent ids, shuffled order.
  const std::vector<TeleportLink> a{{40, 7, 1}, {7, 40, 2}, {12, 99, 3}, {99, 12, 4}};
  auto p = webo::resolveTeleportPartners(a, diag);
  check(p == std::vector<int>({1, 0, 3, 2}) && diag.empty(), "non-adjacent ids pair by equality");
  // Odd count whose ids still resolve (one-directional link).
  diag.clear();
  const std::vector<TeleportLink> b{{1, 2, 0}, {2, 3, 0}, {3, 1, 0}};
  p = webo::resolveTeleportPartners(b, diag);
  check(p == std::vector<int>({1, 2, 0}) && diag.empty(), "odd count pairs by ids");
  // Unpaired id.
  diag.clear();
  const std::vector<TeleportLink> c{{1, 2, 0}, {2, 9, 0}};
  p = webo::resolveTeleportPartners(c, diag);
  check(p == std::vector<int>({1, -1}) && !diag.empty(), "unpaired id: no teleport, diagnostic");
  // Duplicate id: list[id] is overwritten by the later record (0x00423325).
  diag.clear();
  const std::vector<TeleportLink> d{{5, 6, 0}, {6, 5, 0}, {5, 6, 0}};
  p = webo::resolveTeleportPartners(d, diag);
  check(p == std::vector<int>({1, 2, 1}) && !diag.empty(),
        "duplicate id: the last record in stream order wins, with a diagnostic");
}

// Write a synthetic level in explicit little-endian order. Use teleport.a3d
// for locator placements and 0x004347EC records (teleport_fx.a3d x2) for IDs.
struct Blob {
  std::vector<std::uint8_t> bytes;
  void u8(std::uint8_t v) { bytes.push_back(v); }
  void le(std::uint64_t v, unsigned width) {
    for (unsigned i = 0; i < width; ++i) u8(static_cast<std::uint8_t>(v >> (8 * i)));
  }
  void u16(std::uint16_t v) { le(v, 2); }
  void u32(std::uint32_t v) { le(v, 4); }
  void u64(std::uint64_t v) { le(v, 8); }
  void text(std::string_view s) {
    u32(static_cast<std::uint32_t>(s.size()));
    bytes.insert(bytes.end(), s.begin(), s.end());
  }
  void u32s(int n) { for (int i = 0; i < n; ++i) u32(0); }
  void u64s(int n) { for (int i = 0; i < n; ++i) u64(0); }
};

std::span<const std::byte> view(const std::vector<std::uint8_t>& v) {
  return {reinterpret_cast<const std::byte*>(v.data()), v.size()};
}

std::uint64_t bitsOf(double d) { return std::bit_cast<std::uint64_t>(d); }
constexpr std::uint64_t kB0 = 0x0000000000000000ULL;
constexpr std::uint64_t kB1 = 0x3FF0000000000000ULL;
constexpr std::uint64_t kB50 = 0x4049000000000000ULL;

void levelModel(Blob& out, std::string_view name) {
  out.u32(0);
  out.text(name);
  out.u64s(3);
  out.text("Object 09");
  out.u32(0);
  out.u64s(12);
  out.text("node");
  out.u64s(2);
  out.u32(0);
  out.u8(0);
  out.u8(0);
  out.u8(0);
}

// A world placement in the layout poseAfterA3d matches: unit yaw, translation
// (0, ty, 50), i.e. r = 50 on the tube.
void placement(Blob& out, std::string_view name, double ty) {
  out.text(name);
  out.text("Object 09");
  out.u32(0);
  out.u64(kB1);
  out.u64(kB0);
  out.u64(kB0);
  out.u64s(6);
  out.u64(kB0);
  out.u64(bitsOf(ty));
  out.u64(kB50);
}

struct Pad {
  std::int16_t id;      // 0x00423291 stream id
  std::uint32_t link;   // object +0x98
  double y;             // base double [1]
};

void padRecord(Blob& out, const Pad& p) {
  out.u64(kB0);
  out.u64(bitsOf(p.y));
  out.u64s(3);
  out.u8(0);
  out.u8(0);
  out.u16(static_cast<std::uint16_t>(p.id));
  out.u8(0);   // 0x00432283 flag +0x40
  out.u8(0);   // no model on the base
  out.u8(0);   // flag +0x41
  out.u64s(6); // 0x0043130E: two triples
  out.u32(0);
  out.u32(0);  // two empty counted arrays
  levelModel(out, "teleport_fx.a3d");
  levelModel(out, "teleport_fx.a3d");
  out.u32(p.link);  // +0x98
  out.u64(kB0);     // +0xA0
  out.u64s(3);      // +0x1B0
  for (int i = 0; i < 4; ++i) out.text("n");
}

// `located` are the Y values of the teleport.a3d placements in file order;
// `pads` are the 0x004347EC records in stream order.
std::vector<std::uint8_t> teleLevel(const std::vector<Pad>& pads, const std::vector<double>& located) {
  Blob out;
  out.text("Level 23");
  out.u32(3);
  out.u32s(5);
  for (int i = 0; i < 4; ++i) out.u8(0);
  out.u32s(3);
  out.u32(0);
  out.u32s(2);
  out.text("prefix");
  out.u32(112);
  out.u64(kB0);
  out.u32(0);
  out.u32(0);
  out.u32(0);
  out.u32s(4);
  out.u64s(12);
  out.text("world");
  out.u64s(2);
  out.u32(0);
  out.u32(96);
  out.u64(kB0);
  out.u32s(4);
  {
    const std::size_t fixedAt = out.bytes.size();
    placement(out, "Entrance.a3d", 44.0);
    for (double y : located) placement(out, "teleport.a3d", y);
    while (out.bytes.size() - fixedAt < 6144) out.u8(0x6D);
  }
  out.text("in");
  out.text("out");
  out.u32s(32);
  out.text("Object 09");
  out.u32(0);
  out.u64s(12);
  out.text("first");
  out.u64s(2);
  out.u32(0);
  out.u8(1);
  out.u8(1);
  out.u32(0);
  out.u8(1);
  out.text("Object 09");
  out.u32(0);
  out.u64s(12);
  out.text("second");
  out.u64s(2);
  out.u32(0);
  out.u8(0);
  out.u8(1);
  out.u32(0);
  out.u32(0);
  out.u32s(4);
  out.u32(0);                              // 0x00448650
  out.u32(0);                              // 0x00448710
  out.u32(0);                              // hatches
  out.u32(0);                              // value_5568
  out.u32s(4);                             // 426c6b, spatial, 424187, 42acac
  out.u32(static_cast<std::uint32_t>(pads.size()));  // 0x00448CB0
  for (const auto& p : pads) padRecord(out, p);
  out.u32s(4);                             // 4342bf, 41e038, tagged, named
  out.u32s(11);
  out.u32(0);
  for (int i = 0; i < 5; ++i) out.u8(0);
  out.u16(0);
  out.u32(0);
  out.u32(0);
  return out.bytes;
}

std::vector<double> yOf(const std::vector<Pad>& pads) {
  std::vector<double> y;
  for (const auto& p : pads) y.push_back(p.y);
  return y;
}

std::size_t teleportPlacements(const webo::LevelObjects& o) {
  std::size_t n = 0;
  for (const auto& p : o.placed) n += p.kind == webo::PlacedKind::Teleport;
  return n;
}

void testSynthetic() {
  std::cout << "structured teleport links (synthetic compiled level)\n";
  // Use shuffled, non-adjacent ID pairs 40<->7 and 12<->99 to distinguish locator pairing.
  {
    const std::vector<Pad> pads{{40, 7, 30}, {7, 40, 40}, {12, 99, 50}, {99, 12, 60}};
    const auto bytes = teleLevel(pads, yOf(pads));
    const auto original = bytes;
    auto objs = webo::findLevelObjects(view(bytes));
    check(objs.has_value() && teleportPlacements(*objs) == 4, "four pads are located");
    if (objs) {
      check(objs->teleportLinks.size() == 4 && objs->teleportLinks[0].id == 40 &&
                objs->teleportLinks[0].partnerId == 7 && objs->teleportLinks[3].id == 99,
            "ids projected in stream order");
      check(objs->teleportPartner == std::vector<int>({1, 0, 3, 2}) && objs->teleportLinkDiagnostic.empty(),
            "non-adjacent ids pair by equality, not by position");
      check(objs->hatchFallbackReason.empty(), "synthetic level is accepted by the structured reader");
    }
    check(bytes == original, "findLevelObjects leaves the input bytes unchanged");

    // Trailing bytes after the complete level are accepted (require_eof=false).
    auto trailing = bytes;
    trailing.insert(trailing.end(), 64, 0xA5);
    const auto trailingOriginal = trailing;
    auto links = webo::readTeleportLinks(view(trailing));
    check(links.has_value() && links->size() == 4 && (*links)[1].id == 7 && (*links)[1].partnerId == 40,
          "trailing input is accepted and ignored");
    check(trailing == trailingOriginal, "reading with trailing input leaves it unchanged");
  }
  // An odd locator count can still contain correctly linked pads.
  {
    const std::vector<Pad> pads{{1, 2, 30}, {2, 3, 40}, {3, 1, 50}};
    auto objs = webo::findLevelObjects(view(teleLevel(pads, yOf(pads))));
    check(objs.has_value() && teleportPlacements(*objs) == 3 &&
              objs->teleportPartner == std::vector<int>({1, 2, 0}) && objs->teleportLinkDiagnostic.empty(),
          "three pads (odd count) pair by ids");
  }
  // Duplicate and unpaired ids reach LevelObjects with a diagnostic.
  {
    const std::vector<Pad> dup{{5, 6, 30}, {6, 5, 40}, {5, 6, 50}};
    auto objs = webo::findLevelObjects(view(teleLevel(dup, yOf(dup))));
    check(objs.has_value() && objs->teleportPartner == std::vector<int>({1, 2, 1}) &&
              objs->teleportLinkDiagnostic.find("last") != std::string::npos,
          "duplicate id: last record wins, diagnostic recorded");
    const std::vector<Pad> lone{{1, 2, 30}, {2, 9, 40}};
    objs = webo::findLevelObjects(view(teleLevel(lone, yOf(lone))));
    check(objs.has_value() && objs->teleportPartner == std::vector<int>({1, -1}) &&
              objs->teleportLinkDiagnostic.find("no pad carries") != std::string::npos,
          "unpaired id: no partner, diagnostic recorded");
  }
  // The structured pads cannot be correlated with the located ones: no links,
  // a diagnostic, and RetailLevel keeps the labelled file-order fallback.
  {
    const std::vector<Pad> pads{{1, 2, 30}, {2, 1, 40}};
    auto objs = webo::findLevelObjects(view(teleLevel(pads, {30.0, 41.0})));
    check(objs.has_value() && objs->teleportLinks.empty() && objs->teleportPartner.empty() &&
              objs->teleportLinkDiagnostic.find("does not match") != std::string::npos,
          "Y mismatch between located and structured pad: no links, diagnostic");
    objs = webo::findLevelObjects(view(teleLevel(pads, {30.0})));
    check(objs.has_value() && objs->teleportLinks.empty() &&
              objs->teleportLinkDiagnostic.find("structured pads vs") != std::string::npos,
          "count mismatch: no links, diagnostic");
  }
  // A rejected structured read: only the reader's invalid_argument is translated.
  {
    const std::vector<Pad> pads{{1, 2, 30}, {2, 1, 40}};
    auto bytes = teleLevel(pads, yOf(pads));
    bytes.resize(bytes.size() - 6);  // cut into the final required words
    const auto original = bytes;
    auto links = webo::readTeleportLinks(view(bytes));
    check(!links.has_value() && links.error().starts_with("teleport links: "),
          "truncated level: readTeleportLinks reports the reader's message");
    auto objs = webo::findLevelObjects(view(bytes));
    check(objs.has_value() && teleportPlacements(*objs) == 2 && objs->teleportLinks.empty() &&
              objs->teleportPartner.empty() && objs->teleportLinkDiagnostic.starts_with("teleport links: "),
          "rejected read: pads are still located, links empty, fallback diagnostic recorded");
    check(bytes == original, "rejected read leaves the input unchanged");
    const std::vector<std::uint8_t> junk(16, 0x11);
    check(!webo::readTeleportLinks(view(junk)).has_value(), "junk input is rejected, not thrown");
  }
}

struct Expect {
  const char* level;
  std::vector<int> partner;
};
// Literal partner table indexed by locator order.
const std::vector<Expect> kExpect = {
    {"H06_MT_1", {2, 3, 0, 1}},    {"H08_MT_1", {1, 0, 3, 2}},   {"H10_MT_1", {1, 0}},
    {"M12_MT_1", {1, 0}},          {"M16_MG_1", {1, 0}},         {"M17_MT_1", {2, 3, 0, 1}},
    {"M18_MT_1", {3, 2, 1, 0}},    {"M19_MT_1", {1, 0, 3, 2}},   {"M21_KF_1", {1, 0}},
    {"MM_TELE_003", {2, 3, 0, 1}}, {"SHOW_KF3", {1, 0}},         {"SHOW_MT3", {1, 0}},
    {"VH08_MT", {1, 0}},           {"VH10_MT", {1, 0}},          {"VH12_KF1", {2, 2, 3, 2}},
    {"VH13_MT", {1, 0}},           {"VH18_JC", {2, 3, 0, 2}},    {"VH21_JC", {1, 0, 3, 2}},
};

void testRetail(const fs::path& dir) {
  std::cout << "retail teleport pairing\n";
  auto lib = webo::AssetLibrary::open(dir / "lemmings.box", dir / "patch.box");
  if (!lib) {
    check(false, lib.error());
    return;
  }
  for (const auto& e : kExpect) {
    std::string file;
    for (const auto& n : webo::levelFileNames(*lib))
      if (n.find(std::string("\\") + e.level + ".LVL") != std::string::npos) file = n;
    check(!file.empty(), std::format("{} in archive", e.level));
    if (file.empty()) continue;
    auto blob = lib->read(file);
    auto objs = webo::findLevelObjects(*blob);
    auto grid = webo::findCollisionGrid(*blob);
    auto setup = webo::sim::loadRetailLevel(*lib, std::string("\\") + e.level + ".LVL");
    check(objs && grid && setup.has_value(), std::format("{} loads", e.level));
    if (!objs || !grid || !setup) continue;
    check(objs->teleportLinkDiagnostic.empty(),
          std::format("{} link diagnostic empty (got '{}')", e.level, objs->teleportLinkDiagnostic));
    std::vector<webo::TeleportTrigger> pads;
    for (const auto& p : objs->placed)
      if (p.kind == webo::PlacedKind::Teleport)
        pads.push_back(webo::teleportTriggerFromPose(p.pose, grid->width, grid->height));
    check(pads.size() == e.partner.size() && setup->teleports.size() == e.partner.size(),
          std::format("{} {} teleports", e.level, e.partner.size()));
    if (pads.size() != e.partner.size() || setup->teleports.size() != e.partner.size()) continue;
    for (std::size_t i = 0; i < pads.size(); ++i) {
      const auto& t = setup->teleports[i];
      const auto& dst = pads[static_cast<std::size_t>(e.partner[i])];
      check(t.x == pads[i].x && t.y == pads[i].y && t.width == pads[i].width &&
                t.height == pads[i].height && t.destX == dst.destCol &&
                t.destY == grid->height - 1 - dst.destWorldY,
            std::format("{} pad {} -> pad {}", e.level, i, e.partner[i]));
    }
  }
}
}  // namespace

int main() {
  testResolver();
  testSynthetic();
  if (const char* dir = std::getenv("WEBOLUTION_GAME_DIR"))
    testRetail(dir);
  else
    std::cout << "retail teleport pairing: SKIP (WEBOLUTION_GAME_DIR unset)\n";
  std::cout << std::format("{} checks, {} failures\n", g_checks, g_failures);
  return g_failures == 0 ? 0 : 1;
}
