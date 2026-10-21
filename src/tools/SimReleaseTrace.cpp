// SimReleaseTrace -- offline tick-by-tick trace of webo::sim::World on a
// retail level, for the D1a differential check against a live capture of
// the original (see tools/harness/measure_d1_release_trace.py and
// build/tmp/d1a-release-walkout-trace/diff_trace.py).
//
// D1a (PLAN.md "Phase D -- reconnect the simulation", D1 release and
// walk-out). Read-only use of the existing sim/assets API: this builds the
// same LevelSetup src/__test__/test_world.cpp::testRetailSpawn already
// builds for MM_CLIMB_002 (terrain from the stored collision grid, one
// Entrance from the hatch's own stored words via webo::hatchSpawnCell), and
// runs World::tick() for N ticks instead of one, printing one CSV row per
// lemming per tick. src/sim/World.* and src/assets/** are not modified.
//
// A default-constructed Lemming spawned by World::spawn() starts
// Facing::Right, State::Faller (World.h ~line 76-79) -- this tool never
// hand-places a Lemming the way the retired old/src/tools/compare_tick.cpp
// did; World::tick()'s own spawn() releases from the Entrance.
//
//   SimReleaseTrace <game-dir> <level> <ticks> <releaseIntervalTicks> [totalLemmings]
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
#include "assets/CollisionGrid.h"
#include "assets/LevelIndex.h"
#include "assets/LevelObjects.h"
#include "sim/World.h"

#include <cstdlib>
#include <filesystem>
#include <format>
#include <iostream>
#include <string>

namespace {

int usage() {
  std::cerr << "usage: SimReleaseTrace <game-dir> <level> <ticks> "
               "<releaseIntervalTicks> [totalLemmings]\n";
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
  const int totalLemmingsOverride = argc > 5 ? std::atoi(argv[5]) : -1;

  if (ticks <= 0 || releaseIntervalTicks <= 0) {
    std::cerr << "error: ticks and releaseIntervalTicks must be positive\n";
    return usage();
  }

  auto lib = webo::AssetLibrary::open(gameDir / "lemmings.box", gameDir / "patch.box");
  if (!lib) {
    std::cerr << "error: " << lib.error() << "\n";
    return 1;
  }

  std::string file;
  for (const auto& name : webo::levelFileNames(*lib))
    if (name.find(levelName) != std::string::npos) {
      file = name;
      break;
    }
  if (file.empty()) {
    std::cerr << "error: " << levelName << " is not in the archive\n";
    return 1;
  }

  auto blob = lib->read(file);
  if (!blob) {
    std::cerr << "error: " << blob.error() << "\n";
    return 1;
  }

  auto objs = webo::findLevelObjects(*blob);
  auto grid = webo::findCollisionGrid(*blob);
  if (!objs || !grid) {
    std::cerr << "error: " << levelName << ": no objects or no grid\n";
    return 1;
  }
  if (objs->hatches.empty()) {
    std::cerr << "error: " << levelName << ": no hatch record\n";
    return 1;
  }
  if (objs->hatches.size() > 1) {
    std::cerr << "warning: " << levelName << ": " << objs->hatches.size()
              << " hatch records; using the first (this brief's scope is"
                 " MM_CLIMB_002, which has exactly one)\n";
  }
  const auto& h = objs->hatches.front();

  using namespace webo::sim;

  const auto [cx, cy] = webo::hatchSpawnCell(h, grid->width, grid->height);

  LevelSetup setup;
  setup.terrain = Terrain::fromCollisionGrid(*grid);
  Entrance e;
  e.x = cx;
  e.y = cy;
  e.source = Entrance::Source::HatchWords;
  e.sheetType = h.type;
  setup.entrances.push_back(e);
  setup.totalLemmings = totalLemmingsOverride >= 0 ? totalLemmingsOverride : h.count;
  setup.releaseIntervalTicks = releaseIntervalTicks;

  std::cerr << std::format(
      "{}: hatch words x {} y {} count {} type {} -> cell ({}, {})\n", levelName, h.x,
      h.y, h.count, h.type, cx, cy);
  std::cerr << std::format(
      "totalLemmings {}  releaseIntervalTicks {}  ticks {}\n", setup.totalLemmings,
      setup.releaseIntervalTicks, ticks);

  World w(setup);

  std::cout << "tick,lemming_index,x,y,facing,state,spawned,saved,dead\n";
  // Tick 0 marker: no lemmings yet (spawn() has not run).
  std::cout << std::format("0,,,,,,{},{},{}\n", w.spawned(), w.saved(), w.dead());

  for (int t = 1; t <= ticks; ++t) {
    w.tick();
    const auto& lemmings = w.lemmings();
    for (std::size_t i = 0; i < lemmings.size(); ++i) {
      const Lemming& l = lemmings[i];
      std::cout << std::format("{},{},{},{},{},{},{},{},{}\n", t, i, l.x, l.y,
                               static_cast<int>(l.facing), stateName(l.state), w.spawned(),
                               w.saved(), w.dead());
    }
  }

  return 0;
}
