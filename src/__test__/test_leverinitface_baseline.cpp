// lever-initial-face-impl-001: genuine PRE-EDIT observation of the four MM_W_012 levers through the
// public loader (loadRetailLevel) and the shared elapsed path (World::advanceElapsedMs). It uses only
// API that exists both before and after the change, so the same measurement procedure is repeated by
// test_leverinitface.cpp after the edit.
//
// Labels: SERIALIZED = read straight from the level bytes (lemmings::read_compiled_level); REAL = the
// public loader's result; SYNTHETIC = a walker placed inside a real lever's box (a fixture, not natural
// play). The original executable is never run. Observation only: the sole assertions are driver
// integrity, so this file records the wrong-default behaviour instead of failing on it.
#include "assets/Box.h"
#include "lemmings/compiled_level.hpp"
#include "sim/RetailLevel.h"
#include "sim/World.h"

#include <bit>
#include <cmath>
#include <cstdint>
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

struct Stored {
  int id = 0;
  int face = 0;  // stored u32 [+0xDC] as a signed value
  int boxX = 0, boxY = 0;
  std::vector<std::array<std::uint32_t, 4>> minus, plus;
};

sim::World loadWorld(const AssetLibrary& lib) {
  auto setup = sim::loadRetailLevel(lib, "MM_W_012");
  if (!setup) {
    check(false, "load MM_W_012: " + setup.error());
    return sim::World{};
  }
  return sim::World(std::move(*setup));
}

std::size_t placeWalker(sim::World& w, std::size_t leverIndex, sim::Facing facing) {
  const sim::Lever lev = w.setup().levers.at(leverIndex);
  sim::Lemming l;
  l.x = lev.x + 1;
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

const sim::FluidGate* gateById(const sim::World& w, int id) {
  for (const auto& g : w.setup().gates)
    if (g.id == id) return &g;
  return nullptr;
}

}  // namespace

int main(int argc, char** argv) {
  const char* dir = std::getenv("WEBOLUTION_GAME_DIR");
  if (!dir || !*dir || !std::filesystem::exists(std::filesystem::path(dir) / "lemmings.box")) {
    std::cout << "lever initial face baseline: skipped, WEBOLUTION_GAME_DIR has no lemmings.box\n";
    return 0;
  }
  const std::filesystem::path root(dir);
  auto lib = AssetLibrary::open(root / "lemmings.box", root / "patch.box");
  if (!lib) {
    std::cerr << lib.error() << "\n";
    return 1;
  }
  note("lever-initial-face-impl-001 BASELINE observations (production sources before any edit)");

  auto resolved = sim::resolveRetailLevel(*lib, "MM_W_012");
  check(resolved.has_value(), "resolve MM_W_012");
  if (!resolved) return 1;
  auto blob = lib->read(resolved->file);
  check(blob.has_value(), "read MM_W_012");
  if (!blob) return 1;
  const std::span<const std::uint8_t> raw(reinterpret_cast<const std::uint8_t*>(blob->data()), blob->size());
  const lemmings::CompiledLevel level = lemmings::read_compiled_level(raw, false);

  sim::World probe = loadWorld(*lib);
  const int width = probe.terrain().width(), height = probe.terrain().height();
  std::vector<Stored> stored;
  for (const auto& t : level.gadgets.tagged_entities) {
    if (t.type != 1) continue;
    Stored s;
    s.id = static_cast<int>(t.id);
    s.face = static_cast<std::int32_t>(t.values[0]);
    const double rx = std::bit_cast<double>(t.double_bits[0]);
    const double ry = std::bit_cast<double>(t.double_bits[1]);
    int col = static_cast<int>(std::lround(rx)) % width;
    if (col < 0) col += width;
    s.boxX = col - 1;
    s.boxY = height - 1 - (static_cast<int>(ry) + 6);
    s.minus = t.record_arrays[0];
    s.plus = t.record_arrays[1];
    stored.push_back(s);
  }
  check(stored.size() == 4, "four tagged type-1 records");

  for (const Stored& s : stored) {
    std::size_t idx = probe.setup().levers.size();
    for (std::size_t i = 0; i < probe.setup().levers.size(); ++i)
      if (probe.setup().levers[i].x == s.boxX && probe.setup().levers[i].y == s.boxY) idx = i;
    check(idx < probe.setup().levers.size(), std::format("record id {} has a placed lever", s.id));
    if (idx >= probe.setup().levers.size()) continue;
    std::string lists;
    for (const auto& m : s.minus) lists += std::format(" -1:(id {} type {})", m[0], m[1]);
    for (const auto& m : s.plus) lists += std::format(" +1:(id {} type {})", m[0], m[1]);
    note(std::format("SERIALIZED lever id={} ({},{}) stored face={}{}", s.id, s.boxX, s.boxY, s.face, lists));
    note(std::format("REAL load: lever index {} ({},{}) loaded state={} delivered-at-load={}", idx, s.boxX, s.boxY,
                     probe.setup().levers[idx].state, probe.setup().leverStats.delivered));
    for (const sim::Facing facing : {sim::Facing::Left, sim::Facing::Right}) {
      sim::World w = loadWorld(*lib);
      placeWalker(w, idx, facing);
      const int before = w.setup().levers[idx].state;
      int changedAt = -1;
      for (int u = 0; u < 4; ++u) {
        w.advanceElapsedMs(16, false, [&](double) { return true; });
        if (changedAt < 0 && w.setup().levers[idx].state != before) changedAt = u + 1;
      }
      std::string tg;
      for (const auto* lst : {&s.minus, &s.plus})
        for (const auto& m : *lst) {
          const sim::FluidGate* g = gateById(w, static_cast<int>(m[0]));
          if (g) tg += std::format(" gate{}(req={} done={} h={})", g->id, g->request, g->done, g->height);
        }
      note(std::format("SYNTHETIC lever ({},{}) walker facing {}: state {} -> {} (changed at update {}), delivered={}{}",
                       s.boxX, s.boxY, static_cast<int>(facing), before, w.setup().levers[idx].state, changedAt,
                       w.setup().leverStats.delivered, tg));
    }
  }
  note(std::format("REAL load: zoneWaterCells={}", probe.terrain().zoneWaterCells()));

  if (argc > 1) {
    std::filesystem::create_directories(std::filesystem::path(argv[1]).parent_path());
    std::ofstream(argv[1], std::ios::binary) << g_log.str();
  }
  std::cout << g_log.str();
  std::cout << std::format("lever initial face baseline: {} checks, {} failures\n", g_checks, g_failures);
  return g_failures == 0 ? 0 : 1;
}
