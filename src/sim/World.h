// The simulation: lemmings, the skills applied to them, and the tick that
// moves everything forward.
//
// Deterministic by construction. There is no clock, no randomness and no
// floating point in the step: the same level plus the same ordered list of
// commands always produces the same outcome, which is what makes the
// differential oracle in PLAN.md 2 usable as a regression test.
//
// Most of the constants below are still provisional; three are not.
//
// They started as the well-documented classic-Lemmings values (fall 3px/tick,
// splat above 63px, a six-pixel step-up limit, twelve bricks per builder).
// Revolution is a Lemmings game and those are the right shape, but it is a
// different engine, so each one has to be checked against it. Three have been,
// off the running original through the dinput shim (PLAN.md 2.2): the
// simulation step rate, the walking speed, and the climber's rise. The rest
// are still guesses and are marked as such, one by one, on `Rules`.

#pragma once

#include "Terrain.h"

#include <array>
#include <cstdint>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace webo::sim {

enum class Skill : std::uint8_t {
  Climber, Floater, Bomber, Blocker, Builder, Basher, Miner, Digger,
};
std::string_view skillName(Skill s);

enum class State : std::uint8_t {
  Faller, Walker, Climber, Hoister, Floater, Blocker,
  // Shrugger is where a builder goes when its last brick is spent. MEASURED
  // live, round 6: the twelfth brick's percent-90 callback builds the state
  // whose vtable store is `mov [eax], 0x004B0C78` at 0x0041AB42, which
  // `read_steppers.py --states` names `shrug`, and the lemming walked again
  // 0.966 s later. See Rules::shrugCycleMs.
  Builder, Shrugger, Basher, Miner, Digger, Exploder, Splatter, Exited, Dead,
  // Bounce off a trampoline pad. MEASURED, round 13: vtable 0x004B0810,
  // type 0x21; fall land on cell bit 0x1000 attaches it (0x00416560) ahead
  // of the splat arm. See Rules::bounceRiseCells.
  Bounce,
  // Drown in water (and acid). MEASURED, round 14/15: vtable 0x004B0570,
  // type 0x22; cell bit 0x40 on the ground under the feet attaches it
  // (0x00415386 / 0x004154D8). Enter sets anim "drown" (0x004BF130). Acid
  // Fluid / Entrance_acid are not a separate state â€” same attach. See
  // Rules::drownMs. docs/scratch/phase3-round15-acid.md.
  Drown,
  // In a teleport pad. MEASURED, round 16: type 0x28 (vtable 0x004B0C1C,
  // cartwheel anim) then type 0x2c (0x004B0CE8); object step 0x00434B0F
  // relocates to partner pose. See Rules::teleportDelayTicks.
  Teleporting,
};
std::string_view stateName(State s);

enum class Facing : std::int8_t { Left = -1, Right = 1 };

// Moments the shell can hear. The simulation stays free of SDL and of asset
// names: it only says *what happened*. Mapping each cue onto a `.WAV` is the
// mixer's job. Drained with takeCues() after each tick (or after assign).
enum class SoundCue : std::uint8_t {
  Assign,   // a skill stuck -- Chink
  OhNo,     // a bomber fuse was lit -- Ohno
  Explode,  // the fuse ran out -- Explode
  Splat,    // landed too hard -- Splat
  Exit,     // reached the exit -- Yippee
};

struct Lemming {
  int x = 0;
  int y = 0;               // the lemming's feet
  Facing facing = Facing::Right;
  State state = State::Faller;

  int fallDistance = 0;    // pixels fallen since leaving the ground
  int frame = 0;           // ticks spent in the current state
  int bricksLeft = 0;      // builder
  // The bomber countdown, in MILLISECONDS, because that is what the original
  // keeps: the countdown object at vtable 0x004B0990 holds the double 5.0 and
  // its step subtracts the frame's own time slice (`fsub qword ptr [ecx]` at
  // 0x00413F53). MEASURED live off that double on the lemming's +0x374 list:
  // it runs 4.994 -> 0.133 at exactly 1.000 s per second. -1 means never
  // primed; 0 means the fuse has already run out, which is how a second bomb
  // is refused.
  int fuseMs = -1;

  // SpeedPad FASTER boost. MEASURED round 17: parallel state 0x004B0A38
  // (type 0x10) enter 0x0041688C sets lemming+0x50=2 and a 5.0 s timer;
  // 0x0042E2BF runs the main state twice while +0x50==2. 0 = inactive;
  // >0 counts down in ms like fuseMs. speedPadLatched mirrors pad+0xb8:
  // one attach per visit until the lemming leaves the AABB.
  int fasterMs = 0;
  bool speedPadLatched = false;

  bool canClimb = false;   // climber and floater are permanent, not states
  bool canFloat = false;

  // Vertical direction. MEASURED, round 9: the engine dword at lemming+0x44,
  // +1 upright / -1 inverted. Anti-grav's cartwheel enter (`0x0042F5E3`,
  // called from `0x0041AE1B`) negates it. In THIS sim's Y-down terrain +1
  // means falling increases y (toward the floor) and -1 means falling
  // decreases y (toward the ceiling). Round 30 MEASURED that step-up, climb,
  // hoist and dig all follow this same axis (walk probe add [+0x44], climb
  // `*5.0*dt`, dig `Y -= vdir`, dig mask 0x20 when inverted). See
  // docs/scratch/phase3-round9-antigrav.md and phase3-round30-inverted.md.
  int verticalDir = 1;

  // Lemming sheet index. MEASURED, round 29: hatch+0xB0 copied to
  // [lemming+0x4c] at spawn `0x0042E05F`. Sheet table `0x004BFFC8`:
  // 0 Lemmings, 1 LemmingsWater, 2 LemmingsAcid. Landing splat exemption
  // matches this against zone bits 0x200 / 0x4000 (FUN_00416433).
  // docs/scratch/phase3-round29-splat-exempt.md.
  int sheetType = 0;

  bool alive() const { return state != State::Exited && state != State::Dead; }
  bool active() const { return alive() && state != State::Splatter; }
};

// Where lemmings arrive and leave. Positions are in terrain cells.
struct Entrance {
  int x = 0;
  int y = 0;
  // Which read produced x and y. The original releases from the hatch's own
  // stored int16 words (webo::HatchRecord, `0x00425F1C` / `0x00425F2E`), so a
  // retail load supplies HatchWords and that is what the sim matches. The
  // `Entrance*.a3d` translation is the fallback: hand-built levels, the
  // synthetic setups in main.cpp, and any retail level whose hatch record did
  // not parse. The two are not interchangeable -- on all 144 retail hatch
  // records the stored Y is the translation's world Y minus 2, and that
  // difference is stored, not computed.
  enum class Source : std::uint8_t { EntranceModel, HatchWords };
  Source source = Source::EntranceModel;
  // Hatch type word → [lemming+0x4c]. MEASURED round 29: from HatchRecord.type
  // (hatch+0xB0). 1 = LemmingsWater, 2 = LemmingsAcid.
  int sheetType = 0;
};
// Save trigger. MEASURED, round 11: the original's exit object keeps an AABB
// at +0x48..+0x70 and tests the lemming's raw (X,Y,Z) with 0x00430BA1
// (inclusive on every edge). Live on MM_CLIMB_002 and MM_FLOAT the box is
// exactly 2 wide and 3 tall in world units; Z is the fixed slab [0, 1].
// `x`/`y` are the minimum corner in Terrain space (Y-down); a point is in
// when x âˆˆ [x, x+width] and y âˆˆ [y, y+height] â€” closed, matching the
// original's fcomp, not a half-open rectangle. See
// docs/scratch/phase3-round11-exit.md.
struct Exit { int x = 0; int y = 0; int width = 2; int height = 3; };

// One teleport pad. MEASURED round 16: object vtable 0x004B1250, AABB via
// 0x00430BA1; partner at object+0x260. x/y/width/height are Terrain Y-down
// inclusive spans (12Ã—8). destX/destY are the partner feet cell after the
// snap (partner pose column, terrain row of partner world Y). Facing is
// preserved. docs/scratch/phase3-round16-teleport.md.
struct Teleport {
  int x = 0;
  int y = 0;
  int width = 12;
  int height = 8;
  int destX = 0;
  int destY = 0;
};

