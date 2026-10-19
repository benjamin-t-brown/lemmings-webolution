#include "CellMesh.h"

#include <algorithm>
#include <cstring>
#include <format>

namespace webo {
namespace {

constexpr std::uint32_t kSolid = 0x0001;
constexpr std::uint32_t kDoor = 0x0400;
constexpr std::uint32_t kSolidOrDoor = kSolid | kDoor;

constexpr int kBuckets = 32;
constexpr int kBandsPerLevel = 6;
constexpr int kColumnsPerBucket = 8;
constexpr int kRowsPerBand = 16;

constexpr double kUvNudge = 1.0 / 512.0;    // 0.001953125
constexpr double kUvNudge128 = 1.0 / 128.0; // 0.0078125

// FUN_004136E0 / FUN_00413710 over the table FUN_00439D60 fills with i/64.
double uvU(int x) { return (x & 31) / 32.0; }
double uvV(int y) { return 1.0 - (y & 31) / 32.0; }

// The three colour dwords the engine writes are (0, hi) repeated three times,
// so the quad carries three copies of one double. FUN_004608BB sets
// entry+0xD8..+0xEC; the pairs are (0.0f, hi) for every site.
double colourFromHi(std::uint32_t hi) {
  const std::uint64_t bits = (static_cast<std::uint64_t>(hi) << 32);
  double d = 0;
  std::memcpy(&d, &bits, sizeof(d));
  return d;
}

struct Part {
  int y_start = 0;
  int y_end = 0;
  bool merged = false;
  bool door = false;
};

struct Record {
  int y_start = 0;
  int y_end = 0;
  int x0 = 0;
  int x1 = 0;
  bool door = false;
  int r4 = 0, r5 = 0, r6 = 0, r7 = 0, r8 = 0, r9 = 0;
};

struct Gap {
  int x = 0;   // the boundary column the gap sits on (or the row, for a top gap)
  int start = 0;
  int end = 0;
  bool door = false;
};

class Builder {
 public:
  Builder(std::span<const std::uint32_t> table, const CollisionGrid& grid)
      : table_(table), grid_(grid) {
    touched_.assign(table.size(), 0);
    for (std::size_t i = 0; i < table.size(); ++i)
      if (table[i] != 0xFFFFFFFFu)
        ++keySlots_;
  }

  std::uint32_t cell(int x, int y) const {
    x &= 0xFF;
    if (grid_.height > 0) {
      if (y >= grid_.height)
        y = grid_.height - 1;
      if (y < 0)
        y = 0;
    } else {
      y = 0;
    }
    const std::size_t idx = static_cast<std::size_t>(y) * CollisionGrid::kWidth + x;
    return idx < grid_.cells.size() ? grid_.cells[idx] : 0;
  }

  bool solidNoDoor(int x, int y) const {
    const std::uint32_t c = cell(x, y);
    return (c & kSolid) != 0 && (c & kDoor) == 0;
  }

  // FUN_0043A31D: key `layer + (x + y*0x101)*2`.
  std::uint32_t vertex(int x, int y, int layer) {
    const std::size_t key = static_cast<std::size_t>(layer) + (x + y * 0x101) * 2;
    if (key < touched_.size())
      touched_[key] = 1;
    if (key < table_.size() && table_[key] != 0xFFFFFFFFu)
      return table_[key];
    ++unresolved_;
    return 0;
  }

  // FUN_0043A1C6: the (x, y) record list at world+0x43C. Empty in retail.
  std::uint32_t vertexDoor(int x, int y) {
    for (const auto& rec : records_)
      if (rec[0] == x && rec[1] == y)
        return static_cast<std::uint32_t>(rec[2]);
    ++unresolved_;
    return 0;
  }

