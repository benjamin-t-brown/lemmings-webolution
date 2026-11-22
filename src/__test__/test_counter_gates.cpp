// Short state/collision/corpus invariants. No scripted level route or save.
#include "sim/RetailLevel.h"
#include "assets/Box.h"
#include "assets/GameData.h"
#include "assets/CollisionGrid.h"
#include "lemmings/compiled_level.hpp"
#include "app/MapScene.h"
#include "app/TubeScene.h"
#include "assets/LevelIndex.h"
#include <cassert>
#include <cmath>
#include <filesystem>
#include <iostream>

using namespace webo;
using namespace webo::sim;

static CounterGate gate(int mode = 2) {
  CounterGate g;
  g.id = 7; g.left = 10; g.right = 13; g.bottom = 2; g.top = 8;
  g.mode = mode; g.request = 1; g.quota = 1;
  return g;
}

static LevelSetup setup(CounterGate g) {
  LevelSetup s;
  s.terrain = Terrain(256, 96);
  s.terrain.fillBox(0, 94, 256, 2, Material::Steel);
  s.totalLemmings = 0;
  s.timeLimitTicks = 10000;
  g.initialise(s.terrain);
  s.counterGates.push_back(g);
  s.objectIds[7] = {ObjectClass::CounterGate, 0, "unit"};
  return s;
}

static Lemming walker(int x, Facing face = Facing::Right) {
  Lemming l; l.x = x; l.y = 93; l.state = State::Walker; l.facing = face;
  return l;
}

