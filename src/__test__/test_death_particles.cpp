// Small logic checks only: event delivery and presentation lifetime. No routes,
// replays, frozen builds, original execution or original-fidelity assertions.
#include "app/DeathParticles.h"
#include "sim/World.h"
#include <cassert>
#include <iostream>

using namespace webo;
int main() {
  sim::LevelSetup setup;
  setup.terrain = sim::Terrain(64, 80);
  setup.totalLemmings = 0;
  setup.terrain.fillBox(0, 60, 64, 20, sim::Material::Earth);
  setup.terrain.set(21, 60, sim::Material::Steel);
  sim::World world(setup);
  int blasts = 0, splats = 0;
  world.setDeathObserver([&](const sim::Lemming& l, bool blast,
                            std::span<const std::array<int, 2>> cells) {
    if (blast) {
      ++blasts;
      assert(l.state == sim::State::Dead && world.dead() >= 1);
      assert(!cells.empty());
      for (const auto& cell : cells) {
        assert(world.terrain().at(cell[0], cell[1]) == sim::Material::Empty);
        assert(!(cell[0] == 21 && cell[1] == 60));
      }
    } else {
      ++splats;
      assert(l.state == sim::State::Splatter && cells.empty());
    }
  });
  sim::Lemming bomber;
  bomber.x = 20; bomber.y = 59; bomber.state = sim::State::Exploder;
  world.place(bomber);
  for (int i = 0; i < 20; ++i) world.tick();
  assert(blasts == 1 && world.dead() == 1);
  assert(world.terrain().at(21, 60) == sim::Material::Steel);
  sim::Lemming falling;
  falling.x = 40; falling.y = 10; falling.state = sim::State::Faller;
  world.place(falling);
  for (int i = 0; i < 40; ++i) world.tick();
  assert(splats == 1 && blasts == 1 && world.dead() == 2);

  app::DeathParticle p;
  p.origin = {255, 50, 0.75}; p.bornMs = 1000;
  p.vx = 10; p.vy = 8; p.gravity = -32;
  assert(p.position(1000).y == 50);
  assert(p.position(1500).y == 50);
  assert(p.position(2000).y == 42);
  p.vy = -8; p.gravity = 32;
  assert(p.position(2000).y == 58);
  std::cout << "death notifications and gravity-only presentation: passed\n";
}
