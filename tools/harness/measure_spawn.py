"""Compare a live lemming's creation position with the parsed entrance.

The spawn writer is 0x0042DE51 (see docs/scratch/phase2-spawn.md). It stores

    lemming+0x08 = (double) int16 at hatch+0x9A
    lemming+0x10 = (double) int16 at hatch+0x9C
    lemming+0x18 = 0.5

and publishes the lemming in the vector at 0x004CBB18 before the release
counter increments. This script reads that vector and every hatch (vtable
0x004B0F50), and prints the entrance translation parsed from the .LVL the
same way LevelObjects does.

    python tools/harness/measure_spawn.py --entrances MM_CLIMB_002 MM_FLOAT
    python tools/harness/measure_spawn.py --hatch-words MM_CLIMB_002
    python tools/harness/measure_spawn.py --hatch-words   # all 102 + summary
    python tools/harness/measure_spawn.py                 # one snapshot
    python tools/harness/measure_spawn.py --watch         # next lemming born

`--watch` samples the new record immediately and then for about a second, so
a Y that is still the int16 is pre-fall and a Y that has already stepped is
not. It does not enter a level; run goto_level.py first.

`--hatch-words` is the file side of the same three words, found by the shape
of the record rather than by a remembered offset. See hatch_words() below.
"""
from __future__ import annotations

import math
import struct
import sys
import time
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
from diffscan import regions  # noqa: E402
from lrstate import attach, i32, read_mem, u32, ui_state  # noqa: E402
from memdump import k32  # noqa: E402

# Hatch that owns the release tick. measure_spawn.md.
HATCH_VT = 0x004B0F50
# Lemming constructed by the release tick. Overwritten to 0x004B0F6C only by
# the other constructor at 0x00426390, which is not the release tick.
LEMMING_VT = 0x004B10E8
OTHER_VT = 0x004B0F6C
# Vector filled by 0x004230A2 via 0x004233A0. +0 count, +0x0C pointer array.
ACTOR_LIST = 0x004CBB18
RELEASED = 0x004CBA78
PATH_BUFFER = 0x004CBBF8

BOX = Path(r"C:\progs\Lemmings Revolution\lemmings.box")


def f64(h, addr):
    b = read_mem(h, addr, 8)
    if not b or len(b) != 8:
        return None
    return struct.unpack("<d", b)[0]


def i16(h, addr):
    b = read_mem(h, addr, 2)
    if not b or len(b) != 2:
        return None
    return struct.unpack("<h", b)[0]


def loaded_level(h) -> str:
    raw = read_mem(h, PATH_BUFFER, 64)
    return raw.split(b"\0")[0].decode("latin1", "replace") if raw else ""


def actor_ptrs(h) -> list[int]:
    count = i32(h, ACTOR_LIST)
    arr = u32(h, ACTOR_LIST + 0x0C)
    if count is None or arr is None or not 0 < count <= 400 or arr < 0x10000:
        return []
    raw = read_mem(h, arr, count * 4)
    if not raw or len(raw) != count * 4:
        return []
    return list(struct.unpack(f"<{count}I", raw))


def describe_actor(h, ptr: int) -> str | None:
    vt = u32(h, ptr)
    if vt not in (LEMMING_VT, OTHER_VT, HATCH_VT):
        return None
    x, y, z = f64(h, ptr + 0x08), f64(h, ptr + 0x10), f64(h, ptr + 0x18)
    if x is None:
        return None
    kind = {LEMMING_VT: "lemming", OTHER_VT: "lemming-subclass", HATCH_VT: "hatch"}.get(vt, "actor")
    return (
        f"  {kind} {ptr:#010x} vt {vt:#010x}  "
        f"+08 {x:.6f}  +10 {y:.6f}  +18 {z:.6f}"
    )


def hatches(h) -> list[int]:
    """Heap objects whose first dword is the hatch vtable."""
    needle = struct.pack("<I", HATCH_VT)
    found = []
    for base, size in regions(h):
        blob = read_mem(h, base, size)
        if not blob:
            continue
        start = 0
        while True:
            i = blob.find(needle, start)
            if i < 0:
                break
            if i % 4 == 0:
                found.append(base + i)
            start = i + 4
    return found


