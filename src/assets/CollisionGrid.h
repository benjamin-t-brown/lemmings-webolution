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
//   u32 N, u32[N] N is 49858 in all 102 levels. Per-level surface indices,
//                 loaded into `+0x42C` (`FUN_0043B640` at `0x0043A07C`,
//                 which shifts the count by 2). The identical ring mesh is
//                 a different object (Cylinder.h).
//   u32 R, R x 12 bytes    the 12-byte record array the engine loads into
//                          `+0x43C` (`FUN_0043B9C0`, `0x0043A0A0`)
//   u32 C, u32[C] the collision grid; C == 256 * height
//   u32 B, u8[B]  B is 12288 in all 102 levels. Loaded into `+0x664` by
//                 `FUN_0043B700` at `0x0043A0E8`, which does not shift the
//                 count. `FUN_00403F40` indexes `y * 128 + (x & 0xff) / 2`
//                 and the byte selects a 24-byte record at `+0x678`.
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
// Each named bit was read off the instruction that tests or sets it.
// `0x4`, `0x8`, `0x10` and `0x20` are the four directional refusal bits the
// bash, mine and dig bites test, read in round 5; every bit in a retail
// level's grid now has an instruction behind it, and what is still unread is
// which code, if any, *sets* them. See PLAN.md 1.6 and
// docs/scratch/phase3-round5-live.md.

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
  // Object zones. MEASURED round 29: Fluid object[+0x128] 0/1/2 stamps these
  // at FUN_00428a47 (`0x00428D63` / `0x00428D84` / `0x00428DA5` and sibling
  // sites). Glow ^water_glow / ^lava_glow / ^acid_glow. Fall land
  // FUN_00416433 skips splat for 0x200+[+0x4c]==1 and 0x4000+[+0x4c]==2;
  // 0x2000 has no splat arm. Not drown (0x40). See
  // docs/scratch/phase3-round29-splat-exempt.md.
  kCellZoneType0 = 0x0200,  // water zone
  kCellZoneType1 = 0x2000,  // lava zone (no splat exemption)
  kCellZoneType2 = 0x4000,  // acid zone
  // A BLOCKER'S FIELD. Round 8: read and then measured live, and it is the
  // first cell bit established as runtime state rather than level authoring.
  // Zero cells of all 102 shipped grids carry it
  // (`measure_blocker.py --audit`, 0 of 2,506,752).
  //
  // Written and erased only by the block state (vtable 0x004B07B8), through
  // its enter and exit hooks, neither of which has an `e8` caller anywhere in
  // .text -- they are reached only through the vtable slots:
  //
  //   0x0040E20C  enter: 0x0040E312  80 cc 08           or  ah, 8
  //   0x0040E0DF  exit:  0x0040E1D5  25 ff f7 00 00     and eax, 0xf7ff
  //
  // Both run the same loop: ONE column -- the blocker's own trunc(X), since
  // the hook truncates X + [+0x360] and adds 1 when [+0x40] == -1 and
  // [+0x360] is -1.0 facing left -- and FOUR rows, `trunc(Y) + j * [+0x44]`
  // for j = 0..3 (`cmp eax, [0x004B10B8]` at 0x0040E2C9, that dword being 3),
  // i.e. the feet row and the three above it. Each is a read-modify-write of
  // the whole cell, so every other bit survives.
  //
  // Live, three blockers on MM_BLOCK_001_A and MM_Block_Bomb_A, four cells
  // each, predicted from the above before every click and matched cell for
  // cell, all 0x00000000 -> 0x00000800, within 5 ms of the button press, and
  // nothing else in the 256 x 96 grid touched. Bombing one of them put all
  // four back to 0x00000000 two milliseconds after its state became explode.
  //
  // Who reads it. A .text scan over every immediate 0x800 / 0xf7ff and every
  // high-byte mask landing on bits 8..15 finds, outside the CRT:
  //
  //   0x0041C430 / 0x0041C4C9 / 0x0041C563  the walk probe 0x0041BFFC, at the
  //       feet row and the two above it, each taken BEFORE that cell's own
  //       `and 1` and each falling through to 0x0042F66B -- the turn -- unless
  //       [lemming+0x4c] == 3. MEASURED: ten walkers turned, both facings,
  //       with the probe column equal to the stamped column.
  //   0x0040C110  the bash bite's third cell test (after 0x2 and the
  //       directional 0x4/0x8): a set bit refuses the removal. READ.
  //   0x00418AF9  the mine stepper, which does not merely refuse -- 0x00418B06
  //       calls 0x0042F66B and leaves the bite. READ.
  //   0x0043F63E  in 0x0043F510, called from 0x004082D7 and 0x0041DF89,
  //       alongside tests of 0x2, 0x200 and 0x1 on the same cell. Unread.
  //   the dig bite 0x00415A43 has NO 0x800 test.
  //
  // One consequence that is READ and not measured: the faller's landing probe
  // accepts any non-zero cell with 0x80 clear (0x0041A99A, the basis of
  // `solid()` below), and a stamped air cell reads 0x00000800, so a lemming
  // falling down a blocker's column should land on its field. Nothing was
  // dropped into a field this round -- see the scratch note for the run that
  // would settle it. docs/scratch/phase3-round8-blocker.md.
  kCellBlocker = 0x0800,
  // WATER / DROWN cell. MEASURED, round 14 + 15: effect type 0x2f (vtable
  // 0x004B09C0, constructor 0x004151F0, attached at every spawn 0x0042E02C
  // as a side-effect on [lemming+0x374]) steps at 0x0041527B:
  //   0x00415386  83 e0 40           and eax, 0x40
  // Ground cell is (trunc(X), trunc(Y)-[+0x44]). When set and the object
  // loop at 0x004C89D8 / 0x00420A52 finds no match, attaches drown type
  // 0x22 (ctor 0x00408700, vtable 0x004B0570, enter sets "drown" at
  // 0x004BF130). Sibling type 0x2e (ctor 0x00406C20, vtable 0x004B04D0)
  // when that loop matches — MEASURED unused in retail: absolute refs to
  // 0x004C89D8 are only the two probe reads; live list words stay empty on
  // acid/lava/water Fluid levels; paint 0x40 on H02_JC_1 (^acid_glow) and
  // authored 0x40 on M12_MT_1 (^lava) both attached drown only (0 type2e).
  // Round 15: acid-as-hazard is this same drown path — not a separate bit,
  // not Entrance_acid / LemmingsAcid ([+0x4c] sheet), not ^acid_glow
  // (Fluid subtype 2 visual at 0x0042A8EA). Live paint of 0x40 on
  // MM_CLIMB_002: fall→drown, ~2.0 s, leave actor list (`measure_water.py`
  // / `measure_acid.py`). Authored in 3/102 .LVL grids; not the
  // Entrance_water census (that correlates with zone 0x200). Setter sites
  // 0x0042092E / 0x00420A0F still have no found E8 caller. See
  // docs/scratch/phase3-round14-water.md and phase3-round15-acid.md.
  kCellFlag0x40 = 0x0040,
  // The same step, taken when 0x40 is clear:
  //   0x00415552  81 e2 00 01 00 00  and edx, 0x100
  // That branch constructs the object at 0x00415710 (vtable 0x004B09D8) when
  // 0x00415800 returns nonzero, and may set lem[+0x5f]=1 (0x00415627).
  // MEASURED, round 15: paint 0x100 on MM_CLIMB_002 floor → flag5f flip,
  // zero drown / type2e. Not an acid death bit.
  // Round 31 MEASURED authored-only writer: no `or`/`and`-imm that sets or
  // clears 0x100 on a cell word in .text; no `push 0x100` feeds CELL_STAMP
  // `0x004311D0`; no and-clear `0xfffffeff` near `call 0x40de60`. Authored
  // in 3/102 grids (H08_MT_1, H22_MT_1, VH06_MT) — none are ^acid* levels.
  // See docs/scratch/phase3-round15-acid.md and phase3-round31-cell-writers.md.
  kCellFlag0x100 = 0x0100,
  // DOOR / GATE solid stamp marker. MEASURED round 31 (host already named
  // door/gate-shaped in round 26): painted and erased with solid and
  // indestructible as the word 0x403 by object vtable 0x004B0F00.
  //   0x004275C1  81 ca 03 04 00 00  or edx, 0x403    stored back
  //   0x00427641  81 e2 fc fb 00 00  and edx, 0xfbfc  clears exactly 0x403
  // Paint FN 0x00427554 / erase FN 0x004275D4 loop the object's cell rect
  // [+0x40 .. +0x44) × [+0x4c .. +0x48). Callers: anim step 0x004270DC
  // (from object step 0x00423749 = vt+0x14), init 0x0042727C (from
  // 0x0042421D = vt+0x1c), and world pass 0x0043B150 (lists 0x004C8968 /
  // 0x004C8988 when byte [obj+0x58] is set — paint — or its erase sibling).
  // Pose double +0x98 animates toward +0x90; on settle, paint sets +0x58=0
  // and erase sets +0x58=1. Rumble SFX on nonzero→new state commit is the
  // same object (round 26). Reader: mesh rebuild scan 0x004615E9 (via wrap
  // 0x00462264 / 0x0046226E) at 0x00461704 `and eax, 0x400` and 0x00461843
  // `and eax, 0x401; cmp eax, 0x401`. Authored 0x403 also appears offline
  // (14827 cells / 51 levels) — runtime paint/erase toggles the same shape.
  // Does not change the round-15 0x100 reader story.
  // See docs/scratch/phase3-round31-cell-writers.md.
  kCellFlag0x400 = 0x0400,
  // The four DIRECTIONAL refusal bits. Round 5 read them off the three
  // skills' bites, which is why the scan below never found them: each bite
  // builds a mask on the stack from the lemming's own direction and then
  // `and`s the cell with it, so there is no `and reg, imm` on a cell word
  // anywhere for 0x4, 0x8, 0x10 or 0x20.
  //
  //   bash 0x0040C357 -> 0x0040C0AF   mask 0x4 (0x0040C0DC), or 0x8
  //                                   (0x0040C0EC) when [lemming+0x40] == -1
  //   mine 0x004188F6                 the same pair, 0x004189B0 / 0x004189C0
  //   dig  0x00415A43                 mask 0x10 (0x00415A86), or 0x20
  //                                   (0x00415A96) when [lemming+0x44] == -1
  //
  // and in each case the cell is written only when `cell & 0x2` and
  // `cell & mask` are both clear (dig's test is `and eax, 2` at 0x00415B1B
  // then `and edx, [ebp-0x10]` at 0x00415B2A, `je` to the store). So:
  //
  //   0x0004  cannot be bashed or mined to the RIGHT ([+0x40] == 1)
  //   0x0008  cannot be bashed or mined to the LEFT  ([+0x40] == -1)
  //   0x0010  cannot be dug while upright            ([+0x44] == 1)
  //   0x0020  cannot be dug while inverted           ([+0x44] == -1)
  //
  // The level data agrees with that reading. MM_BASH_002, a bash level,
  // carries 2,016 cells of the word 0x5 (solid | 0x4) and no 0x21;
  // MM_DIG_000 carries 876 of 0x21 (solid | 0x20) and no 0x5, and its only
  // other solid word is 0x3, which no skill may touch. Live, on the stock
  // exe: four diggers on MM_DIG_000 cleared 0x21 cells upright and gave up
  // on 0x3 without touching the grid, and a left-facing basher on
  // MM_BASH_002 cleared five columns of 0x5. Round 30 MEASURED the inverted
  // dig halves on forced-[+0x44]==-1 diggers: painted 0x21 refused (0 diffs),
  // painted 0x11 cleared with Y += 1 per cycle (Y-up). Zero of 102 retail
  // grids carry 0x10. Still unread live: 0x4 facing right. See
  // docs/scratch/phase3-round5-live.md and phase3-round30-inverted.md.
  kCellNoBashRight = 0x0004,
  kCellNoBashLeft = 0x0008,
  kCellNoDigUpright = 0x0010,
  kCellNoDigInverted = 0x0020,
  // Tested on the cell during the walk. The walk step 0x0041BDD4 calls
  // 0x0041BFFC at 0x0041BEE6, and that function does
  //   0x0041C154  81 e2 00 80 00 00  and edx, 0x8000
  // on the word saved at 0x0041C10B from the probe at 0x0041C104. The probe
  // at 0x0041C147 is a different cell and is not the one tested. When the
  // bit is set it pushes 0x2b and calls 0x0042ECC9. No instruction that sets
  // 0x8000 on a cell was found.
  kCellFlag0x8000 = 0x8000,
  // Trampoline / bounce marker. MEASURED, round 13: fall land at
  // 0x00416560 `and edx, 0x1000` attaches bounce vtable 0x004B0810; bounce
  // floor probe 0x0040f309 same bit re-attaches ("Bounce hit trampoline").
  // Zero of 102 .LVL grids carry it (`measure_trampoline.py --audit`);
  // trampoline enter 0x0041B838 paints `| 0x1083` then clears bit 0, exit
  // 0x0041B683 clears with `and 0xf77c`. Runtime state like the blocker
  // field, not level authoring. See docs/scratch/phase3-round13-trampoline.md.
  kCellTrampoline = 0x1000,
};

