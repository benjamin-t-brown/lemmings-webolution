// Placed models and the skill-panel inventory inside a .LVL.
//
// Each retail level embeds one or more `.a3d` blocks (entrances, exits,
// hazards, machinery, a little scenery). Right after the length-prefixed
// name comes a nearby `Object 09`, four zero pad bytes, and a run of
// doubles. Bytes 72..95 of that run (indices 9, 10, 11) are the world-space
// translation; the preceding doubles carry a Y-axis rotation (unit cos/sin).
// Theme variants (`Entrance_water.a3d`, `Exit_any.a3d`, …) share the layout.
//
// Across the corpus those points sit on the tube: entrances near the inner
// radius (~50), exits just outside the outer (~65). Radius is discarded when
// the point is unrolled onto the occupancy mask -- the same mapping
// `rasteriseSurface` uses.
//
// A few `.a3d` names use a different Object 09 subtype (non-zero pad after
// the name -- `fish.a3d`, most `light_*.a3d`) or an identity transform at the
// origin (`teleport_fx.a3d`). Those are not world placements and are skipped.
//
// Skill counts live in a short trailer near EOF: eight little-endian u32
// counts (climber … digger), four pad bytes, then the marker
// `01 00 00 00 08 00 00 00`. Verified on all 102 retail levels.

#pragma once

#include "Cylinder.h"
#include "SurfaceMesh.h"

#include <array>
#include <cstddef>
#include <cstdint>
#include <expected>
#include <span>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace webo {

struct WorldPose {
  Vec3 position{};
  // Yaw around +Y, radians, derived from the cos/sin pair ahead of the
  // translation. Recorded for later rendering; the sim does not use it yet.
  double yaw = 0;
};

// Coarse label for a non-entrance/exit placement. Names come from the `.a3d`
// basename; this is a hint for later wiring, not a sim behaviour.
enum class PlacedKind {
  Other,
  AntiGrav,
  Teleport,
  PressurePad,
  SpeedUp,
  Lever,
  Pipe,
  Deathroller,
  Buzzsaw,
  Creature,  // Crab, Scorp, robin
  Scenery,   // PlaceHolder, light_*
};

struct PlacedModel {
  std::string name;  // basename, e.g. "anti_grav.a3d"
  WorldPose pose;
  PlacedKind kind = PlacedKind::Other;
};

struct LevelObjects {
  std::vector<WorldPose> entrances;
  std::vector<WorldPose> exits;
  // Everything else that shares the Object 09 + tube translation layout.
  std::vector<PlacedModel> placed;
};

// Skill panel order matches classic Lemmings / sim::Skill.
struct LevelSkills {
  std::array<std::uint32_t, 8> counts{};  // climber … digger
};

PlacedKind classifyPlacedModel(std::string_view name);

// Find every world-placed `.a3d` (entrances, exits, and the rest).
std::expected<LevelObjects, std::string> findLevelObjects(std::span<const std::byte>);

// Read the eight skill-panel counts from the level trailer.
std::expected<LevelSkills, std::string> findLevelSkills(std::span<const std::byte>);

// Map a world point onto mask / terrain pixels. `pixelsPerUnit` and `worldH`
// must match the values used to build the occupancy mask.
std::pair<int, int> worldToMaskPixel(const Vec3& p, int maskWidth, int maskHeight,
                                     double worldH, int pixelsPerUnit = 2);

}  // namespace webo
