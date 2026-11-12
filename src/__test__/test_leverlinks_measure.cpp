// lever-links-research-001: read-only measurement of the serialized lever link
// fields. Source-built; the original executable is never run. Reports what the
// unchanged structured reader (lemmings::read_compiled_level) holds for every
// tagged type-1 record (vtable 0x004B1288, ctor 0x00436C4E, reader 0x00436FED),
// locates each field in the raw member bytes independently of that reader, and
// resolves each message target id against every actor collection.
//
// It asserts only measurement integrity (level readable, raw-byte location of
// every lever unique, clone lever boxes tie to a structured record). It does not
// assert an in-game effect: the receiver behaviour is a static-analysis claim
// documented in docs/scratch/lever-links-research-001.md.
#include "assets/Box.h"
#include "assets/CollisionGrid.h"
#include "assets/LevelIndex.h"
#include "assets/LevelObjects.h"
#include "lemmings/compiled_level.hpp"
#include "sim/RetailLevel.h"
#include "sim/World.h"

#include <algorithm>
#include <bit>
#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <filesystem>
#include <format>
#include <fstream>
#include <iostream>
#include <map>
#include <optional>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

namespace fs = std::filesystem;

namespace {

constexpr const char* kPrimaryLevel = "MM_W_012";

using Bytes = std::vector<std::uint8_t>;

struct Pat {
  Bytes b;
  std::vector<bool> wild;  // true = any byte
  void u8(std::uint8_t v) { b.push_back(v); wild.push_back(false); }
  void any() { b.push_back(0); wild.push_back(true); }
  void le(std::uint64_t v, int n) {
    for (int i = 0; i < n; ++i) u8(static_cast<std::uint8_t>(v >> (8 * i)));
  }
};

std::vector<std::size_t> findAll(const Bytes& d, const Pat& p, std::size_t from = 0, std::size_t to = SIZE_MAX) {
  std::vector<std::size_t> hits;
  if (p.b.empty() || d.size() < p.b.size()) return hits;
  const std::size_t last = std::min(to, d.size() - p.b.size());
  for (std::size_t i = from; i <= last; ++i) {
    bool ok = true;
    for (std::size_t k = 0; k < p.b.size() && ok; ++k)
      if (!p.wild[k] && d[i + k] != p.b[k]) ok = false;
    if (ok) hits.push_back(i);
  }
  return hits;
}

std::uint32_t rd32(const Bytes& d, std::size_t o) {
  return static_cast<std::uint32_t>(d[o]) | (static_cast<std::uint32_t>(d[o + 1]) << 8) |
         (static_cast<std::uint32_t>(d[o + 2]) << 16) | (static_cast<std::uint32_t>(d[o + 3]) << 24);
}

std::uint64_t fnv1a(const Bytes& d) {
  std::uint64_t h = 1469598103934665603ull;
  for (auto c : d) { h ^= c; h *= 1099511628211ull; }
  return h;
}

// Base block 0x00423291: five doubles (+8,+10,+18,+20,+30), flag +0x38, flag +0x28, int16 id (+0x3A).
Pat basePattern(const lemmings::EntityBaseFields& e) {
  Pat p;
  for (auto bits : e.double_bits) p.le(bits, 8);
  p.any();
  p.any();
  p.le(static_cast<std::uint16_t>(e.id), 2);
  return p;
}

struct Target {
  std::string collection;
  std::size_t index = 0;
  std::string detail;
  std::uint16_t idRaw = 0;
};

struct Census {
  std::size_t levels = 0, levelsWithLever = 0, levers = 0, leversWithAnyRecord = 0;
  std::size_t totalRecords = 0, unresolvedRecords = 0, negativeIds = 0;
  std::map<std::string, std::size_t> targetClass;
  std::map<std::uint32_t, std::size_t> msgTypes;
  std::map<std::int64_t, std::size_t> initWord;
  std::map<std::string, std::size_t> bothListsEmpty;  // level -> levers without any record
  std::size_t listLenMismatch = 0;  // |+0xbc| != |+0xcc|
  std::size_t sameTargetsBothLists = 0;
  std::size_t flagA0Set = 0, flagB0Set = 0;
};

std::string modelName(const lemmings::ModelEntity& m) { return m.model ? m.model->name : std::string(); }

bool isLeverName(const std::string& s) {
  std::string l;
  for (char c : s) l.push_back(static_cast<char>(std::tolower(static_cast<unsigned char>(c))));
  return l.find("lever") != std::string::npos;
}

}  // namespace

