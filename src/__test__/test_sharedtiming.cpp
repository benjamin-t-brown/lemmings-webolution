// Check RuntimeSimState::advanceElapsed -> World::advanceElapsedMs input
// policy and update count and order on installed MM_W_012. A fixture places
// a walker near lever 2 at (183,60).
#include "app/RuntimeHud.h"
#include "assets/Box.h"
#include "sim/RetailLevel.h"
#include "sim/World.h"

#include <cstdlib>
#include <filesystem>
#include <format>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

namespace {

using namespace webo;

int g_failures = 0;
int g_checks = 0;
std::ostringstream g_log;

void check(bool cond, const std::string& what) {
  ++g_checks;
  if (!cond) {
    ++g_failures;
    std::cerr << "  FAIL  " << what << "\n";
    g_log << "FAIL " << what << "\n";
  }
}
void note(const std::string& line) { g_log << line << "\n"; }

sim::World loadWorld(const AssetLibrary& lib) {
  auto setup = sim::loadRetailLevel(lib, "MM_W_012");
  if (!setup) {
    check(false, "load MM_W_012: " + setup.error());
    return sim::World{};
  }
  return sim::World(std::move(*setup));
}

constexpr std::size_t kTargetLever = 2;  // (183,60)

// SYNTHETIC walker beside the real lever. `dxFromRight` is relative to the lever box's right
// edge (lev.x + lev.width); the feet row is the first with a solid cell below it in the box rows.
std::size_t placeWalker(sim::World& w, sim::Facing facing, int dxFromRight) {
  const sim::Lever lev = w.setup().levers.at(kTargetLever);
  sim::Lemming l;
  l.x = lev.x + lev.width + dxFromRight;
  l.facing = facing;
  l.state = sim::State::Walker;
  l.y = lev.y + lev.height;
  for (int y = lev.y; y <= lev.y + lev.height; ++y) {
    if (w.terrain().solid(l.x, y + 1)) {
      l.y = y;
      break;
    }
  }
  w.place(l);
  return w.lemmings().size() - 1;
}

std::string leverLine(const sim::World& w, std::size_t actor) {
  const auto& l = w.lemmings().at(actor);
  return std::format("clockMs={} tick={} updates={} lever2.state={} actor{}=(x={} y={} state={})",
                     w.levelClockMs(), w.tickCount(), w.explicitUpdateCount(),
                     w.setup().levers.at(kTargetLever).state, actor, l.x, l.y, static_cast<int>(l.state));
}

bool sourceContains(const std::filesystem::path& file, const std::string& needle) {
  std::ifstream in(file, std::ios::binary);
  std::stringstream ss;
  ss << in.rdbuf();
  return ss.str().find(needle) != std::string::npos;
}

}  // namespace