def describe_hatch(h, ptr: int) -> str:
    x, y, z = f64(h, ptr + 0x08), f64(h, ptr + 0x10), f64(h, ptr + 0x18)
    w98, w9a, w9c = i16(h, ptr + 0x98), i16(h, ptr + 0x9A), i16(h, ptr + 0x9C)
    typ = u32(h, ptr + 0xB0)
    return (
        f"  hatch {ptr:#010x}  doubles +08 {x}  +10 {y}  +18 {z}\n"
        f"    words +98 count {w98}  +9A {w9a}  +9C {w9c}  type +B0 {typ}"
    )


def snapshot(h) -> None:
    print(f"ui {ui_state(h)}  loaded {loaded_level(h)!r}  "
          f"released {i32(h, RELEASED)}  actors {i32(h, ACTOR_LIST)}")
    hs = hatches(h)
    print(f"{len(hs)} hatch vtables")
    for p in hs:
        print(describe_hatch(h, p))
    ptrs = actor_ptrs(h)
    print(f"{len(ptrs)} actor-list entries")
    shown = 0
    for p in ptrs:
        line = describe_actor(h, p)
        if line:
            print(line)
            shown += 1
    print(f"{shown} of those are a lemming or a hatch")


def watch(h, seconds: float = 25.0) -> int:
    """Sample the next actor-list birth, then its Y for one second."""
    print(f"ui {ui_state(h)}  loaded {loaded_level(h)!r}  "
          f"released {i32(h, RELEASED)}", flush=True)
    for p in hatches(h):
        print(describe_hatch(h, p), flush=True)
    before = set(actor_ptrs(h))
    print(f"watching; {len(before)} actors already listed", flush=True)
    t0 = time.perf_counter()
    born = None
    while time.perf_counter() - t0 < seconds:
        now = actor_ptrs(h)
        fresh = [p for p in now if p not in before]
        if fresh:
            # Read the doubles before any printing. The fall integrator is
            # already running by the time the pointer is published, and a
            # print here costs more than a step.
            first = [(p, f64(h, p + 0x08), f64(h, p + 0x10), f64(h, p + 0x18),
                      u32(h, p)) for p in fresh]
            born_at = time.perf_counter() - t0
            born = fresh
            rel = i32(h, RELEASED)
            print(f"birth at t={born_at:.3f}s  released {rel}  "
                  f"new {len(fresh)}", flush=True)
            for p, x, y, z, vt in first:
                print(f"  {p:#010x} vt {vt:#010x}  +08 {x}  +10 {y}  +18 {z}",
                      flush=True)
            break
        before = set(now)
    if not born:
        print("no new actor within the wait")
        return 2
    # Follow Y. The fall stepper subtracts from +0x10 on the 200 ms tick, so
    # a series that stays on the int16 is still the creation value.
    print("follow +10 (vertical) for 1.2s:", flush=True)
    t1 = time.perf_counter()
    last = {}
    while time.perf_counter() - t1 < 1.2:
        t = time.perf_counter() - t1
        for p in born:
            y = f64(h, p + 0x10)
            prev = last.get(p)
            if prev is None or (y is not None and y != prev[1]):
                x = f64(h, p + 0x08)
                z = f64(h, p + 0x18)
                print(f"  t={t:6.3f}s  {p:#010x}  +08 {x}  +10 {y}  +18 {z}",
                      flush=True)
                last[p] = (t, y)
        time.sleep(0.01)
    return 0