  std::vector<Record> partLists(int x0, int y0) const {
    std::vector<std::vector<Part>> lists(8);
    for (int r = 0; r < 8; ++r) {
      int c = 0;
      while (c < 0x10) {
        while (c < 0x10 && (cell(x0 + r, y0 + c) & kSolidOrDoor) == 0)
          ++c;
        if (c < 0x10) {
          Part p;
          p.y_start = y0 + c;
          p.door = (cell(x0 + r, y0 + c) & kDoor) != 0;
          if (!p.door) {
            for (;;) {
              ++c;
              if (c > 0xF)
                break;
              const std::uint32_t cc = cell(x0 + r, y0 + c);
              if ((cc & kSolid) == 0 || (cc & kDoor) != 0)
                break;
            }
          } else {
            for (;;) {
              ++c;
              if (c > 0xF)
                break;
              if ((cell(x0 + r, y0 + c) & kDoor) == 0)
                break;
            }
          }
          p.y_end = y0 - 1 + c;
          lists[r].push_back(p);
        }
      }
    }

    std::vector<Record> recs;
    for (int r = 0; r < 8; ++r) {
      for (std::size_t i = 0; i < lists[r].size(); ++i) {
        const Part& rec = lists[r][i];
        int r2 = r;
        if (!rec.merged) {
          if (r + 1 < 8) {
            std::size_t j = 0;
            for (;;) {
              ++r2;
              if (r2 < 8) {
                j = 0;
                auto& later = lists[r2];
                while (j < later.size() && !(later[j].y_start == rec.y_start &&
                                             later[j].y_end == rec.y_end &&
                                             later[j].door == rec.door))
                  ++j;
                if (j < later.size())
                  later[j].merged = true;
              }
              const std::size_t cnt = r2 < 8 ? lists[r2].size() : 0;
              if (!(j < cnt && r2 < 8))
                break;
            }
            --r2;
          }
          Record out;
          out.y_start = rec.y_start;
          out.y_end = rec.y_end;
          out.x0 = x0 + r;
          out.x1 = x0 + r2;
          out.door = rec.door;
          recs.push_back(out);
        }
      }
    }
    return recs;
  }

  void annotate(std::vector<Record>& recs, std::vector<Gap>& left,
                std::vector<Gap>& right, std::vector<Gap>& top) const {
    for (auto& rec : recs) {
      rec.r4 = static_cast<int>(left.size());
      if (rec.x0 >= 0)
        sideGaps(rec, rec.x0, (rec.x0 - 1) & 0xFF, left);
      rec.r5 = static_cast<int>(left.size());
      rec.r6 = static_cast<int>(right.size());
      if (rec.x1 < 0x100)
        sideGaps(rec, rec.x1, (rec.x1 + 1) & 0xFF, right);
      rec.r7 = static_cast<int>(right.size());
      rec.r8 = static_cast<int>(top.size());
      if (rec.y_end < grid_.height - 1)
        topGaps(rec, top);
      rec.r9 = static_cast<int>(top.size());
    }
  }

  std::vector<CellMeshQuad> mesh0(const std::vector<Record>& recs,
                                  const std::vector<Gap>& right,
                                  const std::vector<Gap>& top) {
    std::vector<CellMeshQuad> quads;
    for (const auto& rec : recs) {
      quads.push_back(topQuad(rec));
      for (int k = rec.r6; k < rec.r7; ++k) {
        const Gap& g = right[k];
        const double u = uvU(g.x + 1) == 0.0 ? 1.0 : uvU(g.x + 1);
        const double v0 = uvV(g.start);
        double v1 = uvV(g.end + 1);
        if (v1 == 1.0)
          v1 = 0.0;
        CellMeshQuad q;
        if (!g.door) {
          q.index[0] = vertex(g.x + 1, g.start, 1);
          q.index[1] = vertex(g.x + 1, g.end + 1, 1);
        } else {
          q.index[0] = vertexDoor(g.x + 1, g.start);
          q.index[1] = vertexDoor(g.x + 1, g.end + 1);
        }
        q.index[2] = vertex(g.x + 1, g.end + 1, 0);
        q.index[3] = vertex(g.x + 1, g.start, 0);
        q.uv = {{{u - kUvNudge, v0 - kUvNudge}, {u - kUvNudge, v1 + kUvNudge},
                 {u - kUvNudge, v1 + kUvNudge}, {u - kUvNudge, v0 - kUvNudge}}};
        q.colour = colourFromHi(0x4064B800u);
        q.coord = rec.x0;
        quads.push_back(q);
      }
      for (int k = rec.r8; k < rec.r9; ++k) {
        const Gap& g = top[k];
        const double u0 = uvU(g.start);
        const double u1 = uvU(g.end + 1) == 0.0 ? 1.0 : uvU(g.end + 1);
        double v = uvV(g.x) - kUvNudge128;
        if (v == 1.0)
          v = 0.0;
        CellMeshQuad q;
        if (!g.door) {
          q.index[0] = vertex(g.start, g.x + 1, 1);
          q.index[3] = vertex(g.end + 1, g.x + 1, 1);
        } else {
          q.index[0] = vertexDoor(g.start, g.x + 1);
          q.index[3] = vertexDoor(g.end + 1, g.x + 1);
        }
        q.index[1] = vertex(g.start, g.x + 1, 0);
        q.index[2] = vertex(g.end + 1, g.x + 1, 0);
        q.uv = {{{u0 + kUvNudge, v - kUvNudge}, {u0 + kUvNudge, v - kUvNudge},
                 {u1 - kUvNudge, v - kUvNudge}, {u1 - kUvNudge, v - kUvNudge}}};
        q.colour = colourFromHi(0x406FE000u);
        q.coord = rec.x0;
        quads.push_back(q);
      }
    }
    return quads;
  }

