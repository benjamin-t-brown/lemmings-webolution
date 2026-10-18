"""Round 14: water hazard → drown via cell bit 0x40.

Static (stock exe, image base 0x400000), before any live write:

    Every lemming spawn (0x0042E02C) attaches type 0x2f (vtable 0x004B09C0)
    as a side-effect on [lemming+0x374], then fall as the main state.

    0x2f step 0x0041527B:
        004152f0  sub eax, [ecx+0x44]     ; ground row = trunc(Y) - vdir
        00415379  call 0x40de60           ; cell (trunc X, that row)
        00415386  and eax, 0x40
        if set:
          loop 0x004C89D8 / call 0x420a52 → attach type 0x2e
          else → drown ctor 0x00408700, vtable 0x004B0570, type 0x22
        enter 0x0040874c pushes "drown" at 0x004BF130
        step 0x00408766 accumulates dt at state+8; > 2.0 → return 2 (die)

Offline: 0x40 is rare in .LVL (3 levels); Entrance_water.a3d (20) correlates
with zone 0x200, not authored 0x40 — so water paints 0x40 at runtime (Fluid).

Modes
-----
    python tools/harness/measure_water.py --audit
    python tools/harness/measure_water.py --enter MM_CLIMB_002 --paint \\
            --cols 32,33 --row 31 --watch 12 --hz 80
    python tools/harness/measure_water.py --enter MM_W_012 --watch-only 40
    python tools/harness/measure_water.py --enter H18_MG_1 --watch-only 40

Cap --hz. Do not put UCRT64 bin on PATH here. Actor list 0x004CBB18 only.
"""
from __future__ import annotations

import argparse
import os
import struct
import sys
import time
from pathlib import Path

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))

from lrstate import attach, i32, u32, ui_state  # noqa: E402
from measure_spawn import LEMMING_VT, OTHER_VT, actor_ptrs  # noqa: E402
from memdump import k32, read_mem  # noqa: E402
from read_grid_siblings import levels_from_box  # noqa: E402
from read_lvl_grid import find_grid  # noqa: E402
import measure_stepup as SU  # noqa: E402

WATER_BIT = 0x40
ZONE0, ZONE1, ZONE2 = 0x200, 0x2000, 0x4000
OFF_X, OFF_Y = 0x08, 0x10
OFF_FACING, OFF_VDIR = 0x40, 0x44
OFF_TYPE = 0x4C
OFF_STATE = 0x340
LEM_READ = 0x344

VT_WALK = 0x004B0D28
VT_FALL = 0x004B0A20
VT_DROWN = 0x004B0570
VT_TYPE2E = 0x004B04D0  # sibling attach when 0x4c89d8 match
VT_NAMES = {
    VT_WALK: "walk",
    VT_FALL: "fall",
    VT_DROWN: "drown",
    VT_TYPE2E: "type2e",
    0x004B0960: "climb",
    0x004B0AF8: "flip",
    0x004B0668: "splat",
    0x004B0CA8: "cartwheel",
    0x004B0810: "bounce",
    0x004B0680: "fall-680",
    0x004B09C0: "fluid-probe",
}


def write_mem(h, addr: int, data: bytes) -> bool:
    return SU.write_mem(h, addr, data)


def grid_addr(h, col: int, row: int):
    return SU.grid_addr(h, col, row)


def read_cell(h, col: int, row: int):
    return SU.read_cell(h, col, row)


def paint(h, cells: list[tuple[int, int]]):
    """OR WATER_BIT onto cells. Returns [(addr, old, col, row, new)]."""
    saved = []
    for col, row in cells:
        a = grid_addr(h, col, row)
        if a is None:
            print(f"  skip ({col},{row}): out of range", flush=True)
            continue
        old = read_cell(h, col, row)
        if old is None:
            continue
        new = old | WATER_BIT
        saved.append((a, old, col, row, new))
        write_mem(h, a, struct.pack("<I", new))
        print(f"  paint ({col},{row}) {old:#010x} -> {new:#010x}", flush=True)
    return saved


def restore(h, saved) -> None:
    for a, old, col, row, _new in reversed(saved):
        write_mem(h, a, struct.pack("<I", old or 0))
        print(f"  restore ({col},{row}) -> {old:#010x}", flush=True)
    print(f"restored {len(saved)} grid cells", flush=True)


