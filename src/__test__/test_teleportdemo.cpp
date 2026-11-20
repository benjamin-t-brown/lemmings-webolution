// Compare literal installed pad pairs under the two sim::loadRetailLevel builds.
// kFileOrder pairs i with i+n/2. kByIds matches this pad's +0x98 word to its
// partner's base ID. Pad indices follow locator order, including on MM_TELE_003.
// Calculate the source box and destination pose with teleportTriggerFromPose.
#include "assets/Box.h"
#include "assets/CollisionGrid.h"
#include "assets/LevelIndex.h"
#include "assets/LevelObjects.h"
#include "sim/RetailLevel.h"
#include "sim/World.h"

#include <cstdlib>
#include <filesystem>
#include <format>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

namespace fs = std::filesystem;

namespace {

#ifdef TELEPORTDEMO_FROZEN
constexpr bool kFrozen = true;
constexpr const char* kSide = "before";
#else
constexpr bool kFrozen = false;
constexpr const char* kSide = "after";
#endif

struct Level {
  const char* name;
  std::vector<int> fileOrder;  // half-list pairing by locator index
  std::vector<int> byIds;      // id pairing, locator index -> partner locator index
};

const std::vector<Level> kLevels = {
    {"H06_MT_1", {2, 3, 0, 1}, {2, 3, 0, 1}},
    {"H08_MT_1", {2, 3, 0, 1}, {1, 0, 3, 2}},
    {"H10_MT_1", {1, 0}, {1, 0}},
    {"M12_MT_1", {1, 0}, {1, 0}},
    {"M16_MG_1", {1, 0}, {1, 0}},
    {"M17_MT_1", {2, 3, 0, 1}, {2, 3, 0, 1}},
    {"M18_MT_1", {2, 3, 0, 1}, {3, 2, 1, 0}},
    {"M19_MT_1", {2, 3, 0, 1}, {1, 0, 3, 2}},
    {"M21_KF_1", {1, 0}, {1, 0}},
    {"MM_TELE_003", {2, 3, 0, 1}, {2, 3, 0, 1}},
    {"SHOW_KF3", {1, 0}, {1, 0}},
    {"SHOW_MT3", {1, 0}, {1, 0}},
    {"VH08_MT", {1, 0}, {1, 0}},
    {"VH10_MT", {1, 0}, {1, 0}},
    {"VH12_KF1", {2, 3, 0, 1}, {2, 2, 3, 2}},
    {"VH13_MT", {1, 0}, {1, 0}},
    {"VH18_JC", {2, 3, 0, 1}, {2, 3, 0, 2}},
    {"VH21_JC", {2, 3, 0, 1}, {1, 0, 3, 2}},
};

int g_failures = 0, g_checks = 0;
std::ofstream g_report;

void check(bool ok, const std::string& what) {
  ++g_checks;
  g_report << (ok ? "ok   " : "FAIL ") << what << '\n';
  if (!ok) {
    ++g_failures;
    std::cerr << "  FAIL  " << what << "\n";
  }
}

bool sameTeleport(const webo::sim::Teleport& t, const webo::sim::Teleport& e) {
  return t.x == e.x && t.y == e.y && t.width == e.width && t.height == e.height &&
         t.destX == e.destX && t.destY == e.destY;
}

}  // namespace

