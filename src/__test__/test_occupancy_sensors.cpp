// Check sensor state, dispatch and installed records.
#include "sim/RetailLevel.h"
#include "assets/Box.h"
#include "lemmings/compiled_level.hpp"
#include <bit>
#include <cassert>
#include <cmath>
#include <filesystem>
#include <iostream>

using namespace webo;
using namespace webo::sim;

static LevelSetup fixture(double cooldown = 0.0) {
  LevelSetup s;
  s.terrain = Terrain(32, 32);
  for (int x = 0; x < 32; ++x) s.terrain.set(x, 20, Material::Earth);
  s.totalLemmings = 0;
  CounterGate gate;
  gate.id = 7; gate.left = 20; gate.right = 22; gate.bottom = 10; gate.top = 14;
  gate.initialise(s.terrain);
  s.counterGates.push_back(gate);
  s.objectIds[7] = {ObjectClass::CounterGate, 0, "unit"};
  OccupancySensor sensor;
  sensor.id = 8; sensor.min = {2,12,0}; sensor.max = {4,12,1};
  sensor.cooldownSeconds = cooldown;
  sensor.occupied = {{7,4,0,0}}; sensor.empty = {{7,5,0,0}};
  s.occupancySensors.push_back(sensor);
  s.objectIds[8] = {ObjectClass::OccupancySensor, 0, "unit"};
  return s;
}

static Lemming actor(int x, int y = 19) {
  Lemming l; l.x = x; l.y = y; l.state = State::Blocker;
  return l;
}

static void stateAndCooldown() {
  World w(fixture(5)); w.place(actor(2)); w.place(actor(4));
  w.advanceElapsedMs(0);
  assert(w.setup().occupancySensors[0].state == 1);
  assert(w.setup().occupancySensors[0].contactActor == -1);
  assert(w.setup().leverStats.delivered == 2); // empty then occupied, not per actor
  assert(w.setup().counterGates[0].request == 1);
  w.advanceElapsedMs(100);
  assert(w.setup().occupancySensors[0].contactActor == 0); // first pointer, not count 2
  assert(w.setup().leverStats.delivered == 2);
  assert(std::abs(w.setup().occupancySensors[0].cooldownRemaining - 4.9) < 1e-9);

  // The expiry update only decrements the cooldown. The next update dispatches empty.
  auto s = fixture(5);
  s.occupancySensors[0].state = 1;
  s.occupancySensors[0].cooldownRemaining = .25;
  s.counterGates[0].request = 1;
  World empty(s);
  empty.advanceElapsedMs(100); empty.advanceElapsedMs(200);
  assert(empty.setup().occupancySensors[0].state == 1);
  assert(empty.setup().leverStats.delivered == 0);
  empty.advanceElapsedMs(0);
  assert(empty.setup().occupancySensors[0].state == 2);
  assert(empty.setup().counterGates[0].request == 2);
  assert(empty.setup().leverStats.delivered == 1);
  empty.advanceElapsedMs(0);
  assert(empty.setup().leverStats.delivered == 1); // repeated empty state is inert

  // If occupied as a positive cooldown crosses zero, the state stays 1 and
  // its list is not resent. The subsequent empty arm is still delayed.
  World held(s); held.place(actor(2)); held.advanceElapsedMs(100); held.advanceElapsedMs(200);
  assert(held.setup().occupancySensors[0].state == 1);
  assert(held.setup().leverStats.delivered == 0);
  assert(held.setup().occupancySensors[0].cooldownRemaining == 5);

  World zero(fixture()); zero.place(actor(2)); zero.advanceElapsedMs(0);
  zero.advanceElapsedMs(0);
  assert(zero.setup().leverStats.delivered == 4);
  assert(zero.setup().counterGates[0].request == 1); // empty-before-occupied ordering

  World legacy(fixture(5)); legacy.place(actor(2)); legacy.tick();
  assert(legacy.setup().occupancySensors[0].state == 1);
  assert(legacy.setup().counterGates[0].request == 1);
  assert(legacy.setup().counterGates[0].target == 0); // receiver sees same-tick send
}

