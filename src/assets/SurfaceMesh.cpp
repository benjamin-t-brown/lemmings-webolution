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
                     SurfaceMesh& out, std::set<std::uint32_t>& used) {
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
  collectFromGrid(mesh, grid0, out.ringStride, out, used);
  if (!grid1.empty())
    collectFromGrid(mesh, grid1, out.ringStride, out, used);
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
  return mask;
}

}  // namespace webo
