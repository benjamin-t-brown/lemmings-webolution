// Short legacy invariants: rescue accounting, contact ownership and loss paths.
// Synthetic cells establish logic only; installed parsing establishes placements.
#include "sim/RetailLevel.h"
#include "app/MapScene.h"
#include "app/TubeScene.h"
#include "assets/Box.h"
#include <array>
#include <cassert>
#include <cmath>
#include <filesystem>
#include <iostream>
#include <string_view>
#include <utility>

using namespace webo;
using namespace webo::sim;

static LevelSetup fixture(int x = 10) {
  LevelSetup s;
  s.terrain = Terrain(256, 64);
  s.terrain.fillBox(0, 60, 256, 4, Material::Steel);
  s.totalLemmings = 0;
  EvilRat r;
  r.actor.x = x; r.actor.y = 59; r.actor.sheetType = 3;
  r.actor.state = State::Walker;
  r.actor.canFloat = true;
  s.rats.push_back(r);
  return s;
}

static Lemming victim(int x, int y = 59) {
  Lemming l; l.x = x; l.y = y; l.state = State::Walker;
  return l;
}

static void checkUninterruptedRatDrowning(LevelSetup setup) {
  World world(std::move(setup));
  world.tick();
  assert(world.rats()[0].actor.state == State::Drown);
  world.place(victim(11));
  world.advanceElapsedMs(0);
  assert(world.rats()[0].pendingVictim == -1 && !world.lemmings()[0].ratVictim);

  const int duration = world.rules().drownMs;
  world.advanceElapsedMs(duration - 1);
  assert(world.rats()[0].actor.state == State::Drown);
  const auto actors = app::injectedWalkersFromWorld(world, world.terrain().height());
  assert(actors.size() == 2 && actors.back().sheetType == 3 && actors.back().renderEnabled);
  assert(actors.back().clip == std::string_view("drown") && actors.back().clipSeconds > 0.0);

  world.advanceElapsedMs(1);
  assert(world.rats()[0].actor.state == State::Dead);
  assert(app::injectedWalkersFromWorld(world, world.terrain().height()).size() == 1);
  assert(world.lemmings()[0].alive());
  assert(world.spawned() == 0 && world.dead() == 0 && world.saved() == 0);
}

