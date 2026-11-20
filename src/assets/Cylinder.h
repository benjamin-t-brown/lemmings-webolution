// Tube geometry in the .LVL. X wraps around the cylinder and Y measures height.
// Layout at the position returned by findCylinderMesh:
//   u32 faceCount, unidentified (112 in retail)
//   f64 thickness (10.0), u32 ringCount (7)
//   ringCount * 32 * 2 triples of f64 x, y, z, inner then outer at each angular step
//   u32 secondCount (7), secondCount * 32 uninterpreted doubles
//   u32 one, unidentified (1), u32 surfaceCount, surfaceCount triples (539..1558)
//   u32 secondaryCount (0), secondaryCount opaque f64 triples
//   u32 tile count (48), then outer and inner counted-string blocks of that length
// Vertex doubles can be unaligned. The structured reader counts rings and double blocks independently.
// The 49858-word surface index is counted separately from header[3] (LevelIndex.h).
// Physics uses CollisionGrid.h.
// Surface primitive and winding remain unresolved. The ring mesh uses 32 steps of 11.25 degrees,
// seven rings at Y=0..96 in steps of 16, and radii 50 and 60 in retail data.
// Tile loop .text:0043AA91 uses 8 columns by 3 rows, or four angular steps by two bands per tile.

#pragma once

#include <cstdint>
#include <expected>
#include <span>
#include <string>
#include <vector>

namespace webo {

struct Vec3 {
  double x = 0;
  double y = 0;
  double z = 0;
};

struct CylinderMesh {
  // Unidentified word (112 in retail).
  std::uint32_t faceCount = 0;

  double thickness = 0;          // outerRadius - innerRadius
  std::uint32_t ringCount = 0;   // rings of vertices, so bands = ringCount - 1
  std::uint32_t angularSteps = 0;  // around the tube; 32 in the retail data

  // ringCount * angularSteps * 2 vertices, inner before outer at each step.
  std::vector<Vec3> vertices;

  // Independent double blocks in block/component order. The compatibility name gives no vertex mapping.
  // The retail values repeat with period 8.
  std::vector<double> radii;

  // The per-level surface geometry. This is the part that varies.
  std::vector<Vec3> surface;

  // Unidentified word (1 in retail).
  std::uint32_t one = 0;

  // The shared count covers both 48-slot blocks. Each block names 24 tiles
  // and leaves the other surface's slots empty, e.g. "Cylinder\MM_CLIMB_002\OUT_MM_CLIMB_002_0_0".
  std::vector<std::string> tileTextures;       // all prefixed OUT_
  std::vector<std::string> innerTileTextures;  // all prefixed INN_

  // Where the mesh was found, so callers can keep parsing after it.
  std::size_t offset = 0;
  std::size_t endOffset = 0;

  // Legacy success retains the structured reader's rejection and byte cursor.
  std::string meshFallbackReason;

  const Vec3& vertex(std::uint32_t ring, std::uint32_t step, bool outer) const;

  double innerRadius() const;
  double outerRadius() const;
  double ringSpacing() const;  // 0 when there is only one ring
  double height() const;

  // Bands of surface between rings.
  std::uint32_t bandCount() const { return ringCount > 0 ? ringCount - 1 : 0; }

  // Tile dimensions from the original's draw loop bounds.
  static constexpr std::uint32_t kTileColumns = 8;
  static constexpr std::uint32_t kTileRows = 3;

  std::uint32_t stepsPerTileColumn() const { return angularSteps / kTileColumns; }
  std::uint32_t bandsPerTileRow() const { return bandCount() / kTileRows; }
};

// Read the ordered prefix and world, then the compatibility surface/tile tail.
// Consume the secondary triples without exposing them. Only a structured rejection
// enables the fallback, which requires equal ring/radius counts and zero secondary triples.
std::expected<CylinderMesh, std::string> findCylinderMesh(std::span<const std::byte>);

}  // namespace webo
