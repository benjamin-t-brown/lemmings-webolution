// Terrain collision flags stored as a counted 256 x 96 u32 array in each retail .LVL.
// FUN_0043B640 at 0x0043A0C4 reads into world +0x41C, and FUN_0040DE60 probes those words.
// This grid supplies physics. SurfaceMesh and rasteriseSurface use the separate draw lattice.
// The ordered reader consumes prefix and world sections. The legacy locator starts at:
//   u32 8, "Cylinder", then u32[5] zeros, u32 height (96) and f64 (0.36).
//   u32 N, u32[N] surface indices (49858). FUN_0043B640 at 0x0043A07C loads +0x42C
//                 and shifts the count by 2. The ring mesh in Cylinder.h is separate.
//   u32 R, R x 12 bytes. FUN_0043B9C0 at 0x0043A0A0 loads +0x43C.
//   u32 C, u32[C] collision flags. C equals 256 * height.
//   u32 B, u8[B] light-map bytes (12288). FUN_0043B700 at 0x0043A0E8 loads +0x664.
//                 FUN_00403F40 reads a byte from the map at +0x664 using y * 128 + (x & 0xff) / 2.
//                 That byte indexes the 24-byte palette records at +0x678.
// FUN_00439FD3 passes the height word to setHeight at 0x0043A048. All retail grids have 96 rows.
// The directional refusal bits 0x4, 0x8, 0x10 and 0x20 have known readers and unidentified writers.

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
  // FUN_00428a47 stamps Fluid object[+0x128] types 0/1/2 at 0x00428D63 / 0x00428D84 / 0x00428DA5.
  // Their materials are ^water_glow / ^lava_glow / ^acid_glow. Landing in FUN_00416433 exempts
  // tribes for 0x200+[+0x4c]==1 and 0x4000+[+0x4c]==2. 0x2000 has no exemption. Drowning uses 0x40.
  kCellZoneType0 = 0x0200,  // water zone
  kCellZoneType1 = 0x2000,  // lava zone (no splat exemption)
  kCellZoneType2 = 0x4000,  // acid zone
  // Runtime blocker field (vtable 0x004B07B8). Enter 0x0040E20C ORs ah, 8 at 0x0040E312.
  // Exit 0x0040E0DF clears it at 0x0040E1D5 (25 ff f7 00 00, and eax, 0xf7ff). Other bits survive.
  // Both hooks are virtual with no e8 callers. Stamp trunc(X + [+0x360]), plus 1 when [+0x40] == -1
  // and [+0x360] is -1.0. Rows are trunc(Y) + j * [+0x44], j=0..3 (cmp eax, [0x004B10B8] at 0x0040E2C9).
  // MM_BLOCK_001_A and MM_Block_Bomb_A have the same four-cell footprint (0x00000000 to 0x00000800 in air).
  // Walk 0x0041BFFC checks it at 0x0041C430 / 0x0041C4C9 / 0x0041C563 before and 1,
  // calling turn 0x0042F66B unless [lemming+0x4c] == 3. Bash 0x0040C110 refuses after 0x2 and 0x4/0x8.
  // Mine 0x00418AF9 calls turn at 0x00418B06. Dig 0x00415A43 has no 0x800 check.
  // 0x0043F510 checks it at 0x0043F63E alongside 0x2, 0x200 and 0x1 (callers 0x004082D7 / 0x0041DF89).
  // A stamped air cell meets the solid() condition at 0x0041A99A (inferred landing on the field).
  kCellBlocker = 0x0800,
  // Drown floor bit. Type 0x2f (vtable 0x004B09C0, ctor 0x004151F0) attaches at spawn
  // 0x0042E02C via [lemming+0x374]. Step 0x0041527B tests `and eax, 0x40` (83 e0 40) at 0x00415386 for
  // (trunc(X), trunc(Y)-[+0x44]). With no object match at 0x004C89D8 / 0x00420A52 it
  // starts type 0x22 (ctor 0x00408700, vtable 0x004B0570, "drown" at 0x004BF130).
  // A match starts type 0x2e (ctor 0x00406C20, vtable 0x004B04D0), unused in retail.
  // The drown path triggers with injected 0x40 in H02_JC_1 (^acid_glow) and MM_CLIMB_002,
  // and with authored 0x40 in M12_MT_1 (^lava).
  // Entrance_acid and LemmingsAcid refer to actor type [+0x4c]. 0x0042A8EA uses ^acid_glow for Fluid subtype 2.
  // Entrance_water correlates with 0x200. Drown setters 0x0042092E / 0x00420A0F have no known E8 caller.
  kCellFlag0x40 = 0x0040,
  // With 0x40 clear, 0x00415552 tests `and edx, 0x100` (81 e2 00 01 00 00).
  // A nonzero result from 0x00415800 constructs
  // the 0x00415710 object (vtable 0x004B09D8) and may set lem[+0x5f]=1 at 0x00415627.
  // In MM_CLIMB_002, injecting 0x100 into two floor cells (0x1 to 0x101)
  // set flag5f without triggering drown or type2e.
  // Authored on H08_MT_1, H22_MT_1 and VH06_MT, all without ^acid* materials.
  // No `push 0x100` feeds CELL_STAMP 0x004311D0, and no 0xfffffeff clear follows `call 0x40de60`.
  kCellFlag0x100 = 0x0100,
  // Door/gate vtable 0x004B0F00 stamps solid and steel with this bit (word 0x403).
  // 0x00427554 paints via 0x004275C1 (or edx, 0x403). 0x004275D4 erases via
  // 0x00427641 (and edx, 0xfbfc). The rect is [+0x40 .. +0x44) by [+0x4c .. +0x48).
  // Callers: anim 0x004270DC via 0x00423749 (vt+0x14), init 0x0042727C via 0x0042421D (vt+0x1c),
  // and world pass 0x0043B150 (0x004C8968 / 0x004C8988, flag [obj+0x58]).
  // Pose +0x98 moves toward +0x90. On settle, paint sets +0x58=0 and erase sets +0x58=1.
  // A transition from a nonzero state plays the rumble SFX.
  // Mesh scan 0x004615E9 (0x00462264 / 0x0046226E) uses `and eax, 0x400` at 0x00461704
  // and `and eax, 0x401; cmp eax, 0x401` at 0x00461843.
  // 0x403 also occurs in authored grids. The 0x100 bit is independent.
  kCellFlag0x400 = 0x0400,
  // Directional refusal masks are built from lemming direction. Removal requires both 0x2 and the mask clear.
  //   Bash 0x0040C357 -> 0x0040C0AF: 0x4 at 0x0040C0DC, or 0x8 at 0x0040C0EC if [lemming+0x40] == -1.
  //   Mine 0x004188F6: same pair at 0x004189B0 / 0x004189C0.
  //   Dig 0x00415A43: 0x10 at 0x00415A86, or 0x20 at 0x00415A96 if [lemming+0x44] == -1.
  //   Dig tests steel via `and eax, 2` at 0x00415B1B and uses `and edx, [ebp-0x10]` at 0x00415B2A.
  //   0x0004 refuses rightward bash/mine, 0x0008 leftward bash/mine.
  //   0x0010 refuses upright dig, 0x0020 inverted dig.
  // MM_BASH_002 uses 0x5 (solid | 0x4), and MM_DIG_000 uses 0x21 (solid | 0x20) and 0x3.
  // With [+0x44]==-1, dig refuses 0x21 and clears 0x11 while Y increases.
  kCellNoBashRight = 0x0004,
  kCellNoBashLeft = 0x0008,
  kCellNoDigUpright = 0x0010,
  kCellNoDigInverted = 0x0020,
  // Walk step 0x0041BDD4 calls 0x0041BFFC at 0x0041BEE6. At 0x0041C154 it tests edx & 0x8000
  // on the cell from 0x0041C104 saved at 0x0041C10B. 0x0041C147 probes a different cell.
  // A set bit passes 0x2b to 0x0042ECC9. The writer is unresolved.
  kCellFlag0x8000 = 0x8000,
  // Runtime trampoline marker. Landing at 0x00416560 tests `and edx, 0x1000` and attaches vtable 0x004B0810.
  // Bounce probe 0x0040f309 can reattach it ("Bounce hit trampoline"). Enter 0x0041B838 ORs 0x1083
  // then clears bit 0. Exit 0x0041B683 masks with 0xf77c. The authored .LVL grids omit the bit.
  kCellTrampoline = 0x1000,
};

