"""Locate a loaded level's own data in the running original, by known plaintext.

The earlier approach scanned every committed page for anything that *looked*
like an occupancy mask -- a byte per cell, mostly 0/1, 2-35% set. That never
finished, and it was the wrong shape of search twice over:

  * it is a pure-Python byte loop over hundreds of megabytes, and
  * it assumes the original rasterises a 2D mask at all. The terrain in .LVL
    is polygon geometry on a lattice (see PLAN.md 1.6), so a byte-per-cell
    mask may simply not exist in the engine. Finding nothing would then prove
    nothing.

This searches for bytes we already know are in the file instead. If the
engine loads a level's surface vertices, those exact doubles are somewhere in
its address space, and `bytes.find` -- which is C, not a Python loop -- walks
a 16 MB region in milliseconds. Once a needle is located, the structures
around it are the engine's own level data, and the collision representation
can be read off rather than guessed at.

Needles, strongest first:

  surface    two consecutive surface vertices, 48 bytes of IEEE doubles.
             Essentially unique; also tells us the vertex array's base.
  grid       a window of the .LVL index array containing several non-empty
             entries, so it is not just a run of 0xFFFFFFFF.
  tile       a tile texture name, which confirms the right level is loaded
             even if the geometry is transformed on load.

Usage (with the game running and a level entered):
    python find_level_data.py MM_CLIMB_002
"""
from __future__ import annotations

import ctypes
import struct
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
from memdump import (  # noqa: E402
    MEM_COMMIT,
    MEMORY_BASIC_INFORMATION,
    PAGE_GUARD,
    PAGE_NOACCESS,
    PROCESS_QUERY_INFORMATION,
    PROCESS_VM_READ,
    find_pids,
    k32,
    read_mem,
)

REPO = Path(__file__).resolve().parents[2]
GAME_DIR = Path(r"C:\progs\Lemmings Revolution")
EMPTY = 0xFFFFFFFF


# ---------------------------------------------------------------- the archive

def read_box(path: Path):
    d = path.read_bytes()
    assert d[:6] == b"LEMBOX", path
    count, _namebytes = struct.unpack_from("<II", d, 6)
    p = 14
    names = []
    for _ in range(count):
        (n,) = struct.unpack_from("<I", d, p)
        p += 4
        names.append(d[p : p + n].decode("latin1"))
        p += n
    (c2,) = struct.unpack_from("<I", d, p)
    p += 4
    offs = struct.unpack_from(f"<{c2}I", d, p)
    p += 4 * c2
    (c3,) = struct.unpack_from("<I", d, p)
    p += 4
    sizes = struct.unpack_from(f"<{c3}I", d, p)
    return d, names, offs, sizes


def level_bytes(level_id: str) -> bytes:
    d, names, offs, sizes = read_box(GAME_DIR / "lemmings.box")
    for i, n in enumerate(names):
        if n.upper().endswith(".LVL") and level_id.upper() in n.upper():
            return d[offs[i] : offs[i] + sizes[i]]
    raise SystemExit(f"no level matching {level_id}")


# ------------------------------------------------------------- level geometry

def parse_chain(b: bytes):
    """The cylinder record chain: ring vertices, radii, surface, tile names."""
    import math

    f64 = lambda p: struct.unpack_from("<d", b, p)[0]
    u32 = lambda p: struct.unpack_from("<I", b, p)[0]

    for at in range(16, min(len(b), 400000)):
        if at + 24 > len(b):
            break
        x, y, z = f64(at), f64(at + 8), f64(at + 16)
        if not (x == 0.0 and y == 0.0 and 10.0 < z < 4096.0):
            continue
        rings = u32(at - 4)
        thick = f64(at - 12)
        if not (1 <= rings <= 64) or not (math.isfinite(thick) and thick > 0):
            continue
        c = 0
        while at + (c + 1) * 24 <= len(b):
            p = at + c * 24
            vx, vy, vz = f64(p), f64(p + 8), f64(p + 16)
            # All three bounds matter. Dropping the z bound lets the word
            # after the vertex run read as one more vertex, and the run then
            # fails the whole-rings check by exactly one.
            if not (
                math.hypot(vx, vz) > 5
                and abs(vy) < 4096
                and abs(vx) < 4096
                and abs(vz) < 4096
            ):
                break
            c += 1
        if not c or c % (rings * 2):
            continue
        steps = c // rings // 2
        p = at + c * 24
        if u32(p) != rings:
            continue
        p += 4 + rings * steps * 8
        _one, sc = u32(p), u32(p + 4)
        p += 8
        surface_at = p
        surface = [
            (f64(p + k * 24), f64(p + k * 24 + 8), f64(p + k * 24 + 16)) for k in range(sc)
        ]
        p += sc * 24
        _z0, tc = u32(p), u32(p + 4)
        p += 8
        tiles = []
        for _ in range(tc * 2):
            (ln,) = struct.unpack_from("<I", b, p)
            tiles.append(b[p + 4 : p + 4 + ln].decode("latin1"))
            p += 4 + ln
        return dict(surface=surface, surface_at=surface_at, tiles=tiles, end=p)
    raise SystemExit("could not parse the cylinder chain")


