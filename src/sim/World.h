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
  Builder, Basher, Miner, Digger, Exploder, Splatter, Exited, Dead,
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
  int fuse = -1;           // bomber countdown in ticks; -1 when not primed

  bool canClimb = false;   // climber and floater are permanent, not states
  bool canFloat = false;

  bool alive() const { return state != State::Exited && state != State::Dead; }
  bool active() const { return alive() && state != State::Splatter; }
};

// Where lemmings arrive and leave. Positions are in terrain cells.
struct Entrance { int x = 0; int y = 0; };
struct Exit { int x = 0; int y = 0; int width = 8; int height = 8; };

struct LevelSetup {
  Terrain terrain;
  std::vector<Entrance> entrances;
  std::vector<Exit> exits;
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
    // billboard went 34.190, 35.190, 36.190 … one per 200 ms tick, up the
    // column-128 wall and again up the next. One world unit is two mask rows.
    int climbSpeed = 2;         // mask rows per tick

    // READ FROM THE BINARY, not yet confirmed live. The lemming's own fall is
    // the state at vtable 0x004B0A20 -- the one that carries the splat test,
    // the floater check and the umbrella, so it is the state a falling lemming
    // is in. Its speed comes from an accessor, 0x00416038, which branches on
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
    // units per tick, which is 8 mask rows, halving to 4 under the umbrella.
    // The same arithmetic with the climb's 5.0 gives 1 world unit per tick and
    // with the walk's 5.0 gives 1 column per tick, and both of those were
    // measured on the running original -- which is what fixes the units here.
    //
    // An earlier read took 25.0 from the state at 0x004B0C60 and put this at
    // 10. That state is also a faller by its animation, but it has no splat
    // test, no umbrella and no floater check, so it is some other descent and
    // not this one. PLAN.md 2.3 has both.
    int fallSpeed = 8;          // mask rows per tick while falling

    // READ FROM THE BINARY, same accessor: exactly half the fall, because the
    // umbrella path divides by the 2.0 at 0x004B03F8. The flag it branches on
    // is the lemming's [+0x4b], set when the stepper starts the "float"
    // animation at 0x004162C2.
    int floatSpeed = 4;         // mask rows per tick under an umbrella

    // READ FROM THE BINARY. The faller keeps the distance fallen as a double
    // at its own +0x18, in world units, incremented by 1.0 per sub-step at
    // 0x00416318, and tests it twice:
    //
    //   004162d9  dc1de8034b00  fcomp qword ptr [0x004b03e8]  ; 10.0 -> umbrella
    //   004165ff  dc1da8104b00  fcomp qword ptr [0x004b10a8]  ; 30.0 -> splat
    //
    // Ten and thirty world units, so 20 and 60 mask rows. The splat is skipped
    // when the lemming's has-floater byte [+0x49] is set -- which is what our
    // `floating` already stands for, since the umbrella opens well before the
    // splat distance -- and the landing cell's own flags can exempt it too
    // (bit 0x80 is not landable at all). The classic values were 16 and 63.
    int floatAfter = 20;        // umbrella opens this far into the fall
    int splatDistance = 60;     // falling further than this kills

    // PROVISIONAL from here down: classic-Lemmings values, not this engine's.
    //
    // The skill intervals below were looked for in the binary and are not
    // there in this form, which is a finding rather than a gap. `bash`, `dig`
    // and `mine` are one class: all three vtables carry the same slot-1
    // stepper, 0x0041B077, and their enter hooks differ only in the animation
    // they select and the rate they give it. That stepper's own debug strings
    // call the mechanism a "staggered action" -- 0x004BF564 is "Staggered
    // action new frame %d" -- and it fires the removal when the animation
    // reaches a new frame. So what classic Lemmings expresses as a tick
    // interval, this engine expresses as an animation rate living on the
    // controller at the lemming's +0x128, and reading it means reading that
    // controller (0x004683E9 writes a double to its +0x1D8; dig's hook passes
    // 0.75, against the 0.5 its .SPR strip carries). Until then these stay
    // classic.
    //
    // A terrain removal is read, but it is not yet anybody's. FUN_0041B683
    // clears four cells stepped by [+0x44] from the lemming's own cell --
    // loop bound 3 at 0x004B10B8, inclusive from 0 -- with `and 0xf77c` at
    // 0x0041B783 clearing 0x1|0x2|0x80|0x800, plus one further cell with
    // `and 0xe77c`. Four engine cells is four world units, which is eight of
    // our mask rows.
    //
    // It is deliberately not assigned to a field, for a stronger reason than
    // the count being ambiguous: that function is slot 3 of the *trampoline*
    // vtable 0x004B0D08 and has no direct caller in .text, while bash, dig
    // and mine carry different slot-3 entries (0x00401320, 0x00415995,
    // 0x004188C3). So it has not been shown to be the skills' removal at all.
    // Reading what calls it is the next step, and until then neither
    // diggerDepth nor basherReach may be set from it.
    // tools/harness/read_steppers.py prints all of this.
    int maxStepUp = 6;          // a walker climbs steps up to this high
    int builderBricks = 12;
    int brickLength = 6;
    int builderInterval = 8;    // ticks between bricks
    int basherInterval = 4;
    int basherReach = 4;        // pixels removed per bite
    int minerInterval = 6;
    int diggerInterval = 4;
    int diggerDepth = 1;
    // PROVISIONAL in length -- five seconds is the classic fuse and was not
    // measured here -- but now expressed at the measured step rate, so it is
    // five seconds of game time rather than five seconds of a tick rate we
    // had guessed.
    int bomberFuse = 5 * 5;     // ticks
    int explosionRadius = 9;
    int lemmingHeight = 8;      // for head-room checks
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

  // Pending sound cues since the last drain. Cleared by the call.
  std::vector<SoundCue> takeCues();

private:
  void cue(SoundCue c);
  void spawn();
  void step(Lemming& l, std::size_t index);
  void stepFaller(Lemming& l);
  void stepWalker(Lemming& l, std::size_t index);
  void stepClimber(Lemming& l);
  void stepHoister(Lemming& l);
  void stepBuilder(Lemming& l);
  void stepBasher(Lemming& l);
  void stepMiner(Lemming& l);
  void stepDigger(Lemming& l);
  void explode(Lemming& l);

  bool solidAt(int x, int y) const { return setup_.terrain.solid(x, y); }
  bool blockedBy(const Lemming& l, std::size_t self) const;
  bool atExit(const Lemming& l) const;
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
