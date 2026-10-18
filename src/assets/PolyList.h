// PolyList 09 chunks inside a .LVL.
//
// Hundreds of these sit after the cylinder mesh. Each is an MFC-style
// length-prefixed "PolyList 09" name followed by a 16-byte header and a run
// of fixed-stride records. The records that match the size formula are
// render attributes, not a collision mesh: colour and texture coordinates,
// and an index that lands in the surface pool on most records (87% on the
// corpus; the misses are already true of the first vertex). Walkable cells
// come from the stored collision grid (CollisionGrid.h).
//
// Record layout, measured on all 102 retail levels by
// tools/harness/read_polylist_tail.py:
//
//   kind 5: 3 vertices, then one u32     stride 3*52+4 = 160
//   kind 7: 4 vertices, then one u32     stride 4*52+4 = 212
//
// One vertex is 52 bytes:
//
//   +0   u32   index into CylinderMesh::surface
//   +4   f64   u
//   +12  f64   v
//   +20  f64   r
//   +28  f64   g
//   +36  f64   b
//   +44  f64   a
//
// The bytes after the first vertex are that same vertex repeated, not a
// normal, a second UV set, or a differently typed colour. On 110,347 kind-5
// records and 50,354 kind-7 records every double is finite. On every vertex,
// r/g/b are integers in [0, 255] and a is in [0, 255]; u and v sit in the
// same band as the leading vertex (mostly the unit square, a minority out to
// about 3, a few slightly negative). Later indices occupy the same numeric
// range as the leading index, [0, 1333], and they are not copies of it.
// Against each level's surface pool, vertices 0..3 hit on 87.3%, 87.2%,
// 87.2%, and 90.6% of records (vertex 3 only on kind 7). The leading vertex
// is the 87.3%, so the misses are not a property of the tail.
// Alpha is the field that is shared: on more than 99% of records every
// vertex in the record carries the same a. Kind 7 alpha is always a whole
// number. Kind 5 alpha is a whole number on 86,873 of 110,347 records and
// 178.5 on the other 23,474. 178.5 is the only non-integer, and 255 * 0.7
// is 178.5.
//
// The trailing u32 — kind 5 at +156, kind 7 at +208 — is stored as `endWord`
// and not named. It is not padding. On those 102 levels it ranges up to
// 10456 (kind 5) and 9296 (kind 7), takes far more than a handful of values,
// and changes from record to record inside most blocks. It does not match
// the block header's u2 or u3, the kind, or the block's count.
//
// Kind 4 and kind 6 blocks occur and do not match this stride. They are
// counted in rawCount and left unread. Three vertices versus four is the
// measured record shape; this does not by itself identify the draw call.

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

// One formula-matched record: 3 or 4 vertices, then an unidentified u32.
struct PolyRecord {
  std::uint32_t cornerCount = 0;  // 3 (kind 5) or 4 (kind 7)
  std::array<PolyVertex, 4> corners{};
  std::uint32_t endWord = 0;  // unidentified; see the comment above
};

struct PolyListBlock {
  std::uint32_t kind = 0;   // 5 or 7 for formula-matched blocks
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
