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
  sim::LevelSetup timeoutSetup;
  timeoutSetup.totalLemmings = 1; // No entrance: cannot finish by resolving actors.
  timeoutSetup.timeLimitTicks = 3;
  sim::World timeoutWorld(timeoutSetup);
  RuntimeSimState terminalClock;
  terminalClock.advance(timeoutWorld, 2.0, true);
  if (!timeoutWorld.timedOut() || timeoutWorld.finished() || timeoutWorld.tickCount() != 10 ||
      terminalClock.endTransition().phase() != LevelEndPhase::Pending) {
    std::cerr << "timeout did not enter pending while continuing simulation\n";
    return 1;
  }
  terminalClock.togglePause();
  const double pendingTimer = terminalClock.endTransition().timerSeconds();
  if (terminalClock.advance(timeoutWorld, 20.0, true) || timeoutWorld.tickCount() != 10 ||
      terminalClock.endTransition().timerSeconds() != pendingTimer) {
    std::cerr << "pause advanced pending simulation/countdown\n"; return 1;
  }
  terminalClock.resume();
  for (int i = 0; i < 5 && !terminalClock.endTransition().snapshot(); ++i)
    terminalClock.advance(timeoutWorld, 2.0, true);
  const auto terminalTick = timeoutWorld.tickCount();
  if (!terminalClock.endTransition().snapshot() || terminalTick <= 30 || terminalTick > 34 ||
      terminalClock.advance(timeoutWorld, 2.0, true) || timeoutWorld.tickCount() != terminalTick) {
    std::cerr << "result boundary failed to freeze after pending countdown\n"; return 1;
  }
  sim::LevelSetup resolvedSetup;
  resolvedSetup.totalLemmings = 0;
  sim::World resolvedWorld(resolvedSetup);
  RuntimeSimState resolvedClock;
  if (!resolvedClock.advance(resolvedWorld, 0.2, true) || resolvedWorld.tickCount() != 1 ||
      resolvedClock.endTransition().phase() != LevelEndPhase::Pending ||
      resolvedClock.endTransition().timerSeconds() != 7.0) {
    std::cerr << "resolved World did not enter pending without trigger subtraction\n";
    return 1;
  }
  sim::LevelSetup lateSetup;
  lateSetup.terrain = sim::Terrain(16, 12);
  for (int x = 0; x < 16; ++x) lateSetup.terrain.set(x, 6, sim::Material::Earth);
  lateSetup.entrances.push_back({1, 3}); // Faller starts with a real gap to the floor.
  lateSetup.exits.push_back({4, 5, 0, 0});
  lateSetup.totalLemmings = 1; lateSetup.saveTarget = 1; lateSetup.timeLimitTicks = 1;
  sim::World lateWorld(lateSetup);
  RuntimeSimState lateClock;
  lateClock.advance(lateWorld, 0.2, true);
  if (lateWorld.saved() != 0 || lateClock.endTransition().phase() != LevelEndPhase::Pending) {
    std::cerr << "late-rescue fixture did not latch timeout before rescue\n"; return 1;
  }
  for (int i = 0; i < 5 && !lateClock.endTransition().snapshot(); ++i)
    lateClock.advance(lateWorld, 2.0, true);
  if (!lateClock.endTransition().snapshot() || lateClock.endTransition().snapshot()->success ||
      lateClock.endTransition().snapshot()->saved != 1 || !lateWorld.won()) {
    std::cerr << "late rescue upgraded timeout loss or final counts were stale: tick=" << lateWorld.tickCount()
              << " spawned=" << lateWorld.spawned() << " saved=" << lateWorld.saved() << '\n';
    for (const auto& actor : lateWorld.lemmings())
      std::cerr << actor.x << ',' << actor.y << ' ' << sim::stateName(actor.state) << '\n';
    return 1;
  }
  lateSetup.exits.clear(); lateSetup.totalLemmings = 3; lateSetup.releaseIntervalTicks = 2;
  sim::World pendingHatches(lateSetup), legacyHatches(lateSetup);
  RuntimeSimState hatchClock;
  hatchClock.advance(pendingHatches, 2.0, true);
  for (int i = 0; i < 10; ++i) legacyHatches.tick();
  if (pendingHatches.spawned() != 3 || legacyHatches.spawned() != 1) {
    std::cerr << "pending hatches did not continue (or default diagnostic changed)\n"; return 1;
  }
  auto typeSetup = lateSetup;
  typeSetup.totalLemmings = 0; typeSetup.timeLimitTicks = 100; typeSetup.entrances.clear();
  sim::Lemming actor;
  actor.x = 1; actor.y = 5; actor.state = sim::State::Walker; actor.sheetType = 3;
  sim::World ratWorld(typeSetup);
  ratWorld.place(actor);
  RuntimeSimState ratClock;
  ratClock.advance(ratWorld, 0.2, true);
  if (!ratWorld.lemmings()[0].alive() || ratClock.endTransition().phase() != LevelEndPhase::Pending) {
    std::cerr << "sheet3 rat was incorrectly counted as an unresolved lemming\n"; return 1;
  }
  actor.sheetType = 0; actor.state = sim::State::Splatter;
  sim::World dyingWorld(typeSetup);
  dyingWorld.place(actor);
  RuntimeSimState dyingClock;
  dyingClock.advance(dyingWorld, 0.2, true);
  if (dyingWorld.activeCount() != 0 || !dyingWorld.lemmings()[0].alive() ||
      dyingClock.endTransition().phase() != LevelEndPhase::Running) {
    std::cerr << "dying animation was wrongly excluded using activeCount\n"; return 1;
  }
  return 0;
}
