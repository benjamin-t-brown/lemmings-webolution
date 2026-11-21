// Terrain uses an X-wrapped, Y-down grid to represent the cylinder.
// Material controls excavation resistance. The original render lattice
// uses 0xFFFFFFFF for empty and small indices for surface pieces.

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

  // X wraps. Out-of-grid probes return Empty above and Steel below.
  Material at(int x, int y) const;
  bool solid(int x, int y) const { return isSolid(at(x, y)); }

  // Replacing a cell's material also clears its authored Basher refusal bits.
  void set(int x, int y, Material m);
  // Gate OR 0x403 / AND 0xFFFFFBFC. Other flag planes remain intact;
  // imported residual words retain their original landability on clear.
  void setGateCollision(int x, int y, bool closed);

  // The renderer uses this version to skip terrain updates when cells are unchanged.
  std::uint64_t version() const { return version_; }

  // The blocker field is a runtime plane for cell bit 0x800.
  // The original stamps with or ah, 8 at 0x0040E312 and clears with
  // and eax, 0xf7ff at 0x0040E1D5. An air word of 0x00000000 becomes
  // 0x00000800 and remains non-solid.
  bool blockerField(int x, int y) const;
  void setBlockerField(int x, int y, bool on);
  std::size_t blockerFieldCells() const;

  // Authored anti_grav.a3d markers use cell bit 0x8000. At 0x0041C104 /
  // 0x0041C154 the walker samples trunc(Y) - [+0x44] and requires a matching
  // object overlap before entering the cartwheel that flips gravity.
  bool antiGrav(int x, int y) const;
  void setAntiGrav(int x, int y, bool on);
  std::size_t antiGravCells() const;

  // Directional refusal for a Basher facing right (+1) or left (-1).
  bool bashRefused(int x, int y, int facing) const;

  // Dig bite 0x00415A43 selects cell mask 0x10 upright or 0x20 inverted
  // using [+0x44] at 0x00415A86 / 0x00415A96. It tests the mask at 0x00415B2A.
  // MM_DIG_000 uses the inverted refusal bit.
  bool noDigUpright(int x, int y) const;
  void setNoDigUpright(int x, int y, bool on);
  bool noDigInverted(int x, int y) const;
  void setNoDigInverted(int x, int y, bool on);
  // True when the dig bite must refuse this cell for the given verticalDir
  // (+1 / -1). Steel (0x2) is Material::Steel and is checked separately.
  bool digRefused(int x, int y, int verticalDir) const;

  // The trampoline state paints runtime bit 0x1000 (OR 0x1083, then clears
  // solidity). Fall land at 0x00416433 / 0x00416560 attaches bounce
  // (0x004B0810) on this bit before checking for splat.
  bool trampoline(int x, int y) const;
  void setTrampoline(int x, int y, bool on);
  std::size_t trampolineCells() const;

  // Hazard bit 0x40 (kCellFlag0x40) serves both water and acid. Spawn
  // attaches type 0x2f (vtable 0x004B09C0), which tests
  // (trunc(X), trunc(Y)-[+0x44]) at 0x00415386. With no match in the
  // 0x004C89D8 object list it attaches drown (type 0x22, 0x004B0570).
  // The type 0x2e Fluid-match sibling and ^acid_glow / Entrance_acid /
  // LemmingsAcid are separate from the cell hazard.
  bool waterHazard(int x, int y) const;
  void setWaterHazard(int x, int y, bool on);
  std::size_t waterHazardCells() const;

  // Fluid[+0x128] 0/1/2 stamps 0x200 / 0x2000 / 0x4000 at FUN_00428a47.
  // FUN_00416433 skips splat for 0x200 with sheet 1 or 0x4000 with sheet 2.
  // Lava zone 0x2000 provides no exemption. Drown uses bit 0x40.
  bool zoneWater(int x, int y) const;   // cell bit 0x200
  void setZoneWater(int x, int y, bool on);
  std::size_t zoneWaterCells() const;
  // Receiver painters 0x004311D0 (set) / 0x0043127E (clear) edit only bit
  // 0x200. Truncate the original Y-up AABB bounds toward zero and exclude
  // the upper bounds. Set adds one width when ftol(maxX) < minX. Clear
  // uses the raw bound. Wrap X and clamp Y like 0x0040DE60, then flip Y.
  void paintZoneWaterRect(double minX, double minY, double maxX, double maxY, bool set);
  bool zoneAcid(int x, int y) const;    // cell bit 0x4000
  void setZoneAcid(int x, int y, bool on);
  std::size_t zoneAcidCells() const;
  // True when the landing cell / sheet pair matches the binary's arms.
  bool splatExempt(int x, int y, int sheetType) const;

  // 0x0043F510 cuts with the row-major 7 x 7 bitmap at 0x004B1528 (37 set bytes).
  // 0043f536 sets width [ebp-0x20] and 0043f53d loads 0x4b1528 into
  // [ebp-0x1c]. 0043f5e9 reads a byte and 0043f7f5 clears its cell.
  // The original anchors at trunc(X - 3.5), trunc(Y - 3.5 + 1.5).
  // The feet-cell window here spans x-4..x+2, y-4..y+2.
  // Vertical symmetry lets the bitmap serve the Y-down grid.
  // Skip indestructible 0x2 (0x0043F61B) and blocker bit 0x800 (0x0043F63E).
  // The original also skips zone bit 0x200 (0x0043F62B). The 9 x 9 bitmap
  // at 0x004B155C belongs to 0x0041DF89. Returns the cells removed.
  std::size_t clearExplosion(int x, int y);
  // Clears an axis-aligned box, the shape bashers and diggers carve.
  std::size_t clearBox(int x, int y, int w, int h);
  // Fills a box, which is what a builder's brick is.
  std::size_t fillBox(int x, int y, int w, int h, Material m);

  // Normalises an X to [0, width).
  int wrapX(int x) const;

  std::size_t solidCells() const;
  std::span<const Material> cells() const { return cells_; }

  // Build a mask from render-lattice indices for the lattice tools.
  // 0xFFFFFFFF is empty. Every other index is solid.
  static Terrain fromLevelGrid(std::span<const std::uint32_t> grid, int width, int height);

  // Import the .LVL collision mask (assets/CollisionGrid.h). Flip its Y-up
  // rows to match worldToMaskPixel, including MM_CLIMB_002's pen floor.
  // Zero and 0x80 non-landable words map to Empty. Indestructible 0x2
  // (0x004248E6) maps to Steel, and other landable cells map to Earth.
  static Terrain fromCollisionGrid(const CollisionGrid& grid);

private:
  std::size_t index(int x, int y) const;

  int width_ = 0;
  int height_ = 0;
  std::uint64_t version_ = 0;
  std::vector<Material> cells_;
  std::vector<std::uint32_t> gateResidualWords_;
  // Authored 0x04 / 0x08 bits on the live cells. Allocated on first import.
  std::vector<std::uint8_t> bashRefusalFlags_;
  // A lazy byte plane for the runtime blocker field.
  std::vector<std::uint8_t> blocker_;
  // Imported anti-grav bit 0x8000, one byte per cell.
  std::vector<std::uint8_t> antiGrav_;
  // Imported dig refusal bits 0x10 / 0x20.
  std::vector<std::uint8_t> noDigUpright_;
  std::vector<std::uint8_t> noDigInverted_;
  // Lazy byte plane for runtime trampoline bit 0x1000.
  std::vector<std::uint8_t> trampoline_;
  // Imported drown bit 0x40, one byte per cell.
  std::vector<std::uint8_t> waterHazard_;
  // Zone bits 0x200 / 0x4000 from the .LVL (Fluid-stamped in the original).
  std::vector<std::uint8_t> zoneWater_;
  std::vector<std::uint8_t> zoneAcid_;
};

}  // namespace webo::sim
