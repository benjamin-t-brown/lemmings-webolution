#include "RetailLevel.h"

#include "assets/Box.h"
#include "assets/CollisionGrid.h"
#include "assets/GameData.h"
#include "assets/LevelIndex.h"
#include "assets/LevelObjects.h"
#include "assets/Sprite.h"
#include "lemmings/compiled_level.hpp"

#include <algorithm>
#include <array>
#include <bit>
#include <cmath>
#include <cstdint>
#include <format>
#include <optional>
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

// Extract the archive path's complete id: "COMPILED\\MM_CLIMB_002.LVL" yields "MM_CLIMB_002".
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

// Register gadget ids in construction order, with the last duplicate winning
// (0x00423310..0x00423325). Convert entities_42acac to FluidGate and paint the
// load mask at 0x0042ACAC after the grid loads at 0x0043B640. Pair tagged
// type-1 records by pose, then apply the stored lever faces and send their lists
// (0x00444417 -> 0x0043703F -> 0x00436E6A). Unreadable records leave the
// authored mask and placement-only levers intact.
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

  const auto otherRef = [](const auto& entity, std::size_t index, const char* collection) {
    const auto& bits = entity.double_bits;
    return ObjectRef{ObjectClass::Other, static_cast<int>(index), collection,
                     std::array{std::bit_cast<double>(bits[0]), std::bit_cast<double>(bits[1]),
                                std::bit_cast<double>(bits[2])}};
  };
  const auto addOther = [&](const auto& entities, const char* collection) {
    for (std::size_t i = 0; i < entities.size(); ++i)
      s.objectIds[entities[i].id] = otherRef(entities[i], i, collection);
  };
  // Loader order of 0x004432C1: 0x448650, 0x448710, 0x4487F0, 0x4488F0, 0x4489F0, 0x448AF0,
  // 0x448BD0 (receivers), 0x448CB0, 0x448D90, 0x448E70, then the tagged and named readers.
  addOther(g.basic_entities, "basic_entities");
  addOther(g.linked_entities, "linked_entities");
  addOther(g.entities_425e50, "entities_425e50");
  addOther(g.entities_426c6b, "entities_426c6b");
  addOther(g.spatial_entities, "spatial_entities");
  for (const auto& e : g.entities_424187) {
    CounterGate gate;
    gate.id = e.id;
    const auto v = [&](int i) { return static_cast<std::int32_t>(e.values[i]); };
    gate.left = v(0); gate.right = v(1); gate.top = v(2); gate.bottom = v(3);
    gate.mode = v(4); gate.request = v(5); gate.quota = v(6);
    // Mode 0 ignores the stored counter coordinates and direction (0x423749).
    if (gate.hasCounter()) {
      gate.counterX = v(7); gate.counterY = v(8); gate.direction = v(9);
    }
    gate.position = {std::bit_cast<double>(e.double_bits[0]),
                     maskHeight - 1.0 - std::bit_cast<double>(e.double_bits[1]),
                     std::bit_cast<double>(e.double_bits[2])};
    s.objectIds[e.id] = {ObjectClass::CounterGate, static_cast<int>(s.counterGates.size()),
                         "entities_424187"};
    s.counterGates.push_back(gate);
  }
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
    gate.lane = e.value_16c;  // [+0x16C], wave lane
    gate.done = 2;  // ctor 0x00428680
    gate.armed = false;
    s.objectIds[e.id] = ObjectRef{ObjectClass::Gate, static_cast<int>(s.gates.size()), "entities_42acac"};
    s.gates.push_back(gate);
  }
  addOther(g.entities_4347ec, "entities_4347ec");
  addOther(g.basic_4342bf, "basic_4342bf");
  addOther(g.spatial_41e038, "spatial_41e038");
  // Register tagged ids before named entities can replace them.
  std::size_t sensorIndex = 0;
  for (std::size_t i = 0; i < g.tagged_entities.size(); ++i) {
    const auto& t = g.tagged_entities[i];
    s.objectIds[t.id] = t.type == 2
        ? ObjectRef{ObjectClass::OccupancySensor, static_cast<int>(sensorIndex++), "tagged_entities"}
        : otherRef(t, i, "tagged_entities");
  }
  for (std::size_t entity = 0; entity < g.named_entities.size(); ++entity) {
    const auto& e = g.named_entities[entity];
    s.objectIds[e.id] = otherRef(e, entity, "named_entities");
    for (std::size_t i = 0; i < s.namedHazards.size(); ++i)
      if (s.namedHazards[i].entityIndex == entity)
        s.objectIds[e.id] = {ObjectClass::NamedHazard, int(i), "named_entities"};
  }

  // Reader load pass, in stream order, on the populated terrain.
  for (FluidGate& gate : s.gates)
    gate.initialise(s.terrain);

  // Remove fluid-only cells in registered dry receivers. Real floors, walls
  // and bridges remain solid, and World::groundAt uses the live fluid height
  // for receiver support.
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

  // 0x42421D -> 0x42727C, after reader fluid masks, before lever load-face sends.
  for (auto& gate : s.counterGates) gate.initialise(s.terrain);

  // Type-0 pads use the serialized volume and message list (0x431B30).
  // Pair each tagged type-1 lever with an unclaimed lever.a3d at the same
  // transformed pose. Records without a match stay unlinked.
  std::vector<bool> claimed(s.levers.size(), false);
  for (const auto& t : g.tagged_entities) {
    if (t.type == 2) {
      OccupancySensor sensor;
      sensor.id = t.id;
      sensor.enabled = t.flag_38;
      sensor.state = static_cast<std::int32_t>(t.values[0]);
      sensor.cooldownSeconds = std::bit_cast<double>(t.double_98);
      for (std::size_t axis = 0; axis < 3; ++axis) {
        sensor.min[axis] = std::bit_cast<double>(t.double_bits_48_60[axis]);
        sensor.max[axis] = std::bit_cast<double>(t.double_bits_48_60[axis + 3]);
      }
      for (const auto& r : t.record_arrays[0])
        sensor.occupied.push_back({static_cast<std::int32_t>(r[0]), r[1], r[2], r[3]});
      for (const auto& r : t.record_arrays[1])
        sensor.empty.push_back({static_cast<std::int32_t>(r[0]), r[1], r[2], r[3]});
      s.gadgetContactOrder.push_back({false, s.occupancySensors.size(), true});
      s.occupancySensors.push_back(std::move(sensor));
      continue;
    }
    if (t.type == 0) {
      PressurePad pad;
      pad.id = t.id;
      pad.state = static_cast<std::int32_t>(t.values[0]);
      for (std::size_t axis = 0; axis < 3; ++axis) {
        pad.min[axis] = std::bit_cast<double>(t.double_bits_48_60[axis]);
        pad.max[axis] = std::bit_cast<double>(t.double_bits_48_60[axis + 3]);
      }
      for (const auto& r : t.record_arrays[0])
        pad.messages.push_back({static_cast<std::int32_t>(r[0]), r[1], r[2], r[3]});
      s.gadgetContactOrder.push_back({true, s.pressurePads.size()});
      s.pressurePads.push_back(std::move(pad));
      continue;
    }
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
      s.gadgetContactOrder.push_back({false, j});
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
  // Retain an unpaired lever's placement-only state.
  for (std::size_t j = 0; j < s.levers.size(); ++j)
    if (!claimed[j]) s.gadgetContactOrder.push_back({false, j});

  // After constructing all targets, apply each stored lever face and send its list once.
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

  // Case-sensitive substring fallback takes the first archive hit (e.g. "COMPILED\\MM_CLIMB_002.LVL").
  for (const auto& name : names)
    if (name.find(levelId) != std::string::npos)
      return resolved(name, LevelMatch::Substring);

  // Accept a case-insensitive substring only when it names one distinct level.
  // The map includes mixed-case ids such as "MM_Block_Bomb_A" and "m06_kf_1".
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

  // 4265E8 reads the shared actor then the initial gravity-flip flag.
  // 42FB5F always creates a faller; +48/+49 are climb/float capabilities.
  try {
    const auto decoded = lemmings::read_compiled_level(
        {reinterpret_cast<const std::uint8_t*>(blob->data()), blob->size()}, false);
    if (!decoded.entities.subclass_actors.empty()) {
      const auto sprites = loadSpriteSheet(lib, "SPRITES\\EVILRATHI.SPR");
      if (!sprites) return std::unexpected(file + ": rat sprites: " + sprites.error());
      const auto duration = [&](const char* name) {
        const auto* clip = sprites->find(name);
        if (!clip || clip->animations.empty() || !std::isfinite(clip->rate) || clip->rate <= 0)
          throw std::invalid_argument("missing/invalid rat clip");
        return 0.06 * clip->animations.size() / clip->rate; // 468833
      };
      for (const auto& e : decoded.entities.subclass_actors) {
        EvilRat rat;
        for (unsigned axis = 0; axis < 3; ++axis) {
          rat.initialPosition[axis] = std::bit_cast<double>(e.position_bits[axis]);
          if (!std::isfinite(rat.initialPosition[axis]))
            throw std::invalid_argument("nonfinite rat position");
        }
        if (e.value_4c != 3 || (e.values_40_44[0] != 1 && e.values_40_44[0] != UINT32_MAX) ||
            (e.values_40_44[1] != 1 && e.values_40_44[1] != UINT32_MAX) ||
            rat.initialPosition[0] < 0 || rat.initialPosition[0] >= grid->width ||
            rat.initialPosition[1] < 0 || rat.initialPosition[1] >= grid->height ||
            std::floor(rat.initialPosition[0]) != rat.initialPosition[0] ||
            std::floor(rat.initialPosition[1]) != rat.initialPosition[1])
          throw std::invalid_argument("unsupported rat placement");
        rat.id = e.id;
        rat.storedFlag38 = e.flag_38; // not an actor-update enable bit (426414/42E2BF)
        rat.renderEnabled = e.flag_28;
        rat.actor.x = int(rat.initialPosition[0]);
        rat.actor.y = grid->height - 1 - int(rat.initialPosition[1]);
        rat.actor.facing = e.values_40_44[0] == 1 ? Facing::Right : Facing::Left;
        rat.actor.verticalDir = e.values_40_44[1] == 1 ? 1 : -1;
        rat.actor.sheetType = 3;
        rat.actor.canClimb = e.flags_370_48_49_4a[1];
        rat.actor.canFloat = e.flags_370_48_49_4a[2];
        rat.initialFlip = e.subclass_flag_3a4;
        rat.lane = rat.initialPosition[2];
        constexpr const char* punches[]{"punch0", "punch1", "punch2", "punch3"};
        for (unsigned i = 0; i < 4; ++i) rat.punchDurations[i] = duration(punches[i]);
        rat.openDuration = duration("umbopen");
        s.rats.push_back(rat);
      }
    }
  } catch (const std::invalid_argument& e) {
    return std::unexpected(file + ": rat records: " + e.what());
  }

  const int kTicksPerSecond = World::Rules{}.ticksPerSecond;  // 5 Hz clone clock.
  s.releaseFirstCountdownSeconds = 1.3; // 0x004B0F48.
  s.releaseArmSeconds = 0.723061695938398;  // Provisional for all installed levels (MM_CLIMB_002 basis).
  // Provisional capacity floor: reserve ordinary entry time and two updates
  // for arming and final release. Player adjustments still use stored bounds.
  const double startup = std::max(5.0, s.releaseArmSeconds) + s.releaseFirstCountdownSeconds +
                         2.0 / kTicksPerSecond;
  for (const auto& h : objs->hatches) {
    if (h.rateMin < 1 || h.rateMax > static_cast<int>(rateTable->size()) || h.rateMin > h.rateMax)
      return std::unexpected(file + ": invalid stored hatch rate limits");
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
    hs.initialRate = hs.clampedRate;
    if (objs->hatches.size() > 1 && info->timeLimitSeconds > 0 && h.count > 1) {
      while (hs.initialRate < h.rateMax &&
             startup + (h.count - 1) * rateTable->secondsPerLemming(hs.initialRate) >=
                 info->timeLimitSeconds)
        ++hs.initialRate;
    }
    report.hatches.push_back(hs);

    const auto [cx, cy] = webo::hatchSpawnCell(h, grid->width, grid->height);
    Entrance e;
    e.x = cx;
    e.y = cy;
    e.source = Entrance::Source::HatchWords;
    e.sheetType = h.type;
    e.releaseCount = h.count;
    e.direction = h.direction;
    e.alternateDirection = h.alternateDirection;
    e.releaseRate = hs.initialRate;
    e.releaseIntervalSeconds = rateTable->secondsPerLemming(e.releaseRate);
    e.releaseIntervalTicks = rateTable->intervalTicks(e.releaseRate, kTicksPerSecond);
    s.entrances.push_back(e);
  }

  s.releaseIntervalSeconds = s.entrances.front().releaseIntervalSeconds;
  s.releaseIntervalTicks = s.entrances.front().releaseIntervalTicks;

  // Read the rescue total, save target and time limit from LEVELMAP.DAT.
  s.totalLemmings = static_cast<int>(info->totalLemmings);
  s.saveTarget = static_cast<int>(info->saveTarget);
  s.timeLimitTicks = static_cast<int>(info->timeLimitSeconds) * kTicksPerSecond;

  // 0x426C6B reads tribe from exit +0x98 after the spatial base. Type 4 accepts
  // every tribe. Correlate triggers with records in stream order.
  try {
    const auto decoded = lemmings::read_compiled_level(
        {reinterpret_cast<const std::uint8_t*>(blob->data()), blob->size()}, false);
    const auto& exits = decoded.gadgets.entities_426c6b;
    if (exits.size() != objs->exits.size())
      return std::unexpected(std::format("{}: {} exit records vs {} exit placements", file,
                                         exits.size(), objs->exits.size()));
    for (std::size_t i = 0; i < exits.size(); ++i) {
      const ExitTrigger box = webo::exitTriggerFromPose(objs->exits[i], grid->width, grid->height);
      s.exits.push_back({box.x, box.y, box.width, box.height,
                         static_cast<int>(exits[i].value_98)});
    }
  } catch (const std::invalid_argument& error) {
    return std::unexpected(std::format("{}: serialized exits: {}", file, error.what()));
  }

  // Pair pads using the linked id (+0x98) in the 0x004347EC records correlated
  // with placed poses. 0x00434BCD looks up the id registered at 0x00423325.
  // Missing destination ids leave the source pad unlinked.
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
    // Provisional file-order pairing (i <-> i+n/2) when serialized ids or pose
    // correlation are unavailable. teleportLinkDiagnostic records the reason.
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

  // Apply serialized links after terrain and placed objects exist.
  if (std::any_of(objs->placed.begin(), objs->placed.end(),
                  [](const auto& p) { return p.name == "Crab.a3d"; })) {
    try {
      const auto decoded = lemmings::read_compiled_level(
          {reinterpret_cast<const std::uint8_t*>(blob->data()), blob->size()}, false);
      for (std::size_t i = 0; i < decoded.gadgets.named_entities.size(); ++i) {
        const auto& e = decoded.gadgets.named_entities[i];
        if (!e.model || e.model->name != "Crab.a3d") continue;
        // The stationary STAT branch requires the installed crabs' flag values.
        if (e.type != "STAT" || !e.model->object || e.value_c0 != 5 ||
            !e.flag_1d1 || e.flag_1d2 || e.flag_1d3 ||
            e.triple_arrays[0].empty() ||
            e.triple_arrays[0].size() != e.triple_arrays[1].size())
          return std::unexpected(file + ": unsupported crab behavior/volumes");
        Crab crab;
        crab.entityIndex = i;
        crab.attackSound = e.names[3]; // 0x4333D2: fourth name -> sound +0x1CC
        crab.enabled = e.flag_38;
        for (std::size_t axis = 0; axis < 3; ++axis)
          crab.boxOrigin[axis] = std::bit_cast<double>(e.double_bits_48_60[axis]);
        for (std::size_t j = 0; j < e.triple_arrays[0].size(); ++j) {
          Crab::Volume volume;
          for (std::size_t axis = 0; axis < 3; ++axis) {
            volume.min[axis] = std::bit_cast<double>(e.triple_arrays[0][j][axis]);
            volume.max[axis] = std::bit_cast<double>(e.triple_arrays[1][j][axis]);
            if (!std::isfinite(crab.boxOrigin[axis]) || !std::isfinite(volume.min[axis]) ||
                !std::isfinite(volume.max[axis]) || volume.min[axis] > volume.max[axis])
              return std::unexpected(file + ": invalid crab contact volume");
          }
          crab.volumes.push_back(volume);
        }
        const auto duration = [&](const std::string& name) -> double {
          for (const auto& a : e.model->object->auxiliary_records)
            if (a.name == name) return std::bit_cast<double>(a.leading_double_bits);
          return 0.0;
        };
        crab.restDuration = duration(e.names[0]);  // +0x148, looping
        crab.attackDuration = duration(e.names[1]);  // +0xC8, one shot
        if (!std::isfinite(crab.restDuration) || crab.restDuration <= 0 ||
            !std::isfinite(crab.attackDuration) || crab.attackDuration <= 0)
          return std::unexpected(file + ": crab lacks rest/attack animation durations");
        s.crabs.push_back(std::move(crab));
      }
      const auto placed = std::count_if(objs->placed.begin(), objs->placed.end(),
          [](const auto& p) { return p.name == "Crab.a3d"; });
      if (static_cast<std::size_t>(placed) != s.crabs.size())
        return std::unexpected(file + ": crab placements lack structured records");
    } catch (const std::invalid_argument& e) {
      return std::unexpected(file + ": crab records: " + e.what());
    }
  }

  // Named records supply each hazard's placement and behavior, including its patrol.
  const auto hazardKind = [](std::string_view name) -> std::optional<NamedHazard::Kind> {
    if (name == "deathroller.a3d") return NamedHazard::Kind::Deathroller;
    if (name == "buzzsaw.a3d") return NamedHazard::Kind::Buzzsaw;
    if (name == "Scorp.a3d") return NamedHazard::Kind::Scorpion;
    if (name == "robin.a3d") return NamedHazard::Kind::Robin;
    return {};
  };
  if (std::any_of(objs->placed.begin(), objs->placed.end(),
                  [&](const auto& p) { return hazardKind(p.name).has_value(); })) {
    try {
      const auto level = lemmings::read_compiled_level(
          {reinterpret_cast<const std::uint8_t*>(blob->data()), blob->size()}, false);
      const auto number = [](std::uint64_t bits) {
        const double v = std::bit_cast<double>(bits);
        if (!std::isfinite(v)) throw std::invalid_argument("nonfinite hazard field");
        return v;
      };
      for (std::size_t i = 0; i < level.gadgets.named_entities.size(); ++i) {
        const auto& e = level.gadgets.named_entities[i];
        if (!e.model) continue;
        const auto kind = hazardKind(e.model->name);
        if (!kind) continue;
        if (!e.model->object || (e.type != "STAT" && e.type != "DYNA") ||
            (e.type == "DYNA" && (e.subclass_name != "STAT" || *kind != NamedHazard::Kind::Deathroller)) ||
            e.flag_1d2 || e.flag_1d3 || e.triple_arrays[0].empty() ||
            e.triple_arrays[0].size() != e.triple_arrays[1].size())
          throw std::invalid_argument("unsupported named hazard contract");
        NamedHazard h;
        h.attackSound = e.names[3]; // same named-entity sound field
        h.kind = *kind; h.entityIndex = i; h.id = e.id; h.enabled = e.flag_38;
        h.moving = e.type == "DYNA";
        h.allContacts = e.flag_1d1;
        // Selector 0 (buzzsaw) removes the actor without a particle state;
        // 7, 8 and 14 all select 0x40A6A0 via 0x4331D0.
        h.particles = e.value_c0 == 7 || e.value_c0 == 8 || e.value_c0 == 14;
        if (!h.particles && e.value_c0 != 0) throw std::invalid_argument("unsupported hazard death selector");
        for (unsigned axis=0; axis<3; ++axis) {
          h.initialPosition[axis] = h.position[axis] = number(e.double_bits[axis]);
          h.boxOrigin[axis] = number(e.double_bits_48_60[axis]);
        }
        for (std::size_t j=0; j<e.triple_arrays[0].size(); ++j) {
          Crab::Volume box;
          for (unsigned axis=0; axis<3; ++axis) {
            box.min[axis] = number(e.triple_arrays[0][j][axis]);
            box.max[axis] = number(e.triple_arrays[1][j][axis]);
            if (box.min[axis] > box.max[axis]) throw std::invalid_argument("inverted hazard volume");
          }
          h.volumes.push_back(box);
        }
        const auto duration = [&](const std::string& name) {
          if (name.empty()) return 0.0;
          for (const auto& a : e.model->object->auxiliary_records)
            if (a.name == name) return number(a.leading_double_bits);
          throw std::invalid_argument("missing hazard animation '" + name + "'");
        };
        h.restDuration = duration(e.names[0]); h.attackDuration = duration(e.names[1]);
        if (h.restDuration <= 0 || (!e.names[1].empty() && h.attackDuration <= 0))
          throw std::invalid_argument("invalid hazard clip duration");
        if (h.moving) {
          for (unsigned j=0; j<4; ++j) h.cutOffsets[j] = number(e.dynamic_pairs[j]);
          for (unsigned route=0; route<2; ++route) {
            const auto index = e.dynamic_values[route];
            if (route == 1 && index == UINT32_MAX) continue;
            if (index >= level.entities.groups.size()) throw std::invalid_argument("hazard path outside table");
            const auto& source = level.entities.groups[index];
            auto& dest = h.paths[route]; dest.loop = source.flag;
            dest.junctions.assign(source.integer_values.begin(), source.integer_values.end());
            for (std::size_t k=0; k<source.records.size(); ++k) {
              const auto& r = source.records[k];
              NamedHazard::Segment segment;
              // SHOW_KF3 uses an infinite-duration, zero-velocity endpoint.
              segment.duration = std::bit_cast<double>(r.leading_double_bits); segment.direction = r.value;
              for (unsigned axis=0; axis<2; ++axis) {
                segment.start[axis] = number(r.double_bits[axis]);
                segment.velocity[axis] = number(r.double_bits[axis+2]);
              }
              if (segment.direction > 3 || std::isnan(segment.duration) || segment.duration < 0 ||
                  (std::isinf(segment.duration) && segment.velocity != std::array<double,2>{0,0}) ||
                  (segment.duration == 0 && k+1 != source.records.size()))
                throw std::invalid_argument("invalid hazard path segment");
              dest.segments.push_back(segment);
            }
            if (dest.segments.empty() || (dest.segments.size()>1 &&
                std::none_of(dest.segments.begin(), dest.segments.end(), [](const auto& v) { return v.duration > 0; })))
              throw std::invalid_argument("empty/zero-length hazard loop");
            for (auto junction : dest.junctions) if (junction >= dest.segments.size())
              throw std::invalid_argument("hazard junction outside path");
          }
          h.path = h.requestedPath = e.dynamic_values[2]; h.segment = e.dynamic_values[3];
          h.travel = number(e.subclass_doubles[0]); h.speed = number(e.subclass_doubles[1]);
          h.reverse = e.dynamic_flag_200;
          if (h.path > 1 || h.segment >= h.paths[h.path].segments.size() || h.speed < 0 ||
              (!h.paths[1].segments.empty() && h.paths[0].junctions.size() != h.paths[1].junctions.size()))
            throw std::invalid_argument("invalid hazard patrol state");
        }
        s.namedHazards.push_back(std::move(h));
      }
      const auto placed = std::count_if(objs->placed.begin(), objs->placed.end(),
          [&](const auto& p) { return hazardKind(p.name).has_value(); });
      if (std::size_t(placed) != s.namedHazards.size())
        throw std::invalid_argument("hazard placements lack structured records");
    } catch (const std::invalid_argument& e) {
      return std::unexpected(file + ": named hazards: " + e.what());
    }
  }
  // Named hazard targets must exist before the levers' load-time fan-out.
  report.gateLinkDiagnostic = linkLevelGates(*blob, *grid, s, report.pairedLevers);

  for (std::size_t i = 0; i < skills->counts.size(); ++i)
    s.skills[i] = static_cast<int>(skills->counts[i]);
  // Read command stock from the serialized resource (0x45B86D/0x454725).
  s.nukeStock = 0;
  try {
    const auto decoded = lemmings::read_compiled_level(
        std::span(reinterpret_cast<const std::uint8_t*>(blob->data()), blob->size()), false);
    s.nukeStock = static_cast<int>(std::min<std::uint32_t>(decoded.gadgets.values_27d0[9], 200));
  } catch (const std::invalid_argument&) {
    // Provisional: skill-trailer fallback supplies no command-stock field.
  }

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
