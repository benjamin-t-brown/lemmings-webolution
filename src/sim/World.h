// Simulates actors, skill assignments and level updates on the terrain grid.

#pragma once

#include "Terrain.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <functional>
#include <map>
#include <optional>
#include <span>
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
  // A builder enters Shrugger after its last brick. The 90% callback
  // installs shrug vtable 0x004B0C78 at 0x0041AB42.
  Builder, Shrugger, Basher, Miner, Digger, Exploder, Splatter, Exited, Dead,
  // A fall onto bit 0x1000 attaches bounce (type 0x21, vtable 0x004B0810)
  // at 0x00416560 before the splat check.
  Bounce,
  // Ground bit 0x40 attaches drown for water or acid (0x00415386 /
  // 0x004154D8, type 0x22, vtable 0x004B0570). Enter plays "drown"
  // (0x004BF130). Entrance_acid identifies a hatch, independent of this hazard.
  Drown,
  // Pad transit enters type 0x28 (0x004B0C1C, cartwheel), then type 0x2c
  // (0x004B0CE8). Object step 0x00434B0F relocates to the partner pose.
  Teleporting,
  // The cartwheel side effect (0x2b) and reversed fall (0x13) share one
  // clone state until this lemming reaches support in its new gravity.
  AntiGrav,
};
std::string_view stateName(State s);

enum class Facing : std::int8_t { Left = -1, Right = 1 };

// Simulation audio events. The mixer maps each cue to its .WAV.
enum class SoundCue : std::uint8_t {
  Assign,   // a skill stuck -- Action (0x4556B5); refusals emit no cue
  OhNo,     // a bomber fuse ended outside Nuke -- Ohno
  Explode,  // the fuse ran out -- Explode
  Splat,    // landed too hard -- Splat
  Exit,     // reached the exit -- Yippee
  LevelStart, // first hatch spawn -- Letsgo (presentation only)
  Bash,     // Common excavation cue (Mine).
  BuilderWarning, // a builder spends brick 3, 2 or 1 -- Chink
  Work = Bash, // a successful bash, mine or dig stroke removed terrain
};

// Audio-only snapshot in terrain coordinates; never used by simulation.
// sample is nonempty for serialized object sounds and supplemental effects.
struct SoundEvent {
  std::optional<SoundCue> cue;
  std::string sample;
  std::optional<std::array<double, 3>> position; // X, terrain feet Y, radial slab
};

struct Lemming {
  int x = 0;
  int y = 0;               // the lemming's feet
  Facing facing = Facing::Right;
  State state = State::Faller;

  int fallDistance = 0;    // pixels fallen since leaving the ground
  int frame = 0;           // State-local animation progress in ticks or milliseconds.
  int bricksLeft = 0;      // builder
  // Basher's completed cycles, capped at Rules::basherGraceCycles.
  // The first cycles can cross air. Constructor 0x0040BBA0 clears state+0x44.
  int bashCycles = 0;
  // Fuse in milliseconds. The original +0x374 side-effect list holds
  // state 0x004B0990, whose countdown uses seconds (0x00413F53).
  // -1 means unprimed. Zero marks an expired fuse and bars a second assignment.
  int fuseMs = -1;
  // Nuke owns the replacement Bomber state until crater and removal.
  bool nukeExploding = false;

  // FASTER (type 0x10, vtable 0x004B0A38) sets lemming+0x50=2
  // for 5.0 s at 0x0041688C. 0x0042E2BF steps the main state twice.
  // The timer counts down in ms, with zero inactive. The latch mirrors
  // pad+0xb8 and rearms when the actor leaves the AABB.
  int fasterMs = 0;
  bool speedPadLatched = false;

  // Store excluded pad indices per actor, transposing the original's
  // pad+0x248 list of actor IDs. Leaving a pad's closed AABB rearms it.
  std::vector<std::size_t> teleportPadsLatched;

  bool canClimb = false;   // climber and floater are permanent, not states
  bool canFloat = false;

  // Gravity sign: lemming+0x44 is +1 upright or -1 inverted.
  // Cartwheel enter 0x0041AE1B calls 0x0042F5E3 to negate it.
  // In the clone's Y-down grid, +1 falls toward the floor and -1
  // toward the ceiling. Step-up, climb, hoist and dig use the same axis.
  int verticalDir = 1;

  // 0x0042E05F copies hatch+0xB0 to [lemming+0x4c].
  // Sheet table 0x004BFFC8 maps 0 Lemmings, 1 LemmingsWater, 2 LemmingsAcid.
  // FUN_00416433 exempts matching sheets on zone bits 0x200 / 0x4000 from splat.
  int sheetType = 0;

  // Continuous movement adds this signed adjustment in seconds to update dt.
  // A partial spawn or same-update handoff subtracts time already elapsed.
  // A positive value carries time left after spending whole cells into the
  // next update. The tick path leaves this at zero unless enabled in LevelSetup.
  double stepBudgetSeconds = 0.0;

  // Live only in AntiGrav. Collision still samples each crossed integer row.
  double antiGravFraction = 0.0;  // distance toward the next row
  double antiGravTurn = 0.0;      // fraction of the half-turn completed

  // Closing terrain gates raise actor+0x5D (0x4270DC). Consumed on the
  // next actor update, like the original forced transition (0x42E340).
  bool gateCrushed = false;
  // Rat +3A0 retains this actor until its punch finishes (426414/408ECC).
  bool ratVictim = false;
  // Presentation source for the initial lane, even if the first update lands.
  bool releasedFromHatch = false;

  bool alive() const { return state != State::Exited && state != State::Dead; }
  bool active() const { return alive() && state != State::Splatter && !ratVictim; }
  double feetY() const {
    return state == State::AntiGrav ? y + verticalDir * antiGravFraction : y;
  }
};

// Placed subclass actors (448550 -> 448630), outside the rescue population.
struct EvilRat {
  Lemming actor;
  std::int16_t id = 0;
  std::array<double, 3> initialPosition{};
  bool storedFlag38 = false, renderEnabled = true;
  bool initialFlip = false;
  int pendingVictim = -1;
  unsigned punch = 0, attacks = 0;
  std::array<double, 4> punchDurations{0.48, 0.48, 0.48, 0.48};
  double animationSeconds = 0.0;
  double openDuration = 0.48;
  double lane = 0.5;
};

