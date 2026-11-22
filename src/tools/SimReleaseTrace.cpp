// Trace webo::sim::World on a retail level loaded by webo::sim::loadRetailLevelReport.
// SimReleaseTrace <game-dir> <level> <ticks> <releaseIntervalTicks>
//     [totalLemmings] [--real-seconds <armSeconds> <firstCountdownSeconds> <intervalSeconds>]
// Supply the release interval in 200 ms ticks. LevelInfo does not select a RELEASERATE.PC row.
// totalLemmings overrides the hatch count at hatch+0x98.
// --real-seconds sets the arm delay and first countdown. A positive interval overrides
// the recurring cadence on single-hatch levels. Both modes require releaseIntervalTicks.
// --continuous-stepper-clock sets LevelSetup::continuousStepperClock independently of hatch timing.
// World::tick() releases State::Faller actors via World::spawn() using the hatch's facing.
// Output CSV: tick,lemming_index,x,y,facing,state,spawned,saved,dead. spawned is the RELEASED count.
// Tick 0 is the initial state. Later rows follow one World::tick() each; lemming_index is the spawn-order vector index.

#include "assets/Box.h"
#include "sim/RetailLevel.h"
#include "sim/World.h"

#include <cstdlib>
#include <filesystem>
#include <format>
#include <iostream>
#include <string>
#include <vector>

namespace {

int usage() {
  std::cerr << "usage: SimReleaseTrace <game-dir> <level> <ticks> "
               "<releaseIntervalTicks> [totalLemmings] "
               "[--real-seconds <armSeconds> <firstCountdownSeconds> <intervalSeconds>] "
               "[--continuous-stepper-clock]\n";
  return 2;
}

}  // namespace

int main(int argc, char** argv) {
  if (argc < 5)
    return usage();

  const std::filesystem::path gameDir = argv[1];
  const std::string levelName = argv[2];
  const int ticks = std::atoi(argv[3]);
  const int releaseIntervalTicks = std::atoi(argv[4]);

  int totalLemmingsOverride = -1;
  double realSecondsArm = 0.0, realSecondsFirst = 0.0, realSecondsInterval = 0.0;
  bool useRealSeconds = false;
  bool useContinuousStepperClock = false;

  // Remove "--continuous-stepper-clock" from the argument tail before parsing positionals and "--real-seconds".
  std::vector<std::string> tail;
  for (int i = 5; i < argc; ++i) {
    if (std::string(argv[i]) == "--continuous-stepper-clock")
      useContinuousStepperClock = true;
    else
      tail.push_back(argv[i]);
  }

  std::size_t argi = 0;
  if (argi < tail.size() && tail[argi] != "--real-seconds") {
    totalLemmingsOverride = std::atoi(tail[argi].c_str());
    ++argi;
  }
  if (argi < tail.size() && tail[argi] == "--real-seconds") {
    if (argi + 3 >= tail.size())
      return usage();
    realSecondsArm = std::atof(tail[argi + 1].c_str());
    realSecondsFirst = std::atof(tail[argi + 2].c_str());
    realSecondsInterval = std::atof(tail[argi + 3].c_str());
    useRealSeconds = true;
    argi += 4;
  }

  if (ticks <= 0 || releaseIntervalTicks <= 0) {
    std::cerr << "error: ticks and releaseIntervalTicks must be positive\n";
    return usage();
  }

  auto lib = webo::AssetLibrary::open(gameDir / "lemmings.box", gameDir / "patch.box");
  if (!lib) {
    std::cerr << "error: " << lib.error() << "\n";
    return 1;
  }

  // Apply CLI release overrides after loading the level.
  auto report = webo::sim::loadRetailLevelReport(*lib, levelName);
  if (!report) {
    std::cerr << "error: " << report.error() << "\n";
    return 1;
  }
  if (report->hatches.size() > 1) {
    std::cerr << "warning: " << levelName << ": " << report->hatches.size()
              << " hatch records; elapsed timing uses each hatch's loaded interval."
                 " The default trace population comes from the first hatch.\n";
  }
  const auto& h = report->hatches.front();

  using namespace webo::sim;

  LevelSetup setup = std::move(report->setup);
  const Entrance primaryEntrance = setup.entrances.front();
  setup.totalLemmings = totalLemmingsOverride >= 0 ? totalLemmingsOverride : h.count;
  setup.releaseIntervalTicks = releaseIntervalTicks;
  if (useRealSeconds) {
    setup.releaseArmSeconds = realSecondsArm;
    setup.releaseFirstCountdownSeconds = realSecondsFirst;
    setup.releaseIntervalSeconds = realSecondsInterval;
    if (setup.entrances.size() == 1) {
      setup.entrances.front().releaseIntervalSeconds = realSecondsInterval;
    }
  }
  setup.continuousStepperClock = useContinuousStepperClock;

  std::cerr << std::format(
      "{}: hatch words x {} y {} count {} type {} -> cell ({}, {})\n", levelName, h.x,
      h.y, h.count, h.type, primaryEntrance.x, primaryEntrance.y);
  std::cerr << std::format(
      "totalLemmings {}  releaseIntervalTicks {}  ticks {}\n", setup.totalLemmings,
      setup.releaseIntervalTicks, ticks);
  if (useRealSeconds) {
    std::cerr << std::format(
        "real-seconds model: armSeconds {}  firstCountdownSeconds {}  intervalSeconds {}\n",
        setup.releaseArmSeconds, setup.releaseFirstCountdownSeconds,
        setup.releaseIntervalSeconds);
  }
  if (useContinuousStepperClock) {
    std::cerr << "D1n: continuous sub-tick stepper clock ON "
                 "(LevelSetup::continuousStepperClock)\n";
  }

  World w(setup);

  std::cout << "tick,lemming_index,x,y,facing,state,spawned,saved,dead\n";
  // Tick 0 marks the initial state before any spawn.
  std::cout << std::format("0,,,,,,{},{},{}\n", w.spawned(), w.saved(), w.dead());

  for (int t = 1; t <= ticks; ++t) {
    w.tick();
    const auto& lemmings = w.lemmings();
    if (lemmings.empty()) {
      // Emit a marker with blank lemming_index for ticks when the hatch has released nobody yet.
      std::cout << std::format("{},,,,,,{},{},{}\n", t, w.spawned(), w.saved(), w.dead());
      continue;
    }
    for (std::size_t i = 0; i < lemmings.size(); ++i) {
      const Lemming& l = lemmings[i];
      std::cout << std::format("{},{},{},{},{},{},{},{},{}\n", t, i, l.x, l.y,
                               static_cast<int>(l.facing), stateName(l.state), w.spawned(),
                               w.saved(), w.dead());
    }
  }

  return 0;
}
