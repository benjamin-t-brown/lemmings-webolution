// Assignment contract only: no scripted route or claim of original fidelity.
#include "app/RuntimePick.h"
#include "sim/World.h"

#include <cstdio>
#include <cstdlib>

namespace {
using namespace webo;
using namespace webo::sim;

void check(bool ok, const char* message) {
  if (!ok) { std::fprintf(stderr, "FAIL: %s\n", message); std::exit(1); }
}

World supportedActor(State state, Facing facing, int frame) {
  LevelSetup setup;
  setup.terrain = Terrain(64, 48);
  setup.terrain.set(23, 25, Material::Built);
  setup.skills.fill(2);
  World world(std::move(setup));
  Lemming actor;
  actor.x = 23; actor.y = 24; actor.facing = facing;
  actor.state = state; actor.frame = frame;
  world.place(actor);
  return world;
}
}

int main() {
  constexpr Skill skills[]{Skill::Climber, Skill::Floater, Skill::Bomber, Skill::Blocker,
                           Skill::Builder, Skill::Basher, Skill::Miner, Skill::Digger};
  constexpr State jobs[]{State::Shrugger, State::Shrugger, State::Shrugger, State::Blocker,
                         State::Builder, State::Basher, State::Miner, State::Digger};
  for (Facing facing : {Facing::Left, Facing::Right}) {
    // Early and late in the shrug must both be interruptible.
    for (int frame : {0, 4}) {
      for (std::size_t i = 0; i < std::size(skills); ++i) {
        auto world = supportedActor(State::Shrugger, facing, frame);
        const auto skill = skills[i];
        const auto& actor = world.lemmings()[0];
        check(app::passesKnownPickExclusions(actor, skill), "shrug passes hover eligibility");
        check(world.assign(0, skill), "shrug accepts each walker skill");
        const auto& assigned = world.lemmings()[0];
        check(assigned.x == 23 && assigned.y == 24 && assigned.facing == facing,
              "new job starts at the shrug position and facing");
        check(assigned.state == jobs[i], "work skills replace shrug; capabilities and fuse coexist");
        check(world.setup().skills[i] == 1, "one use of the assigned skill is spent");
        if (i >= 3) check(assigned.frame == 0, "new work starts at its entry frame");
        if (skill == Skill::Builder)
          check(assigned.bricksLeft == world.rules().builderBricks, "chained builder has fresh bricks");
        if (skill == Skill::Basher) check(assigned.bashCycles == 0, "basher starts a fresh job");
        if (skill == Skill::Climber) check(assigned.canClimb, "climber capability is applied");
        if (skill == Skill::Floater) check(assigned.canFloat, "floater capability is applied");
        if (skill == Skill::Bomber)
          check(assigned.fuseMs == world.rules().bomberFuseMs, "bomber fuse starts during shrug");
        if (i < 3) {
          check(assigned.frame == frame, "parallel skill leaves shrug progress intact");
          check(!world.assign(0, skill), "duplicate parallel skill is still refused");
          check(world.setup().skills[i] == 1, "duplicate refusal spends no stock");
        }
      }
    }
  }

  // Adding shrug must not broaden the existing work-skill state boundary.
  for (State state : {State::Faller, State::Climber, State::Hoister, State::Floater,
                      State::Exploder, State::Splatter, State::Exited, State::Dead,
                      State::Bounce, State::Drown, State::Teleporting}) {
    for (Skill skill : {Skill::Blocker, Skill::Builder, Skill::Basher, Skill::Miner, Skill::Digger}) {
      auto world = supportedActor(state, Facing::Right, 2);
      check(!world.assign(0, skill), "other non-assignable states still refuse work skills");
      check(world.lemmings()[0].state == state && world.lemmings()[0].frame == 2,
            "refusal preserves the current state");
      check(world.setup().skills[static_cast<std::size_t>(skill)] == 2,
            "refusal spends no stock");
    }
  }
  std::puts("Shrug assignment: all checks passed");
}