// Where lemmings arrive and leave. Positions are in terrain cells.
struct Entrance {
  int x = 0;
  int y = 0;
  // Retail spawns use the stored int16 HatchRecord coordinates (0x00425F1C /
  // 0x00425F2E). The authored Y is two units below Entrance*.a3d on retail
  // hatches. Use the model translation only when hatch words are unavailable.
  enum class Source : std::uint8_t { EntranceModel, HatchWords };
  Source source = Source::EntranceModel;
  // Stored hatch +0xB4/+0xB6; direction becomes the next birth's direction.
  // Direction fields occupy the padding before sheetType.
  bool alternateDirection = false;
  std::int16_t direction = 1;
  // HatchRecord.type (hatch+0xB0) becomes [lemming+0x4c].
  // 1 = LemmingsWater, 2 = LemmingsAcid.
  int sheetType = 0;
  // Stored hatch +0x98, decremented per birth (0x425B72..0x425BCB).
  // -1 lets hand-built setups divide the level total evenly in stream order.
  int releaseCount = -1;
};
// Exit AABB at +0x48..+0x70. 0x00430BA1 tests raw (X,Y,Z) with closed
// edges. MM_CLIMB_002 and MM_FLOAT use 2x3 boxes with Z in [0,1].
// This trigger stores the minimum corner in Y-down terrain cells.
struct Exit {
  int x = 0;
  int y = 0;
  int width = 2;
  int height = 3;
  // Serialized exit +0x98 (0x426C6B); compare with lemming +0x4C at
  // 0x426B79..0x426BBB. 0 normal, 1 water, 2 acid, 4 any tribe.
  int sheetType = 0;

  bool accepts(int type) const {
    return type != 3 && (sheetType == type || sheetType == 4);
  }
};

// Teleport vtable 0x004B1250 uses a closed AABB (0x00430BA1) and
// partner at object+0x260. The 12x8 spans are Y-down. Snap to the
// partner's feet cell while preserving facing.
struct Teleport {
  int x = 0;
  int y = 0;
  int width = 12;
  int height = 8;
  int destX = 0;
  int destY = 0;
};

// speedup_two_way.a3d uses vtable 0x004B1178. FUN_00432742 calls
// 0x00430BA1 on a closed box (VH21_JC: X at pose +/-1, Y at ty..ty+6).
// object+0xbc filters facing (0 = any) and +0xa0==0 requires
// upright (+0x44==1). Message (-2,6,1) attaches FASTER.
struct SpeedPad {
  int x = 0;
  int y = 0;
  int width = 2;
  int height = 6;
  int faceFilter = 0;      // 0 any; else Â±1 must match Facing
  bool requireUpright = true;
};

// lever.a3d uses vtable 0x004B1288. 0x00436D30 checks list 0x004C88D0,
// AABB 0x00430BA1 and facing vs +0xb8 (state 1 requires -1, else +1).
// 0x00436E14 plays Lever (DAT_004E32E4). 0x00436E6A stores facing and
// sends +0xcc (+1) or +0xbc (-1) through 0x00433AC0 -> FUN_00433B2E.
// Reader 0x00433FB9 loads each target id, type and two parameters.
// Dispatch 0x00433B2E maps 0 to vt+0xC(p1,p2), 4 to vt+4 and 5 to vt+8.
// Negative ids bypass lookup. The clone ignores the special -2 arm.
struct LeverMessage {
  std::int32_t targetId = -1;
  std::uint32_t type = 0;
  std::uint32_t p1 = 0;
  std::uint32_t p2 = 0;
};

struct Lever {
  int x = 0;
  int y = 0;
  int width = 2;
  int height = 6;
  // Mirrors object+0xb8. E16_KF_1 starts at 1.
  int state = 1;
  // The elapsed contact path decrements a positive cooldown before
  // testing eligibility. A pull reloads it from object+0x98.
  // Installed levers store 0.0. The per-actor tick path ignores cooldown.
  double cooldownSeconds = 0.0;
  double cooldownRemaining = 0.0;
  // Reader 0x00436FED supplies the linked type-1 record. The record's
  // base id addresses the lever. The load call (0x0043703F -> 0x00436E6A)
  // applies stored [+0xDC] and sends +0xBC for -1 or +0xCC for +1.
  // Without a unique pose match, only the local toggle is available.
  bool linked = false;
  int serializedId = -1;
  int initialFace = 0;
  std::vector<LeverMessage> faceMinus;
  std::vector<LeverMessage> facePlus;
};

// Tagged type 0, pressure_pad.a3d (vt 0x4B1148). 0x431A59 tests the
// actor point against the serialized closed AABB, without a facing/state
// filter. 0x431950 latches +0xB8 to 1 and sends +0xBC only on that edge.
// Its cooldown cannot rearm that latch; only loading a fresh level does.
struct PressurePad {
  int id = -1;
  int state = 0;
  // Original cylinder coordinates (Y up). Installed pads span Z [0,1].
  std::array<double, 3> min{}, max{};
  std::vector<LeverMessage> messages;
};

// Tagged type 2 shares the object registry. Occupancy dispatch is
// 0x43725A -> 0x437285/29A -> 0x4372AF.
struct OccupancySensor {
  int id = -1, state = 0;
  bool enabled = true; // base +0x38; vt+4/vt+8 enable/disable updates
  // +0xA4 holds the first overlapping actor pointer. The clone stores
  // a lemming index or -2-rat index, with -1 for no contact.
  int contactActor = -1;
  std::array<double, 3> min{}, max{};
  double cooldownSeconds = 0.0, cooldownRemaining = 0.0;
  std::vector<LeverMessage> occupied, empty;
};

// The original updates tagged objects in stream order before fluid receivers.
struct GadgetContactRef {
  bool pressurePad = false;
  std::size_t index = 0;
  bool sensor = false;
};

// Terrain gate (vt 0x4B0F00). Reader 0x424187 loads bounds, mode,
// request, quota, counter coordinates and direction. Mode 0 ignores the
// counter fields (often CDCDCDCD). Modes 1 and 2 count Y and X crossings.
struct CounterGate {
  int id = -1;
  int left = 0, right = 0, top = 0, bottom = 0; // original Y up, half-open
  int mode = 0, request = 2, previousRequest = 0;
  int quota = 0, count = 0, counterX = 0, counterY = 0, direction = 0;
  double delay = 0.0;                         // +0xF0
  double target = 1.0, progress = 1.0;        // +0x90 / +0x98; 0 open, 1 closed
  bool collisionClosed = true;
  std::array<double, 3> position{};             // audio snapshot, terrain Y down
  bool hasCounter() const { return mode == 1 || mode == 2; }
  int remaining() const { return std::clamp(quota - count, 0, 99); }
  bool containsCell(int x, int originalY, int width) const;
  void initialise(Terrain&);
  void paintCollision(Terrain&, bool closed);
  // 0x4270DC: advance at 3/sec; stamp/clear only after overshooting an end.
  void animate(double dt, Terrain&);
  // 0x426FEC: opening squares p; closing uses 1-(1-p)^2.
  double surfaceFraction() const;
};

