// lever-init-research-001: read-only measurement of the exact cell traversal of the
// original rect painters 0x004311D0 (set) / 0x0043127E (clear) over the installed
// MM_W_012 gate rectangles. Source-built; the original executable is never run and
// no original bytes are executed.
//
// The traversal below is written from the instruction trace in
// build/lever-init-research-001/static-trace.md, not from the earlier 200-cell
// geometric enumeration:
//   * every double -> int conversion is the helper 0x0049C5AC: FSTCW, RC |= 0xC00,
//     FISTP (truncate toward zero), restore -- i.e. a C++ static_cast<int>(double);
//   * x loop: x from ftol(min.x) while x < ftol(max.x)  (JGE exit: upper bound exclusive);
//     the SET variant adds 0x100 to ftol(max.x) when (double)ftol(max.x) < min.x; CLEAR has no wrap;
//   * y loop: y from ftol(min.y) while y < ftol(max.y)  (exclusive; bound re-evaluated per step);
//   * cell lookup 0x0040DE60: x &= 0xFF; y clamped to [0, height-1]; element x + (y << 8).
// The reader 0x0042ACAC runs the CLEAR variant (init request == 1) or the SET variant
// (otherwise) over the stream AABB as stored (+0x50 .. +0x68 is NOT recomputed first).
// The settled endpoint of a type-4 message is the SET variant over y max = min.y + high.
//
// It asserts only measurement integrity. It does not assert in-game behavior.
#include "assets/Box.h"
#include "assets/CollisionGrid.h"
#include "assets/LevelIndex.h"
#include "lemmings/compiled_level.hpp"

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
#include <stdexcept>
#include <string>
#include <vector>

namespace fs = std::filesystem;

namespace {

constexpr const char* kPrimaryLevel = "MM_W_012";
constexpr std::uint32_t kZoneBit0 = 0x200;  // gate kind 0
constexpr std::uint32_t kZoneBit1 = 0x2000;  // gate kind 1

struct Rect {
  double minx, miny, maxx, maxy;
};

// 0x0049C5AC: truncate toward zero.
int ftol(double v) { return static_cast<int>(v); }

// Cells visited by the painter loops, in visit order. cellIndex is the 0x0040DE60 element index.
struct Visit {
  int x, y;           // loop counters (x before the mask, y before the clamp)
  std::size_t index;  // (x & 0xFF) + (clamp(y) << 8)
};

std::vector<Visit> traverse(const Rect& r, bool setVariant, int gridHeight) {
  std::vector<Visit> out;
  int xmax = ftol(r.maxx);
  if (setVariant && static_cast<double>(xmax) < r.minx) xmax += 0x100;  // 0x4311F1..0x431207
  for (int x = ftol(r.minx); x < xmax; ++x) {
    for (int y = ftol(r.miny); y < ftol(r.maxy); ++y) {
      int cy = y >= gridHeight ? gridHeight - 1 : y;
      if (cy < 0) cy = 0;
      out.push_back({x, y, static_cast<std::size_t>(x & 0xFF) + (static_cast<std::size_t>(cy) << 8)});
    }
  }
  return out;
}

struct GateRow {
  std::size_t index = 0;
  int id = 0;
  std::uint32_t request = 0, kind = 0;
  Rect rect{};
  double high = 0, low = 0;
  bool targeted = false;
  std::size_t visitedInit = 0, flaggedInit = 0, changedInit = 0;
  std::size_t visitedSettled = 0, flaggedAtSettled = 0, storedFlaggedInColumns = 0, storedFlaggedOutsideSettled = 0;
  std::size_t inclusiveGeometry = 0;
  int rowsFirst = 0, rowsLast = 0, colsFirst = 0, colsLast = 0;
};

}  // namespace

