#include "RetailLevel.h"

#include "assets/Box.h"
#include "assets/CollisionGrid.h"
#include "assets/GameData.h"
#include "assets/LevelIndex.h"
#include "assets/LevelObjects.h"
#include "lemmings/compiled_level.hpp"

#include <algorithm>
#include <array>
#include <bit>
#include <cmath>
#include <cstdint>
#include <format>
#include <span>
#include <stdexcept>

namespace webo::sim {

namespace {

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

// lever-gates-impl-001: serialized lever links, receiver gates and the load-time zone-mask pass.
// Reads the structured gadget records (lemmings::read_compiled_level), builds the base-id table in
// the loader's construction order (a later construction wins, 0x00423310..0x00423325), turns every
// entities_42acac record into a FluidGate and runs the reader's load pass over the terrain that
// was just populated from the file grid (0x0042ACAC runs after 0x0043B640 filled the grid), pairs
// each tagged type-1 record with its placed lever and finally applies each linked lever's stored
// face once, as its state and as its list send (0x00444417 -> 0x0043703F -> 0x00436E6A runs last). Returns a diagnostic when the records cannot be read, in
// which case the level keeps the stored mask and unlinked levers (the pre-existing behaviour).
std::string linkLevelGates(std::span<const std::byte> bytes, const CollisionGrid& grid,
                           LevelSetup& s, std::size_t& pairedLevers) {
  const int maskWidth = grid.width, maskHeight = grid.height;
  lemmings::CompiledLevel level;
  try {
    const std::span<const std::uint8_t> raw(reinterpret_cast<const std::uint8_t*>(bytes.data()),
                                            bytes.size());
    level = lemmings::read_compiled_level(raw, /*require_eof=*/false);
  } catch (const std::invalid_argument& error) {
    return std::format("serialized gadget records: {}", error.what());
  }
  const auto& g = level.gadgets;

  const auto addOther = [&](const auto& v, const char* collection) {
    for (const auto& e : v)
      s.objectIds[e.id] = ObjectRef{ObjectClass::Other, -1, collection};
  };
  // Loader order of 0x004432C1: 0x448650, 0x448710, 0x4487F0, 0x4488F0, 0x4489F0, 0x448AF0,
  // 0x448BD0 (receivers), 0x448CB0, 0x448D90, 0x448E70, then the tagged and named readers.
  addOther(g.basic_entities, "basic_entities");
  addOther(g.linked_entities, "linked_entities");
  addOther(g.entities_425e50, "entities_425e50");
  addOther(g.entities_426c6b, "entities_426c6b");
  addOther(g.spatial_entities, "spatial_entities");
  addOther(g.entities_424187, "entities_424187");
  for (const auto& e : g.entities_42acac) {
    FluidGate gate;
    gate.id = e.id;
    gate.request = static_cast<int>(e.values_18c_128_168[0]);
    gate.kind = static_cast<int>(e.values_18c_128_168[1]);
    gate.minX = std::bit_cast<double>(e.double_bits_48_60[0]);
    gate.minY = std::bit_cast<double>(e.double_bits_48_60[1]);
    gate.maxX = std::bit_cast<double>(e.double_bits_48_60[3]);
    gate.maxY = std::bit_cast<double>(e.double_bits_48_60[4]);
    gate.high = std::bit_cast<double>(e.doubles_98_1a8_a0[0]);
    gate.low = std::bit_cast<double>(e.doubles_98_1a8_a0[2]);
    gate.lane = e.value_16c;  // [+0x16C], wave lane (fluid-surface-impl-001)
    gate.done = 2;  // ctor 0x00428680
    gate.armed = false;
    s.objectIds[e.id] = ObjectRef{ObjectClass::Gate, static_cast<int>(s.gates.size()), "entities_42acac"};
    s.gates.push_back(gate);
  }
  addOther(g.entities_4347ec, "entities_4347ec");
  addOther(g.basic_4342bf, "basic_4342bf");
  addOther(g.spatial_41e038, "spatial_41e038");
  addOther(g.tagged_entities, "tagged_entities");
  addOther(g.named_entities, "named_entities");

  // Reader load pass, in stream order, on the populated terrain.
  for (FluidGate& gate : s.gates)
    gate.initialise(s.terrain);

  // empty-receiver-fall-001: fromCollisionGrid admits every nonzero landable
  // word, including a fluid-zone bit by itself. The reader's load pass above
  // clears the zone plane in a dry receiver, but previously left that baked
  // Earth behind as a permanent lid. Only discard fluid-only words belonging
  // to a registered receiver; retain actual floor, wall and bridge materials.
  // Runtime support comes from World::groundAt's live height, not this mask.
  constexpr auto fluidBits = kCellZoneType0 | kCellZoneType1 | kCellZoneType2;
  for (const FluidGate& gate : s.gates) {
    double span = gate.maxX - gate.minX;
    if (span < 0.0)
      span += maskWidth;
    for (int x = 0; x < maskWidth; ++x) {
      double dx = std::fmod(x - gate.minX, double(maskWidth));
      if (dx < 0.0)
        dx += maskWidth;
      if (dx >= span)
        continue;
      for (int row = 0; row < maskHeight; ++row) {
        if (row < gate.minY || row >= gate.minY + gate.high)
          continue;
        const auto word = grid.at(x, row);
        if ((word & fluidBits) && !(word & ~fluidBits))
          s.terrain.set(x, maskHeight - 1 - row, Material::Empty);
      }
    }
  }

  // Pair each tagged type-1 record (a lever) with its placed lever. The record carries the same
  // pose as the lever.a3d placement; the pairing is by that pose through the production box
  // transform and never by file order. A record that matches no unclaimed lever stays unlinked.
  std::vector<bool> claimed(s.levers.size(), false);
  for (const auto& t : g.tagged_entities) {
    if (t.type != 1)
      continue;
    const double rx = std::bit_cast<double>(t.double_bits[0]);
    const double ry = std::bit_cast<double>(t.double_bits[1]);
    int col = static_cast<int>(std::lround(rx)) % std::max(1, maskWidth);
    if (col < 0)
      col += maskWidth;
    const int boxX = col - 1;
    const int boxY = maskHeight - 1 - (static_cast<int>(ry) + 6);
    for (std::size_t j = 0; j < s.levers.size(); ++j) {
      Lever& lev = s.levers[j];
      if (claimed[j] || lev.x != boxX || lev.y != boxY)
        continue;
      claimed[j] = true;
      ++pairedLevers;
      lev.linked = true;
      lev.serializedId = t.id;
      lev.initialFace = static_cast<std::int32_t>(t.values[0]);
      lev.cooldownSeconds = std::bit_cast<double>(t.double_98);
      const auto toMessages = [](const std::vector<std::array<std::uint32_t, 4>>& records) {
        std::vector<LeverMessage> out;
        for (const auto& r : records)
          out.push_back({static_cast<std::int32_t>(r[0]), r[1], r[2], r[3]});
        return out;
      };
      lev.faceMinus = toMessages(t.record_arrays[0]);  // list +0xBC
      lev.facePlus = toMessages(t.record_arrays[1]);   // list +0xCC
      break;
    }
  }

  // Once per load, after every lever record and receiver exists (lever-initial-face-impl-001):
  // state := stored face and the same face's list is sent. Reload rebuilds the whole LevelSetup.
  fanOutLeverLoadFaces(s);
  return {};
}

}  // namespace

std::expected<ResolvedLevel, std::string> resolveRetailLevel(const webo::AssetLibrary& lib,
                                                              std::string_view levelId) {
  const auto notFound = [&] { return std::format("{}: not in the archive", levelId); };
  if (levelId.empty())
    return std::unexpected(notFound());
  const auto names = webo::levelFileNames(lib);
  const auto resolved = [](const std::string& file, LevelMatch how) {
    return ResolvedLevel{file, idFromFileName(file), how};
  };

  for (const auto& name : names)
    if (ciEquals(idFromFileName(name), levelId))
      return resolved(name, LevelMatch::ExactId);

  // Same convention SimReleaseTrace.cpp and test_world.cpp already use: the
  // archive stores "COMPILED\\MM_CLIMB_002.LVL"; callers pass the level id as
  // a substring. Case-sensitive and first-hit, exactly as before.
  for (const auto& name : names)
    if (name.find(levelId) != std::string::npos)
      return resolved(name, LevelMatch::Substring);

  // The map names some levels in mixed case ("MM_Block_Bomb_A", "m06_kf_1"); the
  // archive reads case-insensitively, so match that way, but only when it names
  // one level (archive spellings differing only in case are one level).
  const std::string wanted = webo::normaliseAssetName(levelId);
  std::vector<const std::string*> hits;
  std::vector<std::string> canonical;
  for (const auto& name : names) {
    std::string key = webo::normaliseAssetName(name);
    if (key.find(wanted) == std::string::npos)
      continue;
    if (std::find(canonical.begin(), canonical.end(), key) != canonical.end())
      continue;
    canonical.push_back(std::move(key));
    hits.push_back(&name);
  }
  if (hits.empty())
    return std::unexpected(notFound());
  if (hits.size() > 1) {
    std::string list;
    for (const std::string* hit : hits)
      list += (list.empty() ? "" : ", ") + idFromFileName(*hit);
    return std::unexpected(std::format("{}: ambiguous level id (matches {})", levelId, list));
  }
  return resolved(*hits.front(), LevelMatch::CaseInsensitive);
}

std::expected<RetailLevelReport, std::string> loadRetailLevelReport(
    const webo::AssetLibrary& lib, std::string_view levelId) {
  const auto resolved = resolveRetailLevel(lib, levelId);
  if (!resolved)
    return std::unexpected(resolved.error());
  const std::string& file = resolved->file;

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

  // Each pad's destination is the pad whose serialized id equals this pad's
  // linked id (+0x98; list lookup at 0x00434BCD, list filled by id at
  // 0x00423325). The ids come from the structured 0x004347EC records,
  // correlated with the located poses by LevelObjects (objs->teleportLinks
  // non-empty). A pad whose linked id no pad carries gets no teleport
  // (diagnostic in LevelObjects).
  std::vector<TeleportTrigger> telePads;
  for (const auto& p : objs->placed)
    if (p.kind == PlacedKind::Teleport)
      telePads.push_back(webo::teleportTriggerFromPose(p.pose, grid->width, grid->height));
  const std::size_t n = telePads.size();
  if (!objs->teleportLinks.empty()) {
    for (std::size_t i = 0; i < n; ++i) {
      const int j = objs->teleportPartner[i];
      if (j < 0)
        continue;
      const auto& src = telePads[i];
      const auto& dst = telePads[static_cast<std::size_t>(j)];
      s.teleports.push_back({src.x, src.y, src.width, src.height, dst.destCol,
                             grid->height - 1 - dst.destWorldY});
    }
  } else if (n >= 2 && (n % 2) == 0) {
    // FALLBACK (objs->teleportLinkDiagnostic says why): the old stand-in,
    // file order i <-> i+n/2, used only when the structured ids are rejected
    // or cannot be correlated with the located pads.
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

  // lever-gates-impl-001: after the terrain, the placed levers and every other object exist.
  report.gateLinkDiagnostic = linkLevelGates(*blob, *grid, s, report.pairedLevers);

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
