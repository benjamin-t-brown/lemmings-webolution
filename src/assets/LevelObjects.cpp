#include "LevelObjects.h"

#include "SurfaceMesh.h"

#include "lemmings/compiled_level.hpp"

#include <algorithm>
#include <bit>
#include <cctype>
#include <cmath>
#include <cstring>
#include <format>
#include <stdexcept>
#include <string_view>

namespace webo {

std::expected<SelectedHatchCamera, std::string> readSelectedHatchCamera(
    std::span<const std::byte> bytes) {
  try {
    // The caller's storage backs this view for the entire read; bytes after the
    // complete serialized level are accepted (require_eof=false).
    const std::span<const std::uint8_t> raw(
        reinterpret_cast<const std::uint8_t*>(bytes.data()), bytes.size());
    const lemmings::CompiledLevel level = lemmings::read_compiled_level(raw, /*require_eof=*/false);
    SelectedHatchCamera result;
    result.selector = std::bit_cast<std::int32_t>(level.gadgets.value_5568);
    result.hatchCount = level.gadgets.entities_425e50.size();
    if (result.selector < 0)
      return std::unexpected(std::format("selected-hatch camera: unsupported negative selector {}",
                                         result.selector));
    if (static_cast<std::size_t>(result.selector) < result.hatchCount) {
      // 0x00423291 reads the base triple into hatch+8; 0x00444FEF copies
      // its first double. Spawn words and model rotations are separate data.
      result.yawUnits = std::bit_cast<double>(
          level.gadgets.entities_425e50[result.selector].double_bits[0]);
      if (!std::isfinite(result.yawUnits))
        return std::unexpected("selected-hatch camera: nonfinite selected base yaw");
    }
    return result;
  } catch (const std::invalid_argument& error) {
    return std::unexpected(std::format("selected-hatch camera: {}", error.what()));
  }
}

namespace {

std::uint32_t u32At(std::span<const std::byte> d, std::size_t o) {
  std::uint32_t v;
  std::memcpy(&v, d.data() + o, 4);
  return v;
}

double f64At(std::span<const std::byte> d, std::size_t o) {
  double v;
  std::memcpy(&v, d.data() + o, 8);
  return v;
}

bool isEntranceName(std::string_view name) {
  return name.size() >= 12 && name.starts_with("Entrance") && name.ends_with(".a3d");
}

bool isExitName(std::string_view name) {
  return name.size() >= 8 && name.starts_with("Exit") && name.ends_with(".a3d");
}

// After a length-prefixed `.a3d` name, the next `Object 09` opens the model.
// Four zero pad bytes follow the name, then the double run whose [9],[10],[11]
// are the translation. Non-zero pad is a different Object 09 subtype (fish,
// most lights) and is skipped.
std::expected<WorldPose, std::string> poseAfterA3d(std::span<const std::byte> d,
                                                   std::size_t a3dAt, std::string_view kind) {
  constexpr std::size_t kSearch = 80;
  constexpr std::size_t kPad = 4;
  constexpr std::size_t kDoubles = 12;
  constexpr std::size_t kTx = 9;

  std::size_t obj = std::string::npos;
  const std::size_t limit = std::min(d.size(), a3dAt + kSearch);
  for (std::size_t i = a3dAt; i + 13 <= limit; ++i) {
    if (u32At(d, i) == 9 && std::memcmp(d.data() + i + 4, "Object 09", 9) == 0) {
      obj = i;
      break;
    }
  }
  if (obj == std::string::npos)
    return std::unexpected(std::format("{}: no Object 09 after .a3d", kind));

  if (obj + 4 + 9 + kPad > d.size() || u32At(d, obj + 4 + 9) != 0)
    return std::unexpected(std::format("{}: Object 09 pad is not zero", kind));

  const std::size_t start = obj + 4 + 9 + kPad;
  if (start + kDoubles * 8 > d.size())
    return std::unexpected(std::format("{}: transform run past end of file", kind));

  const double cosY = f64At(d, start + 0 * 8);
  const double sinY = f64At(d, start + 2 * 8);
  const double tx = f64At(d, start + kTx * 8);
  const double ty = f64At(d, start + (kTx + 1) * 8);
  const double tz = f64At(d, start + (kTx + 2) * 8);

  if (!std::isfinite(tx) || !std::isfinite(ty) || !std::isfinite(tz))
    return std::unexpected(std::format("{}: non-finite translation", kind));
  if (!std::isfinite(cosY) || !std::isfinite(sinY))
    return std::unexpected(std::format("{}: non-finite rotation", kind));
  // Reject identity-at-origin FX (teleport_fx) and misaligned subtypes.
  if (std::abs(cosY * cosY + sinY * sinY - 1.0) > 0.05)
    return std::unexpected(std::format("{}: rotation is not a unit yaw", kind));

  const double r = std::hypot(tx, tz);
  // Loose tube check: reject obvious garbage without rejecting the exit's
  // ~65 radius that sits just outside the outer skin.
  if (r < 20.0 || r > 100.0 || ty < -10.0 || ty > 200.0)
    return std::unexpected(std::format("{}: translation off the tube (r={:.1f}, y={:.1f})",
                                       kind, r, ty));

  WorldPose pose;
  pose.position = {tx, ty, tz};
  pose.yaw = std::atan2(sinY, cosY);
  return pose;
}

std::uint16_t u16At(std::span<const std::byte> d, std::size_t o) {
  std::uint16_t v;
  std::memcpy(&v, d.data() + o, 2);
  return v;
}

bool integralDouble(double v) {
  return std::isfinite(v) && v == std::floor(v) && std::abs(v) < 1e6;
}

// The last length-prefixed printable name record ending just before `at`.
// The hatch's is the model name, "entrance" or "entrance_acid"; it is
// reported so a hit can be eyeballed, and is not part of the match.
std::string nameBefore(std::span<const std::byte> d, std::size_t at) {
  constexpr std::size_t kBack = 0x40;
  std::string last;
  std::size_t i = at > kBack ? at - kBack : 0;
  while (i + 4 <= at) {
    const std::uint32_t n = u32At(d, i);
    if (n >= 3 && n <= 48 && i + 4 + n <= at) {
      bool printable = true;
      for (std::size_t k = 0; k < n; ++k) {
        const auto c = static_cast<unsigned char>(d[i + 4 + k]);
        if (c < 32 || c >= 127) {
          printable = false;
          break;
        }
      }
      if (printable) {
        last.assign(reinterpret_cast<const char*>(d.data() + i + 4), n);
        i += 4 + n;
        continue;
      }
    }
    ++i;
  }
  return last;
}

bool looksLikeA3dName(std::span<const std::byte> bytes) {
  if (bytes.size() < 5 || bytes.size() > 40)
    return false;
  if (bytes.size() < 4 ||
      std::memcmp(bytes.data() + bytes.size() - 4, ".a3d", 4) != 0)
    return false;
  for (std::byte b : bytes) {
    const auto c = static_cast<unsigned char>(b);
    if (c < 32 || c >= 127)
      return false;
  }
  return true;
}

}  // namespace

PlacedKind classifyPlacedModel(std::string_view name) {
  if (name == "anti_grav.a3d")
    return PlacedKind::AntiGrav;
  if (name == "teleport.a3d")
    return PlacedKind::Teleport;
  if (name == "pressure_pad.a3d")
    return PlacedKind::PressurePad;
  if (name == "speedup_two_way.a3d")
    return PlacedKind::SpeedUp;
  if (name == "lever.a3d")
    return PlacedKind::Lever;
  if (name == "pipe_drainage.a3d" || name == "pipe_supply.a3d")
    return PlacedKind::Pipe;
  if (name == "deathroller.a3d")
    return PlacedKind::Deathroller;
  if (name == "buzzsaw.a3d")
    return PlacedKind::Buzzsaw;
  if (name == "Crab.a3d" || name == "Scorp.a3d" || name == "robin.a3d")
    return PlacedKind::Creature;
  if (name == "PlaceHolder.a3d" || name.starts_with("light_"))
    return PlacedKind::Scenery;
  return PlacedKind::Other;
}

std::expected<std::vector<TeleportLink>, std::string> readTeleportLinks(
    std::span<const std::byte> bytes) {
  try {
    // The caller's storage backs this view for the whole read; bytes after the
    // complete serialized level are accepted (require_eof=false).
    const std::span<const std::uint8_t> raw(
        reinterpret_cast<const std::uint8_t*>(bytes.data()), bytes.size());
    const lemmings::CompiledLevel level = lemmings::read_compiled_level(raw, /*require_eof=*/false);
    const auto isPadModel = [](const lemmings::LevelModel& m) {
      constexpr std::string_view kName = "teleport_fx.a3d";
      return m.name.size() == kName.size() &&
             std::equal(m.name.begin(), m.name.end(), kName.begin(), [](char a, char b) {
               return std::tolower(static_cast<unsigned char>(a)) == b;
             });
    };
    std::vector<TeleportLink> links;
    for (const auto& e : level.gadgets.entities_4347ec) {
      if (!isPadModel(e.models_1c8_1cc[0]) && !isPadModel(e.models_1c8_1cc[1]))
        continue;
      links.push_back({e.id, e.value_98, std::bit_cast<double>(e.double_bits[1])});
    }
    return links;
  } catch (const std::invalid_argument& error) {
    return std::unexpected(std::format("teleport links: {}", error.what()));
  }
}

std::vector<int> resolveTeleportPartners(std::span<const TeleportLink> links,
                                         std::string& diagnostic) {
  // 0x00423310..0x00423325 stores every constructed actor into list[id] and
  // overwrites an occupied slot, so with a repeated id the last record in stream
  // order is the one 0x00434BE8 finds. A slot nobody filled holds zero
  // (0x00443ABD), i.e. no partner.
  std::vector<int> partner(links.size(), -1);
  for (std::size_t k = 0; k < links.size(); ++k) {
    int found = -1;
    int matches = 0;
    for (std::size_t j = 0; j < links.size(); ++j)
      if (static_cast<std::int64_t>(links[j].id) == static_cast<std::int64_t>(links[k].partnerId)) {
        found = static_cast<int>(j);
        ++matches;
      }
    partner[k] = found;
    if (matches != 1) {
      if (!diagnostic.empty())
        diagnostic += "; ";
      diagnostic += matches == 0
                        ? std::format("teleport pad {} (id {}) links id {}: no pad carries it, no teleport",
                                      k, links[k].id, links[k].partnerId)
                        : std::format("teleport pad {} (id {}) links id {}: {} pads carry it, the last "
                                      "in stream order (pad {}) is used",
                                      k, links[k].id, links[k].partnerId, matches, found);
    }
  }
  return partner;
}

std::expected<LevelObjects, std::string> findLevelObjects(std::span<const std::byte> d) {
  LevelObjects objs;
  std::string err;

  for (std::size_t i = 0; i + 8 <= d.size(); ++i) {
    const std::uint32_t n = u32At(d, i);
    if (n < 5 || n > 40 || i + 4 + n > d.size())
      continue;
    const auto nameBytes = d.subspan(i + 4, n);
    if (!looksLikeA3dName(nameBytes))
      continue;

    const std::string name(reinterpret_cast<const char*>(nameBytes.data()), n);
    auto pose = poseAfterA3d(d, i, name);
    if (!pose) {
      // Nested name references and FX subtypes are common; only keep the
      // first hard failure for the empty-result error path.
      if (err.empty() && (isEntranceName(name) || isExitName(name)))
        err = pose.error();
      i += 3 + n;
      continue;
    }

    if (isEntranceName(name))
      objs.entrances.push_back(*pose);
    else if (isExitName(name))
      objs.exits.push_back(*pose);
    else
      objs.placed.push_back(
          PlacedModel{.name = name, .pose = *pose, .kind = classifyPlacedModel(name)});

    i += 3 + n;
  }

  // Prefer the structured entity list: the byte-pattern locator below can only
  // see records that happen to satisfy its plausibility filter. Only the
  // recovered decoder's own rejection falls back, and its message is kept.
  try {
    objs.hatches = readHatchRecords(d);
  } catch (const std::invalid_argument& error) {
    objs.hatchFallbackReason = error.what();
    objs.hatches = findHatchRecords(d);
  }

  // Teleport links: structured 0x004347EC ids, accepted only when they
  // correlate one-to-one (count and Y, in order) with the located pads.
  {
    std::vector<const PlacedModel*> pads;
    for (const auto& p : objs.placed)
      if (p.kind == PlacedKind::Teleport)
        pads.push_back(&p);
    auto links = readTeleportLinks(d);
    if (!links) {
      if (!pads.empty())
        objs.teleportLinkDiagnostic = links.error();
    } else if (links->size() != pads.size()) {
      objs.teleportLinkDiagnostic = std::format(
          "teleport links: {} structured pads vs {} located teleport.a3d", links->size(), pads.size());
    } else {
      bool ok = true;
      for (std::size_t i = 0; i < pads.size() && ok; ++i)
        if (std::abs((*links)[i].y - pads[i]->pose.position.y) > 0.01) {
          ok = false;
          objs.teleportLinkDiagnostic =
              std::format("teleport links: pad {} Y {} does not match located Y {}", i,
                          (*links)[i].y, pads[i]->pose.position.y);
        }
      if (ok && !links->empty()) {
        objs.teleportLinks = std::move(*links);
        objs.teleportPartner = resolveTeleportPartners(objs.teleportLinks, objs.teleportLinkDiagnostic);
      }
    }
  }

  if (objs.entrances.empty() && objs.exits.empty())
    return std::unexpected(err.empty() ? "no entrance/exit .a3d placements" : err);
  return objs;
}

namespace {

std::expected<LevelSkills, std::string> findLegacyLevelSkills(std::span<const std::byte> d) {
  // Historical locator: the last incidental `1, 8` pair selects eight words
  // starting 36 bytes earlier. This is not a required serialized marker; keep
  // the scanner only for inputs the recovered ordered reader rejects.
  constexpr std::byte kMarker[] = {
      std::byte{1}, std::byte{0}, std::byte{0}, std::byte{0},
      std::byte{8}, std::byte{0}, std::byte{0}, std::byte{0},
  };
  if (d.size() < 36 + sizeof kMarker)
    return std::unexpected("level too short for skill trailer");

  std::size_t at = std::string::npos;
  for (std::size_t i = d.size() - sizeof kMarker; i != static_cast<std::size_t>(-1); --i) {
    if (std::memcmp(d.data() + i, kMarker, sizeof kMarker) == 0) {
      at = i;
      break;
    }
    if (i == 0)
      break;
  }
  if (at == std::string::npos || at < 36)
    return std::unexpected("skill trailer marker not found");

  LevelSkills skills;
  for (std::size_t k = 0; k < skills.counts.size(); ++k) {
    const std::uint32_t n = u32At(d, at - 36 + k * 4);
    if (n > 200)
      return std::unexpected(std::format("implausible skill count {} at slot {}", n, k));
    skills.counts[k] = n;
  }
  return skills;
}

}  // namespace

std::expected<LevelSkills, std::string> findLevelSkills(std::span<const std::byte> d) {
  const std::span<const std::uint8_t> bytes(
      reinterpret_cast<const std::uint8_t*>(d.data()), d.size());
  lemmings::CompiledLevel level;
  try {
    // The caller's storage backs this view for the entire read. As with the
    // hatch adapter, bytes after the complete serialized level are accepted.
    level = lemmings::read_compiled_level(bytes, /*require_eof=*/false);
  } catch (const std::invalid_argument& error) {
    auto legacy = findLegacyLevelSkills(d);
    if (!legacy)
      return std::unexpected(std::format("structured skill reader: {}; legacy skill trailer: {}",
                                         error.what(), legacy.error()));
    legacy->skillFallbackReason = error.what();
    return legacy;
  }

  // 0x0045B86D reads eleven consecutive words, represented by values_27d0.
  // Project only the first eight, in the existing panel/sim order. The other
  // three words get no new meaning here; original panel offsets are not this
  // portable struct's layout. See level_entity_tail and phase-c1bg evidence.
  LevelSkills skills;
  for (std::size_t k = 0; k < skills.counts.size(); ++k) {
    const std::uint32_t n = level.gadgets.values_27d0[k];
    // Retained portable adapter policy, not original-game validation. A
    // decoded invalid count must not be replaced by a legacy scanner result.
    if (n > 200)
      return std::unexpected(std::format("implausible skill count {} at slot {}", n, k));
    skills.counts[k] = n;
  }
  return skills;
}

std::vector<HatchRecord> readHatchRecords(std::span<const std::byte> d) {
  // `0x004432C1` reads the sections in order; the third gadget list call
  // `0x004487F0` reads its records through the hatch reader `0x00425E50`,
  // which is the shared spatial base `0x0043130E` (two triples into
  // hatch+0x48 / +0x60, then two counted triple arrays) followed by the
  // hatch's own fields. `lemmings::read_compiled_level` is that reader,
  // unmodified; this function only projects what it returns.
  //
  // Field mapping, original destination offsets in brackets:
  //   value_b0            [+0xB0] -> type
  //   shorts_98_9a_9c     [+0x98, +0x9A, +0x9C] -> count, x, y
  //   values_a0_a4_a8     [+0xA0, +0xA4, +0xA8] -> rate, rateMin, rateMax
  //   double_bits_48_60   [+0x48] -> boxMin, [+0x60] -> boxMax (bits only)
  //   model->name                 -> model
  // Everything else the record carries (id, flag_28/_38/_40/_41/_ac/_ad/_b6,
  // short_b4, the five base doubles and both counted triple arrays) is
  // deliberately not projected: no evidence establishes what those mean, and
  // guessing one into a gameplay field would invent a rule.
  const std::span<const std::uint8_t> bytes(
      reinterpret_cast<const std::uint8_t*>(d.data()), d.size());
  const lemmings::CompiledLevel level =
      lemmings::read_compiled_level(bytes, /*require_eof=*/false);

  std::vector<HatchRecord> out;
  out.reserve(level.gadgets.entities_425e50.size());
  for (const auto& e : level.gadgets.entities_425e50) {
    HatchRecord h;
    h.count = e.shorts_98_9a_9c[0];
    h.x = e.shorts_98_9a_9c[1];
    h.y = e.shorts_98_9a_9c[2];
    h.type = static_cast<int>(e.value_b0);
    h.rate = static_cast<int>(e.values_a0_a4_a8[0]);
    h.rateMin = static_cast<int>(e.values_a0_a4_a8[1]);
    h.rateMax = static_cast<int>(e.values_a0_a4_a8[2]);
    // The decoder retains binary64 bit patterns; transfer them, do not compute.
    h.boxMin = {std::bit_cast<double>(e.double_bits_48_60[0]),
                std::bit_cast<double>(e.double_bits_48_60[1]),
                std::bit_cast<double>(e.double_bits_48_60[2])};
    h.boxMax = {std::bit_cast<double>(e.double_bits_48_60[3]),
                std::bit_cast<double>(e.double_bits_48_60[4]),
                std::bit_cast<double>(e.double_bits_48_60[5])};
    h.at = 0;  // The structured reader supplies no field offset.
    if (e.model)
      h.model = e.model->name;
    out.push_back(std::move(h));
  }
  return out;
}

std::vector<HatchRecord> findHatchRecords(std::span<const std::byte> d) {
  // The hatch loader `0x00425E50` calls the base loader `0x0043130E` and then
  // reads its own fields out of the stream. Stream primitives, each
  // disassembled with `tools/harness/read_steppers.py --at`:
  //
  //   0x00402540 u8   0x00423360 u16   0x00402560 u32   0x00402580 f64
  //   0x00480BB0 Vec3 (three f64)
  //
  // Read order, the tail of `0x0043130E` then `0x00425E50`:
  //
  //   0x00431328  Vec3 -> hatch+0x48
  //   0x0043133A  Vec3 -> hatch+0x60
  //               f64  (the hatch+0x78 reader)
  //   0x00425EF1  u32  -> hatch+0xB0   lemming type
  //   0x00425F02  u16  -> hatch+0x98   release count
  //   0x00425F14  u16  -> hatch+0x9A   spawn X
  //   0x00425F26  u16  -> hatch+0x9C   spawn Y
  //   0x00425F38  u32  -> hatch+0xA0
  //   0x00425F49  u32  -> hatch+0xA4
  //   0x00425F5A  u32  -> hatch+0xA8
  //   0x00425F6B  u8
  //
  // So the locator looks for that run of seven doubles -- an 8 x 5 box with
  // z 0 then z 1, every component integral -- followed by a small type dword
  // and a plausible triple. Nothing here is an offset carried over from one
  // level to the next.
  //
  // INFERENCE (not established by the disassembly above): the seventh double
  // is not a field at all. The shared spatial base reads the two triples into
  // +0x48 and +0x60 and then two *counted* triple arrays; when both counts
  // are zero their eight bytes read back as the binary64 +0.0 this pattern
  // tests for. That is why the filter below only matches records whose arrays
  // are empty, and why a record with a populated array -- or a box that is
  // not exactly 8 x 5 -- is invisible here and needs `readHatchRecords`.
  constexpr std::size_t kDoubles = 7;
  constexpr std::size_t kTypeAt = kDoubles * 8;
  constexpr std::size_t kTripleAt = kTypeAt + 4;
  constexpr std::size_t kTailAt = kTripleAt + 6;
  constexpr std::size_t kRecordBytes = kTailAt + 12 + 1;

  std::vector<HatchRecord> out;
  if (d.size() < kRecordBytes)
    return out;
  for (std::size_t p = 0; p + kRecordBytes <= d.size(); ++p) {
    // Cheap reject before reading the rest of the run.
    const double x0 = f64At(d, p);
    if (!integralDouble(x0) || x0 < 0.0 || x0 > 4096.0)
      continue;
    double v[kDoubles];
    bool ok = true;
    for (std::size_t k = 0; k < kDoubles; ++k) {
      v[k] = f64At(d, p + k * 8);
      if (!integralDouble(v[k])) {
        ok = false;
        break;
      }
    }
    if (!ok)
      continue;
    if (v[2] != 0.0 || v[5] != 1.0 || v[6] != 0.0)
      continue;
    if (v[3] - v[0] != 8.0 || v[4] - v[1] != 5.0)
      continue;
    const std::uint32_t type = u32At(d, p + kTypeAt);
    if (type > 8)
      continue;
    const int count = u16At(d, p + kTripleAt);
    const int x = u16At(d, p + kTripleAt + 2);
    const int y = u16At(d, p + kTripleAt + 4);
    if (count < 1 || count > 200 || x >= 256 || y >= 256)
      continue;

    // The three tail u32s (hatch+0xA0/+0xA4/+0xA8), right after the
    // count/x/y u16 triple -- see HatchRecord's own comment and D1b
    // (docs/scratch/phase-d1-release-trigger.md).
    const std::uint32_t rate = u32At(d, p + kTailAt);
    const std::uint32_t rateMin = u32At(d, p + kTailAt + 4);
    const std::uint32_t rateMax = u32At(d, p + kTailAt + 8);

    HatchRecord h;
    h.count = count;
    h.x = x;
    h.y = y;
    h.type = static_cast<int>(type);
    h.rate = static_cast<int>(rate);
    h.rateMin = static_cast<int>(rateMin);
    h.rateMax = static_cast<int>(rateMax);
    h.boxMin = {v[0], v[1], v[2]};
    h.boxMax = {v[3], v[4], v[5]};
    h.at = p + kTripleAt;
    h.model = nameBefore(d, p);
    out.push_back(std::move(h));
  }
  return out;
}

std::pair<int, int> hatchSpawnCell(const HatchRecord& h, int maskWidth, int maskHeight) {
  if (maskWidth <= 0 || maskHeight <= 0)
    return {0, 0};
  int x = h.x % maskWidth;
  if (x < 0)
    x += maskWidth;
  // The word is already a file row (0x0049C5AC truncates), and
  // Terrain::fromCollisionGrid flips file row r to height - 1 - r.
  int y = maskHeight - 1 - h.y;
  if (y < 0)
    y = 0;
  if (y >= maskHeight)
    y = maskHeight - 1;
  return {x, y};
}

std::pair<int, int> worldToMaskPixel(const Vec3& p, int maskWidth, int maskHeight,
                                     double worldH, int pixelsPerUnit) {
  if (maskWidth <= 0 || maskHeight <= 0 || pixelsPerUnit < 1 || worldH <= 0)
    return {0, 0};
  const auto [ang, y] = unrollCylinder(p);
  int x = static_cast<int>(std::floor(ang * maskWidth));
  int py = static_cast<int>(std::floor((worldH - y) * pixelsPerUnit));
  // Wrap X; clamp Y into the mask.
  x %= maskWidth;
  if (x < 0)
    x += maskWidth;
  if (py < 0)
    py = 0;
  if (py >= maskHeight)
    py = maskHeight - 1;
  return {x, py};
}

ExitTrigger exitTriggerFromPose(const WorldPose& pose, int maskWidth, int maskHeight) {
  ExitTrigger e;
  if (maskWidth <= 0 || maskHeight <= 0)
    return e;
  const auto [ang, yUp] = unrollCylinder(pose.position);
  // Round 12: live exit AABBs on Exit_water / Exit_any / Exit_acid (and the
  // 46 corpus placements where ang*width sits past .5) centre on
  // round(ang*width), not floor. Round 11's two Exit.a3d dumps both had
  // frac < 0.5, so floor happened to agree. Wrap after rounding so 255.6
  // becomes column 0, not 256.
  int col = static_cast<int>(std::lround(ang * static_cast<double>(maskWidth)));
  col %= maskWidth;
  if (col < 0)
    col += maskWidth;
  // Integer a3d translations truncate to themselves; match hatchSpawnCell's
  // chop rather than worldToMaskPixel's floor(worldH - y), which is one cell
  // off for an integer y and was the wrong axis for this box.
  const int worldY = static_cast<int>(yUp);
  const int worldMinY = worldY - 9;
  const int worldMaxY = worldY - 6;
  e.x = col - 1;
  e.y = maskHeight - 1 - worldMaxY;
  e.width = 2;
  e.height = (maskHeight - 1 - worldMinY) - e.y;  // 3 when the pose is in range
  if (e.height < 0)
    e.height = 0;
  return e;
}

TeleportTrigger teleportTriggerFromPose(const WorldPose& pose, int maskWidth,
                                        int maskHeight) {
  TeleportTrigger t;
  if (maskWidth <= 0 || maskHeight <= 0)
    return t;
  const auto [ang, yUp] = unrollCylinder(pose.position);
  int col = static_cast<int>(std::lround(ang * static_cast<double>(maskWidth)));
  col %= maskWidth;
  if (col < 0)
    col += maskWidth;
  const int worldY = static_cast<int>(yUp);
  // Live AABB on MM_TELE_003: X[col±6] Y[ty .. ty+8], closed.
  t.x = col - 6;
  t.y = maskHeight - 1 - (worldY + 8);
  t.width = 12;
  t.height = 8;
  t.destCol = col;
  t.destWorldY = worldY;
  return t;
}

SpeedPadTrigger speedPadTriggerFromPose(const WorldPose& pose, int maskWidth,
                                        int maskHeight) {
  SpeedPadTrigger t;
  if (maskWidth <= 0 || maskHeight <= 0)
    return t;
  const auto [ang, yUp] = unrollCylinder(pose.position);
  int col = static_cast<int>(std::lround(ang * static_cast<double>(maskWidth)));
  col %= maskWidth;
  if (col < 0)
    col += maskWidth;
  const int worldY = static_cast<int>(yUp);
  // Live AABB on VH21_JC: X[col±1] Y[ty .. ty+6], closed.
  t.x = col - 1;
  t.y = maskHeight - 1 - (worldY + 6);
  t.width = 2;
  t.height = 6;
  return t;
}

LeverTrigger leverTriggerFromPose(const WorldPose& pose, int maskWidth,
                                  int maskHeight) {
  // Same live box shape as SpeedPad (E16_KF_1: X[20..22] Y[13..19] at ty=13).
  LeverTrigger t;
  const SpeedPadTrigger box =
      speedPadTriggerFromPose(pose, maskWidth, maskHeight);
  t.x = box.x;
  t.y = box.y;
  t.width = box.width;
  t.height = box.height;
  return t;
}

}  // namespace webo
