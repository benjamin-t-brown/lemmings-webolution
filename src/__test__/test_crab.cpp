// Small legacy logic checks: relative contact bounds, attack latch, loss
// accounting and deterministic animation state. No scripted gameplay route.
#include "sim/RetailLevel.h"
#include "app/BackdropScene.h"
#include "app/MapScene.h"
#include "assets/Box.h"
#include "assets/GameData.h"

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
  s.terrain = Terrain(256, 64);
  s.totalLemmings = 0;
  Crab c;
  c.boxOrigin = {110, 9, 0};
  c.volumes.push_back({{0, 0, 0}, {14, 7, 1}});
  c.restDuration = 1;
  c.attackDuration = 1.1666666269302368;
  s.crabs.push_back(c);
  return s;
}
Lemming actor(int x, int originalY, State state = State::Walker) {
  Lemming l;
  l.x = x; l.y = 63 - originalY; l.state = state;
  return l;
}
void units() {
  for (int x : {110, 124}) for (int y : {9, 16}) {
    World w(fixture());
    auto l = actor(x, y);
    l.facing = Facing::Left; l.verticalDir = -1; l.sheetType = 2;
    w.place(l); w.place(l);
    w.place(actor(109, y)); w.place(actor(125, y));
    w.place(actor(x, 8)); w.place(actor(x, 17));
    w.place(actor(x, y, State::Dead));
    w.place(actor(x, y, State::Exited));
    w.place(actor(x, y, State::Splatter));
    int particles = 0;
    w.setDeathObserver([&](const Lemming& victim, bool blast, auto cells) {
      check(victim.state == State::Dead && !blast && cells.empty(), "kill emits body pieces without terrain damage");
      ++particles;
    });
    w.advanceElapsedMs(0);
    check(w.dead() == 2 && particles == 2 && w.setup().crabs[0].attacking,
          "closed relative volume kills all contacts once, including its far two columns");
    check(w.lemmings()[2].active() && w.lemmings()[3].active() &&
          w.lemmings()[4].active() && w.lemmings()[5].active(), "outside actors survive");
    w.place(l);
    w.advanceElapsedMs(0);
    check(w.lemmings().back().active() && w.dead() == 2,
          "attack latch does not kill a new contact before completion");
    w.advanceElapsedMs(100);
    check(w.setup().crabs[0].attacking && w.setup().crabs[0].animationSeconds == 0.1,
          "attack uses admitted simulation time");
  }
  // A stable non-mover stays in the volume until the next attack. This checks
  // rearming without a replay or a per-level command route.
  auto s = fixture();
  s.terrain.fillBox(0, 55, 256, 1, Material::Earth);
  World w(s);
  w.place(actor(116, 9, State::Blocker));
  w.advanceElapsedMs(0);
  w.place(actor(116, 9, State::Blocker));
  for (int i = 0; i < 11; ++i) w.advanceElapsedMs(100);
  check(w.dead() == 1, "attack stays busy for its stored duration");
  w.advanceElapsedMs(67);
  check(w.dead() == 2 && w.setup().crabs[0].attacking &&
        w.setup().crabs[0].animationSeconds == 0, "completion rearms and can attack in the same update");
  w.advanceElapsedMs(1167);
  check(!w.setup().crabs[0].attacking, "empty attack returns to idle");
  World a(s), b(s);
  a.place(actor(116, 9, State::Blocker)); b.place(actor(116, 9, State::Blocker));
  for (int dt : {0, 17, 83, 100, 0, 100, 100, 100, 100, 100, 100, 100, 100, 100, 67, 500}) {
    a.advanceElapsedMs(dt); b.advanceElapsedMs(dt);
    check(a.dead() == b.dead() && a.setup().crabs[0].attacking == b.setup().crabs[0].attacking &&
          a.setup().crabs[0].animationSeconds == b.setup().crabs[0].animationSeconds,
          "identical admitted updates reproduce creature state");
  }
  auto disabled = s; disabled.crabs[0].enabled = false;
  World inactive(disabled); inactive.place(actor(116, 9)); inactive.advanceElapsedMs(0);
  check(inactive.dead() == 0, "disabled hazard stays inert");
  World reloaded(s);
  check(!reloaded.setup().crabs[0].attacking && reloaded.setup().crabs[0].animationSeconds == 0,
        "reload starts with rest animation and no attack latch");
}
// Fast legacy coverage for fractional surface/contact boundaries. This guards
// the integer-coordinate adapter without adding a scripted per-level route.
void surfaceContact() {
  for (int kind : {0, 2}) {
    auto s = fixture();
    FluidGate g;
    g.kind = kind; g.minX = 110; g.maxX = 125;
    g.minY = 9; g.height = g.high = 7.5;
    s.gates.push_back(g);
    auto water = actor(116, 16);
    water.sheetType = kind == 0 ? 1 : 2;
    World above(s);
    above.place(water); above.advanceElapsedMs(0);
    check(above.dead() == 0 && above.lemmings()[0].active() &&
          !above.setup().crabs[0].attacking, "matching fractional surface above box is safe");

    auto edge = s; edge.gates[0].height = edge.gates[0].high = 7.0;
    World touching(edge);
    touching.place(water); touching.advanceElapsedMs(0);
    check(touching.dead() == 1, "surface on the closed box edge still touches crab");

    World floor(s);
    auto atCrab = water; atCrab.y = actor(116, 9).y;
    // A walker placed underwater is lifted by attachFluidHazard before
    // contact. A stationary actor lets this check reach the crab itself.
    atCrab.state = State::Blocker;
    floor.place(atCrab); floor.advanceElapsedMs(0);
    check(floor.dead() == 1, "matching tribe at crab's own cell is not immune");

    auto dry = s; dry.gates[0].height = 0; dry.gates[0].request = 1;
    World drained(dry);
    drained.place(water); drained.advanceElapsedMs(0);
    check(drained.dead() == 1, "drained tank retains ordinary crab contact");

    auto away = s; away.gates[0].minX = 120;
    World outside(away);
    outside.place(water); outside.advanceElapsedMs(0);
    check(outside.dead() == 1, "nearby fluid cannot lift a point outside its span");

    World ordinary(s);
    auto otherTribe = water; otherTribe.sheetType = 0;
    ordinary.place(otherTribe); ordinary.advanceElapsedMs(0);
    check(ordinary.dead() == 1, "other tribes receive no surface adapter exemption");

    World inverted(s);
    water.verticalDir = -1;
    inverted.place(water); inverted.advanceElapsedMs(0);
    check(inverted.dead() == 1, "inverted actor is not supported by the fluid surface");
  }
}
void installed(const AssetLibrary& lib) {
  const auto table = loadLevelTable(lib);
  check(bool(table), "read installed map"); if (!table) return;
  const auto& mapping = app::MapScene::nodeMapping();
  int levels = 0, count = 0;
  for (int node : {15, 19, 21, 36, 39, 44, 46, 49, 63, 67, 84, 96}) {
    const auto& id = table->groups.at(mapping[node][0]).at(mapping[node][1]).id;
    auto report = loadRetailLevelReport(lib, id);
    check(bool(report), id + " production load"); if (!report) continue;
    const auto& crabs = report->setup.crabs;
    check(!crabs.empty(), id + " has its structured crab records");
    ++levels; count += int(crabs.size());
    auto resolved = resolveRetailLevel(lib, id);
    auto blob = lib.read(resolved->file);
    auto scene = app::BackdropScene::build(lib, *blob, id);
    check(bool(scene), id + " production scene"); if (!scene) continue;
    auto presentation = scene->crabPresentation(*blob);
    check(bool(presentation), id + " rest/attack clips parse"); if (!presentation) continue;
    check(presentation->bindings.size() == crabs.size(), id + " every hazard has a rendered model");
    for (const auto& c : crabs) {
      check(c.volumes.size() == 1 && c.volumes[0].max == std::array<double,3>{14,7,1},
            id + " relative volume resolves");
      check(c.restDuration == 1 && c.attackDuration > 1.16 && c.attackDuration < 1.17,
            id + " stored animation durations");
      std::cout << "node=" << node << " level=" << id << " entity=" << c.entityIndex
                << " contact=[" << c.boxOrigin[0] << ',' << c.boxOrigin[0]+14 << "]x["
                << c.boxOrigin[1] << ',' << c.boxOrigin[1]+7 << "] STAT rest=" << c.restDuration
                << " attack=" << c.attackDuration << '\n';
    }
    std::vector<std::array<double,12>> idle;
    std::array<double,12> placement{};
    scene->visitRoots([&](const auto& source, const SceneNode& root) {
      if (source.owner != "named-crab") return;
      placement = root.matrix;
      for (const auto& child : root.children) idle.push_back(child.matrix);
    });
    auto state = crabs;
    state[0].animationSeconds = 0.25;
    scene->setCrabStates(*presentation, state);
    bool animated = false;
    scene->visitRoots([&](const auto& source, const SceneNode& root) {
      if (source.owner != "named-crab" || source.entityIndex != state[0].entityIndex) return;
      for (std::size_t i = 0; i < root.children.size(); ++i)
        animated |= root.children[i].matrix != idle[i];
    });
    check(animated, id + " rest animation moves the creature parts");
    if (node == 15) {
      check(crabs[0].boxOrigin == std::array<double,3>{110,9,0}, "Seafood Sarnie contact origin");
      state[0].attacking = true; state[0].animationSeconds = 0.5;
      scene->setCrabStates(*presentation, state);
      bool attacked = false;
      scene->visitRoots([&](const auto& source, const SceneNode& root) {
        if (source.owner != "named-crab") return;
        check(root.matrix == placement, "stationary crab retains its stored world transform");
        for (std::size_t i = 0; i < root.children.size(); ++i) attacked |= root.children[i].matrix != idle[i];
      });
      check(attacked, "attack animates model parts at the data placement");
    }
  }
  check(levels == 12 && count == 14, "all 14 installed crabs on 12 levels resolve");
}
}
int main() {
  units();
  surfaceContact();
  if (const char* dir = std::getenv("WEBOLUTION_GAME_DIR")) {
    const std::filesystem::path root(dir);
    auto lib = AssetLibrary::open(root/"lemmings.box", root/"patch.box");
    check(bool(lib), "open installed assets"); if (lib) installed(*lib);
  }
  std::cout << "crab failures=" << failures << '\n';
  return failures ? 1 : 0;
}