// One SpeedPad. MEASURED round 17: speedup_two_way.a3d â†’ object vtable
// 0x004B1178. AABB via 0x00430BA1 inside FUN_00432742; live VH21_JC boxes
// are X âˆˆ [poseÂ±1] Ã— Y âˆˆ [ty .. ty+6] (width=2, height=6, closed). Facing
// filter at object+0xbc (0 = any); +0xa0==0 requires upright (+0x44==1).
// Embed message (âˆ’2,6,1) â†’ FASTER. docs/scratch/phase3-round17-speedpad.md.
struct SpeedPad {
  int x = 0;
  int y = 0;
  int width = 2;
  int height = 6;
  int faceFilter = 0;      // 0 any; else Â±1 must match Facing
  bool requireUpright = true;
};

// One Lever. MEASURED + READ round 21: lever.a3d â†’ object vtable
// 0x004B1288. Detection: list 0x004C88D0 + AABB 0x00430BA1 in 0x00436D30;
// facing filter vs toggle +0xb8 (+0xb8==1 needs face âˆ’1, else face +1).
// Activate 0x00436E14 plays cue Lever (DAT_004E32E4), 0x00436E6A sets
// +0xb8 to the pull facing and fans message list +0xcc (face +1) or +0xbc
// (face âˆ’1) through 0x00433AC0 â†’ FUN_00433B2E (type 4 / type 5 to linked
// object ids). No lemming parallel state. Live E16_KF_1 AABB matches
// SpeedPad's 2Ã—6. docs/scratch/phase3-round21-lever.md.
struct Lever {
  int x = 0;
  int y = 0;
  int width = 2;
  int height = 6;
  // Mirrors object+0xb8. MEASURED E16 start = 1.
  int state = 1;
};

// Balloon cue. READ round 18: string 0x004C0D24 registered to
// object+0x1c8f8 / DAT_004E3300 (FUN_0045e51f); preload 0x0045EB2B;
// unload 0x0045F172. Zero absolute refs to DAT_004E3300; +0x1c8f8 only
// those three sites; no balloon.a3d; SOUNDS\BALLOON.WAV asset-only.
// Exit Models\exit\balloon_* are decorations, not this gadget.
// Clean negative: retail never plays the cue / never attaches an effect.
// docs/scratch/phase3-round18-balloon.md. Mutate-alone: set true.
inline constexpr bool kBalloonGadgetInRetail = false;

// Fire cue. READ round 19: string 0x004C0D2C registered to
// object+0x1c8fc / DAT_004E3304 (FUN_0045e51f); preload 0x0045EB41;
// unload 0x0045F186. Zero absolute refs to DAT_004E3304; +0x1c8fc only
// those three sites; no fire/flame/burn a3d; SOUNDS\FIRE.WAV and
// SPRITES\FIRE.SPR asset-only (no exe/LVL path refs). Mine _FireBraize*
// / lava glow textures are decorations, not this gadget.
// Clean negative: retail never plays the cue / never attaches an effect.
// docs/scratch/phase3-round19-fire.md. Mutate-alone: set true.
inline constexpr bool kFireGadgetInRetail = false;

// Witch cue. READ round 20: string 0x004C0D34 registered to
// object+0x1c8a8 / DAT_004E32B0 (FUN_0045e51f; not sequential +0x1c900);
// preload 0x0045EB57; unload 0x0045F19A. One absolute play ref:
// 0x0045D200 mov ecx,[DAT_004E32B0] â†’ call FUN_0044521b (Action-style),
// inside FUN_0045cff9 pause arm, gated on strcmpi(DAT_004cbbf8,
// "compiled\\e10_kf_1.lvl"). On match: play Witch and skip pause UI.
// No witch.a3d; orphan TEXTURES\MODELS\WITCH\*; SOUNDS\WITCH.WAV for that
// easter egg only. No lemming object/AABB/cell-bit/parallel-state path.
// docs/scratch/phase3-round20-witch.md. Mutate-alone: set true.
inline constexpr bool kWitchGadgetInRetail = false;

// ForceF cue. READ round 22: string 0x004C0CC8 registered to
// object+0x1c8d0 / DAT_004E32D8 (FUN_0045e51f); store disp 0x0045E614;
// preload 0x0045EA39; unload 0x0045F0AA. Zero absolute refs to
// DAT_004E32D8 (whole PE); +0x1c8d0 only those three sites; no
// force/fan/blow a3d; SOUNDS\FORCEF.WAV and SOUNDS\FORCEFLD.WAV
// asset-only (FORCEFLD has no cue string in retail exe).
// Clean negative: retail never plays the cue / never attaches an effect.
// docs/scratch/phase3-round22-forcef.md. Mutate-alone: set true.
inline constexpr bool kForceFGadgetInRetail = false;

// Axe sheet. READ round 23: string 0x004C0D6C registered via sprite
// registrar FUN_00446040 (not SFX FUN_0041d7d0) to object+0x1c914 /
// DAT_004E331C; store disp 0x0045E7D7; preload 0x0045EBEB; unload-family
// 0x0045EEDC / 0x0045EEF7. Sole absolute ref: 0x00496E40 mov eax,
// [DAT_004E331C] â†’ call FUN_00496d5b inside FUN_00496e39 (UI sprite bind
// from FUN_004194f0 / level-UI init FUN_004188f6) â€” not SFX play helpers
// FUN_0044535b / FUN_0044521b. SPRITES\AXE.SPR + TEXTURES\SHEETS\$AXE0.BMP;
// no axe.a3d; no SOUNDS\AXE.WAV. LVL "Axe" hits are embedded name tables.
// No lemming object/AABB/cell-bit/parallel-state path.
// docs/scratch/phase3-round23-axe.md. Mutate-alone: set true.
inline constexpr bool kAxeGadgetInRetail = false;

// Bubble cue. READ round 24: string 0x004C0C9C registered to
// object+0x1c8bc / DAT_004E32C4 (FUN_0045e51f); preload 0x0045E9CB;
// unload 0x0045F046. Absolute play loads 0x00404697 / 0x004046C4 inside
// type-0x23 step FUN_00404372 (vtable 0x004B03C0): lookup FUN_00404cc0 â†’
// channel FUN_00406a00 â†’ SFX FUN_0044535b (height delta >5, one-shot
// +0x1a). Attach: sole ctor FUN_004041fd call at 0x0041C3BA from walker
// FUN_0041bffc when lem+0x54 fluid object mismatches sheet +0x4c /
// fluid+0x128 (or subtype 1). Enter sets anim "drown"; subtype 1 also
// plays Die DAT_004E32DC. Distinct from water-hazard drown type 0x22 /
// 0x004B0570. No bubble.a3d; SOUNDS\BUBBLE.WAV + SPRITES\BUBBLES.SPR
// (FX name Bubbles 0x004BF128, not this cue). No Bubble world gadget â€”
// cue is SFX on that fluid-mismatch state.
// docs/scratch/phase3-round24-bubble.md. Mutate-alone: set true.
inline constexpr bool kBubbleGadgetInRetail = false;

// Scrape cue. READ round 25: string 0x004C0CA4 registered to
// object+0x1c8c0 / DAT_004E32C8 (FUN_0045e51f); preload 0x0045E9E1;
// unload 0x0045F05A. Absolute play loads only 0x0040C139 (bash cell
// helper FUN_0040c0af refuse â†’ FUN_00445274) and 0x00415B33 (dig-bite
// FUN_00415a43 refuse â†’ FUN_00406a00 â†’ FUN_0044535b â†’ walk FUN_0042f8b9).
// Bash bite FUN_0040c357 calls the helper. Dig/bash success play Mine
// DAT_004E32E8; mine refuse plays MineMetal DAT_004E32EC â€” not Scrape.
// No scrape.a3d; SOUNDS\SCRAPE.WAV. Cue is SFX on already-measured
// steel/blocker refuse arms; no exclusive Scrape gadget / state.
// docs/scratch/phase3-round25-scrape.md. Mutate-alone: set true.
inline constexpr bool kScrapeGadgetInRetail = false;