static void volumesAndRegistry() {
  for (auto l : {actor(1), actor(5), actor(2,18), actor(2,20)}) {
    World w(fixture()); w.place(l); w.advanceElapsedMs(0);
    assert(w.setup().occupancySensors[0].state == 2);
    assert(w.setup().counterGates[0].request == 2);
  }
  auto s = fixture(); s.occupancySensors[0].min[0] = 31; s.occupancySensors[0].max[0] = 1;
  World seam(s); seam.place(actor(0)); seam.advanceElapsedMs(0);
  assert(seam.setup().counterGates[0].request == 1);
  s = fixture(); s.occupancySensors[0].min[2] = .1;
  World radial(s); radial.place(actor(2)); radial.advanceElapsedMs(0);
  assert(radial.setup().occupancySensors[0].state == 2);
  s = fixture(); auto dead = actor(2); dead.state = State::Dead;
  World removed(s); removed.place(dead); removed.advanceElapsedMs(0);
  assert(removed.setup().occupancySensors[0].state == 2);

  s = fixture(5); s.occupancySensors[0].cooldownRemaining = 3;
  dispatchLeverMessages(s, {{8,5,0,0}});
  World disabled(s); disabled.place(actor(2)); disabled.advanceElapsedMs(200);
  assert(!disabled.setup().occupancySensors[0].enabled);
  assert(disabled.setup().occupancySensors[0].cooldownRemaining == 3);
  assert(disabled.setup().occupancySensors[0].state == 0);
  dispatchLeverMessages(s, {{8,4,0,0}});
  World enabled(s); enabled.place(actor(2)); enabled.advanceElapsedMs(100);
  assert(enabled.setup().occupancySensors[0].contactActor == 0);
  assert(std::abs(enabled.setup().occupancySensors[0].cooldownRemaining - 2.9) < 1e-9);

  // Use a synthetic sensor to exercise fluid target dispatch.
  s = fixture(); FluidGate fluid; fluid.id = 9; fluid.request = 1;
  fluid.minX = 24; fluid.maxX = 26; fluid.minY = 1; fluid.high = 4;
  s.gates.push_back(fluid); s.objectIds[9] = {ObjectClass::Gate,0,"unit"};
  s.occupancySensors[0].occupied = {{9,4,0,0}};
  World wet(s); wet.place(actor(2)); wet.advanceElapsedMs(0);
  assert(wet.setup().gates[0].request == 0);
}

static void installed(const AssetLibrary& lib) {
  int total = 0;
  for (const auto id : {"M12_MT_1", "H08_MT_1"}) {
    auto loaded = loadRetailLevelReport(lib, id); assert(loaded);
    const auto& setup = loaded->setup;
    const auto raw = lib.read(std::string("COMPILED\\") + id + ".LVL"); assert(raw);
    const auto level = lemmings::read_compiled_level(
        {reinterpret_cast<const std::uint8_t*>(raw->data()), raw->size()});
    int index = 0;
    for (const auto& t : level.gadgets.tagged_entities) {
      if (t.type != 2) continue;
      const auto& sensor = setup.occupancySensors.at(index);
      const auto ref = setup.objectIds.at(t.id);
      assert(ref.cls == ObjectClass::OccupancySensor && ref.index == index);
      assert(sensor.state == int(t.values[0]) && sensor.enabled == t.flag_38);
      assert(sensor.cooldownSeconds == 5 && sensor.cooldownRemaining == 0);
      assert(sensor.occupied.size() == t.record_arrays[0].size());
      assert(sensor.empty.empty() && t.record_arrays[1].empty());
      for (int axis = 0; axis < 3; ++axis) {
        assert(sensor.min[axis] == std::bit_cast<double>(t.double_bits_48_60[axis]));
        assert(sensor.max[axis] == std::bit_cast<double>(t.double_bits_48_60[axis+3]));
      }
      // Isolate contact for each stored sensor using its authored messages, gates and terrain.
      LevelSetup s; s.terrain = setup.terrain; s.totalLemmings = 0;
      s.counterGates = setup.counterGates; s.objectIds = setup.objectIds;
      s.occupancySensors = setup.occupancySensors;
      for (auto& other : s.occupancySensors) other.enabled = false;
      s.occupancySensors[index].enabled = true;
      World w(s);
      w.place(actor(int(sensor.min[0]), setup.terrain.height()-1-int(sensor.min[1])));
      w.advanceElapsedMs(0);
      assert(w.setup().occupancySensors[index].state == 1);
      assert(w.setup().leverStats.delivered == sensor.occupied.size());
      for (std::size_t n = 0; n < sensor.occupied.size(); ++n) {
        const auto& m = sensor.occupied[n]; const auto& r = t.record_arrays[0][n];
        assert(m.targetId == int(r[0]) && m.type == r[1] && m.p1 == r[2] && m.p2 == r[3]);
        const auto target = setup.objectIds.at(m.targetId);
        assert(target.cls == ObjectClass::CounterGate);
        assert(w.setup().counterGates.at(target.index).request == (m.type == 4 ? 1 : 2));
        assert(w.setup().counterGates.at(target.index).target == (m.type == 4 ? 0 : 1));
        std::cout << id << " sensor " << sensor.id << " -> gate " << m.targetId
                  << " type " << m.type << " OK\n";
      }
      // On departure the installed empty list sends nothing, leaving the receiver latched.
      s.occupancySensors[index] = w.setup().occupancySensors[index];
      s.occupancySensors[index].cooldownRemaining = 0;
      s.counterGates = w.setup().counterGates;
      World leave(s); leave.advanceElapsedMs(0);
      assert(leave.setup().occupancySensors[index].state == 2);
      assert(leave.setup().leverStats.delivered == 0);
      ++index; ++total;
    }
    assert(index == (std::string_view(id) == "M12_MT_1" ? 5 : 2));
  }
  assert(total == 7);
}

int main(int argc, char** argv) {
  assert(argc == 2);
  stateAndCooldown(); volumesAndRegistry();
  const std::filesystem::path dir(argv[1]);
  const auto lib = AssetLibrary::open(dir/"lemmings.box", dir/"patch.box"); assert(lib);
  installed(*lib);
  std::cout << "occupancy sensor invariants passed\n";
}