int main() {
  const char* dir = std::getenv("WEBOLUTION_GAME_DIR");
  if (!dir) {
    std::cout << "lever init measurement: SKIP (WEBOLUTION_GAME_DIR unset)\n";
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

  std::string name;
  for (const auto& n : webo::levelFileNames(*lib))
    if (n.find(kPrimaryLevel) != std::string::npos) name = n;
  if (name.empty()) {
    fail("MM_W_012 not found in archives");
    return 1;
  }
  auto blob = lib->read(name);
  if (!blob) {
    fail("cannot read " + name);
    return 1;
  }
  const std::vector<std::uint8_t> raw(reinterpret_cast<const std::uint8_t*>(blob->data()),
                                      reinterpret_cast<const std::uint8_t*>(blob->data()) + blob->size());
  lemmings::CompiledLevel level;
  try {
    level = lemmings::read_compiled_level(std::span<const std::uint8_t>(raw.data(), raw.size()), false);
  } catch (const std::invalid_argument& e) {
    fail(std::string("structured reader rejected level: ") + e.what());
    return 1;
  }
  auto cg = webo::findCollisionGrid(*blob);
  if (!cg) {
    fail("collision grid not found");
    return 1;
  }
  const webo::CollisionGrid& grid = *cg;
  if (grid.width != 256 || grid.height <= 0 || grid.cells.size() != static_cast<std::size_t>(256) * grid.height)
    fail("unexpected grid shape");

  const auto& g = level.gadgets;
  const auto dbl = [](std::uint64_t b) { return std::bit_cast<double>(b); };

  // Lever-targeted gate ids and, for the observed lever, its own face -1 list (ids 5 and 6 per lever-links-research-001).
  std::set<int> targetedIds;
  for (const auto& t : g.tagged_entities) {
    if (t.type != 1) continue;
    for (const auto& list : t.record_arrays)
      for (const auto& rec : list)
        if (static_cast<std::int32_t>(rec[0]) >= 0) targetedIds.insert(static_cast<std::int32_t>(rec[0]));
  }

  std::vector<GateRow> rows;
  for (std::size_t i = 0; i < g.entities_42acac.size(); ++i) {
    const auto& e = g.entities_42acac[i];
    GateRow r;
    r.index = i;
    r.id = e.id;
    r.request = e.values_18c_128_168[0];
    r.kind = e.values_18c_128_168[1];
    r.rect = {dbl(e.double_bits_48_60[0]), dbl(e.double_bits_48_60[1]), dbl(e.double_bits_48_60[3]),
              dbl(e.double_bits_48_60[4])};
    r.high = dbl(e.doubles_98_1a8_a0[0]);
    r.low = dbl(e.doubles_98_1a8_a0[2]);
    r.targeted = targetedIds.count(r.id) != 0;
    const std::uint32_t bit = r.kind == 0 ? kZoneBit0 : kZoneBit1;
    if (r.kind > 1) fail(std::format("gate {} has kind {} (reader paints only kinds 0/1)", i, r.kind));

    // Reader 0x42ACAC at load: request == 1 -> CLEAR over the stored AABB; else SET over the stored AABB.
    const bool clearAtInit = r.request == 1;
    const auto init = traverse(r.rect, !clearAtInit, grid.height);
    r.visitedInit = init.size();
    for (const auto& v : init) {
      const bool flagged = (grid.cells[v.index] & bit) != 0;
      if (flagged) ++r.flaggedInit;
      if (clearAtInit ? flagged : !flagged) ++r.changedInit;
    }
    if (!init.empty()) {
      r.colsFirst = init.front().x;
      r.colsLast = init.back().x;
      r.rowsFirst = init.front().y;
      r.rowsLast = init.front().y;
      for (const auto& v : init) r.rowsLast = std::max(r.rowsLast, v.y);
    }

    // Settled endpoint of a type-4 message (step 0x42B24D req 0): +0x68 = +0x50 + high, SET over the AABB.
    Rect settled = r.rect;
    settled.maxy = r.rect.miny + r.high;
    const auto end = traverse(settled, true, grid.height);
    r.visitedSettled = end.size();
    std::set<std::size_t> endSet;
    for (const auto& v : end) endSet.insert(v.index);
    r.flaggedAtSettled = endSet.size();  // every visited cell has the bit after a SET

    // Stored-grid comparison: bit over the gate's columns, all rows in the inclusive geometric window used by
    // lever-links-research-001 (rows ftol(min.y)..ftol(min.y+high) inclusive).
    const int y0 = ftol(r.rect.miny), y1 = ftol(r.rect.miny + r.high);
    for (int y = y0; y <= y1; ++y)
      for (int x = ftol(r.rect.minx); x < ftol(r.rect.maxx); ++x) {
        ++r.inclusiveGeometry;
        const std::size_t idx = static_cast<std::size_t>(x & 0xFF) + (static_cast<std::size_t>(y) << 8);
        if ((grid.cells[idx] & bit) != 0) {
          ++r.storedFlaggedInColumns;
          if (!endSet.count(idx)) ++r.storedFlaggedOutsideSettled;
        }
      }
    rows.push_back(r);
  }

  std::ostringstream out;
  out << std::format("member: {} bytes={} grid {}x{} cells; entities_42acac={} targeted_ids={}\n", name, raw.size(),
                     grid.width, grid.height, rows.size(), targetedIds.size());
  out << "Traversal (static-trace.md H): ftol=truncate; x/y upper bounds EXCLUSIVE; SET adds 0x100 on wrap, CLEAR does not; "
         "cell = (x&0xFF) + (clamp(y,0,H-1)<<8)\n";
  out << "Columns: idx id req kind rect(minx,miny,maxx,maxy) high low | init: cols[first..last] rows[first..last] "
         "visited flagged-before changed | settled(type4): visited flagged | stored-bit cells in inclusive window "
         "(rows y0..ftol(y0+high), 8 cols) / outside the settled set | targeted\n";
  std::size_t sumVisited = 0, sumChanged = 0, sumFlagged = 0, sumSettled = 0, sumInclusive = 0, sumStored = 0;
  std::size_t untargetedChanged = 0, untargetedVisited = 0;
  for (const auto& r : rows) {
    out << std::format(
        "gate[{}] id={} req={} kind={} rect=({},{},{},{}) high={} low={} | init cols[{}..{}] rows[{}..{}] visited={} "
        "flagged={} changed={} | settled visited={} flagged={} | inclusive-window cells={} stored-bit={} outside-settled={} | {}\n",
        r.index, r.id, r.request, r.kind, r.rect.minx, r.rect.miny, r.rect.maxx, r.rect.maxy, r.high, r.low, r.colsFirst,
        r.colsLast, r.rowsFirst, r.rowsLast, r.visitedInit, r.flaggedInit, r.changedInit, r.visitedSettled,
        r.flaggedAtSettled, r.inclusiveGeometry, r.storedFlaggedInColumns, r.storedFlaggedOutsideSettled,
        r.targeted ? "targeted" : "NOT targeted");
    if (r.targeted) {
      sumVisited += r.visitedInit;
      sumChanged += r.changedInit;
      sumFlagged += r.flaggedInit;
      sumSettled += r.visitedSettled;
      sumInclusive += r.inclusiveGeometry;
      sumStored += r.storedFlaggedInColumns;
    } else {
      untargetedVisited += r.visitedInit;
      untargetedChanged += r.changedInit;
    }
  }
  const std::size_t targetedCount = static_cast<std::size_t>(
      std::count_if(rows.begin(), rows.end(), [](const GateRow& r) { return r.targeted; }));
  out << std::format("TOTAL targeted gates={} init visited={} already-flagged={} changed={} | settled visited={} | "
                     "inclusive-window cells={} stored-bit={}\n",
                     targetedCount, sumVisited, sumFlagged, sumChanged, sumSettled, sumInclusive, sumStored);
  out << std::format("TOTAL untargeted gates={} init visited={} changed={}\n", rows.size() - targetedCount,
                     untargetedVisited, untargetedChanged);

  // Ids 5 and 6 (observed lever (183,60), face -1 list).
  std::size_t obsVisited = 0, obsChanged = 0, obsSettled = 0, obsFound = 0;
  for (const auto& r : rows)
    if (r.id == 5 || r.id == 6) {
      ++obsFound;
      obsVisited += r.visitedInit;
      obsChanged += r.changedInit;
      obsSettled += r.visitedSettled;
    }
  out << std::format("OBSERVED ids 5,6: gates found={} init visited={} changed={} settled(type4) cells={}\n", obsFound,
                     obsVisited, obsChanged, obsSettled);

  // Overlap and whole-grid context: do any two targeted/untargeted init rectangles share a cell, and how many
  // stored 0x200 / 0x2000 cells exist in the whole grid vs inside the union of init rectangles.
  std::set<std::size_t> unionCells;
  std::size_t visitsTotal = 0;
  for (const auto& r : rows) {
    for (const auto& v : traverse(r.rect, r.request != 1, grid.height)) {
      unionCells.insert(v.index);
      ++visitsTotal;
    }
  }
  std::size_t stored200 = 0, stored2000 = 0, stored200InUnion = 0;
  for (std::size_t i = 0; i < grid.cells.size(); ++i) {
    if (grid.cells[i] & kZoneBit0) {
      ++stored200;
      if (unionCells.count(i)) ++stored200InUnion;
    }
    if (grid.cells[i] & kZoneBit1) ++stored2000;
  }
  out << std::format("OVERLAP init visits={} unique cells={} (overlap {}); whole stored grid: 0x200 cells={} (inside union {}), 0x2000 cells={}\n",
                     visitsTotal, unionCells.size(), visitsTotal - unionCells.size(), stored200, stored200InUnion,
                     stored2000);

  // Counter-check of the earlier prediction arithmetic against the traced traversal.
  const std::size_t inclusivePrediction = targetedCount * 200;
  out << std::format("PREDICTIONS: inherited 3072 (=16*192) vs traced changed={} ; inherited 384 (=2*192) vs traced ids5/6 settled={} ; "
                     "geometric-inclusive enumeration 200/gate -> {} for {} gates (NOT visited by the binary)\n",
                     sumChanged, obsSettled, inclusivePrediction, targetedCount);

  // Integrity assertions only (measurement validity, not game behavior).
  if (rows.empty()) fail("no entities_42acac gates");
  if (targetedCount != 16) fail(std::format("expected 16 lever-targeted gates in {}, found {}", kPrimaryLevel, targetedCount));
  if (obsFound != 2) fail("lever (183,60) targets ids 5 and 6: expected both gates");
  for (const auto& r : rows)
    if (r.visitedInit == 0 && r.targeted) fail(std::format("gate {} visited no cells", r.index));

  std::error_code ec;
  fs::create_directories("lever-init-research-001", ec);
  std::ofstream("lever-init-research-001/gate-cells-MM_W_012.txt") << out.str();
  std::cout << out.str();
  std::cout << (failures ? "lever init measurement: FAIL\n" : "lever init measurement: ok\n");
  return failures ? 1 : 0;
}