// Runtime stamp and unstamp are 0x004311D0 and 0x0043127E. Directional writers remain unresolved.
// kCellNoBashRight and related bits come from the .LVL and use and edx, [ebp-0x10].
// kCellBlocker is runtime state: or ah, 8 at 0x0040E312 sets 0x800 on a cell.
// 0x0041036F (`and ecx, 4`) and 0x00461512 (`and edx, 4`) test static bytes 0x004C55C8 / 0x004ECA78.
inline constexpr std::uint32_t kCellUnreadBits = 0;

struct CollisionGrid {
  // X wraps at 256, via and eax, 0xff at 0x0040DE6A.
  static constexpr int kWidth = 256;

  int width = kWidth;
  int height = 0;                    // one cell per world unit; 96 in every retail level
  std::vector<std::uint32_t> cells;  // row-major, cells[y * width + x]

  // Serialized f64 between height and the surface array (0.36 in retail).
  // Inferred match to world+0x408, which 0x43A4EC scales by 32.0 for angular pass bounds.
  double cylinderExtent = 0.0;

  // Empty on structured success; recovered-reader rejection on legacy fallback.
  std::string diagnostic;

  bool empty() const { return cells.empty(); }

  // The cell at (x, y), X wrapping the way the engine wraps it. Out of range
  // vertically returns 0.
  std::uint32_t at(int x, int y) const;

  // Landing probe 0x0041A99A treats nonzero cells with 0x80 clear as solid.
  bool solid(int x, int y) const;

  std::size_t solidCells() const;
};

// Reads the prefix and world in section order. A recovered-reader rejection
// permits the legacy Cylinder walk, with its cause retained in diagnostic.
std::expected<CollisionGrid, std::string> findCollisionGrid(std::span<const std::byte>);

std::expected<CollisionGrid, std::string> loadCollisionGrid(const AssetLibrary&,
                                                            std::string_view name);

}  // namespace webo