// Rumble cue. READ round 26: string 0x004C0CFC registered to
// object+0x1c8e8 / DAT_004E32F0 (FUN_0045e51f); preload 0x0045EAD3;
// unload 0x0045F122. Sole absolute play load 0x00423823 inside object
// step FUN_00423749 (vtable 0x004B0F00 +0x14): on deferred state commit
// when previous +0xa4 != 0 â†’ FUN_00406a00 â†’ FUN_0044535b. Count helper
// FUN_00423858 walks list 0x004C88D0 (object may open/close via +0x90 /
// +0x58); cue itself attaches no lemming state. Imm32 hits at
// 0x46dfc3 / 0x489453 are call+0x1c8 false positives, not +0x1c8e8.
// No rumble.a3d; SOUNDS\RUMBLE.WAV. No exclusive Rumble gadget.
// docs/scratch/phase3-round26-rumble.md. Mutate-alone: set true.
inline constexpr bool kRumbleGadgetInRetail = false;

// Changeop cue. READ round 27: string 0x004C0CB4 registered to
// object+0x1c8c8 / DAT_004E32D0 (FUN_0045e51f); preload 0x0045EA0D;
// unload 0x0045F082. Absolute play loads only 0x0045AD07 (skill-panel
// select FUN_0045ac5c, gated dword[0x004C6A08]==2 ? FUN_0044521b) and
// 0x0048EB0C (options slider FUN_0048e8d5 / vtable 0x004B24D4 when
// target==this+0x44 ? FUN_00404cc0 ? FUN_00497f39). UI SFX only; no
// actor-list path. No changeop.a3d; SOUNDS\CHANGEOP.WAV (+ orphan
// OLD-CHANGEOP.WAV). No exclusive Changeop gadget.
// docs/scratch/phase3-round27-changeop.md. Mutate-alone: set true.
inline constexpr bool kChangeopGadgetInRetail = false;

// Die cue. READ round 28: string 0x004C0CD0 registered to
// object+0x1c8d4 / DAT_004E32DC (FUN_0045e51f); preload 0x0045EA4F;
// unload 0x0045F0BE. Sole absolute play load imm 0x004042CF
// (mov ecx,[DAT_004E32DC] at 0x004042CD) inside type-0x23 enter
// FUN_00404261 (vtable 0x004B03C0 +8) when fluid+0x128==1 →
// FUN_00406a00 → FUN_0044535b. SFX on already-measured fluid-mismatch
// death (Round 24); not a separate Die state. Distinct from FallSplat
// DAT_004E32E0 (splat enter FUN_0040b3ec / type 0x1a) and from Splat
// DAT_004E32D4 (0 abs play refs). No die.a3d; SOUNDS\DIE.WAV. No
// exclusive Die gadget.
// docs/scratch/phase3-round28-die.md. Mutate-alone: set true.
inline constexpr bool kDieGadgetInRetail = false;

struct LevelSetup {
  Terrain terrain;
  std::vector<Entrance> entrances;
  std::vector<Exit> exits;
  std::vector<Teleport> teleports;
  std::vector<SpeedPad> speedPads;
  std::vector<Lever> levers;
  int totalLemmings = 10;
  int saveTarget = 1;
  int releaseIntervalTicks = 20;  // ticks between spawns
  int timeLimitTicks = 0;         // 0 = untimed
  // Remaining skill-panel counts, indexed like Skill. -1 means unlimited
  // (hand-built test levels). Retail .LVL loads fill these from the trailer.
  std::array<int, 8> skills{-1, -1, -1, -1, -1, -1, -1, -1};
};

class World {
public:
  // How fast things happen. Named so the oracle work has something concrete
  // to correct rather than magic numbers scattered through the code.
  //
  // Each field says whether it was MEASURED off the running original or is
  // still PROVISIONAL. Do not quietly promote a guess: the point of the
  // distinction is that a provisional value can be wrong by a factor of four
  // and nothing in the code will tell you, which is exactly what happened to
  // the tick rate.
  //
  // Two timing facts come from the game's own data rather than from here, and
  // both have now been confirmed live: the spawn interval comes from
  // RELEASERATE.PC via webo::ReleaseRates (its doubles are literal seconds --
  // the original released a lemming every 2.889 s on a level whose table
  // entry is 2.8865 s), and the level limit comes from LEVELMAP.DAT (its
  // u32 is literal seconds -- the original counts it down one per real
  // second, from a static we can read at 0x004CBAAC).
  struct Rules {
    // MEASURED. The original steps its lemmings five times a second, on a
    // flat 200 ms period. It is *not* frame-locked: the same 200 ms holds at
    // 144, 30 and 27 frames per second, so the engine runs the simulation off
    // a real-time accumulator and its render loop is free. See
    // tools/harness/measure_tick.py and PLAN.md 2.2.
    int ticksPerSecond = 5;

    // MEASURED. A walker advances exactly one lattice column per step --
    // 1.000 of the 256 columns MM_CLIMB_002's surface ring is divided into,
    // over many steps on several lemmings, never a fraction of one. Which
    // also means the horizontal mask resolution is the engine's own unit, not
    // a choice of ours. tools/harness/measure_motion.py.
    int walkSpeed = 1;          // mask columns per tick

    // MEASURED. A climber rises one world unit per step. On MM_CLIMB_002 the
    // billboard went 34.190, 35.190, 36.190 â€¦ one per 200 ms tick, up the
    // column-128 wall and again up the next. The terrain is one cell per
    // world unit -- the resolution the original's own stored mask uses
    // (assets/CollisionGrid.h) -- so this is the measured number with no
    // conversion.
    int climbSpeed = 1;         // cells per tick

    // READ FROM THE BINARY, and confirmed live on a walk-off the y = 48 ring
    // (docs/scratch/phase3-round2.md): three steps of exactly âˆ’4.000 world
    // units (48.190 â†’ 44.190 â†’ 40.190 â†’ 36.190). The vtable Round 2 could not
    // catch has since been read off the heap during a fall: objects whose
    // first dword is 0x004B0A20, whose +0x18 climbs one per world unit of
    // descent (docs/scratch/phase3-round3b-float-splat.md). Round 3b also
    // measured the same âˆ’4.000 a tick on two further levels, eighteen
    // consecutive ticks on MM_H_002 and eleven on VH13_KF_1.
    // The lemming's own fall is the state at vtable 0x004B0A20 -- the
    // one that carries the splat test, the floater check and the umbrella.
    // Its speed comes from an accessor, 0x00416038, which branches on
    // the lemming's umbrella flag:
    //
    //   0041604a  dd05180a4b00  fld   qword ptr [0x004b0a18]  ; 20.0
    //   00416050  dc35f8034b00  fdiv  qword ptr [0x004b03f8]  ; 2.0, if open
    //   0041607b  dc4d0c        fmul  qword ptr [ebp+0xc]     ; seconds
    //
    // and the stepper subtracts the result from Y:
    //
    //   00416213  db4044        fild  dword ptr [eax+0x44]    ; direction
    //   00416216  dc4de0        fmul  qword ptr [ebp-0x20]    ; * the distance
    //   00416219  dc6a10        fsubr qword ptr [edx+0x10]    ; Y - that
    //
    // So the constant is world units per *second*: 20.0 * 0.2 s = 4 world
    // units per tick, halving to 2 under the umbrella. The terrain is one
    // cell per world unit, so that is the value here as it stands.
    // The same arithmetic with the climb's 5.0 gives 1 world unit per tick and
    // with the walk's 5.0 gives 1 column per tick, and both of those were
    // measured on the running original -- which is what fixes the units here.
    //
    // An earlier read took 25.0 from the state whose vtable is 0x004B0C60 and
    // put this at 10. The constant itself is at 0x004B0C40 (0x0041A758 does
    // `fmul qword ptr [0x004b0c40]`); 0x004B0C60 is the vtable, and eight
    // bytes read there are 1.96e-307, not a double anyone meant.
    // That state is also a faller by its animation, but it has no splat
    // test, no umbrella and no floater check, so it is some other descent and
    // not this one. PLAN.md 2.3 has both.
    int fallSpeed = 4;          // cells per tick while falling

    // READ FROM THE BINARY AND MEASURED LIVE. Exactly half the fall, because
    // the umbrella path divides by the 2.0 at 0x004B03F8. The flag it branches
    // on is the lemming's [+0x4b], set when the stepper starts the "float"
    // animation at 0x004162C2.
    //
    // Live, on E10_KF_1, a lemming given the floater byte at birth and
    // sampled off the actor list fell -4.000 world units a 200 ms tick down
    // to y = 62.160 and then -2.010 +/- 0.03 for fourteen consecutive ticks
    // (62.160, 60.140, 58.130, 56.120 ...), with [+0x4b] flipping to 1 on the
    // tick the step halved. 20.0 -> 10.0 world units a second.
    // docs/scratch/phase3-round3b-float-splat.md.
    //
    // One thing our stepper does not model, measured the same pass: the
    // halving is not immediate. [+0x4b] is set when the "umbopen" animation
    // *finishes*, about 9.6 world units (0.48 s) after the umbrella opens, so
    // the original keeps falling at 4 for those ticks. Encoding that needs a
    // field this round was told not to invent; it is recorded in the note.
    int floatSpeed = 2;         // cells per tick under an umbrella

