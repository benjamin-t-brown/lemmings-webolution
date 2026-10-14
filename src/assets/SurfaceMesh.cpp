#include "SurfaceMesh.h"

#include <algorithm>
#include <cmath>
#include <cstring>
#include <format>
#include <numbers>
#include <set>
#include <vector>

namespace webo {
namespace {

constexpr std::uint32_t kEmpty = 0xFFFFFFFFu;

double dist(const Vec3& a, const Vec3& b) {
  const double dx = a.x - b.x, dy = a.y - b.y, dz = a.z - b.z;
  return std::sqrt(dx * dx + dy * dy + dz * dz);
}

std::pair<double, std::size_t> medianPairDist(const CylinderMesh& mesh,
                                              std::span<const std::uint32_t> grid, int delta) {
  std::vector<double> ds;
  for (std::size_t k = 0; k + static_cast<std::size_t>(delta) < grid.size(); ++k) {
    const auto a = grid[k], b = grid[k + static_cast<std::size_t>(delta)];
    if (a == kEmpty || b == kEmpty)
      continue;
    if (a >= mesh.surface.size() || b >= mesh.surface.size())
      continue;
    ds.push_back(dist(mesh.surface[a], mesh.surface[b]));
  }
  if (ds.size() < 8)
    return {-1, ds.size()};
  std::ranges::sort(ds);
  return {ds[ds.size() / 2], ds.size()};
}

bool near(double a, double b, double tol) { return std::abs(a - b) <= tol; }

// A quad is accepted when its four edges match the lattice geometry. The
// tolerances are loose enough for both the inner and outer chords.
bool looksLikeFace(const Vec3 q[4], double thickness, double ringSpacing, bool ringFace) {
  const double ab = dist(q[0], q[1]);  // should be thickness
  const double dc = dist(q[3], q[2]);  // should be thickness
  const double ad = dist(q[0], q[3]);  // angular chord or ring spacing
  const double bc = dist(q[1], q[2]);
  if (!near(ab, thickness, 1.5) || !near(dc, thickness, 1.5))
    return false;
  if (ringFace)
    return near(ad, ringSpacing, 1.5) && near(bc, ringSpacing, 1.5);
  // Angular: outer chord ~11.74, inner ~9.80. Accept anything in between.
  constexpr double kMinChord = 8.0;
  constexpr double kMaxChord = 13.5;
  return ad >= kMinChord && ad <= kMaxChord && bc >= kMinChord && bc <= kMaxChord;
}

void collectFromGrid(const CylinderMesh& mesh, std::span<const std::uint32_t> grid, int ringStride,
                     SurfaceMesh& out, std::set<std::uint32_t>& used, bool openEdges) {
  const double thick = mesh.thickness;
  const double ring = mesh.ringSpacing();

  const auto tryQuad = [&](std::size_t k, int step, bool isRing) {
    if (k + static_cast<std::size_t>(step) + 1 >= grid.size())
      return;
    const std::uint32_t i0 = grid[k];
    const std::uint32_t i1 = grid[k + 1];
    const std::uint32_t i2 = grid[k + static_cast<std::size_t>(step) + 1];
    const std::uint32_t i3 = grid[k + static_cast<std::size_t>(step)];
    if (i0 == kEmpty || i1 == kEmpty || i2 == kEmpty || i3 == kEmpty)
      return;
    if (i0 >= mesh.surface.size() || i1 >= mesh.surface.size() || i2 >= mesh.surface.size() ||
        i3 >= mesh.surface.size())
      return;

    SurfaceQuad q;
    q.v[0] = mesh.surface[i0];
    q.v[1] = mesh.surface[i1];
    q.v[2] = mesh.surface[i2];
    q.v[3] = mesh.surface[i3];
    if (!looksLikeFace(q.v, thick, ring, isRing))
      return;

    used.insert(i0);
    used.insert(i1);
    used.insert(i2);
    used.insert(i3);

    // A half-step post (key+8) between the ends means the chord crosses a
    // wall. MM_CLIMB_002's walkers turn at column 168, the last vertex
    // before the post at 172, and the same insertion sits at 64, 80 and
    // 184. The post's own ring quad is a line at the far column; the turn
    // is at this vertex, so the wall is raised here and the floor stops.
    if (!isRing && k + 9 < grid.size()) {
      const auto h0 = grid[k + 8];
      const auto h1 = grid[k + 9];
      if (h0 != kEmpty && h1 != kEmpty && h0 < mesh.surface.size() && h1 < mesh.surface.size()) {
        const Vec3& pa = mesh.surface[h0];
        const Vec3& pb = mesh.surface[h1];
        const Vec3& post = std::hypot(pa.x, pa.z) >= std::hypot(pb.x, pb.z) ? pa : pb;
        const Vec3& near = std::hypot(q.v[0].x, q.v[0].z) >= std::hypot(q.v[1].x, q.v[1].z) ? q.v[0] : q.v[1];
        const double outerCut = mesh.innerRadius() + mesh.thickness * 0.5;
        if (std::hypot(post.x, post.z) > outerCut && std::abs(post.y - near.y) < 1.5 && ring > 0) {
          q.blocked = true;
          Vec3 top = near;
          top.y += ring;
          out.openEdges.push_back({near, top});
        }
      }
    }

    out.quads.push_back(q);
    if (isRing)
      ++out.ringQuads;
    else
      ++out.angularQuads;
  };

  for (std::size_t k = 0; k + 17 < grid.size(); ++k)
    tryQuad(k, 16, false);
  if (ringStride > 16) {
    for (std::size_t k = 0; k + static_cast<std::size_t>(ringStride) + 1 < grid.size(); ++k)
      tryQuad(k, ringStride, true);
  }

  // grid1 is a dense index of the same pool. The walked links were found
  // in grid0; running this pass on grid1 fills the ring.
  if (!openEdges)
    return;

  // Edges the quad pass dropped. The thickness partner lives at key-1; when
  // it is absent, tryQuad never sees a face, but the outer vertices are
  // still neighbours. MM_CLIMB_002's walkers travel the key+16 case (the
  // hole under columns 128..144) and turn at the key+Y case (column 128,
  // y 32 -> 48). The bottom skirt (y 0 -> 16) is a full ring of such
  // verticals and is not a wall on the ledge, so it is left out.
  const double outerCut = mesh.innerRadius() + mesh.thickness * 0.5;
  const auto outer = [&](const Vec3& v) { return std::hypot(v.x, v.z) > outerCut; };
  const auto empty = [&](std::size_t slot) {
    return slot >= grid.size() || grid[slot] == kEmpty || grid[slot] >= mesh.surface.size();
  };

  for (std::size_t k = 1; k + 16 < grid.size(); ++k) {
    if (empty(k) || empty(k + 16))
      continue;
    // Both thickness partners present: the quad pass owns this edge.
    if (!empty(k - 1) && !empty(k + 15))
      continue;
    const Vec3& a = mesh.surface[grid[k]];
    const Vec3& b = mesh.surface[grid[k + 16]];
    if (!outer(a) || !outer(b) || std::abs(a.y - b.y) > 1.5)
      continue;
    const double chord = dist(a, b);
    if (chord < 8.0 || chord > 13.5)
      continue;
    // Same half-step rule as the quad pass: a post at key+8 ends the floor
    // at this vertex instead of letting the chord cross it.
    if (!empty(k + 8) && mesh.ringSpacing() > 0) {
      const Vec3& post = mesh.surface[grid[k + 8]];
      if (outer(post) && std::abs(post.y - a.y) < 1.5) {
        Vec3 top = a;
        top.y += mesh.ringSpacing();
        out.openEdges.push_back({a, top});
        continue;
      }
    }
    out.openEdges.push_back({a, b});
  }

  if (ringStride > 16) {
    const double ring = mesh.ringSpacing();
    for (std::size_t k = 1; k + static_cast<std::size_t>(ringStride) < grid.size(); ++k) {
      if (empty(k) || empty(k + static_cast<std::size_t>(ringStride)))
        continue;
      // A wall on the ledge: the partner is missing at the foot and at the
      // top. Requiring the foot to sit above the bottom ring keeps out the
      // y 0 -> 16 skirt, which is one of these at every angular step.
      if (!empty(k - 1) || !empty(k - 1 + static_cast<std::size_t>(ringStride)))
        continue;
      const Vec3& a = mesh.surface[grid[k]];
      const Vec3& b = mesh.surface[grid[k + static_cast<std::size_t>(ringStride)]];
      if (!outer(a) || !outer(b))
        continue;
      // Rising from the ledge, not dropping off it. A link from y 32 down to
      // y 16 exists at every angular step and is not a wall.
      const double lo = std::min(a.y, b.y);
      const double hi = std::max(a.y, b.y);
      if (lo < 31.0 || std::abs((hi - lo) - ring) > 2.0)
        continue;
      const auto pa = unrollCylinder(a).first;
      const auto pb = unrollCylinder(b).first;
      double dang = pa - pb;
      if (dang > 0.5)
        dang -= 1.0;
      if (dang < -0.5)
        dang += 1.0;
      if (std::abs(dang) > 1.0 / 256.0)
        continue;
      out.openEdges.push_back({a, b});
    }
  }
}

void stampDisk(OccupancyMask& mask, int cx, int cy, int radius) {
  const int w = mask.width, h = mask.height;
  const int r2 = radius * radius;
  for (int dy = -radius; dy <= radius; ++dy) {
    const int y = cy + dy;
    if (y < 0 || y >= h)
      continue;
    for (int dx = -radius; dx <= radius; ++dx) {
      if (dx * dx + dy * dy > r2)
        continue;
      int x = cx + dx;
      x %= w;
      if (x < 0)
        x += w;
      mask.solid[static_cast<std::size_t>(y) * w + x] = 1;
    }
  }
}

// Thick stroke in unrolled space. X wraps; a seam-crossing segment is drawn
// in both directions so it is not clipped at the edge.
void stroke(OccupancyMask& mask, double x0, double y0, double x1, double y1, int radius) {
  const int w = mask.width;
  // Unwrap the shortest path around the cylinder for the stroke itself.
  double dx = x1 - x0;
  if (dx > w * 0.5)
    dx -= w;
  if (dx < -w * 0.5)
    dx += w;
  const double dy = y1 - y0;
  const int steps = std::max(1, static_cast<int>(std::ceil(std::hypot(dx, dy))));
  for (int i = 0; i <= steps; ++i) {
    const double t = static_cast<double>(i) / steps;
    double x = x0 + dx * t;
    const double y = y0 + dy * t;
    int ix = static_cast<int>(std::lround(x)) % w;
    if (ix < 0)
      ix += w;
    stampDisk(mask, ix, static_cast<int>(std::lround(y)), radius);
  }
}

}  // namespace

std::size_t OccupancyMask::solidCells() const {
  std::size_t n = 0;
  for (auto c : solid)
    if (c)
      ++n;
  return n;
}

std::pair<double, double> unrollCylinder(const Vec3& v) {
  double a = std::atan2(-v.x, v.z) / (2.0 * std::numbers::pi);
  if (a < 0)
    a += 1.0;
  return {a, v.y};
}

int detectRingStride(const CylinderMesh& mesh, std::span<const std::uint32_t> grid) {
  const double target = mesh.ringSpacing();
  if (target <= 0 || grid.size() < 64)
    return 0;

  double bestScore = 1e99;
  int best = 0;
  std::size_t bestN = 0;
  for (int delta = 48; delta < static_cast<int>(grid.size()) && delta < 20000; delta += 16) {
    const auto [med, n] = medianPairDist(mesh, grid, delta);
    if (med < 0)
      continue;
    const double score = std::abs(med - target);
    // Prefer a closer median; on a tie, prefer more pairs so a sparse false
    // stride cannot beat a well-populated real one.
    if (score < bestScore - 1e-6 || (std::abs(score - bestScore) < 1e-6 && n > bestN)) {
      bestScore = score;
      best = delta;
      bestN = n;
    }
  }
  // Only trust a stride that actually lands near one ring spacing.
  return bestScore < 2.0 ? best : 0;
}

std::expected<SurfaceMesh, std::string> buildSurfaceMesh(const CylinderMesh& mesh,
                                                         std::span<const std::uint32_t> grid0,
                                                         std::span<const std::uint32_t> grid1) {
  if (mesh.surface.empty())
    return std::unexpected("surface mesh: no surface vertices");
  if (grid0.empty())
    return std::unexpected("surface mesh: empty index array");

  SurfaceMesh out;
  out.surfaceVertexCount = mesh.surface.size();
  out.ringStride = detectRingStride(mesh, grid0);
  if (out.ringStride == 0 && !grid1.empty())
    out.ringStride = detectRingStride(mesh, grid1);

  std::set<std::uint32_t> used;
  collectFromGrid(mesh, grid0, out.ringStride, out, used, true);
  if (!grid1.empty())
    collectFromGrid(mesh, grid1, out.ringStride, out, used, false);
  out.verticesUsed = used.size();
  // An empty face list is a valid sparse level, not a parse failure: the
  // caller gets a blank mask and can fall back. Returning an error here made
  // a handful of retail levels look broken when they are only empty.
  return out;
}

OccupancyMask rasteriseSurface(const SurfaceMesh& faces, const CylinderMesh& mesh,
                               int pixelsPerUnit) {
  OccupancyMask mask;
  if (pixelsPerUnit < 1 || faces.quads.empty())
    return mask;

  const double worldH = mesh.height();
  if (worldH <= 0)
    return mask;

  // Circumference in pixels: 32 angular steps keeps columns aligned with the
  // tube lattice. 4 pixels per step at pixelsPerUnit=2 gives width 256.
  mask.width = static_cast<int>(mesh.angularSteps) * pixelsPerUnit * 4;
  mask.height = std::max(1, static_cast<int>(std::ceil(worldH * pixelsPerUnit)));
  mask.solid.assign(static_cast<std::size_t>(mask.width) * mask.height, 0);

  // Unrolled (angle, height) discards radius, so a thickness edge (inner <->
  // outer) collapses to a point. The walkable silhouette is the *other* two
  // edges of each lattice quad: angular (+16) and ring (+Y). Stroke those
  // with a brush so platforms have width a lemming can stand on.
  const int brush = std::max(1, pixelsPerUnit);

  const auto toPixel = [&](const Vec3& v) {
    const auto [a, y] = unrollCylinder(v);
    // Y grows upward in world space; the mask indexes top-down like the rest
    // of the simulation (y=0 is the sky).
    return std::pair{a * mask.width, (worldH - y) * pixelsPerUnit};
  };

  for (const auto& q : faces.quads) {
    if (q.blocked)
      continue;
    // Prefer the outer vertex of each thickness pair (larger radius) so the
    // stroke sits on the outside of the tube.
    const auto pick = [](const Vec3& a, const Vec3& b) {
      return std::hypot(a.x, a.z) >= std::hypot(b.x, b.z) ? a : b;
    };
    const Vec3 p0 = pick(q.v[0], q.v[1]);  // at key k
    const Vec3 p1 = pick(q.v[3], q.v[2]);  // at key k+step
    const auto [x0, y0] = toPixel(p0);
    const auto [x1, y1] = toPixel(p1);
    stroke(mask, x0, y0, x1, y1, brush);
  }
  for (const auto& e : faces.openEdges) {
    const auto [x0, y0] = toPixel(e.a);
    const auto [x1, y1] = toPixel(e.b);
    stroke(mask, x0, y0, x1, y1, brush);
  }
  return mask;
}

}  // namespace webo
