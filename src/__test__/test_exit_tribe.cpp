// Short legacy invariants: exit filtering, any-tribe exits and serialized
// type propagation. No recorded gameplay, injected win or original execution.
#include "assets/Box.h"
#include "assets/LevelObjects.h"
#include "sim/RetailLevel.h"
#include "lemmings/compiled_level.hpp"

#include <bit>
#include <filesystem>
#include <iostream>
#include <sstream>

using namespace webo;
namespace {
int failures = 0;
void check(bool ok, const std::string& message) {
  if (!ok) { ++failures; std::cerr << "FAIL: " << message << '\n'; }
}

void matching() {
  for (int exitType : {0, 1, 2, 4}) {
    for (int actorType : {0, 1, 2, 3}) {
      sim::LevelSetup setup;
      setup.terrain = sim::Terrain(64, 32);
      setup.terrain.fillBox(0, 24, 64, 8, sim::Material::Earth);
      setup.totalLemmings = 0;
      setup.exits.push_back({16, 20, 2, 3, exitType});
      sim::World world(setup);
      sim::Lemming actor;
      actor.x = 16; actor.y = 23; actor.state = sim::State::Walker;
      actor.sheetType = actorType;
      world.place(actor);
      world.tick();
      const bool accepted = actorType != 3 && (exitType == 4 || exitType == actorType);
      const auto label = std::to_string(actorType) + " at exit " + std::to_string(exitType);
      check(world.saved() == int(accepted), label + " save count");
      check(world.lemmings()[0].state == (accepted ? sim::State::Exited : sim::State::Walker),
            label + " state");
      if (!accepted)
        check(world.lemmings()[0].x == 17, label + " continues walking through");
      int exitCues = 0;
      for (const auto cue : world.takeCues()) exitCues += cue == sim::SoundCue::Exit;
      check(exitCues == int(accepted), label + " exit cue");
    }
  }
  // A mismatched overlapping exit must not hide a later matching one.
  sim::LevelSetup setup;
  setup.terrain = sim::Terrain(64, 32);
  setup.terrain.fillBox(0, 24, 64, 8, sim::Material::Earth);
  setup.totalLemmings = 0;
  setup.exits = {{16, 20, 2, 3, 0}, {16, 20, 2, 3, 1}};
  sim::World world(setup);
  sim::Lemming actor;
  actor.x = 16; actor.y = 23; actor.state = sim::State::Walker; actor.sheetType = 1;
  world.place(actor); world.tick();
  check(world.saved() == 1, "later matching exit remains usable");
}

void installed(const std::filesystem::path& gameDir) {
  auto lib = AssetLibrary::open(gameDir / "lemmings.box", gameDir / "patch.box");
  if (!lib) { check(false, lib.error()); return; }
  int levels = 0, exits = 0;
  std::cout << "CENSUS\tlevel\thatches(type:count:x:y:model)\texits(type:x:y:w:h:model)\tusable\n";
  for (const auto& name : lib->names()) {
    if (!name.starts_with("COMPILED\\") || !name.ends_with(".LVL")) continue;
    ++levels;
    auto bytes = lib->read(name);
    if (!bytes) { check(false, bytes.error()); continue; }
    const auto decoded = lemmings::read_compiled_level(
        {reinterpret_cast<const std::uint8_t*>(bytes->data()), bytes->size()});
    auto level = sim::loadRetailLevel(*lib, name);
    auto objects = findLevelObjects(*bytes);
    if (!level || !objects) { check(false, name + " load: " + (!level ? level.error() : objects.error())); continue; }
    const auto& rawExits = decoded.gadgets.entities_426c6b;
    const auto& rawHatches = decoded.gadgets.entities_425e50;
    check(level->entrances.size() == rawHatches.size(), name + " hatch count");
    check(level->exits.size() == rawExits.size(), name + " exit count");
    std::ostringstream hatchText, exitText;
    bool usable = true;
    for (std::size_t i = 0; i < rawHatches.size(); ++i) {
      const auto& h = rawHatches[i];
      const auto type = static_cast<int>(h.value_b0);
      check(level->entrances.at(i).sheetType == type, name + " hatch tribe");
      if (h.shorts_98_9a_9c[0] > 0) {
        bool found = false;
        for (const auto& e : level->exits) found |= e.accepts(type);
        usable &= found;
      }
      if (i) hatchText << ';';
      hatchText << type << ':' << h.shorts_98_9a_9c[0] << ':' << h.shorts_98_9a_9c[1]
                << ':' << h.shorts_98_9a_9c[2] << ':' << (h.model ? h.model->name : "none");
    }
    for (std::size_t i = 0; i < rawExits.size(); ++i) {
      ++exits;
      const auto& raw = rawExits[i]; const auto& e = level->exits.at(i);
      check(e.sheetType == static_cast<int>(raw.value_98), name + " serialized exit tribe");
      // Independently correlate stream order with the located model: the
      // serialized AABB and the existing pose-derived trigger must agree.
      const auto value = [&](int k) { return std::bit_cast<double>(raw.double_bits_48_60[k]); };
      check(value(0) == e.x && level->terrain.height() - 1 - value(4) == e.y &&
                value(3) - value(0) == e.width && value(4) - value(1) == e.height,
            name + " exit placement order/AABB");
      if (i) exitText << ';';
      exitText << e.sheetType << ':' << e.x << ':' << e.y << ':' << e.width << ':' << e.height
               << ':' << (raw.model ? raw.model->name : "none");
    }
    check(usable, name + " every released tribe has a compatible exit");
    std::cout << "CENSUS\t" << name << '\t' << hatchText.str() << '\t' << exitText.str()
              << '\t' << usable << '\n';
  }
  check(levels == 102, "102 installed levels inspected");
  std::cout << "TOTAL levels=" << levels << " exits=" << exits << " failures=" << failures << '\n';
}
}

int main(int argc, char** argv) {
  matching();
  if (argc == 2) installed(argv[1]);
  return failures ? 1 : 0;
}