    // READ FROM THE BINARY AND MEASURED LIVE. The *fall state*, not the
    // lemming, keeps the distance fallen as a double at its own +0x18 -- the
    // lemming object holds no pointer to that state, which is why a heap scan
    // for the state is how it is found (measure_float_splat.py --find-state).
    // It is incremented by 1.0 per sub-step at 0x00416318 and tested twice:
    //
    //   004162d9  dc1de8034b00  fcomp qword ptr [0x004b03e8]  ; 10.0 -> umbrella
    //   004165ff  dc1da8104b00  fcomp qword ptr [0x004b10a8]  ; 30.0 -> splat
    //
    // Ten and thirty world units, and one cell is one world unit, so these
    // are the read values unchanged. The splat is skipped
    // when the lemming's has-floater byte [+0x49] is set -- which is what our
    // `floating` already stands for, since the umbrella opens well before the
    // splat distance -- and the landing cell's own flags can exempt it too
    // (bit 0x80 is not landable at all). The classic values were 16 and 63.
    //
    // Live: on E10_KF_1 the accumulator read 1, 2, 3 ... 9 at y 81.996 down
    // to 73.936 and the umbrella latch [state+0x20] was already 1 by the
    // sample at accumulator 11, y 71.996 -- a hatch Y of 82, so the umbrella
    // opens exactly 10.000 world units into the fall. The accumulator is
    // floor(distance fallen) + 1, so the 10.0 compare (which reads it before
    // the increment) fires on the boundary at ten units.
    //
    // splatDistance was pinned by holding that accumulator: with it held at
    // 29 the lemmings landing from E10_KF_1's 75-unit drop stayed in the
    // actor list and walked (1 death in the first 12 s), and with it held at
    // 31 they died at the untouched rate (13 in the first 12 s, 27 in 24 s).
    // `test ah, 0x41` at 0x00416607 survives on <= 30.0, so the branch's
    // threshold is 30 and nothing else. Unprobed confirmation on a second
    // level: VH13_KF_1's hatch drop is 28 world units, the accumulator ran
    // 1..28, and none of ten lemmings left the actor list on landing.
    //
    // Round 29 MEASURED: after the floater byte, FUN_00416433 also skips
    // splat when (cell & 0x200 && [+0x4c]==1) or (cell & 0x4000 &&
    // [+0x4c]==2). Terrain::splatExempt / Lemming::sheetType. Lava zone
    // 0x2000 has no arm. docs/scratch/phase3-round29-splat-exempt.md.
    int floatAfter = 10;        // umbrella opens this far into the fall
    int splatDistance = 30;     // falling further than this kills

    // PROVISIONAL from here down, with these exceptions, each of which
    // carries its own addresses and series below: `maxStepUp` and
    // `lemmingHeight` (read and measured live in round 4), the whole
    // bash / dig / mine block (round 5), the whole builder block (round 6),
    // `blockerFieldHeight` (round 8) and the whole bomber block (round 7).
    // `basherReach` and the two miner numbers are what is left.
    //
    // ---- bash, dig and mine: MEASURED LIVE, round 5 --------------------
    //
    // "An interval in ticks" is the wrong shape and the three fields that
    // had it are gone. All three skills share slot-1 stepper 0x0041B077,
    // which clears no cell itself: it turns the animation's progress into an
    // integer percent (`fld [0x004B0CE0]` = 100.0 at 0x0041B1A9, truncated
    // at 0x0049C5AC) and, for each percent it crosses, calls the record whose
    // stored frame EQUALS that percent (`cmp eax, [ebp-8]` at 0x0041B21E,
    // `call [eax+0xc]` at 0x0041B248). The removal is in those records.
    //
    // The clock is the animation, not the tick. 0x00468B04 multiplies the
    // strip's frame count by the double 0.06 at 0x004B1940, 0x00468B5D
    // multiplies the current frame index by the same 0.06, and the strip's
    // own divisor at [strip+0x10] cancels in the ratio -- so one whole cycle
    // is `frames * 0.06` seconds. Read live off the controller at
    // lemming+0x128 (frames at [[+0x1F8]+0xC] + [+0x1C8] * 0x98, divisor
    // 1.0 on all three): bash strip 0 is 24 frames, dig strip 4 is 20, mine
    // strip 9 is 25. Live cycle times, measured off the grid writes:
    //
    //   dig   20 x 0.06 = 1.200 s   measured 1.2005 / 1.2017 / 1.1990 s
    //   bash  24 x 0.06 = 1.440 s   measured 1.4380 / 1.4370 / 1.4450 s
    //   mine  25 x 0.06 = 1.500 s   measured 1.4997 / 1.5010 / 1.5005 s
    //
    // At the measured 200 ms tick that is 6.000, 7.200 and 7.500 ticks. Only
    // dig is a whole number of ticks, which is why these are milliseconds
    // and not an interval: `World::stepBasher` samples the real cycle and so
    // acts every 7 or 8 ticks, averaging 7.2, instead of rounding to one of
    // them. The 0.75 dig's enter hook writes to controller +0x1D8
    // (0x004159CA into 0x004683E9) is NOT this cycle's divisor -- the strip
    // itself is 20 frames and 20 x 0.06 is the measured 1.2 s, so whatever
    // that rate does, it does not scale the cycle.
    //
    // docs/scratch/phase3-round5-live.md has the per-tick grid diffs;
    // tools/harness/measure_dig.py re-runs them.
    //
    // FUN_0041B683, the four-cell `and 0xf77c` removal at 0x0041B783, is
    // still nobody's and is now positively excluded: it is slot 3 of the
    // trampoline vtable 0x004B0D08 with no direct caller in .text, and every
    // live removal measured this round stored a **whole dword 0** over the
    // cell (0x21 -> 0x00000000, 0x5 -> 0x00000000, 0x1 -> 0x00000000, 38 of
    // 38 cells), which is `mov dword ptr [edx], 0` in the bites and not an
    // AND of any mask.
    //
    // The distances below were classic-Lemmings *pixel* values carried at the
    // two-rows-per-world-unit resolution our old raster used. The terrain is
    // now one cell per world unit, matching the original's stored mask, so
    // each has been halved to keep the physical size it was standing in for.
    // Halving a guess still leaves a guess: these are no better founded than
    // before, only expressed in the right unit. `maxStepUp` and
    // `lemmingHeight` were on that halved-guess list until round 4, and
    // `basherReach` and `diggerDepth` until round 5; none of them is any
    // more, and the halving had nothing to do with any of the four numbers.
    //
    // READ FROM THE BINARY AND MEASURED LIVE, and it is 2, not the classic 3.
    // The walk stepper 0x0041BDD4 loads no step height -- its only doubles
    // are 2.0, 0.0 and 5.0 -- because the step-up lives in the mask probe
    // 0x0041BFFC that it calls at 0x0041BEE6. That probe's step-up arm is
    // unrolled exactly two deep and the third solid cell is a wall:
    //
    //   0041c47d  83e201    and edx, 1            ; cell(X, Y)   & 1
    //   0041c482  0f8467..  je  0x41c6ef          ;   clear -> the step-DOWN arm
    //   0041c48e  034844    add ecx, [eax+0x44]   ; row += vertical direction
    //   0041c517  83e101    and ecx, 1            ; cell(X, Y+1) & 1
    //   0041c51c  0f848c..  je  0x41c6ae          ;   clear -> adopt (rise 1)
    //   0041c528  034244    add eax, [edx+0x44]   ; row += direction again
    //   0041c5b1  83e001    and eax, 1            ; cell(X, Y+2) & 1
    //   0041c5b6  0f84f2..  je  0x41c6ae          ;   clear -> adopt (rise 2)
    //   0041c5c1  8a5148    mov dl, [ecx+0x48]    ; third solid cell: climber?
    //   0041c5c6  0f84aa..  je  0x41c676          ;   no -> 0x42f66b, turn round
    //
    // 0x0041C6AE is the adopt path (it stores the row through 0x0042EE74).
    // There is no third `add`: over the whole function to its `ret 4` at
    // 0x0041C872, `add ..., [reg+0x44]` appears only at 0x0041C48E,
    // 0x0041C528 and 0x0041C81B (that last one is on the step-down arm).
    // [+0x44] is the integer 1 written at spawn by 0x0042DFE7 and 0x0042F5E3
    // only ever negates it, so the greatest rise the probe can write is 2.
    //
    // Live, by writing single-column pillars of exactly N solid cells into
    // the original's own grid in front of its own walkers (the bit tested is
    // 0x1, so that is the bit written) and watching what its arm does --
    // tools/harness/measure_stepup.py --carve. On VH13_MT, pillars of 1, 2,
    // 3, 4 at columns 29, 33, 37, 41 above feet row 59: **230 adopted rises
    // of exactly 1, 231 of exactly 2 (in a single step, dy +2.000), 114
    // turns at the pillar of 3, and nothing ever reached the pillar of 4**.
    // Repeated on E07_JC_1 (pillars at 216, 224, 232, 240 above feet row 51):
    // rise 1, rise 2, then 14 lemmings turned at the 3. Both bounds, two
    // levels. docs/scratch/phase3-round4-live.md.
    int maxStepUp = 2;          // a walker climbs steps up to this high

