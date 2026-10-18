// Tests for the simulation.
//
// These check mechanics and determinism, not fidelity: the rule constants are
// provisional until the oracle pins them down (see World.h). What is asserted
// here is that a lemming falls, lands, walks, turns at walls, splats from a
// height, that each skill does the thing it is named for, and that the whole
// thing is reproducible tick for tick.

#include "assets/Box.h"
#include "assets/CollisionGrid.h"
#include "assets/Cylinder.h"
#include "assets/LevelIndex.h"
#include "assets/LevelObjects.h"
#include "assets/SurfaceMesh.h"
#include "sim/World.h"

#include <algorithm>
#include <array>
#include <cstdlib>
#include <filesystem>
#include <format>
#include <iostream>
#include <string>
#include <vector>

namespace {

int g_failures = 0;
int g_checks = 0;

void check(bool cond, std::string_view what) {
  ++g_checks;
  if (!cond) {
    ++g_failures;
    std::cerr << "  FAIL  " << what << "\n";
  }
}

using namespace webo::sim;

// A flat floor across the bottom of a 200x100 world.
LevelSetup flatLevel(int floorY = 80) {
  LevelSetup s;
  s.terrain = Terrain(200, 100);
  s.terrain.fillBox(0, floorY, 200, 100 - floorY, Material::Earth);
  // High enough above the floor that the fall is observable -- three ticks of
  // it -- and low enough that it is survivable. Both bounds are read off
  // Rules rather than written out, because the drop that spans more than one
  // tick is a function of fallSpeed: at the value this file was first written
  // against the ten rows it used to hard-code took four ticks, and at the
  // rate since read off the original (World.h) it lands inside the first one,
  // which is a property of the number and not a bug in the test. A drop from
  // the top of the world would exceed splatDistance, which is correct
  // behaviour but not what most of these tests are about; testSplat sets up a
  // long fall deliberately.
  const World::Rules r;
  const int drop = std::min(r.fallSpeed * 3, r.splatDistance - 1);
  s.entrances.push_back({100, floorY - drop});
  s.totalLemmings = 1;
  s.saveTarget = 1;
  s.releaseIntervalTicks = 1;
  return s;
}

void runFor(World& w, int ticks) {
  for (int i = 0; i < ticks; ++i)
    w.tick();
}

// The world wraps, so a lemming left running long enough circles the cylinder
// and comes back. Asserting on the state after an arbitrary number of ticks
// therefore tests the clock, not the mechanic. These helpers watch what
// happens *during* a run instead.
struct Watch {
  int minY = 1 << 30;
  int maxY = -(1 << 30);
  bool everFacedLeft = false;
  bool everFacedRight = false;
  State everState = State::Dead;
  bool sawState = false;

