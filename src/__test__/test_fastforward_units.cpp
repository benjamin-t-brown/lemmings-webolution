// fastforward-impl-001 unit checks: the fast-forward latch, the 7-pass budget in
// RuntimeSimState::advance and the plate-8/9 panel classification. All expected
// tick counts are hand-computed from SimClock's documented step arithmetic
// (0.2 s steps, remainder carried, 10 ticks per advance), not by calling the
// advance code twice. The World is the installed MM_CLIMB_002 level.
//
// Source-test evidence only: 7 is a pass budget (dword 0x4B15F8), not a measured
// speedup, and nothing here establishes original timing fidelity.
#include "app/RuntimeHud.h"
#include "app/RuntimeSkillPanel.h"
#include "assets/Box.h"
#include "sim/RetailLevel.h"

#include <cstdio>
#include <cstdlib>
#include <filesystem>
#include <iostream>

namespace {
void check(bool ok, const char* what) {
  if (!ok) { std::fprintf(stderr, "FAIL: %s\n", what); std::exit(1); }
}
}  // namespace

int main() {
  using namespace webo;
  using namespace webo::app;
  using namespace webo::sim;

  const char* dir = std::getenv("WEBOLUTION_GAME_DIR");
  if (!dir || !*dir || !std::filesystem::exists(std::filesystem::path(dir) / "lemmings.box")) {
    std::cout << "FastForwardUnits: SKIPPED (retail assets unavailable)\n";
    return 0;
  }
  auto library = AssetLibrary::open(std::filesystem::path(dir) / "lemmings.box",
                                    std::filesystem::path(dir) / "patch.box");
  check(bool(library), "open retail archive");
  auto setup = loadRetailLevel(*library, "MM_CLIMB_002");
  check(bool(setup), "load MM_CLIMB_002");
  const auto fresh = [&] { return World(*setup); };

  // -- panel classification (geometry is runtimeSkillPanelIndex's, unchanged) --
  check(runtimeSkillPanelIndex(50, 332) == 8 && runtimeSkillPanelIndex(50, 363) == 8, "plate 8 spans y 332..363");
  check(runtimeSkillPanelIndex(50, 372) == 9 && runtimeSkillPanelIndex(50, 403) == 9, "plate 9 spans y 372..403");
  check(runtimeSkillPanelCommand(18, 332) == RuntimePanelCommand::FastForward, "plate 8 origin pixel");
  check(runtimeSkillPanelCommand(89, 363) == RuntimePanelCommand::FastForward, "plate 8 last pixel");
  check(runtimeSkillPanelCommand(50, 348) == RuntimePanelCommand::FastForward, "plate 8 centre");
  check(runtimeSkillPanelCommand(50, 388) == RuntimePanelCommand::Armageddon, "plate 9 centre");
  check(runtimeSkillPanelCommand(50, 364) == RuntimePanelCommand::None, "gap under plate 8");
  check(runtimeSkillPanelCommand(50, 404) == RuntimePanelCommand::None, "below plate 9");
  check(runtimeSkillPanelCommand(17, 348) == RuntimePanelCommand::None, "left of plate 8");
  check(runtimeSkillPanelCommand(90, 348) == RuntimePanelCommand::None, "right of plate 8");
  check(runtimeSkillPanelCommand(50, 28) == RuntimePanelCommand::None, "skill plate 0 is no command");
  check(runtimeSkillPanelCommand(320, 240) == RuntimePanelCommand::None, "viewport is no command");

  // -- fresh state is unlatched; unlatched advance is the pre-task behaviour --
  {
    RuntimeSimState state;
    check(!state.fastForward(), "fresh RuntimeSimState is unlatched");
    World world = fresh();
    // 0.4 s = exactly two 0.2 s steps per call, one pass: 2 ticks per call.
    for (int i = 0; i < 3; ++i) check(state.advance(world, 0.4), "unlatched advance ticks");
    check(world.tickCount() == 6, "unlatched 3 x 0.4 s = 6 ticks");
    check(!state.fastForward(), "advancing never latches");
  }

  // -- latched: seven same-delta passes ---------------------------------------
  {
    RuntimeSimState state;
    World world = fresh();
    state.latchFastForward();
    check(state.fastForward() && RuntimeSimState::kFastForwardPasses == 7, "latched, budget is 7");
    check(state.advance(world, 0.4), "latched advance ticks");
    check(world.tickCount() == 14, "latched 0.4 s: 7 passes x 2 ticks = 14");
    state.advance(world, 0.4); state.advance(world, 0.4);
    check(world.tickCount() == 42, "three latched frames of 0.4 s = 42 ticks");
    state.latchFastForward();  // explicit enable remains idempotent; player input uses toggle
    check(state.fastForward(), "second explicit enable does not toggle off");
    check(state.advance(world, 0.4) && world.tickCount() == 56, "still fast after explicit enable");
    // P17 supersedes the old second-press latch expectation: public input can
    // cancel speed, then enable it again, without changing the seven-pass budget.
    state.toggleFastForward();
    check(!state.fastForward(), "second player gesture turns fast-forward off");
    check(state.advance(world, 0.4) && world.tickCount() == 58, "normal budget after toggling off");
    state.toggleFastForward();
    check(state.fastForward(), "third player gesture turns fast-forward on again");
    check(state.advance(world, 0.4) && world.tickCount() == 72, "seven-pass budget after toggling on again");
  }
  {
    // 0.5 s: passes owe 2,3,2,3,2,3,2 = 17 ticks (3.5 s = 17.5 steps; 0.1 s carried), then
    // the carried 0.1 s plus 3.5 s = 3.6 s = 18 steps, 35 cumulative (7.0 s = 35 steps).
    RuntimeSimState state;
    World world = fresh();
    state.latchFastForward();
    state.advance(world, 0.5);
    check(world.tickCount() == 17, "latched 0.5 s first frame = 17 ticks");
    state.advance(world, 0.5);
    check(world.tickCount() == 35, "latched 0.5 s second frame = 35 cumulative ticks");
    // Unlatched control on the same delta: 0.5 s = 2.5 steps; 2 then 3 ticks.
    RuntimeSimState plain;
    World control = fresh();
    plain.advance(control, 0.5);
    check(control.tickCount() == 2, "unlatched 0.5 s first frame = 2 ticks");
    plain.advance(control, 0.5);
    check(control.tickCount() == 5, "unlatched 0.5 s second frame = 5 cumulative ticks");
  }
  {
    // The existing 10-ticks-per-advance cap applies to each pass: 3.0 s owes 15
    // steps, each pass takes 10 and carries 1.0 s more, so 7 passes take 70.
    RuntimeSimState state;
    World world = fresh();
    state.latchFastForward();
    state.advance(world, 3.0);
    check(world.tickCount() == 70, "per-pass 10-tick cap: 7 x 10 = 70");
    RuntimeSimState plain;
    World control = fresh();
    plain.advance(control, 3.0);
    check(control.tickCount() == 10, "unlatched 3.0 s stays capped at 10");
  }

  // -- pause preserves the latch and ticks nothing ----------------------------
  {
    RuntimeSimState state;
    World world = fresh();
    state.advance(world, 0.4);                       // 2 unlatched ticks
    state.togglePause();
    state.latchFastForward();                        // pressed while paused
    check(state.paused() && state.fastForward(), "latch set while paused");
    check(!state.advance(world, 10.0) && world.tickCount() == 2, "paused latched advance ticks nothing");
    state.togglePause();
    check(state.fastForward(), "resume keeps the latch");
    check(state.advance(world, 0.4) && world.tickCount() == 2 + 14, "after resume the fast count applies");
    state.togglePause();
    state.resume();
    check(state.fastForward() && !state.paused(), "pause/resume cycle keeps the latch");
  }

  // -- a fresh per-leg state is unlatched (Retry / next map / return to map) --
  {
    RuntimeSimState first;
    first.latchFastForward();
    RuntimeSimState next;
    check(first.fastForward() && !next.fastForward(), "new state starts unlatched");
  }

  // -- the level-end stop ------------------------------------------------------
  {
    // No entrance and one lemming owed: ends by timeout (limit 3 ticks) and the
    // 7 s result countdown. The unlatched run reaches Results at some tick T0 in
    // (30,34] (test_runtimehud measures the same fixture); latched must stop at the
    // same T0 inside the first 70-tick budget instead of ticking on.
    LevelSetup timeout;
    timeout.totalLemmings = 1;
    timeout.timeLimitTicks = 3;
    World plainWorld(timeout), fastWorld(timeout);
    RuntimeSimState plain, fast;
    for (int i = 0; i < 8 && !plain.endTransition().snapshot(); ++i) plain.advance(plainWorld, 2.0, true);
    const auto t0 = plainWorld.tickCount();
    check(plain.endTransition().snapshot() && t0 > 30 && t0 <= 34, "unlatched run reaches Results in (30,34]");
    fast.latchFastForward();
    check(fast.advance(fastWorld, 2.0, true), "latched completion run ticks");
    check(fast.endTransition().snapshot().has_value(), "latched run reaches Results within one advance");
    check(fastWorld.tickCount() == t0, "latched pass loop stops at the same Results tick");
    check(!fast.advance(fastWorld, 2.0, true) && fastWorld.tickCount() == t0, "no ticks after Results");
  }

  // -- plate 8/9 clicks: consumed, no stock read/decrement, selection unchanged;
  //    skill assignment afterwards still works through the existing path --------
  {
    World world = fresh();
    RuntimeSimState state;
    RuntimeSkillSelection selection;
    check(selection.selectIndex(2) && selection.selectedIndex() == 2, "select bomber slot");
    const auto stockBefore = runtimeSkillPanelCounts(world);
    for (int y : {348, 388}) {
      check(selection.consumePanelClick(50, y, stockBefore), "command plate click is consumed");
      check(selection.selectedIndex() == 2, "command plate click keeps the selection");
    }
    // Zero stock must not matter to a command plate (it never reads stock).
    check(selection.consumePanelClick(50, 348, std::array<int, 9>{}) && selection.selectedIndex() == 2,
          "plate 8 ignores stock entirely");
    check(runtimeSkillPanelCounts(world) == stockBefore, "stock untouched by command plates");

    // The caller's rule: only plate 8 latches (main.cpp). Plate 9 stays inert.
    for (int y : {388, 364, 28}) {
      if (runtimeSkillPanelCommand(50, y) == RuntimePanelCommand::FastForward) state.latchFastForward();
    }
    check(!state.fastForward(), "plate 9, the gap and a skill plate never latch");
    if (runtimeSkillPanelCommand(50, 348) == RuntimePanelCommand::FastForward) state.latchFastForward();
    check(state.fastForward(), "plate 8 latches");

    // Run latched until the first lemming is live, then assign through World::assign.
    for (int i = 0; i < 3; ++i) state.advance(world, 0.4);
    check(world.tickCount() == 42, "latched ticks reached the live-lemming window");
    std::size_t target = world.lemmings().size();
    for (std::size_t i = 0; i < world.lemmings().size(); ++i)
      if (world.lemmings()[i].active() && !world.lemmings()[i].canClimb) { target = i; break; }
    check(target < world.lemmings().size(), "an active lemming exists after latched ticks");
    const auto counts = runtimeSkillPanelCounts(world);
    check(counts[0] > 0, "climber stock available");
    selection.selectIndex(0);
    check(world.assign(target, selection.selectedSkill()), "assignment after latching succeeds");
    check(runtimeSkillPanelCounts(world)[0] == counts[0] - 1, "assignment decrements exactly one climber");
    check(world.lemmings()[target].canClimb, "assigned lemming is a climber");
  }

  std::cout << "PASS: fast-forward latch, 7-pass budget, pause, Results stop, plate 8/9 on MM_CLIMB_002\n";
  return 0;
}
