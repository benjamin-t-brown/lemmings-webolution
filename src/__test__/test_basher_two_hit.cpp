// Check that two cuts sharing a column leave the same rows for collision, debris and the mesh.
#include "app/TubeScene.h"
#include "assets/Box.h"
#include "assets/LevelIndex.h"
#include "sim/World.h"

#include <cassert>
#include <filesystem>
#include <iostream>

using namespace webo;

namespace {
void punches(int facing) {
  sim::LevelSetup setup;
  setup.totalLemmings = 0;
  setup.terrain = sim::Terrain(64, 80);
  setup.terrain.fillBox(0, 60, 64, 20, sim::Material::Earth);
  const int x = facing > 0 ? 63 : 0;
  const int target = facing > 0 ? 0 : 63;
  setup.terrain.fillBox(target, 54, 1, 6, sim::Material::Earth);
  sim::World world(setup);
  sim::Lemming actor;
  actor.x = x; actor.y = 59; actor.state = sim::State::Walker;
  actor.facing = facing > 0 ? sim::Facing::Right : sim::Facing::Left;
  world.place(actor);
  assert(world.assign(0, sim::Skill::Basher));
  world.takeCues();
  int cuts = 0;
  app::TubeScene scene;
  world.setTerrainCutObserver([&](const sim::Lemming& l,
                                  std::span<const std::array<int, 2>> cells) {
    assert(cells.size() == 2);
    for (auto c : cells) {
      assert(c[0] == target && !world.terrain().solid(c[0], c[1]));
      assert(c[1] >= (cuts == 0 ? 58 : 56) && c[1] <= (cuts == 0 ? 59 : 57));
    }
    ++cuts;
    scene.emitTerrainDebris(l, cells, world.levelClockMs(), world.terrain().height());
  });
  for (int tick = 0; tick < 7; ++tick) {
    world.tick();
    const auto cues = world.takeCues();
    const bool hit = tick == 0 || tick == 4;
    assert(cues.size() == (hit ? 1u : 0u));
    if (hit) assert(cues.front() == sim::SoundCue::Bash);
    assert(world.lemmings()[0].x == (tick < 6 ? x : target));
    if (tick < 4) {
      assert(world.terrain().solid(target, 56) && world.terrain().solid(target, 57));
      assert(!world.terrain().solid(target, 58) && !world.terrain().solid(target, 59));
      assert(cuts == 1 && scene.deathParticleCount() == 8);
    } else {
      for (int y = 56; y <= 59; ++y) assert(!world.terrain().solid(target, y));
      assert(cuts == 2 && scene.deathParticleCount() == 16);
    }
    assert(world.terrain().solid(target, 55) && world.terrain().solid(target, 60));
  }
}

void oneWayPunch(int refusedFacing, bool upper) {
  const int protectedY = upper ? 3 : 5;
  CollisionGrid grid;
  grid.width = grid.height = 8;
  grid.cells.resize(64);
  const auto stamp = [&](int x, int y, std::uint32_t word) {
    grid.cells[(7 - y) * grid.width + x] = word;
  };
  for (int x = 0; x < grid.width; ++x)
    stamp(x, 6, kCellSolid);
  for (int y = 2; y <= 5; ++y)
    stamp(0, y, kCellSolid);
  stamp(0, protectedY, kCellSolid | (refusedFacing > 0 ? kCellNoBashRight : kCellNoBashLeft));

  for (int facing : {-1, 1}) {
    sim::LevelSetup setup;
    setup.totalLemmings = 0;
    setup.terrain = sim::Terrain::fromCollisionGrid(grid);
    sim::World world(setup);
    sim::Lemming actor;
    actor.x = facing > 0 ? 7 : 1;
    actor.y = 5;
    actor.state = sim::State::Walker;
    actor.facing = facing > 0 ? sim::Facing::Right : sim::Facing::Left;
    world.place(actor);
    assert(world.assign(0, sim::Skill::Basher));
    world.takeSoundEvents();

    const bool refused = facing == refusedFacing;
    const int lastTick = refused && !upper ? 0 : 4;
    for (int tick = 0; tick <= lastTick; ++tick) {
      world.tick();
      const auto events = world.takeSoundEvents();
      assert(events.size() == ((tick == 0 || tick == 4) ? 1u : 0u));
      if (refused && tick == lastTick)
        assert(events.front().sample == "Scrape");
      else if (tick == 0 || tick == 4)
        assert(events.front().cue == sim::SoundCue::Bash);
    }
    assert(world.lemmings()[0].x == actor.x);
    assert(world.lemmings()[0].state == (refused ? sim::State::Walker : sim::State::Basher));
    assert(static_cast<int>(world.lemmings()[0].facing) == (refused ? -facing : facing));
    for (int y = 2; y <= 5; ++y)
      assert(world.terrain().solid(0, y) == (refused && (!upper || y <= 3)));

    assert(world.terrain().bashRefused(0, protectedY, refusedFacing) == refused);
    if (!refused) {
      world.terrain().set(0, protectedY, sim::Material::Built);
      assert(!world.terrain().bashRefused(0, protectedY, refusedFacing));
    }
  }
}

void installedMesh(const std::filesystem::path& dir) {
  auto lib = AssetLibrary::open(dir / "lemmings.box", dir / "patch.box");
  assert(lib);
  std::string name;
  for (const auto& candidate : levelFileNames(*lib))
    if (candidate.find("MM_BASH_002") != std::string::npos) name = candidate;
  assert(!name.empty());
  auto bytes = lib->read(name);
  assert(bytes);
  auto cylinder = findCylinderMesh(*bytes);
  auto index = indexLevel(*bytes);
  auto grid = findCollisionGrid(*bytes);
  assert(cylinder && index && grid);
  auto scene = app::TubeScene::build(*lib, *cylinder, *bytes, index->grid0);
  assert(scene);
  sim::LevelSetup setup;
  setup.totalLemmings = 0;
  setup.terrain = sim::Terrain::fromCollisionGrid(*grid);
  sim::World world(setup);
  sim::Lemming actor;
  actor.x = 153; actor.y = 55; actor.state = sim::State::Walker;
  actor.facing = sim::Facing::Left;
  world.place(actor);
  assert(world.assign(0, sim::Skill::Basher));
  const auto meshSignature = [&] {
    std::vector<std::array<std::uint32_t, 4>> indices;
    for (const auto& block : scene->cellMeshes().cells)
      for (const auto* bank : {&block.mesh0, &block.mesh4})
        for (const auto& q : *bank) indices.push_back(q.index);
    return indices;
  };
  auto previous = meshSignature();
  for (int tick = 0; tick < 5; ++tick) {
    world.tick();
    scene->syncTerrain(world.terrain());
    assert(scene->terrainError().empty() && scene->terrainUnresolved() == 0);
    for (int y = 52; y <= 55; ++y) {
      const bool solid = tick < 4 && y < 54;
      assert(world.terrain().solid(152, y) == solid);
      assert(scene->collisionGrid().solid(152, 95 - y) == solid);
    }
    if (tick == 0 || tick == 4) {
      const auto current = meshSignature();
      assert(current != previous); // each punch produces new mesh faces
      previous = current;
    }
    assert(scene->terrainCellsChanged() == (tick < 4 ? 2u : 4u));
    assert(world.lemmings()[0].x == 153);
  }
  std::cout << "Just Bash: lower two rows then upper two reach collision and mesh\n";
}
}

int main(int argc, char** argv) {
  assert(argc == 2);
  punches(1);
  punches(-1);
  oneWayPunch(1, false);
  oneWayPunch(-1, true);
  installedMesh(argv[1]);
  std::cout << "PASS: two punches, cues/debris, seam mirrors, deferred advance and installed mesh\n";
}
