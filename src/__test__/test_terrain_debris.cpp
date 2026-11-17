// Seconds-long legacy logic check: successful cuts notify after clearing,
// refusals/air do not, and presentation particles expire. No gameplay route.
#include "app/TubeScene.h"
#include "sim/World.h"

#include <cassert>
#include <iostream>
#include <set>

using namespace webo;

namespace {
void cut(sim::State state, bool steel, bool air, int facing = 1, int gravity = 1) {
  sim::LevelSetup setup;
  setup.terrain = sim::Terrain(64, 96);
  setup.totalLemmings = 0;
  const auto material = steel ? sim::Material::Steel : sim::Material::Earth;
  if (!air) setup.terrain.fillBox(0, 0, 64, 96, material);
  sim::World world(setup);
  sim::Lemming actor;
  actor.x = facing > 0 ? 63 : 0; // cut across the cylinder seam
  actor.y = 59;
  actor.facing = facing > 0 ? sim::Facing::Right : sim::Facing::Left;
  actor.verticalDir = gravity;
  actor.state = state;
  world.place(actor);
  app::TubeScene scene;
  std::set<std::array<int, 2>> removed;
  int events = 0;
  std::uint64_t cutClockMs = 0;
  world.setTerrainCutObserver([&](const sim::Lemming& l,
                                  std::span<const std::array<int, 2>> cells) {
    ++events;
    cutClockMs = world.levelClockMs();
    assert(l.state == state && l.verticalDir == gravity && !cells.empty());
    for (const auto& cell : cells) {
      assert(cell[0] >= 0 && cell[0] < world.terrain().width());
      assert(cell[1] >= 0 && cell[1] < world.terrain().height());
      assert(setup.terrain.at(cell[0], cell[1]) == sim::Material::Earth);
      assert(world.terrain().at(cell[0], cell[1]) == sim::Material::Empty);
      assert(removed.insert(cell).second); // never report a cell twice
      if (state == sim::State::Basher) assert(cell[0] == (facing > 0 ? 0 : 63));
      if (state == sim::State::Digger) assert(cell[1] == actor.y + gravity);
    }
    scene.emitTerrainDebris(l, cells, world.levelClockMs(), world.terrain().height());
  });
  // One default-rate bite only, including Digger's later bite phase.
  for (int i = 0; i < 3; ++i) world.tick();
  if (steel || air) {
    assert(events == 0 && scene.deathParticleCount() == 0);
    assert(world.terrain().version() == setup.terrain.version());
  } else {
    assert(events > 0 && !removed.empty());
    assert(scene.deathParticleCount() == removed.size() * 4);
    // Retained particles bridge consecutive default skill bites, then vanish.
    scene.advanceDeathParticles(cutClockMs + 1500);
    assert(scene.deathParticleCount() > 0);
    scene.advanceDeathParticles(cutClockMs + 2000);
    assert(scene.deathParticleCount() == 0);
  }
}
} // namespace

int main() {
  for (const auto state : {sim::State::Basher, sim::State::Miner, sim::State::Digger}) {
    cut(state, false, false);
    cut(state, true, false);
    cut(state, false, true);
    cut(state, false, false, -1);
  }
  cut(sim::State::Digger, false, false, 1, -1);
  std::cout << "PASS: cut debris notifications, seam wrapping, refusals, inverted dig and expiry\n";
}
