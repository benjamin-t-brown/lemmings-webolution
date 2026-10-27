#include "app/RuntimeHud.h"

#include "shell/HudText.h"

namespace webo::app {

shell::HudValues runtimeHudValues(const sim::World& world) {
  shell::HudValues values;
  values.out = world.spawned() - world.saved() - world.dead();
  values.saved = world.saved();
  values.saveTarget = world.setup().saveTarget;
  values.secondsRemaining = static_cast<int>(
      world.timeRemainingTicks() / world.rules().ticksPerSecond);
  return values;
}

void drawRuntimeHud(webo::Image& frame, const webo::Image& font,
                    const sim::World& world, bool paused) {
  const auto values = runtimeHudValues(world);
  const auto cells = shell::fontCells();
  const auto lines = shell::layoutHud(values, {}, cells);
  shell::drawHud(frame, font, lines, cells);
  if (paused) shell::drawCaption(frame, font, cells);
}

void RuntimeSimState::togglePause() {
  paused_ = !paused_;
  clock_ = SimClock{};
}

void RuntimeSimState::resume() {
  if (paused_) {
    paused_ = false;
    clock_ = SimClock{};
  }
}

bool RuntimeSimState::advance(sim::World& world, double elapsedSeconds) {
  if (paused_) return false;
  clock_.advance(elapsedSeconds);
  bool ticked = false;
  while (clock_.takeStep()) {
    world.tick();
    ticked = true;
  }
  return ticked;
}

}  // namespace webo::app
