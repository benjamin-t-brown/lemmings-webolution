// lever-gates-impl-001: serialized lever links -> receiver gates -> the 0x200 zone mask, driven
// through the production loader (loadRetailLevel / loadRetailLevelReport) and the shared elapsed
// path (RuntimeSimState::advanceElapsed -> World::advanceElapsedMs -> updateSegment). Source-built;
// the original executable is never run.
//
// Label key used in the observation file:
//   REAL      installed MM_W_012 data through the public loader, no injected actors
//   SYNTHETIC a walker placed beside/inside a real lever (a fixture, not natural play)
//   FIXTURE   a source-level change to the loaded setup (the supported type-5 lowering request,
//             which no installed MM_W_012 lever sends)
//   UNIT      a hand-built Terrain exercising the rectangle painter
// Expectations come from an independent transcription (struct Ref) of the traced recurrence and
// from rectangles read straight out of the structured level bytes; no number is copied from the
// production code under test. Nothing here claims natural play, visible fluid or original fidelity.
#include "app/RuntimeHud.h"
#include "assets/Box.h"
#include "assets/CollisionGrid.h"
#include "lemmings/compiled_level.hpp"
#include "sim/RetailLevel.h"
#include "sim/World.h"

#include <algorithm>
#include <bit>
#include <cstdint>
#include <cstdlib>
#include <filesystem>
#include <format>
#include <fstream>
#include <iostream>
#include <set>
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

struct Rect {
  int id = 0;
  int request = 0;
  int x0 = 0, x1 = 0;  // original columns [x0, x1)
  int y0 = 0, y1 = 0;  // original rows [y0, y1)
};

std::vector<Rect> g_rects;  // 19 receivers, stream order, read from the level bytes

std::size_t flaggedIn(const sim::Terrain& t, const Rect& r) {
  std::size_t n = 0;
  for (int x = r.x0; x < r.x1; ++x)
    for (int y = r.y0; y < r.y1; ++y)
      if (t.zoneWater(x & 0xFF, t.height() - 1 - y)) ++n;
  return n;
}
const Rect& rectOf(int id) {
  for (const Rect& r : g_rects)
    if (r.id == id) return r;
  static Rect none;
  return none;
}

constexpr std::size_t kTargetLever = 2;  // (183,60)

int rowsAt(double h) { return static_cast<int>(4.0 + h) - 4; }  // painted rows for min.y = 4

// Independent transcription of the traced step (lever-timing section 3 / lever-links section 3),
// specialised to the ids 5/6 gates (kind 0, rate 2.0, low 0, high 24.5, min.y 4). `rows` is the
// number of painted rows from original row 4 up (the mask is a column-aligned stack).
struct Ref {
  int request = 1, done = 2;
  bool armed = false;
  double h = 0.0;
  int rows = 0;
  void message(int type) { request = type == 4 ? 0 : 1; }
  void step(double dt) {
    if (request == done) return;
    if (request == 0) {
      if (h < 24.5) {
        h = dt * 2.0 + h;
      } else {
        h = 24.5;
        if (armed) { armed = false; done = 0; }
      }
      rows = std::max(rows, rowsAt(h));  // set-only
    } else {
      if (h <= 0.0) {
        h = 0.0;
        if (!armed) { armed = true; done = 1; }
      } else {
        rows = 0;  // clear the current AABB
        h = h - dt * 2.0;
        if (0.0 < h) rows = std::max(rows, rowsAt(h));
      }
    }
  }
};

const sim::FluidGate* gateById(const sim::World& w, int id) {
  for (const auto& g : w.setup().gates)
    if (g.id == id) return &g;
  return nullptr;
}

sim::World loadWorld(const AssetLibrary& lib) {
  auto setup = sim::loadRetailLevel(lib, "MM_W_012");
  if (!setup) {
    check(false, "load MM_W_012: " + setup.error());
    return sim::World{};
  }
  return sim::World(std::move(*setup));
}

