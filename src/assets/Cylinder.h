// The cylinder a level is played on.
//
// Lemmings Revolution's levels are not flat: each one wraps around the inside
// and outside of a vertical tube, which is why the simulation's X axis wraps
// and its Y axis does not. This is the tube's geometry, read out of the .LVL
// file and verified against all 102 retail levels.
//
// The mesh turns out to be *identical in every level* -- same ring count, same
// radii, same spacing, same vertex count -- so it is effectively a constant of
// the game that happens to be stored per level. That is worth knowing: the
// renderer can treat the tube as fixed, and only the skin textures and the
// terrain change from level to level.
//
// Layout in the file, at the position `findCylinderMesh` locates:
//
//     u32     faceCount      112 in every retail level; not yet identified
//     f64     thickness      10.0 -- outerRadius - innerRadius
//     u32     ringCount      7
//     ringCount * 32 * 2 * { f64 x, f64 y, f64 z }     ring vertices
//     u32     ringCount      again
//     ringCount * 32 * f64                             per-vertex inner radii
//     u32     one            1 in every retail level; not yet identified
//     u32     surfaceCount
//     surfaceCount * { f64 x, f64 y, f64 z }           the level's surface
//     u32     zero           0 in every retail level
//     u32     48
//     48 * counted string                              OUTER tile names
//     48 * counted string                              INNER tile names
//
// The whole chain parses exactly in all 102 retail levels.
//
// The important asymmetry is that everything above is **constant** across the
// retail levels except `surfaceCount` and its vertices, which range from 539
// to 1558. So the per-level terrain is carried as polygon geometry, not as a
// bitmap mask -- which is what finally explains why the two equal-sized u32
// arrays in the file never factored as a rectangular grid (see LevelIndex.h).
//
// The surface vertices open with a quad -- for the first retail level,
// (0,0,60) (0,16,60) (-11.74,16,59.02) (-11.74,0,59.02), spanning one angular
// step and one ring band on the outer skin -- and then continue in pairs, so
// they read as a strip. The exact primitive and winding are not yet pinned
// down, so this exposes the vertices without claiming a topology.
//
// The vertices are **not** 8-byte aligned in the file (the u32 before them
// puts them on a 4-byte boundary), so they must be read unaligned.
//
// Vertices come in angular order, and within each angular step the inner
// vertex precedes the outer one:
//
//     ring 0: (step 0 inner) (step 0 outer) (step 1 inner) (step 1 outer) ...
//     ring 1: ...
//
// Measured in the retail data: 32 angular steps of exactly 11.25 degrees,
// 7 rings at y = 0, 16, 32, ... 96, inner radius 50 and outer radius 60.
//
// The angular step count divides the tile grid evenly, which is the useful
// part: the skin is addressed as 8 tile columns by 3 tile rows (read off the
// loop bounds at .text:0043AA91 in the original), so one tile column spans
// 32 / 8 = 4 angular steps, and one tile row spans 6 / 3 = 2 ring bands.

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
  // Unidentified. 112 in all 102 retail levels, so it is recorded rather than
  // interpreted; naming it something confident would be a guess.
  std::uint32_t faceCount = 0;

  double thickness = 0;          // outerRadius - innerRadius
  std::uint32_t ringCount = 0;   // rings of vertices, so bands = ringCount - 1
  std::uint32_t angularSteps = 0;  // around the tube; 32 in the retail data

  // ringCount * angularSteps * 2 vertices, inner before outer at each step.
  std::vector<Vec3> vertices;

  // ringCount * angularSteps inner radii, parallel to the inner vertices.
  // Period 8 in the retail data, so the cross-section has 4-fold symmetry.
  std::vector<double> radii;

  // The per-level surface geometry. This is the part that varies.
  std::vector<Vec3> surface;

  // Unidentified; 1 in every retail level. Recorded, not interpreted.
  std::uint32_t one = 0;

  // The skin tiles, in two blocks of 48 slots. The count is stated once and
  // applies to each block; the second follows the first with no header of its
  // own. Half of each block's slots are empty, so a level names 24 outer and
  // 24 inner tiles -- one per cell of the 8x3 grid.
  //   "Cylinder\MM_CLIMB_002\OUT_MM_CLIMB_002_0_0"
  std::vector<std::string> tileTextures;       // all prefixed OUT_
  std::vector<std::string> innerTileTextures;  // all prefixed INN_

  // Where the mesh was found, so callers can keep parsing after it.
  std::size_t offset = 0;
  std::size_t endOffset = 0;

  const Vec3& vertex(std::uint32_t ring, std::uint32_t step, bool outer) const;

  double innerRadius() const;
  double outerRadius() const;
  double ringSpacing() const;  // 0 when there is only one ring
  double height() const;

  // Bands of surface between rings.
  std::uint32_t bandCount() const { return ringCount > 0 ? ringCount - 1 : 0; }

  // The tile grid the skin textures are addressed in, from the original's
  // own loop bounds rather than from this file.
  static constexpr std::uint32_t kTileColumns = 8;
  static constexpr std::uint32_t kTileRows = 3;

  std::uint32_t stepsPerTileColumn() const { return angularSteps / kTileColumns; }
  std::uint32_t bandsPerTileRow() const { return bandCount() / kTileRows; }
};

// Finds the mesh by signature rather than by offset: the first ring's first
// vertex is (0, 0, innerRadius), and the whole vertex run is then validated
// before the match is accepted. The preceding blocks vary between levels, so
// a fixed offset would not work.
std::expected<CylinderMesh, std::string> findCylinderMesh(std::span<const std::byte>);

}  // namespace webo