  void observe(const World& w, State interesting) {
    if (w.lemmings().empty())
      return;
    const Lemming& l = w.lemmings()[0];
    minY = std::min(minY, l.y);
    maxY = std::max(maxY, l.y);
    everFacedLeft |= l.facing == Facing::Left;
    everFacedRight |= l.facing == Facing::Right;
    if (l.state == interesting) {
      sawState = true;
      everState = l.state;
    }
  }
};

Watch runWatching(World& w, int ticks, State interesting = State::Dead) {
  Watch watch;
  for (int i = 0; i < ticks; ++i) {
    w.tick();
    watch.observe(w, interesting);
  }
  return watch;
}

// Advance until the lemming is adjacent to solid ground ahead of it, so a
// skill can be applied where it will actually do something.
bool runUntilAgainstWall(World& w, int maxTicks) {
  for (int i = 0; i < maxTicks; ++i) {
    w.tick();
    if (w.lemmings().empty())
      continue;
    const Lemming& l = w.lemmings()[0];
    if (l.state != State::Walker)
      continue;
    const int dir = static_cast<int>(l.facing);
    if (w.terrain().solid(l.x + dir, l.y - 4))
      return true;
  }
  return false;
}

void testFallAndWalk() {
  std::cout << "falling and walking\n";
  World w(flatLevel());
  runFor(w, 2);
  check(w.spawned() == 1, "a lemming spawns");
  if (w.lemmings().empty())
    return;
  check(w.lemmings()[0].state == State::Faller, "it starts falling");

  runFor(w, 60);
  const Lemming& l = w.lemmings()[0];
  check(l.state == State::Walker, std::format("it lands and walks (state {})", stateName(l.state)));
  check(l.y == 79, std::format("it stands on the floor (y={})", l.y));
  check(l.x > 100, "it walks in its facing direction");
}

void testSplat() {
  std::cout << "splatting\n";
  LevelSetup s = flatLevel(99);   // floor at the very bottom
  s.entrances[0] = {100, 0};      // a long way up
  World w(s);
  runFor(w, 200);
  check(w.dead() == 1, std::format("a long fall kills (dead={})", w.dead()));
  check(w.saved() == 0, "nothing was saved");
}

void testFloater() {
  std::cout << "floater survives the same fall\n";
  LevelSetup s = flatLevel(99);
  s.entrances[0] = {100, 0};
  World w(s);
  w.tick();
  check(!w.lemmings().empty(), "spawned");
  if (w.lemmings().empty())
    return;
  check(w.assign(0, Skill::Floater), "floater applies");
  runFor(w, 400);
  check(w.dead() == 0, std::format("the floater lives (dead={})", w.dead()));
}

void testWallTurn() {
  std::cout << "turning at a wall\n";
  LevelSetup s = flatLevel();
  s.terrain.fillBox(120, 50, 4, 30, Material::Earth);  // a tall wall to the right
  World w(s);
  // Long enough to reach the wall, short enough not to lap the cylinder.
  const Watch watch = runWatching(w, 60);
  check(watch.everFacedRight, "it set off to the right");
  check(watch.everFacedLeft, "and turned around at the wall");
  check(w.lemmings()[0].x < 120, "without passing through it");
}

// maxStepUp, pinned the way it was measured.
//
// Round 4 wrote single-column pillars of exactly 1, 2, 3 and 4 solid cells
// into the original's own collision grid in front of its own walkers and
// watched what its step-up arm (0x0041BFFC) did: it adopted the 1 and the 2
// -- the 2 in a single step -- and turned at the 3, on VH13_MT and again on
// E07_JC_1. This asserts the same three outcomes on the same shape, so a
// later round cannot move the field without this failing.
// docs/scratch/phase3-round4-live.md.
void testStepUpHeight() {
  std::cout << "a walker rises 1 and 2 and turns at 3\n";
  // Written out rather than derived from Rules::maxStepUp, which would make
  // this a tautology that passes at any value of the field. These four rows
  // are what the original did.
  struct Case { int pillar; int highestWalkingY; bool turns; };
  const Case cases[] = {
      {1, 78, false},  // rise 1 adopted
      {2, 77, false},  // rise 2 adopted, in one step
      {3, 79, true},   // a wall
      {4, 79, true},   // a wall, control
  };
  for (const auto& [pillar, wantY, wantTurn] : cases) {
    LevelSetup s = flatLevel();
    // One column, `pillar` cells tall, sitting on the floor at y = 80. The
    // walker's feet are y = 79, so the probed cells are 79 down to 80 - pillar.
    s.terrain.fillBox(130, 80 - pillar, 1, pillar, Material::Earth);
    World w(s);
    // Only the walking samples: the spawn drop is higher than any pillar, so
    // a plain minimum over the run would measure the hatch, not the step.
    int highestWalking = 1 << 30;
    bool turned = false;
    for (int i = 0; i < 70; ++i) {
      w.tick();
      if (w.lemmings().empty())
        continue;  // nothing is released on the first tick
      const Lemming& l = w.lemmings()[0];
      if (l.state == State::Walker)
        highestWalking = std::min(highestWalking, l.y);
      turned |= l.facing == Facing::Left;
    }
    check(highestWalking == wantY,
          std::format("pillar of {}: highest walking y {} (expected {})",
                      pillar, highestWalking, wantY));
    check(turned == wantTurn,
          std::format("pillar of {}: {} at it", pillar,
                      wantTurn ? "turned" : "walked on"));
  }
  const World::Rules r;
  check(r.maxStepUp == 2, "maxStepUp is the 2 the probe writes");
  check(r.lemmingHeight == 3, "lemmingHeight is the 3.0 bias at 0x0042E0FC");
}

void testClimber() {
  std::cout << "climber goes up\n";
  LevelSetup s = flatLevel();
  s.terrain.fillBox(120, 40, 4, 40, Material::Earth);
  World w(s);
  w.tick();
  check(w.assign(0, Skill::Climber), "climber applies");
  const Watch watch = runWatching(w, 200, State::Climber);
  check(watch.sawState, "it entered the climbing state");
  check(watch.minY < 50, std::format("and scaled the wall (highest y reached {})", watch.minY));
  check(!watch.everFacedLeft, "a climber does not turn at the wall");
}

void testClimberTopsOut() {
  std::cout << "climber pulls up onto the ledge\n";
  LevelSetup s = flatLevel();
  // Thick enough that the top is a ledge, not a one-column lip to fall off.
  s.terrain.fillBox(120, 40, 24, 40, Material::Earth);
  World w(s);
  w.tick();
  check(w.assign(0, Skill::Climber), "climber applies on a ledge");
  bool walkedOnTop = false;
  int highest = 1 << 30;
  for (int i = 0; i < 250 && !w.lemmings().empty(); ++i) {
    w.tick();
    const Lemming& l = w.lemmings()[0];
    highest = std::min(highest, l.y);
    if (l.state == State::Walker && l.y < 40 && l.x >= 120 && l.x < 144)
      walkedOnTop = true;
  }
  check(walkedOnTop,
        std::format("it walks on top of the wall (highest y reached {})", highest));
}

// A level with a floor and nothing released: the lemmings are placed, so the
// cells under test are the ones chosen and not wherever a hatch drop landed.
LevelSetup placedLevel() {
  LevelSetup s = flatLevel();
  s.totalLemmings = 0;
  s.releaseIntervalTicks = 1 << 20;
  return s;
}

Lemming walkerAt(int x, Facing f) {
  Lemming l;
  l.x = x;
  l.y = 79;  // feet on flatLevel's floor, whose top solid row is 80
  l.state = State::Walker;
  l.facing = f;
  return l;
}

// Round 10. Three behavioural tests for the three stepper shapes the round
// measured. Every expected number below is WRITTEN OUT from the live series in
// docs/scratch/phase3-round10-steppers.md and mapped once into our downward-Y
// grid; none of them is derived from the Rules field it is testing, because a
// derived expectation passes at any value of the field (round 4's lesson).
//
// The mapping, done once: the engine's grid is Y-up and ours grows downward.
// A wall whose topmost solid row is `t` in our grid has its top surface at
// `t`, a lemming standing on it has its feet at `t - 1`, and the engine's
// "last climbing Y = topSolid - 1" is one row deeper inside the wall, so `t +
// 1` for us. The hoist's "+2 upward" is `y -= 2`.

// The step-down arm: one cell down and a walker, two or more and a faller.
//
// Live (MM_CLIMB_002, holes cut into the pen floor and restored): seven
// lemmings over a one-cell hole at column 34, every one `dy -1.000` with the
// state still `walk`, then `dy +1.000` on the far side; eight over a two-cell
// hole at column 42, every one `walk -> fall`, landing two cells down; and a
// three-cell hole at column 50 that trapped every walker that entered it,
// because a three-cell rise is a wall.
void testWalkerStepsDownOneCell() {
  std::cout << "a walker steps down one cell and falls at two\n";
  // One cell: the floor's top row removed at a single column.
  {
    World w(placedLevel());
    w.terrain().clearBox(140, 80, 1, 1);     // feet can reach y 80 at col 140
    w.place(walkerAt(137, Facing::Right));
    int lowest = -1;
    bool everFell = false;
    bool backUp = false;
    for (int i = 0; i < 8; ++i) {
      w.tick();
      const Lemming& l = w.lemmings()[0];
      lowest = std::max(lowest, l.y);
      everFell |= l.state == State::Faller;
      if (l.x > 140 && l.y == 79)
        backUp = true;
    }
    check(lowest == 80, std::format("it stepped down to y 80 (got {})", lowest));
    check(!everFell, "and never became a faller");
    check(backUp, "then stepped back up on the far side");
  }
  // Two cells: it falls.
  {
    World w(placedLevel());
    w.terrain().clearBox(140, 80, 1, 2);     // rows 80 and 81 gone
    w.place(walkerAt(137, Facing::Right));
    bool everFell = false;
    int lowest = -1;
    for (int i = 0; i < 8; ++i) {
      w.tick();
      const Lemming& l = w.lemmings()[0];
      everFell |= l.state == State::Faller;
      lowest = std::max(lowest, l.y);
    }
    check(everFell, "a two-cell drop becomes a faller");
    check(lowest == 81, std::format("landing two cells down, y 81 (got {})", lowest));
  }
  // Three cells: it falls, and then cannot climb out, because three is a wall.
  {
    World w(placedLevel());
    w.terrain().clearBox(140, 80, 1, 3);
    w.place(walkerAt(137, Facing::Right));
    bool everFell = false;
    for (int i = 0; i < 40; ++i) {
      w.tick();
      everFell |= w.lemmings()[0].state == State::Faller;
    }
    check(everFell, "a three-cell drop becomes a faller");
    check(w.lemmings()[0].y == 82,
          std::format("and is stuck at the bottom, y 82 (got {})",
                      w.lemmings()[0].y));
  }
}

// The climber's column, its last climbing row, and the hoist.
//
// Live: five panel-assigned climbers on MM_CLIMB_002 held X exactly 64.000,
// 80.000, 96.000, 112.000 and 120.000 -- the wall column itself; four
// top-outs came at `topSolid - 1` (46 over 47, 58 over 59, 45 over 46, 61
// over 62); four hoists held 0.840, 0.840, 0.833 and 0.840 s on strip 8; and
// all five placements were `dx 0.000 dy +2.000`.
void testClimbWallColumnTopOutAndHoist() {
  std::cout << "the climber stands on the wall, tops out one cell lower,"
               " and hoists for four ticks\n";
  World w(placedLevel());
  // Columns 120..143 solid from row 40 down, so the topmost solid row is 40.
  w.terrain().fillBox(120, 40, 24, 40, Material::Earth);
  w.place(walkerAt(110, Facing::Right));
  check(w.assign(0, Skill::Climber), "climber applies");

  int climberX = -1;
  int lastClimbingY = -1;
  int hoistTicks = 0;
  int hoistX = -1;
  int walkedOnTopY = -1;
  for (int i = 0; i < 80 && !w.lemmings().empty(); ++i) {
    w.tick();
    const Lemming& l = w.lemmings()[0];
    if (l.state == State::Climber) {
      climberX = l.x;
      lastClimbingY = l.y;              // y falls as it rises; keep the last
    }
    if (l.state == State::Hoister) {
      ++hoistTicks;
      hoistX = l.x;
    }
    if (l.state == State::Walker && walkedOnTopY < 0 && l.y < 40)
      walkedOnTopY = l.y;
  }
  check(climberX == 120,
        std::format("the climber stands ON the wall column 120 (got {})",
                    climberX));
  check(lastClimbingY == 41,
        std::format("its last climbing row is 41, one inside the wall (got {})",
                    lastClimbingY));
  check(hoistTicks == 4,
        std::format("the hoist holds for four ticks (got {})", hoistTicks));
  check(hoistX == 120,
        std::format("and holds X on the wall column (got {})", hoistX));
  check(walkedOnTopY == 39,
        std::format("then it stands on top at y 39 (got {})", walkedOnTopY));
}

// The float halving waits for the `umbopen` animation, so a floater keeps
// falling at full speed for 480 ms after its umbrella opens.
//
// Live, on one lemming given the floater byte as a labelled probe: a 16-unit
// drop never halved at all (19.85 world units/s for the whole 0.805 s), and a
// 28-unit drop halved 0.479 s and 9.564 units after the strip became
// `umbopen`, then ran at 9.972 units/s.
void testFloatHalvingIsDelayed() {
  std::cout << "a floater keeps falling at full speed while umbopen plays\n";
  // A sixteen-cell drop: the original lands before the halving ever starts.
  {
    World w(placedLevel());
    Lemming l;
    l.x = 100;
    l.y = 63;                 // floor's top row is 80, so the feet land at 79
    l.state = State::Faller;
    l.canFloat = true;
    w.place(l);
    const int expect[4] = {67, 71, 75, 79};
    for (int i = 0; i < 4; ++i) {
      w.tick();
      check(w.lemmings()[0].y == expect[i],
            std::format("tick {}: y {} (got {})", i + 1, expect[i],
                        w.lemmings()[0].y));
    }
    w.tick();
    check(w.lemmings()[0].state == State::Walker,
          "and it is walking on the fifth tick");
    check(w.dead() == 0, "nothing died");
  }
  // A twenty-eight-cell drop: four full-speed ticks past the umbrella, then
  // the halving.
  {
    World w(placedLevel());
    Lemming l;
    l.x = 100;
    l.y = 51;
    l.state = State::Faller;
    l.canFloat = true;
    w.place(l);
    const int expect[6] = {55, 59, 63, 67, 71, 73};
    for (int i = 0; i < 6; ++i) {
      w.tick();
      check(w.lemmings()[0].y == expect[i],
            std::format("tick {}: y {} (got {})", i + 1, expect[i],
                        w.lemmings()[0].y));
    }
    check(w.lemmings()[0].state == State::Floater,
          "with the umbrella out by then");
  }
}

// Round 8. The numbers here are WRITTEN OUT from the live measurements, not
// derived from Rules::blockerFieldHeight and not read back out of the
// stamping code -- the round-4 lesson again. Mapped once: the engine's grid is
// Y-up and ours grows downward, so its "feet row + 0 .. 3" is our y down to
// y - 3, and its "turned at world X = C" for a right-facing walker is our
// "never left cell C - 1".
//
//   blocker (170.066, 78.000) -> cells (170, 78) (170, 79) (170, 80) (170, 81)
//   field column 170, five right-facing walkers turned at X = 170.000
//   field column 105, a left-facing walker turned at X = 106.000
//   field column 76,  four left-facing walkers turned at X =  77.000
//   bombing the (170, 78..81) blocker cleared exactly those four cells
//
// docs/scratch/phase3-round8-blocker.md.
void testBlockerField() {
  std::cout << "a blocker stamps one column of four cells\n";
  World w(placedLevel());
  w.place(walkerAt(130, Facing::Right));
  const std::size_t solidBefore = w.terrain().solidCells();
  check(w.terrain().blockerFieldCells() == 0, "no field before the blocker");
  check(w.assign(0, Skill::Blocker), "blocker applies");
  check(w.lemmings()[0].state == State::Blocker, "it is blocking");
  check(w.terrain().blockerFieldCells() == 4,
        std::format("four cells stamped (got {})",
                    w.terrain().blockerFieldCells()));
  for (int y : {79, 78, 77, 76})
    check(w.terrain().blockerField(130, y),
          std::format("(130, {}) carries the field", y));
  check(!w.terrain().blockerField(130, 75), "(130, 75) does not -- four rows");
  check(!w.terrain().blockerField(130, 80), "(130, 80) does not -- the floor");
  check(!w.terrain().blockerField(129, 79) && !w.terrain().blockerField(131, 79),
        "one column only: 129 and 131 are clear at the feet row");
  check(w.terrain().solidCells() == solidBefore,
        "the field is not terrain: no cell became solid");
  check(!w.terrain().solid(130, 79),
        "and the blocker's own cell is still air");
}

void testBlockerTurnsWalkers() {
  std::cout << "a walker turns on the blocker's own column, both ways\n";
  // Approaching from the left, facing right: it turns ON cell 130 and must
  // never reach 131.
  //
  // Round 8 wrote 129 here. The probe column it measured is unchanged -- it
  // is the field's own cell, 130, either way -- but the STORED column moved
  // in round 10, which measured where the original's X is when the probe
  // runs: the shared edge of the two cells, and truncation gives that edge to
  // the right-hand one. Round 8's own live numbers already said so --
  // "field column 170 -> five right-facing walkers turned at X = 170.000" --
  // and round 10 reproduced it on plain terrain walls (right-facing turns at
  // X exactly 64.000 against a wall at column 64, left-facing at X exactly
  // 24.000 against a wall at column 23). See World::stepWalker.
  {
    World w(placedLevel());
    w.place(walkerAt(130, Facing::Right));
    check(w.assign(0, Skill::Blocker), "blocker applies (right-hand case)");
    w.place(walkerAt(120, Facing::Right));
    int furthest = -1;
    int turnedAt = -1;
    for (int i = 0; i < 40; ++i) {
      w.tick();
      const Lemming& l = w.lemmings()[1];
      furthest = std::max(furthest, l.x);
      if (turnedAt < 0 && l.facing == Facing::Left)
        turnedAt = l.x;
    }
    check(turnedAt == 130, std::format("turned in cell 130 (got {})", turnedAt));
    check(furthest == 130,
          std::format("never went past the blocker's column: furthest {}",
                      furthest));
    check(w.lemmings()[1].state == State::Walker, "and it is still walking");
  }
  // Approaching from the right, facing left: it stops one cell further out,
  // because the probe column is cell + facing either way.
  {
    World w(placedLevel());
    w.place(walkerAt(130, Facing::Right));
    check(w.assign(0, Skill::Blocker), "blocker applies (left-hand case)");
    w.place(walkerAt(140, Facing::Left));
    int nearest = 1 << 30;
    int turnedAt = -1;
    for (int i = 0; i < 40; ++i) {
      w.tick();
      const Lemming& l = w.lemmings()[1];
      nearest = std::min(nearest, l.x);
      if (turnedAt < 0 && l.facing == Facing::Right)
        turnedAt = l.x;
    }
    check(turnedAt == 131, std::format("turned in cell 131 (got {})", turnedAt));
    check(nearest == 131,
          std::format("never entered the blocker's column: nearest {}", nearest));
  }
  // The blocker itself is not a wall: with no field there, the same walker
  // walks straight past the same cell. This is the control that separates the
  // field from the lemming.
  {
    World w(placedLevel());
    w.place(walkerAt(130, Facing::Right));   // left a walker, never blocking
    w.place(walkerAt(120, Facing::Right));
    int furthest = -1;
    for (int i = 0; i < 40; ++i) {
      w.tick();
      furthest = std::max(furthest, w.lemmings()[1].x);
    }
    check(furthest > 130,
          std::format("with no blocker it walks past 130 (reached {})", furthest));
  }
}

void testBlockerFieldIsErased() {
  std::cout << "leaving the block state erases the field\n";
  // The bomb. MEASURED: the four cells cleared 2 ms after the state changed
  // to explode, and the crater came a second later.
  {
    World w(placedLevel());
    w.place(walkerAt(130, Facing::Right));
    check(w.assign(0, Skill::Blocker), "blocker applies");
    check(w.assign(0, Skill::Bomber), "and takes a bomb");
    runFor(w, 60);
    check(w.lemmings()[0].state == State::Dead, "it exploded");
    check(w.terrain().blockerFieldCells() == 0,
          std::format("the field is gone (left {})",
                      w.terrain().blockerFieldCells()));
  }
  // Losing the ground. The block step's own route out, 0x0042E913 returning 0.
  {
    World w(placedLevel());
    w.place(walkerAt(130, Facing::Right));
    check(w.assign(0, Skill::Blocker), "blocker applies over a floor");
    w.terrain().clearBox(128, 80, 5, 20);
    w.tick();
    check(w.lemmings()[0].state == State::Faller, "it fell when the floor went");
    check(w.terrain().blockerFieldCells() == 0, "and took its field with it");
  }
  // Another skill. 0x0042E696 runs the outgoing hook whatever replaces it.
  {
    World w(placedLevel());
    w.place(walkerAt(130, Facing::Right));
    check(w.assign(0, Skill::Blocker), "blocker applies");
    check(w.assign(0, Skill::Digger), "then a digger over it");
    check(w.terrain().blockerFieldCells() == 0,
          "the field went with the block state");
  }
  // And a walker is no longer turned once the field is gone.
  {
    World w(placedLevel());
    w.place(walkerAt(130, Facing::Right));
    check(w.assign(0, Skill::Blocker), "blocker applies");
    w.place(walkerAt(120, Facing::Right));
    runFor(w, 12);
    check(w.lemmings()[1].facing == Facing::Left, "the walker was turned");
    check(w.assign(0, Skill::Digger), "the blocker becomes a digger");
    World w2(placedLevel());
    w2.place(walkerAt(120, Facing::Right));
    int furthest = -1;
    for (int i = 0; i < 40; ++i) {
      w2.tick();
      furthest = std::max(furthest, w2.lemmings()[0].x);
    }
    check(furthest > 130,
          std::format("and with no field a walker passes 130 (reached {})",
                      furthest));
  }
  const World::Rules r;
  check(r.blockerFieldHeight == 4,
        "blockerFieldHeight is the four rows the 0x004B10B8 dword 3 gives");
}

// Round 9. A walker whose ground cell carries the anti-grav bit flips
// verticalDir and falls the other way. The expectation is a tick-by-tick
// series, not a field readback: without the flip the walker stays at
// verticalDir +1 and its y never decreases through a fall.
// Round 13. A faller whose landing cell carries the trampoline bit enters
// Bounce and rises (y decreases in this Y-down sim) instead of walking or
// splatting. Mutate-alone: delete the trampoline() branch in stepFaller and
// this fails — the faller walks.
void testTrampolineBounce() {
  std::cout << "trampoline pad bounces a faller\n";
  LevelSetup s;
  s.terrain = Terrain(80, 60);
  s.terrain.fillBox(0, 40, 80, 20, Material::Earth);
  // Open air above the floor — no ceiling so the rise is not head-clipped.
  s.terrain.setTrampoline(40, 40, true);
  check(s.terrain.trampolineCells() == 1, "one trampoline cell");
  check(s.terrain.trampoline(40, 40), "pad is at (40, 40)");
  s.totalLemmings = 0;
  s.saveTarget = 0;
  World w(s);
  Lemming faller;
  faller.x = 40;
  faller.y = 20;
  faller.facing = Facing::Right;
  faller.state = State::Faller;
  faller.verticalDir = 1;
  faller.fallDistance = 0;
  w.place(faller);

  bool sawBounce = false;
  int minY = faller.y;
  int yAtBounce = -1;
  for (int i = 0; i < 40; ++i) {
    w.tick();
    const Lemming& l = w.lemmings()[0];
    if (l.state == State::Bounce) {
      if (!sawBounce)
        yAtBounce = l.y;
      sawBounce = true;
      minY = std::min(minY, l.y);
    }
  }
  check(sawBounce, "faller entered Bounce on the trampoline cell");
  check(yAtBounce == 39,
        std::format("bounce starts on the floor cell above the pad (y={})",
                    yAtBounce));
  check(minY < yAtBounce,
        std::format("bounce rises (y {} -> {})", yAtBounce, minY));
  // Without the pad the same fall walks. Pin the detection, not the motion.
  LevelSetup plain = s;
  plain.terrain.setTrampoline(40, 40, false);
  World w2(plain);
  Lemming f2 = faller;
  w2.place(f2);
  bool bounced2 = false;
  for (int i = 0; i < 40; ++i) {
    w2.tick();
    if (w2.lemmings()[0].state == State::Bounce)
      bounced2 = true;
  }
  check(!bounced2, "no trampoline bit means no bounce");
}

// Round 14. Ground cell bit 0x40 attaches Drown; after drownMs the lemming
// is Dead. Mutate-alone: delete the waterHazard() branch in World::step and
// this fails — the faller walks. Touch World.cpp after restoring so ninja
// rebuilds.
void testWaterHazardDrown() {
  std::cout << "water hazard drowns a faller\n";
  LevelSetup s;
  s.terrain = Terrain(80, 60);
  s.terrain.fillBox(0, 40, 80, 20, Material::Earth);
  s.terrain.setWaterHazard(40, 40, true);
  check(s.terrain.waterHazardCells() == 1, "one water cell");
  check(s.terrain.waterHazard(40, 40), "hazard is at (40, 40)");
  s.totalLemmings = 0;
  s.saveTarget = 0;
  World w(s);
  Lemming faller;
  faller.x = 40;
  faller.y = 20;
  faller.facing = Facing::Right;
  faller.state = State::Faller;
  faller.verticalDir = 1;
  faller.fallDistance = 0;
  w.place(faller);

  bool sawDrown = false;
  int drownTick = -1;
  int deadTick = -1;
  for (int i = 0; i < 40; ++i) {
    w.tick();
    const Lemming& l = w.lemmings()[0];
    if (l.state == State::Drown && !sawDrown) {
      sawDrown = true;
      drownTick = i;
    }
    if (l.state == State::Dead && deadTick < 0)
      deadTick = i;
  }
  check(sawDrown, "faller entered Drown on the water cell");
  check(deadTick > drownTick,
        std::format("drown then dead (drown@{} dead@{})", drownTick, deadTick));
  // Attach tick sets Drown frame=0; then 10 ticks of stepDrown reach
  // drownMs=2000 and die. deadTick - drownTick == 10.
  check(deadTick - drownTick == 10,
        std::format("drown lasts 10 ticks / 2.0 s (delta={})",
                    deadTick - drownTick));
  check(w.dead() == 1, "drown increments dead count");

  LevelSetup plain = s;
  plain.terrain.setWaterHazard(40, 40, false);
  World w2(plain);
  Lemming f2 = faller;
  w2.place(f2);
  bool drowned2 = false;
  for (int i = 0; i < 40; ++i) {
    w2.tick();
    if (w2.lemmings()[0].state == State::Drown)
      drowned2 = true;
  }
  check(!drowned2, "no 0x40 bit means no drown");
}

// Round 15. Acid-as-hazard is the same 0x40 → Drown path (no State::Acid,
// no separate duration). Live: paint 0x40 on H02_JC_1 (^acid) and under
// Entrance_acid [+0x4c]=1 walkers on H23_mg_1 → drown stype 0x22, 0 type2e.
// Mutate-alone: drownMs 2000→200 in World.h — this duration check fails.
// Touch World.h after restore so ninja rebuilds.
void testAcidHazardSharesDrown() {
  std::cout << "acid hazard shares water drown (0x40)\n";
  LevelSetup s;
  s.terrain = Terrain(80, 60);
  s.terrain.fillBox(0, 40, 80, 20, Material::Earth);
  s.terrain.setWaterHazard(40, 40, true);
  s.totalLemmings = 0;
  s.saveTarget = 0;
  World w(s);
  // Round 15 live: hatch type 1 (entrance_acid) → [+0x4c]=1 still drowns
  // on 0x40 (H23_mg_1 paint). Shared bit/duration is the claim.
  Lemming faller;
  faller.x = 40;
  faller.y = 20;
  faller.facing = Facing::Right;
  faller.state = State::Faller;
  faller.verticalDir = 1;
  faller.fallDistance = 0;
  w.place(faller);

  int drownTick = -1;
  int deadTick = -1;
  for (int i = 0; i < 40; ++i) {
    w.tick();
    const Lemming& l = w.lemmings()[0];
    if (l.state == State::Drown && drownTick < 0)
      drownTick = i;
    if (l.state == State::Dead && deadTick < 0)
      deadTick = i;
  }
  check(drownTick >= 0, "0x40 cell attaches Drown (acid shares this path)");
  check(deadTick - drownTick == 10,
        std::format("shared drownMs=2000 → 10 ticks (delta={})",
                    deadTick - drownTick));
  check(w.dead() == 1, "shared drown increments dead");
}

// Round 16. A walker whose feet sit in a teleport AABB relocates to the
// partner pad after teleportDelayTicks. Mutate-alone: delete the
// teleportAt() arm in World::step and this fails — the walker stays put.
// Touch World.cpp after restoring so ninja rebuilds.
void testTeleportPad() {
  std::cout << "teleport pad relocates a walker\n";
  LevelSetup s;
  s.terrain = Terrain(256, 96);
  s.terrain.fillBox(0, 50, 256, 46, Material::Earth);
  // Source AABB covering (80, 40) in terrain space; dest at (200, 40).
  s.teleports.push_back({74, 32, 12, 8, 200, 40});
  s.teleports.push_back({194, 32, 12, 8, 80, 40});
  s.totalLemmings = 0;
  s.saveTarget = 0;
  World w(s);
  Lemming walker;
  walker.x = 80;
  walker.y = 40;
  walker.facing = Facing::Left;
  walker.state = State::Walker;
  walker.verticalDir = 1;
  w.place(walker);

  bool sawTeleport = false;
  for (int i = 0; i < 20; ++i) {
    w.tick();
    const Lemming& l = w.lemmings()[0];
    if (l.state == State::Teleporting)
      sawTeleport = true;
    if (l.state == State::Walker && l.x == 200 && l.y == 40) {
      check(sawTeleport, "entered Teleporting before the snap");
      check(l.facing == Facing::Left, "facing preserved across teleport");
      // Without pads the same walker never reaches column 200 in 20 ticks.
      LevelSetup plain = s;
      plain.teleports.clear();
      World w2(plain);
      Lemming wkr = walker;
      w2.place(wkr);
      bool jumped = false;
      for (int j = 0; j < 20; ++j) {
        w2.tick();
        if (w2.lemmings()[0].x == 200)
          jumped = true;
      }
      check(!jumped, "no teleport list means no relocate");
      return;
    }
  }
  check(false, "walker relocated to partner (200, 40) within 20 ticks");
}

// Round 17. A walker whose feet sit in a SpeedPad AABB gets a temporary
// 2× walk (FASTER). Mutate-alone: delete the speedPadAt() arm in
// World::step, or set Rules::fasterMs = 0, and this fails. Touch World.cpp
// after restoring so ninja rebuilds.
void testSpeedPadFaster() {
  std::cout << "speed pad doubles walk rate for fasterMs\n";
  LevelSetup s;
  s.terrain = Terrain(256, 96);
  s.terrain.fillBox(0, 50, 256, 46, Material::Earth);
  // AABB covering feet at (80, 49): x=79 width=2 → [79..81], y=46 height=6.
  s.speedPads.push_back({79, 46, 2, 6, 0, true});
  s.totalLemmings = 0;
  s.saveTarget = 0;
  World w(s);
  Lemming walker;
  walker.x = 80;
  walker.y = 49;
  walker.facing = Facing::Right;
  walker.state = State::Walker;
  walker.verticalDir = 1;
  w.place(walker);

  w.tick();
  check(w.lemmings()[0].fasterMs == w.rules().fasterMs,
        "FASTER timer armed on pad enter");
  const int xAfter1 = w.lemmings()[0].x;
  check(xAfter1 == 82, "first boosted tick walks two columns (80->82)");

  LevelSetup plain = s;
  plain.speedPads.clear();
  World w2(plain);
  Lemming wkr = walker;
  w2.place(wkr);
  w2.tick();
  check(w2.lemmings()[0].fasterMs == 0, "no pad means no boost");
  check(w2.lemmings()[0].x == 81, "plain walker advances one column");

  // Duration: 5000 ms / 200 ms = 25 ticks of boost after the arming tick.
  World w3(s);
  Lemming w3l = walker;
  w3.place(w3l);
  int boostedTicks = 0;
  for (int i = 0; i < 40; ++i) {
    w3.tick();
    if (w3.lemmings()[0].fasterMs > 0)
      ++boostedTicks;
  }
  check(boostedTicks == 25, "fasterMs lasts 25 ticks at 200 ms");
}

// Round 21. A walker whose feet sit in a Lever AABB with the facing that
// matches +0xb8 flips Lever::state to that facing. No lemming attach.
// Mutate-alone: delete the leverAt() arm in World::step and this fails.
// Touch World.cpp after restoring so ninja rebuilds.
void testLeverToggle() {
  std::cout << "lever toggles state on facing-gated AABB pull\n";
  LevelSetup s;
  s.terrain = Terrain(256, 96);
  s.terrain.fillBox(0, 50, 256, 46, Material::Earth);
  // AABB covering feet at (80, 49): x=79 width=2 → [79..81], y=46 height=6.
  s.levers.push_back({79, 46, 2, 6, 1});  // state 1 → needs face Left
  s.totalLemmings = 0;
  s.saveTarget = 0;
  World w(s);
  Lemming walker;
  walker.x = 80;
  walker.y = 49;
  walker.facing = Facing::Left;
  walker.state = State::Walker;
  walker.verticalDir = 1;
  w.place(walker);

  w.tick();
  check(w.setup().levers[0].state == -1,
        "pull left flips lever state 1 -> -1");
  check(w.lemmings()[0].state == State::Walker,
        "lever does not change lemming state");
  check(w.lemmings()[0].fasterMs == 0, "lever is not a SpeedPad");

  // Same facing again must not re-fire (filter now needs face +1).
  w.tick();
  check(w.setup().levers[0].state == -1,
        "same facing does not re-toggle");

  // Turn around and pull the other way.
  Lemming& live = const_cast<Lemming&>(w.lemmings()[0]);
  live.facing = Facing::Right;
  live.x = 80;
  live.y = 49;
  live.state = State::Walker;
  w.tick();
  check(w.setup().levers[0].state == 1,
        "pull right flips lever state -1 -> 1");

  LevelSetup plain = s;
  plain.levers.clear();
  World w2(plain);
  Lemming wkr = walker;
  w2.place(wkr);
  w2.tick();
  check(w2.setup().levers.empty(), "no lever means nothing to toggle");
}

// Round 18. Balloon is a registered SFX cue with no retail play site and
// no world gadget. Mutate-alone: set kBalloonGadgetInRetail = true in
// World.h and this fails. Touch World.h after restoring so ninja rebuilds.
void testBalloonCueUnused() {
  std::cout << "balloon cue is unused in retail (clean negative)\n";
  check(!kBalloonGadgetInRetail,
        "Balloon has no lemming effect to simulate");
}

// Round 19. Fire is a registered SFX cue with no retail play site and
// no world gadget. Mutate-alone: set kFireGadgetInRetail = true in
// World.h and this fails. Touch World.h after restoring so ninja rebuilds.
void testFireCueUnused() {
  std::cout << "fire cue is unused in retail (clean negative)\n";
  check(!kFireGadgetInRetail,
        "Fire has no lemming effect to simulate");
}

// Round 20. Witch is a registered SFX cue with one UI play site (e10_kf_1
// pause easter egg) and no lemming gadget. Mutate-alone: set
// kWitchGadgetInRetail = true in World.h and this fails. Touch World.h
// after restoring so ninja rebuilds.
void testWitchCueNoLemmingsGadget() {
  std::cout << "witch cue has no lemming gadget (UI easter egg only)\n";
  check(!kWitchGadgetInRetail,
        "Witch has no lemming effect to simulate");
}

// Round 22. ForceF is a registered SFX cue with no retail play site and
// no world gadget. Mutate-alone: set kForceFGadgetInRetail = true in
// World.h and this fails. Touch World.h after restoring so ninja rebuilds.
void testForceFCueUnused() {
  std::cout << "forcef cue is unused in retail (clean negative)\n";
  check(!kForceFGadgetInRetail,
        "ForceF has no lemming effect to simulate");
}

// Round 23. Axe is a HUD sprite sheet (SPRITES\AXE.SPR) with one UI bind
// site and no lemming gadget. Mutate-alone: set kAxeGadgetInRetail = true
// in World.h and this fails. Touch World.h after restoring so ninja
// rebuilds.
void testAxeSheetNoLemmingsGadget() {
  std::cout << "axe sheet has no lemming gadget (HUD sprite only)\n";
  check(!kAxeGadgetInRetail,
        "Axe has no lemming effect to simulate");
}

// Round 24. Bubble is a registered SFX cue played from type-0x23 fluid-
// mismatch state step; no Bubble world gadget / no bubble.a3d.
// Mutate-alone: set kBubbleGadgetInRetail = true in World.h and this
// fails. Touch World.h after restoring so ninja rebuilds.
void testBubbleCueNoGadget() {
  std::cout << "bubble cue has no world gadget (SFX on type-0x23)\n";
  check(!kBubbleGadgetInRetail,
        "Bubble has no exclusive gadget to simulate");
}

// Round 25. Scrape is a registered SFX cue played from dig-bite and
// bash cell-helper steel/blocker refusal arms; no scrape.a3d / no
// Scrape gadget. Mutate-alone: set kScrapeGadgetInRetail = true in
// World.h and this fails. Touch World.h after restoring so ninja
// rebuilds.
void testScrapeCueNoGadget() {
  std::cout << "scrape cue has no world gadget (dig/bash refuse SFX)\n";
  check(!kScrapeGadgetInRetail,
        "Scrape has no exclusive gadget to simulate");
}

// Round 26. Rumble is a registered SFX cue played from object step
// FUN_00423749 (vtable 0x004B0F00) on deferred state commit; no
// rumble.a3d / no Rumble gadget. Mutate-alone: set
// kRumbleGadgetInRetail = true in World.h and this fails. Touch
// World.h after restoring so ninja rebuilds.
void testRumbleCueNoGadget() {
  std::cout << "rumble cue has no world gadget (object state-commit SFX)\n";
  check(!kRumbleGadgetInRetail,
        "Rumble has no exclusive gadget to simulate");
}

// Round 27. Changeop is a registered SFX cue played from skill-panel
// select FUN_0045ac5c and options SFX-slider FUN_0048e8d5; no
// changeop.a3d / no Changeop gadget. Mutate-alone: set
// kChangeopGadgetInRetail = true in World.h and this fails. Touch
// World.h after restoring so ninja rebuilds.
void testChangeopCueNoGadget() {
  std::cout << "changeop cue has no world gadget (UI skill/volume SFX)\n";
  check(!kChangeopGadgetInRetail,
        "Changeop has no exclusive gadget to simulate");
}

// Round 28. Die is a registered SFX cue played solely from type-0x23
// enter FUN_00404261 when fluid+0x128==1; no die.a3d / no Die gadget.
// Distinct from FallSplat (type 0x1a splat enter) and unused Splat.
// Mutate-alone: set kDieGadgetInRetail = true in World.h and this
// fails. Touch World.h after restoring so ninja rebuilds.
// Round 29. Matching zone + sheetType skips splat after a long fall.
// Live: MM_W_012 type-1 + 0x200 + raise-acc 31 → walk; H23 type-2 + paint
// 0x4000 + raise-acc → walk; type-2 onto 0x3 without zone → splat.
// Mutate-alone: delete the splatExempt() guard in stepFaller — sheet-1
// on zone water then dies. Touch World.cpp after restore so ninja rebuilds.
void testSplatZoneExempt() {
  std::cout << "zone + sheetType skips splat\n";
  LevelSetup s;
  s.terrain = Terrain(80, 60);
  s.terrain.fillBox(0, 40, 80, 20, Material::Earth);
  s.terrain.setZoneWater(40, 40, true);
  s.totalLemmings = 0;
  s.saveTarget = 0;

  auto run = [&](int sheet, bool expectWalk) {
    World w(s);
    Lemming faller;
    faller.x = 40;
    faller.y = 5;
    faller.facing = Facing::Right;
    faller.state = State::Faller;
    faller.verticalDir = 1;
    faller.fallDistance = 0;
    faller.sheetType = sheet;
    w.place(faller);
    for (int i = 0; i < 50; ++i)
      w.tick();
    const Lemming& l = w.lemmings()[0];
    if (expectWalk) {
      check(l.state == State::Walker || l.state == State::Faller,
            std::format("sheet {} should survive (state={})", sheet,
                        static_cast<int>(l.state)));
      check(l.state != State::Splatter, "sheet+zone must not splat");
      check(w.dead() == 0, "no dead when exempt");
    } else {
      check(l.state == State::Splatter || w.dead() == 1,
            std::format("sheet {} without match should splat", sheet));
    }
  };

  // Drop from y=5 onto floor 40 is 35 cells > splatDistance 30.
  run(1, true);   // LemmingsWater + 0x200
  run(0, false);  // ordinary sheet, same zone — splat
  run(2, false);  // acid sheet on water zone — splat

  s.terrain.setZoneWater(40, 40, false);
  s.terrain.setZoneAcid(40, 40, true);
  run(2, true);   // LemmingsAcid + 0x4000
  run(1, false);  // water sheet on acid zone — splat
}

void testDieCueNoGadget() {
  std::cout << "die cue has no world gadget (SFX on type-0x23 enter)\n";
  check(!kDieGadgetInRetail,
        "Die has no exclusive gadget to simulate");
}

void testAntiGravFlipsGravity() {
  std::cout << "anti-grav pad flips gravity\n";
  LevelSetup s;
  s.terrain = Terrain(80, 60);
  // Floor at y 40..59, ceiling solid at y 10..14, open shaft between.
  s.terrain.fillBox(0, 40, 80, 20, Material::Earth);
  s.terrain.fillBox(0, 10, 80, 5, Material::Steel);
  // Pad under the floor the walker will stand on, at column 50.
  s.terrain.setAntiGrav(50, 40, true);
  check(s.terrain.antiGravCells() == 1, "one anti-grav cell");
  check(s.terrain.antiGrav(50, 40), "pad is at (50, 40)");
  // No entrance release -- place a walker on the floor just left of the pad.
  s.totalLemmings = 0;
  s.saveTarget = 0;
  World w(s);
  Lemming walker;
  walker.x = 48;
  walker.y = 39;
  walker.facing = Facing::Right;
  walker.state = State::Walker;
  walker.verticalDir = 1;
  w.place(walker);

  std::vector<std::tuple<int, int, int, State>> series;
  for (int i = 0; i < 20; ++i) {
    w.tick();
    const Lemming& l = w.lemmings()[0];
    series.push_back({l.x, l.y, l.verticalDir, l.state});
  }

  int flipTick = -1;
  for (int i = 0; i < static_cast<int>(series.size()); ++i) {
    if (std::get<2>(series[static_cast<std::size_t>(i)]) == -1) {
      flipTick = i;
      break;
    }
  }
  check(flipTick >= 0, "verticalDir became -1 within 20 ticks");
  if (flipTick >= 0) {
    const auto& at = series[static_cast<std::size_t>(flipTick)];
    check(std::get<0>(at) == 50,
          std::format("flip is on the pad column (x={})", std::get<0>(at)));
    check(std::get<3>(at) == State::Faller,
          "flip puts the lemming into Faller");
    // After the flip, y must decrease (fall toward the ceiling).
    int minY = std::get<1>(at);
    for (int i = flipTick; i < static_cast<int>(series.size()); ++i)
      minY = std::min(minY, std::get<1>(series[static_cast<std::size_t>(i)]));
    check(minY < std::get<1>(at),
          std::format("inverted fall decreases y (from {} down to {})",
                      std::get<1>(at), minY));
  }

  // Behaviour gate: the same setup with the pad cleared must never invert.
  s.terrain.setAntiGrav(50, 40, false);
  World w2(s);
  w2.place(walker);
  bool everInverted = false;
  int minY2 = walker.y;
  for (int i = 0; i < 20; ++i) {
    w2.tick();
    const Lemming& l = w2.lemmings()[0];
    everInverted |= l.verticalDir < 0;
    minY2 = std::min(minY2, l.y);
  }
  check(!everInverted, "without the pad, verticalDir stays +1");
  check(minY2 >= walker.y,
        "without the pad, y never falls upward");
}

// Round 30. Inverted step-up / dig refuse. Live on stock exe: m06_kf_1
// inverted walkers adopted dy -1 / -2 (Y-up) on hangs painted under the
// ceiling; forced-vdir diggers refused painted 0x21 and cleared painted
// 0x11 with Y += 1 per cycle. docs/scratch/phase3-round30-inverted.md.
void testInvertedStepUp() {
  std::cout << "inverted walker rises 1 and 2 against gravity\n";
  struct Case {
    int hang;
    int wantY;
  };
  // Y-down: ceiling top-solid row 40, feet at 41, hang of `hang` cells
  // toward +y. Adopting rise k moves feet to 41 + k.
  const Case cases[] = {{1, 42}, {2, 43}};
  for (const auto& [hang, wantY] : cases) {
    LevelSetup s = placedLevel();
    s.terrain.fillBox(0, 40, 200, 1, Material::Earth);  // ceiling
    s.terrain.fillBox(50, 41, 1, hang, Material::Earth);
    World w(s);
    Lemming walker = walkerAt(48, Facing::Right);
    walker.y = 41;
    walker.verticalDir = -1;
    w.place(walker);
    int maxWalking = -1;
    for (int i = 0; i < 40; ++i) {
      w.tick();
      const Lemming& l = w.lemmings()[0];
      if (l.state == State::Walker)
        maxWalking = std::max(maxWalking, l.y);
    }
    check(maxWalking == wantY,
          std::format("inverted hang {}: max walking y {} (expected {})", hang,
                      maxWalking, wantY));
  }
}

void testInvertedDigRefuseAndClear() {
  std::cout << "inverted digger refuses 0x20, clears 0x10, steps toward gravity\n";
  // Refuse: ceiling dig-row stamped noDigInverted.
  {
    LevelSetup s = placedLevel();
    s.terrain = Terrain(200, 100, Material::Empty);
    s.terrain.fillBox(40, 49, 20, 1, Material::Earth);
    for (int x = 40; x < 60; ++x)
      s.terrain.setNoDigInverted(x, 49, true);
    check(s.terrain.digRefused(50, 49, -1),
          "fixture: digRefused(-1) on painted 0x20 cell");
    World w(s);
    Lemming d = walkerAt(50, Facing::Right);
    d.y = 50;
    d.verticalDir = -1;
    w.place(d);
    check(w.terrain().digRefused(50, 49, -1),
          "fixture after World: digRefused still set");
    check(w.assign(0, Skill::Digger), "inverted digger applies");
    const std::size_t before = w.terrain().solidCells();
    const int y0 = w.lemmings()[0].y;
    State afterBite = State::Digger;
    int yAfter = y0;
    for (int i = 0; i < 30; ++i) {
      w.tick();
      const Lemming& l = w.lemmings()[0];
      if (l.state != State::Digger || l.y != y0) {
        afterBite = l.state;
        yAfter = l.y;
        break;
      }
    }
    check(w.terrain().solidCells() == before,
          std::format("0x20 cells are not cleared while inverted (solids {} -> {})",
                      before, w.terrain().solidCells()));
    check(yAfter == y0,
          std::format("refused digger does not step Y ({} -> {}, state {})",
                      y0, yAfter, static_cast<int>(afterBite)));
    check(afterBite == State::Walker,
          std::format("refused digger returns to walk (state {})",
                      static_cast<int>(afterBite)));
  }
  // Clear: same cells stamped noDigUpright only — inverted mask is 0x20.
  {
    LevelSetup s = placedLevel();
    s.terrain = Terrain(200, 100, Material::Empty);
    s.terrain.fillBox(40, 46, 20, 4, Material::Earth);
    for (int y = 46; y < 50; ++y)
      for (int x = 40; x < 60; ++x)
        s.terrain.setNoDigUpright(x, y, true);
    check(!s.terrain.digRefused(50, 49, -1),
          "fixture: 0x10 does not refuse inverted dig");
    World w(s);
    Lemming d = walkerAt(50, Facing::Right);
    d.y = 50;
    d.verticalDir = -1;
    w.place(d);
    check(w.assign(0, Skill::Digger), "inverted digger on 0x10 applies");
    const std::size_t before = w.terrain().solidCells();
    const int y0 = w.lemmings()[0].y;
    runFor(w, 40);
    check(w.terrain().solidCells() < before,
          "0x10 cells clear while inverted");
    // Y-down: dig toward gravity (v=-1) decreases y.
    check(w.lemmings()[0].y < y0,
          std::format("inverted digger stepped toward gravity (y {} -> {})",
                      y0, w.lemmings()[0].y));
  }
}

void testDigger() {
  std::cout << "digger goes down\n";
  World w(flatLevel());
  runFor(w, 60);
  const int before = static_cast<int>(w.terrain().solidCells());
  const int y0 = w.lemmings()[0].y;
  check(w.assign(0, Skill::Digger), "digger applies");
  runFor(w, 60);
  check(w.terrain().solidCells() < static_cast<std::size_t>(before), "terrain was removed");
  check(w.lemmings()[0].y > y0, "the digger descended");
}

void testBasher() {
  std::cout << "basher goes through\n";
  LevelSetup s = flatLevel();
  s.terrain.fillBox(120, 50, 10, 30, Material::Earth);
  World w(s);
  check(runUntilAgainstWall(w, 200), "the lemming reached the wall");
  const std::size_t before = w.terrain().solidCells();
  check(w.assign(0, Skill::Basher), "basher applies");
  runFor(w, 100);
  check(w.terrain().solidCells() < before, "the basher removed terrain");
}

// Round 5: what bash and dig remove, and when.
//
// Every number here is written out rather than derived from a Rules field --
// the round-4 lesson (see testStepUpHeight) is that a derived expectation
// passes at any value of the field it came from. These are the per-tick grid
// diffs measured off the original on MM_DIG_000 and MM_BASH_002, converted
// once: our y grows downward where the engine's grid is Y-up, so the
// engine's "feet row + 0..3" is our `y` down to `y - 3`, and its "row below
// the feet" is our `y + 1`.
void testDigFootprintAndCadence() {
  std::cout << "a digger takes three cells, one row, every six ticks\n";
  World w(flatLevel());
  runFor(w, 60);
  const int x0 = w.lemmings()[0].x;
  const int y0 = w.lemmings()[0].y;
  check(w.assign(0, Skill::Digger), "digger applies");

  // Which tick of the state each removal landed on, and how many cells it
  // took. MEASURED: 3 cells a cycle (columns trunc(X)-1, trunc(X), trunc(X)+1
  // in the row below the feet), four cycles of it on every digger watched.
  std::vector<int> at;
  std::vector<int> took;
  std::size_t prev = w.terrain().solidCells();
  for (int t = 0; t < 26; ++t) {
    w.tick();
    const std::size_t now = w.terrain().solidCells();
    if (now < prev) {
      at.push_back(t);
      took.push_back(static_cast<int>(prev - now));
    }
    prev = now;
  }

  check(took.size() >= 4, std::format("at least four cycles dug ({} seen)", took.size()));
  bool allThree = !took.empty();
  for (int n : took)
    allThree = allThree && n == 3;
  check(allThree, std::format("every cycle took 3 cells (first four: {}, {}, {}, {})",
                              took.size() > 0 ? took[0] : -1, took.size() > 1 ? took[1] : -1,
                              took.size() > 2 ? took[2] : -1, took.size() > 3 ? took[3] : -1));

  // The cycle is 1.200 s and the tick is 200 ms, so the dig is the one skill
  // of the three that lands on whole ticks: six of them, every time. The
  // bite is at 25% of the cycle, so the first falls in the tick covering
  // 300 ms, which is the state's second.
  if (at.size() >= 4) {
    check(at[0] == 1, std::format("first bite on the state's second tick (was {})", at[0]));
    check(at[1] - at[0] == 6 && at[2] - at[1] == 6 && at[3] - at[2] == 6,
          std::format("six ticks between bites (gaps {}, {}, {})", at[1] - at[0],
                      at[2] - at[1], at[3] - at[2]));
  }

  // Three columns wide, centred, and the fourth column each side untouched.
  for (int dy = 1; dy <= 4; ++dy) {
    check(!isSolid(w.terrain().at(x0 - 1, y0 + dy)), "column x-1 was dug");
    check(!isSolid(w.terrain().at(x0, y0 + dy)), "column x was dug");
    check(!isSolid(w.terrain().at(x0 + 1, y0 + dy)), "column x+1 was dug");
    check(isSolid(w.terrain().at(x0 - 2, y0 + dy)),
          std::format("column x-2 is still there at y+{}", dy));
    check(isSolid(w.terrain().at(x0 + 2, y0 + dy)),
          std::format("column x+2 is still there at y+{}", dy));
  }
  check(w.lemmings()[0].y >= y0 + 4, "the digger sank one row per cycle");
}

void testBashFootprintAndCadence() {
  std::cout << "a basher takes one column, four rows, every 7 or 8 ticks\n";
  LevelSetup s = flatLevel();
  s.terrain.fillBox(120, 50, 10, 30, Material::Earth);
  World w(s);
  check(runUntilAgainstWall(w, 200), "the lemming reached the wall");
  const int y0 = w.lemmings()[0].y;
  const int x0 = w.lemmings()[0].x;
  const int dir = static_cast<int>(w.lemmings()[0].facing);
  check(w.assign(0, Skill::Basher), "basher applies");

  std::vector<int> at;
  std::vector<int> took;
  std::size_t prev = w.terrain().solidCells();
  for (int t = 0; t < 46; ++t) {
    w.tick();
    const std::size_t now = w.terrain().solidCells();
    if (now < prev) {
      at.push_back(t);
      took.push_back(static_cast<int>(prev - now));
    }
    prev = now;
  }

  check(took.size() >= 6, std::format("at least six cycles bashed ({} seen)", took.size()));
  bool allFour = !took.empty();
  for (int n : took)
    allFour = allFour && n == 4;
  check(allFour, std::format("every cycle took 4 cells (first four: {}, {}, {}, {})",
                             took.size() > 0 ? took[0] : -1, took.size() > 1 ? took[1] : -1,
                             took.size() > 2 ? took[2] : -1, took.size() > 3 ? took[3] : -1));

  // 1.440 s of animation against a 200 ms tick is 7.2 ticks, which cannot be
  // one integer. Sampled, the gaps run 8, 7, 7, 7, 7 -- 36 ticks for five
  // cycles, the 7.2 the original measures, rather than a rounded 7 or 8.
  if (at.size() >= 6) {
    const int want[] = {8, 7, 7, 7, 7};
    bool gapsMatch = true;
    for (int k = 0; k < 5; ++k)
      gapsMatch = gapsMatch && at[k + 1] - at[k] == want[k];
    check(gapsMatch,
          std::format("bash gaps 8,7,7,7,7 (were {},{},{},{},{})", at[1] - at[0],
                      at[2] - at[1], at[3] - at[2], at[4] - at[3], at[5] - at[4]));
    check(at[5] - at[0] == 36, std::format("five cycles span 36 ticks (was {})", at[5] - at[0]));
  }

  // One column per cycle, four rows deep from the feet row upward, and the
  // row below the feet left alone -- a basher cuts a tunnel, it does not dig.
  check(!isSolid(w.terrain().at(x0 + dir, y0)), "the feet row of the first column went");
  check(!isSolid(w.terrain().at(x0 + dir, y0 - 3)), "three rows up went too");
  check(isSolid(w.terrain().at(x0 + dir, y0 - 4)), "the fifth row up is still there");
  check(isSolid(w.terrain().at(x0 + dir, y0 + 1)), "the row below the feet is still there");
}

// Round 6: when a builder lays a brick, and which cells it writes.
//
// MEASURED off the original on MM_BUILD -- three builders, three processes,
// both facings, twelve bricks each, 72 of 72 cells 0x00000000 -> 0x00000001 at
// animation percent 81, 0.9597 s apart. The numbers here are written out, not
// derived from Rules: that is the round-4 lesson, and the tick list below is
// the one thing a derived expectation would hide, because 960 ms against a
// 200 ms tick is 4.8 and no single integer interval can produce it.
//
// Converted once, as the dig and bash tests are: the engine's grid is Y-up and
// this Terrain grows downward, so the engine's "the builder's own row, then one
// row up" is our `y`, then `y - 1`.
void testBuilderFootprintAndCadence() {
  std::cout << "a builder lays two cells on its own row, twelve times, then shrugs\n";
  World w(flatLevel());
  runFor(w, 60);
  check(w.lemmings()[0].state == State::Walker, "walking before the assign");
  const int x0 = w.lemmings()[0].x;
  const int y0 = w.lemmings()[0].y;
  const int dir = static_cast<int>(w.lemmings()[0].facing);
  const std::size_t before = w.terrain().solidCells();
  check(w.assign(0, Skill::Builder), "builder applies");

  std::vector<int> at;
  std::vector<int> laid;
  std::size_t prev = before;
  int shrugAt = -1;
  int walkAfterShrug = -1;
  int shrugTicks = 0;
  int xAtShrug = 0;
  int yAtShrug = 0;
  for (int t = 0; t < 80; ++t) {
    w.tick();
    const std::size_t now = w.terrain().solidCells();
    if (now > prev) {
      at.push_back(t);
      laid.push_back(static_cast<int>(now - prev));
    }
    prev = now;
    const State st = w.lemmings()[0].state;
    if (st == State::Shrugger) {
      ++shrugTicks;
      if (shrugAt < 0) {
        shrugAt = t;
        xAtShrug = w.lemmings()[0].x;
        yAtShrug = w.lemmings()[0].y;
      }
    } else if (shrugAt >= 0 && walkAfterShrug < 0) {
      walkAfterShrug = t;
      check(st == State::Walker, "a spent builder shrugs and then walks");
    }
  }

  // Twelve bricks of two cells each. 0x0041011C's loop runs i = 1 and 2
  // (`cmp [ebp-0x10], 3` at 0x004101AB), and the counter at state+0x3c starts
  // at 12 (`mov [eax+0x3c], 0xc` at 0x0040F948).
  check(laid.size() == 12, std::format("twelve bricks were laid ({} seen)", laid.size()));
  bool allTwo = !laid.empty();
  for (int n : laid)
    allTwo = allTwo && n == 2;
  check(allTwo, std::format("every brick is 2 cells (first four: {}, {}, {}, {})",
                            laid.size() > 0 ? laid[0] : -1, laid.size() > 1 ? laid[1] : -1,
                            laid.size() > 2 ? laid[2] : -1, laid.size() > 3 ? laid[3] : -1));
  check(w.terrain().solidCells() == before + 24,
        std::format("24 cells of terrain in all (was {})",
                    w.terrain().solidCells() - before));

  // The cadence. 960 ms a cycle, the brick 75% in (81% less the 6.25% the
  // enter seek at 0x004B08F8 skips), against a 200 ms tick: the bricks land
  // on these ticks of the state, with gaps of 5, 5, 5, 4, 5, 5, 5, 5, 4, 5, 5
  // -- eleven gaps spanning 53 ticks, which averages 4.82. An interval of 5
  // would span 55 and an interval of 4 would span 44.
  const std::vector<int> wantAt{3, 8, 13, 18, 22, 27, 32, 37, 42, 46, 51, 56};
  std::string sawAt;
  for (int v : at)
    sawAt += (sawAt.empty() ? "" : ",") + std::to_string(v);
  check(at == wantAt,
        std::format("bricks on ticks 3,8,13,18,22,27,32,37,42,46,51,56 (were {})", sawAt));
  if (at.size() == 12)
    check(at[11] - at[0] == 53, std::format("eleven gaps span 53 ticks (was {})",
                                            at[11] - at[0]));

  // The footprint. Brick k is the two cells at offsets +1 and +2 ahead of the
  // builder, on the builder's OWN row, and the builder then steps one column
  // on and one row up -- so brick k sits at columns x0 + k*dir and
  // x0 + (k+1)*dir, on row y0 - (k - 1).
  for (int k = 1; k <= 12; ++k) {
    const int row = y0 - (k - 1);
    check(isSolid(w.terrain().at(x0 + k * dir, row)),
          std::format("brick {} filled the column {} ahead", k, k));
    check(isSolid(w.terrain().at(x0 + (k + 1) * dir, row)),
          std::format("brick {} filled the column {} ahead", k, k + 1));
  }
  // The builder's own cell is never filled: the brick starts one column
  // ahead, which is what makes the staircase a staircase.
  check(!isSolid(w.terrain().at(x0, y0)), "the cell the builder stood in is still air");
  check(!isSolid(w.terrain().at(x0 - dir, y0)), "and the cell behind it");
  // And the brick is two long, not three.
  check(!isSolid(w.terrain().at(x0 + 3 * dir, y0)),
        "the third cell ahead of the first brick is still air");

  // One column forward and one row up per brick, twelve times.
  check(xAtShrug == x0 + 12 * dir,
        std::format("the builder advanced twelve columns (x {} -> {})", x0, xAtShrug));
  check(yAtShrug == y0 - 12,
        std::format("and rose twelve rows (y {} -> {})", y0, yAtShrug));
  // The shrugger stands on its own last brick: brick twelve filled
  // (x0+12*dir, y0-11), which is the cell below it.
  check(isSolid(w.terrain().at(xAtShrug, yAtShrug + 1)),
        "the shrugger is standing on the brick it just laid");
  check(shrugAt == 56, std::format("the shrug starts on the twelfth brick's tick (was {})",
                                   shrugAt));
  check(shrugTicks == 5, std::format("the shrug lasts five ticks (was {})", shrugTicks));
  check(walkAfterShrug == 61,
        std::format("and it walks again on tick 61 (was {})", walkAfterShrug));
}

// Round 6: the builder's refusal, measured. At percent 90 the engine raises
// the row first and then probes the cell it is about to step into
// (`call 0x40de60` at 0x0040FE77, `cmp dword ptr [eax], 0` at 0x0040FE7C). A
// non-zero cell turns the lemming (0x0042F66B at 0x0040FEE3) and takes it out
// of the build state, and the brick counter is NOT decremented -- the
// decrement at 0x0040FF93 sits past that branch.
//
// Live: a left-facing builder with room for six bricks laid six whole ones and
// a seventh half-brick, then found (153, 31) = 0x00000001 on its eighth cycle,
// turned, and walked with its counter stuck at 5.
void testBuilderRefusesAtAWall() {
  std::cout << "a builder that steps into terrain turns, and keeps the brick\n";
  // Where an unobstructed builder would be standing, so the wall can go
  // exactly four columns ahead of it. The simulation is deterministic, so the
  // same 60 ticks put the lemming in the same cell on both worlds.
  int x0 = 0;
  int y0 = 0;
  int dir = 1;
  {
    World probe(flatLevel());
    runFor(probe, 60);
    x0 = probe.lemmings()[0].x;
    y0 = probe.lemmings()[0].y;
    dir = static_cast<int>(probe.lemmings()[0].facing);
  }
  LevelSetup s = flatLevel();
  s.terrain.fillBox(x0 + 4 * dir, y0 - 10, 1, 11, Material::Earth);
  World w(s);
  runFor(w, 60);
  check(w.lemmings()[0].x == x0 && w.lemmings()[0].y == y0,
        "the wall did not disturb the walk up to the assign");
  check(w.assign(0, Skill::Builder), "builder applies");

  std::vector<int> laid;
  std::size_t prev = w.terrain().solidCells();
  int bricksWhenItStopped = -1;
  State stateWhenItStopped = State::Builder;
  for (int t = 0; t < 40; ++t) {
    w.tick();
    const std::size_t now = w.terrain().solidCells();
    if (now > prev)
      laid.push_back(static_cast<int>(now - prev));
    prev = now;
    if (bricksWhenItStopped < 0 && w.lemmings()[0].state != State::Builder) {
      bricksWhenItStopped = w.lemmings()[0].bricksLeft;
      stateWhenItStopped = w.lemmings()[0].state;
    }
  }

  // Bricks one and two land clear of the wall; brick three has its far cell
  // already standing and brick four its near cell, so each of those writes
  // one cell -- the stamp is an `or`, so a brick laid into terrain changes
  // nothing. Then the step-into probe hits the wall.
  const std::vector<int> wantLaid{2, 2, 1, 1};
  std::string sawLaid;
  for (int v : laid)
    sawLaid += (sawLaid.empty() ? "" : ",") + std::to_string(v);
  check(laid == wantLaid,
        std::format("bricks wrote 2, 2, 1, 1 cells (were {})", sawLaid));
  check(stateWhenItStopped == State::Walker,
        "the refused builder walks rather than shrugging");
  check(bricksWhenItStopped == 9,
        std::format("three bricks were spent and the fourth was not (left {})",
                    bricksWhenItStopped));
  check(w.lemmings()[0].facing == (dir > 0 ? Facing::Left : Facing::Right),
        "and it turned round");
}

void testSkillRules() {
  std::cout << "the bash / dig / mine constants are the measured ones\n";
  const World::Rules r;
  check(r.basherReach == 1, "basherReach is 1 column (was classic 2)");
  check(r.basherHeight == 4, "basherHeight is 4 rows");
  check(r.diggerWidth == 3, "diggerWidth is 3 cells (was 5)");
  check(r.diggerDepth == 1, "diggerDepth is 1 row");
  check(r.minerReach == 2, "minerReach is 2 columns ahead");
  check(r.basherCycleMs == 1440, "bash cycle is 24 animation frames of 60 ms");
  check(r.diggerCycleMs == 1200, "dig cycle is 20 animation frames of 60 ms");
  check(r.minerCycleMs == 1500, "mine cycle is 25 animation frames of 60 ms");
  check(r.basherBitePercent == 12 && r.diggerBitePercent == 25 && r.minerBitePercent == 12,
        "the first bite of each cycle is at the record frame read live");

  // Round 6, the builder. Each of these is the read or the measurement named
  // in World.h, written out here so a change to the field fails this too.
  check(r.builderBricks == 12, "builderBricks is 12 (state+0x3c, 0x0040F948)");
  check(r.brickLength == 2, "brickLength is 2 cells (was classic 3)");
  check(r.builderCycleMs == 960, "build cycle is 16 animation frames of 60 ms");
  check(r.builderBrickPercent == 81, "the brick record's frame is 81");
  check(r.builderEnterPercent == 6, "the 16.0 enter seek is a sixteenth of a cycle");
  check(r.shrugCycleMs == 960, "the shrug is 8 frames of 60 ms over divisor 0.5");

  // Round 7, the bomber. The fuse is a time, not a tick count: the integer 5
  // at 0x0045B1B3 becomes a double and the enter multiplies by the 1000 at
  // 0x004657B2.
  check(r.bomberFuseMs == 5000, "the fuse is 5000 ms (5 x 1000, not 25 ticks)");
  check(r.exploderCycleMs == 1020,
        "the explode strip is 17 animation frames of 60 ms");
}

void testBuilder() {
  std::cout << "builder adds terrain\n";
  World w(flatLevel());
  runFor(w, 60);
  const std::size_t before = w.terrain().solidCells();
  check(w.assign(0, Skill::Builder), "builder applies");
  const Watch watch = runWatching(w, 200);
  check(w.terrain().solidCells() > before, "the builder added terrain");
  check(watch.minY < 79, std::format("and rose as it built (highest y {})", watch.minY));
}

// MEASURED LIVE, round 7. Six bombs across MM_BOMB and MM_Block_Bomb_A,
// clicked from inside a 650 Hz whole-grid diff loop
// (tools/harness/measure_bomb.py --bomb):
//
//   click -> flapper   4.9967 4.9985 4.9981 4.9976 4.9998 4.9980 s
//   flapper -> blast   1.0211 1.0193 1.0226 1.0206 1.0191 1.0201 s
//
// so the fuse is five seconds, the `Ohno` cue is at the END of it, and the
// crater is 1.02 s later again. Every number below is written out rather than
// read back off `Rules`, which is round 4's lesson.
void testBomberFuseAndCrater() {
  std::cout << "the bomber: a 25-tick fuse, a flap, then the 7x7 stencil\n";
  World w(flatLevel(80));
  runFor(w, 20);
  check(!w.lemmings().empty() && w.lemmings()[0].state == State::Walker,
        "a walker is out and on the floor");
  const int feet = w.lemmings()[0].y;
  check(feet == 79, std::format("its feet are at y 79 (was {})", feet));
  w.takeCues();

  check(w.assign(0, Skill::Bomber), "the bomber applies");
  {
    const auto cues = w.takeCues();
    check(cues.size() == 1 && cues[0] == SoundCue::Assign,
          "assigning it chinks, and says nothing else yet");
  }
  const std::size_t before = w.terrain().solidCells();

  int ohNoTick = -1, explodeTick = -1, exploderTick = -1, firstGridTick = -1;
  int bombX = -1, walkedTo = -1;
  for (int t = 1; t <= 45; ++t) {
    w.tick();
    for (SoundCue c : w.takeCues()) {
      if (c == SoundCue::OhNo && ohNoTick < 0)
        ohNoTick = t;
      if (c == SoundCue::Explode && explodeTick < 0)
        explodeTick = t;
    }
    if (!w.lemmings().empty()) {
      const Lemming& l = w.lemmings()[0];
      if (l.state == State::Walker)
        walkedTo = l.x;
      if (l.state == State::Exploder && exploderTick < 0) {
        exploderTick = t;
        bombX = l.x;
      }
    }
    if (firstGridTick < 0 && w.terrain().solidCells() != before)
      firstGridTick = t;
  }

  // 5000 ms at the measured 200 ms tick.
  check(ohNoTick == 25, std::format("oh-no on tick 25 (was {})", ohNoTick));
  check(exploderTick == 25,
        std::format("and the explode animation starts on the same tick (was {})",
                    exploderTick));
  // The old stepper cued oh-no one second from the end, which on a 25-tick
  // fuse is tick 20. Nothing happens on tick 20 now.
  check(ohNoTick != 20, "not one second early, which is where it used to be");
  // 1020 ms of `explode` strip: the tick that covers it is the sixth.
  check(explodeTick == 31, std::format("the blast on tick 31 (was {})", explodeTick));
  check(firstGridTick == 31,
        std::format("and the terrain is untouched until then (first change on tick {})",
                    firstGridTick));
  check(w.dead() == 1, "the bomber died");
  // The countdown sits on the lemming's own +0x374 list, so it keeps walking.
  check(walkedTo == bombX,
        std::format("it walked through the whole fuse and stopped where it blew"
                    " ({} vs {})", walkedTo, bombX));

  // The crater. The window is columns bombX-4 .. bombX+2 and rows 76..82 --
  // the blast cell is y 80, one below the feet row, because 0x00416B5B drops
  // the Y by one vertical unit when the blast state is installed. Rows 76..79
  // are open air above this floor, so the cells that go are the stencil's
  // bottom three rows: 7 + 5 + 3.
  const std::size_t removed = before - w.terrain().solidCells();
  check(removed == 15, std::format("15 cells of floor went (got {})", removed));
  const Terrain& t = w.terrain();
  check(t.at(t.wrapX(bombX - 4), 80) == Material::Empty &&
        t.at(t.wrapX(bombX + 2), 80) == Material::Empty,
        "row 80 is cleared seven wide, bombX-4 to bombX+2");
  check(t.at(t.wrapX(bombX - 5), 80) == Material::Earth &&
        t.at(t.wrapX(bombX + 3), 80) == Material::Earth,
        "and the columns either side of the window are not");
  check(t.at(t.wrapX(bombX - 3), 81) == Material::Empty &&
        t.at(t.wrapX(bombX + 1), 81) == Material::Empty &&
        t.at(t.wrapX(bombX - 4), 81) == Material::Earth,
        "row 81 is five wide, inset by one -- the stencil's . # # # # # .");
  check(t.at(t.wrapX(bombX - 2), 82) == Material::Empty &&
        t.at(t.wrapX(bombX), 82) == Material::Empty &&
        t.at(t.wrapX(bombX - 3), 82) == Material::Earth,
        "row 82 is three wide -- . . # # # . .");
  check(t.at(t.wrapX(bombX), 83) == Material::Earth,
        "and row 83 is below the window");
}

// MEASURED LIVE, round 7: a bombed blocker's four field cells cleared 3.7 ms
// and 4.2 ms after the state became the flapper -- that is, at the END of the
// fuse -- and the crater came 1.02 s after that. So there are six ticks in
// which the blocker is neither blocking nor dead.
void testBombedBlockerLosesItsFieldAtTheFuse() {
  std::cout << "a bombed blocker drops its field at the fuse, not at the blast\n";
  World w(flatLevel(80));
  runFor(w, 20);
  check(w.assign(0, Skill::Blocker), "the blocker applies");
  check(w.terrain().blockerFieldCells() == 4, "and stamps four cells");
  check(w.assign(0, Skill::Bomber), "and then takes a bomb");
  int fieldGoneTick = -1, deadTick = -1;
  for (int t = 1; t <= 45; ++t) {
    w.tick();
    if (fieldGoneTick < 0 && w.terrain().blockerFieldCells() == 0)
      fieldGoneTick = t;
    if (deadTick < 0 && w.lemmings()[0].state == State::Dead)
      deadTick = t;
  }
  check(fieldGoneTick == 25,
        std::format("the field goes on tick 25, with the fuse (was {})", fieldGoneTick));
  check(deadTick == 31,
        std::format("and it dies on tick 31, six ticks later (was {})", deadTick));
  check(deadTick - fieldGoneTick == 6,
        "which is the 1020 ms explode animation between them");
}

void testBomberAndSteel() {
  std::cout << "bomber, and steel resisting it\n";
  World w(flatLevel());
  runFor(w, 60);
  const std::size_t before = w.terrain().solidCells();
  check(w.assign(0, Skill::Bomber), "bomber applies");
  check(!w.assign(0, Skill::Bomber), "a second fuse is refused");
  runFor(w, 200);
  check(w.dead() == 1, "the bomber died");
  check(w.terrain().solidCells() < before, "and left a hole");

  // fillBox only fills empty cells, so the floor has to be built as steel from
  // the start rather than painted over the earth one.
  LevelSetup s;
  s.terrain = Terrain(200, 100);
  s.terrain.fillBox(0, 80, 200, 20, Material::Steel);
  s.entrances.push_back({100, 70});
  s.totalLemmings = 1;
  s.saveTarget = 1;
  s.releaseIntervalTicks = 1;
  World steel(s);
  runFor(steel, 60);
  const std::size_t steelBefore = steel.terrain().solidCells();
  steel.assign(0, Skill::Bomber);
  runFor(steel, 200);
  check(steel.terrain().solidCells() == steelBefore, "steel survived the explosion");
}

void testExit() {
  std::cout << "reaching the exit\n";
  LevelSetup s = flatLevel();
  // Floor fill starts at y=80; feet rest at 79. Measured box is 2 x 3
  // inclusive (round 11), covering x 128..130 and y 76..79.
  // Behavioural pin: the old half-open test (`y < y+height`) leaves y=79
  // outside [76, 79) and never saves — flip only the comparison in
  // World::atExit to watch this fail. The live original hits the exact max
  // doubles (222 and 34).
  s.exits.push_back({128, 76, 2, 3});
  World w(s);
  runFor(w, 300);
  check(w.saved() == 1, std::format("the lemming was saved (saved={})", w.saved()));
  check(w.won(), "the level is won");
}

// Round 12: nothing previously called exitTriggerFromPose. Widening the
// measured 2×3 back to the old 12-column guess left all 16 tests green.
// Pin the exact Terrain rectangle from a known .a3d pose, including a case
// where round(ang*width) ≠ floor (Exit_water e14_kf_1), and assert a
// lemming one cell outside each edge is not saved.
void testExitTriggerFromPose() {
  std::cout << "exitTriggerFromPose extent\n";
  constexpr int kW = 256;
  constexpr int kH = 96;

  // MM_CLIMB_002 Exit.a3d — live AABB X[220..222] Y[31..34] (round 11).
  // ang*256 ≈ 221.016; floor and round agree.
  {
    webo::WorldPose pose;
    pose.position = {49.062, 40.0, 42.35388};
    const webo::ExitTrigger box = webo::exitTriggerFromPose(pose, kW, kH);
    check(box.x == 220 && box.y == 61 && box.width == 2 && box.height == 3,
          std::format("MM_CLIMB_002 box (got {},{},{}x{})", box.x, box.y, box.width,
                      box.height));
  }

  // e14_kf_1 Exit_water.a3d — live AABB X[10..12] Y[16..19] (round 12).
  // ang*256 ≈ 10.989; floor would centre on 10 and miss the dump.
  {
    webo::WorldPose pose;
    pose.position = {-17.26225182172313, 25.0, 62.44198834698115};
    const webo::ExitTrigger box = webo::exitTriggerFromPose(pose, kW, kH);
    check(box.x == 10 && box.y == 76 && box.width == 2 && box.height == 3,
          std::format("e14_kf_1 water box (got {},{},{}x{})", box.x, box.y, box.width,
                      box.height));

    LevelSetup s = flatLevel();
    s.totalLemmings = 0;  // no hatch releases; only placed lemmings
    s.exits.push_back({box.x, box.y, box.width, box.height});
    World w(s);

    Lemming inside;
    inside.x = box.x + 1;
    inside.y = box.y + 1;
    inside.state = State::Walker;
    w.place(inside);
    w.tick();
    check(w.saved() == 1, "centre of measured box saves on the next tick");

    const std::array<std::pair<int, int>, 4> outside = {{
        {box.x - 1, box.y + 1},              // left of min X
        {box.x + box.width + 1, box.y + 1},  // right of max X
        {box.x + 1, box.y - 1},              // above min Y (terrain)
        {box.x + 1, box.y + box.height + 1}, // below max Y
    }};
    const char* edgeNames[] = {"left", "right", "above", "below"};
    for (std::size_t i = 0; i < outside.size(); ++i) {
      LevelSetup edge = flatLevel();
      edge.totalLemmings = 0;
      edge.exits.push_back({box.x, box.y, box.width, box.height});
      World ew(edge);
      Lemming l;
      l.x = outside[i].first;
      l.y = outside[i].second;
      l.state = State::Walker;
      ew.place(l);
      ew.tick();
      check(ew.saved() == 0,
            std::format("one cell {} the box does not save", edgeNames[i]));
    }
  }
}

void testDeterminism() {
  std::cout << "determinism\n";
  const auto run = [] {
    LevelSetup s = flatLevel();
    s.totalLemmings = 5;
    s.releaseIntervalTicks = 7;
    s.terrain.fillBox(140, 50, 6, 30, Material::Earth);
    World w(s);
    std::vector<int> trace;
    for (int t = 0; t < 400; ++t) {
      if (t == 50)
        w.assign(0, Skill::Digger);
      if (t == 80 && w.lemmings().size() > 1)
        w.assign(1, Skill::Builder);
      if (t == 120 && w.lemmings().size() > 2)
        w.assign(2, Skill::Blocker);
      w.tick();
      for (const auto& l : w.lemmings())
        trace.push_back(l.x * 1000 + l.y * 10 + static_cast<int>(l.state));
    }
    return trace;
  };
  const auto a = run();
  const auto b = run();
  check(!a.empty(), "the run produced a trace");
  check(a == b, "two identical runs produce identical traces");
}

void testPickAndNuke() {
  std::cout << "picking and the nuke\n";
  LevelSetup s = flatLevel();
  s.totalLemmings = 3;
  s.releaseIntervalTicks = 5;
  World w(s);
  runFor(w, 80);
  check(w.activeCount() == 3, "three lemmings are active");
  const auto& l = w.lemmings()[0];
  const auto hit = w.pick(l.x, l.y - 4, 6);
  check(hit.has_value(), "a click near a lemming picks it");
  check(!w.pick(l.x + 500, l.y, 2).has_value(), "a click far away picks nothing");

  w.nuke();
  runFor(w, 300);
  check(w.activeCount() == 0, "the nuke cleared the level");
  check(w.finished(), "and the level is finished");
}

// The level clock. LEVELMAP.DAT gives every level a time limit, so the field
// has to do something; this pins down exactly what, which is narrower than it
// sounds. Running out closes the entrance. It does not kill anyone, and it
// does not end the level -- that is the game shell's call.
void testTimeLimit() {
  std::cout << "the level clock\n";

  LevelSetup s = flatLevel();
  s.totalLemmings = 20;
  s.releaseIntervalTicks = 5;
  s.timeLimitTicks = 40;
  World w(s);

  check(!w.timedOut(), "a fresh level has not timed out");
  check(w.timeRemainingTicks() == 40, "the clock starts at the limit");

  runFor(w, 20);
  check(!w.timedOut(), "still running at half time");
  check(w.timeRemainingTicks() == 20, "the clock counts down");
  check(w.activeCount() > 0, "lemmings are being released");

  runFor(w, 25);  // past the limit
  check(w.timedOut(), "the clock runs out");
  check(w.timeRemainingTicks() == 0, "no time remains");

  // The entrance closes, but the clock itself kills nobody.
  const int releasedAtTimeout = w.activeCount() + w.saved() + w.dead();
  runFor(w, 100);
  const int releasedLater = w.activeCount() + w.saved() + w.dead();
  check(releasedLater == releasedAtTimeout, "no more lemmings are released after the timeout");
  check(releasedAtTimeout < s.totalLemmings,
        "the timeout stopped the entrance before the level emptied");

  // A limit of 0 is untimed, which is what an unsupplied limit gives.
  LevelSetup untimed = flatLevel();
  untimed.totalLemmings = 4;
  untimed.releaseIntervalTicks = 5;
  World u(untimed);
  runFor(u, 200);
  check(!u.timedOut(), "a level with no limit never times out");
  check(u.timeRemainingTicks() == 0, "and reports no remaining time");
  check(u.activeCount() + u.saved() + u.dead() == 4, "all of its lemmings were released");
}

// Sound cues are how the shell hears the simulation without the simulation
// knowing about SDL. They have to fire at the right moments and de-dupe the
// ones a nuke would otherwise stack.
// The climb tutorial's mask, not a hand-built wall. A climber released at the
// entrance has to leave the lower ledge: that is the whole of this level.
void testRetailClimb(const std::filesystem::path& dir) {
  std::cout << "climber on MM_CLIMB_002\n";
  auto lib = webo::AssetLibrary::open(dir / "lemmings.box", dir / "patch.box");
  if (!lib) {
    check(false, lib.error());
    return;
  }
  const webo::OccupancyMask* mask = nullptr;
  webo::OccupancyMask owned;
  int entranceX = 0;
  int entranceY = 0;
  for (const auto& name : webo::levelFileNames(*lib)) {
    if (name.find("MM_CLIMB_002") == std::string::npos)
      continue;
    auto blob = lib->read(name);
    if (!blob) {
      check(false, blob.error());
      return;
    }
    auto objs = webo::findLevelObjects(*blob);
    auto mesh = webo::findCylinderMesh(*blob);
    auto index = webo::indexLevel(*blob);
    if (!objs || objs->entrances.empty() || !mesh || !index) {
      check(false, "MM_CLIMB_002 did not yield a surface");
      return;
    }
    auto faces = webo::buildSurfaceMesh(*mesh, index->grid0, index->grid1);
    if (!faces) {
      check(false, faces.error());
      return;
    }
    owned = webo::rasteriseSurface(*faces, *mesh, 2);
    mask = &owned;
    const auto px = webo::worldToMaskPixel(objs->entrances[0].position, owned.width,
                                            owned.height, mesh->height(), 2);
    entranceX = px.first;
    entranceY = px.second;
    break;
  }
  if (!mask) {
    check(false, "MM_CLIMB_002 was not in the archive");
    return;
  }

  LevelSetup s;
  s.terrain = Terrain(mask->width, mask->height);
  for (int y = 0; y < mask->height; ++y)
    for (int x = 0; x < mask->width; ++x)
      if (mask->solid[static_cast<std::size_t>(y) * mask->width + x])
        s.terrain.set(x, y, Material::Earth);
  s.entrances.push_back({entranceX, entranceY});
  s.totalLemmings = 1;
  s.releaseIntervalTicks = 1;
  s.skills[static_cast<std::size_t>(Skill::Climber)] = 10;

  World w(s);
  w.tick();
  check(w.assign(0, Skill::Climber), "the climb level accepts a climber");

  int landed = -1;
  int highest = 1 << 30;
  bool climbed = false;
  for (int i = 0; i < 500 && !w.lemmings().empty(); ++i) {
    w.tick();
    const Lemming& l = w.lemmings()[0];
    if (landed < 0 && l.state == State::Walker)
      landed = l.y;
    if (l.state == State::Climber)
      climbed = true;
    highest = std::min(highest, l.y);
  }
  check(landed > 0, "the climber finds the ledge");
  check(climbed, "the climber starts up the wall");
  check(landed > 0 && highest <= landed - 20,
        std::format("and leaves the ledge (landed {}, highest {})", landed, highest));
}

// Where a released lemming starts. Not a mechanic test: these three triples
// were read out of the running original's hatch (words +0x9A / +0x9C) and out
// of the live lemming a fresh birth produced (+0x08 / +0x10), so they pin our
// parse and our spawn to measurements rather than to each other. The levels
// and the numbers are the ones in docs/scratch/phase3-round3.md.
void testRetailSpawn(const std::filesystem::path& dir) {
  std::cout << "spawn from the hatch words\n";
  auto lib = webo::AssetLibrary::open(dir / "lemmings.box", dir / "patch.box");
  if (!lib) {
    check(false, lib.error());
    return;
  }
  struct Case {
    const char* level;
    int count, x, y;  // the live hatch words
    int landWorldY;   // the live lemming's +0x10 when that birth landed
  };
  const Case cases[] = {
      {"MM_CLIMB_002", 10, 32, 42, 32},
      {"MM_FLOAT", 10, 65, 85, 81},
      {"MM_DIG_000", 20, 84, 79, 75},
  };
  for (const Case& c : cases) {
    std::string file;
    for (const auto& name : webo::levelFileNames(*lib))
      if (name.find(c.level) != std::string::npos) {
        file = name;
        break;
      }
    if (file.empty()) {
      check(false, std::format("{} is not in the archive", c.level));
      continue;
    }
    auto blob = lib->read(file);
    if (!blob) {
      check(false, blob.error());
      continue;
    }
    auto objs = webo::findLevelObjects(*blob);
    auto grid = webo::findCollisionGrid(*blob);
    if (!objs || !grid) {
      check(false, std::format("{}: no objects or no grid", c.level));
      continue;
    }
    check(objs->hatches.size() == 1,
          std::format("{}: one hatch record (got {})", c.level, objs->hatches.size()));
    if (objs->hatches.empty())
      continue;
    const auto& h = objs->hatches.front();
    check(h.count == c.count && h.x == c.x && h.y == c.y,
          std::format("{}: words ({}, {}, {}) are the live ({}, {}, {})", c.level, h.count,
                      h.x, h.y, c.count, c.x, c.y));

    const auto [cx, cy] = webo::hatchSpawnCell(h, grid->width, grid->height);
    LevelSetup s;
    s.terrain = Terrain::fromCollisionGrid(*grid);
    s.entrances.push_back({cx, cy, Entrance::Source::HatchWords});
    s.totalLemmings = 1;
    s.releaseIntervalTicks = 1;
    World w(s);
    w.tick();
    check(w.lemmings().size() == 1, std::format("{}: one lemming released", c.level));
    if (w.lemmings().empty())
      continue;
    const Lemming& l = w.lemmings().front();
    // One tick of fall has already happened, so compare the released cell,
    // not the current one: spawn's own write is (x, height - y).
    std::cout << std::format("  {}: words x {} y {} -> cell ({}, {}), first lemming x {}\n",
                             c.level, h.x, h.y, cx, cy, l.x);
    check(l.x == cx, std::format("{}: released column is the x word ({} vs {})", c.level,
                                 l.x, cx));
    check(cy == grid->height - 1 - h.y,
          std::format("{}: the cell is the Y-down flip of the y word ({} vs {})", c.level,
                      cy, grid->height - 1 - h.y));
    check(l.facing == Facing::Right,
          std::format("{}: released facing is our +1 (the original writes +0x40 = 1; "
                      "which column direction that is stays unverified)", c.level));

    // The drop from the word, on the stored grid, against the live landing.
    // This is the part that would catch a one-cell error in the flip: the
    // original's birth on each of these levels came to rest with +0x10 at
    // landWorldY, which is terrain row height - 1 - landWorldY.
    const int wantRow = grid->height - 1 - c.landWorldY;
    int restRow = -1;
    for (int i = 0; i < 200 && !w.lemmings().empty(); ++i) {
      w.tick();
      const Lemming& f = w.lemmings().front();
      if (f.state == State::Walker) {
        restRow = f.y;
        break;
      }
    }
    std::cout << std::format("    lands at row {} (live world Y {} is row {})\n", restRow,
                             c.landWorldY, wantRow);
    check(restRow == wantRow,
          std::format("{}: the drop from the word lands where the original did "
                      "({} vs {})", c.level, restRow, wantRow));
  }
}

void testSoundCues() {
  std::cout << "sound cues\n";

  // No exit on this floor: the bomber test needs the lemming alive for the
  // whole fuse, and an exit under the entrance would swallow it first.
  LevelSetup s = flatLevel(80);
  s.totalLemmings = 1;
  s.releaseIntervalTicks = 1;
  World w(s);
  runFor(w, 30);
  check(w.activeCount() == 1, "a lemming is out");
  check(w.takeCues().empty(), "walking emits no cues");

  check(w.assign(0, Skill::Bomber), "bomber assigns");
  {
    const auto cues = w.takeCues();
    check(cues.size() == 1 && cues[0] == SoundCue::Assign, "assigning a bomber chinks once");
  }

  // MEASURED LIVE, round 7: the fuse is 5000 ms, which at the measured 200 ms
  // tick is 25 ticks, and `Ohno` is requested at the END of it -- the same
  // instant the explode animation starts. The blast is a further 1020 ms, so
  // the tick that covers it is the sixth after that. These counts are written
  // out, not taken off `Rules`: the old version of this test computed them
  // from `bomberFuse` and `ticksPerSecond` and so could not have failed when
  // the cue was in the wrong place.
  runFor(w, 24);
  check(w.takeCues().empty(), "quiet for the first 24 ticks of the fuse");
  w.tick();  // tick 25: the 5000 ms event
  {
    const auto cues = w.takeCues();
    check(cues.size() == 1 && cues[0] == SoundCue::OhNo,
          "oh-no on tick 25, at the end of the fuse");
  }
  runFor(w, 5);
  check(w.takeCues().empty(), "then five quiet ticks of explode animation");
  w.tick();  // tick 31
  {
    const auto cues = w.takeCues();
    check(std::find(cues.begin(), cues.end(), SoundCue::Explode) != cues.end(),
          "and the blast on tick 31");
  }

  // A splat from a high entrance.
  LevelSetup tall = flatLevel(80);
  tall.entrances = {{50, 0}};
  tall.totalLemmings = 1;
  tall.releaseIntervalTicks = 1;
  World splat(tall);
  runFor(splat, 80);
  {
    const auto cues = splat.takeCues();
    check(std::find(cues.begin(), cues.end(), SoundCue::Splat) != cues.end(),
          "a long fall splatters audibly");
  }
}

}  // namespace

