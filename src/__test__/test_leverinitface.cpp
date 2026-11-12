// lever-initial-face-impl-001: the serialized lever starting face ([+0xDC]) is applied to the runtime
// Lever::state AND to the load-time fan-out in one step (0x00444417 -> 0x0043703F -> 0x00436E6A), driven
// through the production loader (loadRetailLevel / loadRetailLevelReport) and the shared elapsed path
// (World::advanceElapsedMs). Source-built; the original executable is never run.
//
// Label key used in the observation file:
//   SERIALIZED the stored faces, lists and receiver rectangles read straight from the level bytes
//              (lemmings::read_compiled_level), independently of the production pairing/state code
//   REAL       installed MM_W_012 through the public loader, no injected actors
//   SYNTHETIC  a walker placed inside a real lever's box (a fixture, not natural play)
//   FIXTURE    the installed level bytes with ONE stored-face word rewritten, served to the unchanged
//              loader through a temporary patch.box (the only way to give a stored-face list that is not
//              empty, since every installed stored-face list is empty), or a hand-built LevelSetup
// Expected directions come from the stored face and the accepted contract "state s accepts the pulling
// facing -s; the pull sets state := facing and sends that face's list", never from the new output.
// Nothing here claims natural play, visible fluid or full timing fidelity.
#include "assets/Box.h"
#include "lemmings/compiled_level.hpp"
#include "sim/RetailLevel.h"
#include "sim/World.h"

#include <algorithm>
#include <array>
#include <bit>
#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <filesystem>
#include <format>
#include <fstream>
#include <iostream>
#include <optional>
#include <set>
#include <span>
#include <sstream>
#include <string>
#include <vector>

namespace {

using namespace webo;
namespace fs = std::filesystem;

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

struct Message {
  int id = 0;
  std::uint32_t type = 0;
};

struct Stored {
  int id = 0;
  int face = 0;  // stored u32 [+0xDC] as a signed value
  int boxX = 0, boxY = 0;
  std::vector<Message> minus, plus;  // lists +0xBC (face -1) and +0xCC (face +1)
  const std::vector<Message>& list(int face) const { return face == 1 ? plus : minus; }
};

struct Rect {
  int id = 0, request = 0, x0 = 0, x1 = 0, y0 = 0, y1 = 0;
};

sim::World worldOf(const AssetLibrary& lib) {
  auto setup = sim::loadRetailLevel(lib, "MM_W_012");
  if (!setup) {
    check(false, "load MM_W_012: " + setup.error());
    return sim::World{};
  }
  return sim::World(std::move(*setup));
}

// SYNTHETIC walker in lever `idx`'s box facing `facing`; it does not walk for the first 0.2 s.
void placeWalker(sim::World& w, std::size_t idx, int facing) {
  const sim::Lever lev = w.setup().levers.at(idx);
  sim::Lemming l;
  l.x = lev.x + 1;
  l.facing = static_cast<sim::Facing>(facing);
  l.state = sim::State::Walker;
  l.y = lev.y + lev.height;
  for (int y = lev.y; y <= lev.y + lev.height; ++y) {
    if (w.terrain().solid(l.x, y + 1)) {
      l.y = y;
      break;
    }
  }
  w.place(l);
}

void update(sim::World& w, int n = 1) {
  for (int i = 0; i < n; ++i) w.advanceElapsedMs(16, false, [](double) { return true; });
}

const sim::FluidGate* gateById(const sim::World& w, int id) {
  for (const auto& g : w.setup().gates)
    if (g.id == id) return &g;
  return nullptr;
}

std::size_t flaggedIn(const sim::Terrain& t, const Rect& r) {
  std::size_t n = 0;
  for (int x = r.x0; x < r.x1; ++x)
    for (int y = r.y0; y < r.y1; ++y)
      if (t.zoneWater(x & 0xFF, t.height() - 1 - y)) ++n;
  return n;
}

std::size_t leverAtBox(const sim::LevelSetup& s, const Stored& st) {
  for (std::size_t i = 0; i < s.levers.size(); ++i)
    if (s.levers[i].x == st.boxX && s.levers[i].y == st.boxY) return i;
  return s.levers.size();
}

std::string describeLists(const Stored& s) {
  std::string out;
  for (const auto& m : s.minus) out += std::format(" -1:(id {} type {})", m.id, m.type);
  for (const auto& m : s.plus) out += std::format(" +1:(id {} type {})", m.id, m.type);
  return out;
}

// ---- FIXTURE support: a temporary patch.box that serves the installed level with one word changed.
void put32(std::vector<std::byte>& v, std::uint32_t x) {
  for (int i = 0; i < 4; ++i) v.push_back(static_cast<std::byte>((x >> (8 * i)) & 0xFF));
}
bool writeBox(const fs::path& path, const std::vector<std::pair<std::string, std::vector<std::byte>>>& entries) {
  std::vector<std::byte> names;
  for (const auto& [name, payload] : entries) {
    put32(names, static_cast<std::uint32_t>(name.size()));
    for (char c : name) names.push_back(static_cast<std::byte>(c));
  }
  const auto count = static_cast<std::uint32_t>(entries.size());
  std::vector<std::byte> out;
  for (char c : std::string("LEMBOX")) out.push_back(static_cast<std::byte>(c));
  put32(out, count);
  put32(out, static_cast<std::uint32_t>(names.size()));
  out.insert(out.end(), names.begin(), names.end());
  put32(out, count);
  auto offset = static_cast<std::uint32_t>(22 + names.size() + count * 8);
  for (const auto& [name, payload] : entries) {
    put32(out, offset);
    offset += static_cast<std::uint32_t>(payload.size());
  }
  put32(out, count);
  for (const auto& [name, payload] : entries) put32(out, static_cast<std::uint32_t>(payload.size()));
  for (const auto& [name, payload] : entries) out.insert(out.end(), payload.begin(), payload.end());
  std::ofstream file(path, std::ios::binary);
  file.write(reinterpret_cast<const char*>(out.data()), static_cast<std::streamsize>(out.size()));
  file.close();
  return static_cast<bool>(file);
}

// Little-endian u32 words of the stored record `face, count, id0, type0` for the lever whose stored
// face word is rewritten; the pattern must occur exactly once so the patched word is the lever's own.
std::vector<std::byte> words(std::initializer_list<std::uint32_t> w) {
  std::vector<std::byte> v;
  for (std::uint32_t x : w) put32(v, x);
  return v;
}

}  // namespace

