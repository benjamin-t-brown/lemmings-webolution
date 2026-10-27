#pragma once

#include "assets/Bitmap.h"
#include "shell/PauseCaption.h"
#include "sim/World.h"
#include "app/SimClock.h"

namespace webo::app {

// Production window composition: values are read from the same World that
// supplies the scene actors. The caller owns the cached font sheet.
shell::HudValues runtimeHudValues(const sim::World& world);
void drawRuntimeHud(webo::Image& frame, const webo::Image& font,
                    const sim::World& world, bool paused);

// Shared production state gate used by the SDL loop and the deterministic
// offline integration test. Pausing discards the accumulator and elapsed
// samples while paused; resuming begins a fresh SimClock interval.
class RuntimeSimState {
 public:
  bool paused() const { return paused_; }
  void togglePause();
  void resume();
  bool advance(sim::World& world, double elapsedSeconds);

 private:
  SimClock clock_;
  bool paused_ = false;
};

}  // namespace webo::app
