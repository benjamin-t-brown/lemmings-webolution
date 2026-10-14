"""Read a level's collision grid straight out of its `.LVL` file.

The grid is **stored**, not computed. It is a length-prefixed array of
256 x 96 `u32` flag cells sitting verbatim in every `.LVL`, and the engine
bulk-reads it into the world object at load:

    0043b654  call 0x00402560          ; N = stream.readU32()
    0043b666  call 0x00405170          ; container.resize(N + slack)
    0043b672  call 0x00405170          ; container.resize(N)
    0043b680  shl  edx, 2              ; N * 4 bytes
    0043b68e  call 0x0043b5b0          ; stream.read(container.data, N*4)

`FUN_0043B640` is called on the world object's `+0x41C` at `0x0043A0C4`,
inside the level-load function `FUN_00439FD3`, and `+0x41C` is the container
`FUN_0040DE60` probes for collision. So the bytes in the file are the bytes
the engine collides against, with no transformation in between.

That was verified rather than inferred: the 98,304 bytes at offset 240,011 of
`COMPILED\\MM_CLIMB_002.LVL` are **byte-identical** to the live grid this
harness read out of the running process (`read_grid.py`,
`build/live_dump/live_grid.bin`), zero bytes differing, and the sequence
occurs exactly once in the file.

Consequences, recorded because they overturn earlier readings:

  * The surface lattice (`grid0`/`grid1`, `SurfaceMesh`, `rasteriseSurface`)
    is the engine's **render** input. Collision does not come from it, so the
    ring stride, the quad rule and the half-step wall rule have no bearing on
    what a lemming stands on.
  * Our rasteriser's sparse masks were never the terrain. `MM_DIG_000` is
    35.0% solid in the file against 1.35% from our raster, and `VH13_KF_1` --
    which our loader rasterises to nothing at all -- carries a real if minimal
    mask of 5.5%.

Finding the chunk: scan for a `u32` count that is a multiple of 256 in a
plausible range, whose payload is entirely small flag words. Across all 102
retail levels that matches **exactly once per level**, every one 256 x 96, so
the fingerprint is specific. A production reader should anchor it in the
chunk order instead of scanning; see PLAN.md.

Needs nothing but a `.LVL` file -- no running game, no exe. Usage:

    python tools/harness/read_lvl_grid.py <file.lvl>            # one level
    python tools/harness/read_lvl_grid.py --dir <dir-of-lvls>   # the corpus
    python tools/harness/read_lvl_grid.py <file.lvl> --compare build/live_dump/live_grid.bin

Extract a level with `lrtool extract <game-dir> "COMPILED\\NAME.LVL" out.lvl`,
or the lot with `lrtool dump <game-dir> <out-dir> .LVL`.
"""

from __future__ import annotations

import argparse
import collections
import glob
import os
import struct
import sys
import zlib

# The grid is 256 cells around the tube -- the engine's own wrap, `and eax,
# 0xff` at 0x0040DE6A -- by one cell per world unit of height. Every retail
# level is 96 rows, but the height is read from the file, not assumed.
GRID_WIDTH = 256

# A cell is a u32 of flags. These are the ones read off the instructions that
# test or set them (PLAN.md 1.6); the scan only relies on cells being small.
FLAG_NAMES = {
    0x0001: "solid",
    0x0002: "indestructible",
    0x0080: "not-landable",
    0x0200: "zone-type0",
    0x0800: "blocker",
    0x2000: "zone-type1",
    0x4000: "zone-type2",
}

# A cell word never comes close to this; an index or a float does. This is what
# separates the grid from every other length-prefixed array in the file.
MAX_CELL = 0x10000
MAX_DISTINCT = 16


def solid(cell: int) -> bool:
    """The engine's own test, from the faller's landing probe at 0x0041A99A:
    a cell is standable when it is non-zero and bit 0x80 is clear."""
    return bool(cell) and not (cell & 0x80)


def find_grid(data: bytes):
    """(count_offset, cells) for the one collision grid in a `.LVL`, or None.

    Scans for a `u32` count that is a whole number of 256-cell rows and whose
    payload is entirely small flag words. The 16-word peek is what keeps this
    cheap: almost every candidate offset dies on it without unpacking the
    whole array.
    """
    n = len(data)
    p = 0
    while p + 4 < n:
        count = struct.unpack_from("<I", data, p)[0]
        if (
            count % GRID_WIDTH == 0
            and GRID_WIDTH * 16 <= count <= GRID_WIDTH * 256
            and p + 4 + count * 4 <= n
        ):
            peek = struct.unpack_from("<16I", data, p + 4)
            if all(v < MAX_CELL for v in peek):
                cells = struct.unpack_from(f"<{count}I", data, p + 4)
                hist = collections.Counter(cells)
                if len(hist) <= MAX_DISTINCT and max(hist) < MAX_CELL:
                    return p, cells
        p += 1
    return None