def sample(h, ptr: int, t: float):
    buf = read_mem(h, ptr, LEM_READ)
    if not buf or len(buf) != LEM_READ:
        return None
    if struct.unpack_from("<I", buf, 0)[0] not in (LEMMING_VT, OTHER_VT):
        return None
    sp = struct.unpack_from("<I", buf, OFF_STATE)[0]
    vt = (u32(h, sp) or 0) if sp else 0
    stype = (u32(h, sp + 0x14) or 0) if sp else 0
    acc = 0.0
    if sp and vt == VT_DROWN:
        ab = read_mem(h, sp + 8, 8)
        if ab and len(ab) == 8:
            acc = struct.unpack("<d", ab)[0]
    return {
        "t": t,
        "ptr": ptr,
        "x": struct.unpack_from("<d", buf, OFF_X)[0],
        "y": struct.unpack_from("<d", buf, OFF_Y)[0],
        "facing": struct.unpack_from("<i", buf, OFF_FACING)[0],
        "vdir": struct.unpack_from("<i", buf, OFF_VDIR)[0],
        "type": struct.unpack_from("<i", buf, OFF_TYPE)[0],
        "vt": vt,
        "stype": stype,
        "acc": acc,
        "name": VT_NAMES.get(vt, f"{vt:#010x}"),
        "strip": struct.unpack_from("<i", buf, 0x128 + 0x1C8)[0],
    }


def lemmings(h) -> list[int]:
    return [p for p in actor_ptrs(h) if u32(h, p) in (LEMMING_VT, OTHER_VT)]