def _pose_after(data: bytes, a3d_at: int):
    """LevelObjects.cpp poseAfterA3d, enough to recover the translation."""
    limit = min(len(data), a3d_at + 80)
    obj = None
    for i in range(a3d_at, limit - 13):
        if struct.unpack_from("<I", data, i)[0] == 9 and data[i + 4:i + 13] == b"Object 09":
            obj = i
            break
    if obj is None or obj + 17 > len(data):
        return None
    if struct.unpack_from("<I", data, obj + 13)[0] != 0:
        return None
    start = obj + 17
    if start + 12 * 8 > len(data):
        return None
    doubles = struct.unpack_from("<12d", data, start)
    cos_y, sin_y = doubles[0], doubles[2]
    tx, ty, tz = doubles[9], doubles[10], doubles[11]
    if not all(map(math.isfinite, (tx, ty, tz, cos_y, sin_y))):
        return None
    if abs(cos_y * cos_y + sin_y * sin_y - 1.0) > 0.05:
        return None
    radius = math.hypot(tx, tz)
    if not 20.0 <= radius <= 100.0 or not -10.0 <= ty <= 200.0:
        return None
    ang = math.atan2(-tx, tz) / (2.0 * math.pi)
    if ang < 0:
        ang += 1.0
    return tx, ty, tz, radius, ang * 256.0


def _box():
    sys.path.insert(0, str(Path(__file__).resolve().parents[1] / "proto"))
    from box import Box  # noqa: E402

    return Box(str(BOX))


def level_names() -> list[str]:
    """Every COMPILED\\*.LVL in the box, by bare level name."""
    out = []
    for n in _box().names:
        u = n.upper()
        if u.startswith("COMPILED\\") and u.endswith(".LVL"):
            out.append(n.split("\\")[-1][:-4])
    return sorted(out)


def level_bytes(level: str) -> bytes:
    name = "COMPILED\\" + level.upper()
    if not name.endswith(".LVL"):
        name += ".LVL"
    return _box().read(name)


# --- the hatch record's own spawn words -------------------------------------
#
# The hatch loader is vtable slot 6, 0x00425E50. It calls the base loader
# 0x0043130E first, then reads its own fields straight out of the level
# stream. Stream primitives, all disassembled with read_steppers.py --at:
#
#   0x00402540  u8      0x00423360  u16
#   0x00402560  u32     0x00402580  f64      0x00480BB0  Vec3 (3 x f64)
#
# The tail of 0x0043130E and then 0x00425E50 read, in this order:
#
#   0x00431328  0x00480BB0 -> Vec3 hatch+0x48
#   0x0043133A  0x00480BB0 -> Vec3 hatch+0x60
#               one f64 (the hatch+0x78 reader)
#   0x00425EF1  0x00402560 -> u32  hatch+0xB0   lemming type
#   0x00425F02  0x00423360 -> u16  hatch+0x98   release count
#   0x00425F14  0x00423360 -> u16  hatch+0x9A   spawn X   <- lemming+0x08
#   0x00425F26  0x00423360 -> u16  hatch+0x9C   spawn Y   <- lemming+0x10
#   0x00425F38  0x00402560 -> u32  hatch+0xA0
#   0x00425F49  0x00402560 -> u32  hatch+0xA4
#   0x00425F5A  0x00402560 -> u32  hatch+0xA8
#   0x00425F6B  0x00402540 -> u8
#
# So the locator looks for that run of 7 doubles, not for an offset. Across
# the corpus the two Vec3 are always an axis-aligned 8 x 5 box in unrolled
# space with z 0 and 1, which is what makes the run recognisable:
#
#   Vec3 A = (x0, y0, 0)   Vec3 B = (x0 + 8, y0 + 5, 1)   then f64 0.0
#
# Every field is checked, nothing is assumed from a previous level, and the
# record is reported with the names in front of it so a hit can be eyeballed.
_HATCH_NAMES = ("entrance", "entrance_acid")

# The stored collision grid is 256 x 96, one cell per world unit, in all 102
# retail levels (src/assets/CollisionGrid.h: C / 256 == 96 everywhere).
#
# The Y word is a world Y measured up; the engine turns a world Y into a file
# row by truncating it (its float-to-int helper 0x0049C5AC sets the x87
# rounding field to chop -- `or ah, 0xc` -- before `fistp`), so an integer
# word is already the file row. Terrain::fromCollisionGrid flips file row r to
# terrain row 95 - r. One cell is one world unit; never the lattice 2.
GRID_W, GRID_H = 256, 96


