// Placed models and skill stocks in a .LVL. The locator looks for a counted .a3d name,
// a nearby Object 09, four zero pad bytes, and doubles 9..11 (bytes 72..95) for position.
// The preceding cos/sin pair encodes Y rotation. Entrance_water.a3d and Exit_any.a3d share this layout.
// fish.a3d and most light_*.a3d use nonzero pad. teleport_fx.a3d may use identity at the origin.
// The locator skips those cases. rasteriseSurface uses the same unroll, discarding radius.
// Skill stock is the first eight words of the eleven-word record at 0x0045B86D.
// Only rejected ordered input uses the locator's incidental 1,8 pair as a cue.

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
  // Yaw in radians around +Y, from the serialized cos/sin pair.
  double yaw = 0;
};

// Classifies non-entrance/exit placements by .a3d basename.
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

// Serialized hatch release coordinates. Hatch vtable 0x004B0F50 slot 5 (0x00425A95)
// sign-extends int16 fields +0x9A and +0x9C at 0x00425C30 for lemming ctor 0x0042DE51.
// 0x00425E50 loads the words at 0x00425F1C and 0x00425F2E independently of Entrance*.a3d translation.
// Lemming +0x08/+0x10 inherit the spawn coordinates. Example (count, X, Y) tuples: MM_CLIMB_002
// (10, 32, 42), MM_FLOAT (10, 65, 85), MM_DIG_000 (20, 84, 79). spawnCell uses the stored words.
struct HatchRecord {
  int count = 0;  // u16 -> hatch+0x98, this hatch's release count
  int x = 0;      // u16 -> hatch+0x9A, unrolled column; becomes lemming+0x08
  int y = 0;      // u16 -> hatch+0x9C, world Y, Y-up; becomes lemming+0x10
  int type = 0;   // u32 -> hatch+0xB0, the lemming-type index
  // 0x00425DE3 clamps rate to [rateMin, rateMax] and indexes RELEASERATE.PC from 1.
  // webo::ReleaseRates::secondsPerLemming is the interval for that hatch.
  int rate = 0;      // u32 -> hatch+0xA0, preferred RELEASERATE.PC row
  int rateMin = 1;   // u32 -> hatch+0xA4, clamp lower bound
  int rateMax = 99;  // u32 -> hatch+0xA8, clamp upper bound
  // 0x425F8D / 0x425FA8: stored hatch+0xB4 direction and normalized +0xB6.
  // 0x425C6E..0x425CAC uses -1 for left, otherwise right, then alternates
  // this hatch's next direction (1 -> -1, anything else -> 1) when enabled.
  std::int16_t direction = 1;
  bool alternateDirection = false;
  // Base Vec3 corners at hatch+0x48 and +0x60. findHatchRecords requires an 8 x 5 unrolled
  // box with z 0 and 1. readHatchRecords preserves the stored size.
  Vec3 boxMin{};
  Vec3 boxMax{};
  // Count word offset from findHatchRecords. readHatchRecords leaves it zero.
  std::size_t at = 0;
  // findHatchRecords uses the preceding name ("entrance" or "entrance_acid").
  // readHatchRecords uses the serialized model name, empty if absent.
  std::string model;
};

// Links from 0x004347EC teleport_fx.a3d records. 0x00423291 reads id to object +0x3A.
// 0x00423310..0x00423325 assigns 0x004CBB18[id]. 0x00434BCD..0x00434BF3 resolves
// partnerId at object +0x98 via list[index] (0x00444BA1 -> 0x004461D0)
// and stores the actor at +0x260.
// y is base double [1] for matching locator poses, including MM_TELE_003.
struct TeleportLink {
  std::int32_t id = 0;
  std::uint32_t partnerId = 0;
  double y = 0;
};

// Project links in stream order via lemmings::read_compiled_level(bytes, false).
// Translate only the reader's std::invalid_argument.
std::expected<std::vector<TeleportLink>, std::string> readTeleportLinks(
    std::span<const std::byte>);

// partner[k] is the index of the last link whose id matches links[k].partnerId (0x00423325),
// or -1 for an empty slot (0x00443ABD). Report missing and repeated ids.
// Original list slots also hold non-pad actors.
std::vector<int> resolveTeleportPartners(std::span<const TeleportLink> links,
                                         std::string& diagnostic);

struct LevelObjects {
  std::vector<WorldPose> entrances;
  std::vector<WorldPose> exits;
  // Everything else that shares the Object 09 + tube translation layout.
  std::vector<PlacedModel> placed;
  // The hatches' stored spawn words, in stream order.
  std::vector<HatchRecord> hatches;
  // Structured std::invalid_argument message when using the findHatchRecords fallback.
  std::string hatchFallbackReason;
  // One link per placed teleport.a3d, from 0x004347EC records matched by count and Y in order.
  // Empty for absent pads or failed projection/correlation (see teleportLinkDiagnostic).
  std::vector<TeleportLink> teleportLinks;
  // For each teleportLinks entry, the index (into teleportLinks) of the pad it
  // links to, or -1 when no pad carries the linked id.
  std::vector<int> teleportPartner;
  // Reader, correlation or ID diagnostics. Failed correlation leaves the file-order stand-in.
  std::string teleportLinkDiagnostic;
};

