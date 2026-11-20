// Check how loadRetailLevel and loadRetailLevelReport assemble LevelSetup,
// then report results for the installed corpus. Requires WEBOLUTION_GAME_DIR
// with lemmings.box.

#include "assets/Box.h"
#include "assets/CollisionGrid.h"
#include "assets/GameData.h"
#include "assets/LevelIndex.h"
#include "assets/LevelObjects.h"
#include "sim/RetailLevel.h"
#include "sim/World.h"

#include <algorithm>
#include <cstdlib>
#include <filesystem>
#include <format>
#include <fstream>
#include <iostream>
#include <string>

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

using namespace webo::sim;

// Cross-check MM_CLIMB_002's hatchSpawnCell, rate-20 interval, LevelInfo totals,
// skills and exits against individual parser results.
void testMmClimb002(const std::filesystem::path& dir) {
  std::cout << "loadRetailLevel: MM_CLIMB_002 matches the already-tested primitives\n";

  auto lib = webo::AssetLibrary::open(dir / "lemmings.box", dir / "patch.box");
  if (!lib) {
    check(false, lib.error());
    return;
  }

  std::string file;
  for (const auto& name : webo::levelFileNames(*lib))
    if (name.find("MM_CLIMB_002") != std::string::npos) {
      file = name;
      break;
    }
  if (file.empty()) {
    check(false, "MM_CLIMB_002 is not in the archive");
    return;
  }

  auto actual = loadRetailLevelReport(*lib, "MM_CLIMB_002");
  check(actual.has_value(),
        std::format("loadRetailLevelReport succeeds (got {})",
                    actual.has_value() ? std::string() : actual.error()));
  if (!actual)
    return;
  const LevelSetup& s = actual->setup;

  // Independently recompute from the primitives.
  auto blob = lib->read(file);
  check(blob.has_value(), "the .LVL reads");
  if (!blob)
    return;
  auto objs = webo::findLevelObjects(*blob);
  auto grid = webo::findCollisionGrid(*blob);
  auto skills = webo::findLevelSkills(*blob);
  auto infos = webo::loadLevelMap(*lib);
  auto rates = webo::loadReleaseRateTable(*lib);
  check(objs.has_value() && grid.has_value() && skills.has_value() && infos.has_value() &&
            rates.has_value(),
        "every underlying primitive succeeds");
  if (!objs || !grid || !skills || !infos || !rates)
    return;

  check(objs->hatches.size() == 1,
        std::format("MM_CLIMB_002 has one hatch record (got {})", objs->hatches.size()));
  if (objs->hatches.empty())
    return;
  const auto& h = objs->hatches.front();
  check(h.rate == 20, std::format("hatch rate word is 20 (got {})", h.rate));

  // One entrance, at hatchSpawnCell.
  check(s.entrances.size() == 1,
        std::format("one entrance (got {})", s.entrances.size()));
  if (!s.entrances.empty()) {
    const auto [ex, ey] = webo::hatchSpawnCell(h, grid->width, grid->height);
    check(s.entrances.front().x == ex && s.entrances.front().y == ey,
          std::format("entrance at hatchSpawnCell ({}, {}) (got ({}, {}))", ex, ey,
                      s.entrances.front().x, s.entrances.front().y));
    check(s.entrances.front().source == Entrance::Source::HatchWords,
          "entrance source is HatchWords, not the Entrance*.a3d fallback");
    check(s.entrances.front().sheetType == h.type, "entrance sheetType is the hatch's type");
  }

  // The interval in seconds comes from RELEASERATE.PC row 20.
  const double expectedSeconds = rates->secondsPerLemming(20);
  check(expectedSeconds == 2.8864866275534937,
        std::format("rate 20 is 2.8864866275534937 s (got {:.17g})", expectedSeconds));
  check(s.releaseIntervalSeconds == expectedSeconds,
        std::format("setup.releaseIntervalSeconds is the rate-20 seconds exactly (got {:.17g})",
                    s.releaseIntervalSeconds));
  check(s.releaseFirstCountdownSeconds == 1.3, "releaseFirstCountdownSeconds is the READ 1.3");

  // LevelInfo totals.
  const webo::LevelInfo* info = nullptr;
  for (const auto& i : *infos)
    if (i.id == "MM_CLIMB_002")
      info = &i;
  check(info != nullptr, "MM_CLIMB_002 has a LEVELMAP.DAT row");
  if (info) {
    check(s.totalLemmings == static_cast<int>(info->totalLemmings),
          std::format("totalLemmings is LevelInfo's {} (got {})", info->totalLemmings,
                      s.totalLemmings));
    check(s.saveTarget == static_cast<int>(info->saveTarget),
          std::format("saveTarget is LevelInfo's {} (got {})", info->saveTarget, s.saveTarget));
    const int expectedTicks =
        static_cast<int>(info->timeLimitSeconds) * World::Rules{}.ticksPerSecond;
    check(s.timeLimitTicks == expectedTicks,
          std::format("timeLimitTicks is LevelInfo's seconds * ticksPerSecond = {} (got {})",
                      expectedTicks, s.timeLimitTicks));
  }

  // Skills as read.
  bool skillsMatch = true;
  for (std::size_t i = 0; i < skills->counts.size(); ++i)
    if (s.skills[i] != static_cast<int>(skills->counts[i]))
      skillsMatch = false;
  check(skillsMatch, "skills match findLevelSkills exactly");

  // Exits as read: same count, same box exitTriggerFromPose computes.
  check(s.exits.size() == objs->exits.size(),
        std::format("exit count matches (got {} vs {})", s.exits.size(), objs->exits.size()));
  for (std::size_t i = 0; i < objs->exits.size() && i < s.exits.size(); ++i) {
    const webo::ExitTrigger box =
        webo::exitTriggerFromPose(objs->exits[i], grid->width, grid->height);
    check(s.exits[i].x == box.x && s.exits[i].y == box.y && s.exits[i].width == box.width &&
              s.exits[i].height == box.height,
          std::format("exit {} matches exitTriggerFromPose", i));
  }

  // Advance the assembled World until it releases a lemming.
  World w(s);
  for (int t = 0; t < 20; ++t)
    w.tick();
  check(w.spawned() >= 1, "at least one lemming spawned in 20 ticks");
}

