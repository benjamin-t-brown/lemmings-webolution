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
// Skill counts are the first eight words of the ordered eleven-word panel
// record read by 0x0045B86D. The historical locator's nearby `1,8` pair was
// observed across 102 retail levels, not established as a serialized marker.
// That locator remains a diagnosed fallback for rejected structured input.

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
  // That corpus observation came through `findHatchRecords`, which *requires*
  // 8 x 5, so it cannot speak for records of another size; `readHatchRecords`
  // reports whatever the stream holds.
  Vec3 boxMin{};
  Vec3 boxMax{};
  // File offset of the count word, as located by `findHatchRecords`. The
  // structured reader (`readHatchRecords`) supplies no field offset, so it
  // leaves this 0; no consumer reads it.
  std::size_t at = 0;
  // The nearest preceding name from `findHatchRecords` ("entrance" or
  // "entrance_acid"); the record's own model name from `readHatchRecords`,
  // empty when the record carries no model. The two need not be the same
  // string, and nothing keys behaviour off it.
  std::string model;
};

// Serialized link fields of one 0x004347EC record whose model is
// teleport_fx.a3d. `id` is the base id read by 0x00423291 (stored at object
// +0x3A; 0x00423310..0x00423325 then sets actor list 0x004CBB18 [id] = object);
// `partnerId` is the word stored at object +0x98. 0x00434BCD..0x00434BF3 looks
// `partnerId` up in that same list (0x00444BA1 is the generic list[index]
// accessor -> 0x004461D0) and stores the result at +0x260, so the partner is the
// actor whose base id equals `partnerId`. Retained live data agrees on
// MM_TELE_003. `y` is base double [1], kept only to verify the correlation with
// the locator poses.
struct TeleportLink {
  std::int32_t id = 0;
  std::uint32_t partnerId = 0;
  double y = 0;
};

// Project the teleport records, in stream order, through the unchanged
// lemmings::read_compiled_level(bytes, false). Only the reader's
// std::invalid_argument is translated.
std::expected<std::vector<TeleportLink>, std::string> readTeleportLinks(
    std::span<const std::byte>);

// partner[k] = index of the link whose id equals links[k].partnerId; with a
// repeated id the last in stream order (the list slot is overwritten, 0x00423325),
// or -1 when none carries it (the slot is zero, 0x00443ABD). `diagnostic` describes
// every unmatched or repeated id. Ids of non-pad actors share the same list in
// the original and are not visible here (see native-023 notes).
std::vector<int> resolveTeleportPartners(std::span<const TeleportLink> links,
                                         std::string& diagnostic);

struct LevelObjects {
  std::vector<WorldPose> entrances;
  std::vector<WorldPose> exits;
  // Everything else that shares the Object 09 + tube translation layout.
  std::vector<PlacedModel> placed;
  // The hatches' stored spawn words, in stream order.
  std::vector<HatchRecord> hatches;
  // Empty when `hatches` came from the structured level reader. Otherwise the
  // `std::invalid_argument` message that reader threw, with `hatches` from the
  // `findHatchRecords` byte-pattern fallback. Reported, never discarded.
  std::string hatchFallbackReason;
  // native-023: one entry per `placed` teleport.a3d, in locator order, from the
  // structured 0x004347EC records once count and Y were verified against the
  // locator. Empty when the structured projection was rejected or could not be
  // correlated (see teleportLinkDiagnostic) or the level has no teleports.
  std::vector<TeleportLink> teleportLinks;
  // For each teleportLinks entry, the index (into teleportLinks) of the pad it
  // links to, or -1 when no pad carries the linked id.
  std::vector<int> teleportPartner;
  // Empty on structured success (and for levels without teleports). Otherwise
  // the reader's message, or why correlation failed; callers then keep the
  // file-order stand-in. Unresolved/duplicate ids are also described here.
  std::string teleportLinkDiagnostic;
};

// Physical panel indices 0..7 match sim::Skill; see phase-c1-panel-indices.
struct LevelSkills {
  std::array<std::uint32_t, 8> counts{};  // climber … digger
  // Empty on structured success; otherwise the recovered reader's decorated
  // invalid_argument rejection, with counts supplied by the legacy scanner.
  std::string skillFallbackReason;
};

PlacedKind classifyPlacedModel(std::string_view name);

// Find every world-placed `.a3d` (entrances, exits, and the rest).
std::expected<LevelObjects, std::string> findLevelObjects(std::span<const std::byte>);

// Read the eight skill-panel stocks through the recovered ordered level reader.
// Counts above 200 are rejected as portable adapter policy. Only a reader
// invalid_argument enables legacy scanning; combined failure reports both causes.
std::expected<LevelSkills, std::string> findLevelSkills(std::span<const std::byte>);

// Portable serialized entry metadata, not the original hatch/controller ABI.
// 0x00443BDC stores value_5568; 0x00444D1E selects hatch+8 (base yaw),
// defaulting to 128 when the nonnegative index is outside the hatch list.
struct SelectedHatchCamera {
  std::int32_t selector = 0;
  std::size_t hatchCount = 0;
  double yawUnits = 128.0;
};

// Ordered recovered reader, require_eof=false. Negative selectors and nonfinite
// selected yaw are rejected as portable safeguards, not original validation.
// Only reader invalid_argument is translated; caller storage stays borrowed
// throughout parsing. There is no byte-pattern or diagnostic-yaw fallback.
std::expected<SelectedHatchCamera, std::string> readSelectedHatchCamera(
    std::span<const std::byte>);

// Every hatch record in the stream, found by the shape of the run the hatch
// loader reads, not by a remembered offset. See HatchRecord. This is now only
// the fallback for inputs the structured reader below rejects: it is a
// plausibility filter (type <= 8, count in [1,200], x/y < 256, an exactly
// 8 x 5 box, seven integral doubles), so a supported record with a populated
// counted triple array or a differently sized box is invisible to it.
std::vector<HatchRecord> findHatchRecords(std::span<const std::byte>);

// Every hatch record, read through the recovered serialized compiled-level
// reader (`lemmings::read_compiled_level`, gadget list `0x004487F0` ->
// `0x00425E50`) instead of a byte pattern: the whole file is decoded in
// section order and `gadgets.entities_425e50` is projected onto HatchRecord
// in stream order. EOF is not required -- this extracts records, it is not a
// file validator.
//
// Throws `std::invalid_argument` (the decoder's own message) when the input
// is not a compiled level the recovered reader supports; `findLevelObjects`
// catches exactly that and falls back to `findHatchRecords`.
//
// This establishes the serialized *read* layout only. It says nothing about
// the original in-memory hatch object, its allocation, its resource
// resolution or any release timing.
std::vector<HatchRecord> readHatchRecords(std::span<const std::byte>);

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