    // ---- the builder: MEASURED LIVE, round 6 ---------------------------
    //
    // The builder is on the same record mechanism as bash / dig / mine. Its
    // state vtable is 0x004B08DC (`mov [edx], 0x4b08dc` at 0x0040F93F), its
    // step is slot +4 = 0x00410302 whose tail is `call 0x41b077` at
    // 0x0041047B, and its constructor 0x0040F8D1 registers exactly TWO
    // records into the list at state+0x18:
    //
    //   0040f9a8  c7 40 08 51 00 00 00  mov [eax+8], 0x51      ; frame 81
    //   0040f9b2  c7 41 0c 1c 01 41 00  mov [ecx+0xc], 0x41011c    ; the brick
    //   0040fa06  c7 42 08 5a 00 00 00  mov [edx+8], 0x5a      ; frame 90
    //   0040fa10  c7 40 0c cc fb 40 00  mov [eax+0xc], 0x40fbcc    ; advance
    //
    // read out of the running process, entry for entry, on all three builders
    // watched. So there is no `builderInterval` and that field is gone: one
    // cycle is the build strip's `frames * 0.06 / divisor`, which is strip 2,
    // 16 frames, divisor 1.0 = 960 ms. Measured off the grid writes:
    // 33 gaps across three builders, every one between 0.955 and 0.969 s,
    // mean 0.9597 s = 4.80 ticks. Not a whole number of ticks, like bash and
    // mine.
    //
    // THE BRICK is 0x0041011C, and it is TWO cells, not three:
    //
    //   00410199  c7 45 f0 01 00 00 00  mov [ebp-0x10], 1   ; i = 1
    //   004101ab  83 7d f0 03           cmp [ebp-0x10], 3   ; while i < 3
    //   004101b7  0f af 42 40           imul eax, [edx+0x40]    ; i * facing
    //   004101eb  e8 70 dc ff ff        call 0x40de60           ; the cell
    //   004101f8  83 ca 01              or  edx, 1              ; THE BRICK
    //   004101fe  89 10                 mov dword ptr [eax], edx
    //
    // at `(p + i*facing, trunc(Y))` for i = 1 and 2, where p is the stamp's
    // own column `trunc(X + [+0x360])` -- and the enter hook's snap cancels
    // the facing bias exactly as the bash bite's does (facing left it adds 1
    // to the truncation at 0x0040FB0B and `[+0x360]` is -1.0), so p is
    // `trunc(X at the click)` in both facings. The row is the lemming's OWN
    // row, not the row below it: a builder at world Y 23 writes row 23 and
    // then stands at 24.
    //
    // Live, three builders on MM_BUILD in three separate processes, both
    // facings, 12 bricks each: **72 of 72 cells went 0x00000000 ->
    // 0x00000001**, at animation percent **81** every time.
    //
    //   left-facing  (185.147, 23) -> (183,23)(184,23) (182,24)(183,24) ...
    //                                 (172,34)(173,34)
    //   right-facing (159.845, 23) -> (160,23)(161,23) (161,24)(162,24) ...
    //                                 (171,34)(172,34)
    //   left-facing  (233.966, 49) -> (231,49)(232,49) ... (220,60)(221,60)
    //
    // So a brick is plain solid. It is NOT the `or edx, 0x403` at 0x004275C1
    // -- that function 0x00427554 has three E8 callers (0x004271D5,
    // 0x00427509, 0x0043B1F8) and the builder's chain 0x0041011C ->
    // 0x0043B016 -> 0x0043AE6C reaches none of them -- so bricks are
    // destructible, and Material::Built is the right material. The stamp is
    // an OR, so a brick laid where terrain already stands changes nothing.
    int builderBricks = 12;     // 0x0040F948 stores 0xc at state+0x3c, and
                                // three builders laid exactly twelve
    int brickLength = 2;        // cells per brick, at offsets +1 and +2 ahead
                                // (was the classic 3; the loop runs i = 1, 2)
    int builderCycleMs = 16 * 60;   // 960 ms = 4.8 ticks, measured 0.9597 s
    int builderBrickPercent = 81;   // the first record's stored frame
    // The build state does not start at percent 0. Its enter hook seeks the
    // animation forward once:
    //
    //   0040fb99  e8 66 8f 05 00  call 0x468b04        ; frames * 0.06 / div
    //   0040fb9e  dc 35 f8 08 4b 00  fdiv [0x4b08f8]   ; the double 16.0
    //
    // and hands the quotient to the same advance method the per-tick step
    // uses, so it advances the clock by one sixteenth of a whole cycle --
    // 1/16 = 6.25%, whatever the frame count is. Live: the first sample after
    // every click read percent 9-10 about 25 ms in, and the first brick came
    // 0.692-0.694 s later, i.e. 0.715 s after the click against the 0.718 s
    // (81 - 6.25)% predicts. So the first brick is at 75% of a cycle, not at
    // 81%, and this field is why.
    int builderEnterPercent = 6;    // 6.25% rounded; the 16.0 at 0x004B08F8
    // Where a spent builder goes. MEASURED: strip 10, 8 frames, divisor 0.5,
    // so `frames * 0.06 / divisor` = 0.96 s -- and the shrug ran 0.966 s on
    // both builders that reached it before walking again. That divisor is
    // the first one in this project that is not 1.0, and it settles that the
    // divisor belongs in the DURATION even though it cancels in the percent
    // (round 5 only ever saw 1.0).
    int shrugCycleMs = 8 * 60 * 2;  // 960 ms; 8 frames of 60 ms over 0.5

    // MEASURED LIVE, round 5. One cycle of each skill's animation, in
    // milliseconds: frames * 60, where 60 ms is the double 0.06 at
    // 0x004B1940 and the frame counts were read off the live strip.
    int basherCycleMs = 24 * 60;   // 1440 ms = 7.2 ticks
    int diggerCycleMs = 20 * 60;   // 1200 ms = 6.0 ticks
    int minerCycleMs = 25 * 60;    // 1500 ms = 7.5 ticks
    // Where in the cycle the first bite fires, as the integer percent the
    // stepper compares against each record's stored frame. Read live out of
    // the record lists: bash frames 12, 16, 60, 64 (row offsets 0, 1, 3, 2);
    // dig 25 (the bite) and 30 (the Y step); mine 12, 12, 16, 16, 20 (args
    // 3, 2, 1, 0, -1). We fire a whole cycle's removal on the tick that
    // covers the first of them -- the later ones are 50-370 ms behind it,
    // inside the same 200 ms tick or the next, and nothing in this
    // simulation looks at the terrain between them.
    int basherBitePercent = 12;
    int diggerBitePercent = 25;
    int minerBitePercent = 12;