// Bits with no test, set, or clear on a cell word. There are none left.
//
// tools/harness/read_steppers.py --cell-flags searches .text for test/and/or/
// xor/bt/push immediates, and-clear masks, mov-reg-then-test, and shr-then-
// test-1, and reads every immediate pushed into the stamp 0x004311D0 and the
// unstamp 0x0043127E. It found nothing for 0x4, 0x8 and 0x20 and could not:
// those bits are tested through a mask the bite builds on the stack from the
// lemming's direction, so the instruction is `and edx, [ebp-0x10]` and no
// immediate appears. Round 5 read all four of them off the bash, mine and
// dig bites -- see kCellNoBashRight above -- and confirmed the permissive
// half of two of them live.
// What the search did see, and rejected: `or ah, 8` at 0x0040E312 sets 0x800
// (AH is bits 8..15), the known blocker bit -- round 8 followed it to the
// block state's enter hook and measured the footprint, see kCellBlocker;
// `and ecx, 4` at 0x0041036F and `and edx, 4` at 0x00461512 test static bytes
// at 0x004C55C8 and 0x004ECA78, not the grid.
// Still unread is the *writer*: no instruction sets any of the four on a
// cell. They arrive in the `.LVL` bytes, which is consistent with them being
// level authoring rather than runtime state -- and round 8 sharpened that
// contrast by settling the one bit that is NOT authoring: 0x800 appears in no
// shipped grid at all and is written and erased during play.
inline constexpr std::uint32_t kCellUnreadBits = 0;

struct CollisionGrid {
  // 256 cells around the tube. Not a choice of ours: the engine wraps X with
  // `and eax, 0xff` at 0x0040DE6A.
  static constexpr int kWidth = 256;

  int width = kWidth;
  int height = 0;                    // one cell per world unit; 96 in every retail level
  std::vector<std::uint32_t> cells;  // row-major, cells[y * width + x]

  // The `f64` between the height and the surface array on the same chunk walk:
  // 0.36 in all 102 retail levels. The tube walker's pass bounds use the same
  // value as a fraction of a turn (`0x43A4EC` reads it from `world+0x408` and
  // scales it by 32.0), so it is recorded here rather than assumed. That the
  // runtime `world+0x408` equals this stored field is an inference, not a
  // byte-for-byte pairing; the A2 fixture's nine-view pass used 0.36 fixed.
  double cylinderExtent = 0.0;

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