int main(int argc, char** argv) {
  std::cout << std::unitbuf;
  World empty(fixture());
  assert(empty.lemmings().empty() && empty.finished() && empty.activeCount() == 0);
  assert(empty.spawned() == 0 && empty.saved() == 0 && empty.dead() == 0);

  // The closed +/-1 test claims only one victim, freezes it and prevents skills.
  // Completion counts it exactly once; zero dt/pause cannot finish a punch.
  for (int dx : {-1, 0, 1}) for (int dy : {-1, 0, 1}) {
    World w(fixture()); w.place(victim(10 + dx, 59 + dy));
    w.place(victim(10, 59));
    w.advanceElapsedMs(0);
    assert(w.rats()[0].pendingVictim == 0 && w.lemmings()[0].ratVictim);
    assert(!w.assign(0, Skill::Floater) && !w.finished() && w.dead() == 0);
    auto drawn = app::injectedWalkersFromWorld(w, 64);
    assert(drawn.size() == 3 && !drawn[0].renderEnabled);
    assert(drawn.back().sheetType == 3 && std::string(drawn.back().clip) == "punch0");
    const auto cues = w.takeSoundEvents();
    const std::array<double, 3> victimPosition{double(10 + dx), double(59 + dy), 0.75};
    assert(cues.size() == 1 && !cues[0].cue && cues[0].sample == "Ohno" &&
           cues[0].position == victimPosition);
    for (int i = 0; i < 10; ++i) w.advanceElapsedMs(0);
    assert(w.dead() == 0);
    w.advanceElapsedMs(481);
    assert(w.dead() == 1 && w.lemmings()[0].state == State::Dead);
    assert(!w.lemmings()[0].ratVictim && w.rats()[0].pendingVictim == -1);
    assert(w.spawned() == 0 && w.saved() == 0);
    assert(w.takeSoundEvents().empty());
  }
  World outside(fixture()); outside.place(victim(12)); outside.advanceElapsedMs(0);
  assert(outside.rats()[0].pendingVictim == -1);
  auto airSetup = fixture(); airSetup.terrain = Terrain(256,64);
  World airborne(airSetup); airborne.place(victim(10)); airborne.advanceElapsedMs(0);
  assert(airborne.rats()[0].pendingVictim == -1);
  World seam(fixture(255)); seam.place(victim(0)); seam.advanceElapsedMs(0);
  assert(seam.rats()[0].pendingVictim == 0);

  auto s = fixture(); s.rats.push_back(s.rats[0]);
  World competing(s); competing.place(victim(10)); competing.advanceElapsedMs(0);
  assert(competing.rats()[0].pendingVictim == 0 && competing.rats()[1].pendingVictim == -1);
  competing.advanceElapsedMs(600); assert(competing.dead() == 1);

  s = fixture(); s.exits.push_back({0,0,255,63,4});
  World exits(s); exits.advanceElapsedMs(200);
  assert(exits.rats()[0].actor.alive() && exits.rats()[0].actor.x == 11);
  assert(exits.saved() == 0 && exits.dead() == 0);
  s = fixture(); s.rats[0].actor.gateCrushed = true;
  World crushed(s); crushed.advanceElapsedMs(100);
  assert(crushed.rats()[0].actor.state == State::Dead && crushed.dead() == 0);
  s = fixture();
  s.terrain.setWaterHazard(10, 60, true);
  checkUninterruptedRatDrowning(s);
  for (int kind : {0, 1, 2}) {
    s = fixture();
    FluidGate gate;
    gate.kind = kind;
    gate.minX = 10; gate.maxX = 11;
    gate.height = gate.high = 4.5;
    gate.done = gate.request;
    s.gates.push_back(gate);
    checkUninterruptedRatDrowning(s);
  }
  s = fixture(); s.terrain = Terrain(256,64); s.rats[0].actor.state = State::Faller;
  s.rats[0].actor.canFloat = false;
  World fallen(s); fallen.advanceElapsedMs(3000);
  assert(!fallen.rats()[0].actor.active() && fallen.dead() == 0);

  // A rat removed mid-punch cannot leave a permanently captive rescue actor.
  s = fixture();
  Crab crab; crab.boxOrigin = {10,4,0};
  crab.volumes.push_back({{0,0,0},{0,0,1}}); s.crabs.push_back(crab);
  World interrupted(s); interrupted.place(victim(10)); interrupted.advanceElapsedMs(0);
  assert(interrupted.rats()[0].actor.state == State::Dead && interrupted.dead() == 0);
  interrupted.advanceElapsedMs(1);
  assert(interrupted.dead() == 1 && !interrupted.lemmings()[0].ratVictim);

  // Delayed named-hazard ownership survives insertion of new rescue actors.
  s = fixture();
  NamedHazard hazard; hazard.boxOrigin = {10,4,0};
  hazard.volumes.push_back({{0,0,0},{0,0,1}});
  hazard.allContacts = false; hazard.restDuration = 1.0;
  s.namedHazards.push_back(hazard);
  World latched(s); latched.advanceElapsedMs(0);
  assert(latched.setup().namedHazards[0].pendingVictim == -2);
  latched.place(victim(100)); latched.advanceElapsedMs(1100);
  assert(latched.rats()[0].actor.state == State::Dead && latched.dead() == 0);
  assert(latched.lemmings()[0].alive());

  // Rats use the common gate crossing rule, while rescue totals stay separate.
  s = fixture(9);
  CounterGate gate; gate.mode = 2; gate.request = 1; gate.quota = 1;
  gate.left = 10; gate.right = 13; gate.bottom = 2; gate.top = 8;
  gate.initialise(s.terrain); s.counterGates.push_back(gate);
  World counted(s); counted.advanceElapsedMs(0); counted.advanceElapsedMs(200);
  assert(counted.setup().counterGates[0].count == 1);
  assert(counted.spawned() == 0 && counted.dead() == 0 && counted.saved() == 0);

  if (argc != 2) return 2;
  const std::filesystem::path root(argv[1]);
  auto lib = AssetLibrary::open(root/"lemmings.box", root/"patch.box"); assert(lib);
  auto map = app::MapScene::build(*lib, {}); assert(map);
  int count = 0, levels = 0;
  for (std::size_t node = 0; node < 102; ++node) {
    auto id = map->levelForNode(node); assert(id);
    auto setup = loadRetailLevel(*lib, *id); assert(setup);
    if (setup->rats.empty()) continue;
    ++levels; count += int(setup->rats.size());
    std::cout << node << ',' << *id << ',' << setup->rats.size() << '\n';
    for (const auto& r : setup->rats) {
      assert(!r.storedFlag38 && r.renderEnabled && r.actor.sheetType == 3);
      assert(r.actor.state == State::Faller && !r.initialFlip);
      assert(!r.actor.canClimb && r.actor.canFloat);
      assert(r.initialPosition[0] == r.actor.x);
      assert(r.initialPosition[1] == setup->terrain.height() - 1 - r.actor.y);
      assert(r.lane == 0.5 && std::abs(r.punchDurations[0] - 0.48) < 1e-9);
    }
    if (node == 28) {
      assert(setup->rats.size() == 2 && setup->rats[0].actor.x == 155 && setup->rats[1].actor.x == 173);
      World w(*setup);
      const auto actors = app::injectedWalkersFromWorld(w, setup->terrain.height());
      assert(actors.size() == 2 && actors[0].sheetType == 3 && actors[0].clip == std::string("fall"));
    }
  }
  assert(count == 45 && levels == 17);
  std::cout << "evil rat invariants passed: " << count << " rats / " << levels << " levels\n";
}
