// shared-timing-impl-001: genuine source-built baseline of the MM_W_012 lever route
// through the LEGACY tick-driven path (RuntimeSimState::advance -> SimClock ->
// World::tick). It uses only API that exists both before and after the shared
// elapsed-time change, so the same driver records the pre-edit baseline and then
// proves the documented compatibility wrapper still behaves the same.
//
// Source-built only: the original executable is never run. Observations are
// written to the file named by argv[1] and echoed; the only assertions are driver
// integrity and legacy-wrapper invariants that held in the pre-edit baseline.
#include "app/RuntimeHud.h"
#include "assets/Box.h"
#include "lemmings/compiled_level.hpp"
#include "sim/RetailLevel.h"
#include "sim/World.h"

#include <bit>
#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <span>
#include <filesystem>
#include <format>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

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

std::string describe(const sim::World& w, std::size_t actor) {
  const auto& l = w.lemmings()[actor];
  return std::format("tick={} lever0.state={} actor{}=(x={} y={} state={} facing={} frame={})",
                     w.tickCount(), w.setup().levers.empty() ? 99 : w.setup().levers[0].state,
                     actor, l.x, l.y, static_cast<int>(l.state), static_cast<int>(l.facing), l.frame);
}

sim::World loadWorld(const AssetLibrary& lib) {
  auto setup = sim::loadRetailLevel(lib, "MM_W_012");
  if (!setup) {
    check(false, "load MM_W_012: " + setup.error());
    return sim::World{};
  }
  return sim::World(std::move(*setup));
}

// Synthetic fixture on the real level: a walker standing in the box of lever 0, facing
// the pulling direction. Labelled SYNTHETIC; not natural play.
std::size_t placeAtLever(sim::World& w, sim::Facing facing, int dxFromLeverX = 0,
                         std::size_t leverIndex = 0) {
  const sim::Lever& lev = w.setup().levers.at(leverIndex);
  sim::Lemming l;
  l.x = lev.x + dxFromLeverX;
  l.y = lev.y + lev.height;
  l.facing = facing;
  l.state = sim::State::Walker;
  w.place(l);
  return w.lemmings().size() - 1;
}

// lever-initial-face-impl-001: SERIALIZED stored starting face ([+0xDC]) of the tagged type-1 record
// whose pose lands on placed lever `idx`, read straight from the level bytes and independent of the
// production loader's pairing and state code. 0 = no such record.
int storedFace(const AssetLibrary& lib, const sim::World& w, std::size_t leverIndex) {
  auto resolved = sim::resolveRetailLevel(lib, "MM_W_012");
  if (!resolved) return 0;
  auto blob = lib.read(resolved->file);
  if (!blob) return 0;
  const std::span<const std::uint8_t> raw(reinterpret_cast<const std::uint8_t*>(blob->data()), blob->size());
  const lemmings::CompiledLevel level = lemmings::read_compiled_level(raw, false);
  const sim::Lever& lev = w.setup().levers.at(leverIndex);
  const int width = w.terrain().width(), height = w.terrain().height();
  for (const auto& t : level.gadgets.tagged_entities) {
    if (t.type != 1) continue;
    int col = static_cast<int>(std::lround(std::bit_cast<double>(t.double_bits[0]))) % width;
    if (col < 0) col += width;
    const int boxY = height - 1 - (static_cast<int>(std::bit_cast<double>(t.double_bits[1])) + 6);
    if (col - 1 == lev.x && boxY == lev.y) return static_cast<std::int32_t>(t.values[0]);
  }
  return 0;
}

}  // namespace

