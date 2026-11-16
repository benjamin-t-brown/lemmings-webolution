// Small synthetic units for playtest 9 Y3/Y5. No installed route, save or device.
#include "app/RuntimeHud.h"
#include "audio/CuePlayback.h"
#include "sim/World.h"

#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <vector>

using namespace webo;
namespace {
int failures = 0;
void check(bool passed, const char* message) {
  if (!passed) { ++failures; std::cerr << "FAIL: " << message << '\n'; }
}

sim::LevelSetup floor() {
  sim::LevelSetup setup;
  setup.terrain = sim::Terrain(128, 96);
  setup.terrain.fillBox(0, 80, 128, 16, sim::Material::Earth);
  setup.totalLemmings = 0;
  setup.skills.fill(-1);
  return setup;
}

void builder(bool fastForward, bool buffered) {
  sim::World world(floor());
  sim::Lemming actor;
  actor.x = 16; actor.y = 79; actor.state = sim::State::Walker;
  world.place(actor);
  check(world.assign(0, sim::Skill::Builder), "builder assignment succeeds");
  world.takeCues(); // assignment stays silent, distinct from last-brick Chink
  app::RuntimeSimState runtime;
  if (fastForward) runtime.latchFastForward();
  audio::CuePlayback playback;
  int warnings = 0;
  playback.setObserver([&](std::string_view name) {
    check(name == "Chink", "builder uses Chink only");
    ++warnings;
  });
  std::vector<int> counts;
  int previous = 12;
  const auto afterUpdate = [&] {
    const int remaining = world.lemmings()[0].bricksLeft;
    if (remaining != previous) {
      const auto cues = buffered ? std::vector<sim::SoundCue>{} : world.takeCues();
      if (previous <= 3) counts.push_back(previous);
      if (!buffered) {
        check(cues.size() == (previous <= 3 ? 1u : 0u),
              "exactly one warning for each of bricks 3, 2, 1, none earlier");
        for (auto cue : cues) playback.play(cue);
      }
      previous = remaining;
    } else if (!buffered) {
      check(world.takeCues().empty(), "no sound between brick events");
    }
  };
  runtime.togglePause();
  const auto clock = world.levelClockMs();
  check(!runtime.advanceElapsed(world, 4.0, false, afterUpdate), "pause skips skill updates");
  check(world.levelClockMs() == clock && world.takeCues().empty(), "pause creates no queued sounds");
  runtime.resume();
  for (int frame = 0; frame < 1000 && previous > 0; ++frame)
    runtime.advanceElapsed(world, 0.02, false, afterUpdate);
  playback.drain(world);
  check(counts == std::vector<int>{3, 2, 1}, "all last-three brick events survive fast-forward/batching");
  check(warnings == 3, "each final brick requests one warning");
  playback.drain(world);
  check(warnings == 3, "draining twice cannot replay warnings");
}

void basher(bool solid, bool steel) {
  auto setup = floor();
  if (solid) setup.terrain.fillBox(17, 72, 18, 8,
      steel ? sim::Material::Steel : sim::Material::Earth);
  sim::World world(setup);
  sim::Lemming actor;
  actor.x = 16; actor.y = 79; actor.state = sim::State::Walker;
  world.place(actor);
  check(world.assign(0, sim::Skill::Basher), "basher assignment succeeds");
  world.takeCues();
  int sounds = 0;
  for (int tick = 0; tick < 100; ++tick) {
    world.tick();
    const auto cues = world.takeCues();
    for (const auto cue : cues) {
      check(cue == sim::SoundCue::Bash, "bashing emits only its own sound");
      check(audio::routedCueName(cue) == "Mine", "bashing uses the evidenced Mine sample");
      ++sounds;
    }
    check(cues.size() <= 1, "one bite cannot emit a sound per cleared cell");
    if (world.lemmings()[0].state != sim::State::Basher) break;
  }
  check(solid && !steel ? sounds > 1 : sounds == 0,
        "successful bites repeat; empty swings and steel refusals make no Mine sound");
  check(world.takeCues().empty(), "no persistent or looping bash cue");
}
} // namespace

int main() {
  for (bool fastForward : {false, true})
    for (bool buffered : {false, true}) builder(fastForward, buffered);
  basher(true, false); basher(false, false); basher(true, true);
  if (failures) return EXIT_FAILURE;
  std::cout << "PASS: last-three warnings, successful bash bites, pause, fast-forward and drains\n";
}