// Fluid-zone receiver (vtable 0x004B1020, ctor 0x00428680, reader
// 0x0042ACAC, step 0x0042B24D). Request [+0x18C] is 0 to raise
// (message 4) or 1 to drain (5). Done [+0x188] starts at 2 and
// settles to request. Armed [+0x1B0] starts at 0.
// Height [+0x1A8] ramps by dt * rate between high [+0x98] and low [+0xA0].
// The Y-up AABB spans (+0x48,+0x50) to (+0x60,+0x68).
// Kind [+0x128] selects 0x200 (water) or 0x2000 (lava). The clone
// tracks lava height and state without a 0x2000 terrain plane.
struct FluidGate {
  int id = -1;
  int kind = 0;
  int request = 0;
  int done = 2;
  bool armed = false;
  double minX = 0.0, minY = 0.0, maxX = 0.0, maxY = 0.0;
  double height = 0.0, high = 0.0, low = 0.0;
  // Render-only tail of 0x0042B24D. Lane is serialized value_16c
  // ([+0x16C]). Wave phase [+0x148] starts at zero and advances on
  // every update, even with request == done. Use the stored mesh until the first update.
  std::uint32_t lane = 0;
  double wavePhase = 0.0;
  static double wavePeriodFor(int kind);
  bool animated = false;
  static constexpr double kWavePeriod = 2.0;  // Kind-0 wave period, in seconds.

  // Rate table 0x004BF6F8 + kind * 0x48 (startup writer 0x00428848):
  // kind 0 = 2.0, kind 1 = 0.5, kind 2 = 1.0.
  static double rateFor(int kind);
  bool paintsZonePlane() const { return kind == 0; }
  // Reader 0x0042ACAC load pass: request 1 -> height = low and CLEAR the AABB; else
  // height = high and SET it. The AABB is the stream one (+0x68 is not recomputed).
  void initialise(Terrain& terrain);
  // 0x0042B24D advances height and mask, then wave phase, on the same
  // elapsed seconds as the lever contact.
  void step(double dt, Terrain& terrain);
  // Height ramp and zone-mask paint only.
  void stepHeight(double dt, Terrain& terrain);
  // `[+0x148] += dt; while (2.0 < phase) phase -= 2.0` (0x0042B24D tail, the jump target of the
  // request == done early exit at 0x42B29E).
  void advanceWave(double dt);
};

// The base reader (0x00423310) registers every object by id in
// construction order, with the last one winning. Dispatch applies traced
// messages to gates, sensors and supported named hazards.
enum class ObjectClass : std::uint8_t { Gate, Other, NamedHazard, CounterGate, OccupancySensor };
struct ObjectRef {
  ObjectClass cls = ObjectClass::Other;
  int index = -1;                 // LevelSetup index, or serialized index for Other
  const char* collection = "";   // loader collection name
  // Presentation anchor for Other receivers, whose motion is not simulated.
  std::optional<std::array<double, 3>> cylinderPosition{};
};

// Lever dispatch outcomes.
struct LeverDispatchStats {
  std::uint64_t delivered = 0;     // supported message applied to a gate/hazard/sensor
  std::uint64_t unresolved = 0;    // id slot empty (0x00433B2E skips it)
  std::uint64_t unsupported = 0;   // type 0/other types, special ids, non-gate targets
};

// Balloon is an asset-only cue. FUN_0045e51f registers 0x004C0D24 at
// object+0x1c8f8 / DAT_004E3300 (load 0x0045EB2B, unload 0x0045F172).
// SOUNDS\BALLOON.WAV has no play path or balloon.a3d gadget.
// Models\exit\balloon_* are exit decorations.
inline constexpr bool kBalloonGadgetInRetail = false;

// Fire is an asset-only cue. FUN_0045e51f registers 0x004C0D2C at
// object+0x1c8fc / DAT_004E3304 (load 0x0045EB41, unload 0x0045F186).
// SOUNDS\FIRE.WAV and SPRITES\FIRE.SPR have no executable or .LVL use.
// _FireBraize* and lava glow are decoration.
inline constexpr bool kFireGadgetInRetail = false;

// Witch is a pause cue. FUN_0045e51f registers 0x004C0D34 at
// object+0x1c8a8 / DAT_004E32B0 (load 0x0045EB57, unload 0x0045F19A).
// Pause arm FUN_0045cff9 compares DAT_004cbbf8 with "compiled\\e10_kf_1.lvl".
// On match, 0x0045D200 calls FUN_0044521b and skips the pause UI.
// SOUNDS\WITCH.WAV serves that branch. TEXTURES\MODELS\WITCH\* is unused
// and there is no witch.a3d gadget or +0x1c900 Witch slot.
inline constexpr bool kWitchGadgetInRetail = false;

// ForceF is an asset-only cue. FUN_0045e51f registers 0x004C0CC8 at
// object+0x1c8d0 / DAT_004E32D8 (store 0x0045E614, load 0x0045EA39,
// unload 0x0045F0AA). SOUNDS\FORCEF.WAV has no play path, and
// SOUNDS\FORCEFLD.WAV has no cue string.
inline constexpr bool kForceFGadgetInRetail = false;

// Axe is a render-only miner refusal effect. 418D75..418E5B constructs
// the child, 4194F0 -> 496E40 binds 4E331C (SPRITES/AXE.SPR), and
// 4196C0 -> 493761 submits the draw. TubeScene presents the sprite.
inline constexpr bool kAxeGadgetInRetail = false;

// Bubble is a fluid-mismatch cue. FUN_0045e51f registers 0x004C0C9C
// at object+0x1c8bc / DAT_004E32C4 (load 0x0045E9CB, unload 0x0045F046).
// Walker FUN_0041bffc attaches FUN_004041fd at 0x0041C3BA when lem+0x54
// mismatches sheet +0x4c / fluid+0x128 or the fluid is subtype 1.
// Type 0x23 (vtable 0x004B03C0) enters "drown" and plays Die DAT_004E32DC
// for subtype 1. FUN_00404372 plays SOUNDS\BUBBLE.WAV at 0x00404697 /
// 0x004046C4 via FUN_00404cc0 -> FUN_00406a00 -> FUN_0044535b
// on height delta >5 (one-shot +0x1a). Water-hazard drown is type 0x22
// (0x004B0570). Bubbles 0x004BF128 names SPRITES\BUBBLES.SPR. No bubble.a3d.
inline constexpr bool kBubbleGadgetInRetail = false;

// Scrape is a refusal cue. FUN_0045e51f registers 0x004C0CA4 at
// object+0x1c8c0 / DAT_004E32C8 (load 0x0045E9E1, unload 0x0045F05A).
// Bash FUN_0040c357 calls FUN_0040c0af, which plays via FUN_00445274
// at 0x0040C139. Dig FUN_00415a43 plays via FUN_00406a00 ->
// FUN_0044535b at 0x00415B33, then walks via FUN_0042f8b9.
// Success plays Mine DAT_004E32E8. Miner refusal plays MineMetal
// DAT_004E32EC. SOUNDS\SCRAPE.WAV serves the cue without a scrape.a3d.
inline constexpr bool kScrapeGadgetInRetail = false;

// Rumble is a gate cue. FUN_0045e51f registers 0x004C0CFC at
// object+0x1c8e8 / DAT_004E32F0 (load 0x0045EAD3, unload 0x0045F122).
// At a deferred commit with nonzero prior +0xa4, 0x00423823 in
// FUN_00423749 (vtable 0x004B0F00 +0x14) calls FUN_00406a00 ->
// FUN_0044535b. FUN_00423858 scans list 0x004C88D0 for the gate count
// (+0x90 / +0x58). 0x46dfc3 / 0x489453 refer to call+0x1c8.
// SOUNDS\RUMBLE.WAV plays without a rumble.a3d gadget.
inline constexpr bool kRumbleGadgetInRetail = false;

