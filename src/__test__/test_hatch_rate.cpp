// Seconds-long invariants: clicked hatch limits and v1 replay's hatch selector.
#include "app/Replay.hpp"
#include "assets/Box.h"
#include "assets/LevelObjects.h"

#include <cstdlib>
#include <iostream>

using namespace webo;
namespace {
int failures = 0;
void check(bool ok, const char* message) {
  if (!ok) { ++failures; std::cerr << "FAIL " << message << '\n'; }
}
void units(const AssetLibrary& lib, const std::filesystem::path& directory) {
  const auto rates = loadReleaseRateTable(lib);
  const auto setup = sim::loadRetailLevel(lib, "MM_CLIMB_002");
  auto control = app::RuntimeReleaseRateControl::create(lib, "MM_CLIMB_002");
  check(rates && setup && control, "installed single-hatch control loads");
  if (!rates || !setup || !control) return;
  sim::World world(*setup), idle(*setup);
  check(control->current() == 20 && control->minimum() == 1 && control->maximum() == 50,
        "Just Climb retains installed rate and bounds");
  check(!control->press(std::nullopt, 3, world) && !control->press(1, 3, world) &&
        !control->press(0, 2, world), "missing hatch, invalid index and middle button are inert");
  world.advanceElapsedMs(1000); idle.advanceElapsedMs(1000);
  control->press(0, 3, world); control->release(3);
  check(control->current() == 21 && world.setup().releaseIntervalSeconds == rates->secondsPerLemming(21),
        "right click raises the existing interval-table index");
  for (int i = 0; i < 1500 && !idle.spawned(); ++i) {
    world.advanceElapsedMs(1); idle.advanceElapsedMs(1);
    check(world.spawned() == idle.spawned(), "rate change preserves in-flight first countdown");
  }
  check(idle.spawned() == 1, "first countdown completes");
  control->press(0, 1, world); control->release(1);
  check(control->current() == 20, "left click lowers one index");
  control->press(0, 3, world); control->advance(0.399, 0, world);
  check(control->current() == 21, "hold starts with one click");
  control->advance(0.001, 0, world);
  check(control->current() == 22, "repeat starts at 400ms");
  control->advance(10, std::nullopt, world); control->advance(10, 0, world);
  check(control->current() == 22 && !control->holding(), "leaving hatch cancels hold permanently");
  control->press(0, 3, world); control->advance(10, 0, world); control->reset();
  check(control->current() == 50, "upper authored limit");
  control->press(0, 1, world); control->advance(10, 0, world); control->reset();
  check(control->current() == 1, "lower authored limit");

  for (const auto* level : {"M06_KF_1", "E10_KF_1"}) {
    auto multi = app::RuntimeReleaseRateControl::create(lib, level);
    const auto bytes = lib.read(std::string("COMPILED\\") + level + ".LVL");
    const auto multiSetup = sim::loadRetailLevel(lib, level);
    check(multi && bytes && multiSetup, "installed multi-hatch inputs load");
    if (!multi || !bytes || !multiSetup) continue;
    const auto records = readHatchRecords(*bytes);
    check(multi->hatchCount() == records.size() && records.size() >= 2, "all hatch limits loaded");
    sim::World shared(*multiSetup);
    for (std::size_t h = 0; h < records.size(); ++h) {
      check(multi->minimum(h) == records[h].rateMin && multi->maximum(h) == records[h].rateMax,
            "each hatch has its own authored limits");
      const int expected = std::clamp(multi->current()+1, records[h].rateMin, records[h].rateMax);
      check(multi->press(h, 3, shared), "every hatch accepts right input");
      check(multi->current() == expected && shared.setup().releaseIntervalSeconds == rates->secondsPerLemming(expected),
            "clicked hatch clamps the shared rate");
      multi->advance(10, (h+1)%records.size(), shared);
      check(!multi->holding(), "moving between hatches cancels the first hold");
      multi->press(h, 1, shared); multi->release(1);
      check(multi->current() == std::clamp(expected-1, records[h].rateMin, records[h].rateMax),
            "every hatch accepts left input within its limits");
      std::cout << level << " hatch=" << h << " limits=" << records[h].rateMin << ".."
                << records[h].rateMax << " shared=" << multi->current() << '\n';
    }
  }

  // E10's secondary hatch has a different fixed rate. This catches accidentally
  // validating new recordings against primary limits; index zero also exercises
  // the old panel-recording layout without rewriting a historical replay.
  for (std::size_t h : {0u, 1u}) {
    auto fixed = app::RuntimeReleaseRateControl::create(lib, "E10_KF_1");
    const auto fixedSetup = sim::loadRetailLevel(lib, "E10_KF_1");
    if (!fixed || !fixedSetup) { check(false, "replay unit setup"); return; }
    sim::World replayWorld(*fixedSetup);
    app::RuntimeSimState runtime;
    app::RuntimeSkillSelection selection;
    auto recorder = app::ReplayRecorder::create(directory, "E10_KF_1", false, 0, fixed->current());
    if (!recorder) { check(false, "replay unit writer"); return; }
    fixed->press(h, 3, replayWorld); fixed->release(3);
    recorder->commands(replayWorld, runtime, selection, fixed->current(), fixed->rateHatch());
    const auto file = recorder->finish(replayWorld, runtime, "rate-codec-unit");
    check(bool(file), "rate unit recording closes");
    if (file) check(app::replayLevel(lib, *file) == 0, "v1 primary/secondary rate records replay");
  }
}
}
int main(int argc, char** argv) {
  const auto game = std::getenv("WEBOLUTION_GAME_DIR");
  if (!game || argc != 2) return 2;
  const auto library = AssetLibrary::open(std::filesystem::path(game)/"lemmings.box",
                                         std::filesystem::path(game)/"patch.box");
  if (!library) return 2;
  units(*library, argv[1]);
  return failures ? 1 : 0;
}