    // MEASURED LIVE, round 5: the footprints, in cells, relative to the
    // lemming's own cell. Each is the per-tick grid diff, not a guess.
    //
    // bash  the leading column only -- `trunc(X) + facing` -- four rows
    //       deep from the feet row upward. On MM_BASH_002, a right-facing
    //       basher at (190.455, 24) cleared (191, 24) (191, 25) (191, 27)
    //       (191, 26) at percents 12, 16, 60, 64, then columns 192, 193,
    //       194 the same way, one per cycle. A left-facing one at
    //       (156.97, 40) cleared columns 152, 151, 150, 149, 148 the same
    //       four rows each. The bite also re-sweeps any column between the
    //       column it entered on (state+0x4C) and the leading column, four
    //       rows per bite -- a catch-up path over cells it has already
    //       emptied, and the one live cell it ever caught was (152, 44).
    int basherReach = 1;        // columns removed per cycle (classic was 2)
    int basherHeight = 4;       // rows, from the feet row upward
    // dig   three cells, `trunc(X) - 1 .. + 1`, in the row below the feet,
    //       then Y steps one cell down. Four diggers on MM_DIG_000 cleared
    //       3 cells x 4 cycles each, every one 0x21 -> 0x00000000.
    int diggerWidth = 3;        // cells across (was a hard-coded 5)
    int diggerDepth = 1;        // rows per cycle -- now measured, not "it
                                // cannot halve"
    // mine  columns `+1` and `+2` ahead, rows feet-1 .. feet+3 in the
    //       original's Y-up grid, minus the one cell the bite skips
    //       (arg 2 with dx 1, the `0x004B10B8` dword 3 test at 0x00418A1E),
    //       then X advances one column and Y one row per cycle. Measured on
    //       MM_MINE: (194,71) (195,71) then (196,71) (195,70) (196,70) then
    //       (197,70) (197,71) (196,69) (197,69), the lemming going
    //       (193,72) -> (194,71) -> (195,70) -> (196,69).
    int minerReach = 2;         // columns ahead the bite probes

    // ---- the blocker: MEASURED LIVE, round 8 ---------------------------
    //
    // How tall a blocker's field is, in cells, counting the feet row. The
    // stamp loop at 0x0040E2BD runs `j = 0` while `j <= [0x004B10B8]`, and
    // that dword is 3, so four rows -- the feet row and the three above it,
    // one column wide, at the blocker's own trunc(X). Live: three blockers,
    // four cells each, 0x00000000 -> 0x00000800, predicted from this read
    // before every click and matched cell for cell; and all four cleared
    // again when one was bombed. Nothing else in the 256 x 96 grid moved in
    // any run.
    //
    // There is no radius and no width to tune here, which is the point: the
    // list scan this replaced had a `|dx| <= 3` either side of the blocker
    // and the original has one column.
    // docs/scratch/phase3-round8-blocker.md, tools/harness/measure_blocker.py.
    int blockerFieldHeight = 4;  // cells, from the feet row upward
    // ---- trampoline / bounce: MEASURED + READ, round 13 ----------------
    //
    // Detection is cell bit 0x1000 (Terrain::trampoline), painted at runtime
    // by trampoline-state enter 0x0041B838 and tested on fall land at
    // 0x00416560 â€” before splat. Bounce step 0x0040EEC4 lifts by
    // `-12.5*t*t + 35.0*t` (peak 24.5 at t = 1.4 = DAT_004c55a8) and moves
    // X by facing * 5.0 * dt. Live pad paint on MM_CLIMB_002 rose ~10 cells
    // before a head hit cut the arc; the formula peak is the open-air bound.
    // Integer sim: rise `bounceRiseCells` at `bounceSpeed` cells/tick along
    // -verticalDir, then Faller. tools/harness/measure_trampoline.py.
    int bounceRiseCells = 24;  // peak of -12.5 t^2 + 35 t at t=1.4
    int bounceSpeed = 7;       // 35.0 world-units/s * 0.2 s tick
    // ---- water / drown: MEASURED + READ, round 14 ----------------------
    //
    // Detection is cell bit 0x40 (Terrain::waterHazard). Every spawn attaches
    // type 0x2f (0x0042E02C) as a side-effect; its step probes ground
    // `(trunc(X), trunc(Y)-[+0x44])`, tests `and 0x40` at 0x00415386, and
    // constructs drown (0x00408700 â†’ vtable 0x004B0570) when the object loop
    // at 0x004C89D8 does not match. Enter 0x0040874c sets "drown". Step
    // 0x00408766 accumulates dt at state+8 and returns 2 when > 2.0.
    // Live paint on MM_CLIMB_002: five fallâ†’drown attaches, each left the
    // actor list with acc â‰ˆ 2.000 (~2.0 s). Integer sim: drownMs / tick.
    // tools/harness/measure_water.py. docs/scratch/phase3-round14-water.md.
    //
    // Round 15 MEASURED: acid-as-hazard is this same path. No separate
    // acidMs / State::Acid. Type 0x2e sibling unused (empty 0x004C89D8).
    // Entrance_acid â†’ [+0x4c]=1 (LemmingsAcid sheet); ^acid_glow is Fluid
    // subtype 2 visual. tools/harness/measure_acid.py.
    // docs/scratch/phase3-round15-acid.md.
    int drownMs = 2000;  // 2.0 s at state+8; 10 ticks at ticksPerSecond=5
    // ---- teleport: MEASURED + READ, round 16 ---------------------------
    //
    // Detection is object AABB on teleport.a3d (vtable 0x004B1250), not a
    // cell bit â€” 0/18 exclusive bits vs teleport levels (`measure_teleport.py
    // --audit`). Overlap 0x00430BA1 sites 0x00435467 / 0x004354CB walk the
    // shared list; attach type 0x28 (0x0041A310) with countdown push 4 at
    // state+0x18, then type 0x2c (0x0041B590). Relocate at 0x0043503C:
    // X = partner[+0x1b0] (+ facingÃ—6Ã—t), Y = partner[+0x1b8] + 2.0
    // (0x004B1238). Live MM_TELE_003: walkâ†’0x28â†’0x2câ†’jump to partner in
    // ~0.8 s; facing preserved. Integer sim: delay then snap to partner
    // feet (destY = terrain row of partner world Y, not Y+2 â€” the +2 is a
    // brief in-air pose before walk/fall). teleport_fx.a3d is visual only.
    // tools/harness/measure_teleport.py.
    int teleportDelayTicks = 4;  // state+0x18 countdown from push 4
    // ---- SpeedPad / FASTER: MEASURED + READ, round 17 ------------------
    //
    // Detection is object AABB on speedup_two_way.a3d (vtable 0x004B1178),
    // not a cell bit â€” 0/9 exclusive bits (`measure_speedpad.py --audit`).
    // Overlap FUN_00432742 walks list 0x004C88D0 + 0x00430BA1, filters
    // facing (+0xbc) and vdir (+0xa0). Activate FUN_00432632 dispatches
    // embed msg (âˆ’2,6,1) via FUN_00433B2E â†’ FASTER ctor FUN_00416730
    // (vtable 0x004B0A38, type 0x10, +0x11=1 â†’ parallel list +0x374).
    // Enter 0x0041688C: +0x50=2, timer double 5.0. Tick 0x0041677C
    // counts down; expiry sets +0x50=1. While +0x50==2, 0x0042E2BF sets
    // local_14=2 (main state twice per frame). Cue SpeedPad at 0x004C0D10
    // loads into DAT_004E32F8 and plays on activate (0x004326D1).
    // pressure_pad.a3d is not this object (trampoline round-13 negative;
    // co-occurs on some speed levels only). Live VH21_JC: walker on pad
    // â†’ FASTER on +0x378 list, timer 5.0â†’4.44; +0x50 poke ~2Ã— column rate.
    // tools/harness/measure_speedpad.py.
    int fasterMs = 5000;  // FASTER enter double 5.0 at state+0x18
    // ---- the bomber: READ AND MEASURED LIVE, round 7 -------------------
    //
    // The fuse is five seconds, and it is a time and not a tick count. The
    // bomb factory `0x0045B197` (entry [2] of the skill table at
    // `0x0045B570`, so panel index 2, which round 8 measured) pushes the
    // integer 5 at `0x0045B1B3`; the countdown's constructor `fild`s it into
    // a double at `0x00413D74`; its enter `0x00413DF9` schedules an event at
    // `clock + trunc(5.0) * 1000` ms, the 1000 being the dword
    // `mov [edx], 0x3e8` at `0x004657B2`. There is no stored 25 anywhere.
    //
    // MEASURED, six bombs across MM_BOMB and MM_Block_Bomb_A, each timed
    // from inside the grid-diff loop at ~650 Hz:
    //
    //   click -> flapper   4.9967 4.9985 4.9981 4.9976 4.9998 4.9980 s
    //   flapper -> blast   1.0211 1.0193 1.0226 1.0206 1.0191 1.0201 s
    //
    // and the countdown double itself, read off the lemming's +0x374 list,
    // came down 4.9940 -> 0.1330 over 4.8616 s: 1.000 s of fuse per second.
    // The lemming keeps its own state through all of it -- one primed walker
    // went from X 144.770 to 119.770, exactly 25.000 columns, which is 25
    // ticks of walkSpeed.
    int bomberFuseMs = 5000;
    // What happens at the end of the fuse is NOT the explosion. The 5000 ms
    // event plays `Ohno` (`0x00414435`, gated on the byte at `0x004CBB40`,
    // which reads 0x00 in play -- see the note below) and installs the
    // "flapper" state, vtable `0x004B0A58`, whose enter `0x0041695C` starts
    // the animation `explode`. Only when that strip finishes
    // (`call [eax+0x44]` at `0x00416AC6`) does `0x004081B0` build the real
    // blast state `0x004B0550`, whose step `0x00408227` cuts the crater.
    //
    // READ live off the controller: `explode` is strip 6, 17 frames,
    // divisor 1.0, so `17 * 0.06 / 1.0` = 1.020 s -- against 1.0191-1.0226 s
    // measured five times. Round 6's live strip table has [5] and [6] both at
    // 17 frames; [6] is the one a flapper actually plays.
    int exploderCycleMs = 17 * 60;   // 1020 ms
    // So `OhNo` is cued at 5000 ms and the crater lands 1020 ms after that,
    // 6020 ms after the click. Our OhNo used to fire at `fuse ==
    // ticksPerSecond` -- one second early by construction -- and our crater
    // landed on the same tick as the cue. Both moved.
    //
    // The gate byte. `0x004CBB40` is in `.data`'s uninitialised tail: that
    // section's virtual size is 0x1185B8 and its raw size is 0x7000, so the
    // file has no byte for it and the loader zero-fills it. The static read's
    // "0xFF in the image" was a read of the file at that offset, which is
    // past the section's raw data. Live, on two levels, before and after five
    // bombs, it read 0x00 -- and `test edx, edx / jne` at 0x00414426 means
    // zero is the arm, not the mute. Nothing in `.text` stores it, so the
    // play is never skipped and there is no quirk here to preserve.
    //
    // There is no `explosionRadius` any more. The footprint is a stored
    // 7 x 7 bitmap; see Terrain::clearExplosion. MEASURED: six blasts, 75
    // cells, every one `0x00000001` -> `0x00000000`, and in five of the six
    // the set of cells that changed was EXACTLY the set the stencil predicts
    // from the grid as it was before the click (the sixth also lost two
    // neighbouring blockers' fields, because the crater took their floor).
    // Nothing outside the window changed in any of the six whole-grid diffs.
    // docs/scratch/phase3-round7-live.md; tools/harness/measure_bomb.py.
    // READ FROM THE BINARY AND MEASURED LIVE, and it is 3, not the classic 4.
    // The head-room sample is the climber's, and it is not a constant inside
    // the test: 0x004139BF (called only from 0x004138E8 in the climb stepper
    // 0x004137CA) loads no .rdata double and has no immediate 3 or 4. It
    // takes its position from 0x0042F35F, which returns X + [+0x350] and
    // Y + [+0x358], and [+0x358] is the double 3.0 --
    //
    //   0042e0fc  c781 5c030000 00000840   mov [ecx+0x35c], 0x40080000
    //
    // in the lemming constructor 0x0042DE51, and written again at 0x0042F616
    // whenever the vertical flip 0x0042F5E3 leaves [+0x44] == 1 (upright).
    // Two probes come off that biased position, and they are one cell apart:
    //
    //   00413a53  call 0x40de60  ; cell(trunc(X+[+0x350]) - [+0x40], Y+3)
    //   00413a5b  jne 0x413a70   ;   nonzero -> "Climber hit head" (0x004BF2FC)
    //   00413b22  2b4244         sub eax, [edx+0x44]   ; row = (Y+3) - 1
    //   00413b50  call 0x40de60  ; cell(trunc(X+[+0x350]),          Y+2)
    //   00413b58  jne 0x413bfd   ;   nonzero -> keep climbing
    //                            ;   zero    -> "Climber topping out"
    //
    // Live, both arms, with the climber byte [lemming+0x48] -- the byte
    // 0x0041C5C1 reads -- written at birth and a pillar carved in the
    // original's own grid (measure_stepup.py --poke-climb --carve):
    //
    //  * ceiling: one solid cell at column 231 row 58 over a 12-cell pillar
    //    at column 232. 12 of 12 climbers hit head at feet Y = 55.000 --
    //    58 - 55 = 3 -- and X jumped 232 -> 231 (the `fsubr [edx+8]` at
    //    0x00413A79). A bias of 4 would have fired a tick earlier, at 54.
    //  * top-out: the same pillar with no ceiling, top solid row 62. 12 of
    //    12 climbers' last climbing Y was 61.000, never 60 and never 62, so
    //    the probe row is Y + 2 = the bias minus one vertical unit.
    //
    // This field is the CEILING bias only. Round 4 cited two differences it
    // does not capture and round 10 measured both; they are the two fields
    // below and `Rules::hoistLift`.  docs/scratch/phase3-round4-live.md,
    // docs/scratch/phase3-round10-steppers.md.
    int lemmingHeight = 3;      // head room: the ceiling probe, Y + 3

