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

bool RuntimeSimState::advance(sim::World& world, double elapsedSeconds, bool trackCompletion) {
  if (paused_) return false;
  clock_.advance(elapsedSeconds);
  bool ticked = false;
  while ((!trackCompletion || end_.phase() != LevelEndPhase::Results) && clock_.takeStep()) {
    // Native hatches continue while timeout-pending (425A95). Default World
    // fixtures keep their old timeout-release policy; connected play opts in.
    world.tick(trackCompletion);
    ticked = true;
    if (trackCompletion) {
      int retainedNonRat = 0;
      for (const auto& lemming : world.lemmings())
        if (lemming.alive() && lemming.sheetType != 3) ++retainedNonRat;
      // DERIVED ordinary-population adapter. Native5098 is selected-type
      // unreleased hatch population, not an active-actor count. Nuke and
      // selected-type/preplaced population generalization remain separate.
      end_.afterSimulationSlice(1.0 / world.rules().ticksPerSecond,
          {world.timedOut(), retainedNonRat, world.setup().totalLemmings - world.spawned(),
           world.saved(), world.setup().saveTarget});
    }
  }
  return ticked;
}

}  // namespace webo::app
