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

// A hatch's own record in the level stream. This is where a released
// lemming's first position comes from: the release tick (hatch vtable
// `0x004B0F50` slot 5 -> `0x00425A95`) sign-extends the two int16 at hatch
// `+0x9A` and `+0x9C` and passes them as doubles to the lemming constructor
// `0x0042DE51` (`0x00425C30`). Nothing copies the `Entrance*.a3d`
// translation into those words, and nothing subtracts anything from it:
// the hatch loader `0x00425E50` reads them straight out of the stream at
// `0x00425F1C` and `0x00425F2E`.
//
// MEASURED, three levels, live: `MM_CLIMB_002` (10, 32, 42), `MM_FLOAT`
// (10, 65, 85), `MM_DIG_000` (20, 84, 79) -- the live hatch words and the
// live lemming `+0x08` / `+0x10` of a fresh birth are these numbers. See
// `docs/scratch/phase3-round3.md` and `tools/harness/measure_spawn.py
// --hatch-words`.
//
// OBSERVED over all 144 records in the 102 retail levels, not a rule the
// code may use: `x == boxMin.x + 4` and `y == entrance world Y - 2`.
// `spawnCell` below reads the stored words; it does not reconstruct them.
struct HatchRecord {
  int count = 0;  // u16 -> hatch+0x98, this hatch's release count
  int x = 0;      // u16 -> hatch+0x9A, unrolled column; becomes lemming+0x08
  int y = 0;      // u16 -> hatch+0x9C, world Y, Y-up; becomes lemming+0x10
  int type = 0;   // u32 -> hatch+0xB0, the lemming-type index
  // The three tail u32s. READ, D1b (docs/scratch/phase-d1-release-trigger.md):
  // `0x00425DE3` clamps `rate` into `[rateMin, rateMax]` and uses the result
  // as a 1-based index into the 99-row `RELEASERATE.PC` table
  // (`webo::ReleaseRates::secondsPerLemming`) to get this hatch's own
  // seconds-per-lemming. MEASURED on the full retail corpus (D3a, round 1):
  // of 144 hatch records across all 102 levels, 0 have `rate` outside their
  // own `[rateMin, rateMax]` -- D1b's "this is unchecked" hypothesis holds.
  int rate = 0;      // u32 -> hatch+0xA0, preferred RELEASERATE.PC row
  int rateMin = 1;   // u32 -> hatch+0xA4, clamp lower bound
  int rateMax = 99;  // u32 -> hatch+0xA8, clamp upper bound
  // The two Vec3 the base loader's tail reads into hatch+0x48 and +0x60: an
  // axis-aligned 8 x 5 box in unrolled space, z 0 and 1, on every record in
  // the corpus. Kept because they are what makes the record recognisable.
  Vec3 boxMin{};
  Vec3 boxMax{};
  std::size_t at = 0;       // file offset of the count word
  std::string model;        // "entrance" or "entrance_acid"
};

