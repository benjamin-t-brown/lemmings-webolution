// SimReleaseTrace -- offline tick-by-tick trace of webo::sim::World on a
// retail level, for the D1a differential check against a live capture of
// the original (see tools/harness/measure_d1_release_trace.py and
// build/tmp/d1a-release-walkout-trace/diff_trace.py).
//
// D1a (PLAN.md "Phase D -- reconnect the simulation", D1 release and
// walk-out). D3a (round 1) rebuilt this tool's LevelSetup on the new
// production loader, webo::sim::loadRetailLevelReport (sim/RetailLevel.h),
// instead of hand-building the minimal entrance-only setup
// src/__test__/test_world.cpp::testRetailSpawn also used to build. The CLI
// is unchanged -- every argument here is still read and still applied on
// top of the loader's own result, in the same order, so a caller cannot
// tell the difference, and D3a's own round-note shows a byte-identical
// re-run of D1g's exact invocation as the proof. src/sim/World.* is not
// modified.
//
// A default-constructed Lemming spawned by World::spawn() starts
// Facing::Right, State::Faller (World.h ~line 76-79) -- this tool never
// hand-places a Lemming the way the retired old/src/tools/compare_tick.cpp
// did; World::tick()'s own spawn() releases from the Entrance.
//
//   SimReleaseTrace <game-dir> <level> <ticks> <releaseIntervalTicks>
//       [totalLemmings] [--real-seconds <armSeconds> <firstCountdownSeconds> <intervalSeconds>]
//
// <releaseIntervalTicks> is NOT guessed here: LevelInfo carries no
// release-rate index field (see the brief's "Already settled" -- no code
// anywhere resolves which RELEASERATE.PC row applies to a given level), so
// the caller supplies the value tools/harness/measure_d1_release_trace.py
// measured live (RELEASED counter against the wall clock, converted to
// ticks at the measured 200 ms / 5 Hz tick). [totalLemmings], if given,
// overrides the hatch's own release count word (hatch+0x98); otherwise that
// word is used, matching testRetailSpawn.
//
// D1e extension: the optional trailing "--real-seconds <arm> <first>
// <interval>" flag sets LevelSetup::releaseArmSeconds /
// releaseFirstCountdownSeconds / releaseIntervalSeconds, which switches
// World::tick() over to the new real-seconds accumulator model
// (docs/scratch/phase-d1-release-trigger.md and the D1c/D1d follow-ups)
// instead of the integer-tick model <releaseIntervalTicks> alone drives. The
// positional <releaseIntervalTicks> argument is still required (and still
// read by LevelSetup) even when this flag is given, since it is otherwise
// unused by the new model and the old CLI shape is not being redesigned.
//
// D1n extension: an independent, optional "--continuous-stepper-clock"
// switch (no argument, may appear anywhere after the required four
// positionals, in any order relative to "--real-seconds") sets
// LevelSetup::continuousStepperClock, the opt-in fall/walk sub-tick clock
// (docs/scratch/phase-d1-stepper-clock.md). Omitting it, as every call site
// predating this round does, leaves World's fixed-count-per-tick
// stepFaller/stepWalker loops untouched.
//
// Output (stdout) is a CSV with a header row:
//   tick,lemming_index,x,y,facing,state,spawned,saved,dead
// tick 0 is the initial state, before any World::tick() call -- always no
// lemmings (spawn() has not run yet), emitted as a single marker row so the
// tick axis in this file starts at the same place the live capture's does.
// Ticks 1..N each follow exactly one World::tick() call. lemming_index is
// World::lemmings()'s own vector index, which is spawn order (spawn() only
// ever push_back()s -- see World::spawn(), World.cpp), the same identity
// the live capture uses (first-seen order in the actor list).

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

  // D1n: pull "--continuous-stepper-clock" out of the tail first, wherever
  // it appears, so the rest of the (unchanged) positional/"--real-seconds"
  // parsing below sees exactly the argument shape it always has.
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

  // D3a (round 1): the production loader replaces this tool's own
  // hand-built, entrance-only LevelSetup -- it now also carries the real
  // exits/teleports/speed pads/levers/skills/totals the brief's D3 gate
  // needs, none of which this tool used to set (every field it left
  // default was LevelSetup's own default, matching World.h). The CLI below
  // still overrides totalLemmings/releaseIntervalTicks/the real-seconds
  // triple exactly as before, in the same order, so this tool's own output
  // contract (and D1's byte-identical re-run of it) is unchanged.
  auto report = webo::sim::loadRetailLevelReport(*lib, levelName);
  if (!report) {
    std::cerr << "error: " << report.error() << "\n";
    return 1;
  }
  if (report->hatches.size() > 1) {
    std::cerr << "warning: " << levelName << ": " << report->hatches.size()
              << " hatch records; the loader used the first for release timing"
                 " (this tool traces one hatch's spawns; see RetailLevel.h)\n";
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
  // Tick 0 marker: no lemmings yet (spawn() has not run).
  std::cout << std::format("0,,,,,,{},{},{}\n", w.spawned(), w.saved(), w.dead());

  for (int t = 1; t <= ticks; ++t) {
    w.tick();
    const auto& lemmings = w.lemmings();
    if (lemmings.empty()) {
      // D1e: the real-seconds model can leave the hatch unarmed/counting down
      // for several ticks, so "no lemmings yet" is no longer tick 0 only --
      // emit the same marker row shape (blank lemming_index) so a reader
      // that pulls "spawned" per tick (diff_trace.py's load_sim) has a row
      // at every tick, matching the pre-existing tick-0 convention above.
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