// Changeop is a UI cue. FUN_0045e51f registers 0x004C0CB4 at
// object+0x1c8c8 / DAT_004E32D0 (load 0x0045EA0D, unload 0x0045F082).
// Skill select FUN_0045ac5c plays at 0x0045AD07 through FUN_0044521b
// when dword[0x004C6A08]==2. Slider FUN_0048e8d5 (vtable 0x004B24D4)
// plays at 0x0048EB0C via FUN_00404cc0 -> FUN_00497f39 for target==this+0x44.
// SOUNDS\CHANGEOP.WAV is active. OLD-CHANGEOP.WAV is unused. No changeop.a3d.
inline constexpr bool kChangeopGadgetInRetail = false;

// Die is a lava contact cue. FUN_0045e51f registers 0x004C0CD0 at
// object+0x1c8d4 / DAT_004E32DC (load 0x0045EA4F, unload 0x0045F0BE).
// Type 0x23 enter FUN_00404261 (vtable 0x004B03C0 +8) loads it at
// 0x004042CD (operand 0x004042CF) for fluid+0x128==1, then calls
// FUN_00406a00 -> FUN_0044535b. FallSplat DAT_004E32E0 is used by
// FUN_0040b3ec (type 0x1a). Splat DAT_004E32D4 has no play references.
// SOUNDS\DIE.WAV plays without a die.a3d gadget.
inline constexpr bool kDieGadgetInRetail = false;

// Crab.a3d records are stationary STAT hazards (0x4333D2).
// 0x430C9C adds relative boxes +0x78/+0x88 to the broad-box minimum.
struct Crab {
  struct Volume { std::array<double, 3> min{}, max{}; };
  std::size_t entityIndex = 0;  // named_entities stream index, shared with the scene
  std::array<double, 3> boxOrigin{};
  std::vector<Volume> volumes;
  bool enabled = true;
  double restDuration = 1.0, attackDuration = 1.0;
  bool attacking = false;
  double animationSeconds = 0.0;
  std::string attackSound;
};

// Named STAT/DYNA hazards (0x4333D2 / 0x42576F) use serialized
// patrol segments, sampled at start + velocity*time (0x451997).
struct NamedHazard {
  enum class Kind { Deathroller, Buzzsaw, Scorpion, Robin };
  struct Segment {
    double duration = 0;
    unsigned direction = 0;  // 0 left, 1 right, 2 down, 3 up; XOR reverse
    std::array<double, 2> start{}, velocity{};
  };
  struct Path {
    bool loop = false;
    std::vector<Segment> segments;
    std::vector<unsigned> junctions;
  };
  Kind kind = Kind::Deathroller;
  std::size_t entityIndex = 0;
  int id = -1;
  bool enabled = true, moving = false, moved = false;
  std::array<double, 3> position{}, initialPosition{}, boxOrigin{};
  std::vector<Crab::Volume> volumes;
  std::array<double, 4> cutOffsets{}; // minX/minY/maxX/maxY relative to position
  std::array<Path, 2> paths;
  unsigned path = 0, requestedPath = 0, segment = 0;
  double travel = 0, speed = 1;
  bool reverse = false;
  bool allContacts = true, attacking = false, particles = true;
  int pendingVictim = -1;
  double animationSeconds = 0, restDuration = 1, attackDuration = 0;
  std::string attackSound;

  void advancePatrol(double dt, Terrain&);
};

struct LevelSetup {
  Terrain terrain;
  std::vector<Entrance> entrances;
  std::vector<Exit> exits;
  std::vector<Teleport> teleports;
  std::vector<SpeedPad> speedPads;
  std::vector<Lever> levers;
  std::vector<PressurePad> pressurePads;
  std::vector<OccupancySensor> occupancySensors;
  std::vector<CounterGate> counterGates;
  std::vector<Crab> crabs;
  std::vector<NamedHazard> namedHazards;
  std::vector<EvilRat> rats;
  std::vector<GadgetContactRef> gadgetContactOrder;
  // Each level owns its receiver state, object-id table and dispatch counts.
  std::vector<FluidGate> gates;
  std::map<int, ObjectRef> objectIds;
  LeverDispatchStats leverStats;
  int totalLemmings = 10;
  int saveTarget = 1;
  int releaseIntervalTicks = 20;  // ticks between spawns
  // Positive releaseIntervalSeconds selects the shared elapsed scheduler.
  // It waits releaseArmSeconds, seeds once from the original's 1.3 seconds
  // (0x004B0F48), then decrements per update. At zero it releases and adds
  // the RELEASERATE.PC interval to the accumulator. The original keeps a
  // per-hatch double at hatch+0xC0. Zero interval selects the tick scheduler.
  double releaseArmSeconds = 0.0;             // Provisional for all installed levels (MM_CLIMB_002 basis).
  double releaseFirstCountdownSeconds = 0.0;  // One-time seed in seconds.
  double releaseIntervalSeconds = 0.0;        // Recurring RELEASERATE.PC interval, in seconds.

  // Enable elapsed fall/walk budgets on the tick entry, independently of
  // hatch timing. A stepper spends whole cells and can hand off leftover
  // seconds within the update. When disabled, it uses cells per tick.
  bool continuousStepperClock = false;

  int timeLimitTicks = 0;         // 0 = untimed
  // Remaining skill-panel counts, indexed like Skill. -1 means unlimited
  // (hand-built test levels). Retail .LVL loads fill these from the trailer.
  std::array<int, 8> skills{-1, -1, -1, -1, -1, -1, -1, -1};
  int nukeStock = 1; // Retail loader supplies serialized values_27d0[9].
};

// Dispatch serialized lists in order (0x00433AC0 -> 0x00433B2E),
// resolving base ids in LevelSetup::objectIds. For fluid receivers,
// types 4 and 5 write request 0 and 1. Count unsupported or missing targets.
void dispatchLeverMessages(LevelSetup& setup, const std::vector<LeverMessage>& list);
// 0x00436E6A: face +1 -> state := 1 and send list +0xCC; face -1 -> state := -1 and send list
// +0xBC; any other face does nothing. The explicit update path calls this on lever contact.
void activateLeverFace(LevelSetup& setup, Lever& lever, int face);
// Apply each linked lever's stored face and send its list once at load
// (0x00444417 -> vt+0x28 = 0x0043703F -> 0x00436E6A).
void fanOutLeverLoadFaces(LevelSetup& setup);

class World {
public:
  // Timing and distances for the clone's terrain grid and skill updates.
  // RELEASERATE.PC supplies release intervals in seconds through webo::ReleaseRates.
  // LEVELMAP.DAT supplies the level limit in seconds (original clock 0x004CBAAC).
  struct Rules {
    // Nominal simulation clock, independent of render cadence.
    int ticksPerSecond = 5;

    // One walking column is one collision-mask lattice unit.
    int walkSpeed = 1;          // mask columns per tick