int main(int argc, char** argv) {
  std::cout << std::unitbuf;
  // 0x403 is gate-owned; refusal/water/blocker planes survive both writes.
  CollisionGrid grid; grid.width = 256; grid.height = 96;
  grid.cells.assign(256 * 96, 0);
  grid.cells[2 * 256 + 10] = 0x403;
  grid.cells[2 * 256 + 11] = 0x423;
  auto t = Terrain::fromCollisionGrid(grid);
  auto g = gate(); g.request = 2; g.initialise(t);
  assert(t.at(10, 93) == Material::Steel && t.noDigInverted(11, 93));
  g.target = 0;
  g.animate(0.1, t);
  assert(std::abs(g.progress - 0.7) < 1e-10 && std::abs(g.surfaceFraction() - 0.49) < 1e-10);
  assert(g.collisionClosed);
  g.animate(0.3, t);
  assert(!g.collisionClosed && !t.solid(10, 93));
  assert(t.noDigInverted(11, 93) && t.solid(11, 93));
  g.target = 1;
  g.animate(0.1, t);
  assert(std::abs(g.surfaceFraction() - 0.51) < 1e-10);
  g.animate(0.3, t);
  assert(g.collisionClosed && t.at(10, 93) == Material::Steel);

  auto s = setup(gate());
  dispatchLeverMessages(s, {{7,5,0,0}}); assert(s.counterGates[0].request == 2);
  dispatchLeverMessages(s, {{7,4,0,0}}); assert(s.counterGates[0].request == 1);
  dispatchLeverMessages(s, {{7,0,0,0}}); assert(s.counterGates[0].request == 1);
  World w(s); w.place(walker(9));
  w.advanceElapsedMs(0); // settle initial request; no Rumble on initial commit
  assert(w.takeSoundEvents().empty());
  for (int i = 0; i < 10; ++i) w.advanceElapsedMs(0);
  assert(w.setup().counterGates[0].count == 0);
  w.advanceElapsedMs(200);
  assert(w.setup().counterGates[0].count == 1 && w.setup().counterGates[0].remaining() == 0);
  assert(!w.setup().counterGates[0].collisionClosed);
  for (int i = 0; i < 8; ++i) w.advanceElapsedMs(200);
  assert(w.setup().counterGates[0].collisionClosed && w.dead() == 0);
  int rumbles = 0;
  for (const auto& e : w.takeSoundEvents()) rumbles += e.sample == "Rumble";
  assert(rumbles == 1);

  g = gate(); g.quota = 50; g.direction = 1;
  World directional(setup(g)); directional.place(walker(9));
  directional.advanceElapsedMs(0); directional.advanceElapsedMs(200);
  assert(directional.setup().counterGates[0].count == 1);
  directional.place(walker(13, Facing::Left)); directional.advanceElapsedMs(200);
  assert(directional.setup().counterGates[0].count == 0);

  g = gate(); g.left = 254; g.right = 258;
  World seam(setup(g)); seam.place(walker(253));
  seam.advanceElapsedMs(0); seam.advanceElapsedMs(200);
  assert(seam.setup().counterGates[0].count == 1);

  g = gate(1); g.top = 4;
  World vertical(setup(g));
  Lemming falling; falling.x = 11; falling.y = 90; falling.state = State::Faller;
  vertical.place(falling); vertical.advanceElapsedMs(0); vertical.advanceElapsedMs(200);
  assert(vertical.setup().counterGates[0].count == 1);

  // Sensor cooldown/list ordering reaches the same registry as a pad/lever.
  s = setup(gate()); s.counterGates[0].request = 2;
  OccupancySensor sensor; sensor.min = {0,2,0}; sensor.max = {0,2,1};
  sensor.occupied = {{7,4,0,0}}; sensor.empty = {{7,5,0,0}};
  s.occupancySensors.push_back(sensor);
  World sensed(s); sensed.place(walker(0)); sensed.advanceElapsedMs(0);
  assert(sensed.setup().counterGates[0].request == 1 && sensed.setup().leverStats.delivered == 2);
  sensed.advanceElapsedMs(200);
  assert(sensed.setup().counterGates[0].request == 2);

  s = setup(gate());
  s.counterGates[0].previousRequest = 2;
  s.counterGates[0].request = 2; s.counterGates[0].count = 7;
  PressurePad pad; pad.min = {9,2,0}; pad.max = {9,2,1}; pad.messages = {{7,4,0,0}};
  s.pressurePads.push_back(pad);
  World reset(s); reset.place(walker(9)); reset.advanceElapsedMs(0);
  assert(reset.setup().counterGates[0].count == 0 && reset.setup().counterGates[0].request == 1);

  s = setup(gate());
  s.counterGates[0].target = 1; s.counterGates[0].request = s.counterGates[0].previousRequest = 2;
  World crushed(s); auto stationary = walker(11); stationary.state = State::Blocker;
  crushed.place(stationary);
  crushed.advanceElapsedMs(100); assert(crushed.lemmings()[0].gateCrushed);
  crushed.advanceElapsedMs(100); assert(crushed.dead() == 1 && !crushed.lemmings()[0].alive());

  if (argc != 2) return 2;
  const std::filesystem::path dir(argv[1]);
  const auto lib = AssetLibrary::open(dir / "lemmings.box", dir / "patch.box"); assert(lib);
  const auto map = app::MapScene::build(*lib, {}); assert(map);
  std::size_t total = 0, levels = 0, counted = 0;
  for (std::size_t node = 0; node < 102; ++node) {
    const auto id = map->levelForNode(node); assert(id);
    const auto report = loadRetailLevelReport(*lib, *id); assert(report);
    const auto& gates = report->setup.counterGates;
    if (gates.empty()) continue;
    ++levels; total += gates.size();
    std::cout << node << ',' << *id << ',' << gates.size() << '\n';
    for (const auto& item : gates) {
      counted += item.hasCounter();
      assert(item.right > item.left && item.top > item.bottom);
      assert(report->setup.objectIds.at(item.id).cls == ObjectClass::CounterGate);
      if (item.hasCounter()) assert(item.quota > 0 && item.quota <= 50);
    }
    if (node == 25) {
      assert(gates.size() == 1 && gates[0].mode == 0 && !gates[0].hasCounter());
      const auto resolved = resolveRetailLevel(*lib, *id); assert(resolved);
      const auto bytes = lib->read(resolved->file); assert(bytes);
      const auto mesh = findCylinderMesh(*bytes); const auto index = indexLevel(*bytes);
      assert(mesh && index);
      auto scene = app::TubeScene::build(*lib, *mesh, *bytes, index->grid0); assert(scene);
      // Use installed vertexDoor records to exercise the individual gate-face indices.
      // Uniform pool[0] indices would collapse the faces.
      assert(scene->cellMeshes().unresolved == 0);
      scene->syncTerrain(report->setup.terrain, gates);
      scene->syncCounterGates(gates);
      const auto closed = scene->terrainMesh().surface;
      auto open = gates; open[0].progress = open[0].target = 0;
      scene->syncCounterGates(open);
      std::size_t moved = 0;
      const auto& after = scene->terrainMesh().surface;
      for (std::size_t i = 0; i < after.size(); ++i)
        moved += closed[i].x != after[i].x || closed[i].y != after[i].y || closed[i].z != after[i].z;
      assert(moved >= 4);
      const auto revision = scene->counterGateRevision();
      scene->syncCounterGates(open); assert(scene->counterGateRevision() == revision);
    }
    if (node == 35) {
      const auto resolved = resolveRetailLevel(*lib, *id); assert(resolved);
      const auto bytes = lib->read(resolved->file); assert(bytes);
      const auto mesh = findCylinderMesh(*bytes); const auto index = indexLevel(*bytes);
      assert(mesh && index);
      auto scene = app::TubeScene::build(*lib, *mesh, *bytes, index->grid0); assert(scene);
      assert(scene->cellMeshes().unresolved == 0);
      // The static strip above gate 10 starts at y=89, between stored height keys.
      // Its generated outer corners must stay put when the gate closes.
      const auto* cell = scene->cellMeshes().at(16, 5); assert(cell);
      const auto& surface = scene->terrainMesh().surface;
      const auto strip = std::find_if(cell->mesh0.begin(), cell->mesh0.end(), [&](const auto& quad) {
        return !quad.gate && quad.coord == 128 && surface[quad.index[0]].y == 89.0 &&
            surface[quad.index[1]].y == 96.0;
      });
      assert(strip != cell->mesh0.end());
      const auto corner = strip->index[0];
      assert(corner >= mesh->surface.size());
      const auto position = surface[corner];
      auto closed = gates;
      for (auto& item : closed)
        if (item.id == 10) item.progress = item.target = 1.0;
      scene->syncCounterGates(gates);
      scene->syncCounterGates(closed);
      assert(surface[corner].x == position.x && surface[corner].y == position.y &&
             surface[corner].z == position.z);

      // Isolate body-footprint clipping by drawing into an empty depth buffer.
      // At the same progress, opening and closing have different eased depths.
      auto lower = std::ranges::find(closed, 1, &CounterGate::id);
      assert(lower != closed.end());
      app::CameraParams params;
      params.yawUnits = 22;
      params.levelEyeHeight = report->setup.terrain.height();
      const auto camera = app::buildCamera(params);
      app::SceneOptions options;
      options.injectedWalkers = {{22, 34, 0.5, 1}};
      app::Raster raster(camera.width, camera.height);
      const auto showsBody = [&](double progress, double target) {
        lower->progress = progress;
        lower->target = target;
        scene->syncCounterGates(closed);
        raster.clear(0, 0, 0);
        scene->drawInjectedWalkers(raster, camera, options, *lib);
        return std::ranges::any_of(raster.framebuffer().rgb, [](auto v) { return v != 0; });
      };
      assert(!showsBody(1.0, 1.0));
      assert(showsBody(0.5, 0.0));
      assert(showsBody(0.0, 0.0));
      assert(!showsBody(0.5, 1.0));
      assert(!showsBody(1.0, 1.0));
    }
    if (node == 43) {
      assert(std::any_of(gates.begin(), gates.end(),
          [](const auto& item) { return item.quota == 10 && item.mode == 2; }));
      const auto resolved = resolveRetailLevel(*lib, *id); assert(resolved);
      const auto bytes = lib->read(resolved->file); assert(bytes);
      const auto mesh = findCylinderMesh(*bytes); const auto index = indexLevel(*bytes);
      assert(mesh && index);
      auto scene = app::TubeScene::build(*lib, *mesh, *bytes, index->grid0); assert(scene);
      app::CameraParams params; params.yawUnits = 170; params.levelEyeHeight = 80;
      const auto camera = app::buildCamera(params);
      app::Raster initial(640,480), crossed(640,480);
      initial.clear(0,0,0); crossed.clear(0,0,0);
      scene->renderCounterGates(initial, camera, *lib, gates);
      auto states = gates;
      for (auto& item : states) if (item.hasCounter()) ++item.count;
      scene->renderCounterGates(crossed, camera, *lib, states);
      assert(initial.framebuffer().rgb != crossed.framebuffer().rgb);
      assert(std::any_of(initial.framebuffer().rgb.begin(), initial.framebuffer().rgb.end(),
          [](auto v) { return v != 0; }));
    }
  }
  assert(total == 231 && levels == 52 && counted == 22);
  std::cout << "counter gate invariants passed: " << total << " gates / " << levels << " levels / " << counted << " quota gates\n";
}
