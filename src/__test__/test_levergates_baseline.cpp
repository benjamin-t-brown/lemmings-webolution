// lever-gates-impl-001 BASELINE: what the production load + runtime path does with the MM_W_012
// zone mask BEFORE serialized lever links / receiver gates exist. Source-built, original never run.
//
// This driver deliberately uses only the pre-existing public API (loadRetailLevel,
// RuntimeSimState::advanceElapsed -> World::advanceElapsedMs, Terrain::zoneWater*) so the same
// source compiled against the untouched production tree (build/lever-gates-impl-001/pre-edit)
// records the genuine "before" state; gate rectangles are read independently from the level bytes
// through the structured reader, never through new production code.
//
// Label key: REAL = installed data, SYNTHETIC = one walker placed beside lever (183,60).
#include "app/RuntimeHud.h"
#include "assets/Box.h"
#include "lemmings/compiled_level.hpp"
#include "sim/RetailLevel.h"
#include "sim/World.h"

#include <bit>
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
std::ostringstream g_log;

void check(bool cond, const std::string& what) {
  if (!cond) {
    ++g_failures;
    std::cerr << "  FAIL  " << what << "\n";
    g_log << "FAIL " << what << "\n";
  }
}

struct Rect {
  int id = 0;
  int request = 0;
  int x0 = 0, x1 = 0;  // [x0, x1) original columns
  int y0 = 0, y1 = 0;  // [y0, y1) original rows (exclusive, truncated)
};

std::size_t flaggedIn(const sim::Terrain& t, const Rect& r) {
  std::size_t n = 0;
  for (int x = r.x0; x < r.x1; ++x)
    for (int y = r.y0; y < r.y1; ++y)
      if (t.zoneWater(x & 0xFF, t.height() - 1 - y)) ++n;
  return n;
}

}  // namespace

int main(int argc, char** argv) {
  const char* dir = std::getenv("WEBOLUTION_GAME_DIR");
  if (!dir || !*dir || !std::filesystem::exists(std::filesystem::path(dir) / "lemmings.box")) {
    std::cout << "lever gates baseline: skipped, WEBOLUTION_GAME_DIR has no lemmings.box\n";
    return 0;
  }
  const std::filesystem::path root(dir);
  auto lib = AssetLibrary::open(root / "lemmings.box", root / "patch.box");
  if (!lib) {
    std::cerr << lib.error() << "\n";
    return 1;
  }
  g_log << "lever-gates-impl-001 BASELINE (pre-edit production semantics; REAL MM_W_012 via loadRetailLevel)\n";

  // Independent gate rectangles from the structured reader (entities_42acac), in stream order.
  auto resolved = sim::resolveRetailLevel(*lib, "MM_W_012");
  check(resolved.has_value(), "resolve MM_W_012");
  if (!resolved) return 1;
  auto blob = lib->read(resolved->file);
  check(blob.has_value(), "read MM_W_012");
  if (!blob) return 1;
  const std::span<const std::uint8_t> raw(reinterpret_cast<const std::uint8_t*>(blob->data()), blob->size());
  const lemmings::CompiledLevel level = lemmings::read_compiled_level(raw, false);
  std::vector<Rect> rects;
  for (const auto& g : level.gadgets.entities_42acac) {
    Rect r;
    r.id = g.id;
    r.request = static_cast<int>(g.values_18c_128_168[0]);
    r.x0 = static_cast<int>(std::bit_cast<double>(g.double_bits_48_60[0]));
    r.x1 = static_cast<int>(std::bit_cast<double>(g.double_bits_48_60[3]));
    r.y0 = static_cast<int>(std::bit_cast<double>(g.double_bits_48_60[1]));
    r.y1 = static_cast<int>(std::bit_cast<double>(g.double_bits_48_60[4]));
    rects.push_back(r);
  }
  check(rects.size() == 19, "19 gate records");

  auto setup = sim::loadRetailLevel(*lib, "MM_W_012");
  check(setup.has_value(), "loadRetailLevel MM_W_012");
  if (!setup) return 1;
  sim::World w(std::move(*setup));
  const std::size_t stored = w.terrain().zoneWaterCells();
  g_log << std::format("REAL load: zoneWaterCells={}\n", stored);
  check(stored == 3600, "BASELINE: MM_W_012 starts with 3600 stored flagged cells (nothing is cleared at load)");
  std::size_t inRects = 0, req1 = 0, ids56 = 0;
  for (const Rect& r : rects) {
    const std::size_t n = flaggedIn(w.terrain(), r);
    inRects += n;
    if (r.request == 1) req1 += n;
    if (r.id == 5 || r.id == 6) ids56 += n;
  }
  g_log << std::format("REAL load: flagged in 19 gate rects={} (request-1 rects={}, ids 5+6={})\n", inRects, req1, ids56);
  check(inRects == 3600 && req1 == 3072 && ids56 == 384, "BASELINE: 3600 = 3072 request-1 + 528 request-0; ids 5/6 hold 384");

  // SYNTHETIC: a walker facing -1 on lever (183,60) (index 2) pulls it; the zone mask does not move.
  const sim::Lever lev = w.setup().levers.at(2);
  check(lev.x == 183 - 1 || lev.x > 0, "lever 2 exists");
  sim::Lemming l;
  l.x = lev.x + lev.width + 1;
  l.facing = sim::Facing::Left;
  l.state = sim::State::Walker;
  l.y = lev.y + lev.height;
  for (int y = lev.y; y <= lev.y + lev.height; ++y)
    if (w.terrain().solid(l.x, y + 1)) { l.y = y; break; }
  w.place(l);
  app::RuntimeSimState state;
  int fired = -1;
  for (int f = 1; f <= 400; ++f) {
    state.advanceElapsed(w, 0.016, true);
    if (fired < 0 && w.setup().levers[2].state == -1) fired = f;
  }
  g_log << std::format("SYNTHETIC walker: lever2 fired at frame {}, lever2.state={}, zoneWaterCells after 400 x 16 ms={}\n",
                       fired, w.setup().levers[2].state, w.terrain().zoneWaterCells());
  check(fired > 0, "SYNTHETIC: the lever was pulled");
  check(w.terrain().zoneWaterCells() == 3600, "BASELINE: pulling lever (183,60) does not alter the 3600 flagged cells");

  const std::string out = argc > 1 ? argv[1] : "";
  if (!out.empty()) {
    std::filesystem::create_directories(std::filesystem::path(out).parent_path());
    std::ofstream(out, std::ios::binary) << g_log.str();
  }
  std::cout << g_log.str();
  std::cout << (g_failures == 0 ? "lever gates baseline: PASS\n" : "lever gates baseline: FAIL\n");
  return g_failures == 0 ? 0 : 1;
}
