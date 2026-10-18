// The terrain a lemming walks on, digs through and builds across.
//
// Revolution wraps its level around a cylinder, but the simulation does not
// need to know that: walking far enough to the right brings you back to where
// you started, which is exactly a 2D grid whose X axis wraps. So terrain here
// is a flat mask with horizontal wraparound and a hard top and bottom. The
// cylinder is the renderer's problem.
//
// Cells carry a material rather than a bool, because the original stores one
// (its grid holds 0xFFFFFFFF for empty and small indices otherwise) and
// because some materials resist digging.

#pragma once

#include <cstddef>
#include <cstdint>
#include <span>
#include <vector>

namespace webo {
struct CollisionGrid;
}

namespace webo::sim {

enum class Material : std::uint8_t {
  Empty = 0,
  Earth,      // diggable
  Rock,       // blocks bashers and miners
  Steel,      // blocks everything
  Built,      // placed by a builder; diggable like earth
};

inline bool isSolid(Material m) { return m != Material::Empty; }
inline bool isDiggable(Material m) {
  return m == Material::Earth || m == Material::Built;
}

class Terrain {
public:
  Terrain() = default;
  Terrain(int width, int height, Material fill = Material::Empty);

  int width() const { return width_; }
  int height() const { return height_; }
  bool empty() const { return cells_.empty(); }

  // X wraps; Y does not. Reads outside the top are Empty, below the bottom
  // Steel, so a lemming cannot fall out of the world or climb past the sky.
  Material at(int x, int y) const;
  bool solid(int x, int y) const { return isSolid(at(x, y)); }

  void set(int x, int y, Material m);

  // A blocker's field. MEASURED, round 8: the original keeps it as bit 0x800
  // of the same u32 cell the material lives in, so it is a flag on a cell and
  // not a material -- the stamp is `or ah, 8` at 0x0040E312 and the erase is
  // `and eax, 0xf7ff` at 0x0040E1D5, each a read-modify-write that leaves
  // every other bit of the cell alone. Three live blockers stamped four cells
  // apiece and the word went 0x00000000 -> 0x00000800 and back, so the cells
  // it marks are ordinary air that stays air. A separate bit layer here is
  // the faithful shape; folding it into Material would make a blocker's
  // field destroy the terrain under it.
  //
  // Zero cells of all 102 shipped grids carry 0x800
  // (`measure_blocker.py --audit`), so unlike the four directional refusal
  // bits this is runtime state and nothing else.
  bool blockerField(int x, int y) const;
  void setBlockerField(int x, int y, bool on);
  std::size_t blockerFieldCells() const;

  // An anti-grav pad. MEASURED, round 9: cell bit 0x8000 in the stored grid.
  // Every retail level that embeds `anti_grav.a3d` has at least one such cell,
  // and no other level does (18 of 18, `measure_antigrav.py --audit`). The
  // walk probe tests it on the ground under the feet
  // (`trunc(Y) - [+0x44]` at 0x0041C104 / 0x0041C154) and, when an anti-grav
  // object also overlaps, attaches the cartwheel state whose enter hook flips
  // gravity. Authored in the .LVL like the directional refusal bits, not
  // runtime state. See docs/scratch/phase3-round9-antigrav.md.
  bool antiGrav(int x, int y) const;
  void setAntiGrav(int x, int y, bool on);
  std::size_t antiGravCells() const;

  // Dig directional refuse bits. MEASURED round 5 (upright half) + round 30
  // (inverted half): dig bite 0x00415A43 builds mask 0x10, or 0x20 when
  // [+0x44]==-1 (0x00415A86 / 0x00415A96), then `and` at 0x00415B2A.
  //   0x10 = no dig while upright; 0x20 = no dig while inverted.
  // Zero retail cells carry 0x10; eight levels carry 0x20 (MM_DIG_000 etc).
  // Live: inverted digger cleared painted 0x11 and refused painted 0x21;
  // upright dig of 0x21 was round 5. See
  // docs/scratch/phase3-round30-inverted.md.
  bool noDigUpright(int x, int y) const;
  void setNoDigUpright(int x, int y, bool on);
  bool noDigInverted(int x, int y) const;
  void setNoDigInverted(int x, int y, bool on);
  // True when the dig bite must refuse this cell for the given verticalDir
  // (+1 / -1). Steel (0x2) is Material::Steel and is checked separately.
  bool digRefused(int x, int y, int verticalDir) const;

