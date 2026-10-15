// The terrain mask, read out of the `.LVL` where the original keeps it.
//
// This is the collision data, and it is **stored, not derived**. Every level
// file carries a length-prefixed array of 256 x 96 `u32` flag cells, and the
// engine bulk-reads it into the object it collides against -- `FUN_0043B640`
// on the world object's `+0x41C` at `0x0043A0C4`, which is the container
// `FUN_0040DE60` probes. The bytes in the file are the bytes a lemming stands
// on, with no transformation in between.
//
// That was verified against the running original rather than inferred: on
// five levels the array in the file is byte-identical to the grid read out of
// the live process (PLAN.md 1.6, `tools/harness/read_grid.py` for the live
// side and `read_lvl_grid.py` for the file side).
//
// Do not confuse this with `SurfaceMesh` / `rasteriseSurface`. Those read the
// cylindrical vertex lattice, which is what the engine *draws*; the mask here
// is what it collides against. The two are different data and they disagree
// substantially -- the lattice rasterises to 1.35%..23% solid where the stored
// mask is 35%..63%.
//
// Locating the chunk
// ------------------
// Anchored in the chunk order, not by signature. The grid is the third array
// in a fixed sequence that begins at the literal "Cylinder" chunk:
//
//   u32 8, "Cylinder"
//   u32[5]        all zero in all 102 levels
//   u32 height    96 in all 102 levels
//   f64           0.36 in all 102 levels
//   u32 N, u32[N] N is 49858 in all 102 levels -- the tube, which is the same
//                 in every level (Cylinder.h)
//   u32 R, R x 12 bytes    the 12-byte record array the engine loads into
//                          `+0x43C` (`FUN_0043B9C0`, `0x0043A0A0`)
//   u32 C, u32[C] the collision grid; C == 256 * height
//
// That walk lands on the grid in **all 102 retail levels**, and the `height`
// field equals `C / 256` in every one. It is also how the engine gets its row
// count: `FUN_00439FD3` reads a `u32` off the stream and hands it to
// `setHeight` at `0x0043A048`.
//
// One honest caveat on `height`: every retail level is 96 rows, so "this
// field is the height" is consistent with the data rather than proven by it.
// A level of a different height would settle it and none exists.
//
// Cell flags
// ----------
// Each known bit was read off the instruction that tests or sets it; the
// unknown ones are listed because they are common, not rare, and the mask is
// not fully understood until they are read. See PLAN.md 1.6.

#pragma once

#include <cstddef>
#include <cstdint>
#include <expected>
#include <span>
#include <string>
#include <string_view>
#include <vector>

namespace webo {

class AssetLibrary;

// Flag bits in a cell, each with the address that reads it.
enum CollisionFlag : std::uint32_t {
  // Solid and destructible. Tested at 0x00424905, set by the builder's
  // `or 1` at 0x004101F8.
  kCellSolid = 0x0001,
  // Indestructible. Tested first at 0x004248E6 and never cleared.
  kCellIndestructible = 0x0002,
  // Not landable: the faller skips this cell at 0x0041A99A. Stamped by
  // 0x004311AE.
  kCellNotLandable = 0x0080,
  // Object zones, selected by the lemming's `+0x128` type: ten sites in
  // 0x428..0x42B.
  kCellZoneType0 = 0x0200,
  kCellZoneType1 = 0x2000,
  kCellZoneType2 = 0x4000,
  // A blocker's field, cleared at 0x0040E1D5.
  kCellBlocker = 0x0800,
};

// Bits seen in retail data with no instruction read for them yet. They are
// not rare -- across the corpus 0x403 appears on 29,654 cells and 0x21 on
// 20,896 -- so treat a cell as understood only for the bits above.
inline constexpr std::uint32_t kCellUnreadBits =
    0x0004 | 0x0008 | 0x0020 | 0x0040 | 0x0100 | 0x0400 | 0x8000;

struct CollisionGrid {
  // 256 cells around the tube. Not a choice of ours: the engine wraps X with
  // `and eax, 0xff` at 0x0040DE6A.
  static constexpr int kWidth = 256;

  int width = kWidth;
  int height = 0;                    // one cell per world unit; 96 in every retail level
  std::vector<std::uint32_t> cells;  // row-major, cells[y * width + x]

  bool empty() const { return cells.empty(); }

  // The cell at (x, y), X wrapping the way the engine wraps it. Out of range
  // vertically returns 0.
  std::uint32_t at(int x, int y) const;

  // The engine's own solidity test, from the faller's landing probe at
  // 0x0041A99A: non-zero, and bit 0x80 clear. Anything else is a guess.
  bool solid(int x, int y) const;

  std::size_t solidCells() const;
};

// Reads the grid out of a level file. Fails rather than guessing when the
// chunk walk does not land on a well-formed array.
std::expected<CollisionGrid, std::string> findCollisionGrid(std::span<const std::byte>);

std::expected<CollisionGrid, std::string> loadCollisionGrid(const AssetLibrary&,
                                                            std::string_view name);

}  // namespace webo
