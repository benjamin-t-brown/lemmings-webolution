// PolyList 09 render attributes in a .LVL. A counted "PolyList 09" name precedes
// a 16-byte header and fixed-stride records. Physics uses CollisionGrid.h.
// Kind 5 has three vertices and a u32 (160 bytes). Kind 7 has four and a u32 (212 bytes).
// Each 52-byte vertex uses +0 u32 CylinderMesh::surface index, which can exceed the pool,
// then f64 +4 u, +12 v, +20 r, +28 g, +36 b and +44 a.
// Retail RGB channels are integers in [0,255]. Alpha is usually shared within a record,
// with 178.5 as the only fractional value. UVs can extend outside the unit square.
// The trailing endWord (+156 for kind 5, +208 for kind 7) varies independently of header words u2 and u3.
// Kinds 4 and 6 use other strides and count only toward rawCount.

#pragma once

#include <array>
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

// Three or four vertices followed by an unidentified u32.
struct PolyRecord {
  std::uint32_t cornerCount = 0;  // 3 (kind 5) or 4 (kind 7)
  std::array<PolyVertex, 4> corners{};
  std::uint32_t endWord = 0;  // unidentified trailing word
};

struct PolyListBlock {
  std::uint32_t kind = 0;   // 5 or 7
  std::uint32_t count = 0;
  std::uint32_t u2 = 0;     // block header word; often a running id
  std::uint32_t u3 = 0;
  std::size_t offset = 0;   // start of payload (after the name)
  std::vector<PolyRecord> records;
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
