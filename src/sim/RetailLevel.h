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
// Playable-v1 release policy (playtest 10 Z6): every nonempty hatch emits
// one lemming on each shared release tick. Entrances retain the stored per-hatch
// counts; their sum equals LevelInfo.totalLemmings on all 102 installed levels.
// LevelInfo remains the authoritative total. The shared interval/rate control
// starts from the first hatch, raised only if its largest quota cannot fit the
// time limit. Original per-hatch intervals and door phases remain outside this
// provisional simultaneous scheduler.

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

// Each stored hatch's numbers. Count also reaches Entrance::releaseCount;
// secondary intervals are reported but the playable-v1 cadence stays shared.
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
  int initialSharedRate = 1; // Effective shared rate, including the provisional capacity floor.
  // Every hatch record found, in stream order. hatches.front() is the one
  // whose rate/min/max drove setup.releaseIntervalSeconds et al.
  std::vector<RetailHatchSummary> hatches;
  // lever-gates-impl-001: tagged type-1 records paired with a placed lever (by pose, never by
  // file order) and why the serialized gadget records could not be applied, if they could not.
  std::size_t pairedLevers = 0;
  std::string gateLinkDiagnostic;
};

// Builds the `sim::LevelSetup` the original would run for one retail level,
// straight from the archive:
//
//   - terrain from the stored collision grid (CollisionGrid.h)
//   - one Entrance per hatch record, at hatchSpawnCell (LevelObjects.h);
//     release timing (releaseArmSeconds/releaseFirstCountdownSeconds/
//     releaseIntervalSeconds, see World.h) from the *first* hatch's own
//     rate word, clamped into its own min/max, looked up in RELEASERATE.PC
//     -- see this header's provisional shared-cadence policy
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

// How a caller-supplied level id was matched to an archive path. Precedence:
//   ExactId         the whole id equals the archive file's id, ignoring case
//                   ("m06_kf_1" -> "COMPILED\\M06_KF_1.LVL"). Wins over any
//                   substring hit, so "M06_KF_1" never lands on "M06_KF_10".
//   Substring       the pre-existing case-sensitive substring match, first hit in
//                   archive order (partial ids, "\\NAME.LVL"). Unchanged behaviour.
//   CaseInsensitive only when nothing above matched: a case-insensitive substring
//                   that names exactly one distinct level; more than one is an
//                   ambiguity error, never a guess.
enum class LevelMatch { ExactId, Substring, CaseInsensitive };

struct ResolvedLevel {
  std::string file;  // archive path, e.g. "COMPILED\\MM_BLOCK_BOMB_A.LVL"
  std::string id;    // the archive file's own id, e.g. "MM_BLOCK_BOMB_A"
  LevelMatch match = LevelMatch::Substring;
};

// The one level-id resolution shared by loadRetailLevel and the application
// (src/app/main.cpp). Archive reads are case-insensitive and patch.box wins over
// lemmings.box, so two spellings of one path are one level.
std::expected<ResolvedLevel, std::string> resolveRetailLevel(const AssetLibrary& lib,
                                                              std::string_view levelId);

}  // namespace webo::sim