    // One climbing cell is one world unit (assets/CollisionGrid.h).
    int climbSpeed = 1;         // cells per tick

    // Fall vtable 0x004B0A20 accumulates descent at +0x18. Accessor
    // 0x00416038 loads 20.0 at 0041604a (0x004b0a18), halves it for an
    // open umbrella at 00416050 (0x004b03f8), and multiplies by seconds
    // at 0041607b ([ebp+0xc]). 00416213 / 00416216 scale [ebp-0x20] by
    // gravity [eax+0x44]. 00416219 subtracts that from Y [edx+0x10].
    // At 200 ms, 20 units/s is four cells, as on MM_H_002 and VH13_KF_1.
    // The separate descent state
    // 0x004B0C60 uses 25.0 at 0x004B0C40 (0x0041A758 loads 0x004b0c40).
    int fallSpeed = 4;          // cells per tick while falling

    // The float branch divides by 2.0 at 0x004B03F8 once 0x004162C2
    // sets lemming [+0x4b] at the end of "umbopen" and starts "float".
    // On E10_KF_1, an open umbrella reduces descent to 10 units/s.
    int floatSpeed = 2;         // cells per tick under an umbrella

    // Fall state +0x18 accumulates descent in world units at 0x00416318.
    // 004162d9 compares 0x004b03e8 (10.0) and latches opening at
    // [state+0x20]. 004165ff compares 0x004b10a8 (30.0).
    // 0x00416607 (test ah, 0x41) survives at or below 30.0.
    // Floater [+0x49] and FUN_00416433's sheet [+0x4c] matches
    // (1 on 0x200, 2 on 0x4000) exempt splat. Bit 0x80 prevents
    // landing. Lava zone 0x2000 provides no splat exemption.
    int floatAfter = 10;        // umbrella opens this far into the fall
    int splatDistance = 30;     // falling further than this kills

    // Bash, dig and mine use animation callbacks. 0x0041B077 converts
    // progress to percent (fld [0x004B0CE0] loads 100.0 at 0x0041B1A9,
    // truncated by 0x0049C5AC). For each crossed percent, 0x0041B21E
    // matches the record frame (cmp eax, [ebp-8]) and 0x0041B248 dispatches
    // its callback (call [eax+0xc]).
    // 0x00468B04 / 0x00468B5D scale frame count / index by 0.06 s
    // (0x004B1940). The divisor [strip+0x10] cancels in the ratio.
    // The strip table is on the controller at lemming+0x128, with
    // frame count at [[+0x1F8]+0xC] + [+0x1C8] * 0x98. Bash/dig/mine
    // occupy strips 0/4/9 at rate 1.0. Dig enter 0x004159CA ->
    // 0x004683E9 writes 0.75 to controller +0x1D8, a separate field.
    // The bites zero whole cell words. Slot 3 of trampoline vtable
    // 0x004B0D08 holds FUN_0041B683 (and 0xf77c at 0x0041B783).
    //
    // Walk step 0x0041BDD4 calls probe 0x0041BFFC at 0x0041BEE6.
    // 0041c47d tests solidity and 0041c482 branches to 0x41c6ef for
    // clear ground. 0041c48e (0x0041C48E) and 0041c528 (0x0041C528)
    // add [+0x44] for the two rises. Tests 0041c517 and 0041c5b1
    // branch at 0041c51c and 0041c5b6 to 0x41c6ae (0x0041C6AE),
    // which stores the row through 0x0042EE74. A third solid cell
    // tests climb ability at 0041c5c1 and branches at 0041c5c6 to
    // 0x41c676 (turn 0x42f66b) when unavailable. The step-down arm
    // adds [+0x44] at 0x0041C81B before return at 0x0041C872.
    // Spawn 0x0042DFE7 sets [+0x44] to 1 and 0x0042F5E3 negates it.
    // A rise of three cells takes the wall arm, as on VH13_MT and E07_JC_1.
    int maxStepUp = 2;          // a walker climbs steps up to this high

    // Builder vtable 0x004B08DC (0x4b08dc at 0x0040F93F) steps
    // via slot +4 at 0x00410302, tail-calling 0x41b077 at 0x0041047B.
    // Ctor 0x0040F8D1 registers two records at state+0x18.
    // 0040f9a8 / 0040fa06 set frame +8 to 81% / 90%.
    // 0040f9b2 / 0040fa10 set callback +0xc to 0x41011c / 0x40fbcc.
    // Strip 2 lasts 16 * 0.06 s / 1.0 = 960 ms.
    // Brick 0x0041011C stamps cells at offsets 1 and 2 times facing
    // [edx+0x40]. 00410199 / 004101ab set the [ebp-0x10] loop bounds,
    // 004101b7 multiplies by facing and 004101eb calls 0x40de60.
    // 004101f8 / 004101fe OR and store 0x00000001 at the feet row.
    // The 0x0040FB0B snap offsets the left-facing [+0x360] bias so
    // trunc(X + [+0x360]) is the placement column in either direction.
    // The brick chain 0x0041011C -> 0x0043B016 -> 0x0043AE6C paints
    // destructible cells. The separate OR 0x403 at 0x004275C1 in
    // 0x00427554 is called by 0x004271D5, 0x00427509 and 0x0043B1F8.
    // The two-cell bricks on MM_BUILD leave existing solid cells intact.
    int builderBricks = 12;     // 0x0040F948 sets state+0x3c to 0xc.
    int brickLength = 2;        // Cells at offsets +1 and +2 ahead.
    int builderCycleMs = 16 * 60;   // Milliseconds per strip-2 cycle.
    int builderBrickPercent = 81;   // the first record's stored frame
    // Enter seeks one sixteenth of the cycle. 0040fb99 calls 0x468b04
    // for strip duration, and 0040fb9e divides by 16.0 at 0x4b08f8.
    // That leaves roughly 75% of the cycle before the first brick.
    int builderEnterPercent = 6;    // 6.25% rounded; the 16.0 at 0x004B08F8
    // Shrug uses strip 10 at half speed. The divisor scales duration
    // even though it cancels when computing progress.
    int shrugCycleMs = 8 * 60 * 2;  // 960 ms; 8 frames of 60 ms over 0.5

    // Skill cycle duration uses frame count times 60 ms (0.06 s at 0x004B1940).
    int basherCycleMs = 24 * 60;   // 1440 ms = 7.2 ticks
    int diggerCycleMs = 20 * 60;   // 1200 ms = 6.0 ticks
    int minerCycleMs = 25 * 60;    // 1500 ms = 7.5 ticks
    // Callback percents: bash 12/16/60/64 (rows 0/1/3/2), dig 25/30
    // (bite/advance), mine 12/12/16/16/20 (args 3/2/1/0/-1).
    // Provisional hybrid sampling groups the lower and upper bash punches
    // at 12% and 60%, then advances at 92%. Dig and mine sample whole bites.
    int basherBitePercent = 12;
    int diggerBitePercent = 25;
    int minerBitePercent = 12;

