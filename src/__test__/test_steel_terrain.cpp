// Check protected cells and steel material boundaries after runtime mesh rebuilds.
#include "app/TubeScene.h"
#include "assets/Box.h"
#include "assets/LevelIndex.h"
#include "sim/World.h"

#include <algorithm>
#include <cassert>
#include <cmath>
#include <filesystem>
#include <iostream>

using namespace webo;

namespace {
bool near(const Vec3& a, const Vec3& b) {
  return std::abs(a.x - b.x) < 1e-9 && std::abs(a.y - b.y) < 1e-9 &&
         std::abs(a.z - b.z) < 1e-9;
}

void blast(bool nuke) {
  sim::LevelSetup setup;
  setup.totalLemmings = 0;
  setup.terrain = sim::Terrain(64, 80);
  setup.terrain.fillBox(0, 60, 64, 20, sim::Material::Earth);
  for (int y = 60; y < 80; ++y)
    for (int x = 21; x < 24; ++x) setup.terrain.set(x, y, sim::Material::Steel);
  sim::World world(setup);
  sim::Lemming actor;
  actor.x = 20; actor.y = 59; actor.state = sim::State::Blocker;
  world.place(actor);
  if (nuke) world.nuke();
  else assert(world.assign(0, sim::Skill::Bomber));
  for (int i = 0; i < 45; ++i) world.tick();
  assert(world.dead() == 1);
  assert(world.terrain().solidCells() < setup.terrain.solidCells());
  for (int y = 60; y < 80; ++y)
    for (int x = 21; x < 24; ++x)
      assert(world.terrain().at(x, y) == sim::Material::Steel);
  std::cout << (nuke ? "nuke" : "bomber") << ": steel collision intact, earth cratered\n";
}

void installedMesh(const std::filesystem::path& dir, bool explosion) {
  auto lib = AssetLibrary::open(dir / "lemmings.box", dir / "patch.box");
  assert(lib);
  std::string name;
  for (const auto& candidate : levelFileNames(*lib))
    if (candidate.find("E20_MG_1") != std::string::npos) name = candidate;
  assert(!name.empty());
  auto bytes = lib->read(name);
  assert(bytes);
  auto cylinder = findCylinderMesh(*bytes);
  auto index = indexLevel(*bytes);
  auto grid = findCollisionGrid(*bytes);
  assert(cylinder && index && grid);
  auto scene = app::TubeScene::build(*lib, *cylinder, *bytes, index->grid0);
  assert(scene);
  auto terrain = sim::Terrain::fromCollisionGrid(*grid);
  // Upper shelf immediately left of the steel pillar at columns 141..143.
  // Coordinates here are clone Y-down; file row 81 is clone row 14.
  assert(terrain.at(140, 14) == sim::Material::Earth);
  assert(terrain.at(141, 14) == sim::Material::Steel);
  if (explosion) assert(terrain.clearExplosion(139, 13) > 0);
  else assert(terrain.clearBox(138, 14, 3, 1) == 3);
  scene->syncTerrain(terrain);
  assert(scene->terrainError().empty() && scene->terrainUnresolved() == 0);
  for (int y = 0; y < grid->height; ++y)
    for (int x = 0; x < grid->width; ++x) {
      assert(scene->collisionGrid().solid(x, y) == terrain.solid(x, grid->height - 1 - y));
      if (grid->at(x, y) & kCellIndestructible) {
        assert(scene->collisionGrid().at(x, y) == grid->at(x, y));
        if (grid->solid(x, y))
          assert(terrain.at(x, grid->height - 1 - y) == sim::Material::Steel);
      }
    }
  const auto* block = scene->cellMeshes().at(17, 5);
  assert(block);
  const auto key = [&](int x, int y) { return index->grid0[(x + y * 257) * 2 + 1]; };
  const std::array<std::uint32_t, 4> steelFace{
      key(141, 80), key(141, 96), key(144, 96), key(144, 80)};
  for (auto i : steelFace) assert(i != 0xffffffffu);
  // The entire steel front retains its original plane, independently of how
  // the neighbouring snow's rectangles now split. This catches mixed merging.
  for (const auto* bank : {&block->mesh0, &block->mesh4})
    assert(std::ranges::any_of(*bank, [&](const auto& q) { return q.index == steelFace; }));
  // Newly exposed steel uses coordinate height 112, placing this corner at Y=81.
  const auto expected = cylinderTemplateVertex(*cylinder, 141, 81, 1, int(cylinder->faceCount));
  const auto oldShape = cylinderTemplateVertex(*cylinder, 138, 81, 1, grid->height);
  assert(expected && expected->y == 81 && oldShape && oldShape->y == 80);
  bool steelSide = false, ordinaryShape = false;
  for (const auto* bank : {&block->mesh0, &block->mesh4})
    for (const auto& q : *bank)
      for (auto i : q.index) {
        const auto& v = scene->terrainMesh().surface.at(i);
        steelSide |= near(v, *expected);
        ordinaryShape |= near(v, *oldShape);
      }
  // The bomb exposes a different span on the steel side; the dig's one-cell
  // notch must use the precise row-81 corner above.
  if (!explosion) assert(steelSide && ordinaryShape);
  std::cout << "E20_MG_1 " << (explosion ? "blast" : "dig")
            << ": steel (141,81 file / 141,14 clone) intact; steel face unchanged;"
            << " new steel side Y=" << expected->y << "; ordinary shaping retained\n";
}

void fullMixedBlock() {
  CollisionGrid grid;
  grid.height = 16;
  grid.cells.assign(256 * 16, kCellSolid);
  for (int y = 0; y < 16; ++y) grid.cells[y * 256 + 4] |= kCellIndestructible;
  std::vector<std::uint32_t> keys(257 * 17 * 2);
  for (std::size_t i = 0; i < keys.size(); ++i) keys[i] = static_cast<std::uint32_t>(i);
  auto mesh = buildCellMeshes(keys, {}, grid, keys.size());
  assert(mesh && !mesh->at(0, 0)->hasInner);
  const std::array<std::pair<int, int>, 1> blocks{{{0, 0}}};
  assert(rebuildCellMeshCells(*mesh, keys, {}, grid, keys.size(), blocks));
  assert(!mesh->at(0, 0)->hasInner && !mesh->at(0, 0)->wall);
}
}

int main(int argc, char** argv) {
  assert(argc == 2);
  blast(false);
  blast(true);
  fullMixedBlock();
  installedMesh(argv[1], false);
  installedMesh(argv[1], true);
}
