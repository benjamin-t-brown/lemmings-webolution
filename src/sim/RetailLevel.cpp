#include "RetailLevel.h"

#include "assets/Box.h"
#include "assets/CollisionGrid.h"
#include "assets/GameData.h"
#include "assets/LevelIndex.h"
#include "assets/LevelObjects.h"

#include <algorithm>
#include <cmath>
#include <format>

namespace webo::sim {

namespace {

// Same convention SimReleaseTrace.cpp and test_world.cpp already use: the
// archive stores "COMPILED\\MM_CLIMB_002.LVL"; callers pass the level id as
// a substring.
std::string findLevelFile(const webo::AssetLibrary& lib, std::string_view levelId) {
  for (const auto& name : webo::levelFileNames(lib))
    if (name.find(levelId) != std::string::npos)
      return name;
  return {};
}

bool ciEquals(std::string_view a, std::string_view b) {
  return a.size() == b.size() &&
         std::equal(a.begin(), a.end(), b.begin(), [](char x, char y) {
           return std::toupper(static_cast<unsigned char>(x)) ==
                  std::toupper(static_cast<unsigned char>(y));
         });
}

// The matched archive path's own id, not the caller's (possibly partial)
// levelId -- "COMPILED\\MM_CLIMB_002.LVL" -> "MM_CLIMB_002".
std::string idFromFileName(std::string_view name) {
  const auto slash = name.find_last_of("\\/");
  std::string_view base = slash == std::string_view::npos ? name : name.substr(slash + 1);
  constexpr std::string_view kExt = ".LVL";
  if (base.size() > kExt.size())
    base = base.substr(0, base.size() - kExt.size());
  return std::string(base);
}

const webo::LevelInfo* findLevelInfo(const std::vector<webo::LevelInfo>& infos,
                                     std::string_view fileId) {
  for (const auto& info : infos)
    if (ciEquals(info.id, fileId))
      return &info;
  return nullptr;
}

}  // namespace

std::expected<RetailLevelReport, std::string> loadRetailLevelReport(
    const webo::AssetLibrary& lib, std::string_view levelId) {
  const std::string file = findLevelFile(lib, levelId);
  if (file.empty())
    return std::unexpected(std::format("{}: not in the archive", levelId));

  auto blob = lib.read(file);
  if (!blob)
    return std::unexpected(std::format("{}: {}", file, blob.error()));

  auto objs = webo::findLevelObjects(*blob);
  if (!objs)
    return std::unexpected(std::format("{}: {}", file, objs.error()));
  auto grid = webo::findCollisionGrid(*blob);
  if (!grid)
    return std::unexpected(std::format("{}: {}", file, grid.error()));
  auto skills = webo::findLevelSkills(*blob);
  if (!skills)
    return std::unexpected(std::format("{}: {}", file, skills.error()));

  auto infos = webo::loadLevelMap(lib);
  if (!infos)
    return std::unexpected(std::format("{}: LEVELMAP.DAT: {}", file, infos.error()));
  auto rateTable = webo::loadReleaseRateTable(lib);
  if (!rateTable)
    return std::unexpected(std::format("{}: RELEASERATE.PC: {}", file, rateTable.error()));

  const std::string fileId = idFromFileName(file);
  const webo::LevelInfo* info = findLevelInfo(*infos, fileId);
  if (!info)
    return std::unexpected(std::format("{}: no LEVELMAP.DAT row for id {}", file, fileId));

  if (objs->hatches.empty())
    return std::unexpected(std::format("{}: no hatch record (no old/src-style guessed "
                                       "spawn point; see RetailLevel.h)",
                                       file));
  if (objs->exits.empty())
    return std::unexpected(std::format("{}: no Exit*.a3d placement", file));

  RetailLevelReport report;
  LevelSetup& s = report.setup;
  s.terrain = Terrain::fromCollisionGrid(*grid);

  // One Entrance per hatch, in stream order -- World::spawn() already
  // round-robins setup.entrances (World.cpp, unchanged here). Only the
  // first hatch's own rate/min/max drives the one shared release-timing
  // model; see RetailLevelReport and this header's "what is lost" note.
  const int kTicksPerSecond = World::Rules{}.ticksPerSecond;  // MEASURED, World.h
  for (const auto& h : objs->hatches) {
    const auto [cx, cy] = webo::hatchSpawnCell(h, grid->width, grid->height);
    Entrance e;
    e.x = cx;
    e.y = cy;
    e.source = Entrance::Source::HatchWords;
    e.sheetType = h.type;
    s.entrances.push_back(e);

    RetailHatchSummary hs;
    hs.count = h.count;
    hs.x = h.x;
    hs.y = h.y;
    hs.type = h.type;
    hs.rate = h.rate;
    hs.rateMin = h.rateMin;
    hs.rateMax = h.rateMax;
    hs.clampedRate = std::clamp(h.rate, h.rateMin, h.rateMax);
    hs.rateOutsideClamp = h.rate < h.rateMin || h.rate > h.rateMax;
    hs.intervalSeconds = rateTable->secondsPerLemming(hs.clampedRate);
    report.hatches.push_back(hs);
  }

  const RetailHatchSummary& primary = report.hatches.front();
  // D1b (docs/scratch/phase-d1-release-trigger.md): the fixed one-time seed
  // is READ (1.3, at 0x004B0F48); the per-cycle interval is this hatch's own
  // RELEASERATE.PC row; the arm delay is DERIVED on MM_CLIMB_002 only and
  // kept labelled as such -- it is not re-derived per level here, since no
  // other level's arm delay has been measured.
  s.releaseFirstCountdownSeconds = 1.3;
  s.releaseArmSeconds = 0.723061695938398;  // DERIVED, MM_CLIMB_002 only
  s.releaseIntervalSeconds = primary.intervalSeconds;
  s.releaseIntervalTicks =
      rateTable->intervalTicks(primary.clampedRate, kTicksPerSecond);

  // Totals from LEVELMAP.DAT, not summed hatch words: round 1's own corpus
  // probe found sum(hatch.count) == LevelInfo.totalLemmings on all 102
  // retail levels, and the brief's acceptance criterion names LevelInfo as
  // the source.
  s.totalLemmings = static_cast<int>(info->totalLemmings);
  s.saveTarget = static_cast<int>(info->saveTarget);
  s.timeLimitTicks = static_cast<int>(info->timeLimitSeconds) * kTicksPerSecond;

  for (const auto& pose : objs->exits) {
    const ExitTrigger box = webo::exitTriggerFromPose(pose, grid->width, grid->height);
    s.exits.push_back({box.x, box.y, box.width, box.height});
  }

  // Teleports pair file order i <-> i+n/2 -- the existing old/src stand-in
  // (the brief's "Already settled": the original links by stream id at
  // object+0x98, not yet parsed from the .LVL).
  std::vector<TeleportTrigger> telePads;
  for (const auto& p : objs->placed)
    if (p.kind == PlacedKind::Teleport)
      telePads.push_back(webo::teleportTriggerFromPose(p.pose, grid->width, grid->height));
  const std::size_t n = telePads.size();
  if (n >= 2 && (n % 2) == 0) {
    const std::size_t half = n / 2;
    for (std::size_t i = 0; i < n; ++i) {
      const auto& src = telePads[i];
      const auto& dst = telePads[(i + half) % n];
      s.teleports.push_back({src.x, src.y, src.width, src.height, dst.destCol,
                             grid->height - 1 - dst.destWorldY});
    }
  }

  for (const auto& p : objs->placed) {
    if (p.kind != PlacedKind::SpeedUp)
      continue;
    const SpeedPadTrigger box = webo::speedPadTriggerFromPose(p.pose, grid->width, grid->height);
    s.speedPads.push_back({box.x, box.y, box.width, box.height, 0, true});
  }

  for (const auto& p : objs->placed) {
    if (p.kind != PlacedKind::Lever)
      continue;
    const LeverTrigger box = webo::leverTriggerFromPose(p.pose, grid->width, grid->height);
    s.levers.push_back({box.x, box.y, box.width, box.height, 1});
  }

  for (std::size_t i = 0; i < skills->counts.size(); ++i)
    s.skills[i] = static_cast<int>(skills->counts[i]);

  return report;
}

std::expected<LevelSetup, std::string> loadRetailLevel(const webo::AssetLibrary& lib,
                                                        std::string_view levelId) {
  auto report = loadRetailLevelReport(lib, levelId);
  if (!report)
    return std::unexpected(report.error());
  return std::move(report->setup);
}

}  // namespace webo::sim