    // Each bash cycle cuts ahead from the feet row upward. With reach 1,
    // X=190.455 facing right on MM_BASH_002 cuts column 191.
    // The original can re-sweep columns starting at state+0x4C.
    int basherReach = 1;        // Probe and cut width in columns.
    int basherHeight = 4;       // rows, from the feet row upward
    // Basher ctor 0x0040BBA0 clears the removal flag (+0x41) and cycle
    // counter (+0x44). The 92% callback 0x0040C6E8 advances +0x48 by
    // facing. On support (FUN_0042E913), two completed cycles and no cut
    // select walk (FUN_0042F8B9). Otherwise cap the counter at two and
    // clear the flag. Off support selects fall.
    // Bite 0x0040C357 also tests the flag after argument 0 at 12%.
    // Helper 0x0040C0AF refuses steel, directional bits and blocker fields,
    // and sets +0x41 only for a nonzero cell it cleared. Provisional:
    // the clone preflights all rows at the first punch. The original's
    // 12% open-air stop sees the feet row.
    int basherGraceCycles = 2;
    // A dig clears the row toward gravity, centered on trunc(X), then
    // advances into it. The three-cell cut on MM_DIG_000 clears word 0x21.
    int diggerWidth = 3;        // Cells across the cut.
    int diggerDepth = 1;        // Rows per cycle.
    // Mine probes two columns ahead at feet-1 through feet+3 in Y-up.
    // 0x00418A1E skips the arg 2, dx 1 cell using dword 3 at 0x004B10B8.
    // Each cycle advances one column and one row, as on MM_MINE.
    int minerReach = 2;         // columns ahead the bite probes

    // 0x0040E2BD stamps one column at trunc(X), from the feet through
    // row offset [0x004B10B8] (3). Air words go from 0x00000000 to
    // 0x00000800 without becoming solid.
    int blockerFieldHeight = 4;  // cells, from the feet row upward
    // Trampoline enter 0x0041B838 paints bit 0x1000. Fall land
    // 0x00416560 tests it before splat. Bounce step 0x0040EEC4 uses
    // -12.5*t*t + 35.0*t (peak 24.5 at t=1.4, DAT_004c55a8), and
    // moves X by facing * 5.0 * dt. Provisional: the clone rises in
    // whole cells against gravity until its head hits or it reaches the cap.
    int bounceRiseCells = 24;  // peak of -12.5 t^2 + 35 t at t=1.4
    int bounceSpeed = 7;       // 35.0 world-units/s * 0.2 s tick
    // Spawn 0x0042E02C attaches type 0x2f, which probes
    // (trunc(X), trunc(Y)-[+0x44]) for bit 0x40 at 0x00415386.
    // With no match in the 0x004C89D8 list, it constructs drown
    // (0x00408700 -> 0x004B0570). 0x0040874c sets "drown".
    // 0x00408766 accumulates seconds at state+8 and removes the actor
    // above 2.0. Acid uses the same hazard bit. Type 0x2e is a separate
    // Fluid-match sibling. Entrance_acid and LemmingsAcid identify a
    // hatch and sheet. ^acid_glow is a subtype 2 Fluid visual.
    int drownMs = 2000;  // 2.0 s at state+8 (10 ticks at 5 Hz).
    // teleport.a3d (vtable 0x004B1250) uses overlap 0x00430BA1 at
    // 0x00435467 / 0x004354CB. Type 0x28 (0x0041A310) counts down
    // from push 4 at state+0x18 before type 0x2c (0x0041B590).
    // 0x0043503C places X at partner[+0x1b0] + facing*6*t and Y at
    // partner[+0x1b8] + 2.0 (0x004B1238). The clone snaps to the
    // partner's feet cell after the delay, as on MM_TELE_003.
    // teleport_fx.a3d is visual.
    int teleportDelayTicks = 4;  // state+0x18 countdown from push 4
    // speedup_two_way.a3d (vtable 0x004B1178) uses FUN_00432742 to
    // scan list 0x004C88D0 via 0x00430BA1, filtering facing (+0xbc)
    // and gravity (+0xa0). FUN_00432632 sends (-2,6,1) via FUN_00433B2E
    // to FASTER ctor FUN_00416730 (vtable 0x004B0A38, type 0x10,
    // +0x11=1, parallel list +0x374 / +0x378).
    // Enter 0x0041688C sets +0x50=2 and 5.0 s. 0x0041677C restores
    // +0x50=1 at expiry. 0x0042E2BF sets local_14=2 to step twice.
    // SpeedPad string 0x004C0D10 loads into DAT_004E32F8 and plays at
    // 0x004326D1. pressure_pad.a3d is a separate gadget.
    int fasterMs = 5000;  // FASTER enter double 5.0 at state+0x18
    // Bomber factory 0x0045B197 (skill table 0x0045B570, index 2)
    // pushes 5 at 0x0045B1B3. Ctor 0x00413D74 converts it to seconds.
    // Enter 0x00413DF9 schedules clock + trunc(seconds) * 1000 ms
    // (0x004657B2 writes 0x3e8). The +0x374 fuse is parallel to movement.
    int bomberFuseMs = 5000;
    // Fuse expiry plays Ohno at 0x00414443 when 0x004CBB40 is zero
    // and enters flapper vtable 0x004B0A58. 0x0041695C starts "explode"
    // (strip 6, 17 frames at rate 1.0). Finish slot [eax+0x44] at
    // 0x00416AC6 calls 0x004081B0 to construct blast 0x004B0550.
    // Its step 0x00408227 cuts the crater.
    int exploderCycleMs = 17 * 60;   // 1020 ms
    // Nuke request at level base 0x004C6A08 + 0x5138 == 0x004CBB40
    // gates Ohno. 0x00444A98 writes the request via its receiver.
    // Event 7 also writes the byte at 0x00457017.
    // Terrain::clearExplosion applies the 7x7 stencil, including on MM_BOMB
    // and MM_Block_Bomb_A.
    //
    // Climb step 0x004137CA calls the head checks 0x004139BF at
    // 0x004138E8. 0x0042F35F returns X + [+0x350], Y + [+0x358].
    // Ctor 0x0042DE51 stores upright head bias 3.0 (0042e0fc,
    // [ecx+0x35c] = 0x40080000). Flip 0x0042F5E3 restores it at
    // 0x0042F616 when [+0x44] == 1.
    // 00413a53 calls 0x40de60 one cell behind facing [+0x40] at head
    // height. 00413a5b branches to 0x413a70 ("Climber hit head",
    // 0x004BF2FC), with the X correction at 0x00413A79 (fsubr [edx+8]).
    // 00413b22 subtracts [edx+0x44] for the wall-column probe at
    // 00413b50. 00413b58 branches to 0x413bfd to keep climbing.
    // The wall's climb capability check reads [lemming+0x48] at 0x0041C5C1.
    int lemmingHeight = 3;      // head room: the ceiling probe, Y + 3

