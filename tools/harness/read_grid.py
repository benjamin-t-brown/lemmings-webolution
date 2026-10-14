"""Read the original's own collision grid out of its memory.

Why this exists
---------------
PLAN.md 1.6 concluded there was no occupancy buffer in the engine, on the
strength of a sweep of all 182 MB of committed memory that found no
byte-per-cell mask and no grid with a small alphabet of byte values. That was
a false negative, and the reason is in the shape of the cells: they are
**4-byte flag words**, so a page of them is neither `{0, 1}` nor a
small-alphabet byte grid. The sweep could not have found this buffer.

The grid is reached through the cell probe, which is a plain read of the
shipped bytes (`read_steppers.py` has the disassembler; every address here
was taken from the stock exe, SHA-256
`32675bdb49866e76502713251fdfd2b09078c2481c72ca22ff68ebf4b3737a2f`):

    0040de6a  25 ff 000000   and  eax, 0xff            ; x wraps at 256
    0040de78  3b 91 18040000 cmp  edx, [ecx + 0x418]   ; y clamped to the row count
    0040de9f  c1 e2 08       shl  edx, 8               ; y * 256
    0040dea5  03 c2          add  eax, edx             ; + x
    0040deab  81 c1 1c040000 add  ecx, 0x41c           ; the container
    0040deb1  e8 5a6effff    call 0x00404d10

and the container's element accessor is a flat array of dwords:

    00404d1a  8b 48 0c       mov  ecx, [eax + 0xc]     ; the data pointer
    00404d20  8d 04 91       lea  eax, [ecx + edx*4]   ; base + i*4

The allocator confirms the shape. `FUN_0043AA63(rows)` resizes that container
to `rows * 256` and records the row count, and it has exactly one caller:

    0043aa6d  c1 e0 08       shl  eax, 8               ; rows * 256
    0043aa7a  e8 f1a6fcff    call 0x00405170           ; resize
    0043aa85  89 91 18040000 mov  [ecx + 0x418], edx   ; the row count

So the world object -- the static instance at `0x004C6A10`, which is the same
object the fall stepper probes and whose height `FUN_0040BAA0` returns -- holds
a 256-wide, row-major grid of `u32` flag cells. Hence the two addresses this
script reads.

What the flag bits mean
-----------------------
Read off the instructions that test and set them, not guessed:

    0x0001  solid, destructible. Tested at 0x00424905 (`and edx, 1`) before a
            removal; set by the builder at 0x004101F8 (`or edx, 1`).
    0x0002  indestructible. Tested first at 0x004248E6 (`and edx, 2`), and a
            cell with it set is never cleared.
    0x0080  not landable. The faller's landing probe skips a cell with this
            bit at 0x0041A99A (`and edx, 0x80`), and it is stamped by
            `FUN_0043119B` at 0x004311AE.
    0x0200  stamped from an object's bounding box when the object's type field
            `+0x128` is 0 (0x00428D63, 0x0042AFA3, 0x0042B395, 0x0042B4AB).
    0x2000  the same, for type 1 (0x00428D84, 0x0042AFC4, ...).
    0x4000  the same, for type 2 (0x00428DA5).
    0x0800  the blocker's own bit: cleared at 0x0040E1D5 (`and eax, 0xf7ff`)
            when a blocker goes away.

A removal clears 0x1, 0x2, 0x80 and 0x800 together --
`0x0041B783 and edx, 0xf77c` -- in the stepper that `bash`, `dig` and `mine`
share.

The landing predicate is therefore **non-zero and not 0x80**, which is what
`solid()` below implements, and it is the same test the engine makes at
0x0041A99A. Note that this makes a cell stamped only 0x200 landable: the
object zones are terrain as far as a falling lemming is concerned.

Usage
-----
Needs the game running with a level loaded (`fresh_level.py` puts one up).

    python tools/harness/read_grid.py
    python tools/harness/read_grid.py --out build/live_dump/grid_climb
    python tools/harness/read_grid.py --compare build/live_dump/ref_climb_mask.bin

Writes `<out>.bin` (the raw `u32` cells, little-endian, row-major) and
`<out>.png` (solid cells white, 0x80-only cells grey, empty black).
"""
from __future__ import annotations

import argparse
import struct
import sys
import zlib
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
from lrstate import attach, u32  # noqa: E402
from memdump import read_mem  # noqa: E402

# The static world object, and the two fields of its collision grid. Both are
# absolute: relocations are stripped, so the image always loads at 0x400000.
WORLD = 0x004C6A10
ROW_COUNT = WORLD + 0x418  # 0x004C6E28
GRID_DATA_PTR = WORLD + 0x41C + 0x0C  # 0x004C6E38