int main() {
  const char* dir = std::getenv("WEBOLUTION_GAME_DIR");
  if (!dir) {
    std::cout << "lever link measurement: SKIP (WEBOLUTION_GAME_DIR unset)\n";
    return 0;
  }
  auto lib = webo::AssetLibrary::open(fs::path(dir) / "lemmings.box", fs::path(dir) / "patch.box");
  if (!lib) {
    std::cerr << "open: " << lib.error() << "\n";
    return 1;
  }
  int failures = 0;
  const auto fail = [&](const std::string& why) {
    std::cerr << "FAIL: " << why << "\n";
    ++failures;
  };

  std::ostringstream primary, census;
  Census c;
  std::string primaryFile;

  for (const auto& name : webo::levelFileNames(*lib)) {
    auto blob = lib->read(name);
    if (!blob) continue;
    ++c.levels;
    const Bytes raw(reinterpret_cast<const std::uint8_t*>(blob->data()),
                    reinterpret_cast<const std::uint8_t*>(blob->data()) + blob->size());
    lemmings::CompiledLevel level;
    try {
      level = lemmings::read_compiled_level(std::span<const std::uint8_t>(raw.data(), raw.size()), false);
    } catch (const std::invalid_argument& e) {
      census << std::format("{} READ-REJECTED: {}\n", name, e.what());
      fail(name + " rejected by structured reader");
      continue;
    }
    const auto& g = level.gadgets;

    // ---- id -> target resolution table, in construction (= stream) order.
    // 0x00423310..0x00423325 stores every actor into list[id]; a later actor with the same id wins.
    std::map<int, Target> byId;
    std::map<int, int> idCount;
    const auto addBase = [&](const char* coll, std::size_t i, const lemmings::EntityBaseFields& b,
                             const std::string& detail) {
      const int id = b.id;
      byId[id] = {coll, i, detail, static_cast<std::uint16_t>(b.id)};
      ++idCount[id];
    };
    const auto addModels = [&](const char* coll, const auto& v) {
      for (std::size_t i = 0; i < v.size(); ++i) addBase(coll, i, v[i], "model='" + modelName(v[i]) + "'");
    };
    addModels("basic_entities@0x448650", g.basic_entities);
    addModels("linked_entities@0x448710", g.linked_entities);
    addModels("entities_425e50@0x4487F0", g.entities_425e50);
    addModels("entities_426c6b@0x4488F0", g.entities_426c6b);
    addModels("spatial_entities@0x4489F0", g.spatial_entities);
    for (std::size_t i = 0; i < g.entities_424187.size(); ++i) addBase("entities_424187@0x448AF0", i, g.entities_424187[i], "no model");
    addModels("entities_42acac@0x448BD0", g.entities_42acac);
    addModels("entities_4347ec@0x448CB0", g.entities_4347ec);
    addModels("basic_4342bf@0x448D90", g.basic_4342bf);
    addModels("spatial_41e038@0x448E70", g.spatial_41e038);
    for (std::size_t i = 0; i < g.tagged_entities.size(); ++i) {
      const auto& t = g.tagged_entities[i];
      addBase("tagged@0x433DD9", i, t, std::format("tag={} model='{}'", t.type, modelName(t)));
    }
    for (std::size_t i = 0; i < g.named_entities.size(); ++i) {
      const auto& n = g.named_entities[i];
      addBase("named", i, n, std::format("{}/{} model='{}'", n.type, n.subclass_name, modelName(n)));
    }

    // ---- lever = tagged type 1 (case 1 of 0x00433DD9 -> ctor 0x00436C4E).
    std::vector<std::size_t> leverIdx;
    for (std::size_t i = 0; i < g.tagged_entities.size(); ++i)
      if (g.tagged_entities[i].type == 1) leverIdx.push_back(i);

    // ---- clone view of the same level (what the production loader keeps).
    std::vector<webo::PlacedModel> locator;
    auto objs = webo::findLevelObjects(*blob);
    if (objs)
      for (const auto& p : objs->placed)
        if (p.kind == webo::PlacedKind::Lever) locator.push_back(p);
    std::vector<webo::sim::Lever> cloneLevers;
    bool cloneLoaded = false;
    {
      auto rl = webo::sim::loadRetailLevel(*lib, name);
      if (rl) { cloneLevers = rl->levers; cloneLoaded = true; }
    }
    const bool isPrimary = name.find(kPrimaryLevel) != std::string::npos;
    if (isPrimary) primaryFile = name;

    if (!leverIdx.empty()) ++c.levelsWithLever;
    census << std::format("LEVEL {} tagged={} (type1/lever-class={}) locator.lever.a3d={} cloneLevers={}{}\n", name,
                          g.tagged_entities.size(), leverIdx.size(), locator.size(), cloneLevers.size(),
                          cloneLoaded ? "" : " (clone load failed)");
    if (leverIdx.size() != locator.size())
      census << "  NOTE: type-1 record count != lever.a3d locator count\n";

    std::optional<webo::CollisionGrid> storedGrid;
    if (isPrimary) {
      auto cg = webo::findCollisionGrid(*blob);
      if (cg) storedGrid = std::move(*cg);
      else fail(name + ": collision grid not found for stored-grid baseline");
    }
    std::ostringstream* P = isPrimary ? &primary : nullptr;
    if (P) {
      const webo::Box::Entry* pe = lib->patch() ? lib->patch()->find(name) : nullptr;
      const bool fromPatch = pe != nullptr;
      const webo::Box::Entry* be = lib->base().find(name);
      const webo::Box::Entry* used = fromPatch ? pe : be;
      *P << std::format("member: {} (resolved via {}; base has it: {}, patch has it: {})\n", used ? used->name : name,
                        fromPatch ? "patch.box" : "lemmings.box", be != nullptr, fromPatch);
      if (used)
        *P << std::format("  archive={} entry offset={} (0x{:x}) size={} (0x{:x})\n",
                          (fromPatch ? lib->patch()->path() : lib->base().path()).filename().string(), used->offset,
                          used->offset, used->size, used->size);
      *P << std::format("  member bytes read={} FNV-1a64=0x{:016x}\n", raw.size(), fnv1a(raw));
      *P << std::format("  structured reader consumed {} bytes; tagged={} named={} total actors={}\n",
                        level.consumed_bytes, g.tagged_entities.size(), g.named_entities.size(), [&] {
                          std::size_t n = 0;
                          for (auto& [i, k] : idCount) n += static_cast<std::size_t>(k);
                          return n;
                        }());
      *P << std::format("  distinct actor ids={} ids shared by >1 actor={}\n", byId.size(), [&] {
        std::size_t n = 0;
        for (auto& [i, k] : idCount) n += k > 1;
        return n;
      }());
    }

    // ---- tie each type-1 record to a clone lever by position, never by file order.
    // leverTriggerFromPose gives box.x = pose.x - 1 and box.y = H - 1 - (pose.y + 6)
    // (LevelObjects.cpp speedPadTriggerFromPose, H = collision mask height). The stream record
    // carries the same pose as doubles (+8,+10,+18), so the pair is accepted only when both
    // coordinates agree exactly under that transform.
    int maskH = 0;
    {
      auto rl = webo::sim::loadRetailLevel(*lib, name);
      if (rl) maskH = rl->terrain.height();
    }
    std::vector<int> locatorOf(leverIdx.size(), -1);
    for (std::size_t k = 0; k < leverIdx.size(); ++k) {
      const auto& t = g.tagged_entities[leverIdx[k]];
      const double rx = std::bit_cast<double>(t.double_bits[0]);
      const double ry = std::bit_cast<double>(t.double_bits[1]);
      int found = -1, hits = 0;
      for (std::size_t j = 0; j < cloneLevers.size(); ++j)
        if (std::abs(cloneLevers[j].x - (rx - 1.0)) < 1e-9 &&
            std::abs(cloneLevers[j].y - (static_cast<double>(maskH) - 1.0 - (ry + 6.0))) < 1e-9) {
          found = static_cast<int>(j);
          ++hits;
        }
      if (hits == 1) locatorOf[k] = found;
    }
    // search window for each lever's tail: from its record start to the next lever record start
    std::vector<std::size_t> recStarts;
    for (std::size_t k = 0; k < leverIdx.size(); ++k) {
      const auto& t = g.tagged_entities[leverIdx[k]];
      std::size_t start = SIZE_MAX;
      std::size_t n = 0;
      for (auto h : findAll(raw, basePattern(t)))
        if (h >= 4 && rd32(raw, h - 4) == 1u) { start = h - 4; ++n; }
      recStarts.push_back(n == 1 ? start : SIZE_MAX);
    }

    for (std::size_t k = 0; k < leverIdx.size(); ++k) {
      const std::size_t ti = leverIdx[k];
      const auto& t = g.tagged_entities[ti];
      ++c.levers;
      std::ostringstream L;
      const double rx = std::bit_cast<double>(t.double_bits[0]);
      const double ry = std::bit_cast<double>(t.double_bits[1]);
      const double rz = std::bit_cast<double>(t.double_bits[2]);
      L << std::format("  LEVER stream#{} tagged[{}] id={} (raw int16 0x{:04x}) model='{}' pos=({:.4f},{:.4f},{:.4f})\n", k, ti,
                       t.id, static_cast<std::uint16_t>(t.id), modelName(t), rx, ry, rz);
      if (locatorOf[k] >= 0 && static_cast<std::size_t>(locatorOf[k]) < cloneLevers.size()) {
        const auto& cl = cloneLevers[static_cast<std::size_t>(locatorOf[k])];
        L << std::format("    clone Lever[{}] (matched by exact pose, not file order) box x={} y={} {}x{} state={}\n", locatorOf[k],
                         cl.x, cl.y, cl.width, cl.height, cl.state);
      } else {
        L << "    clone Lever: NO exact position match under the pose transform\n";
        if (isPrimary) fail(name + ": lever record not paired with a clone Lever by position");
      }

      // raw-byte location, independent of the structured reader.
      std::string rawNote;
      const auto baseHits = findAll(raw, basePattern(t));
      std::vector<std::size_t> recHits;
      for (auto h : baseHits)
        if (h >= 4 && rd32(raw, h - 4) == 1u) recHits.push_back(h - 4);
      if (recHits.size() != 1) {
        rawNote = std::format("raw location AMBIGUOUS/ABSENT: base-pattern hits={} with preceding type word 1 hits={}", baseHits.size(), recHits.size());
        if (isPrimary) fail(std::string(name) + ": lever raw location not unique");
      } else {
        const std::size_t rec = recHits[0];
        // tail after the spatial block: double +0x98, flag +0xb0, flag +0xa0, u32 +0xdc, list+0xbc, list+0xcc
        Pat tail;
        tail.le(t.double_98, 8);
        tail.any();
        tail.any();
        tail.le(t.values[0], 4);
        tail.le(t.record_arrays[0].size(), 4);
        for (const auto& r : t.record_arrays[0]) for (auto w : r) tail.le(w, 4);
        tail.le(t.record_arrays[1].size(), 4);
        for (const auto& r : t.record_arrays[1]) for (auto w : r) tail.le(w, 4);
        // Window: this record's start up to the next lever record's start (or 256 KiB). The embedded
        // model sits before the tail, so the tail is the LAST hit in the window.
        std::size_t windowEnd = rec + 0x40000;
        for (auto s : recStarts)
          if (s != SIZE_MAX && s > rec && s < windowEnd) windowEnd = s;
        const auto tailHits = findAll(raw, tail, rec, windowEnd);
        if (tailHits.empty()) {
          rawNote = std::format("record starts at +0x{:x}; tail pattern not found before +0x{:x}", rec, windowEnd);
          if (isPrimary) fail("lever tail not located");
        } else {
          const std::size_t o = tailHits.back();
          rawNote = std::format("record(type word)@+0x{:x}  base id int16@+0x{:x}  tail@+0x{:x} (last of {} hit(s) in window to +0x{:x})", rec,
                                rec + 4 + 42, o, tailHits.size(), windowEnd);
          L << "    raw: " << rawNote << "\n";
          L << std::format("    raw: cooldown double(+0x98)@+0x{:x}=bits 0x{:016x} ({} )  flag(+0xb0)@+0x{:x}={}  flag(+0xa0)@+0x{:x}={}\n", o,
                           t.double_98, std::bit_cast<double>(t.double_98), o + 8, static_cast<int>(raw[o + 8]), o + 9,
                           static_cast<int>(raw[o + 9]));
          L << std::format("    raw: init word(+0xdc)@+0x{:x}=0x{:08x} (int {})\n", o + 10, t.values[0],
                           static_cast<std::int32_t>(t.values[0]));
          std::size_t cur = o + 14;
          for (int li = 0; li < 2; ++li) {
            const auto& list = t.record_arrays[static_cast<std::size_t>(li)];
            L << std::format("    list{} ({}) count@+0x{:x}={} (raw u32 0x{:08x})\n", li,
                             li == 0 ? "object+0xBC, dispatched when pull sets +0xB8=-1 (face -1)"
                                     : "object+0xCC, dispatched when pull sets +0xB8=+1 (face +1)",
                             cur, list.size(), rd32(raw, cur));
            cur += 4;
            for (std::size_t r = 0; r < list.size(); ++r) {
              const auto& m = list[r];
              const int tid = static_cast<std::int32_t>(m[0]);
              L << std::format("      rec{} @+0x{:x}: raw words [target id 0x{:08x}={}, msg type 0x{:08x}={}, p1 0x{:08x}, p2 0x{:08x}]", r, cur,
                               m[0], tid, m[1], m[1], m[2], m[3]);
              ++c.totalRecords;
              ++c.msgTypes[m[1]];
              if (tid < 0) {
                ++c.negativeIds;
                L << "  -> negative id (special arm of 0x00433B2E)\n";
              } else if (auto it = byId.find(tid); it != byId.end()) {
                const auto& tg = it->second;
                L << std::format("  -> {}[{}] {}{}\n", tg.collection, tg.index, tg.detail,
                                 idCount[tid] > 1 ? std::format(" (id shared by {} actors, last in stream order wins)", idCount[tid]) : "");
                ++c.targetClass[tg.collection + " " + tg.detail.substr(0, tg.detail.find(" model="))];
                if (tg.collection.rfind("entities_42acac", 0) == 0 && tg.index < g.entities_42acac.size()) {
                  // Receiver class vtable 0x004B1020 (ctor 0x00428680, reader 0x0042ACAC): type 4 -> [+0x18c]=0,
                  // type 5 -> [+0x18c]=1. Stream fields as the reader stores them; AABB x=[+0x48,+0x60), y=[+0x50,+0x68).
                  const auto& e = g.entities_42acac[tg.index];
                  const auto d = [](std::uint64_t b) { return std::bit_cast<double>(b); };
                  const double d98 = d(e.doubles_98_1a8_a0[0]), da0 = d(e.doubles_98_1a8_a0[2]);
                  const double minx = d(e.double_bits_48_60[0]), miny = d(e.double_bits_48_60[1]);
                  const double maxx = d(e.double_bits_48_60[3]), maxy = d(e.double_bits_48_60[4]);
                  const std::uint32_t initMode = e.values_18c_128_168[0], kind = e.values_18c_128_168[1];
                  const double h0 = initMode == 1 ? da0 : d98;  // reader: 1a8 := (mode==1 ? [+0xa0] : [+0x98])
                  L << std::format(
                      "         receiver 0x4B1020: init mode[+0x18c]={} kind[+0x128]={} (0->zone bit 0x200, 1->0x2000) [+0x168]={} "
                      "high[+0x98]={} streamH[+0x1a8]={} low[+0xa0]={}\n"
                      "         AABB min=({},{}) max=({},{}) initial height={} -> flagged x[{},{}) y[{},{}) {}; after type 4: height {} ; after type 5: height {} (zone emptied)\n",
                      initMode, kind, e.values_18c_128_168[2], d98, d(e.doubles_98_1a8_a0[1]), da0, minx, miny, maxx, maxy, h0,
                      static_cast<int>(minx), static_cast<int>(maxx), static_cast<int>(miny), static_cast<int>(miny + h0),
                      initMode == 1 ? "(none: drained at load, bit cleared)" : "(bit set at load)", d98, da0);
                  // Stored-grid baseline (what the clone copies at load, Terrain.cpp fromCollisionGrid): cells
                  // carrying the zone bits over this gate's x range and y rows [miny, ftol(miny+high)].
                  if (storedGrid) {
                    const int x0 = static_cast<int>(minx), x1 = static_cast<int>(maxx);
                    const int y0 = static_cast<int>(miny), y1 = static_cast<int>(miny + d98);
                    std::size_t cells = 0, z200 = 0, z2000 = 0, z200ExclTop = 0;
                    for (int yy = y0; yy <= y1; ++yy)
                      for (int xx = x0; xx < x1; ++xx) {
                        if (xx < 0 || xx >= storedGrid->width || yy < 0 || yy >= storedGrid->height) continue;
                        const std::uint32_t w = storedGrid->at(xx, yy);
                        ++cells;
                        if (w & 0x200u) { ++z200; if (yy < y1) ++z200ExclTop; }
                        if (w & 0x2000u) ++z2000;
                      }
                    L << std::format(
                        "         stored grid x[{},{}) y[{}..{}] inclusive: cells={} with 0x200={} (rows y<{}: {}) with 0x2000={}\n", x0,
                        x1, y0, y1, cells, z200, y1, z200ExclTop, z2000);
                  }
                }
              } else {
                ++c.unresolvedRecords;
                L << "  -> NO actor carries this id (list slot empty/0x00443ABD; 0x00433B2E skips)\n";
              }
              cur += 16;
            }
          }
          c.initWord[static_cast<std::int32_t>(t.values[0])]++;
        }
      }
      if (rawNote.empty() || rawNote.rfind("raw location", 0) == 0 || rawNote.rfind("record starts", 0) == 0)
        L << "    raw: " << rawNote << "\n";
      if (t.record_arrays[0].size() != t.record_arrays[1].size()) ++c.listLenMismatch;
      if (t.record_arrays[0].empty() && t.record_arrays[1].empty()) ++c.bothListsEmpty[name];
      else ++c.leversWithAnyRecord;
      if (t.flag_a0) ++c.flagA0Set;
      if (t.flag_b0) ++c.flagB0Set;
      {
        std::vector<int> a, b;
        for (auto& r : t.record_arrays[0]) a.push_back(static_cast<int>(r[0]));
        for (auto& r : t.record_arrays[1]) b.push_back(static_cast<int>(r[0]));
        std::sort(a.begin(), a.end());
        std::sort(b.begin(), b.end());
        if (a == b && !a.empty()) ++c.sameTargetsBothLists;
      }
      if (!isLeverName(modelName(t))) L << "    NOTE: model name does not contain 'lever'\n";
      census << L.str();
      if (P) *P << L.str();
    }
    if (isPrimary) {
      if (leverIdx.size() != 4) fail("MM_W_012 expected 4 type-1 lever records");
      if (cloneLevers.size() != 4) fail("MM_W_012 expected 4 clone levers");
    }
  }

  if (primaryFile.empty()) fail("MM_W_012 not found in archives");

  census << "\n=== CENSUS ===\n";
  census << std::format("levels={} levelsWithLever={} levers(type-1)={} leversWithAnyRecord={} totalRecords={} unresolved={} negativeIds={}\n",
                        c.levels, c.levelsWithLever, c.levers, c.leversWithAnyRecord, c.totalRecords, c.unresolvedRecords, c.negativeIds);
  census << std::format("lists of unequal length={} levers whose two lists carry the same target-id multiset={} flag(+0xa0) set={} flag(+0xb0) set={}\n",
                        c.listLenMismatch, c.sameTargetsBothLists, c.flagA0Set, c.flagB0Set);
  for (auto& [k, v] : c.msgTypes) census << std::format("message type {} x{}\n", k, v);
  for (auto& [k, v] : c.initWord) census << std::format("init word {} x{}\n", k, v);
  for (auto& [k, v] : c.targetClass) census << std::format("target class [{}] x{}\n", k, v);
  for (auto& [k, v] : c.bothListsEmpty) census << std::format("levers with both lists empty: {} x{}\n", k, v);

  std::cout << "=== PRIMARY " << kPrimaryLevel << " ===\n" << primary.str() << census.str().substr(census.str().find("\n=== CENSUS ==="));
  std::error_code ec;
  fs::create_directories("lever-links-research-001", ec);
  std::ofstream("lever-links-research-001/primary-MM_W_012.txt") << primary.str();
  std::ofstream("lever-links-research-001/census.txt") << census.str();
  std::cout << std::format("failures={}\n", failures);
  return failures == 0 ? 0 : 1;
}