int main() {
  const char* dirC = std::getenv("WEBOLUTION_GAME_DIR");
  if (!dirC || !*dirC) {
    std::cout << "TeleportDemo[" << kSide << "]: SKIPPED (set WEBOLUTION_GAME_DIR to retail assets)\n";
    return 0;
  }
  const fs::path dir = dirC;
  auto lib = webo::AssetLibrary::open(dir / "lemmings.box", dir / "patch.box");
  if (!lib) {
    std::cerr << "TeleportDemo: " << lib.error() << '\n';
    return 1;
  }
  const fs::path outDir = fs::path(TELEPORTDEMO_OUT_DIR) / kSide;
  std::error_code ec;
  fs::create_directories(outDir, ec);
  g_report.open(outDir / "teleports.txt");
  g_report << "side=" << kSide << " frozen=" << kFrozen << " levels=" << kLevels.size() << '\n';

  std::size_t changedLevels = 0, matchedOld = 0, matchedIds = 0;
  for (const auto& lv : kLevels) {
    const std::string id = std::string("\\") + lv.name + ".LVL";
    std::string file;
    for (const auto& n : webo::levelFileNames(*lib))
      if (n.find(id) != std::string::npos) file = n;
    check(!file.empty(), std::format("{} is in the effective archive", lv.name));
    if (file.empty()) continue;
    auto blob = lib->read(file);
    auto objs = webo::findLevelObjects(*blob);
    auto grid = webo::findCollisionGrid(*blob);
    auto setup = webo::sim::loadRetailLevel(*lib, id);
    check(objs && grid && setup.has_value(), std::format("{} loads through loadRetailLevel", lv.name));
    if (!objs || !grid || !setup) continue;

    std::vector<webo::TeleportTrigger> pads;
    for (const auto& p : objs->placed)
      if (p.kind == webo::PlacedKind::Teleport)
        pads.push_back(webo::teleportTriggerFromPose(p.pose, grid->width, grid->height));
    check(pads.size() == lv.byIds.size() && pads.size() == lv.fileOrder.size(),
          std::format("{} has {} located pads", lv.name, lv.byIds.size()));
    if (pads.size() != lv.byIds.size()) continue;

    const auto expectation = [&](const std::vector<int>& partner) {
      std::vector<webo::sim::Teleport> want;
      for (std::size_t i = 0; i < pads.size(); ++i) {
        const auto& src = pads[i];
        const auto& dst = pads[static_cast<std::size_t>(partner[i])];
        want.push_back({src.x, src.y, src.width, src.height, dst.destCol,
                        grid->height - 1 - dst.destWorldY});
      }
      return want;
    };
    const auto wantOld = expectation(lv.fileOrder);
    const auto wantIds = expectation(lv.byIds);
    const auto equalTo = [&](const std::vector<webo::sim::Teleport>& want) {
      if (setup->teleports.size() != want.size()) return false;
      for (std::size_t i = 0; i < want.size(); ++i)
        if (!sameTeleport(setup->teleports[i], want[i])) return false;
      return true;
    };
    const bool isOld = equalTo(wantOld), isIds = equalTo(wantIds);
    const bool differs = lv.fileOrder != lv.byIds;
    if (differs) ++changedLevels;
    if (isOld) ++matchedOld;
    if (isIds) ++matchedIds;

    std::string line = std::format("{}: {} teleports, partners by pad:", lv.name, setup->teleports.size());
    for (std::size_t i = 0; i < setup->teleports.size(); ++i) {
      int p = -1;
      for (std::size_t j = 0; j < pads.size(); ++j)
        if (setup->teleports[i].destX == pads[j].destCol &&
            setup->teleports[i].destY == grid->height - 1 - pads[j].destWorldY)
          p = static_cast<int>(j);
      line += std::format(" {}>{}", i, p);
    }
    g_report << line << '\n';

    if (kFrozen) {
      check(isOld, std::format("{} BEFORE: file-order pairing {}", lv.name, differs ? "(differs from ids)" : ""));
      if (differs)
        check(!isIds, std::format("{} BEFORE: does not pair by ids", lv.name));
    } else {
      check(isIds, std::format("{} AFTER: id pairing", lv.name));
      if (differs)
        check(!isOld, std::format("{} AFTER: no longer the file-order pairing", lv.name));
    }
  }
  g_report << std::format("levels whose pairing differs between the rules: {}; matched file order {}, matched ids {}\n",
                          changedLevels, matchedOld, matchedIds);
  check(changedLevels == 6, "exactly six installed levels pair differently under ids than under file order");
  if (kFrozen)
    check(matchedOld == kLevels.size() && matchedIds == kLevels.size() - changedLevels,
          "BEFORE: all 18 levels are file-order, the 12 unchanged ones also equal the id pairing");
  else
    check(matchedIds == kLevels.size() && matchedOld == kLevels.size() - changedLevels,
          "AFTER: all 18 levels are id-paired, the 12 unchanged ones also equal file order");

  std::cout << std::format("TeleportDemo[{}]: {} checks, {} failures\n", kSide, g_checks, g_failures);
  return g_failures == 0 ? 0 : 1;
}