def spawn_cell(x: int, y: int) -> tuple[int, int]:
    """(column, Y-down terrain row) for a hatch's words."""
    return x % GRID_W, max(0, min(GRID_H - 1, GRID_H - 1 - y))


def _integral(v: float) -> bool:
    return math.isfinite(v) and v == math.floor(v) and abs(v) < 1e6


def _names_before(data: bytes, at: int, back: int = 0x40) -> list[tuple[int, str]]:
    """Length-prefixed printable name records in the run just before `at`."""
    out = []
    i = max(0, at - back)
    while i < at:
        if i + 4 > len(data):
            break
        n = struct.unpack_from("<I", data, i)[0]
        if 3 <= n <= 48 and i + 4 + n <= at and all(32 <= c < 127 for c in data[i + 4:i + 4 + n]):
            out.append((i, data[i + 4:i + 4 + n].decode("latin1")))
            i += 4 + n
            continue
        i += 1
    return out


def hatch_words(data: bytes) -> list[dict]:
    """Every hatch record in one level's stream, found by its structure."""
    out = []
    for p in range(0, max(0, len(data) - 79)):
        first = struct.unpack_from("<d", data, p)[0]
        # Cheap reject before unpacking seven doubles.
        if not (_integral(first) and 0.0 <= first <= 4096.0):
            continue
        box3 = struct.unpack_from("<7d", data, p)
        if box3[2] != 0.0 or box3[5] != 1.0 or box3[6] != 0.0:
            continue
        if not all(_integral(v) for v in box3):
            continue
        if box3[3] - box3[0] != 8.0 or box3[4] - box3[1] != 5.0:
            continue
        kind = struct.unpack_from("<I", data, p + 56)[0]
        if kind > 8:
            continue
        count, x, y = struct.unpack_from("<3H", data, p + 60)
        if not (1 <= count <= 200 and x < 256 and y < 256):
            continue
        tail = struct.unpack_from("<3I", data, p + 66)
        names = _names_before(data, p)
        out.append(
            {
                "at": p + 60,          # file offset of the count word
                "count": count,
                "x": x,
                "y": y,
                "type": kind,
                "boxMin": box3[0:3],
                "boxMax": box3[3:6],
                "tail": tail,
                "names": names,
                "model": names[-1][1] if names else "",
            }
        )
    return out


def level_totals() -> dict[str, int]:
    """id -> totalLemmings from COMPILED\\LEVELMAP.DAT (GameData.cpp order)."""
    d = _box().read("COMPILED\\LEVELMAP.DAT")
    out, p = {}, 0
    while p + 4 <= len(d):
        n = struct.unpack_from("<I", d, p)[0]
        p += 4
        if not 0 < n <= 128 or p + n > len(d):
            break
        lid = d[p:p + n].decode("latin1")
        p += n
        n2 = struct.unpack_from("<I", d, p)[0]
        p += 4 + n2
        if p + 12 > len(d):
            break
        _, _, total = struct.unpack_from("<3I", d, p)
        p += 12
        out[lid.upper()] = total
    return out