int main(int argc, char** argv) {
  const char* dir = std::getenv("WEBOLUTION_GAME_DIR");
  if (!dir || !*dir || !std::filesystem::exists(std::filesystem::path(dir) / "lemmings.box")) {
    std::cout << "shared timing: skipped, WEBOLUTION_GAME_DIR has no lemmings.box\n";
    return 0;
  }
  const std::filesystem::path root(dir);
  auto lib = AssetLibrary::open(root / "lemmings.box", root / "patch.box");
  if (!lib) {
    std::cerr << lib.error() << "\n";
    return 1;
  }
  note("shared-timing-impl-001 connected-path observations (RuntimeSimState::advanceElapsed -> World::advanceElapsedMs)");

  // Check main.cpp uses the explicit elapsed-time update entry point.
#ifdef STI_SOURCE_DIR
  {
    const std::filesystem::path appMain = std::filesystem::path(STI_SOURCE_DIR) / "src/app/main.cpp";
    const bool explicitCall = sourceContains(appMain, "runtimeSim.advanceElapsed(");
    const bool legacyCall = sourceContains(appMain, "runtimeSim.advance(");
    note(std::format("callpath: main.cpp advanceElapsed={} legacy advance={}", explicitCall, legacyCall));
    check(explicitCall && !legacyCall, "main.cpp drives the world through advanceElapsed only");
  }
#endif

  // A walker one column outside lever 2 first steps after thirteen 16 ms
  // updates (208 ms). Contact follows movement in the same update.
  {
    sim::World w = loadWorld(*lib);
    app::RuntimeSimState state;
    const std::size_t a = placeWalker(w, sim::Facing::Left, 1);
    const int x0 = w.lemmings()[a].x;
    note("scenario sub200 (SYNTHETIC walker): " + leverLine(w, a));
    check(w.lemmings()[a].state == sim::State::Walker, "fixture: walker stands on the lever row");
    std::vector<std::uint64_t> clocks;
    int firedAtFrame = -1;
    for (int f = 1; f <= 13; ++f) {
      const bool updated = state.advanceElapsed(w, 0.016, true, [&] { clocks.push_back(w.levelClockMs()); });
      check(updated && state.lastPassMs() == 16, "16 ms frame is one 16 ms pass");
      if (firedAtFrame < 0 && w.setup().levers[kTargetLever].state == -1) firedAtFrame = f;
      if (f == 12) {
        note("  frame12 " + leverLine(w, a));
        check(w.lemmings()[a].x == x0 && w.setup().levers[kTargetLever].state == 1, "no movement or contact before 0.2 s of budget");
      }
    }
    note("  frame13 " + leverLine(w, a) + std::format(" firedAtFrame={}", firedAtFrame));
    check(w.explicitUpdateCount() == 13 && clocks.size() == 13, "one world update per 16 ms frame");
    check(w.levelClockMs() == 208 && w.tickCount() == 1, "level clock is integer ms; tick_ derived (208 ms -> tick 1)");
    check(w.lemmings()[a].x == x0 - 1, "walker advanced one column on the update that spent 0.2 s of budget");
    check(firedAtFrame == 13, "contact evaluated after movement in the SAME update (legacy fires only at a later tick)");
    check(w.timeRemainingTicks() == static_cast<std::uint64_t>(w.setup().timeLimitTicks) - w.tickCount(),
          "HUD/timeout state reads the same level clock");
  }

  // The original performs a full update at dt = 0.
  {
    sim::World w = loadWorld(*lib);
    app::RuntimeSimState state;
    const std::size_t a = placeWalker(w, sim::Facing::Left, 0);  // inside the box: contact is live
    const bool updated = state.advanceElapsed(w, 0.0, true);
    note("scenario zero-delta (SYNTHETIC): updated=" + std::to_string(updated) + " " + leverLine(w, a));
    check(updated && w.explicitUpdateCount() == 1 && w.levelClockMs() == 0 && state.lastPassMs() == 0,
          "dt = 0 still runs one zero-delta update and advances no clock");
    check(w.setup().levers[kTargetLever].state == -1, "zero-delta update still evaluates and activates contact");
    // Sub-millisecond frames: the fraction is carried, so 3 x 0.4 ms yields exactly one 1 ms pass.
    sim::World w2 = loadWorld(*lib);
    app::RuntimeSimState s2;
    s2.advanceElapsed(w2, 0.0004, false);
    s2.advanceElapsed(w2, 0.0004, false);
    check(w2.levelClockMs() == 0 && w2.explicitUpdateCount() == 2, "two 0.4 ms frames: two zero-delta updates");
    s2.advanceElapsed(w2, 0.0004, false);
    check(w2.levelClockMs() == 1 && w2.explicitUpdateCount() == 3, "third 0.4 ms frame releases the carried 1 ms");
  }

  // 3. >100 ms stall: capped, discarded time is not carried; exactly 100 ms is unclamped.
  {
    sim::World w = loadWorld(*lib);
    app::RuntimeSimState state;
    state.advanceElapsed(w, 0.100, false);
    check(state.lastPassMs() == 100 && state.discardedStallMs() == 0, "exactly 100 ms is not clamped");
    state.advanceElapsed(w, 0.250, false);
    note(std::format("scenario stall: lastPassMs={} discarded={} clockMs={}", state.lastPassMs(),
                     state.discardedStallMs(), w.levelClockMs()));
    check(state.lastPassMs() == 100 && state.discardedStallMs() == 150 && w.levelClockMs() == 200,
          "250 ms frame simulates 100 ms; the other 150 ms are dropped");
    state.advanceElapsed(w, 0.016, false);
    check(state.lastPassMs() == 16 && w.levelClockMs() == 216, "no catch-up after a stall");
    // An owned deterministic clock disables the wall-clock cap and simulates its declared step.
    app::RuntimeSimState owned;
    sim::World w3 = loadWorld(*lib);
    owned.advanceElapsed(w3, 0.4, false, {}, 0);
    check(w3.levelClockMs() == 400 && w3.tickCount() == 2 && w3.explicitUpdateCount() == 2,
          "owned clock 0.4 s: 400 ms split at two tick boundaries, tick 2");
  }

  // 4. Pause / unpause.
  {
    sim::World w = loadWorld(*lib);
    app::RuntimeSimState state;
    state.advanceElapsed(w, 0.0505, false);  // 50 ms + 0.5 ms carried
    state.togglePause();
    const bool pausedUpdated = state.advanceElapsed(w, 1.0, false);
    state.togglePause();
    check(!pausedUpdated && w.levelClockMs() == 50 && state.carriedMs() == 0.0, "pause drops input and the carry");
    state.advanceElapsed(w, 0.016, false);
    note(std::format("scenario pause: clockMs={} lastPassMs={}", w.levelClockMs(), state.lastPassMs()));
    check(state.lastPassMs() == 16 && w.levelClockMs() == 66, "first unpaused frame carries only its own delta");
  }

  // 5. Seven same-input passes.
  {
    sim::World w = loadWorld(*lib);
    app::RuntimeSimState state;
    state.latchFastForward();
    state.advanceElapsed(w, 0.016, false);
    note(std::format("scenario fastforward: clockMs={} updates={}", w.levelClockMs(), w.explicitUpdateCount()));
    check(w.levelClockMs() == 7 * 16 && w.explicitUpdateCount() == 7, "seven passes each reuse the same 16 ms");
  }

  // 6. Level-second boundary: split into two updates, zero-length tail when it lands on the end.
  for (int variant = 0; variant < 2; ++variant) {
    sim::World w = loadWorld(*lib);
    app::RuntimeSimState state;
    const double lead = variant == 0 ? 0.090 : 0.084;  // 9 x 100 ms + lead -> 990 / 984 ms
    for (int i = 0; i < 9; ++i) state.advanceElapsed(w, 0.100, false);
    state.advanceElapsed(w, lead, false);
    const std::uint64_t before = w.levelClockMs();
    check(w.levelSecondEvents() == 0, "no level-second event before 1000 ms");
    std::vector<std::uint64_t> clocks;
    state.advanceElapsed(w, 0.016, false, [&] { clocks.push_back(w.levelClockMs()); });
    note(std::format("scenario level-second variant{}: before={} update clocks=[{}, {}] events={}", variant, before,
                     clocks.size() > 0 ? clocks[0] : 0, clocks.size() > 1 ? clocks[1] : 0, w.levelSecondEvents()));
    check(clocks.size() == 2 && w.levelSecondEvents() == 1, "boundary splits the pass into two world updates and fires one event");
    if (variant == 0)
      check(clocks.size() == 2 && clocks[0] == 1000 && clocks[1] == 1006, "990 ms + 16 ms: updates end at 1000 and 1006");
    else
      check(clocks.size() == 2 && clocks[0] == 1000 && clocks[1] == 1000, "984 ms + 16 ms: update to 1000 then a zero-length trailing update");
  }

  // With opposing walkers inside lever 2, the first eligible walker in list order
  // activates it. The explicit elapsed path accepts one activation per update.
  {
    sim::World w = loadWorld(*lib);
    app::RuntimeSimState state;
    placeWalker(w, sim::Facing::Left, 0);
    const std::size_t b = placeWalker(w, sim::Facing::Right, 0);
    state.advanceElapsed(w, 0.016, false);
    note("scenario order update1 (SYNTHETIC): " + leverLine(w, b));
    check(w.setup().levers[kTargetLever].state == -1, "update 1: a single activation (-1), no second per-lemming toggle");
    state.advanceElapsed(w, 0.016, false);
    note("scenario order update2 (SYNTHETIC): " + leverLine(w, b));
    check(w.setup().levers[kTargetLever].state == 1, "update 2: the other facing activates in the next update");
    check(w.explicitUpdateCount() == 2, "exactly one contact evaluation per update");
  }

  // 8. REAL level, no injected actors: natural hatch release under the explicit path.
  {
    sim::World w = loadWorld(*lib);
    app::RuntimeSimState state;
    for (int f = 0; f < 220; ++f) state.advanceElapsed(w, 0.016, true);  // 3.52 s
    note(std::format("scenario natural-release (REAL, no synthetic actors): clockMs={} tick={} spawned={} secondEvents={}",
                     w.levelClockMs(), w.tickCount(), w.spawned(), w.levelSecondEvents()));
    check(w.levelClockMs() == 220 * 16 && w.tickCount() == w.levelClockMs() / 200, "clock is the sum of integer frame ms");
    check(w.levelSecondEvents() == 3, "three 1 Hz events in 3.52 s");
    check(w.spawned() >= 1, "the real hatch released a lemming through the continuous release model");
  }

  // 9. Frame partitioning does not change the level clock, tick count or events.
  {
    sim::World a = loadWorld(*lib), b = loadWorld(*lib);
    app::RuntimeSimState sa, sb;
    const std::size_t ia = placeWalker(a, sim::Facing::Left, 3);
    const std::size_t ib = placeWalker(b, sim::Facing::Left, 3);
    for (int i = 0; i < 10; ++i) sa.advanceElapsed(a, 0.100, false);
    for (int i = 0; i < 100; ++i) sb.advanceElapsed(b, 0.010, false);
    note("scenario partition 10x100ms: " + leverLine(a, ia));
    note("scenario partition 100x10ms: " + leverLine(b, ib));
    check(a.levelClockMs() == 1000 && b.levelClockMs() == 1000 && a.tickCount() == b.tickCount() &&
              a.levelSecondEvents() == 1 && b.levelSecondEvents() == 1,
          "same total elapsed gives the same clock, ticks and level-second events");
    check(a.lemmings()[ia].x == b.lemmings()[ib].x && a.lemmings()[ia].y == b.lemmings()[ib].y &&
              a.setup().levers[kTargetLever].state == b.setup().levers[kTargetLever].state,
          "continuous walker motion is partition independent (SYNTHETIC)");
  }

  // 10. Reload: a fresh World and RuntimeSimState restart every clock and the lever state.
  {
    sim::World w = loadWorld(*lib);
    app::RuntimeSimState state;
    placeWalker(w, sim::Facing::Left, 0);
    state.advanceElapsed(w, 0.0505, true);
    sim::World reloaded = loadWorld(*lib);
    app::RuntimeSimState fresh;
    note(std::format("scenario reload: before clockMs={} lever2={}; after clockMs={} lever2={} events={} carry={}",
                     w.levelClockMs(), w.setup().levers[kTargetLever].state, reloaded.levelClockMs(),
                     reloaded.setup().levers[kTargetLever].state, reloaded.levelSecondEvents(), fresh.carriedMs()));
    check(reloaded.levelClockMs() == 0 && reloaded.tickCount() == 0 && reloaded.levelSecondEvents() == 0 &&
              reloaded.explicitUpdateCount() == 0 && reloaded.setup().levers[kTargetLever].state == 1 &&
              fresh.carriedMs() == 0.0,
          "reload restarts level clock, event count, lever state and input carry");
  }

  // Results stops simulation updates.
  {
    sim::World w = loadWorld(*lib);
    app::RuntimeSimState state;
    // Without a timeout or releases, the level stays in Running.
    state.advanceElapsed(w, 0.016, true);
    check(state.endTransition().phase() == app::LevelEndPhase::Running, "completion tracking starts in Running");
  }

  std::cout << g_log.str();
  if (argc > 1) {
    std::filesystem::create_directories(std::filesystem::path(argv[1]).parent_path());
    std::ofstream out(argv[1], std::ios::binary);
    out << g_log.str();
  }
  std::cout << std::format("\n{} checks, {} failures\n", g_checks, g_failures);
  return g_failures == 0 ? 0 : 1;
}
