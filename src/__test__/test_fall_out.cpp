// Check the fall-out boundary and effect placement.
#include "sim/World.h"

#include <algorithm>
#include <cassert>
#include <iostream>

using namespace webo::sim;

static void boundary(bool floating, bool support, bool inverted, bool elapsed) {
  LevelSetup setup;
  setup.terrain = Terrain(64, 96);
  setup.totalLemmings = 0;
  if (support)
    setup.terrain.fillBox(0, inverted ? 0 : 95, 64, 1, Material::Earth);
  World world(std::move(setup));
  Lemming actor;
  actor.x = 32;
  actor.y = inverted ? 12 : 83; // short falls, independently of splat distance
  actor.verticalDir = inverted ? -1 : 1;
  actor.canFloat = floating;
  actor.state = floating ? State::Floater : State::Faller;
  actor.fallDistance = floating ? world.rules().floatAfter : 0;
  world.place(actor);

  int notifications = 0, splatCues = 0;
  world.setDeathObserver([&](const Lemming& l, bool explosion,
                            std::span<const std::array<int, 2>> cells) {
    ++notifications;
    assert(!explosion && cells.empty());
    assert(l.state == State::Splatter && !l.active());
    assert(world.dead() == 1);
    assert(l.y == (inverted ? 0 : 95)); // effect stays on the cylinder edge
  });
  for (int i = 0; i < 200; ++i) {
    if (elapsed) world.advanceElapsedMs(40);
    else world.tick();
    const auto cues = world.takeCues();
    splatCues += std::count(cues.begin(), cues.end(), SoundCue::Splat);
  }
  const auto& result = world.lemmings().front();
  if (support) {
    assert(world.dead() == 0 && notifications == 0 && splatCues == 0);
    assert(result.state == State::Walker);
    assert(result.y == (inverted ? 1 : 94));
  } else {
    assert(world.dead() == 1 && notifications == 1 && splatCues == 1);
    assert(result.state == State::Dead && world.activeCount() == 0);
  }
}

int main() {
  for (bool floating : {false, true})
    for (bool support : {false, true})
      for (bool inverted : {false, true})
        for (bool elapsed : {false, true})
          boundary(floating, support, inverted, elapsed);
  std::cout << "open-edge losses, floater losses, real edge support: passed\n";
}
