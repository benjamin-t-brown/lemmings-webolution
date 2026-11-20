// Load a LevelSetup from the original .LVL, LEVELMAP.DAT and RELEASERATE.PC.
// Missing hatch records or Exit*.a3d placements are load errors.
// Provisional scheduler: each shared tick serves every nonempty hatch.
// Preserve per-hatch quotas and use LevelInfo.totalLemmings for the level total.
// The first hatch supplies the rate and bounds. The time limit can raise the initial rate.

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

// Decoded hatch fields and derived release values. Entrance::releaseCount retains count.
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

// Level data and loader diagnostics.
struct RetailLevelReport {
  LevelSetup setup;
  int initialSharedRate = 1; // Effective shared rate, including the provisional capacity floor.
  // Hatches in stream order. The first supplies the shared rate and bounds.
  std::vector<RetailHatchSummary> hatches;
  // Tagged type-1 levers pair with placements by pose. The diagnostic
  // records why serialized gadget links could not be applied.
  std::size_t pairedLevers = 0;
  std::string gateLinkDiagnostic;
};

// Build terrain from the stored collision grid, place hatches at hatchSpawnCell,
// and read skills from the level trailer. Place Exit*.a3d, teleport.a3d,
// speedup_two_way.a3d and lever.a3d gadgets from their serialized poses.
// Use the id of the matched archive file (MM_CLIMB_002 for
// COMPILED\\MM_CLIMB_002.LVL) for the LEVELMAP.DAT totals and time limit.
// Returns an error for missing levels or unreadable required records.
std::expected<LevelSetup, std::string> loadRetailLevel(const AssetLibrary& lib,
                                                        std::string_view levelId);

// The same build, with the diagnostics `LevelSetup` cannot carry.
std::expected<RetailLevelReport, std::string> loadRetailLevelReport(
    const AssetLibrary& lib, std::string_view levelId);

// Match in order: full id ignoring case, first case-sensitive substring,
// then a case-insensitive substring only if it identifies one distinct level.
// m06_kf_1 resolves to M06_KF_1 (COMPILED\\M06_KF_1.LVL), ahead of M06_KF_10.
// A partial path such as "\\NAME.LVL" can match by substring.
enum class LevelMatch { ExactId, Substring, CaseInsensitive };

struct ResolvedLevel {
  std::string file;  // archive path, e.g. "COMPILED\\MM_BLOCK_BOMB_A.LVL"
  std::string id;    // the archive file's own id, e.g. "MM_BLOCK_BOMB_A"
  LevelMatch match = LevelMatch::Substring;
};

// Loaders and the frontend share this resolver. Archive reads ignore case,
// and patch.box takes precedence over lemmings.box.
std::expected<ResolvedLevel, std::string> resolveRetailLevel(const AssetLibrary& lib,
                                                              std::string_view levelId);

}  // namespace webo::sim
