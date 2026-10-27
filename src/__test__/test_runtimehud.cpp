#include "app/RuntimeHud.h"

#include <iostream>

int main() {
  using namespace webo;
  using namespace webo::app;
  sim::LevelSetup setup;
  setup.timeLimitTicks = 10;
  sim::World world(setup);
  RuntimeSimState state;
  auto initial = runtimeHudValues(world);
  if (!state.advance(world, 0.2)) {
    std::cerr << "running interval did not tick World\n";
    return 1;
  }
  auto running = runtimeHudValues(world);
  if (initial.secondsRemaining != 2 || running.secondsRemaining != 1) {
    std::cerr << "running HUD did not advance with World\n";
    return 1;
  }
  std::cout << "run: ticks-left=" << world.timeRemainingTicks()
            << " hud-seconds=" << running.secondsRemaining << '\n';
  if (state.advance(world, 0.1)) return 1; // partial pre-pause time
  state.togglePause();                    // production P transition
  const auto pausedValues = runtimeHudValues(world);
  if (!state.paused() || state.advance(world, 10.0)) {
    std::cerr << "pause gate advanced World\n";
    return 1;
  }
  if (runtimeHudValues(world).secondsRemaining != pausedValues.secondsRemaining) {
    std::cerr << "HUD changed during elapsed paused time\n";
    return 1;
  }
  std::cout << "pause+10s: ticks-left=" << world.timeRemainingTicks()
            << " hud-seconds=" << runtimeHudValues(world).secondsRemaining << '\n';
  state.togglePause();                    // production P resume transition
  const auto frozenTicks = world.timeRemainingTicks();
  if (state.paused() || state.advance(world, 0.1)) {
    std::cerr << "resume caught up paused/pre-pause accumulator\n";
    return 1;
  }
  if (!state.advance(world, 0.11) || world.timeRemainingTicks() != frozenTicks - 1 ||
      runtimeHudValues(world).secondsRemaining != pausedValues.secondsRemaining) {
    std::cerr << "resumed simulation did not advance from frozen World state\n";
    return 1;
  }
  std::cout << "resume: ticks-left=" << world.timeRemainingTicks()
            << " hud-seconds=" << runtimeHudValues(world).secondsRemaining << '\n';
  Image font;
  font.width = font.height = 128;
  font.rgba.resize(128u * 128u * 4u, 255);
  Image before; before.width = 640; before.height = 480;
  before.rgba.resize(640u * 480u * 4u, 0);
  Image paused = before;
  drawRuntimeHud(before, font, world, false);
  drawRuntimeHud(paused, font, world, true);
  if (before.rgba == paused.rgba) {
    std::cerr << "paused composition produced no caption pixels\n";
    return 1;
  }
  std::cout << "runtime HUD values and paused caption composition pass\n";
  return 0;
}