struct LevelObjects {
  std::vector<WorldPose> entrances;
  std::vector<WorldPose> exits;
  // Everything else that shares the Object 09 + tube translation layout.
  std::vector<PlacedModel> placed;
  // The hatches' stored spawn words, in stream order.
  std::vector<HatchRecord> hatches;
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

// Every hatch record in the stream, found by the shape of the run the hatch
// loader reads, not by a remembered offset. See HatchRecord.
std::vector<HatchRecord> findHatchRecords(std::span<const std::byte>);

// The terrain cell a hatch's stored words name. The words are a world column
// and a world Y measured **up**, in world units -- one cell each, never the
// lattice raster's two.
//
// The grid and Terrain are Y-**down**: Terrain::fromCollisionGrid maps file
// row r to terrain row `height - 1 - r`. The engine turns a world Y into a
// file row by **truncating** it -- its float-to-int helper `0x0049C5AC` sets
// the x87 rounding field to 11b (`or ah, 0xc`, chop) before `fistp` -- so a
// word of 42 is file row 42 and terrain row `height - 1 - 42`.
//
// That is one cell above what `worldToMaskPixel`'s floor(worldH - y) gives
// for an *integer* y, and the words are always integers (42 -> row 42 here,
// row 41 there). What decides it is the chop above plus the *birth* sample:
// the live `+0x10` of a fresh lemming is the word itself -- 0.0013 off 42 on
// MM_CLIMB_002 -- so the release starts at world Y 42, not 41.
//
// The *landing* does not decide it, and must not be cited as if it did: from
// either start row the fall rests on the same floor (feet at world Y 32 / 81
// / 75 on the three levels, the highest solid below being 31 / 80 / 74), so
// the landing is equally consistent with both mappings. Verified against the
// stored grids. See docs/scratch/phase3-round3.md.
std::pair<int, int> hatchSpawnCell(const HatchRecord& h, int maskWidth, int maskHeight);

// Map a world point onto mask / terrain cells. `pixelsPerUnit` and `worldH`
// must match the grid being mapped into, and the two grids in this project
// differ: the stored collision mask (assets/CollisionGrid.h), which is what
// the simulation runs on, is **1** cell per world unit, while the lattice
// raster `rasteriseSurface` produces defaults to 2. There is deliberately no
// default here -- getting it wrong silently halves or doubles every
// coordinate, which is a bug that looks like bad level data.
std::pair<int, int> worldToMaskPixel(const Vec3& p, int maskWidth, int maskHeight,
                                     double worldH, int pixelsPerUnit);

// The save-trigger AABB for one Exit*.a3d placement, in Terrain / mask cells
// (Y-down). MEASURED round 11 on Exit.a3d (MM_CLIMB_002, MM_FLOAT); round 12
// on Exit_water.a3d (e14_kf_1, MM_W_012), Exit_any.a3d (E16_KF_1), and
// Exit_acid.a3d (E23_JC_1) — same 2×3 box for every retail exit model:
//
//   world-up box:  X ∈ [col-1, col+1],  Y ∈ [ty-9, ty-6],  Z ∈ [0, 1]
//   col = round(unroll(pose).angle * maskWidth), ty = trunc(pose.y)
//
// Converted to Terrain Y-down the same way hatchSpawnCell converts a hatch
// word (`height - 1 - worldY`), so the closed interval stays three cells
// tall. width/height are the inclusive spans (2 and 3), matching
// sim::Exit / World::atExit. See docs/scratch/phase3-round11-exit.md and
// docs/scratch/phase3-round12-exit-models.md.
struct ExitTrigger {
  int x = 0;
  int y = 0;
  int width = 2;
  int height = 3;
};
ExitTrigger exitTriggerFromPose(const WorldPose& pose, int maskWidth, int maskHeight);

// The teleport pad AABB for one teleport.a3d placement, in Terrain / mask
// cells (Y-down). MEASURED round 16 on MM_TELE_003 (four live objects,
// vtable 0x004B1250): world-up box X ∈ [col−6, col+6], Y ∈ [ty, ty+8],
// Z ∈ [0, 1]; col = round(unroll(pose).angle * maskWidth), ty = trunc(pose.y).
// Inclusive spans width=12, height=8. Partner is object+0x260 (linked by
// stream id at +0x98 via 0x00444BA1). teleport_fx.a3d is FX-only (no pose).
// See docs/scratch/phase3-round16-teleport.md.
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

// SpeedPad AABB for one speedup_two_way.a3d placement (Terrain Y-down).
// MEASURED round 17 on VH21_JC (vtable 0x004B1178): world-up box
// X ∈ [col−1, col+1], Y ∈ [ty, ty+6], Z ∈ [0, 1]. Inclusive width=2,
// height=6. Facing filter / invert flag live at object+0xbc / +0xa0 are
// not yet parsed from the .LVL — defaults match upright, any-facing pads.
// See docs/scratch/phase3-round17-speedpad.md.
struct SpeedPadTrigger {
  int x = 0;
  int y = 0;
  int width = 2;
  int height = 6;
};
SpeedPadTrigger speedPadTriggerFromPose(const WorldPose& pose, int maskWidth,
                                        int maskHeight);

// Lever AABB for one lever.a3d placement (Terrain Y-down).
// MEASURED round 21 on E16_KF_1 (vtable 0x004B1288): world-up box
// X ∈ [col−1, col+1], Y ∈ [ty, ty+6] — same closed 2×6 as SpeedPad.
// Facing filter is against toggle state +0xb8, not a static pad flag.
// See docs/scratch/phase3-round21-lever.md.
struct LeverTrigger {
  int x = 0;
  int y = 0;
  int width = 2;
  int height = 6;
};
LeverTrigger leverTriggerFromPose(const WorldPose& pose, int maskWidth,
                                  int maskHeight);

}  // namespace webo