GRID_WIDTH = 256  # `and eax, 0xff` at 0x0040DE6A -- the engine's own wrap

# Flag bits, read off the instructions named in the docstring.
BIT_SOLID = 0x0001
BIT_INDESTRUCTIBLE = 0x0002
BIT_NOT_LANDABLE = 0x0080
BIT_ZONE_T0 = 0x0200
BIT_BLOCKER = 0x0800
BIT_ZONE_T1 = 0x2000
BIT_ZONE_T2 = 0x4000

BIT_NAMES = [
    (BIT_SOLID, "solid"),
    (BIT_INDESTRUCTIBLE, "indestructible"),
    (BIT_NOT_LANDABLE, "not-landable"),
    (BIT_ZONE_T0, "zone-type0"),
    (BIT_BLOCKER, "blocker"),
    (BIT_ZONE_T1, "zone-type1"),
    (BIT_ZONE_T2, "zone-type2"),
]

# A row count outside this range means we are reading something that is not the
# grid -- no level loaded, or a wrong address. Our own mask is 192 rows (96
# world units at the measured 2 rows per unit), so anything in this band is
# worth looking at and anything outside it is refused rather than printed.
MIN_ROWS, MAX_ROWS = 8, 4096


def solid(cell: int) -> bool:
    """The engine's own landing test, from 0x0041A99A: non-zero and not 0x80."""
    return cell != 0 and (cell & BIT_NOT_LANDABLE) == 0


def read_grid(handle):
    """The live grid as (rows, list of u32), or a reason it was refused."""
    rows = u32(handle, ROW_COUNT)
    if rows is None:
        return None, f"could not read the row count at {ROW_COUNT:#010x}"
    if not (MIN_ROWS <= rows <= MAX_ROWS):
        return None, (
            f"row count at {ROW_COUNT:#010x} is {rows}, outside {MIN_ROWS}..{MAX_ROWS}. "
            "That is not a loaded level's grid -- is a level actually running?"
        )
    base = u32(handle, GRID_DATA_PTR)
    if not base:
        return None, f"the grid data pointer at {GRID_DATA_PTR:#010x} is null"
    want = rows * GRID_WIDTH * 4
    blob = read_mem(handle, base, want)
    if not blob or len(blob) != want:
        got = 0 if not blob else len(blob)
        return None, (
            f"the data pointer {base:#010x} did not read back {want} bytes (got {got}). "
            "It is not committed memory of that size, so it is not the grid."
        )
    return (rows, list(struct.unpack(f"<{rows * GRID_WIDTH}I", blob))), None


def write_png(path: Path, width: int, height: int, rgb_rows):
    """A minimal 8-bit RGB PNG, so this needs no image library."""
    raw = b"".join(b"\0" + bytes(row) for row in rgb_rows)

    def chunk(tag: bytes, data: bytes) -> bytes:
        return (
            struct.pack(">I", len(data))
            + tag
            + data
            + struct.pack(">I", zlib.crc32(tag + data) & 0xFFFFFFFF)
        )

    path.write_bytes(
        b"\x89PNG\r\n\x1a\n"
        + chunk(b"IHDR", struct.pack(">IIBBBBB", width, height, 8, 2, 0, 0, 0))
        + chunk(b"IDAT", zlib.compress(raw, 9))
        + chunk(b"IEND", b"")
    )


def report(rows: int, cells: list[int]) -> None:
    total = rows * GRID_WIDTH
    n_solid = sum(1 for c in cells if solid(c))
    print(f"grid        {GRID_WIDTH} x {rows}  ({total} cells, u32 each)")
    print(f"            row count read at {ROW_COUNT:#010x}, data at {GRID_DATA_PTR:#010x}")
    print(f"solid       {n_solid}/{total} = {100.0 * n_solid / total:.2f}%"
          "   (non-zero and not 0x80, the engine's test at 0x0041a99a)")
    nonzero = sum(1 for c in cells if c)
    print(f"non-zero    {nonzero}/{total} = {100.0 * nonzero / total:.2f}%")

    print("\nbits set, by cell count:")
    for bit, name in BIT_NAMES:
        n = sum(1 for c in cells if c & bit)
        if n:
            print(f"  {bit:#06x}  {name:<16} {n:>8}  {100.0 * n / total:>6.2f}%")

    print("\ndistinct cell words:")
    hist: dict[int, int] = {}
    for c in cells:
        hist[c] = hist.get(c, 0) + 1
    for word, n in sorted(hist.items(), key=lambda kv: -kv[1])[:16]:
        names = " | ".join(nm for b, nm in BIT_NAMES if word & b) or "-"
        print(f"  {word:#010x}  {n:>8}  {100.0 * n / total:>6.2f}%   {names}")
    if len(hist) > 16:
        print(f"  ... {len(hist) - 16} more distinct words")

    occupied = [y for y in range(rows)
                if any(solid(cells[y * GRID_WIDTH + x]) for x in range(GRID_WIDTH))]
    if occupied:
        print(f"\nrows with any solid cell: {len(occupied)} of {rows}, "
              f"from {min(occupied)} to {max(occupied)}")