  std::vector<CellMeshQuad> mesh4(const std::vector<Record>& recs,
                                  const std::vector<Gap>& left,
                                  const std::vector<Gap>& top) {
    const int n = static_cast<int>(recs.size());
    std::vector<int> order(static_cast<std::size_t>(n));
    for (int i = 0; i < n; ++i)
      order[i] = n - 1 - i;
    // FUN_00463410 0x463F9B..0x4640E2: a selection sort over the record
    // pointers keyed on x1 descending, then y_start ascending.
    for (int a = 0; a < n - 1; ++a) {
      for (int b = a + 1; b < n; ++b) {
        const Record& ra = recs[order[a]];
        const Record& rb = recs[order[b]];
        if (rb.x1 > ra.x1 || (rb.x1 == ra.x1 && rb.y_start < ra.y_start))
          std::swap(order[a], order[b]);
      }
    }

    std::vector<CellMeshQuad> quads;
    for (int oi : order) {
      const Record& rec = recs[oi];
      quads.push_back(topQuad(rec));
      for (int k = rec.r4; k < rec.r5; ++k) {
        const Gap& g = left[k];
        const double u = uvU(g.x);
        const double v0 = uvV(g.start);
        double v1 = uvV(g.end + 1);
        if (v1 == 1.0)
          v1 = 0.0;
        CellMeshQuad q;
        if (!g.door) {
          q.index[2] = vertex(g.x, g.end + 1, 1);
          q.index[3] = vertex(g.x, g.start, 1);
        } else {
          q.index[2] = vertexDoor(g.x, g.end + 1);
          q.index[3] = vertexDoor(g.x, g.start);
        }
        q.index[0] = vertex(g.x, g.start, 0);
        q.index[1] = vertex(g.x, g.end + 1, 0);
        q.uv = {{{u + kUvNudge, v0 - kUvNudge}, {u + kUvNudge, v1 + kUvNudge},
                 {u + kUvNudge, v1 + kUvNudge}, {u + kUvNudge, v0 - kUvNudge}}};
        q.colour = colourFromHi(0x406B1800u);
        q.coord = rec.x0;
        quads.push_back(q);
      }
      for (int k = rec.r8; k < rec.r9; ++k) {
        const Gap& g = top[k];
        const double u0 = uvU(g.start);
        const double u1 = uvU(g.end + 1) == 0.0 ? 1.0 : uvU(g.end + 1);
        double v = uvV(g.x) - kUvNudge128;
        if (v == 1.0)
          v = 0.0;
        CellMeshQuad q;
        if (!g.door) {
          q.index[0] = vertex(g.start, g.x + 1, 1);
          q.index[3] = vertex(g.end + 1, g.x + 1, 1);
        } else {
          q.index[0] = vertexDoor(g.start, g.x + 1);
          q.index[3] = vertexDoor(g.end + 1, g.x + 1);
        }
        q.index[1] = vertex(g.start, g.x + 1, 0);
        q.index[2] = vertex(g.end + 1, g.x + 1, 0);
        q.uv = {{{u0 + kUvNudge, v - kUvNudge}, {u0 + kUvNudge, v - kUvNudge},
                 {u1 - kUvNudge, v - kUvNudge}, {u1 - kUvNudge, v - kUvNudge}}};
        q.colour = colourFromHi(0x406FE000u);
        q.coord = rec.x0;
        quads.push_back(q);
      }
    }
    return quads;
  }