// loadRetailLevel fails cleanly (does not guess) on a level id that is not
// in the archive.
void testUnknownLevel(const std::filesystem::path& dir) {
  std::cout << "loadRetailLevel: an unknown level id fails, not guesses\n";
  auto lib = webo::AssetLibrary::open(dir / "lemmings.box", dir / "patch.box");
  if (!lib) {
    check(false, lib.error());
    return;
  }
  auto result = loadRetailLevel(*lib, "NOT_A_REAL_LEVEL_ID");
  check(!result.has_value(), "an unknown level id is an error, not a default LevelSetup");
}

// Report load errors, hatch counts and each hatch's rate, minimum and maximum.
void writeCorpusSweep(const std::filesystem::path& dir) {
  std::cout << "corpus sweep -> build/tmp/d3a-retail-level-setup/sweep.txt\n";
  auto lib = webo::AssetLibrary::open(dir / "lemmings.box", dir / "patch.box");
  if (!lib) {
    check(false, lib.error());
    return;
  }

  const std::filesystem::path outDir = "build/tmp/d3a-retail-level-setup";
  std::error_code ec;
  std::filesystem::create_directories(outDir, ec);
  std::ofstream out(outDir / "sweep.txt");
  check(out.is_open(), "sweep.txt opens for writing");
  if (!out.is_open())
    return;

  auto files = webo::levelFileNames(*lib);
  std::sort(files.begin(), files.end());

  int ok = 0, failed = 0, outsideClampCount = 0, multiHatchCount = 0;
  out << std::format("# D3a corpus sweep: {} retail .LVL files, loadRetailLevelReport\n", files.size());
  for (const auto& file : files) {
    auto report = loadRetailLevelReport(*lib, file);
    if (!report) {
      out << std::format("{}\tERROR\t{}\n", file, report.error());
      ++failed;
      continue;
    }
    ++ok;
    const auto& s = report->setup;
    bool anyOutside = false;
    std::string hatchDetail;
    for (std::size_t i = 0; i < report->hatches.size(); ++i) {
      const auto& h = report->hatches[i];
      if (h.rateOutsideClamp) {
        anyOutside = true;
        ++outsideClampCount;
      }
      hatchDetail += std::format("{}[rate={} min={} max={} interval={:.6f}s outside={}]",
                                 i == 0 ? "" : ",", h.rate, h.rateMin, h.rateMax,
                                 h.intervalSeconds, h.rateOutsideClamp);
    }
    if (report->hatches.size() > 1)
      ++multiHatchCount;
    out << std::format(
        "{}\tOK\thatches={}\t{}\tentrances={}\texits={}\ttotalLemmings={}\tsaveTarget={}\t"
        "timeLimitTicks={}\tanyRateOutsideClamp={}\n",
        file, report->hatches.size(), hatchDetail, s.entrances.size(), s.exits.size(),
        s.totalLemmings, s.saveTarget, s.timeLimitTicks, anyOutside);
  }
  out << std::format(
      "# {} loaded, {} failed, {} multi-hatch, {} hatch records outside their own clamp\n",
      ok, failed, multiHatchCount, outsideClampCount);

  check(failed == 0, std::format("every retail level loads (got {} failures; see sweep.txt)", failed));
  std::cout << std::format("  {} loaded, {} failed, {} multi-hatch, {} rates outside clamp\n", ok,
                           failed, multiHatchCount, outsideClampCount);
}

}  // namespace

int main() {
  const char* dir = std::getenv("WEBOLUTION_GAME_DIR");
  if (!dir || !*dir) {
    std::cout << "retail level loader: skipped, WEBOLUTION_GAME_DIR is not set\n";
    std::cout << std::format("\n{} checks, {} failures\n", g_checks, g_failures);
    return g_failures == 0 ? 0 : 1;
  }
  const std::filesystem::path root(dir);
  if (!std::filesystem::exists(root / "lemmings.box")) {
    std::cout << "retail level loader: skipped, no lemmings.box\n";
    std::cout << std::format("\n{} checks, {} failures\n", g_checks, g_failures);
    return g_failures == 0 ? 0 : 1;
  }

  testMmClimb002(root);
  testUnknownLevel(root);
  writeCorpusSweep(root);

  std::cout << std::format("\n{} checks, {} failures\n", g_checks, g_failures);
  return g_failures == 0 ? 0 : 1;
}
