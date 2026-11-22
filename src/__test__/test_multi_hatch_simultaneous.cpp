// Scheduler invariants, not a scripted play route or fidelity comparison.
#include "assets/Box.h"
#include "assets/GameData.h"
#include "sim/RetailLevel.h"

#include <algorithm>
#include <iostream>
#include <string>

using namespace webo;
using namespace webo::sim;
namespace {
int failures = 0;
void check(bool ok, const std::string& message) {
  if (!ok) { ++failures; std::cerr << "FAIL " << message << '\n'; }
}

void scheduler() {
  for (int partition : {20, 200, -200, 0}) {
    LevelSetup s;
    s.terrain = Terrain(100, 100);
    s.terrain.fillBox(0, 80, 100, 20, Material::Earth);
    s.totalLemmings = 5;
    Entrance a, b;
    a.x = 10; a.y = 60; a.releaseCount = 2; a.alternateDirection = true;
    b.x = 60; b.y = 60; b.releaseCount = 3; b.sheetType = 1; b.direction = -1;
    s.entrances = {a, b};
    s.releaseIntervalTicks = 2;
    s.releaseFirstCountdownSeconds = 0.1;
    s.releaseIntervalSeconds = partition == 0 ? 0.0 : 0.4;
    World w(s);
    for (int i = 0; i < 150 && w.spawned() < 5; ++i) {
      const int first = w.spawnedFromEntrance(0), second = w.spawnedFromEntrance(1);
      if (partition <= 0) w.tick(); else w.advanceElapsedMs(partition);
      const int da = w.spawnedFromEntrance(0) - first, db = w.spawnedFromEntrance(1) - second;
      check(da <= 1 && db <= 1, "one birth per hatch per update");
      if (first < 2) check(da == db, "nonempty hatches emit together");
      if (first == 0 && da == 1)
        check(w.lemmings()[0].y == w.lemmings()[1].y &&
              w.lemmings()[0].stepBudgetSeconds == w.lemmings()[1].stepBudgetSeconds,
              "simultaneous births receive the same fractional first-fall budget");
      check(w.remainingFromEntrance(0) == 2 - w.spawnedFromEntrance(0), "remaining count follows quota");
    }
    check(w.spawned() == 5 && w.spawnedFromEntrance(0) == 2 && w.spawnedFromEntrance(1) == 3,
          "uneven quotas stop independently and release full total");
    check(w.lemmings()[0].sheetType == 0 && w.lemmings()[1].sheetType == 1 &&
          w.lemmings()[2].sheetType == 0 && w.lemmings()[4].sheetType == 1,
          "serialized tie order keeps each hatch's actor type");
    check(w.lemmings()[0].facing != w.lemmings()[2].facing, "alternation belongs to its emitting hatch");
    const auto total = w.spawned();
    w.advanceElapsedMs(2000);
    check(w.spawned() == total, "exhausted hatches stay empty");

    World nuked(s);
    nuked.nuke(); nuked.advanceElapsedMs(1000);
    check(nuked.spawned() == 0 && nuked.remainingFromEntrance(1) == 0, "nuke cancels every hatch");
  }
  LevelSetup fallback;
  fallback.terrain = Terrain(100, 100);
  fallback.entrances = {{10, 60}, {60, 60}};
  fallback.totalLemmings = 5;
  World w(fallback);
  check(w.remainingFromEntrance(0) == 3 && w.remainingFromEntrance(1) == 2,
        "unspecified hand-built quotas divide evenly, earlier hatch gets remainder");
  fallback.entrances[0].releaseCount = 1;
  World mixed(fallback);
  check(mixed.remainingFromEntrance(0) == 1 && mixed.remainingFromEntrance(1) == 4,
        "fallback divides only the unassigned population");

  fallback.totalLemmings = 2;
  fallback.entrances[0].releaseCount = 5;
  fallback.entrances[1].releaseCount = 5;
  World capped(fallback);
  capped.tick(); capped.tick();
  check(capped.spawned() == 2, "LevelInfo total caps inconsistent entrance quotas");

  LevelSetup independent;
  independent.terrain = Terrain(100, 100);
  independent.totalLemmings = 6;
  independent.entrances = {{10, 60}, {60, 60}};
  independent.releaseFirstCountdownSeconds = 0.125;
  independent.releaseIntervalSeconds = 0.5;
  independent.entrances[0].releaseIntervalSeconds = 0.5;
  independent.entrances[1].releaseIntervalSeconds = 0.25;
  World elapsed(independent);
  elapsed.advanceElapsedMs(125); // Arm.
  elapsed.advanceElapsedMs(125); // Both release.
  check(elapsed.spawnedFromEntrance(0) == 1 && elapsed.spawnedFromEntrance(1) == 1,
        "different intervals share the first-release seed");
  elapsed.setHatchReleaseRate(1, 42, 0.125, 1);
  elapsed.advanceElapsedMs(125);
  check(elapsed.spawned() == 2, "changing the interval keeps the pending countdown");
  elapsed.advanceElapsedMs(125);
  check(elapsed.spawnedFromEntrance(0) == 1 && elapsed.spawnedFromEntrance(1) == 2,
        "the faster hatch releases independently");
  elapsed.advanceElapsedMs(125);
  check(elapsed.spawnedFromEntrance(0) == 1 && elapsed.spawnedFromEntrance(1) == 3,
        "the next release uses the changed interval");
  elapsed.advanceElapsedMs(125);
  check(elapsed.spawnedFromEntrance(0) == 2 && elapsed.spawnedFromEntrance(1) == 3,
        "the other hatch retains its original interval");
}

void census(const AssetLibrary& lib) {
  const auto infos = loadLevelMap(lib);
  check(bool(infos), "installed map loads");
  if (!infos) return;
  int multis = 0, adjusted = 0;
  std::cout << "level,partition_ms,released,total,complete_ms,limit_s,initial_rates,hatch_counts,first_batch_ms\n";
  for (const auto& info : *infos) {
    if (info.isBlank()) continue;
    auto report = loadRetailLevelReport(lib, info.id);
    check(bool(report), info.id + " loads");
    if (!report || report->hatches.size() < 2) continue;
    ++multis;
    int quotaSum = 0;
    for (std::size_t h = 0; h < report->hatches.size(); ++h) {
      check(report->setup.entrances[h].releaseCount == report->hatches[h].count,
            info.id + " installed quota reaches entrance");
      check(report->setup.entrances[h].releaseRate == report->hatches[h].initialRate &&
            report->hatches[h].initialRate >= report->hatches[h].clampedRate &&
            report->hatches[h].initialRate <= report->hatches[h].rateMax,
            info.id + " effective hatch rate reaches entrance within its bounds");
      adjusted += report->hatches[h].initialRate != report->hatches[h].clampedRate;
      quotaSum += report->hatches[h].count;
    }
    check(quotaSum == report->setup.totalLemmings, info.id + " installed quotas sum to LevelInfo total");
    for (int partition : {200, -200, 20}) {
      auto setup = report->setup;
      if (partition == 20) setup.releaseArmSeconds = 0.0; // connected-entry contract
      World w(std::move(setup));
      // Reserve 5 s for entry: the level clock advances while hatch releases are held.
      if (partition == 20) w.advanceElapsedMsWithReleaseGate(5000, false, false);
      std::uint64_t firstBatchMs = 0;
      while (!w.timedOut() && w.spawned() < quotaSum) {
        if (partition < 0) w.tick(); else w.advanceElapsedMs(partition);
        if (!firstBatchMs && w.spawned()) firstBatchMs = w.levelClockMs();
      }
      check(w.spawned() == quotaSum && w.levelClockMs() < std::uint64_t(info.timeLimitSeconds) * 1000,
            info.id + " full total before time limit");
      std::cout << info.id << ',' << partition << ',' << w.spawned() << ',' << quotaSum << ','
                << w.levelClockMs() << ',' << info.timeLimitSeconds << ',';
      for (const auto& hatch : report->hatches) std::cout << hatch.initialRate << ';';
      std::cout << ',';
      for (std::size_t h = 0; h < report->hatches.size(); ++h) {
        check(w.spawnedFromEntrance(h) == report->hatches[h].count, info.id + " full stored per-hatch total");
        std::cout << w.spawnedFromEntrance(h) << ';';
      }
      std::cout << ',' << firstBatchMs << '\n';
    }
  }
  check(multis == 33, "all 33 installed multi-hatch levels checked");
  std::cout << "multi_hatch_levels=" << multis << " initial_rate_adjustments=" << adjusted << '\n';
}
}

int main(int argc, char** argv) {
  scheduler();
  if (argc > 1 && *argv[1]) {
    const auto lib = AssetLibrary::open(std::filesystem::path(argv[1]) / "lemmings.box",
                                      std::filesystem::path(argv[1]) / "patch.box");
    check(bool(lib), "installed assets open");
    if (lib) census(*lib);
  }
  std::cout << "failures=" << failures << '\n';
  return failures ? 1 : 0;
}
