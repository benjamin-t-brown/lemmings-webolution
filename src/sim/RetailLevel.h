// The production retail-level loader: one `.LVL` in the archive to one
// `sim::LevelSetup`, with no reverse engineering left open -- every piece
// this file assembles is already read/measured and already tested
// elsewhere (`assets/LevelObjects.h`'s `hatchSpawnCell` /
// `exitTriggerFromPose` / `teleportTriggerFromPose` /
// `speedPadTriggerFromPose` / `leverTriggerFromPose` / `findLevelSkills`,
// `assets/GameData.h`'s `loadLevelMap` / `loadReleaseRateTable`). D3a
// (PLAN.md Phase D, D3 "one retail level end to end") turns that existing,
// per-field coverage into one function, replacing the two hand-built
// `LevelSetup`s `test_world.cpp::testRetailSpawn` /
// `testRetailReleaseTiming` and `SimReleaseTrace.cpp` build by hand, each
// for `MM_CLIMB_002` only.
//
// Honesty over convenience: there is deliberately no
// `old/src::placeSpawnPoints`-style fallback here. A `.LVL` with no hatch
// record or no `Exit*.a3d` placement is reported as a load failure, not
// papered over with a guessed box. See `build/tmp/d3a-retail-level-setup/
// sweep.txt` for which (if any) retail levels actually hit that path --
// round 1's own corpus probe found none.
//
// What is lost, by construction of `LevelSetup` itself (not a new loss this
// file introduces): `LevelSetup` carries exactly one
// `releaseArmSeconds`/`releaseFirstCountdownSeconds`/`releaseIntervalSeconds`
// triple, but 33 of the 102 retail levels have more than one hatch, and
// `World::spawn()` already round-robins every `Entrance` pushed here against
// that one shared accumulator (`World.cpp`, unchanged by this file). So a
// multi-hatch level gets every hatch's own spawn *position* (one `Entrance`
// per hatch, in stream order) but only the *first* hatch's own release
// cadence -- every other hatch's own `rate`/`rateMin`/`rateMax` is still
// read and reported (`RetailLevelReport::hatches`) but not wired into the
// sim. `totalLemmings` is read from `LevelInfo` (not summed from the hatch
// words) because round 1's own corpus probe found the two always agree
// (sum(hatch.count) == LevelInfo.totalLemmings on all 102 levels) and
// `LevelInfo` is the source the brief's acceptance criterion names.

#pragma once

#include "sim/World.h"

#include <expected>
#include <string>
#include <string_view>
#include <vector>

namespace webo {
class AssetLibrary;
}  // namespace webo

namespace webo::sim {

// One hatch's own numbers, kept because `LevelSetup` has room for only one
// release interval (see this header's own top comment). Not used by
// `World` directly; this is what `loadRetailLevelReport` and the corpus
// sweep report instead of silently dropping it.
struct RetailHatchSummary {
  int count = 0;
  int x = 0;
  int y = 0;
  int type = 0;
  int rate = 0;
  int rateMin = 1;
  int rateMax = 99;
  int clampedRate = 0;            // rate clamped into [rateMin, rateMax]
  bool rateOutsideClamp = false;  // the stored rate itself, before clamping
  double intervalSeconds = 0.0;   // ReleaseRates::secondsPerLemming(clampedRate)
};

// Everything `loadRetailLevel` read, for callers (the corpus sweep, this
// file's own test) that want more than the one `LevelSetup` can carry.
struct RetailLevelReport {
  LevelSetup setup;
  // Every hatch record found, in stream order. hatches.front() is the one
  // whose rate/min/max drove setup.releaseIntervalSeconds et al.
  std::vector<RetailHatchSummary> hatches;
};

// Builds the `sim::LevelSetup` the original would run for one retail level,
// straight from the archive:
//
//   - terrain from the stored collision grid (CollisionGrid.h)
//   - one Entrance per hatch record, at hatchSpawnCell (LevelObjects.h);
//     release timing (releaseArmSeconds/releaseFirstCountdownSeconds/
//     releaseIntervalSeconds, see World.h) from the *first* hatch's own
//     rate word, clamped into its own min/max, looked up in RELEASERATE.PC
//     -- see this header's own "what is lost" note for multi-hatch levels
//   - every Exit*.a3d placement via exitTriggerFromPose
//   - every teleport.a3d / speedup_two_way.a3d / lever.a3d placement via
//     their own trigger functions (teleports paired i <-> i+n/2, the
//     existing old/src stand-in -- "Already settled" in the brief, not a
//     new guess)
//   - totalLemmings / saveTarget / timeLimitTicks from LEVELMAP.DAT
//     (webo::LevelInfo)
//   - skills from the level's own trailer (findLevelSkills)
//
// `levelId` is matched the way `SimReleaseTrace` and `test_world.cpp`
// already do: a substring match against `levelFileNames(lib)` (the archive
// stores paths like "COMPILED\\MM_CLIMB_002.LVL"). LEVELMAP.DAT's own id
// for the *matched* file (not the caller's possibly-partial `levelId`) is
// what resolves `LevelInfo`, so a substring that happens to match the wrong
// file still gets that file's own, self-consistent totals.
//
// Fails, rather than guessing, when: the archive has no such level; the
// `.LVL` has no readable hatch record; the `.LVL` has no readable
// `Exit*.a3d` placement; the collision grid, skill trailer or LEVELMAP.DAT
// row cannot be read.
std::expected<LevelSetup, std::string> loadRetailLevel(const AssetLibrary& lib,
                                                        std::string_view levelId);

// The same build, with the diagnostics `LevelSetup` cannot carry.
std::expected<RetailLevelReport, std::string> loadRetailLevelReport(
    const AssetLibrary& lib, std::string_view levelId);

}  // namespace webo::sim
