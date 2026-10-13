// PolyList 09 chunks inside a .LVL.
//
// Hundreds of these sit after the cylinder mesh. Each is an MFC-style
// length-prefixed "PolyList 09" name followed by a small header and a run of
// fixed-stride vertex records. The records that match the size formula
// (kind 5 → 160 bytes, kind 7 → 212 bytes) begin with a u32 index into the
// cylinder surface vertex pool, then UV and RGBA stored as doubles.
//
// That is render attribute data for the surface, not a separate collision
// mesh: indices land in the surface pool / grid arrays, and the payload is
// colour and texture coordinates. Walkable cells still come from the surface
// lattice (see SurfaceMesh.h).

#pragma once

#include <cstdint>
#include <expected>
#include <span>
#include <string>
#include <vector>

namespace webo {

struct PolyVertex {
  std::uint32_t index = 0;  // into CylinderMesh::surface
  double u = 0;
  double v = 0;
  double r = 0;
  double g = 0;
  double b = 0;
  double a = 0;
};

struct PolyListBlock {
  std::uint32_t kind = 0;   // 5 or 7 for formula-matched blocks
  std::uint32_t count = 0;
  std::uint32_t u2 = 0;     // block header word; often a running id
  std::uint32_t u3 = 0;
  std::size_t offset = 0;   // start of payload (after the name)
  std::vector<PolyVertex> vertices;
};

struct PolyLists {
  std::vector<PolyListBlock> blocks;
  std::size_t rawCount = 0;       // every "PolyList 09" name seen
  std::size_t matchedCount = 0;   // blocks that fit the size formula
};

// Parse every formula-matched PolyList 09 block. Unmatched kinds/sizes are
// counted in rawCount but omitted from blocks.
std::expected<PolyLists, std::string> findPolyLists(std::span<const std::byte>);

}  // namespace webo
