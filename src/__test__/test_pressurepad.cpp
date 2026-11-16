// Small legacy tests for a persistent one-shot latch, closed-volume contact,
// ordered message fan-out and pad presentation. No gameplay route or save IO.
#include "sim/RetailLevel.h"
#include "app/BackdropScene.h"
#include "app/MapScene.h"
#include "assets/Box.h"
#include "assets/GameData.h"
#include "lemmings/compiled_level.hpp"

#include <bit>
#include <cstdlib>
#include <filesystem>
#include <iostream>

using namespace webo;
using namespace webo::sim;
namespace {
int failures = 0;
void check(bool ok, const std::string& why) {
  if (!ok) { ++failures; std::cerr << "FAIL: " << why << '\n'; }
}
LevelSetup fixture() {
  LevelSetup s;
  s.terrain = Terrain(64, 32);
  s.totalLemmings = 0;
  PressurePad pad;
  pad.id = 0; pad.min = {2, 10, 0}; pad.max = {4, 16, 1};
  pad.messages = {{10, 5, 0, 0}};
  s.pressurePads.push_back(pad);
  FluidGate gate;
  gate.id = 10; gate.minX = 30; gate.maxX = 38;
  gate.minY = 2; gate.maxY = 8; gate.high = 6;
  gate.initialise(s.terrain);
  s.gates.push_back(gate);
  s.objectIds[10] = {ObjectClass::Gate, 0, "fixture"};
  return s;
}
Lemming actor(int x, int originalY, State state = State::Faller) {
  Lemming l;
  l.x = x; l.y = 31 - originalY; l.state = state;
  return l;
}
void units() {
  const auto s = fixture();
  for (int x : {2, 4}) for (int row : {10, 16}) {
    World w(s);
    auto l = actor(x, row);
    l.facing = Facing::Left; l.verticalDir = -1; l.sheetType = 2;
    w.place(l); w.place(l);
    w.advanceElapsedMs(0);
    check(w.setup().pressurePads[0].state == 1, "closed corner accepts an inverted falling actor");
    check(w.setup().leverStats.delivered == 1 && w.setup().gates[0].request == 1,
          "multiple contacts dispatch only once, in the activation update");
    for (int i = 0; i < 25; ++i) w.advanceElapsedMs(200);
    check(w.setup().leverStats.delivered == 1, "continued contact cannot rearm the latch");
    check(w.setup().gates[0].height == 0 && w.terrain().zoneWaterCells() == 0,
          "type 5 drains the live height and hazard plane permanently");
    // A dry point below the former surface must no longer attach Drown.
    w.place(actor(34, 3, State::Walker));
    w.advanceElapsedMs(0);
    check(w.lemmings().back().state != State::Drown, "drained volume no longer kills a normal lemming");
  }
  for (const auto& l : {actor(1, 12), actor(5, 12), actor(3, 9), actor(3, 17),
                       actor(3, 12, State::Dead), actor(3, 12, State::Exited)}) {
    World w(s); w.place(l); w.advanceElapsedMs(0);
    check(w.setup().pressurePads[0].state == 0, "outside/removed actors do not fire a pad");
  }
  {
    World wet(s); wet.place(actor(34, 3, State::Walker)); wet.advanceElapsedMs(0);
    check(wet.lemmings()[0].state == State::Drown, "same point is hazardous before draining");
    World reload(s); reload.advanceElapsedMs(0);
    check(reload.setup().pressurePads[0].state == 0 && reload.setup().leverStats.delivered == 0,
          "fresh setup resets the latch without a load-time dispatch");
  }
  for (State state : {State::Walker, State::Blocker, State::Builder, State::Drown, State::Splatter}) {
    World w(s); w.place(actor(3, 12, state)); w.advanceElapsedMs(0);
    check(w.setup().pressurePads[0].state == 1, "pad contact does not impose a walker-only filter");
  }
  // A later tagged object must win if both write the same receiver in one update.
  for (bool padLast : {false, true}) {
    auto ordered = s;
    Lever lever;
    lever.x = 2; lever.y = 15; lever.width = 2; lever.height = 6;
    lever.faceMinus = {{10, 4, 0, 0}};
    ordered.levers.push_back(lever);
    ordered.gadgetContactOrder = padLast ? std::vector<GadgetContactRef>{{false,0},{true,0}}
                                       : std::vector<GadgetContactRef>{{true,0},{false,0}};
    World w(ordered); auto l = actor(3, 12, State::Walker); l.facing = Facing::Left;
    w.place(l); w.advanceElapsedMs(0);
    check(w.setup().gates[0].request == (padLast ? 1 : 0), "tagged stream order decides conflicting messages");
  }
}
std::size_t meshCount(const SceneNode& n) {
  std::size_t count = n.meshes.size();
  for (const auto& c : n.children) count += meshCount(c);
  return count;
}
void installed(const AssetLibrary& lib) {
  auto table = loadLevelTable(lib);
  check(bool(table), "load installed map"); if (!table) return;
  int levels = 0, pads = 0, unresolved = 0;
  const auto& mapping = app::MapScene::nodeMapping();
  for (std::size_t node = 0; node < mapping.size(); ++node) {
    const auto& id = table->groups.at(mapping[node][0]).at(mapping[node][1]).id;
    auto setup = loadRetailLevelReport(lib, id);
    check(bool(setup), id + " production load"); if (!setup || setup->setup.pressurePads.empty()) continue;
    ++levels; pads += int(setup->setup.pressurePads.size());
    int missing = 0, gates = 0, other = 0;
    for (const auto& pad : setup->setup.pressurePads) for (const auto& message : pad.messages) {
      const auto it = setup->setup.objectIds.find(message.targetId);
      if (it == setup->setup.objectIds.end()) ++missing;
      else if (it->second.cls == ObjectClass::Gate) ++gates;
      else ++other;
    }
    unresolved += missing;
    std::cout << "node=" << node << " level=" << id << " pads=" << setup->setup.pressurePads.size()
              << " gate_messages=" << gates << " other_messages=" << other << " missing_ids=" << missing << '\n';
    if (id != "MM_BLOCK_BUILD_BASH") continue;
    const auto& pad = setup->setup.pressurePads[0];
    check(setup->setup.levers.empty() && pad.id == 0 && pad.state == 0 && pad.messages.size() == 5,
          "Seafood Sarnie has one untriggered pad, five messages, no lever");
    check(pad.min == std::array<double,3>{45,20,0} && pad.max == std::array<double,3>{47,26,1},
          "Seafood Sarnie uses its stored volume");
    for (const auto& m : pad.messages) check(m.type == 5 && m.targetId >= 4 && m.targetId <= 8,
                                           "Seafood Sarnie messages drain ids 4..8");
    auto blob = lib.read("COMPILED\\MM_BLOCK_BUILD_BASH.LVL");
    check(bool(blob), "read Seafood Sarnie"); if (!blob) continue;
    auto scene = app::BackdropScene::build(lib, *blob, id);
    check(bool(scene), "production scene admits Seafood Sarnie pad"); if (!scene) continue;
    auto presentation = scene->pressurePadPresentation(*blob);
    check(bool(presentation) && presentation->bindings.size() == 1, "one pad render binding");
    if (!presentation || presentation->bindings.empty()) continue;
    const auto count = [&] {
      std::size_t n = 0;
      scene->visitRoots([&](const auto& root, const auto& tree) {
        if (root.owner == "tagged-pressure-pad") n += meshCount(tree);
      });
      return n;
    };
    const auto before = count();
    scene->setPressurePadStates(*presentation, std::array<std::pair<int,int>,1>{{{0,1}}});
    check(count() == 4 && before > 4, "firing hides cut beams and retains the four-mesh base");
    scene->setPressurePadStates(*presentation, std::array<std::pair<int,int>,1>{{{0,0}}});
    check(count() == before, "fresh state restores visible beams deterministically");
    std::cout << "Seafood Sarnie pad model meshes: ready=" << before << " fired=4\n";
  }
  check(levels == 23 && pads == 45, "installed census retains all 23 levels and 45 tagged pads");
  check(unresolved == 0, "every installed pressure-pad target id resolves");
}
} // namespace
int main() {
  units();
  const char* dir = std::getenv("WEBOLUTION_GAME_DIR");
  if (dir) {
    const std::filesystem::path root(dir);
    auto lib = AssetLibrary::open(root/"lemmings.box", root/"patch.box");
    check(bool(lib), "open installed assets"); if (lib) installed(*lib);
  }
  std::cout << "pressurepad failures=" << failures << '\n';
  return failures ? 1 : 0;
}
