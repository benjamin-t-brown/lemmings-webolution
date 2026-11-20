// Check that each save event reaches the mixer and overlapping exit cheers stay within range.
#include "audio/CuePlayback.h"
#include "audio/OverlapMix.h"
#include "sim/Terrain.h"
#include "sim/World.h"

#include <array>
#include <cstdlib>
#include <iostream>
#include <string_view>
#include <vector>

using namespace webo;

namespace {
int failures = 0;
void check(bool passed, const char* message) {
  if (!passed) {
    ++failures;
    std::cerr << "FAIL: " << message << '\n';
  }
}

void saveEvents(bool drainEachTick) {
  sim::LevelSetup s;
  s.terrain = sim::Terrain(64, 32);
  s.terrain.fillBox(0, 24, 64, 8, sim::Material::Earth);
  s.entrances.push_back({8, 23});
  s.exits.push_back({16, 20, 2, 3});
  s.totalLemmings = s.saveTarget = 4;
  s.releaseIntervalTicks = 1;
  sim::World world(s);
  audio::CuePlayback playback;  // Never start SDL or open a device.
  std::vector<std::string_view> heard;
  playback.setObserver([&](std::string_view cue) { heard.push_back(cue); });
  for (int tick = 0; tick < 100; ++tick) {
    world.tick();
    if (drainEachTick) {
      playback.drain(world);
      check(heard.size() == static_cast<std::size_t>(world.saved()),
            "each save is heard in the update that counts it, without repeats");
    }
  }
  playback.drain(world);
  check(world.saved() == 4 && world.won(), "all four synthetic walkers reach the exit");
  check(heard.size() == 4, "batched saves are not coalesced into one cheer");
  for (const auto cue : heard)
    check(cue == "Yippee", "a saved lemming requests the literal Yippee sample");
  playback.drain(world);
  check(heard.size() == 4, "a second drain cannot replay the same save");
  check(!playback.deviceReady(), "save routing requires no audio device");
}

void overlapHeadroom() {
  // Same-phase voices maximize additive clipping. Averaging preserves the sample peak within S16 limits.
  for (int count : {1, 2, 5, 32, 100}) {
    audio::OverlapMix mix;
    const std::array<std::int16_t, 2> source{28000, -28000};
    for (int n = 0; n < count; ++n) mix.add(source, 1.0f);
    std::array<std::int16_t, 2> out{};
    mix.mixInto(out);
    check(out == source, "overlap cannot clip or amplify an isolated exit bus");
  }
  {
    audio::OverlapMix mix;
    const std::array<std::int16_t, 1> positive{30000}, negative{-30000};
    mix.add(positive, 1.0f);
    mix.add(negative, 1.0f);
    std::array<std::int16_t, 1> out{};
    mix.mixInto(out);
    check(out[0] == 0, "opposite phases cancel before any integer clamping");
  }
  {
    audio::OverlapMix mix;
    const std::array<std::int16_t, 2> source{30000, -30000};
    mix.add(source, 1.0f);
    std::array<std::int16_t, 2> out{32000, -32000};
    mix.mixInto(out);
    check(out[0] == 32767 && out[1] == -32767,
          "only the exit bus is attenuated to available stereo headroom");
  }
  {
    audio::OverlapMix mix;
    const std::array<std::int16_t, 2> source{32767, -32768};
    mix.add(source, 0.5f);
    std::array<std::int16_t, 2> out{};
    mix.mixInto(out);
    check(out[0] == 16383 && out[1] == -16384, "voice volume remains effective");
  }
  {
    audio::OverlapMix mix;
    std::array<std::int16_t, 2> out{1234, -5678};
    mix.mixInto(out);
    check(out[0] == 1234 && out[1] == -5678, "no exit voices leaves other audio untouched");
  }
}
}  // namespace

int main() {
  saveEvents(true);
  saveEvents(false);
  overlapHeadroom();
  std::cout << "exit sound: " << failures << " failures\n";
  return failures == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