// SYNTHETIC walker inside lever 2's box. inBox places it on a box column; otherwise one column to
// the right of the box (it first moves after 0.2 s of walk budget, so contact comes later).
std::size_t placeWalker(sim::World& w, sim::Facing facing, bool inBox) {
  const sim::Lever lev = w.setup().levers.at(kTargetLever);
  sim::Lemming l;
  l.x = inBox ? lev.x + 1 : lev.x + lev.width + 1;
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

// Drives World::advanceElapsedMs and keeps the independent reference in step with every update
// segment: a lever state change reports the message the lever list carries (lowerOnPlus selects
// the FIXTURE where the +1 list carries type 5), then the reference steps with THAT segment's dt.
struct Probe {
  sim::World& w;
  Ref ref;
  bool lowerOnPlus = false;
  int prevState = 1;
  std::uint64_t updates = 0, splits = 0, mismatches = 0;
  std::vector<std::string> samples;
  explicit Probe(sim::World& world) : w(world), prevState(world.setup().levers[kTargetLever].state) {}

  bool compare() {
    const sim::FluidGate* g5 = gateById(w, 5);
    const sim::FluidGate* g6 = gateById(w, 6);
    if (!g5 || !g6) return false;
    bool ok = g5->height == ref.h && g6->height == ref.h && g5->request == ref.request &&
              g6->request == ref.request && g5->done == ref.done && g6->done == ref.done &&
              g5->armed == ref.armed && g6->armed == ref.armed;
    const std::size_t want = static_cast<std::size_t>(8 * ref.rows);
    ok = ok && flaggedIn(w.terrain(), rectOf(5)) == want && flaggedIn(w.terrain(), rectOf(6)) == want;
    ok = ok && w.terrain().zoneWaterCells() == 528 + 2 * want;  // nothing else moved
    return ok;
  }
  void run(int ms) {
    w.advanceElapsedMs(ms, false, [&](double dt) {
      ++updates;
      if (dt != 0.016) ++splits;
      const int now = w.setup().levers[kTargetLever].state;
      if (now != prevState) {
        if (now == -1) ref.message(4);
        else if (lowerOnPlus) ref.message(5);
        prevState = now;
      }
      ref.step(dt);
      if (!compare()) ++mismatches;
      return true;
    });
  }
};

}  // namespace

