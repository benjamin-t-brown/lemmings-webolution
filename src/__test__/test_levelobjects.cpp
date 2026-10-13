// Tests for placed-model reads (entrance/exit + scenery/hazards) and skills.

#include "assets/Box.h"
#include "assets/Cylinder.h"
#include "assets/LevelIndex.h"
#include "assets/LevelObjects.h"
#include "assets/SurfaceMesh.h"

#include <array>
#include <cmath>
#include <cstdlib>
#include <filesystem>
#include <format>
#include <iostream>
#include <map>
#include <string>
#include <string_view>

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

}  // namespace

int main() {
  testWorldToMask();

  const char* dir = std::getenv("WEBOLUTION_GAME_DIR");
  if (!dir) {
    std::cout << "retail placements: SKIP (WEBOLUTION_GAME_DIR unset)\n";
    std::cout << "retail placed models: SKIP (WEBOLUTION_GAME_DIR unset)\n";
    std::cout << "retail skills: SKIP (WEBOLUTION_GAME_DIR unset)\n";
  } else {
    testRetail(dir);
    testRetailPlaced(dir);
    testRetailSkills(dir);
  }

  std::cout << std::format("{} checks, {} failures\n", g_checks, g_failures);
  return g_failures == 0 ? 0 : 1;
}
