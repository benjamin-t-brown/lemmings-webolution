#include "app/RuntimeHud.h"

#include <iostream>

int main() {
  webo::app::RuntimeSimState state;
  webo::sim::World world(webo::sim::LevelSetup{});
  state.advanceElapsed(world, 0.0005);
  const auto updates = world.explicitUpdateCount();
  state.toggleExaminePause();
  if (!state.paused() || !state.examining() || state.carriedMs() != 0 ||
      state.advanceElapsed(world, 2.0) || state.advance(world, 2.0) ||
      world.explicitUpdateCount() != updates || world.tickCount() != 0) {
    std::cerr << "Examine pause did not discard time and freeze both World update paths\n";
    return 1;
  }
  state.toggleExaminePause();
  if (state.paused() || state.examining()) {
    std::cerr << "Examine toggle did not resume play\n";
    return 1;
  }

  state.togglePause();
  if (!state.paused() || state.examining()) {
    std::cerr << "Standard toggle did not enter standard pause\n";
    return 1;
  }
  state.toggleExaminePause();
  if (!state.paused() || !state.examining()) {
    std::cerr << "Standard pause did not switch to examine pause\n";
    return 1;
  }
  state.togglePause();
  if (state.paused() || state.examining()) {
    std::cerr << "Standard toggle from examine did not resume play\n";
    return 1;
  }
  state.toggleExaminePause();
  state.resume();
  if (state.paused() || state.examining() || !state.advanceElapsed(world, 0.001)) {
    std::cerr << "Explicit resume from examine did not restart World updates\n";
    return 1;
  }
}