def print_hatch_words(levels: list[str]) -> None:
    """One table per level, then the corpus distribution."""
    all_levels = levels or level_names()
    nolevel, records = [], 0
    off_model, bad_name, off_total = [], [], []
    totals = level_totals()
    for level in all_levels:
        data = level_bytes(level)
        rows = hatch_words(data)
        ents = entrances(level)
        if not rows:
            nolevel.append(level)
        print(f"{level}: {len(rows)} hatch record(s), {len(ents)} entrance placement(s)")
        for h in rows:
            records += 1
            if h["model"] not in _HATCH_NAMES:
                bad_name.append((level, h["model"]))
            # Nearest entrance placement in (unrolled column, world Y). Column
            # alone is not enough: VH04_JC1 stacks two entrances on column 40.
            # Pairing is geometric on purpose -- it must not assume the 2.
            near, best, dcol = None, None, None
            for name, tx, ty, tz, radius, column in ents:
                dx = abs(((column - h["x"]) + 128.0) % 256.0 - 128.0)
                score = math.hypot(dx, ty - h["y"])
                if best is None or score < best:
                    near, best, dcol = (name, tx, ty, tz, radius, column), score, dx
            ey = near[2] if near else None
            note = ""
            if ey is None:
                note = "  NO ENTRANCE PLACEMENT"
            elif ey - h["y"] != 2.0:
                note = f"  entranceY {ey:g} - y = {ey - h['y']:g}, NOT 2"
                off_model.append((level, h["x"], h["y"], ey, dcol))
            cx, cy = spawn_cell(h["x"], h["y"])
            print(
                f"  words@{h['at']:#08x} model {h['model']!r} type {h['type']}  "
                f"count {h['count']:3d}  x {h['x']:3d}  y {h['y']:3d}  "
                f"-> cell ({cx}, {cy})  "
                f"box x {h['boxMin'][0]:g}..{h['boxMax'][0]:g} "
                f"y {h['boxMin'][1]:g}..{h['boxMax'][1]:g}  "
                f"+A0/A4/A8 {h['tail']}"
            )
            if near:
                print(
                    f"      nearest entrance {near[0]} column {near[5]:.4f} "
                    f"(|dx| {dcol:.4f})  world Y {near[2]:g}{note}"
                )
            else:
                print(f"     {note}")
        # Side observation, not used by the sim: the count words sum to the
        # level's own lemming total. Round 3 wires position only.
        want = totals.get(level.upper())
        have = sum(h["count"] for h in rows)
        if want is not None and rows and have != want:
            off_total.append((level, have, want))
    print()
    print(f"SUMMARY  levels {len(all_levels)}  hatch records {records}")
    print(f"  levels with no hatch record: {len(nolevel)} {nolevel}")
    print(f"  records whose model name is not {_HATCH_NAMES}: {bad_name}")
    print(f"  records whose y is NOT entranceY - 2: {len(off_model)}")
    for level, x, y, ey, dcol in off_model:
        print(f"    {level}: x {x} y {y}, nearest entrance world Y {ey:g} "
              f"(column |dx| {dcol:.4f})")
    print(f"  levels where sum(count word) != LEVELMAP totalLemmings: "
          f"{len(off_total)} {off_total}")


def entrances(level: str) -> list[tuple]:
    """Parsed Entrance*.a3d translations in one retail level."""
    data = level_bytes(level)
    out = []
    i = 0
    while i + 8 <= len(data):
        n = struct.unpack_from("<I", data, i)[0]
        if 12 <= n <= 40 and i + 4 + n <= len(data):
            raw = data[i + 4:i + 4 + n]
            if raw.endswith(b".a3d") and raw.startswith(b"Entrance") and all(32 <= c < 127 for c in raw):
                pose = _pose_after(data, i)
                if pose:
                    out.append((raw.decode("latin1"),) + pose)
                i += 4 + n
                continue
        i += 1
    return out


def print_entrances(levels: list[str]) -> None:
    for level in levels:
        rows = entrances(level)
        print(f"{level}: {len(rows)} entrance placement(s)")
        for name, tx, ty, tz, radius, column in rows:
            print(
                f"  {name}  world ({tx:.6f}, {ty:.6f}, {tz:.6f})  "
                f"radius {radius:.3f}  unrolled column {column:.4f}  world Y {ty:.6f}"
            )


def main() -> int:
    args = sys.argv[1:]
    if "--entrances" in args:
        levels = [a for a in args if not a.startswith("-")]
        if not levels:
            print("name at least one level")
            return 2
        print_entrances(levels)
        return 0
    if "--hatch-words" in args:
        # No level named means the whole corpus, with the distribution.
        print_hatch_words([a for a in args if not a.startswith("-")])
        return 0
    h, pid = attach()
    if not h:
        print("the game is not running")
        return 1
    print(f"pid {pid}")
    try:
        if "--watch" in args:
            return watch(h)
        snapshot(h)
        return 0
    finally:
        k32.CloseHandle(h)


if __name__ == "__main__":
    raise SystemExit(main())
