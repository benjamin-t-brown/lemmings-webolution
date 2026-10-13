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
//     ringCount * 32 * 2 * { f64 x, f64 y, f64 z }
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