  // A trampoline pad marker. MEASURED, round 13: cell bit 0x1000. Fall land
  // `0x00416433` / `0x00416560` attaches bounce (`0x004B0810`) when the foot
  // cell carries it — before the splat arm. Not in any retail .LVL
  // (`measure_trampoline.py --audit`); trampoline-state enter paints it
  // (`|0x1083` then clear solid). Runtime like blockerField. See
  // docs/scratch/phase3-round13-trampoline.md.
  bool trampoline(int x, int y) const;
  void setTrampoline(int x, int y, bool on);
  std::size_t trampolineCells() const;

  // A water / drown cell. MEASURED, round 14: cell bit 0x40 (kCellFlag0x40).
  // Spawn attaches type 0x2f (vtable 0x004B09C0) as a side-effect; its step
  // probes ground `(trunc(X), trunc(Y)-[+0x44])` and `and 0x40` at
  // 0x00415386, then attaches drown vtable 0x004B0570 (type 0x22) when the
  // 0x004C89D8 object loop finds no match. Live: paint 0x40 onto MM_CLIMB_002
  // pen floor → fall→drown, accumulator to 2.0, leave actor list
  // (`measure_water.py --paint`). Authored in 3 of 102 .LVL grids; water
  // theme levels often carry zone 0x200 instead — that is not this bit.
  // Round 15 MEASURED: acid-as-hazard is this same bit/path (no separate
  // acid cell flag). ^acid_glow / Entrance_acid / LemmingsAcid are not it;
  // type 0x2e Fluid-match sibling never attached live. See
  // docs/scratch/phase3-round14-water.md and phase3-round15-acid.md.
  bool waterHazard(int x, int y) const;
  void setWaterHazard(int x, int y, bool on);
  std::size_t waterHazardCells() const;

  // Splat-exemption zones. MEASURED, round 29: Fluid object[+0x128] 0/1/2
  // stamps 0x200 / 0x2000 / 0x4000 (FUN_00428a47 sites). Fall land
  // FUN_00416433 skips splat when (0x200 && sheet==1) or (0x4000 &&
  // sheet==2). 0x2000 (lava) has no splat arm. Not drown (0x40).
  // docs/scratch/phase3-round29-splat-exempt.md.
  bool zoneWater(int x, int y) const;   // cell bit 0x200
  void setZoneWater(int x, int y, bool on);
  std::size_t zoneWaterCells() const;
  bool zoneAcid(int x, int y) const;    // cell bit 0x4000
  void setZoneAcid(int x, int y, bool on);
  std::size_t zoneAcidCells() const;
  // True when the landing cell / sheet pair matches the binary's arms.
  bool splatExempt(int x, int y, int sheetType) const;

