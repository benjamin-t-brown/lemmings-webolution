// Print one placed lemming's cell for N ticks of webo::sim::World.
//
//   compare_tick <game-dir> <level-id> <walker|climber> <x> <y> <left|right> <ticks>
//
// Tick 0 is the cell passed in, before any step. Each later line is the cell
// after one World::tick. Terrain is the level's stored collision grid via
// Terrain::fromCollisionGrid. Nothing is released from an entrance, and
// Rules are the defaults in World.h — this tool does not set them.
//
// tools/harness/compare_tick.py is the live side. It diffs these lines
// against one billboard and stops at the first disagreement.

#include "assets/Box.h"
#include "assets/CollisionGrid.h"
#include "sim/World.h"

#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <string>
#include <string_view>

namespace {

int usage() {
  std::cerr << "usage: compare_tick <game-dir> <level-id> "
               "<walker|climber> <x> <y> <left|right> <ticks>\n";
  return 2;
}

}  // namespace

int main(int argc, char** argv) {
  if (argc != 8)
    return usage();

  const std::filesystem::path gameDir = argv[1];
  const std::string level = argv[2];
  const std::string_view which = argv[3];
  const int x = std::atoi(argv[4]);
  const int y = std::atoi(argv[5]);
  const std::string_view facingName = argv[6];
  const int ticks = std::atoi(argv[7]);
  if (ticks < 0)
    return usage();

  webo::sim::State state;
  if (which == "walker")
    state = webo::sim::State::Walker;
  else if (which == "climber")
    state = webo::sim::State::Climber;
  else
    return usage();

  webo::sim::Facing facing;
  if (facingName == "right")
    facing = webo::sim::Facing::Right;
  else if (facingName == "left")
    facing = webo::sim::Facing::Left;
  else
    return usage();

  auto lib = webo::AssetLibrary::open(gameDir / "lemmings.box", gameDir / "patch.box");
  if (!lib) {
    std::cerr << lib.error() << "\n";
    return 1;
  }

  const std::string archiveName = "COMPILED\\" + level + ".LVL";
  auto grid = webo::loadCollisionGrid(*lib, archiveName);
  if (!grid) {
    std::cerr << grid.error() << "\n";
    return 1;
  }

  webo::sim::LevelSetup setup;
  setup.terrain = webo::sim::Terrain::fromCollisionGrid(*grid);
  // World::tick still calls spawn. An empty quota makes that a no-op, so the
  // placed lemming is the only one stepped. spawn itself is not edited.
  setup.totalLemmings = 0;

  webo::sim::World world(std::move(setup));

  webo::sim::Lemming lem;
  lem.x = x;
  lem.y = y;
  lem.state = state;
  lem.facing = facing;
  // The climb stepper keys off State::Climber. The skill flag is what a
  // walker checks on a wall; set it so a climber placed here is one.
  lem.canClimb = state == webo::sim::State::Climber;
  world.place(lem);

  const auto& at = world.lemmings().at(0);
  const auto& terrain = world.terrain();
  std::cerr << "grid " << terrain.width() << "x" << terrain.height()
            << " cell " << at.x << "," << at.y
            << " solid_here " << (terrain.solid(at.x, at.y) ? 1 : 0)
            << " solid_below " << (terrain.solid(at.x, at.y + 1) ? 1 : 0)
            << " solid_above " << (terrain.solid(at.x, at.y - 1) ? 1 : 0)
            << "\n";

  auto line = [&](int tick) {
    const auto& l = world.lemmings().at(0);
    std::cout << tick << ' ' << l.x << ' ' << l.y << ' ' << webo::sim::stateName(l.state)
              << '\n';
  };

  line(0);
  for (int i = 1; i <= ticks; ++i) {
    world.tick();
    line(i);
  }
  return 0;
}