int main(int argc, char** argv) {
  const char* dir = std::getenv("WEBOLUTION_GAME_DIR");
  if (!dir || !*dir || !std::filesystem::exists(std::filesystem::path(dir) / "lemmings.box")) {
    std::cout << "lever gates: skipped, WEBOLUTION_GAME_DIR has no lemmings.box\n";
    return 0;
  }
  const std::filesystem::path root(dir);
  auto lib = AssetLibrary::open(root / "lemmings.box", root / "patch.box");
  if (!lib) {
    std::cerr << lib.error() << "\n";
    return 1;
  }
  note("lever-gates-impl-001 AFTER observations (production loader + shared elapsed path)");

  // ---- UNIT: the rectangle painter (traced traversal) on a hand-built 256 x 96 terrain.
  {
    sim::Terrain t(256, 96);
    t.paintZoneWaterRect(10.9, 4.9, 12.2, 7.5, true);  // cols [10,12) rows [4,7): truncation, exclusive
    int n = 0;
    for (int x = 0; x < 256; ++x)
      for (int y = 0; y < 96; ++y)
        if (t.zoneWater(x, 95 - y)) {
          ++n;
          check(x >= 10 && x < 12 && y >= 4 && y < 7, "UNIT: only cols 10..11 rows 4..6 are painted");
        }
    check(n == 6, "UNIT: exclusive truncated bounds paint 2 x 3 cells");
    check(!t.zoneWater(12, 95 - 4) && !t.zoneWater(10, 95 - 7) && !t.zoneWater(9, 95 - 5), "UNIT: no off-by-one cell");
    t.paintZoneWaterRect(10.9, 4.9, 12.2, 7.5, false);
    check(t.zoneWaterCells() == 0, "UNIT: clear removes exactly the set rectangle");

    sim::Terrain s(256, 96);
    s.paintZoneWaterRect(254.5, 10.0, 2.5, 11.0, true);  // seam: 254,255,0,1 (set-only fix-up)
    check(s.zoneWaterCells() == 4 && s.zoneWater(254, 95 - 10) && s.zoneWater(255, 95 - 10) &&
              s.zoneWater(0, 95 - 10) && s.zoneWater(1, 95 - 10) && !s.zoneWater(2, 95 - 10),
          "UNIT: set wraps the column seam");
    sim::Terrain c(256, 96);
    for (int x = 0; x < 256; ++x) c.setZoneWater(x, 95 - 10, true);
    c.paintZoneWaterRect(254.5, 10.0, 2.5, 11.0, false);  // clear has no seam fix-up
    check(c.zoneWaterCells() == 256, "UNIT: a wrapped rectangle clears nothing");

    sim::Terrain y(256, 96);
    y.paintZoneWaterRect(5.0, 94.0, 6.0, 100.0, true);  // rows clamp to height-1: original rows 94 and 95
    check(y.zoneWaterCells() == 2 && y.zoneWater(5, 95 - 94) && y.zoneWater(5, 95 - 95), "UNIT: rows clamp to the last row");
    sim::Terrain neg(256, 96);
    neg.paintZoneWaterRect(-0.5, -2.0, 1.9, 1.0, true);  // x truncates toward zero (0..0), rows clamp to 0
    check(neg.zoneWaterCells() == 1 && neg.zoneWater(0, 95), "UNIT: truncation toward zero and low-row clamp");
    sim::Terrain none(256, 96);
    none.paintZoneWaterRect(5.0, 5.0, 5.0, 9.0, true);
    check(none.zoneWaterCells() == 0, "UNIT: empty column range paints nothing");
  }

  // ---- REAL: independent receiver rectangles from the structured level bytes.
  auto resolved = sim::resolveRetailLevel(*lib, "MM_W_012");
  check(resolved.has_value(), "resolve MM_W_012");
  if (!resolved) return 1;
  auto blob = lib->read(resolved->file);
  check(blob.has_value(), "read MM_W_012");
  if (!blob) return 1;
  const std::span<const std::uint8_t> raw(reinterpret_cast<const std::uint8_t*>(blob->data()), blob->size());
  const lemmings::CompiledLevel level = lemmings::read_compiled_level(raw, false);
  for (const auto& g : level.gadgets.entities_42acac) {
    Rect r;
    r.id = g.id;
    r.request = static_cast<int>(g.values_18c_128_168[0]);
    r.x0 = static_cast<int>(std::bit_cast<double>(g.double_bits_48_60[0]));
    r.x1 = static_cast<int>(std::bit_cast<double>(g.double_bits_48_60[3]));
    r.y0 = static_cast<int>(std::bit_cast<double>(g.double_bits_48_60[1]));
    r.y1 = static_cast<int>(std::bit_cast<double>(g.double_bits_48_60[4]));
    g_rects.push_back(r);
  }
  check(g_rects.size() == 19, "19 receiver records");

  // ---- REAL: load result through the public loader.
  auto report = sim::loadRetailLevelReport(*lib, "MM_W_012");
  check(report.has_value(), "loadRetailLevelReport MM_W_012");
  if (!report) return 1;
  check(report->gateLinkDiagnostic.empty(), "serialized gadget records applied: " + report->gateLinkDiagnostic);
  check(report->pairedLevers == 4 && report->setup.levers.size() == 4, "all four tagged type-1 levers paired with a placed lever");
  {
    const sim::LevelSetup& s = report->setup;
    note(std::format("REAL load: zoneWaterCells={} (stored 3600 before this task, see baseline-before-edit.txt)", s.terrain.zoneWaterCells()));
    check(s.terrain.zoneWaterCells() == 528, "AFTER: initialization leaves 528 flagged cells");
    std::size_t req1Left = 0, req0Kept = 0;
    for (const Rect& r : g_rects) {
      const std::size_t n = flaggedIn(s.terrain, r);
      if (r.request == 1) {
        req1Left += n;
        check(n == 0, std::format("request-1 rect id {} cleared (192 cells -> 0)", r.id));
      } else {
        req0Kept += n;
        check(n == static_cast<std::size_t>((r.x1 - r.x0) * (r.y1 - r.y0)), std::format("request-0 rect id {} stays full", r.id));
      }
    }
    check(req1Left == 0 && req0Kept == 528, "16 request-1 regions clear 3072, the three request-0 regions keep 528");
    // empty-receiver-fall-001: raw fluid-only cells are no longer immutable
    // Earth. Independently identify those cells from the installed receiver
    // rectangles; every actual floor/wall and every unrelated plane stays exact.
    auto grid = webo::findCollisionGrid(*blob);
    check(grid.has_value(), "collision grid");
    if (grid) {
      const sim::Terrain stored = sim::Terrain::fromCollisionGrid(*grid);
      bool same = stored.width() == s.terrain.width() && stored.height() == s.terrain.height();
      std::size_t diff = 0, materialDiff = 0, fluidOnlyCells = 0;
      for (int y = 0; y < stored.height(); ++y)
        for (int x = 0; x < stored.width(); ++x) {
          const int row = stored.height() - 1 - y;
          const auto word = grid->at(x, row);
          constexpr auto fluidBits = kCellZoneType0 | kCellZoneType1 | kCellZoneType2;
          const bool fluidOnly = (word & fluidBits) && !(word & ~fluidBits) &&
              std::any_of(g_rects.begin(), g_rects.end(), [&](const Rect& r) {
                return x >= r.x0 && x < r.x1 && row >= r.y0 && row < r.y1;
              });
          fluidOnlyCells += fluidOnly;
          const auto expected = fluidOnly ? sim::Material::Empty : stored.at(x, y);
          if (expected != s.terrain.at(x, y) || stored.waterHazard(x, y) != s.terrain.waterHazard(x, y) ||
              stored.zoneAcid(x, y) != s.terrain.zoneAcid(x, y) || stored.antiGrav(x, y) != s.terrain.antiGrav(x, y))
            same = false;
          if (stored.at(x, y) != s.terrain.at(x, y)) ++materialDiff;
          if (stored.zoneWater(x, y) != s.terrain.zoneWater(x, y)) ++diff;
        }
      check(same, "only receiver fluid-only material changes; floors, walls, hazard, acid and anti-grav stay exact");
      check(fluidOnlyCells == 3600 && materialDiff == fluidOnlyCells,
            "exactly 3600 baked fluid-only Earth cells lose immutable material, including the filled upper receiver");
      check(stored.zoneWaterCells() == 3600 && diff == 3072, "exactly the 3072 request-1 cells changed against the stored grid");
      note(std::format("REAL load: stored-grid zoneWaterCells={} changed zone cells={} removed fluid-only material cells={}",
                       stored.zoneWaterCells(), diff, materialDiff));
    }

    // Serialized links by base id (not file order).
    const sim::Lever& lev = s.levers[kTargetLever];
    check(lev.x == 183 && lev.y == 60 && lev.linked && lev.serializedId == 2, "lever (183,60) is tagged record id 2");
    check(lev.initialFace == 1 && lev.cooldownSeconds == 0.0 && lev.facePlus.empty(), "initial face +1, cooldown 0, empty +1 list");
    check(lev.faceMinus.size() == 2 && lev.faceMinus[0].targetId == 5 && lev.faceMinus[1].targetId == 6 &&
              lev.faceMinus[0].type == 4 && lev.faceMinus[1].type == 4,
          "face -1 list is (id 5, type 4), (id 6, type 4)");
    const auto r5 = s.objectIds.find(5);
    const auto r6 = s.objectIds.find(6);
    check(r5 != s.objectIds.end() && r6 != s.objectIds.end() && r5->second.cls == sim::ObjectClass::Gate &&
              r6->second.cls == sim::ObjectClass::Gate,
          "ids 5 and 6 resolve to receiver gates");
    if (r5 != s.objectIds.end() && r6 != s.objectIds.end()) {
      check(s.gates[static_cast<std::size_t>(r5->second.index)].id == 5 && s.gates[static_cast<std::size_t>(r6->second.index)].id == 6,
            "resolution returns the gate whose serialized id matches");
      check(r5->second.index == 12 && r6->second.index == 11 && r5->second.index != 5 && r6->second.index != 6,
            "ids 5/6 are NOT stream positions 5/6 (stream indices 12/11): file order is not the mapping");
    }
    for (const sim::Lever& l : s.levers) {
      std::string lists;
      for (const auto& m : l.faceMinus) lists += std::format(" -1:{}", m.targetId);
      for (const auto& m : l.facePlus) lists += std::format(" +1:{}", m.targetId);
      note(std::format("REAL lever ({},{}) id={} initialFace={} cooldown={}{}", l.x, l.y, l.serializedId, l.initialFace, l.cooldownSeconds, lists));
      for (const auto* lst : {&l.faceMinus, &l.facePlus})
        for (const auto& m : *lst) {
          const auto it = s.objectIds.find(m.targetId);
          check(it != s.objectIds.end() && it->second.cls == sim::ObjectClass::Gate && m.type == 4,
                std::format("every installed link of lever id {} targets a receiver with type 4", l.serializedId));
        }
    }
    check(s.leverStats.delivered == 0 && s.leverStats.unresolved == 0 && s.leverStats.unsupported == 0,
          "load-time fan-out of the stored faces sends nothing in MM_W_012 (all stored-face lists are empty)");
    for (const auto& g : s.gates) {
      check(g.done == 2 && !g.armed, "receivers start with done 2, armed false");
      check(g.request == rectOf(g.id).request, "request equals the stored word");
    }
  }

  // ---- FIXTURE: unsupported targets/types are bounded and never reach a gate.
  {
    sim::World w = loadWorld(*lib);
    sim::LevelSetup s = w.setup();
    const std::size_t before = s.terrain.zoneWaterCells();
    std::vector<sim::LeverMessage> list = {
        {5, 0, 1, 2},     // type 0 to a real gate: vt+0xC, not traced
        {5, 7, 0, 0},     // unknown type to a real gate
        {2, 4, 0, 0},     // type 4 to a lever (tagged object, another class)
        {999, 4, 0, 0},   // empty id slot
        {-2, 6, 0, 0},    // special arm id
    };
    sim::dispatchLeverMessages(s, list);
    check(s.leverStats.delivered == 0 && s.leverStats.unsupported == 4 && s.leverStats.unresolved == 1,
          "FIXTURE: 4 unsupported + 1 unresolved message, none delivered");
    bool untouched = true;
    for (const auto& g : s.gates) untouched = untouched && g.request == (rectOf(g.id).request) && g.done == 2;
    check(untouched && s.terrain.zoneWaterCells() == before, "FIXTURE: no gate or mask changed by unsupported messages");
    const std::vector<sim::LeverMessage> toSix = {{6, 4, 0, 0}};
    sim::dispatchLeverMessages(s, toSix);
    int raised = 0;
    bool sixRaised = false;
    for (const auto& g : s.gates) {
      if (g.request == 0 && rectOf(g.id).request == 1) {
        ++raised;
        sixRaised = g.id == 6;
      }
    }
    check(s.leverStats.delivered == 1 && raised == 1 && sixRaised, "FIXTURE: a type-4 message reaches only the gate with that id");
  }

  // ---- SYNTHETIC: zero-delta first update. Contact runs before receivers in the same update.
  {
    sim::World w = loadWorld(*lib);
    placeWalker(w, sim::Facing::Left, true);
    w.advanceElapsedMs(0);
    const sim::FluidGate* g5 = gateById(w, 5);
    const sim::FluidGate* g6 = gateById(w, 6);
    note(std::format("SYNTHETIC zero-delta first update: lever.state={} g5(req={} done={} armed={} h={}) g6(req={} done={}) other gate id 9 req={} done={} armed={}",
                     w.setup().levers[kTargetLever].state, g5->request, g5->done, g5->armed, g5->height, g6->request, g6->done,
                     gateById(w, 9)->request, gateById(w, 9)->done, gateById(w, 9)->armed));
    check(w.setup().levers[kTargetLever].state == -1, "zero-delta update still activates the lever");
    check(g5->request == 0 && g6->request == 0 && g5->done == 2 && g6->done == 2 && !g5->armed && g5->height == 0.0,
          "update order: the request written by the contact is seen by the receiver in the SAME update (no settle happened first)");
    check(gateById(w, 9)->request == 1 && gateById(w, 9)->done == 1 && gateById(w, 9)->armed,
          "an untargeted request-1 gate settled (done 1, armed) in that zero-delta update");
    check(w.terrain().zoneWaterCells() == 528, "zero-delta update paints no cell");
    // The never-settled edge: reaching `high` with armed false leaves done at 2 (traced edge state).
    for (int i = 0; i < 1000; ++i) w.advanceElapsedMs(100);
    check(g5->height == 24.5 && g5->done == 2 && !g5->armed && flaggedIn(w.terrain(), rectOf(5)) == 192 &&
              flaggedIn(w.terrain(), rectOf(6)) == 192 && w.terrain().zoneWaterCells() == 912,
          "pre-settle activation reaches 24.5 / 384 cells and keeps repainting (done stays 2)");
  }

  // ---- SYNTHETIC: activation, intermediate ramp, repeat pulls, settle (shared dt per segment).
  {
    sim::World w = loadWorld(*lib);
    Probe p(w);
    placeWalker(w, sim::Facing::Left, false);
    for (int f = 0; f < 12; ++f) p.run(16);  // 192 ms of budget: nothing happens yet
    check(w.setup().levers[kTargetLever].state == 1 && gateById(w, 5)->done == 1 && gateById(w, 5)->armed,
          "before contact the pair has settled drained (done 1, armed) and the lever is untouched");
    check(w.terrain().zoneWaterCells() == 528, "528 until the pull");
    p.run(16);  // 13th update: the walker steps, the lever fires after the lemming, the gate steps with the same dt
    const sim::FluidGate* g5 = gateById(w, 5);
    note(std::format("SYNTHETIC activation update: lever.state={} g5.request={} h={} (first raise step used this update's dt 0.016 -> expect {})",
                     w.setup().levers[kTargetLever].state, g5->request, g5->height, 0.016 * 2.0 + 0.0));
    check(w.setup().levers[kTargetLever].state == -1 && g5->request == 0, "lever (183,60) face -1 sent type 4 to ids 5 and 6");
    check(g5->height == 0.016 * 2.0 + 0.0, "the first raise step already used the activating update's dt (same update)");
    check(gateById(w, 9)->request == 1 && gateById(w, 18)->request == 1 && gateById(w, 8)->request == 1 && gateById(w, 4)->request == 1,
          "untargeted gates (ids 9, 18, 8, 4) are unchanged");

    // Repeat pulls during the ramp: a +1 pull sends nothing (empty list); a later -1 pull re-sends
    // type 4 and must not reset the height.
    p.run(16);
    const std::uint64_t deliveredBefore = w.setup().leverStats.delivered;
    placeWalker(w, sim::Facing::Right, true);  // state -1 accepts +1
    p.run(16);
    check(w.setup().levers[kTargetLever].state == 1 && w.setup().leverStats.delivered == deliveredBefore,
          "a +1 pull sends nothing: the +1 list is empty");
    const double hBeforeRepeat = gateById(w, 5)->height;
    placeWalker(w, sim::Facing::Left, true);
    p.run(16);
    check(w.setup().leverStats.delivered >= deliveredBefore + 2, "the next -1 pull re-sent type 4 to both gates");
    check(gateById(w, 5)->height > hBeforeRepeat && gateById(w, 5)->height < 1.0, "repeat during the ramp did not reset the height");
    for (int f = 0; f < 4; ++f) p.run(16);

    std::uint64_t firstRowsUpdate = 0;
    for (int f = 0; f < 1200 && !(gateById(w, 5)->done == 0 && gateById(w, 5)->height == 24.5); ++f) {
      p.run(16);
      if (p.ref.rows == 1 && firstRowsUpdate == 0) firstRowsUpdate = p.updates;
      if (f % 100 == 0)
        note(std::format("  ramp sample: updates={} h={} rows={} cells(ids5+6)={} total={}", p.updates, p.ref.h, p.ref.rows,
                         flaggedIn(w.terrain(), rectOf(5)) + flaggedIn(w.terrain(), rectOf(6)), w.terrain().zoneWaterCells()));
    }
    const sim::FluidGate* s5 = gateById(w, 5);
    note(std::format("SYNTHETIC settled: updates={} splitSegments={} h={} done={} armed={} region(5)={} region(6)={} total={}", p.updates,
                     p.splits, s5->height, s5->done, s5->armed, flaggedIn(w.terrain(), rectOf(5)), flaggedIn(w.terrain(), rectOf(6)),
                     w.terrain().zoneWaterCells()));
    check(p.mismatches == 0, "every update matched the independent reference (heights, bookkeeping, mask membership, nothing else)");
    check(p.splits > 0, "the run crossed 1000 ms level-clock boundaries (event-split segments were exercised)");
    check(s5->height == 24.5 && s5->done == 0 && !s5->armed && gateById(w, 6)->done == 0, "settled endpoint: h 24.5, done 0, armed false");
    check(flaggedIn(w.terrain(), rectOf(5)) + flaggedIn(w.terrain(), rectOf(6)) == 384, "ids 5 and 6 combined hold 384 cells at the settled endpoint");
    check(w.terrain().zoneWaterCells() == 912, "total = 528 + 384 at the endpoint");
    for (const Rect& r : g_rects)
      if (r.id != 5 && r.id != 6 && r.request == 1)
        check(flaggedIn(w.terrain(), r) == 0, std::format("untargeted request-1 rect id {} is still clear", r.id));
    // Off-by-one at the boundary rows/columns of the raised region.
    for (const int id : {5, 6}) {
      const Rect& r = rectOf(id);
      // ids 5 and 6 are adjacent columns, so "inside" is the union of the two raised rectangles.
      bool edgeClean = true;
      for (int x = r.x0 - 1; x <= r.x1; ++x)
        for (int y = r.y0 - 1; y <= r.y1; ++y) {
          bool inside = false;
          for (const int other : {5, 6}) {
            const Rect& o = rectOf(other);
            inside = inside || (x >= o.x0 && x < o.x1 && y >= o.y0 && y < o.y1);
          }
          if (w.terrain().zoneWater(x & 0xFF, w.terrain().height() - 1 - y) != inside) edgeClean = false;
        }
      check(edgeClean, std::format("rect id {}: rows {}..{} x cols {}..{} set, the ring around it (row {}, row {}, cols {}, {}) clear", id, r.y0,
                                   r.y1 - 1, r.x0, r.x1 - 1, r.y0 - 1, r.y1, r.x0 - 1, r.x1));
    }
    // After settling: repeat pulls change nothing; zero-delta update changes nothing.
    const std::size_t cellsSettled = w.terrain().zoneWaterCells();
    placeWalker(w, sim::Facing::Right, true);
    p.run(16);
    placeWalker(w, sim::Facing::Left, true);
    p.run(16);
    p.run(0);
    p.run(100);
    check(p.mismatches == 0 && w.terrain().zoneWaterCells() == cellsSettled && gateById(w, 5)->done == 0 && gateById(w, 5)->height == 24.5,
          "repeat -1 pull after settling and a zero-delta update change nothing (idempotent)");
  }

  // ---- FIXTURE: the supported lowering request (type 5), absent from installed MM_W_012 levers.
  {
    auto setup = sim::loadRetailLevel(*lib, "MM_W_012");
    check(setup.has_value(), "load for lowering fixture");
    if (setup) {
      // FIXTURE: lever 2's +1 list sends type 5 to the same pair, so a +1 pull drains them.
      setup->levers[kTargetLever].facePlus = {{5, 5, 0, 0}, {6, 5, 0, 0}};
      sim::World w(std::move(*setup));
      Probe p(w);
      p.lowerOnPlus = true;
      placeWalker(w, sim::Facing::Left, true);
      for (int i = 0; i < 60; ++i) p.run(16);  // raise partway (activation at update 13)
      const double hMid = gateById(w, 5)->height;
      note(std::format("FIXTURE lowering: raise in progress h={} rows={}", hMid, p.ref.rows));
      check(hMid > 1.0 && hMid < 24.5 && p.ref.rows > 0, "FIXTURE: lowering requested mid-ramp");
      placeWalker(w, sim::Facing::Right, true);
      std::uint64_t lowered = 0;
      for (int i = 0; i < 1500 && !(gateById(w, 5)->done == 1 && gateById(w, 5)->armed); ++i) {
        p.run(16);
        ++lowered;
      }
      const sim::FluidGate* g5 = gateById(w, 5);
      note(std::format("FIXTURE lowering settled after {} updates: h={} done={} armed={} total={}", lowered, g5->height, g5->done, g5->armed,
                       w.terrain().zoneWaterCells()));
      check(p.mismatches == 0, "FIXTURE: lowering matched the reference on every update (clear, subtract, set shortened)");
      check(g5->height == 0.0 && g5->done == 1 && g5->armed && w.terrain().zoneWaterCells() == 528 && gateById(w, 6)->done == 1,
            "FIXTURE: drained endpoint h 0 / done 1 / armed, mask back to 528");
      // And back up again from the drained state (armed true -> settle writes done 0).
      placeWalker(w, sim::Facing::Left, true);
      for (int i = 0; i < 1500 && !(gateById(w, 5)->done == 0 && gateById(w, 5)->height == 24.5); ++i) p.run(16);
      check(p.mismatches == 0 && gateById(w, 5)->done == 0 && !gateById(w, 5)->armed && w.terrain().zoneWaterCells() == 912,
            "FIXTURE: re-raise after a drain settles with done 0 and 384 cells");
    }
  }

  // ---- SYNTHETIC: shared elapsed path (RuntimeSimState): pause, fast-forward, partition independence.
  {
    sim::World a = loadWorld(*lib);
    sim::World b = loadWorld(*lib);
    app::RuntimeSimState sa, sb;
    placeWalker(a, sim::Facing::Left, false);
    placeWalker(b, sim::Facing::Left, false);
    for (int f = 0; f < 60; ++f) sa.advanceElapsed(a, 0.016, true);  // activation at update 13, h ~ 1.5
    const double hBeforePause = gateById(a, 5)->height;
    sa.togglePause();
    const bool pausedUpdated = sa.advanceElapsed(a, 0.5, true);
    sa.togglePause();
    check(!pausedUpdated && gateById(a, 5)->height == hBeforePause && a.terrain().zoneWaterCells() == 528 + 16 * rowsAt(hBeforePause),
          "pause: no update, the ramp does not advance");
    sa.advanceElapsed(a, 0.016, true);
    Ref r;
    r.request = 0;
    r.done = 1;
    r.armed = true;
    r.h = hBeforePause;
    r.step(0.016);
    check(gateById(a, 5)->height == r.h, "first unpaused frame carries only its own 16 ms (no paused time)");
    // Fast-forward: 7 equal updates per frame. 31 ff frames == 217 plain 16 ms updates.
    sb.latchFastForward();
    for (int f = 0; f < 5; ++f) sb.advanceElapsed(b, 0.016, true);  // 35 updates (the walker fires on update 13)
    sim::World c = loadWorld(*lib);
    app::RuntimeSimState sc;
    placeWalker(c, sim::Facing::Left, false);
    for (int f = 0; f < 35; ++f) sc.advanceElapsed(c, 0.016, true);
    note(std::format("SYNTHETIC fast-forward: 5 ff frames h={} clock={} ms; 35 plain frames h={} clock={} ms", gateById(b, 5)->height, b.levelClockMs(),
                     gateById(c, 5)->height, c.levelClockMs()));
    check(gateById(b, 5)->height == gateById(c, 5)->height && gateById(b, 5)->height > 0.0 && b.levelClockMs() == c.levelClockMs() &&
              b.terrain().zoneWaterCells() == c.terrain().zoneWaterCells(),
          "fast-forward (7 equal-dt passes) reaches exactly the same gate state as the same updates un-latched");
    // Event boundary: 990 ms then a 16 ms frame splits into 10 ms + 6 ms updates.
    sim::World d = loadWorld(*lib);
    placeWalker(d, sim::Facing::Left, true);
    d.advanceElapsedMs(1);  // fire in a tiny update so the ramp is live
    d.advanceElapsedMs(988);
    check(d.levelClockMs() == 989, "clock at 989 ms");
    const double h0 = gateById(d, 5)->height;
    std::vector<double> dts;
    d.advanceElapsedMs(16, false, [&](double dt) { dts.push_back(dt); return true; });
    Ref e;
    e.request = 0;
    e.done = 2;
    e.h = h0;
    e.step(0.011);
    e.step(0.005);
    note(std::format("SYNTHETIC event boundary: segments={} h={} (split reference {})", dts.size(), gateById(d, 5)->height, e.h));
    check(dts.size() == 2 && gateById(d, 5)->height == e.h, "event boundary: two updates (11 ms + 5 ms), each step uses its own segment dt");
  }

  // ---- REAL: reload resets every receiver and the mask.
  {
    sim::World w = loadWorld(*lib);
    Probe p(w);
    placeWalker(w, sim::Facing::Left, true);
    for (int i = 0; i < 100; ++i) p.run(16);
    check(w.terrain().zoneWaterCells() > 528 && p.mismatches == 0, "mutated world differs from a fresh load");
    sim::World fresh = loadWorld(*lib);
    check(fresh.terrain().zoneWaterCells() == 528 && gateById(fresh, 5)->request == 1 && gateById(fresh, 5)->done == 2 &&
              !gateById(fresh, 5)->armed && gateById(fresh, 5)->height == 0.0 && fresh.setup().leverStats.delivered == 0 &&
              fresh.setup().levers[kTargetLever].state == 1,
          "reload: 528 cells, receivers back to request 1 / done 2 / armed false / h 0, lever state as loaded");
    check(w.terrain().zoneWaterCells() > 528, "the earlier world is not affected by the reload (receiver state is world-owned)");
  }

  if (argc > 1) {
    std::filesystem::create_directories(std::filesystem::path(argv[1]).parent_path());
    std::ofstream(argv[1], std::ios::binary) << g_log.str();
  }
  std::cout << g_log.str();
  std::cout << std::format("lever gates: {} checks, {} failures\n", g_checks, g_failures);
  return g_failures == 0 ? 0 : 1;
}
