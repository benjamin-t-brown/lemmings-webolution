// Check clicked hatch limits and the v1 recording's hatch selector.
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
  const auto current = [&] { return world.setup().entrances[0].releaseRate; };
  check(current() == 20 && control->minimum() == 1 && control->maximum() == 50,
        "Just Climb retains installed rate and bounds");
  check(!control->press(std::nullopt, 3, world) && !control->press(1, 3, world) &&
        !control->press(0, 2, world), "missing hatch, invalid index and middle button are inert");
  world.advanceElapsedMs(1000); idle.advanceElapsedMs(1000);
  control->press(0, 3, world); control->release(3);
  check(current() == 21 && world.setup().releaseIntervalSeconds == rates->secondsPerLemming(21),
        "right click raises the existing interval-table index");
  for (int i = 0; i < 1500 && !idle.spawned(); ++i) {
    world.advanceElapsedMs(1); idle.advanceElapsedMs(1);
    check(world.spawned() == idle.spawned(), "rate change preserves in-flight first countdown");
  }
  check(idle.spawned() == 1, "first countdown completes");
  control->press(0, 1, world); control->release(1);
  check(current() == 20, "left click lowers one index");
  const app::HatchHoverTarget hatch[]{{0, 10, 10, 30, 30}};
  const bool actorCanReceiveSkill = world.canAssign(0, sim::Skill::Climber);
  check(actorCanReceiveSkill, "the released lemming can take the selected skill");
  check(!control->pressTarget(hatch, 20, 20, 1, actorCanReceiveSkill) &&
        control->pressTarget(hatch, 20, 20, 1, false) == 0,
        "left press gives way only to an assignable lemming");
  const auto rightTarget = control->pressTarget(hatch, 20, 20, 3, actorCanReceiveSkill);
  check(rightTarget && control->press(hatch[*rightTarget].index, 3, world),
        "right press can adjust the hatch while the lemming overlaps");
  control->advance(0.399, hatch, 20, 20, world);
  check(current() == 21, "hold starts with one click");
  control->advance(0.001, hatch, 20, 20, world);
  check(current() == 22 && control->heldTarget(hatch, 20, 20) == 0,
        "rate and held hover continue through the actor overlap");
  control->release(3);
  control->advance(10, hatch, 20, 20, world);
  check(current() == 22 && !control->holding() && !control->heldTarget(hatch, 20, 20),
        "button release ends the hold and its readout");
  const auto leftTarget = control->pressTarget(hatch, 20, 20, 1, false);
  check(leftTarget && control->press(hatch[*leftTarget].index, 1, world),
        "left hold starts before a lemming passes under the pointer");
  check(!control->pressTarget(hatch, 20, 20, 1, actorCanReceiveSkill),
        "the passing lemming would take a new left press");
  control->advance(0.4, hatch, 20, 20, world);
  check(current() == 20 && control->holding(),
        "the existing left hold repeats during that overlap");
  control->advance(0.1, hatch, 20, 20, world);
  check(current() == 19 && control->holding(), "repeat continues as the lemming passes");
  control->advance(0, hatch, 30, 20, world);
  control->advance(10, hatch, 20, 20, world);
  check(current() == 19 && !control->holding(), "leaving hatch cancels hold permanently");
  control->press(0, 3, world); control->advance(10, hatch, 20, 20, world); control->reset();
  check(current() == 50, "upper authored limit");
  control->press(0, 1, world); control->advance(10, hatch, 20, 20, world); control->reset();
  check(current() == 1, "lower authored limit");

  for (const auto* level : {"M06_KF_1", "E10_KF_1"}) {
    auto multi = app::RuntimeReleaseRateControl::create(lib, level);
    const auto bytes = lib.read(std::string("COMPILED\\") + level + ".LVL");
    const auto multiSetup = sim::loadRetailLevel(lib, level);
    check(multi && bytes && multiSetup, "installed multi-hatch inputs load");
    if (!multi || !bytes || !multiSetup) continue;
    const auto records = readHatchRecords(*bytes);
    check(multi->hatchCount() == records.size() && records.size() >= 2, "all hatch limits loaded");
    sim::World multiWorld(*multiSetup);
    const auto& entrances = multiWorld.setup().entrances;
    for (std::size_t h = 0; h < records.size(); ++h) {
      check(multi->minimum(h) == records[h].rateMin && multi->maximum(h) == records[h].rateMax,
            "each hatch has its own authored limits");
      const auto before = entrances;
      const int expected = std::clamp(before[h].releaseRate + 1, records[h].rateMin, records[h].rateMax);
      check(multi->press(h, 3, multiWorld), "every hatch accepts right input");
      check(entrances[h].releaseRate == expected &&
            entrances[h].releaseIntervalSeconds == rates->secondsPerLemming(expected),
            "clicked hatch updates its rate and interval");
      for (std::size_t other = 0; other < entrances.size(); ++other) {
        if (other != h)
          check(entrances[other].releaseRate == before[other].releaseRate &&
                entrances[other].releaseIntervalSeconds == before[other].releaseIntervalSeconds,
                "a click leaves every other hatch's rate and cadence alone");
      }
      const app::HatchHoverTarget overlapping[]{{(h+1)%records.size(), 10, 10, 30, 30},
                                                {h, 20, 10, 40, 30}};
      multi->advance(0, overlapping, 20, 20, multiWorld);
      check(multi->heldTarget(overlapping, 20, 20) == 1,
            "overlapping hatch geometry preserves the exact held hatch");
      multi->advance(10, overlapping, 19, 20, multiWorld);
      check(!multi->holding(), "moving between hatches cancels the first hold");
      multi->press(h, 1, multiWorld); multi->release(1);
      check(entrances[h].releaseRate == std::clamp(expected - 1, records[h].rateMin, records[h].rateMax),
            "every hatch accepts left input within its limits");
      std::cout << level << " hatch=" << h << " limits=" << records[h].rateMin << ".."
                << records[h].rateMax << " rate=" << entrances[h].releaseRate << '\n';
    }
  }

  // Both hatches reach rate 31. Equal rate values still need two v1 commands.
  auto adjustable = app::RuntimeReleaseRateControl::create(lib, "M06_KF_1");
  auto replaySetup = sim::loadRetailLevel(lib, "M06_KF_1");
  if (!adjustable || !replaySetup) { check(false, "replay unit setup"); return; }
  sim::World replayWorld(std::move(*replaySetup));
  app::RuntimeSimState runtime;
  app::RuntimeSkillSelection selection;
  auto recorder = app::ReplayRecorder::create(directory, "M06_KF_1", false, 0, replayWorld);
  if (!recorder) { check(false, "replay unit writer"); return; }
  for (std::size_t h : {1u, 0u}) {
    adjustable->press(h, 3, replayWorld); adjustable->release(3);
    recorder->commands(replayWorld, runtime, selection);
  }
  const auto file = recorder->finish(replayWorld, runtime, "rate-codec-unit");
  check(bool(file), "rate unit recording closes");
  if (!file) return;
  std::ifstream input(*file, std::ios::binary);
  std::string line;
  int nextHatch = 1;
  while (std::getline(input, line)) {
    if (!line.starts_with("r ")) continue;
    std::istringstream row(line);
    char kind;
    int step, rate, ticks, hatch;
    check(bool(row >> kind >> step >> rate >> ticks >> hatch) && step == 0 &&
          rate == 31 && ticks == rates->intervalTicks(31, 5) && hatch == nextHatch,
          "v1 records each changed hatch in input order");
    --nextHatch;
  }
  check(nextHatch == -1, "v1 writes both commands");
  check(app::replayLevel(lib, *file) == 0, "v1 primary/secondary rate records replay");
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