    // Top-out samples the wall one row below the head. 0x0042F35F
    // supplies X + [+0x350], Y + [+0x358]. 00413b0c loads biased X
    // from [ebp-0x24], and 00413b1a truncates it through 0x49c5ac.
    // 00413b22 subtracts
    // [edx+0x44] from the biased row, then 00413b50 calls 0x40de60.
    // 00413b55 / 00413b58 branch to 0x413bfd for solid, otherwise
    // 00413b5e selects "Climber topping out" (0x004bf310).
    int climberTopOut = 2;      // cells above the feet; NOT lemmingHeight

    // Hoist vtable 0x004B0AF8 (ctor 0x00417C80, type 0x1e at +0x14)
    // starts "flip" (0x004BF41C) at 0x00417CCC. 00417d05 advances the
    // strip (8, 14 frames, rate 1.0) via [edx+0x2c]. 00417d1b calls finished
    // slot +0x44. 00417d25 branches to 0x417dbf while it is playing.
    int hoistCycleMs = 14 * 60;  // 840 ms
    // Hoist adds facing [ecx+0x40] to X at 00417d39 and 3*vdir
    // [edx+0x44] to Y at 00417d6f. 0x0042F8B9 installs Walker via
    // 0x0042E696, calling exit 0x00417DD7. Exit subtracts one
    // vertical unit (00417de4) and one facing unit (00417df6).
    // Net motion is two cells against gravity, with X unchanged.
    int hoistLift = 2;          // cells up, X untouched

    // At 0x004162FE, fall starts "umbopen" (0x004BF384, strip 12,
    // 8 frames at rate 1.0) on passing the 10.0-unit threshold
    // at 0x004B03E8. 0x0041612F..0x00416186 clamps the sub-step to
    // time left in the strip. 0x004162C2 sets [lemming+0x4b] at
    // completion, then 0x00416038 divides speed by 2.0 (0x004B03F8).
    // Landing during "umbopen" spends the whole descent at fallSpeed.
    int floatOpenMs = 8 * 60;   // 480 ms of `umbopen` at full fall speed
  };

  World() = default;
  // A nested Rules default argument would require the enclosing class to be complete.
  explicit World(LevelSetup setup);
  World(LevelSetup setup, Rules rules);

  // Advance one nominal tick, using the same level clock as the elapsed entry.
  // Check levers during each actor's step. continueHatchesAfterTimeout lets
  // releases continue after timeout, as in 425A95.
  void tick(bool continueHatchesAfterTimeout = false);

  // Advance one frame of dtMs. Split at 1 Hz clock events (0x45CFF9)
  // and subdivide segments that reach multiple simulation ticks. Include a
  // zero-length tail when a frame ends on a clock event or has zero input.
  // Each segment releases hatches, steps actors, then checks tagged
  // contacts and receivers (0x4423D2). Fall and walk spend elapsed
  // seconds. Other actor states advance only at tick boundaries.
  // afterUpdate receives each segment's seconds. False ends the frame early.
  void advanceElapsedMs(int dtMs, bool continueHatchesAfterTimeout = false,
                        const std::function<bool(double)>& afterUpdate = {});
  // Pause hatch timing during level entry while other clocks advance.
  void advanceElapsedMsWithReleaseGate(int dtMs, bool continueHatchesAfterTimeout,
                        bool releaseEnabled, const std::function<bool(double)>& afterUpdate = {});

  std::uint64_t tickCount() const { return tick_; }
  // tickCount() divides the shared level clock by msPerTick().
  // levelSecondEvents() counts the 1 Hz events (0x4458B0).
  std::uint64_t levelClockMs() const { return levelClockMs_; }
  std::uint64_t levelSecondEvents() const { return levelSecondEvents_; }
  // World updates executed through advanceElapsedMs (zero-delta ones included).
  std::uint64_t explicitUpdateCount() const { return explicitUpdates_; }
  static constexpr int kLevelEventPeriodMs = 1000;  // repeating event, period 1000 (0x444D1E)

  // The LEVELMAP.DAT limit is in seconds. Zero means untimed.
  bool timedOut() const {
    return setup_.timeLimitTicks > 0 &&
           tick_ >= static_cast<std::uint64_t>(setup_.timeLimitTicks);
  }
  // Returns zero for both untimed and timed-out levels. Use timedOut() to distinguish them.
  std::uint64_t timeRemainingTicks() const {
    if (setup_.timeLimitTicks <= 0 || timedOut())
      return 0;
    return static_cast<std::uint64_t>(setup_.timeLimitTicks) - tick_;
  }
  const Terrain& terrain() const { return setup_.terrain; }
  Terrain& terrain() { return setup_.terrain; }
  const std::vector<Lemming>& lemmings() const { return lemmings_; }
  const std::vector<EvilRat>& rats() const { return setup_.rats; }
  const LevelSetup& setup() const { return setup_; }
  const Rules& rules() const { return rules_; }

  // Player rate control changes the recurring interval and preserves
  // the countdown, seed, arm phase and tick carry.
  void setReleaseInterval(double seconds, int ticks) {
    if (std::isfinite(seconds) && seconds > 0.0 && ticks > 0) {
      setup_.releaseIntervalSeconds = seconds;
      setup_.releaseIntervalTicks = ticks;
    }
  }

  int spawned() const { return spawned_; }
  int spawnedFromEntrance(std::size_t index) const { return spawnedByEntrance_.at(index); }
  int remainingFromEntrance(std::size_t index) const {
    return nukeRequested_ ? 0 : std::max(0, setup_.entrances.at(index).releaseCount -
                                             spawnedByEntrance_.at(index));
  }
  int saved() const { return saved_; }
  int dead() const { return dead_; }
  int remainingToRelease() const { return nukeRequested_ ? 0 : std::max(0, setup_.totalLemmings - spawned_); }
  bool nukeRequested() const { return nukeRequested_; }
  int nukeCountdownMs() const { return nukeCountdownMs_; }
  int activeCount() const;

  bool finished() const;
  bool won() const { return saved_ >= setup_.saveTarget; }

  // Check skill assignment against the current actor and stock without changing either.
  bool canAssign(std::size_t lemmingIndex, Skill skill) const;
  // Apply a skill and return false when the actor or stock makes it unavailable.
  bool assign(std::size_t lemmingIndex, Skill skill);
  // The lemming nearest a point that a click should affect, if any.
  std::optional<std::size_t> pick(int x, int y, int radius = 4) const;

  // Cancel hatches, count down five seconds, then replace live states oldest
  // first with the existing Bomber explosion. Repeated requests are inert.
  void nuke();

  // Insert a lemming at a supplied cell and state, bypassing hatch release.
  void place(Lemming lemming);

  // Both calls drain the same buffer. takeCues discards named samples.
  std::vector<SoundCue> takeCues();
  std::vector<SoundEvent> takeSoundEvents();

  // At the death transition, report the cells the blast actually removed.
  using DeathObserver = std::function<void(const Lemming&, bool explosion,
                                           std::span<const std::array<int, 2>>)>;
  void setDeathObserver(DeathObserver observer) { deathObserver_ = std::move(observer); }

  // Rat contact and closing gates both request the forced death figure.
  using ForcedDeathObserver = std::function<void(const Lemming&)>;
  void setForcedDeathObserver(ForcedDeathObserver observer) {
    forcedDeathObserver_ = std::move(observer);
  }

