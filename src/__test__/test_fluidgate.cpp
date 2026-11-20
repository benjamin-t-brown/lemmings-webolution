// Check fluid height and contact boundaries.
#include "sim/World.h"

#include <cmath>
#include <iostream>

using namespace webo::sim;
namespace {
int failures = 0;
void check(bool ok, const char* why) {
  if (!ok) { ++failures; std::cerr << "FAIL: " << why << '\n'; }
}
FluidGate tank(int kind, double low = 0.0) {
  FluidGate g;
  g.id = 42; g.kind = kind;
  g.minX = 10; g.maxX = 18; g.minY = 9; g.maxY = 15;
  g.high = 6; g.low = low;
  return g;
}
LevelSetup level(int kind, bool drain) {
  LevelSetup s;
  s.terrain = Terrain(40, 40);
  s.terrain.fillBox(0, 30, 40, 10, Material::Earth);
  s.totalLemmings = 0;
  s.continuousStepperClock = true;
  s.gates.push_back(tank(kind));
  s.gates[0].initialise(s.terrain);
  s.objectIds[42] = {ObjectClass::Gate, 0, "fixture"};
  if (drain) dispatchLeverMessages(s, {{42, 5, 0, 0}});
  return s;
}
}
int main() {
  for (int kind = 0; kind < 3; ++kind) {
    const double expectedRate = kind == 0 ? 2.0 : kind == 1 ? 0.5 : 1.0;
    check(FluidGate::rateFor(kind) == expectedRate, "static fluid-kind rate");
    Terrain terrain(40, 40);
    auto g = tank(kind, 1.0);
    g.initialise(terrain);
    g.request = 1;
    g.step(1.0, terrain);
    check(std::abs(g.height - (6.0 - expectedRate)) < 1e-10, "drain advances by rate times dt");
    for (int n = 0; n < 60; ++n) g.step(0.2, terrain);
    check(g.height == 1.0 && g.done == 1 && g.armed, "settle at serialized nonzero low");
    g.request = 0;
    for (int n = 0; n < 60; ++n) g.step(0.2, terrain);
    check(g.height == 6.0 && g.done == 0 && !g.armed, "raise and settle again");
    if (kind == 2)
      check(terrain.zoneWaterCells() == 0 && terrain.zoneAcidCells() == 0,
            "acid height ramp does not invent a zone-plane write");

    // The contact consumer must use the live height: an ordinary walker dies
    // in a full tank, but walks through the same footprint after drain-to-zero.
    for (bool drained : {false, true}) {
      World w(level(kind, drained));
      if (drained) for (int n = 0; n < 70; ++n) w.advanceElapsedMs(200);
      check(w.setup().gates[0].height == (drained ? 0.0 : 6.0), "contact fixture height");
      Lemming actor;
      actor.x = 12; actor.y = 29; actor.state = State::Walker; actor.facing = Facing::Right;
      w.place(actor);
      w.advanceElapsedMs(0);
      check((w.lemmings()[0].state == State::Drown) == !drained, "wet versus drained hazard");
      if (drained) {
        w.advanceElapsedMs(1600);
        check(w.lemmings()[0].state == State::Walker && w.lemmings()[0].x > 18 && w.dead() == 0,
              "ordinary walker crosses drained footprint");
      }
    }
  }
  check(FluidGate::rateFor(-1) == 0.0 && FluidGate::rateFor(3) == 0.0, "unknown kinds stay unsupported");
  return failures ? 1 : 0;
}
