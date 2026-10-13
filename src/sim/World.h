// The simulation: lemmings, the skills applied to them, and the tick that
// moves everything forward.
//
// Deterministic by construction. There is no clock, no randomness and no
// floating point in the step: the same level plus the same ordered list of
// commands always produces the same outcome, which is what makes the
// differential oracle in PLAN.md 2 usable as a regression test.
//
// IMPORTANT -- the constants below are PROVISIONAL.
//
// They are the well-documented classic-Lemmings values (fall 3px/tick, splat
// above 63px, a six-pixel step-up limit, twelve bricks per builder, and so
// on). Revolution is a Lemmings game and these are the right shape, but it is
// a different engine and they have *not* been checked against it. Pinning them
// down is exactly what the oracle is for: drive the original with the dinput
// shim, read lemming state out of its memory each tick, and diff. Until that
// is done this simulates a Lemmings game faithfully, not *this* Lemmings game.

#pragma once

#include "Terrain.h"

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
};

class World {
public:
  // How fast things happen. Named so the oracle work has something concrete
  // to correct rather than magic numbers scattered through the code.
  //
  // **Every value here is still provisional.** They are chosen to make the
  // simulation play like Lemmings, not because they were read out of the
  // original -- that is Phase 3, and it needs the differential oracle. None
  // of them should be cited as the game's real behaviour.
  //
  // The one timing constant that *is* real lives elsewhere, which is why it
  // is worth naming here: the spawn interval comes from RELEASERATE.PC via
  // webo::ReleaseRates, because the game ships that curve as data. The rule
  // is to read whatever the data states and guess only the rest; these are
  // the rest.
  struct Rules {
    int walkSpeed = 1;          // pixels per tick
    int maxStepUp = 6;          // a walker climbs steps up to this high
    int fallSpeed = 3;          // pixels per tick while falling
    int floatSpeed = 1;         // pixels per tick under an umbrella
    int floatAfter = 16;        // umbrella opens this far into the fall
    int splatDistance = 63;     // falling further than this kills
    int climbSpeed = 1;
    int builderBricks = 12;
    int brickLength = 6;
    int builderInterval = 8;    // ticks between bricks
    int basherInterval = 4;
    int basherReach = 4;        // pixels removed per bite
    int minerInterval = 6;
    int diggerInterval = 4;
    int diggerDepth = 1;
    int bomberFuse = 5 * 20;    // ticks
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