int main(int argc, char** argv) {
  const char* dir = std::getenv("WEBOLUTION_GAME_DIR");
  if (!dir || !*dir || !fs::exists(fs::path(dir) / "lemmings.box")) {
    std::cout << "lever initial face: skipped, WEBOLUTION_GAME_DIR has no lemmings.box\n";
    return 0;
  }
  const fs::path root(dir);
  auto lib = AssetLibrary::open(root / "lemmings.box", root / "patch.box");
  if (!lib) {
    std::cerr << lib.error() << "\n";
    return 1;
  }
  note("lever-initial-face-impl-001 AFTER observations (production loader + shared elapsed path)");

  // ---- SERIALIZED: stored faces, lists and receiver rectangles, read independently of production.
  auto resolved = sim::resolveRetailLevel(*lib, "MM_W_012");
  check(resolved.has_value(), "resolve MM_W_012");
  if (!resolved) return 1;
  auto blob = lib->read(resolved->file);
  check(blob.has_value(), "read MM_W_012");
  if (!blob) return 1;
  const std::span<const std::uint8_t> raw(reinterpret_cast<const std::uint8_t*>(blob->data()), blob->size());
  const lemmings::CompiledLevel level = lemmings::read_compiled_level(raw, false);

  sim::World probe = worldOf(*lib);
  const int width = probe.terrain().width(), height = probe.terrain().height();
  std::vector<Stored> stored;
  for (const auto& t : level.gadgets.tagged_entities) {
    if (t.type != 1) continue;
    Stored s;
    s.id = static_cast<int>(t.id);
    s.face = static_cast<std::int32_t>(t.values[0]);
    int col = static_cast<int>(std::lround(std::bit_cast<double>(t.double_bits[0]))) % width;
    if (col < 0) col += width;
    s.boxX = col - 1;
    s.boxY = height - 1 - (static_cast<int>(std::bit_cast<double>(t.double_bits[1])) + 6);
    for (const auto& r : t.record_arrays[0]) s.minus.push_back({static_cast<std::int32_t>(r[0]), r[1]});
    for (const auto& r : t.record_arrays[1]) s.plus.push_back({static_cast<std::int32_t>(r[0]), r[1]});
    stored.push_back(s);
    note(std::format("SERIALIZED lever id={} ({},{}) stored face={}{}", s.id, s.boxX, s.boxY, s.face, describeLists(s)));
  }
  check(stored.size() == 4, "four tagged type-1 records");
  std::vector<Rect> rects;
  for (const auto& g : level.gadgets.entities_42acac) {
    Rect r;
    r.id = static_cast<int>(g.id);
    r.request = static_cast<int>(g.values_18c_128_168[0]);
    r.x0 = static_cast<int>(std::bit_cast<double>(g.double_bits_48_60[0]));
    r.x1 = static_cast<int>(std::bit_cast<double>(g.double_bits_48_60[3]));
    r.y0 = static_cast<int>(std::bit_cast<double>(g.double_bits_48_60[1]));
    r.y1 = static_cast<int>(std::bit_cast<double>(g.double_bits_48_60[4]));
    rects.push_back(r);
  }
  const auto rectOf = [&](int id) {
    for (const Rect& r : rects)
      if (r.id == id) return r;
    return Rect{};
  };
  // Cross-check of the stored data against the task statement (documented, not used as a code path):
  for (const Stored& s : stored) {
    if (s.boxX == 79 && s.boxY == 60) check(s.face == -1, "SERIALIZED (79,60) stores -1");
    if (s.boxX == 189 && s.boxY == 35) check(s.face == -1, "SERIALIZED (189,35) stores -1");
    if (s.boxX == 183 && s.boxY == 60) check(s.face == 1, "SERIALIZED (183,60) stores +1");
    if (s.boxX == 72 && s.boxY == 34) check(s.face == 1, "SERIALIZED (72,34) stores +1");
    check(s.face == 1 || s.face == -1, "every installed stored face is a supported +-1");
    check(s.list(s.face).empty(), std::format("lever id {}: the stored-face list is empty (nothing is sent at load)", s.id));
  }

  // ---- REAL: loaded face and load-time bookkeeping, all four levers.
  auto report = sim::loadRetailLevelReport(*lib, "MM_W_012");
  check(report.has_value(), "loadRetailLevelReport MM_W_012");
  if (!report) return 1;
  check(report->gateLinkDiagnostic.empty() && report->pairedLevers == 4 && report->setup.levers.size() == 4,
        "all four levers paired, serialized records applied");
  {
    const sim::LevelSetup& s = report->setup;
    for (const Stored& st : stored) {
      const std::size_t idx = leverAtBox(s, st);
      check(idx < s.levers.size(), std::format("record id {} has a placed lever", st.id));
      if (idx >= s.levers.size()) continue;
      note(std::format("REAL load: lever[{}] ({},{}) stored face={} loaded state={}", idx, st.boxX, st.boxY, st.face,
                       s.levers[idx].state));
      check(s.levers[idx].state == st.face && s.levers[idx].initialFace == st.face,
            std::format("REAL: lever id {} loads state == stored face {}", st.id, st.face));
      check(s.levers[idx].cooldownRemaining == 0.0, "load leaves the cooldown untouched");
    }
    check(s.leverStats.delivered == 0 && s.leverStats.unresolved == 0 && s.leverStats.unsupported == 0,
          "REAL: no message is sent at load (every installed stored-face list is empty)");
    check(s.terrain.zoneWaterCells() == 528, "REAL: 528-cell initialization is preserved");
    for (const auto& g : s.gates) check(g.done == 2 && !g.armed && g.request == rectOf(g.id).request,
                                        "receivers keep the stored request, done 2, armed false");
  }

  // ---- SYNTHETIC: first accepted / rejected pull for every lever, from the stored face.
  for (const Stored& st : stored) {
    const std::size_t idx = leverAtBox(probe.setup(), st);
    if (idx >= probe.setup().levers.size()) continue;
    const int accept = -st.face;  // state s accepts the pulling facing -s
    const std::vector<Message>& sent = st.list(accept);
    check(!sent.empty(), std::format("lever id {}: the accepted pull's list is non-empty (a visible receiver request)", st.id));
    for (const int facing : {accept, st.face}) {
      sim::World w = worldOf(*lib);
      placeWalker(w, idx, facing);
      update(w);
      const bool accepted = facing == accept;
      note(std::format("SYNTHETIC lever ({},{}) stored {} walker facing {}: state {} -> {}, delivered={}", st.boxX, st.boxY,
                       st.face, facing, st.face, w.setup().levers[idx].state, w.setup().leverStats.delivered));
      check(w.setup().levers[idx].state == (accepted ? accept : st.face),
            std::format("lever id {} facing {}: {}", st.id, facing, accepted ? "accepted, state flips to the pull facing" : "rejected, state unchanged"));
      check(w.setup().leverStats.delivered == (accepted ? sent.size() : 0u) && w.setup().leverStats.unsupported == 0,
            "delivered messages match the accepted face's list (none when rejected)");
      for (const Message& m : sent) {
        const sim::FluidGate* g = gateById(w, m.id);
        check(g != nullptr, "target gate exists");
        if (!g) continue;
        if (accepted) {
          // Same update: the contact's request reaches the receiver and the receiver steps with that dt.
          note(std::format("  gate{} request={} done={} h={}", g->id, g->request, g->done, g->height));
          check(g->request == 0 && g->done == 2 && g->height == 0.016 * 2.0,
                std::format("accepted pull: gate {} raises in the same update (request 0, h == dt*2.0)", m.id));
        } else {
          check(g->request == rectOf(m.id).request && g->height == 0.0, "rejected pull: the targeted receiver is untouched");
        }
      }
      if (accepted)
        check(w.terrain().zoneWaterCells() == 528, "no cell painted by the very first raise step (rows are whole-cell)");
    }
  }

  // ---- SYNTHETIC repeat: two walkers alternate; each accepted pull flips the state and sends its list.
  for (const Stored& st : stored) {
    const std::size_t idx = leverAtBox(probe.setup(), st);
    if (idx >= probe.setup().levers.size()) continue;
    const int accept = -st.face;
    sim::World w = worldOf(*lib);
    placeWalker(w, idx, accept);   // A: pulls first
    placeWalker(w, idx, st.face);  // B: pulls once the state has flipped
    std::vector<int> states;
    std::vector<std::size_t> delivered;
    for (int u = 0; u < 4; ++u) {
      update(w);
      states.push_back(w.setup().levers[idx].state);
      delivered.push_back(w.setup().leverStats.delivered);
    }
    const std::size_t n = st.list(accept).size(), m = st.list(st.face).size();
    note(std::format("SYNTHETIC repeat lever ({},{}): states {} {} {} {} delivered {} {} {} {}", st.boxX, st.boxY, states[0],
                     states[1], states[2], states[3], delivered[0], delivered[1], delivered[2], delivered[3]));
    check(states == std::vector<int>({accept, st.face, accept, st.face}), "repeat: the state alternates one pull per update");
    check(delivered == std::vector<std::size_t>({n, n + m, 2 * n + m, 2 * n + 2 * m}),
          "repeat: each pull sends that facing's list again (stored-face list empty, so only the accepted list adds)");
    const sim::FluidGate* g = gateById(w, st.list(accept).front().id);
    check(g && g->request == 0 && std::abs(g->height - 4 * 0.016 * 2.0) < 1e-12,
          "repeat: re-sending type 4 does not restart the ramp (h keeps growing)");
  }

  // ---- SYNTHETIC ramp: the +1 lever (183,60) ids 5/6 regression (384 cells, total 912).
  {
    const Stored* t = nullptr;
    for (const Stored& st : stored)
      if (st.boxX == 183 && st.boxY == 60) t = &st;
    check(t != nullptr, "lever (183,60) present");
    if (t) {
      const std::size_t idx = leverAtBox(probe.setup(), *t);
      sim::World w = worldOf(*lib);
      placeWalker(w, idx, -t->face);
      update(w, 1000);
      const Rect r5 = rectOf(5), r6 = rectOf(6);
      const std::size_t c5 = flaggedIn(w.terrain(), r5), c6 = flaggedIn(w.terrain(), r6);
      note(std::format("SYNTHETIC ramp (183,60): h5={} h6={} cells5={} cells6={} total={}", gateById(w, 5)->height,
                       gateById(w, 6)->height, c5, c6, w.terrain().zoneWaterCells()));
      check(c5 + c6 == 384 && w.terrain().zoneWaterCells() == 912 && gateById(w, 5)->height == 24.5,
            "ids 5/6 ramp: 384 cells filled, total 912, settled height 24.5");
    }
  }

  // ---- REAL reload: a mutated world and a fresh load; the face and receivers are rebuilt.
  {
    const std::size_t idx = leverAtBox(probe.setup(), stored[0]);
    sim::World w = worldOf(*lib);
    placeWalker(w, idx, -stored[0].face);
    update(w, 3);
    check(w.setup().levers[idx].state == -stored[0].face, "mutated world's lever left its stored face");
    sim::World fresh = worldOf(*lib);
    bool faces = true;
    for (const Stored& st : stored) faces = faces && fresh.setup().levers[leverAtBox(fresh.setup(), st)].state == st.face;
    check(faces, "reload: every lever is back at its stored face");
    check(fresh.setup().leverStats.delivered == 0 && fresh.terrain().zoneWaterCells() == 528 &&
              gateById(fresh, 5)->request == 1 && gateById(fresh, 5)->done == 2 && gateById(fresh, 5)->height == 0.0,
          "reload: no load messages, 528 cells, receivers rebuilt");
    check(w.setup().levers[idx].state == -stored[0].face, "the earlier world is unaffected by the reload");
    note("REAL reload: all four levers back at their stored faces, 528 cells, receivers request 1 / done 2 / h 0");
  }

  // ---- UNIT/FIXTURE: unlinked (legacy / synthetic) levers are not touched by the load stage.
  {
    sim::LevelSetup s;
    s.levers.push_back({10, 10, 2, 6, 1});
    s.levers.back().initialFace = -1;  // ignored: no serialized record is linked
    sim::fanOutLeverLoadFaces(s);
    check(s.levers[0].state == 1 && !s.levers[0].linked && s.leverStats.delivered == 0,
          "FIXTURE: an unlinked lever keeps its constructed state and sends nothing");
    s.levers[0].linked = true;
    s.levers[0].initialFace = 5;  // unsupported stored value: ignored by 0x00436E6A
    sim::fanOutLeverLoadFaces(s);
    check(s.levers[0].state == 1 && s.leverStats.delivered == 0 && s.leverStats.unsupported == 0,
          "FIXTURE: an unsupported stored face leaves the state and sends nothing");
  }

  // ---- FIXTURE through the unchanged loader: a non-empty stored-face list proves the once-only fan-out.
  // Lever id 2 (183,60) stores +1 with face -1 list (id 5 type 4, id 6 type 4). Rewriting only its stored
  // face word to -1 makes the load-time list the two-message list; rewriting it to 5 is unsupported.
  {
    const Stored* t = nullptr;
    for (const Stored& st : stored)
      if (st.boxX == 183 && st.boxY == 60) t = &st;
    check(t != nullptr, "FIXTURE: lever (183,60) record present");
    if (!t) return 1;
    const auto needle = words({1u, static_cast<std::uint32_t>(t->minus.size()), static_cast<std::uint32_t>(t->minus[0].id), t->minus[0].type});
    std::size_t at = std::string::npos, hits = 0;
    for (std::size_t i = 0; i + needle.size() <= blob->size(); ++i)
      if (std::equal(needle.begin(), needle.end(), blob->begin() + static_cast<std::ptrdiff_t>(i))) {
        ++hits;
        at = i;
      }
    check(hits == 1, "FIXTURE: the stored record words occur exactly once in the level bytes");
    const fs::path outDir = (argc > 1 ? fs::path(argv[1]).parent_path() : fs::path(".")) / "fixture";
    fs::create_directories(outDir);
    const auto serve = [&](std::uint32_t faceWord, const char* name) -> std::optional<AssetLibrary> {
      std::vector<std::byte> patched = *blob;
      for (int i = 0; i < 4; ++i) patched[at + static_cast<std::size_t>(i)] = static_cast<std::byte>((faceWord >> (8 * i)) & 0xFF);
      std::vector<std::pair<std::string, std::vector<std::byte>>> entries;
      const std::string want = normaliseAssetName(resolved->file);
      if (lib->patch())
        for (const auto& e : lib->patch()->entries()) {
          if (normaliseAssetName(e.name) == want) continue;
          auto bytes = lib->patch()->read(e);
          if (bytes) entries.emplace_back(e.name, std::move(*bytes));
        }
      entries.emplace_back(resolved->file, std::move(patched));
      const fs::path p = outDir / name;
      if (!writeBox(p, entries)) return std::nullopt;
      auto l = AssetLibrary::open(root / "lemmings.box", p);
      if (!l) return std::nullopt;
      return std::move(*l);
    };
    if (hits == 1) {
      const std::size_t idx = leverAtBox(probe.setup(), *t);
      // (a) stored face -1: state -1 and the -1 list (ids 5, 6) sent exactly once at load.
      auto minusLib = serve(0xFFFFFFFFu, "face-minus.box");
      check(minusLib.has_value(), "FIXTURE: temporary patch.box (stored face -1) opens");
      if (minusLib) {
        auto rep = sim::loadRetailLevelReport(*minusLib, "MM_W_012");
        check(rep.has_value(), "FIXTURE: load with stored face -1");
        if (rep) {
          const sim::LevelSetup& s = rep->setup;
          const sim::FluidGate* g5 = nullptr;
          const sim::FluidGate* g6 = nullptr;
          for (const auto& g : s.gates) {
            if (g.id == 5) g5 = &g;
            if (g.id == 6) g6 = &g;
          }
          note(std::format("FIXTURE stored face -1: state={} delivered={} gate5(req={} done={} h={}) gate6(req={} done={} h={}) cells={}",
                           s.levers[idx].state, s.leverStats.delivered, g5->request, g5->done, g5->height, g6->request, g6->done,
                           g6->height, s.terrain.zoneWaterCells()));
          check(s.levers[idx].state == -1 && s.levers[idx].initialFace == -1, "FIXTURE: state := stored -1");
          check(s.leverStats.delivered == 2 && s.leverStats.unsupported == 0 && s.leverStats.unresolved == 0,
                "FIXTURE: the list is dispatched exactly once at load (2 messages, not 4)");
          check(g5 && g6 && g5->request == 0 && g6->request == 0 && g5->done == 2 && g6->done == 2 && g5->height == 0.0,
                "FIXTURE: ids 5/6 carry request 0 at load, before any update");
          check(s.terrain.zoneWaterCells() == 528, "FIXTURE: the load-time cell pass is unchanged (528)");
          check(s.levers[idx].cooldownRemaining == 0.0, "FIXTURE: load fan-out leaves the cooldown at 0");
          // First pull directions follow the state the face set: -1 accepts +1 (empty +1 list), rejects -1.
          sim::World w(std::move(rep->setup));
          placeWalker(w, idx, -1);
          update(w);
          check(w.setup().levers[idx].state == -1 && w.setup().leverStats.delivered == 2, "FIXTURE: a facing -1 walker is rejected");
          placeWalker(w, idx, 1);
          update(w);
          check(w.setup().levers[idx].state == 1 && w.setup().leverStats.delivered == 2, "FIXTURE: a facing +1 walker is accepted (empty +1 list)");
          // Reload through the same loader dispatches once more, on a fresh setup.
          auto again = sim::loadRetailLevelReport(*minusLib, "MM_W_012");
          check(again && again->setup.leverStats.delivered == 2 && again->setup.levers[idx].state == -1,
                "FIXTURE: reload rebuilds state and fan-out (2 messages on the fresh setup)");
        }
      }
      // (b) unsupported stored value: the lever keeps its constructed +1 and nothing is sent.
      auto oddLib = serve(5u, "face-five.box");
      check(oddLib.has_value(), "FIXTURE: temporary patch.box (stored face 5) opens");
      if (oddLib) {
        auto rep = sim::loadRetailLevelReport(*oddLib, "MM_W_012");
        check(rep.has_value(), "FIXTURE: load with stored face 5");
        if (rep) {
          note(std::format("FIXTURE stored face 5: state={} delivered={}", rep->setup.levers[idx].state, rep->setup.leverStats.delivered));
          check(rep->setup.levers[idx].state == 1 && rep->setup.leverStats.delivered == 0 && rep->setup.leverStats.unsupported == 0,
                "FIXTURE: unsupported stored face keeps the constructed state and sends nothing");
        }
      }
    }
  }

  if (argc > 1) {
    fs::create_directories(fs::path(argv[1]).parent_path());
    std::ofstream(argv[1], std::ios::binary) << g_log.str();
  }
  std::cout << g_log.str();
  std::cout << std::format("lever initial face: {} checks, {} failures\n", g_checks, g_failures);
  return g_failures == 0 ? 0 : 1;
}