int main(int argc, char** argv) {
  const char* dir = std::getenv("WEBOLUTION_GAME_DIR");
  if (!dir || !*dir || !std::filesystem::exists(std::filesystem::path(dir) / "lemmings.box")) {
    std::cout << "shared timing baseline: skipped, WEBOLUTION_GAME_DIR has no lemmings.box\n";
    return 0;
  }
  const std::filesystem::path root(dir);
  auto lib = AssetLibrary::open(root / "lemmings.box", root / "patch.box");
  if (!lib) {
    std::cerr << lib.error() << "\n";
    return 1;
  }

  note("shared-timing-impl-001 legacy-path observations (RuntimeSimState::advance -> World::tick)");
  {
    sim::World probe = loadWorld(*lib);
    note(std::format("MM_W_012: levers={} totalLemmings={} timeLimitTicks={}",
                     probe.setup().levers.size(), probe.setup().totalLemmings,
                     probe.setup().timeLimitTicks));
    for (std::size_t i = 0; i < probe.setup().levers.size(); ++i) {
      const auto& lv = probe.setup().levers[i];
      note(std::format("  lever[{}] x={} y={} w={} h={} state={}", i, lv.x, lv.y, lv.width, lv.height, lv.state));
    }
    check(!probe.setup().levers.empty(), "MM_W_012 has levers");
  }

  // 1. Ordinary sub-200ms frames: 50 ms input. One tick per four frames, lever fires
  //    inside the lemming's own step() at the tick.
  {
    sim::World w = loadWorld(*lib);
    app::RuntimeSimState state;
    const std::size_t a = placeAtLever(w, sim::Facing::Left);
    note("scenario sub200: 8 frames of 50 ms");
    for (int f = 0; f < 8; ++f) {
      const bool ticked = state.advance(w, 0.05, true);
      note(std::format("  frame{} ticked={} {}", f, ticked, describe(w, a)));
    }
    check(w.tickCount() == 2, "legacy: eight 50 ms frames make two 0.2 s ticks");
  }

  // 2. Same-ms (zero) input.
  {
    sim::World w = loadWorld(*lib);
    app::RuntimeSimState state;
    placeAtLever(w, sim::Facing::Left);
    const bool ticked = state.advance(w, 0.0, true);
    note(std::format("scenario zero: advance(0.0) ticked={} tick={}", ticked, w.tickCount()));
    check(!ticked && w.tickCount() == 0, "legacy: zero input makes no update");
  }

  // 3. Stall: 0.25 s then 5.0 s (SimClock caps at 10 ticks; no 100 ms cap exists).
  {
    sim::World w = loadWorld(*lib);
    app::RuntimeSimState state;
    placeAtLever(w, sim::Facing::Left);
    state.advance(w, 0.25, true);
    note(std::format("scenario stall: after 0.25 s tick={}", w.tickCount()));
    check(w.tickCount() == 1, "legacy: 0.25 s makes one tick");
    state.advance(w, 5.0, true);
    note(std::format("scenario stall: after +5.0 s tick={} (per-advance cap)", w.tickCount()));
    check(w.tickCount() == 11, "legacy: 5.0 s stall is capped at ten ticks");
  }

  // 4. Pause / unpause.
  {
    sim::World w = loadWorld(*lib);
    app::RuntimeSimState state;
    placeAtLever(w, sim::Facing::Left);
    state.advance(w, 0.1, true);
    state.togglePause();
    const bool pausedTicked = state.advance(w, 1.0, true);
    state.togglePause();
    state.advance(w, 0.1, true);
    note(std::format("scenario pause: pausedTicked={} tick={} (0.1 s carried is discarded)", pausedTicked, w.tickCount()));
    check(!pausedTicked && w.tickCount() == 0, "legacy: pause discards the accumulator");
  }

  // 5. Seven-pass fast-forward.
  {
    sim::World w = loadWorld(*lib);
    app::RuntimeSimState state;
    placeAtLever(w, sim::Facing::Left);
    state.latchFastForward();
    for (int f = 0; f < 4; ++f) state.advance(w, 0.05, true);
    note(std::format("scenario fastforward: four 50 ms frames, tick={}", w.tickCount()));
    // 4 frames x 7 passes x 50 ms = 1.4 s = seven 0.2 s ticks (pre-edit baseline: tick=7).
    check(w.tickCount() == 7, "legacy: fast-forward reuses the whole frame delta in each of seven passes");
  }

  // 6. Mid-lemming-loop lever ordering: actor0 pulls (the facing the lever's state accepts); actor1
  //    faces the other way and sees the already-toggled state in the same tick.
  //    lever-initial-face-impl-001: lever 0 (79,60) stores starting face -1 (SERIALIZED), so the
  //    accepted first pull faces +1 (state -1 needs face +1, state +1 needs face -1) and the two
  //    toggles return it to the stored face. The old fixture (Left first) and expectation
  //    (state == 1) assumed every lever starts at the constructed +1, which is wrong for this lever.
  {
    sim::World w = loadWorld(*lib);
    app::RuntimeSimState state;
    const int face = storedFace(*lib, w, 0);
    check(face == -1, "SERIALIZED: lever 0 (79,60) stores starting face -1");
    const sim::Facing first = face == 1 ? sim::Facing::Left : sim::Facing::Right;
    const sim::Facing second = face == 1 ? sim::Facing::Right : sim::Facing::Left;
    placeAtLever(w, first);
    placeAtLever(w, second);
    state.advance(w, 0.2, true);
    note(std::format("scenario order: two actors, one tick, lever0.state={} (stored face {}; legacy lets the second actor re-toggle)",
                     w.setup().levers[0].state, face));
    check(w.setup().levers[0].state == face, "legacy: second actor re-toggles inside the same tick");
  }

  // 7. Reload: a fresh World + fresh state starts from tick 0 / lever state as loaded.
  //    lever-initial-face-impl-001: "as loaded" is the SERIALIZED stored face (-1 for lever 0), not the
  //    constructed +1 the old expectation encoded. The pulling walker faces the accepted direction.
  {
    sim::World w = loadWorld(*lib);
    app::RuntimeSimState state;
    const int face = storedFace(*lib, w, 0);
    placeAtLever(w, face == 1 ? sim::Facing::Left : sim::Facing::Right);
    state.advance(w, 0.4, true);
    sim::World reloaded = loadWorld(*lib);
    note(std::format("scenario reload: before tick={} lever0.state={}; after reload tick={} lever0.state={} (stored face {})",
                     w.tickCount(), w.setup().levers[0].state, reloaded.tickCount(), reloaded.setup().levers[0].state, face));
    check(face == -1 && w.setup().levers[0].state != face, "the pulled world's lever left its stored face");
    check(reloaded.tickCount() == 0 && reloaded.setup().levers[0].state == face, "reload restores tick 0 and lever state");
  }

  // 8. Added after the pre-edit baseline run (not in baseline-before-edit.txt): the same legacy
  //    path on the target lever (183,60), index 2. SYNTHETIC walker placement.
  {
    sim::World w = loadWorld(*lib);
    app::RuntimeSimState state;
    const auto& lev = w.setup().levers.at(2);
    check(lev.x == 183 && lev.y == 60, "lever index 2 is the (183,60) target lever");
    const std::size_t a = placeAtLever(w, sim::Facing::Left, 0, 2);
    state.advance(w, 0.2, true);
    note("scenario lever183: " + describe(w, a));
    check(w.setup().levers[2].state == -1, "legacy: lever (183,60) toggles inside the tick");
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