  // What a bomber leaves. MEASURED LIVE, round 7: it is not a circle and
  // there is no radius. `0x0043F510` walks a stored 7 x 7 bitmap at
  // `0x004B1528`, one byte per cell, row-major, and zeroes every cell whose
  // byte is set:
  //
  //     . . # # # . .        37 of the 49 bytes are 1
  //     . # # # # # .
  //     # # # # # # #        0043f536  mov [ebp-0x20], 7
  //     # # # # # # #        0043f53d  mov [ebp-0x1c], 0x4b1528
  //     # # # # # # #        0043f5e9  mov al, [edx]   ; one byte per cell
  //     . # # # # # .        0043f7f5  mov dword ptr [edx], 0
  //     . . # # # . .
  //
  // (x, y) is the exploding lemming's own cell. The window is columns
  // x - 4 .. x + 2 and rows y - 4 .. y + 2: the original anchors at
  // `col0 = trunc(X - 3.5)` and `row0 = trunc(Y - 3.5 + 1.5)` and our x is
  // the truncated live X, so a zero fraction gives x - 4. The bitmap is
  // symmetric top to bottom, so this grid's downward Y needs no flip.
  //
  // Three cells are skipped: `0x2` indestructible (`and edx, 2` at
  // 0x0043F61B), the zone bit `0x200` (0x0043F62B, which this Terrain does
  // not carry) and a blocker's field `0x800` (0x0043F63E). Returns cells
  // removed. The 9 x 9 bitmap at `0x004B155C` belongs to the other caller,
  // `0x0041DF89`, and is not a lemming's bomb.
  std::size_t clearExplosion(int x, int y);
  // Clears an axis-aligned box, the shape bashers and diggers carve.
  std::size_t clearBox(int x, int y, int w, int h);
  // Fills a box, which is what a builder's brick is.
  std::size_t fillBox(int x, int y, int w, int h, Material m);

  // Normalises an X to [0, width).
  int wrapX(int x) const;

  std::size_t solidCells() const;
  std::span<const Material> cells() const { return cells_; }

  // Builds terrain from the surface lattice's index array: 0xFFFFFFFF means
  // empty and anything else is solid. That array is the engine's *render*
  // input, so this is not how a level's terrain is built -- see
  // fromCollisionGrid. Kept because the lattice tooling still uses it.
  static Terrain fromLevelGrid(std::span<const std::uint32_t> grid, int width, int height);

  // Builds terrain from the collision mask stored in the .LVL -- the one the
  // original itself collides against (assets/CollisionGrid.h). Solidity is
  // the engine's own test, and the material comes from the one flag bit that
  // carries a material distinction:
  //
  //   not solid            -> Empty   (zero, or 0x80 not-landable)
  //   0x2 indestructible   -> Steel   (tested first at 0x004248E6, never cleared)
  //   otherwise            -> Earth
  //
  // Rock has no counterpart in the grid. The engine draws one line, between
  // destructible and not, so inventing a second here would be our rule and
  // not the original's; Rock stays unused by this path.
  //
  // Rows are flipped: the file's Y grows upward from the tube floor, matching
  // the lemming's world Y, while this Terrain grows downward so ground is at
  // y+1. Same mapping as worldToMaskPixel. See Terrain.cpp.
  static Terrain fromCollisionGrid(const CollisionGrid& grid);

private:
  std::size_t index(int x, int y) const;

  int width_ = 0;
  int height_ = 0;
  std::vector<Material> cells_;
  // One byte per cell, non-zero where a blocker's field is stamped. Allocated
  // lazily: a level with no blocker on it costs nothing.
  std::vector<std::uint8_t> blocker_;
  // One byte per cell, non-zero where the .LVL stamped 0x8000. Filled by
  // fromCollisionGrid; hand-built tests use setAntiGrav.
  std::vector<std::uint8_t> antiGrav_;
  // Dig refuse 0x10 / 0x20 from the .LVL. Filled by fromCollisionGrid;
  // hand-built tests use setNoDigUpright / setNoDigInverted.
  std::vector<std::uint8_t> noDigUpright_;
  std::vector<std::uint8_t> noDigInverted_;
  // One byte per cell, non-zero where a trampoline pad is stamped. Lazy like
  // blocker_; never filled by fromCollisionGrid (0x1000 is runtime-only).
  std::vector<std::uint8_t> trampoline_;
  // One byte per cell, non-zero where the grid carries 0x40. Filled by
  // fromCollisionGrid; hand-built tests use setWaterHazard.
  std::vector<std::uint8_t> waterHazard_;
  // Zone bits 0x200 / 0x4000 from the .LVL (Fluid-stamped in the original).
  std::vector<std::uint8_t> zoneWater_;
  std::vector<std::uint8_t> zoneAcid_;
};

}  // namespace webo::sim
