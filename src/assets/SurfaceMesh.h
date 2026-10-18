// Surface faces rebuilt from the cylindrical key lattice.
//
// LevelIndex::grid0 is the u32[N] array on the Cylinder walk. It indexes
// surface vertices by a spatial key (see PLAN.md 1.6). grid1 is empty; the
// file has one array, and an empty span contributes nothing. Neighbouring
// keys name neighbouring vertices, so the faces are:
//
//   (k, k+1, k+17, k+16)     thickness x angular step
//   (k, k+1, k+Y+1, k+Y)     thickness x ring, Y = per-level ring stride
//
// Y is not stored in the .LVL; detectRingStride finds it by scoring multiples
// of 16 against the mesh's ring spacing (see PLAN.md 1.6).
//
// This module walks the lattice, keeps every fully-populated quad whose edge
// lengths match those expectations, and can rasterise the result into a flat
// wraparound occupancy mask. The simulation collides on CollisionGrid; this
// mesh is render input.

#pragma once

#include "Cylinder.h"

#include <cstddef>
#include <cstdint>
#include <expected>
#include <span>
#include <string>
#include <vector>

namespace webo {

struct SurfaceQuad {
  Vec3 v[4]{};  // winding: k, k+1, k+1+step, k+step
  // A key+8 post sits on this angular step. The chord is not floor; the
  // walker turns at the near vertex. The wall itself is an open edge.
  bool blocked = false;
};

// An edge the quad pass cannot emit. A face needs its thickness partner on
// both ends; when that slot is empty the outer vertices are still joined
// (key+16 along a ring, or key+Y up a wall) and the walkers use them.
struct SurfaceSpan {
  Vec3 a, b;
};

struct SurfaceMesh {
  std::vector<SurfaceQuad> quads;
  std::vector<SurfaceSpan> openEdges;
  int ringStride = 0;  // derived Y; 0 when no ring faces were found
  std::size_t angularQuads = 0;
  std::size_t ringQuads = 0;

  // How much of the surface vertex pool is referenced by at least one quad.
  std::size_t verticesUsed = 0;
  std::size_t surfaceVertexCount = 0;
};

// Occupancy in unrolled cylinder coordinates: X wraps around the tube, Y is
// height from the bottom of the mesh. solid[y * width + x] is 1 or 0.
struct OccupancyMask {
  int width = 0;
  int height = 0;
  std::vector<std::uint8_t> solid;

  std::size_t solidCells() const;
  bool empty() const { return solid.empty(); }
};

// Unroll a vertex: angle in [0, 1), height in world units (0 at the bottom).
std::pair<double, double> unrollCylinder(const Vec3& v);

// Derive the ring stride Y for one index array: the multiple of 16 whose
// median vertex distance is closest to the mesh's ring spacing. There is no
// matching stored field in the retail .LVL files.
int detectRingStride(const CylinderMesh& mesh, std::span<const std::uint32_t> grid);

// Build faces from grid0. grid1 is accepted and walked when non-empty; the
// retail loader leaves it empty.
std::expected<SurfaceMesh, std::string> buildSurfaceMesh(
    const CylinderMesh& mesh, std::span<const std::uint32_t> grid0,
    std::span<const std::uint32_t> grid1);

// Rasterise quads into a wraparound mask. `pixelsPerUnit` scales world units
// (mesh height is typically 96) into mask pixels; width is the circumference
// in pixels (defaults to a multiple of 32 so each angular step is equal).
OccupancyMask rasteriseSurface(const SurfaceMesh& faces, const CylinderMesh& mesh,
                               int pixelsPerUnit = 2);

}  // namespace webo
