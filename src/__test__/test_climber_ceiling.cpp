#include "sim/World.h"

#include <iostream>
#include <utility>

using namespace webo::sim;

int main() {
  LevelSetup setup;
  setup.totalLemmings = 0;
  setup.terrain = Terrain(16, 12);
  setup.terrain.fillBox(5, 0, 1, 6, Material::Steel);
  setup.terrain.set(6, 0, Material::Steel); // overhang on the right side only
  World world(std::move(setup));

  Lemming actor;
  actor.state = State::Climber;
  actor.y = 3;
  actor.x = 5;
  actor.facing = Facing::Right;
  world.place(actor);
  actor.x = 6;
  actor.facing = Facing::Left;
  world.place(actor);

  world.tick(); // head row -1 samples row 0 on each side of the wall
  const auto& openSide = world.lemmings()[0];
  const auto& overhangSide = world.lemmings()[1];
  if (openSide.state != State::Climber || openSide.y != 2 ||
      overhangSide.state != State::Faller || overhangSide.facing != Facing::Right) {
    std::cerr << "At native Y == height, only the occupied head cell should turn a climber.\n";
    return 1;
  }

  world.tick(); // head row -2 forces the head hit even over the empty side
  if (openSide.state != State::Faller || openSide.facing != Facing::Left ||
      openSide.x != 4 || openSide.y != 1 || openSide.fallDistance != 0) {
    std::cerr << "Above the height limit, the climber should turn and fall on its approach side.\n";
    return 1;
  }
}
