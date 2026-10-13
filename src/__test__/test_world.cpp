// Tests for the simulation.
//
// These check mechanics and determinism, not fidelity: the rule constants are
// provisional until the oracle pins them down (see World.h). What is asserted
// here is that a lemming falls, lands, walks, turns at walls, splats from a
// height, that each skill does the thing it is named for, and that the whole
// thing is reproducible tick for tick.

#include "sim/World.h"

#include <algorithm>
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
  // Just above the floor: a drop from the top of the world exceeds the splat
  // distance, which is correct behaviour but not what most of these tests are
  // about. testSplat sets up a long fall deliberately.
  s.entrances.push_back({100, floorY - 10});
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

void testBlocker() {
  std::cout << "blocker turns others\n";
  LevelSetup s = flatLevel();
  s.totalLemmings = 2;
  s.releaseIntervalTicks = 40;
  World w(s);
  runFor(w, 40);
  if (w.lemmings().empty())
    return;
  check(w.assign(0, Skill::Blocker), "blocker applies");
  const int blockerX = w.lemmings()[0].x;
  runFor(w, 300);
  check(w.lemmings().size() == 2, "the second lemming spawned");
  if (w.lemmings().size() < 2)
    return;
  check(w.lemmings()[0].state == State::Blocker, "the blocker is still blocking");
  check(w.lemmings()[1].facing == Facing::Left ||
            w.lemmings()[1].x < blockerX + 4,
        "the second lemming was turned or stopped");
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
  s.exits.push_back({130, 70, 10, 12});
  World w(s);
  runFor(w, 300);
  check(w.saved() == 1, std::format("the lemming was saved (saved={})", w.saved()));
  check(w.won(), "the level is won");
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

}  // namespace

int main() {
  testFallAndWalk();
  testSplat();
  testFloater();
  testWallTurn();
  testClimber();
  testBlocker();
  testDigger();
  testBasher();
  testBuilder();
  testBomberAndSteel();
  testExit();
  testDeterminism();
  testPickAndNuke();
  testTimeLimit();

  std::cout << std::format("\n{} checks, {} failures\n", g_checks, g_failures);
  return g_failures == 0 ? 0 : 1;
}
