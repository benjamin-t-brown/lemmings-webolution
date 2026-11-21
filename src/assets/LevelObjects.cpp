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
    // Borrow the caller's bytes. Accept trailing data (require_eof=false).
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

// After a counted .a3d name, locate the next Object 09 with four zero pad bytes
// and translation doubles [9],[10],[11]. Skip nonzero-pad subtypes such as fish and most lights.
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
  // Allow exit models just outside the outer shell (radius about 65).
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

// Find the last counted printable name ending before at, e.g. "entrance" or "entrance_acid".
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
    // Borrow the caller's bytes and accept trailing data (require_eof=false).
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
  // 0x00423310..0x00423325 stores actors in list[id], overwriting duplicate IDs.
  // 0x00434BE8 finds the last such actor, or zero when the slot is empty (0x00443ABD).
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
      // Keep the first hard failure for an empty result. Ignore nested reference and FX candidates.
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

  // Use the ordered decoder first. On its rejection, record the cause and try the filtered pattern locator.
  try {
    objs.hatches = readHatchRecords(d);
  } catch (const std::invalid_argument& error) {
    objs.hatchFallbackReason = error.what();
    objs.hatches = findHatchRecords(d);
  }

  // Accept structured 0x004347EC links when count and Y match the located pads in order.
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
  // Fallback only: the last incidental 1, 8 pair selects eight words starting 36 bytes earlier.
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
    // Borrow the caller's bytes and accept trailing data.
    level = lemmings::read_compiled_level(bytes, /*require_eof=*/false);
  } catch (const std::invalid_argument& error) {
    auto legacy = findLegacyLevelSkills(d);
    if (!legacy)
      return std::unexpected(std::format("structured skill reader: {}; legacy skill trailer: {}",
                                         error.what(), legacy.error()));
    legacy->skillFallbackReason = error.what();
    return legacy;
  }

  // 0x0045B86D reads eleven words into values_27d0. Project only the first eight in panel/sim order.
  LevelSkills skills;
  for (std::size_t k = 0; k < skills.counts.size(); ++k) {
    const std::uint32_t n = level.gadgets.values_27d0[k];
    // Clone validation rejects invalid decoded counts without consulting the legacy scanner.
    if (n > 200)
      return std::unexpected(std::format("implausible skill count {} at slot {}", n, k));
    skills.counts[k] = n;
  }
  return skills;
}

std::vector<HatchRecord> readHatchRecords(std::span<const std::byte> d) {
  // 0x004432C1 reads sections in order. The third gadget list 0x004487F0 calls hatch reader
  // 0x00425E50. Its spatial base 0x0043130E reads two triples and two counted triple arrays.
  // lemmings::read_compiled_level supplies these projected fields:
  //   value_b0 [+0xB0] -> type, shorts_98_9a_9c [+0x98, +0x9A, +0x9C] -> count, x, y
  //   values_a0_a4_a8 [+0xA0, +0xA4, +0xA8] -> rate, rateMin, rateMax
  //   double_bits_48_60 [+0x48] -> boxMin, [+0x60] -> boxMax (bits), model->name -> model
  // Both triples are at hatch+0x48 / +0x60.
  //   short_b4 / flag_b6 -> direction / alternateDirection (0x425C6E..0x425CAC)
  // id, flag_28/_38/_40/_41/_ac/_ad, the base doubles and counted arrays are left unprojected.
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
    h.direction = std::bit_cast<std::int16_t>(e.short_b4);
    h.alternateDirection = e.flag_b6;
    // Preserve serialized binary64 bits.
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
  // Legacy hatch locator follows base loader 0x0043130E and hatch loader 0x00425E50:
  // Stream primitives: 0x00402540 u8, 0x00423360 u16, 0x00402560 u32, 0x00402580 f64, 0x00480BB0 Vec3.
  //   0x00431328  Vec3 -> hatch+0x48
  //   0x0043133A  Vec3 -> hatch+0x60
  //   0x00425EF1  u32 -> hatch+0xB0 (type)
  //   0x00425F02 / 0x00425F14 / 0x00425F26  u16 -> hatch+0x98 / hatch+0x9A / hatch+0x9C (count, X, Y)
  //   0x00425F38 / 0x00425F49 / 0x00425F5A  u32 -> hatch+0xA0 / hatch+0xA4 / hatch+0xA8
  //   0x00425F6B  u8
  // The locator requires an integral 8 x 5 box with z = 0 and 1, a zero binary64 at the
  // hatch+0x78 position, then plausible type and spawn words. The zero is inferred to be both
  // empty array counts. readHatchRecords handles other box sizes and populated arrays.
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

    // These u32s at hatch+0xA0/+0xA4/+0xA8 follow the u16 count, X and Y.
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
    // The fallback accepts truncated tails and uses defaults for absent fields.
    if (p + kTailAt + 16 <= d.size()) {
      h.direction = std::bit_cast<std::int16_t>(u16At(d, p + kTailAt + 13));
      h.alternateDirection = d[p + kTailAt + 15] != std::byte{0};
    }
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
  // Exit_water, Exit_any, Exit_acid and Exit.a3d use a rounded angular column.
  // Wrap after rounding so 255.6 maps to column 0 on a 256-cell grid.
  int col = static_cast<int>(std::lround(ang * static_cast<double>(maskWidth)));
  col %= maskWidth;
  if (col < 0)
    col += maskWidth;
  // Truncate world Y as in hatchSpawnCell. worldToMaskPixel floors on the flipped axis.
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
  // MM_TELE_003 original bounds: closed X[col±6], Y[ty .. ty+8].
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
  // VH21_JC original bounds: closed X[col±1], Y[ty .. ty+6].
  t.x = col - 1;
  t.y = maskHeight - 1 - (worldY + 6);
  t.width = 2;
  t.height = 6;
  return t;
}

LeverTrigger leverTriggerFromPose(const WorldPose& pose, int maskWidth,
                                  int maskHeight) {
  // The lever uses SpeedPad bounds (E16_KF_1: X[20..22], Y[13..19] at ty=13).
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
