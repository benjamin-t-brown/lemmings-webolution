#include "LevelObjects.h"

#include <cmath>
#include <cstring>
#include <format>
#include <string_view>

namespace webo {
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

  if (objs.entrances.empty() && objs.exits.empty())
    return std::unexpected(err.empty() ? "no entrance/exit .a3d placements" : err);
  return objs;
}

std::expected<LevelSkills, std::string> findLevelSkills(std::span<const std::byte> d) {
  // Trailer: [8×u32 skills][u32 pad][u32 1][u32 8]…  The last `1, 8` pair in
  // the file is the marker; counts sit 36 bytes before it.
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

}  // namespace webo