  // FUN_00463100: the whole-block quad at entry+8, submitted by the INN pass.
  // Four layer-0 vertices over the whole 8x16 block in the open-top order, the
  // open-top UV formula over the same extent, and a white modulate. The four
  // keys are the ones `buildCellMeshes` already touches for the wall flag, so
  // this adds no new key support.
  CellMeshQuad wholeBlockQuad(int x0, int y0) {
    CellMeshQuad q;
    q.index[0] = vertex(x0, y0, 0);
    q.index[1] = vertex(x0, y0 + kRowsPerBand, 0);
    q.index[2] = vertex(x0 + kColumnsPerBucket, y0 + kRowsPerBand, 0);
    q.index[3] = vertex(x0 + kColumnsPerBucket, y0, 0);
    const double u0 = uvU(x0);
    const double u1 = uvU(x0 + kColumnsPerBucket) == 0.0 ? 1.0 : uvU(x0 + kColumnsPerBucket);
    const double v0 = uvV(y0);
    double v1 = uvV(y0 + kRowsPerBand);
    if (v1 == 1.0)
      v1 = 0.0;
    q.uv = {{{u0, v0}, {u0, v1}, {u1, v1}, {u1, v0}}};
    q.colour = colourFromHi(0x406FE000u);
    q.coord = x0;
    return q;
  }

  bool wallFlag(const std::vector<Record>& recs, int x0, int y0) const {
    if (recs.size() == 1 && recs[0].y_end == y0 + 0xF && recs[0].y_start == y0 &&
        recs[0].x0 == x0 && recs[0].x1 == x0 + 7)
      return false;
    return true;
  }

  std::size_t keySlots() const { return keySlots_; }
  std::size_t unresolved() const { return unresolved_; }
  std::size_t touchedCount() const {
    std::size_t n = 0;
    for (std::size_t i = 0; i < touched_.size(); ++i)
      if (touched_[i] && table_[i] != 0xFFFFFFFFu)
        ++n;
    return n;
  }
  bool supportMatches() const {
    for (std::size_t i = 0; i < table_.size(); ++i) {
      const bool populated = table_[i] != 0xFFFFFFFFu;
      if (populated != (touched_[i] != 0))
        return false;
    }
    return true;
  }
  void setRecords(std::span<const std::array<std::int32_t, 3>> records) {
    records_ = records;
  }

 private:
  // The record's `+4` is the first gap cell and `+2` the last, in both the
  // 0x4ECA58 and 0x4ECA68 lists (FUN_004615E9). `x_rec` is the x the quad's
  // UV is anchored on: the record's x1 for the right list, x0 for the left.
  void sideGaps(const Record& rec, int x_rec, int x, std::vector<Gap>& into) const {
    if (!rec.door) {
      int y = rec.y_start;
      for (;;) {
        while (y <= rec.y_end && solidNoDoor(x, y))
          ++y;
        if (y <= rec.y_end) {
          Gap g;
          g.x = x_rec;
          g.start = y;
          int s = y;
          for (;;) {
            y = s;
            s = y + 1;
            if (rec.y_end < s)
              break;
            if (!(!solidNoDoor(x, s) || (cell(x, s) & kDoor) != 0))
              break;
          }
          g.end = y;
          into.push_back(g);
          y = s;
        }
        if (!(y <= rec.y_end))
          break;
      }
    } else {
      int y = rec.y_start;
      while (y <= rec.y_end && (cell(x, y) & kSolidOrDoor) == kSolidOrDoor)
        ++y;
      if (y <= rec.y_end) {
        Gap g;
        g.x = x_rec;
        g.start = rec.y_start;
        g.end = rec.y_end;
        g.door = true;
        into.push_back(g);
      }
    }
  }

  void topGaps(const Record& rec, std::vector<Gap>& into) const {
    const int y = rec.y_end + 1;
    if (!rec.door) {
      int x = rec.x0;
      for (;;) {
        while (x <= rec.x1 && solidNoDoor(x, y))
          ++x;
        if (x <= rec.x1) {
          Gap g;
          g.x = rec.y_end;
          g.start = x;
          int s = x;
          for (;;) {
            x = s;
            s = x + 1;
            if (rec.x1 < s)
              break;
            if (!(!solidNoDoor(s, y) || (cell(s, y) & kDoor) != 0))
              break;
          }
          g.end = x;
          into.push_back(g);
          x = s;
        }
        if (!(x <= rec.x1))
          break;
      }
    } else {
      int x = rec.x0;
      while (x <= rec.x1 && (cell(x, y) & kSolidOrDoor) == kSolidOrDoor)
        ++x;
      if (x <= rec.x1) {
        Gap g;
        g.x = rec.y_end;
        g.start = rec.x0;
        g.end = rec.x1;
        g.door = true;
        into.push_back(g);
      }
    }
  }