def compare(rows: int, cells: list[int], ref: Path) -> None:
    """Diff the live grid against one of our rasterised masks, cell for cell.

    The two disagree on vertical resolution, and that is itself a finding: the
    engine's grid is one row per world unit (96 rows for the 96-unit tube),
    while our mask supersamples at two (192 rows). Where our mask is an exact
    2x, a live row is compared against the pair of our rows that cover it, and
    we count ours as solid if either of them is -- the generous reading, so the
    "we miss" number below is a floor on the disagreement rather than an
    artefact of the resampling.
    """
    blob = ref.read_bytes()
    total = rows * GRID_WIDTH
    ours_rows = len(blob) // GRID_WIDTH
    if len(blob) % GRID_WIDTH or ours_rows % rows:
        print(f"\n{ref}: {len(blob)} bytes = {GRID_WIDTH} x {ours_rows}, which is not a "
              f"whole multiple of the live {GRID_WIDTH} x {rows}. Not diffing.")
        return
    scale = ours_rows // rows

    def ours_at(x: int, y: int) -> bool:
        return any(blob[(y * scale + s) * GRID_WIDTH + x] != 0 for s in range(scale))

    if scale != 1:
        print(f"\nour mask is {GRID_WIDTH} x {ours_rows}, the engine's {GRID_WIDTH} x {rows}"
              f" -- comparing each live row against our {scale} covering rows")
    both = live_only = ours_only = neither = 0
    for i in range(total):
        live = solid(cells[i])
        ours = ours_at(i % GRID_WIDTH, i // GRID_WIDTH)
        if live and ours:
            both += 1
        elif live:
            live_only += 1
        elif ours:
            ours_only += 1
        else:
            neither += 1
    agree = both + neither
    print(f"\nagainst {ref.name}, cell for cell:")
    print(f"  agree               {agree}/{total} = {100.0 * agree / total:.2f}%")
    print(f"  solid in both       {both}")
    print(f"  live only (we miss) {live_only}")
    print(f"  ours only (we add)  {ours_only}")
    if both + live_only:
        print(f"  recall on the engine's solid cells: "
              f"{100.0 * both / (both + live_only):.1f}%")


def main() -> int:
    ap = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    ap.add_argument("--out", default="build/live_dump/live_grid",
                    help="path stem for the .bin and .png (default build/live_dump/live_grid)")
    ap.add_argument("--compare", help="one of our masks (.bin, one byte per cell) to diff against")
    ap.add_argument("--no-write", action="store_true", help="report only, write nothing")
    args = ap.parse_args()

    handle, pid = attach()
    if not handle:
        print("no Lemmings Revolution process. fresh_level.py puts a level up.",
              file=sys.stderr)
        return 2
    print(f"attached to pid {pid}")

    grid, why = read_grid(handle)
    if grid is None:
        print(f"refusing to report: {why}", file=sys.stderr)
        return 1
    rows, cells = grid
    report(rows, cells)

    if not args.no_write:
        stem = Path(args.out)
        stem.parent.mkdir(parents=True, exist_ok=True)
        bin_path = stem.with_suffix(".bin")
        bin_path.write_bytes(struct.pack(f"<{len(cells)}I", *cells))
        rgb = []
        for y in range(rows):
            row = bytearray()
            for x in range(GRID_WIDTH):
                c = cells[y * GRID_WIDTH + x]
                if solid(c):
                    row += b"\xff\xff\xff"
                elif c:
                    row += b"\x60\x60\x60"  # present but not landable
                else:
                    row += b"\x00\x00\x00"
            rgb.append(row)
        png_path = stem.with_suffix(".png")
        write_png(png_path, GRID_WIDTH, rows, rgb)
        print(f"\nwrote {bin_path} ({bin_path.stat().st_size} bytes) and {png_path}")

    if args.compare:
        compare(rows, cells, Path(args.compare))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