def write_png(path: str, cells, width: int, height: int):
    """Solid cells black, empty white -- the orientation `worldToMaskPixel`
    uses, row 0 at the top."""
    raw = bytearray()
    for y in range(height):
        raw.append(0)  # filter: none
        for x in range(width):
            raw.append(0 if solid(cells[y * width + x]) else 255)

    def chunk(tag: bytes, payload: bytes) -> bytes:
        body = tag + payload
        return struct.pack(">I", len(payload)) + body + struct.pack(">I", zlib.crc32(body))

    png = b"\x89PNG\r\n\x1a\n"
    png += chunk(b"IHDR", struct.pack(">IIBBBBB", width, height, 8, 0, 0, 0, 0))
    png += chunk(b"IDAT", zlib.compress(bytes(raw)))
    png += chunk(b"IEND", b"")
    with open(path, "wb") as f:
        f.write(png)


def report_one(path: str, compare: str | None, write: bool) -> int:
    data = open(path, "rb").read()
    found = find_grid(data)
    if not found:
        print(f"{path}: no collision grid found", file=sys.stderr)
        return 1
    off, cells = found
    height = len(cells) // GRID_WIDTH
    n_solid = sum(1 for c in cells if solid(c))
    print(f"{os.path.basename(path)}")
    print(f"  count u32 at {off}, {len(cells)} cells at {off + 4}, {GRID_WIDTH} x {height}")
    print(f"  solid {n_solid}/{len(cells)} = {100.0 * n_solid / len(cells):.2f}%")
    hist = collections.Counter(cells)
    print("  cell words:")
    for word, k in hist.most_common():
        names = [n for b, n in FLAG_NAMES.items() if word & b]
        extra = word & ~sum(FLAG_NAMES)
        if extra:
            names.append(f"unknown {extra:#06x}")
        print(f"    {word:#010x}  {k:>7}  {100.0 * k / len(cells):5.2f}%  {' | '.join(names) or '-'}")

    if compare:
        live = open(compare, "rb").read()
        mine = struct.pack(f"<{len(cells)}I", *cells)
        same = mine == live
        diff = sum(1 for a, b in zip(mine, live) if a != b)
        print(f"  against {compare}: {'BYTE-IDENTICAL' if same else 'DIFFERS'}, "
              f"{diff} of {min(len(mine), len(live))} bytes differ")

    if write:
        stem = os.path.splitext(path)[0]
        with open(stem + "_grid.bin", "wb") as f:
            f.write(struct.pack(f"<{len(cells)}I", *cells))
        write_png(stem + "_grid.png", cells, GRID_WIDTH, height)
        print(f"  wrote {stem}_grid.bin and {stem}_grid.png")
    return 0


def report_dir(directory: str) -> int:
    # One glob per case, de-duplicated: Windows matches both patterns against
    # the same file, which would otherwise count every level twice.
    seen = {}
    for pattern in ("*.LVL", "*.lvl"):
        for f in glob.glob(os.path.join(directory, "**", pattern), recursive=True):
            seen[os.path.normcase(os.path.abspath(f))] = f
    files = sorted(seen.values())
    if not files:
        print(f"no .LVL files under {directory}", file=sys.stderr)
        return 1
    heights = collections.Counter()
    words = collections.Counter()
    fractions = []
    missing = []
    for f in files:
        found = find_grid(open(f, "rb").read())
        name = os.path.basename(f)
        if not found:
            missing.append(name)
            continue
        _off, cells = found
        heights[len(cells) // GRID_WIDTH] += 1
        words.update(cells)
        frac = 100.0 * sum(1 for c in cells if solid(c)) / len(cells)
        fractions.append((frac, name))
    fractions.sort()
    print(f"{len(files)} levels, {len(fractions)} with a collision grid, {len(missing)} without")
    if missing:
        print(f"  without: {missing}")
    print(f"  grid heights: {dict(heights)}")
    if fractions:
        mid = fractions[len(fractions) // 2]
        print(f"  solid%: min {fractions[0][0]:.1f} ({fractions[0][1]}), "
              f"median {mid[0]:.1f}, max {fractions[-1][0]:.1f} ({fractions[-1][1]})")
    print("  cell words across the corpus:")
    for word, k in words.most_common(20):
        names = [n for b, n in FLAG_NAMES.items() if word & b]
        extra = word & ~sum(FLAG_NAMES)
        if extra:
            names.append(f"unknown {extra:#06x}")
        print(f"    {word:#010x}  {k:>9}  {' | '.join(names) or '-'}")
    return 0


def main() -> int:
    ap = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    ap.add_argument("lvl", nargs="?", help="a .LVL file")
    ap.add_argument("--dir", help="scan every .LVL under this directory")
    ap.add_argument("--compare", help="a live grid dump to check the chunk against")
    ap.add_argument("--no-write", action="store_true", help="report only, write nothing")
    args = ap.parse_args()

    if args.dir:
        return report_dir(args.dir)
    if not args.lvl:
        ap.print_usage(sys.stderr)
        return 2
    return report_one(args.lvl, args.compare, not args.no_write)


if __name__ == "__main__":
    raise SystemExit(main())