  CellMeshQuad topQuad(const Record& rec) {
    const double u0 = uvU(rec.x0);
    const double u1 = uvU(rec.x1 + 1) == 0.0 ? 1.0 : uvU(rec.x1 + 1);
    const double v0 = uvV(rec.y_start);
    double v1 = uvV(rec.y_end + 1);
    if (v1 == 1.0)
      v1 = 0.0;
    CellMeshQuad q;
    if (!rec.door) {
      q.index[0] = vertex(rec.x0, rec.y_start, 1);
      q.index[1] = vertex(rec.x0, rec.y_end + 1, 1);
      q.index[2] = vertex(rec.x1 + 1, rec.y_end + 1, 1);
      q.index[3] = vertex(rec.x1 + 1, rec.y_start, 1);
    } else {
      q.index[0] = vertexDoor(rec.x0, rec.y_start);
      q.index[1] = vertexDoor(rec.x0, rec.y_end + 1);
      q.index[2] = vertexDoor(rec.x1 + 1, rec.y_end + 1);
      q.index[3] = vertexDoor(rec.x1 + 1, rec.y_start);
    }
    q.uv = {{{u0, v0}, {u0, v1}, {u1, v1}, {u1, v0}}};
    q.colour = colourFromHi(0x406FE000u);
    q.coord = rec.x0;
    return q;
  }

  std::span<const std::uint32_t> table_;
  const CollisionGrid& grid_;
  std::vector<char> touched_;
  std::size_t keySlots_ = 0;
  std::size_t unresolved_ = 0;
  std::span<const std::array<std::int32_t, 3>> records_;
};

}  // namespace

std::expected<CellMeshes, std::string> buildCellMeshes(
    std::span<const std::uint32_t> keyTable,
    std::span<const std::array<std::int32_t, 3>> doorRecords,
    const CollisionGrid& grid, std::size_t surfaceVertexCount) {
  if (keyTable.empty())
    return std::unexpected("cell mesh: the level has no (x, y, layer) key table");
  if (grid.height <= 0 || grid.cells.empty())
    return std::unexpected("cell mesh: the level has no collision grid");
  if (grid.height % kRowsPerBand != 0)
    return std::unexpected(
        std::format("cell mesh: grid height {} is not a multiple of {}",
                    grid.height, kRowsPerBand));

  Builder builder(keyTable, grid);
  builder.setRecords(doorRecords);
  CellMeshes out;
  const int bands = grid.height / kRowsPerBand;
  out.cells.reserve(static_cast<std::size_t>(kBuckets) * bands);

  for (int bucket = 0; bucket < kBuckets; ++bucket) {
    const int x0 = bucket * kColumnsPerBucket;
    for (int band = 0; band < bands; ++band) {
      const int y0 = band * kRowsPerBand;
      std::vector<Record> recs = builder.partLists(x0, y0);
      std::vector<Gap> left, right, top;
      builder.annotate(recs, left, right, top);

      CellMeshCell cell;
      cell.bucket = bucket;
      cell.band = band;
      cell.x0 = x0;
      cell.y0 = y0;
      cell.material = static_cast<std::uint32_t>((bucket >> 2) +
                                                 ((bands - 1 - band) / 2) * 8);
      cell.wall = builder.wallFlag(recs, x0, y0);
      if (cell.wall) {
        // FUN_00463100 builds the whole-block quad at entry+8 from four layer-0
        // vertices; they share the key table with the meshes. `0x4622FD`
        // submits it as the INN pass's inline quad.
        cell.inner = builder.wholeBlockQuad(x0, y0);
        cell.hasInner = true;
      }
      cell.mesh0 = builder.mesh0(recs, right, top);
      cell.mesh4 = builder.mesh4(recs, left, top);

      for (const auto& q : cell.mesh0)
        for (std::uint32_t i : q.index)
          if (i >= surfaceVertexCount)
            return std::unexpected(std::format(
                "cell mesh: mesh0 index {} is outside the {}-vertex surface pool",
                i, surfaceVertexCount));
      for (const auto& q : cell.mesh4)
        for (std::uint32_t i : q.index)
          if (i >= surfaceVertexCount)
            return std::unexpected(std::format(
                "cell mesh: mesh4 index {} is outside the {}-vertex surface pool",
                i, surfaceVertexCount));

      out.mesh0Quads += cell.mesh0.size();
      out.mesh4Quads += cell.mesh4.size();
      if (cell.hasInner)
        ++out.innerQuads;
      out.cells.push_back(std::move(cell));
    }
  }

  out.buckets = kBuckets;
  out.bands = static_cast<std::uint32_t>(bands);
  out.keySlots = builder.keySlots();
  out.keySlotsTouched = builder.touchedCount();
  out.unresolved = builder.unresolved();
  out.keySupportMatches = builder.supportMatches();
  return out;
}

}  // namespace webo