    // ---- the climber's top-out and the hoist: round 10 ------------------
    //
    // READ FROM THE BINARY AND MEASURED LIVE. The climb's top-out sample is
    // one cell BELOW its ceiling sample. Both come off the biased position
    // 0x0042F35F returns (X + [+0x350], Y + [+0x358], and [+0x358] is the
    // double 3.0); the top-out then takes one vertical unit back off the row:
    //
    //   00413b0c  dd45dc    fld  [ebp-0x24]   ; X + [+0x350], no facing term
    //   00413b1a  e8..      call 0x49c5ac     ; -> the WALL column
    //   00413b22  2b4244    sub  eax,[edx+0x44]  ; row = trunc(Y+[+0x358]) - 1
    //   00413b50  e8..      call 0x40de60
    //   00413b55  83380 0   cmp  dword [eax], 0
    //   00413b58  0f85..    jne  0x413bfd     ; non-zero -> keep climbing
    //   00413b5e  68..      push 0x004bf310   ; "Climber topping out"
    //
    // so the probe is the wall column `climberTopOut` cells above the feet,
    // and a climber's last climbing row is one below the wall's top.
    //
    // Live, MM_CLIMB_002, five panel-assigned climbers off the actor list
    // (tools/harness/measure_steppers.py --climber): walls at columns 64, 80,
    // 96 and 112, top solid rows 47, 59, 46 and 62, last climbing Y
    // **46.000, 58.000, 45.000, 61.000** -- `topSolid - 1` four times out of
    // four, and a fifth climb on a wall cut into the grid by hand topped out
    // at 43.000 over a top solid row of 44. Round 4 had the same figure on a
    // different level (12 of 12 at Y 61 over a top row of 62).
    int climberTopOut = 2;      // cells above the feet; NOT lemmingHeight

