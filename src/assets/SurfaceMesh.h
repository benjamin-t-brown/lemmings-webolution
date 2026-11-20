// Derive render quads from LevelIndex::grid0, the counted u32[N] Cylinder surface-index array.
// Neighbouring keys give (k, k+1, k+17, k+16) for an angular face and (k, k+1, k+Y+1, k+Y)
// for a ring face. detectRingStride derives Y from mesh spacing. The .LVL has no stride field.
// Keep populated quads with matching edge lengths. The wraparound raster is diagnostic.
// The simulation collides on CollisionGrid. Retail grid1 is empty.

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
  // Provisional: key+8 marks a post that ends the floor and leaves a wall edge.
  bool blocked = false;
};

// Join outer vertices across key+16 or key+Y when missing thickness partners prevent a quad.
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

// Unrolled cylinder occupancy. X wraps and each solid[y * width + x] is 0 or 1.
struct OccupancyMask {
  int width = 0;
  int height = 0;
  std::vector<std::uint8_t> solid;

  std::size_t solidCells() const;
  bool empty() const { return solid.empty(); }
};

// Unroll a vertex: angle in [0, 1), height in world units (0 at the bottom).
std::pair<double, double> unrollCylinder(const Vec3& v);

// Choose Y as the multiple of 16 whose median vertex distance best matches ring spacing.
int detectRingStride(const CylinderMesh& mesh, std::span<const std::uint32_t> grid);

// Walk grid0 and any nonempty grid1.
std::expected<SurfaceMesh, std::string> buildSurfaceMesh(
    const CylinderMesh& mesh, std::span<const std::uint32_t> grid0,
    std::span<const std::uint32_t> grid1);

// Scale world units by pixelsPerUnit. Default circumference width is a multiple of 32 pixels.
OccupancyMask rasteriseSurface(const SurfaceMesh& faces, const CylinderMesh& mesh,
                               int pixelsPerUnit = 2);

}  // namespace webo