int main() {
  testFallAndWalk();
  testSplat();
  testFloater();
  testWallTurn();
  testStepUpHeight();
  testClimber();
  testClimberTopsOut();
  testWalkerStepsDownOneCell();
  testClimbWallColumnTopOutAndHoist();
  testFloatHalvingIsDelayed();
  testBlockerField();
  testBlockerTurnsWalkers();
  testBlockerFieldIsErased();
  testAntiGravFlipsGravity();
  testInvertedStepUp();
  testInvertedDigRefuseAndClear();
  testTrampolineBounce();
  testWaterHazardDrown();
  testAcidHazardSharesDrown();
  testTeleportPad();
  testSpeedPadFaster();
  testLeverToggle();
  testBalloonCueUnused();
  testFireCueUnused();
  testWitchCueNoLemmingsGadget();
  testForceFCueUnused();
  testAxeSheetNoLemmingsGadget();
  testBubbleCueNoGadget();
  testScrapeCueNoGadget();
  testRumbleCueNoGadget();
  testChangeopCueNoGadget();
  testSplatZoneExempt();
  testDieCueNoGadget();
  testDigger();
  testBasher();
  testDigFootprintAndCadence();
  testBashFootprintAndCadence();
  testBuilderFootprintAndCadence();
  testBuilderRefusesAtAWall();
  testSkillRules();
  testBuilder();
  testBomberFuseAndCrater();
  testBombedBlockerLosesItsFieldAtTheFuse();
  testBomberAndSteel();
  testExit();
  testExitTriggerFromPose();
  testDeterminism();
  testPickAndNuke();
  testTimeLimit();
  testSoundCues();

  if (const char* dir = std::getenv("WEBOLUTION_GAME_DIR")) {
    const std::filesystem::path root(dir);
    if (std::filesystem::exists(root / "lemmings.box")) {
      testRetailClimb(root);
      testRetailSpawn(root);
    } else
      std::cout << "climber on MM_CLIMB_002: skipped, no lemmings.box\n";
  } else {
    std::cout << "climber on MM_CLIMB_002: skipped, WEBOLUTION_GAME_DIR is not set\n";
  }

  std::cout << std::format("\n{} checks, {} failures\n", g_checks, g_failures);
  return g_failures == 0 ? 0 : 1;
}