    // MEASURED LIVE. The hoist is its own state -- `read_steppers.py
    // --states` names vtable 0x004B0AF8 (constructor 0x00417C80, which also
    // writes 0x1e at +0x14), animation **`flip`** (0x004BF41C, pushed by the
    // enter hook 0x00417CCC) -- and its step does nothing at all until that
    // animation ends:
    //
    //   00417d05  ff5230    call dword [edx+0x2c]  ; advance by the slice
    //   00417d1b  ff5244    call dword [eax+0x44]  ; finished?
    //   00417d25  0f8494..  je   0x417dbf          ;  no -> return, X/Y alone
    //
    // `+0x44` is the same "strip finished" slot round 7 measured on the
    // flapper, so the hold is `frames * 0.06 / divisor`. READ off the live
    // controller table: `flip` is **strip 8, 14 frames, divisor 1.0 = 840 ms**
    // -- and the strip index was sampled as 8 with the animation running
    // frames 0 through 13 during every hoist. MEASURED: four hoists held
    // 0.840, 0.840, 0.833 and 0.840 s, and round 4 measured 0.834 s on
    // another level. It is NOT a whole number of 200 ms ticks (4.2 of them),
    // which is why this is milliseconds; our old `l.frame >= 2` was 400 ms.
    int hoistCycleMs = 14 * 60;  // 840 ms
    // MEASURED LIVE, and it is 2 -- not `lemmingHeight`, and X does not move.
    // The step writes both:
    //
    //   00417d39  0341 40   add eax,[ecx+0x40]    ; X := trunc(X) + facing
    //   00417d6f  8b5144 / 6bd203 / 03c2          ; Y := trunc(Y) + 3*vdir
    //
    // and then 0x0042F8B9 installs a fresh walk state through 0x0042E696,
    // which runs the flip state's EXIT hook 0x00417DD7 -- and that takes one
    // unit back off each:
    //
    //   00417de4  db4144 / dc6810 / dd5a10        ; Y -= vdir
    //   00417df6  db4140 / dc6808 / dd5a08        ; X -= facing
    //
    // Net: X unchanged, two cells up. Live, five placements, every one
    // `dx 0.000 dy +2.000`: (64.000, 46.000) -> (64.000, 48.000),
    // (80, 58) -> (80, 60), (96, 45) -> (96, 47), (112, 61) -> (112, 63), and
    // a left-facing one (24.000, 43.000) -> (24.000, 45.000). Round 4's
    // (61.000, 232.000) -> feet 63.000 with X still 232 is the same thing.
    int hoistLift = 2;          // cells up, X untouched

    // ---- the float halving is delayed: round 10 -------------------------
    //
    // READ AND MEASURED LIVE. `floatAfter` is when the umbrella *opens*; the
    // speed halves only when the `umbopen` animation ENDS. 0x004162FE starts
    // `umbopen` (0x004BF384) once the distance passes the 10.0 at
    // 0x004B03E8, 0x0041612F..0x00416186 clamps the sub-step to the time left
    // in it, 0x004162C2 then sets `[lemming+0x4b]`, and 0x00416038 is the
    // only thing that divides the 20.0 by the 2.0 at 0x004B03F8 -- on that
    // byte. So between the open and the halving the original is still at
    // `fallSpeed`.
    //
    // READ off the live controller table: `umbopen` is **strip 12, 8 frames,
    // divisor 1.0 = 480 ms**. MEASURED on MM_CLIMB_002, one lemming given
    // the floater byte as a labelled probe (the level stocks no parasol) and
    // dropped 28 world units: the strip went 7 (`fall`) -> 12 (`umbopen`) as
    // the distance passed 10, and 12 -> 11 (`float`) with `[+0x4b]` flipping
    // to 1, **0.479 s** later and 9.564 world units lower. Round 3b measured
    // the same 9.6-9.7 units on a different level. The descent rate after the
    // flag was 9.972 world units/s, so `floatSpeed` 2 is untouched.
    //
    // It is a fixed animation length, not a distance and not a function of
    // the drop: a 16-unit drop in the same run **never halved at all**
    // (15.980 units in 0.805 s = 19.85 u/s for the whole descent), because
    // the umbrella opens half a second in and `umbopen` had not finished when
    // it landed. `World::stepFaller` used to halve the moment the distance
    // passed 10 and would have taken five ticks over that drop against the
    // original's four.
    int floatOpenMs = 8 * 60;   // 480 ms of `umbopen` at full fall speed
  };

  World() = default;
  // Two overloads rather than a defaulted argument: a brace-init default for a
  // nested type is not usable while the enclosing class is still incomplete.
  explicit World(LevelSetup setup);
  World(LevelSetup setup, Rules rules);

  // One fixed step. Everything that changes, changes here.
  void tick();

  std::uint64_t tickCount() const { return tick_; }

  // The level clock, from LEVELMAP.DAT's per-level time limit. A limit of 0
  // means untimed, which is how an unsupplied limit behaves.
  bool timedOut() const {
    return setup_.timeLimitTicks > 0 &&
           tick_ >= static_cast<std::uint64_t>(setup_.timeLimitTicks);
  }
  // Ticks left on the clock, 0 once it has run out. Untimed levels report 0
  // too, so check timedOut() or the setup's limit to tell them apart.
  std::uint64_t timeRemainingTicks() const {
    if (setup_.timeLimitTicks <= 0 || timedOut())
      return 0;
    return static_cast<std::uint64_t>(setup_.timeLimitTicks) - tick_;
  }
  const Terrain& terrain() const { return setup_.terrain; }
  Terrain& terrain() { return setup_.terrain; }
  const std::vector<Lemming>& lemmings() const { return lemmings_; }
  const LevelSetup& setup() const { return setup_; }
  const Rules& rules() const { return rules_; }

  int spawned() const { return spawned_; }
  int saved() const { return saved_; }
  int dead() const { return dead_; }
  int activeCount() const;

  bool finished() const;
  bool won() const { return saved_ >= setup_.saveTarget; }

  // Apply a skill to a lemming. Returns false when it cannot be applied, which
  // is a legitimate outcome rather than an error.
  bool assign(std::size_t lemmingIndex, Skill skill);
  // The lemming nearest a point that a click should affect, if any.
  std::optional<std::size_t> pick(int x, int y, int radius = 4) const;

  // Set every remaining lemming's fuse, as the nuke does.
  void nuke();

  // Insert one lemming already at a cell and in a state. Not spawn: spawn
  // still drops from the entrance, and a live comparison starts from a
  // measured cell instead of that drop.
  void place(Lemming lemming);

  // Pending sound cues since the last drain. Cleared by the call.
  std::vector<SoundCue> takeCues();

private:
  void cue(SoundCue c);
  void spawn();
  void step(Lemming& l);
  void stepFaller(Lemming& l);
  void stepBounce(Lemming& l);
  void stepDrown(Lemming& l);
  void stepWalker(Lemming& l);
  void stepClimber(Lemming& l);
  void stepHoister(Lemming& l);
  void stepBuilder(Lemming& l);
  void stepShrugger(Lemming& l);
  void stepExploder(Lemming& l);
  void stepBasher(Lemming& l);
  void stepMiner(Lemming& l);
  void stepDigger(Lemming& l);
  // Milliseconds in one step, from the measured 5 Hz tick. The skills' cycles
  // are in milliseconds because they are not whole ticks (see Rules).
  int msPerTick() const;
  // How far a floater still falls at `fallSpeed` after its umbrella opens:
  // `floatOpenMs` of `umbopen` animation. See Rules::floatOpenMs.
  int floatOpenCells() const;
  void explode(Lemming& l);

  bool solidAt(int x, int y) const { return setup_.terrain.solid(x, y); }
  // A blocker's field, stamped into the terrain by the block state's enter
  // hook and erased by its exit hook. Round 8 replaced a list scan over the
  // lemmings -- `blockedBy`, which turned anyone within |dx| <= 3 -- with
  // this; the original never looks at the lemming list to block a walker.
  // World.cpp carries the addresses and the live series.
  void stampBlockerField(const Lemming& l);
  void clearBlockerField(const Lemming& l);
  void leaveBlocker(Lemming& l);
  bool atExit(const Lemming& l) const;
  // Round 16: closed AABB test matching live teleport doubles / atExit.
  const Teleport* teleportAt(const Lemming& l) const;
  void stepTeleporting(Lemming& l);
  // Round 17: SpeedPad AABB + facing/vdir filters (FUN_00432742).
  const SpeedPad* speedPadAt(const Lemming& l) const;
  // Round 21: Lever AABB + facing-vs-+0xb8 filter (0x00436D30); non-const
  // so a pull can flip Lever::state like activate 0x00436E6A.
  Lever* leverAt(const Lemming& l);
  void turn(Lemming& l) { l.facing = l.facing == Facing::Right ? Facing::Left : Facing::Right; }

  LevelSetup setup_;
  Rules rules_;
  std::vector<Lemming> lemmings_;
  std::uint64_t tick_ = 0;
  int spawned_ = 0;
  int saved_ = 0;
  int dead_ = 0;
  int sinceSpawn_ = 0;
  std::size_t nextEntrance_ = 0;
  std::vector<SoundCue> cues_;
};

}  // namespace webo::sim