def grid_needle(b: bytes) -> bytes | None:
    """A window of the index array with several non-empty entries in it."""
    (tl,) = struct.unpack_from("<I", b, 0)
    header = struct.unpack_from("<7I", b, 4 + tl)
    nbytes = header[3]
    token = struct.pack("<I", 8) + b"Cylinder"
    pos = b.find(token)
    if pos < 0:
        return None
    start = -1
    for o in range(pos, min(len(b) - 4, pos + 128)):
        if struct.unpack_from("<I", b, o)[0] == EMPTY:
            start = o
            break
    if start < 0 or start + nbytes > len(b):
        return None
    cells = struct.unpack_from(f"<{nbytes // 4}I", b, start)
    # Find a 16-entry window with the most non-empty values; that is the most
    # distinctive stretch, and a run of pure 0xFFFFFFFF would match anywhere.
    best_i, best_n = -1, 0
    for i in range(0, len(cells) - 16):
        n = sum(1 for v in cells[i : i + 16] if v != EMPTY)
        if n > best_n:
            best_i, best_n = i, n
    if best_n < 3:
        return None
    return b[start + best_i * 4 : start + (best_i + 16) * 4]


# ------------------------------------------------------------------ the scan

def regions(h):
    mbi = MEMORY_BASIC_INFORMATION()
    addr = 0
    while addr < 0x7FFF0000:
        if k32.VirtualQueryEx(h, ctypes.c_void_p(addr), ctypes.byref(mbi), ctypes.sizeof(mbi)) == 0:
            break
        base = mbi.BaseAddress or 0
        size = mbi.RegionSize or 0
        if size == 0:
            break
        readable = (
            mbi.State == MEM_COMMIT
            and not (mbi.Protect & PAGE_GUARD)
            and mbi.Protect != PAGE_NOACCESS
        )
        yield base, size, readable
        nxt = base + size
        if nxt <= base:
            break
        addr = nxt


def find_all(h, needles: dict[str, bytes], limit_per_needle: int = 8):
    """One pass over memory, every needle matched per region with bytes.find."""
    found: dict[str, list[int]] = {k: [] for k in needles}
    scanned = 0
    for base, size, readable in regions(h):
        if not readable:
            continue
        # Read in slabs, overlapping by the longest needle so a match that
        # straddles a slab boundary is not missed.
        overlap = max(len(n) for n in needles.values())
        pos = 0
        while pos < size:
            chunk = min(size - pos, 8 * 1024 * 1024)
            data = read_mem(h, base + pos, chunk)
            if not data:
                break
            scanned += len(data)
            for name, needle in needles.items():
                if len(found[name]) >= limit_per_needle:
                    continue
                at = data.find(needle)
                while at >= 0 and len(found[name]) < limit_per_needle:
                    found[name].append(base + pos + at)
                    at = data.find(needle, at + 1)
            if chunk == size - pos:
                break
            pos += chunk - overlap
    return found, scanned


def main() -> int:
    level_id = sys.argv[1] if len(sys.argv) > 1 else "MM_CLIMB_002"
    blob = level_bytes(level_id)
    chain = parse_chain(blob)

    needles: dict[str, bytes] = {}

    # Two consecutive surface vertices: 48 bytes of doubles. Pick vertices
    # whose coordinates are *not* axis-aligned, because the first vertex of
    # every level is (0, 0, radius) and so begins with sixteen zero bytes --
    # a needle that leads with run-length filler is a weak one.
    surf = chain["surface"]
    picked = []
    for k in range(len(surf) - 1):
        a, b2 = surf[k], surf[k + 1]
        if all(abs(v) > 1e-6 for v in (a[0], a[2], b2[0], b2[2])):
            picked.append(k)
            if len(picked) >= 2 and picked[-1] - picked[0] > 20:
                break
    for n, k in enumerate(picked[:2]):
        at = chain["surface_at"] + k * 24
        needles[f"surface{n}"] = blob[at : at + 48]

    g = grid_needle(blob)
    if g:
        needles["grid"] = g

    tile = next((t for t in chain["tiles"] if t), None)
    if tile:
        needles["tile_name"] = tile.encode("latin1")

    print(f"level {level_id}: {len(chain['surface'])} surface vertices")
    for name, n in needles.items():
        print(f"  needle {name:12s} {len(n):3d} bytes  {n[:16].hex()}...")

    procs = find_pids("Lemmings")
    if not procs:
        print("\nthe game is not running -- start it and enter a level first")
        return 1
    pid, exe = procs[0]
    print(f"\nattached to pid {pid} ({exe})")

    h = k32.OpenProcess(PROCESS_VM_READ | PROCESS_QUERY_INFORMATION, False, pid)
    if not h:
        print("OpenProcess failed")
        return 1
    found, scanned = find_all(h, needles)
    k32.CloseHandle(h)

    print(f"scanned {scanned / (1024 * 1024):.1f} MB\n")
    any_hit = False
    for name in needles:
        hits = found[name]
        any_hit = any_hit or bool(hits)
        if hits:
            print(f"  {name:12s} {len(hits)} hit(s): " + ", ".join(f"{a:#010x}" for a in hits))
        else:
            print(f"  {name:12s} not found")

    if not any_hit:
        print(
            "\nNothing matched. Either no level is loaded, or the engine"
            "\ntransforms this data on load rather than keeping it verbatim."
        )
        return 2
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