// Physical panel indices 0..7 match sim::Skill.
struct LevelSkills {
  std::array<std::uint32_t, 8> counts{};  // climber … digger
  // Empty on structured success; otherwise the recovered reader's decorated
  // invalid_argument rejection, with counts supplied by the legacy scanner.
  std::string skillFallbackReason;
};

PlacedKind classifyPlacedModel(std::string_view name);

// Find every world-placed `.a3d` (entrances, exits, and the rest).
std::expected<LevelObjects, std::string> findLevelObjects(std::span<const std::byte>);

// Read eight stocks with the ordered decoder. Clone validation caps counts at 200.
// Only reader invalid_argument permits fallback. If both fail, retain both causes.
std::expected<LevelSkills, std::string> findLevelSkills(std::span<const std::byte>);

// Serialized entry metadata. 0x00443BDC stores value_5568. 0x00444D1E selects hatch+8
// (base yaw), defaulting to 128 for an out-of-range nonnegative index.
struct SelectedHatchCamera {
  std::int32_t selector = 0;
  std::size_t hatchCount = 0;
  double yawUnits = 128.0;
};

// Ordered read with require_eof=false and borrowed storage. Clone checks reject negative
// selectors and nonfinite selected yaw. Translate only reader invalid_argument. No fallback.
std::expected<SelectedHatchCamera, std::string> readSelectedHatchCamera(
    std::span<const std::byte>);

// Legacy pattern locator after reader rejection. Requires type <= 8, count 1..200,
// X/Y < 256 and seven integral doubles for an 8 x 5 box with empty triple arrays.
std::vector<HatchRecord> findHatchRecords(std::span<const std::byte>);

// lemmings::read_compiled_level projects gadgets.entities_425e50 in stream order
// (gadget reader 0x004487F0 -> 0x00425E50). Trailing bytes are allowed.
// findLevelObjects falls back to findHatchRecords only on std::invalid_argument.
// This extracts serialized fields. Runtime allocation, resource resolution and timing are outside the reader.
std::vector<HatchRecord> readHatchRecords(std::span<const std::byte>);

// Spawn words give the column and world Y in unit cells. 0x0049C5AC truncates to
// a file row with x87 rounding 11b (or ah, 0xc before fistp). Terrain::fromCollisionGrid
// maps row r to height - 1 - r, one row above worldToMaskPixel for integral Y.
// Fresh actors inherit world Y at +0x10 (42 on MM_CLIMB_002).
std::pair<int, int> hatchSpawnCell(const HatchRecord& h, int maskWidth, int maskHeight);

// Match pixelsPerUnit and worldH to the destination. Physics uses one cell per unit
// (assets/CollisionGrid.h). rasteriseSurface defaults to two.
std::pair<int, int> worldToMaskPixel(const Vec3& p, int maskWidth, int maskHeight,
                                     double worldH, int pixelsPerUnit);

// Exit*.a3d trigger in Terrain cells. The original bounds are X[col-1,col+1],
// Y[ty-9,ty-6], Z[0,1], with col = round(unroll(pose).angle * maskWidth) and ty = trunc(pose.y).
// The clone uses height - 1 - worldY to flip Y and stores spans 2 and 3 for sim::Exit / World::atExit.
// Original examples: Exit.a3d (MM_CLIMB_002, MM_FLOAT), Exit_water.a3d (e14_kf_1, MM_W_012),
// Exit_any.a3d (E16_KF_1), Exit_acid.a3d (E23_JC_1).
struct ExitTrigger {
  int x = 0;
  int y = 0;
  int width = 2;
  int height = 3;
};
ExitTrigger exitTriggerFromPose(const WorldPose& pose, int maskWidth, int maskHeight);

// teleport.a3d trigger in Terrain cells. Original MM_TELE_003 (vtable 0x004B1250)
// uses closed X[col-6,col+6], Y[ty,ty+8], Z[0,1]. Round the angular column and truncate world Y.
// Partner is object+0x260 (stream id at +0x98 via 0x00444BA1). teleport_fx.a3d supplies the link.
struct TeleportTrigger {
  int x = 0;
  int y = 0;
  int width = 12;
  int height = 8;
  // Destination feet column / world-up Y (before Y-down convert at load).
  int destCol = 0;
  int destWorldY = 0;
};
TeleportTrigger teleportTriggerFromPose(const WorldPose& pose, int maskWidth,
                                        int maskHeight);

// speedup_two_way.a3d trigger in Terrain cells. On VH21_JC (vtable 0x004B1178),
// original bounds are X[col-1,col+1], Y[ty,ty+6], Z[0,1]. The facing filter at object+0xbc
// and invert flag +0xa0 are unparsed. Clone defaults are upright and any-facing.
struct SpeedPadTrigger {
  int x = 0;
  int y = 0;
  int width = 2;
  int height = 6;
};
SpeedPadTrigger speedPadTriggerFromPose(const WorldPose& pose, int maskWidth,
                                        int maskHeight);

// lever.a3d trigger in Terrain cells. On E16_KF_1 (vtable 0x004B1288), bounds are
// X[col-1,col+1], Y[ty,ty+6], like SpeedPad. Facing is tested against toggle state +0xb8.
struct LeverTrigger {
  int x = 0;
  int y = 0;
  int width = 2;
  int height = 6;
};
LeverTrigger leverTriggerFromPose(const WorldPose& pose, int maskWidth,
                                  int maskHeight);

}  // namespace webo