  // Report actual removed cells with wrapped X, after the cut and before
  // movement. Notify only for nonempty removal.
  using TerrainCutObserver = std::function<void(const Lemming&,
      std::span<const std::array<int, 2>>)>;
  void setTerrainCutObserver(TerrainCutObserver observer) { terrainCutObserver_ = std::move(observer); }
  // Presentation notification before a refused miner turns; no gameplay result.
  using MinerRefusalObserver = std::function<void(const Lemming&)>;
  void setMinerRefusalObserver(MinerRefusalObserver observer) { minerRefusalObserver_ = std::move(observer); }

  // Notify presentation when a gadget contact activates.
  enum class GadgetUse { TeleportDeparture, TeleportArrival, SpeedPad };
  using GadgetObserver = std::function<void(GadgetUse, std::size_t, const Lemming&)>;
  void setGadgetObserver(GadgetObserver observer) { gadgetObserver_ = std::move(observer); }

private:
  std::size_t clearSkillBox(const Lemming&, int x, int y, int width, int height);
  void cue(SoundCue c, const Lemming* source = nullptr);
  void cueSample(std::string_view sample, const Lemming& source);
  // Optional first-fall budget in seconds for continuous movement.
  // A negative value uses the update's full dt.
  void spawn(double budgetSeconds = -1.0);
  void spawnFromEntrance(std::size_t index, double budgetSeconds);
  void step(Lemming& l);
  void stepFaller(Lemming& l);
  void stepAntiGrav(Lemming& l);
  void stepBounce(Lemming& l);
  void stepDrown(Lemming& l);
  // Species-aware contact with the live receiver volume.
  bool attachFluidHazard(Lemming& l);
  // Live receiver support is separate from terrain walls/skill material.
  bool groundAt(int x, int y, int verticalDir) const;
  void stepWalker(Lemming& l);
  void stepClimber(Lemming& l);
  void stepHoister(Lemming& l);
  void stepBuilder(Lemming& l);
  void stepShrugger(Lemming& l);
  void stepExploder(Lemming& l);
  void stepNuke(int elapsedMs, bool tickBoundary);
  void stepBasher(Lemming& l);
  void stepMiner(Lemming& l);
  void stepDigger(Lemming& l);
  // Milliseconds per nominal simulation tick.
  int msPerTick() const;
  // The nominal tick in seconds, shared by release and movement.
  double tickDtSeconds() const;
  // Continuous movement dt in seconds: the elapsed segment or the nominal tick.
  double motionDtSeconds() const { return explicitUpdate_ ? updateDtSeconds_ : tickDtSeconds(); }
  // The elapsed entry always uses continuous budgets. The tick entry can opt in.
  bool continuousMotion() const { return explicitUpdate_ || setup_.continuousStepperClock; }
  // Hatch release for one update; `tickBoundary` gates only the integer-tick release model.
  void stepRelease(double dt, bool tickBoundary, bool continueHatchesAfterTimeout);
  // One world update of the explicit path (<= one tick boundary), see advanceElapsedMs.
  void updateSegment(int segmentMs, bool continueHatchesAfterTimeout, bool releaseEnabled);
  // Explicit-path lemming step: the full per-tick step on a tick-boundary update, otherwise
  // only continuous movement with this update's dt.
  void stepElapsed(Lemming& l, bool tickBoundary);
  // Tagged lever/pad/sensor contacts AFTER every lemming moved, before receivers (0x4423D2).
  void stepGadgetContacts(double dt);
  void stepOccupancySensor(OccupancySensor& sensor, double dt);
  // Movement samples keep integer-lattice crossings edge-triggered; standing
  // still in the original's dt*speed strip spends no crossing quota.
  struct GateMotion { int x, y; State state; bool active; };
  std::vector<GateMotion> gateMotionSamples() const;
  void stepCounterGates(double dt, std::span<const GateMotion> before);
  bool consumeGateCrush(Lemming&);
  void stepCrabs(double dt);
  void stepNamedHazards(double dt);
  void stepRats(double dt, bool tickBoundary);
  void finishRatAttack(EvilRat& rat);
  // Stable hazard identities: lemming index, or -2-rat index (-1 is none).
  std::vector<Lemming*> hazardActors();
  int hazardActorId(const Lemming* actor) const;
  // The facing/state filter of 0x00436D30 for one lever and one lemming.
  bool leverAccepts(const Lever& lev, const Lemming& l) const;
  // How far a floater still falls at `fallSpeed` after its umbrella opens:
  // `floatOpenMs` of `umbopen` animation. See Rules::floatOpenMs.
  int floatOpenCells() const;
  void explode(Lemming& l);

  bool solidAt(int x, int y) const { return setup_.terrain.solid(x, y); }
  // Enter stamps the terrain field, and exit erases it.
  void stampBlockerField(const Lemming& l);
  void clearBlockerField(const Lemming& l);
  void leaveBlocker(Lemming& l);
  bool atExit(const Lemming& l) const;
  // Closed teleport AABB, as for exits.
  const Teleport* teleportAt(const Lemming& l) const;
  void stepTeleporting(Lemming& l);
  // SpeedPad AABB, facing and gravity filters (FUN_00432742).
  const SpeedPad* speedPadAt(const Lemming& l) const;
  // Lever AABB and facing filter against +0xb8 (0x00436D30).
  // A pull can flip the state through 0x00436E6A.
  Lever* leverAt(const Lemming& l);
  void turn(Lemming& l) { l.facing = l.facing == Facing::Right ? Facing::Left : Facing::Right; }

  LevelSetup setup_;
  Rules rules_;
  std::vector<Lemming> lemmings_;
  std::uint64_t tick_ = 0;
  // Shared level clock in integer milliseconds and current elapsed-update state.
  std::uint64_t levelClockMs_ = 0;
  std::uint64_t levelSecondEvents_ = 0;
  std::uint64_t explicitUpdates_ = 0;
  bool explicitUpdate_ = false;       // True during an advanceElapsedMs update.
  double updateDtSeconds_ = 0.0;      // dt of the update in progress (explicit path)
  int spawned_ = 0;
  int saved_ = 0;
  int dead_ = 0;
  bool nukeRequested_ = false;
  int nukeCountdownMs_ = 0;
  std::size_t nukeCursor_ = 0;
  int sinceSpawn_ = 0;
  // Elapsed release clock state for setups with releaseIntervalSeconds > 0.
  double releaseElapsedSeconds_ = 0.0;  // real seconds since construction
  bool releaseArmed_ = false;
  double releaseCountdown_ = 0.0;
  std::vector<int> spawnedByEntrance_;
  std::vector<SoundEvent> cues_;
  DeathObserver deathObserver_;
  ForcedDeathObserver forcedDeathObserver_;
  TerrainCutObserver terrainCutObserver_;
  MinerRefusalObserver minerRefusalObserver_;
  GadgetObserver gadgetObserver_;
};

}  // namespace webo::sim