def live_bit_cells(h, bit: int = WATER_BIT) -> set[tuple[int, int]]:
    rows = i32(h, SU.ROW_COUNT) or 96
    base = u32(h, SU.GRID_DATA_PTR)
    out: set[tuple[int, int]] = set()
    if not base:
        return out
    raw = read_mem(h, base, 256 * rows * 4)
    if not raw:
        return out
    for i in range(0, len(raw), 4):
        if struct.unpack_from("<I", raw, i)[0] & bit:
            out.add((i // 4 & 0xFF, i // 4 >> 8))
    return out


def audit() -> int:
    lvls = levels_from_box(Path(r"C:\progs\Lemmings Revolution"))
    ew, ea, bit40 = set(), set(), set()
    z0, z1, z2 = set(), set(), set()
    for name, data in lvls.items():
        if b"Entrance_water" in data:
            ew.add(name)
        if b"Entrance_acid" in data:
            ea.add(name)
        _off, cells = find_grid(data)
        if any(c & WATER_BIT for c in cells):
            bit40.add(name)
        if any(c & ZONE0 for c in cells):
            z0.add(name)
        if any(c & ZONE1 for c in cells):
            z1.add(name)
        if any(c & ZONE2 for c in cells):
            z2.add(name)
    print(f"levels {len(lvls)}")
    print(f"Entrance_water.a3d: {len(ew)}")
    print(f"Entrance_acid.a3d: {len(ea)}")
    print(f"any cell & 0x40: {len(bit40)} -> {sorted(bit40)}")
    print(f"Entrance_water & 0x40: {len(ew & bit40)}")
    print(f"Entrance_water & 0x200: {len(ew & z0)} / {len(ew)}")
    print(f"Entrance_water & 0x2000: {len(ew & z1)} / {len(ew)}")
    print(f"Entrance_water & 0x4000: {len(ew & z2)} / {len(ew)}")
    print(f"Entrance_acid & 0x200: {len(ea & z0)} / {len(ea)}")
    print(f"Entrance_acid & 0x2000: {len(ea & z1)} / {len(ea)}")
    print(f"Entrance_acid & 0x4000: {len(ea & z2)} / {len(ea)}")
    print("NOTE: drown detection reads live cell bit 0x40 (0x00415386);"
          " authored 0x40 is not the water census.")
    return 0


def watch(h, seconds: float, hz: float, painted: set[tuple[int, int]]):
    period = 1.0 / hz if hz > 0 else 0.0
    t0 = time.perf_counter()
    last = {}
    drowns = []
    type2e = []
    deaths = []
    print(f"watching {seconds}s at {hz} Hz; painted {sorted(painted)[:24]}"
          f"{'...' if len(painted) > 24 else ''}", flush=True)
    while time.perf_counter() - t0 < seconds:
        t = time.perf_counter() - t0
        alive = set()
        for p in lemmings(h):
            alive.add(p)
            s = sample(h, p, t)
            if not s:
                continue
            prev = last.get(p)
            under = (int(s["x"]) & 0xFF, int(s["y"]) - s["vdir"])
            if prev is None:
                print(f"  t{t:7.3f}s {p:#010x} birth x={s['x']:.3f}"
                      f" y={s['y']:.3f} vdir={s['vdir']:+d} +0x4c={s['type']}"
                      f" {s['name']} stype={s['stype']:#x} strip {s['strip']}"
                      f" under={under} on_paint={under in painted}",
                      flush=True)
            elif prev["vt"] != s["vt"]:
                print(f"  t{t:7.3f}s {p:#010x} STATE {prev['name']}->"
                      f" {s['name']}  x={s['x']:.3f} y={s['y']:.3f}"
                      f" vdir={s['vdir']:+d} +0x4c={s['type']}"
                      f" stype={s['stype']:#x} acc={s['acc']:.3f}"
                      f" strip {s['strip']}"
                      f" under={under} on_paint={under in painted}",
                      flush=True)
                if s["vt"] == VT_DROWN:
                    drowns.append(s)
                if s["vt"] == VT_TYPE2E:
                    type2e.append(s)
            elif s["vt"] == VT_DROWN and (s["acc"] - prev["acc"]) >= 0.15:
                print(f"  t{t:7.3f}s {p:#010x} DROWN acc {prev['acc']:.3f}->"
                      f" {s['acc']:.3f} x={s['x']:.3f} y={s['y']:.3f}",
                      flush=True)
            last[p] = s
        for p, prev in list(last.items()):
            if p not in alive and prev["vt"] == VT_DROWN:
                deaths.append(prev)
                print(f"  t{t:7.3f}s {p:#010x} LEFT actor list after drown"
                      f" (last acc={prev['acc']:.3f})", flush=True)
                del last[p]
            elif p not in alive:
                del last[p]
        if period:
            time.sleep(period)
    print(f"\n{len(drowns)} drown attaches, {len(type2e)} type2e,"
          f" {len(deaths)} post-drown removals", flush=True)
    return drowns, type2e, deaths


def do_paint(h, cols: list[int], row: int, watch_s: float, hz: float,
             wait: float = 20.0) -> int:
    import assign_skill as A

    print(f"waiting up to {wait}s for lemmings...", flush=True)
    A.wait_for_lemmings(h, wait)
    for p in lemmings(h)[:8]:
        s = sample(h, p, 0.0)
        if s:
            print(f"  pre {p:#010x} x={s['x']:.3f} y={s['y']:.3f}"
                  f" face={s['facing']:+d} vdir={s['vdir']:+d}"
                  f" +0x4c={s['type']} {s['name']}", flush=True)
    cells = [(c & 0xFF, row) for c in cols]
    print("PREDICTION: a lemming whose ground cell (truncX, truncY-vdir)"
          f" is one of {cells} attaches drown vtable 0x004B0570"
          " (type 0x22), then leaves the actor list after ~2.0 s.",
          flush=True)
    for c, r in cells:
        word = read_cell(h, c, r)
        print(f"  before ({c},{r}) = "
              f"{word:#010x}" if word is not None else "<none>", flush=True)
    saved = paint(h, cells)
    painted = {(s[2], s[3]) for s in saved}
    try:
        drowns, type2e, deaths = watch(h, watch_s, hz, painted)
    finally:
        restore(h, saved)
        for c, r in cells:
            word = read_cell(h, c, r)
            print(f"  after  ({c},{r}) = "
                  f"{word:#010x}" if word is not None else
                  f"  after  ({c},{r}) = <unreadable>", flush=True)
    ok = len(drowns) > 0
    print(f"verdict: {'PASS' if ok else 'FAIL'} "
          f"(drown={len(drowns)} type2e={len(type2e)} deaths={len(deaths)})",
          flush=True)
    return 0 if ok else 2


def main() -> int:
    ap = argparse.ArgumentParser(
        description=__doc__.splitlines()[0],
        formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--audit", action="store_true")
    ap.add_argument("--enter", metavar="LEVEL")
    ap.add_argument("--paint", action="store_true")
    ap.add_argument("--cols", default="32,33")
    ap.add_argument("--row", type=int, default=31)
    ap.add_argument("--watch", type=float, default=12.0)
    ap.add_argument("--hz", type=float, default=80.0)
    ap.add_argument("--watch-only", type=float, default=0.0)
    args = ap.parse_args()

    if args.audit:
        return audit()

    h, pid = attach(write=bool(args.paint or args.enter))
    if not h:
        print("the game is not running")
        return 1
    print(f"pid {pid}  ui_state {ui_state(h)}", flush=True)
    try:
        if args.enter:
            from measure_float_splat import enter
            rc = enter(h, args.enter)
            if rc:
                return rc
            from goto_level import loaded_level
            print(f"loaded path: {loaded_level(h)!r}", flush=True)
        if args.watch_only:
            import assign_skill as A
            A.wait_for_lemmings(h, 25.0)
            painted = live_bit_cells(h)
            print(f"live 0x40 cells: {len(painted)} e.g. {sorted(painted)[:16]}",
                  flush=True)
            for bit, label in ((ZONE0, "0x200"), (ZONE1, "0x2000"),
                               (ZONE2, "0x4000")):
                z = live_bit_cells(h, bit)
                print(f"live {label} cells: {len(z)}", flush=True)
            drowns, type2e, deaths = watch(h, args.watch_only, args.hz, painted)
            ok = len(drowns) > 0
            print(f"verdict: {'PASS' if ok else 'FAIL'} "
                  f"(bit40={len(painted)} drown={len(drowns)}"
                  f" type2e={len(type2e)} deaths={len(deaths)})",
                  flush=True)
            return 0 if ok else 2
        if args.paint:
            cols = [int(x) for x in args.cols.split(",") if x.strip()]
            return do_paint(h, cols, args.row, args.watch, args.hz)
        print("nothing to do; pass --audit, --watch-only, or --paint")
        return 0
    finally:
        k32.CloseHandle(h)


if __name__ == "__main__":
    raise SystemExit(main())
