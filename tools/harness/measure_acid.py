"""Round 15: acid as a hazard.

Static (stock exe, image base 0x400000), before any live write:

    Fluid probe (type 0x2f, vtable 0x004B09C0) at 0x0041527B:
        ground cell (trunc X, trunc Y - [+0x44]); and 0x40 at 0x00415386.
        When set:
          loop list 0x004C89D8 / call 0x00420A52 → type 0x2e
            ctor 0x00406C20, vtable 0x004B04D0; enter 0x00406C6C (no
            anim-setter site); step 0x004071F6 accumulates dt, dies when
            anim at lem+0x78 reports done (return 2).
          else → drown type 0x22 / vtable 0x004B0570 (round 14).
        When 0x40 clear: and 0x100 at 0x00415552 → particle object
          0x00415710 / vtable 0x004B09D8 (not a lemming state).

    Fluid glow subtype at 0x0042A892..0x0042A8EA:
        0 → ^water_glow, 1 → ^lava_glow, 2 → ^acid_glow.

    \"LemmingsAcid\" at 0x004C0030 is indexed with LemmingsWater via the
    sheet table at 0x004BFFC8 (0x0044384C) by lemming [+0x4c] — hatch type
    1 sprites, not the hazard.

Modes
-----
    python tools/harness/measure_acid.py --audit
    python tools/harness/measure_acid.py --enter H02_JC_1 --watch-only 50 --hz 40
    python tools/harness/measure_acid.py --enter H18_MG_1 --watch-only 50 --hz 40
    python tools/harness/measure_acid.py --enter MM_CLIMB_002 --paint \\
            --cols 32,33 --row 31 --bit 0x40 --watch 14 --hz 80
    python tools/harness/measure_acid.py --enter MM_CLIMB_002 --paint \\
            --cols 32,33 --row 31 --bit 0x100 --watch 14 --hz 80

Cap --hz. No UCRT64 bin on PATH. Actor list 0x004CBB18 only.
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

BIT40 = 0x40
BIT100 = 0x100
ZONE0, ZONE1, ZONE2 = 0x200, 0x2000, 0x4000
OFF_X, OFF_Y = 0x08, 0x10
OFF_FACING, OFF_VDIR = 0x40, 0x44
OFF_TYPE = 0x4C
OFF_STATE = 0x340
LEM_READ = 0x380  # need +0x374 side-effect ptr

VT_WALK = 0x004B0D28
VT_FALL = 0x004B0A20
VT_DROWN = 0x004B0570
VT_TYPE2E = 0x004B04D0
VT_FLUID_PROBE = 0x004B09C0
VT_PARTICLE = 0x004B09D8
VT_NAMES = {
    VT_WALK: "walk",
    VT_FALL: "fall",
    VT_DROWN: "drown",
    VT_TYPE2E: "type2e",
    VT_FLUID_PROBE: "fluid-probe",
    VT_PARTICLE: "particle-100",
    0x004B0960: "climb",
    0x004B0AF8: "flip",
    0x004B0668: "splat",
    0x004B0CA8: "cartwheel",
    0x004B0810: "bounce",
    0x004B0680: "fall-680",
}

FLUID_LIST = 0x004C89D8


def write_mem(h, addr: int, data: bytes) -> bool:
    return SU.write_mem(h, addr, data)


def grid_addr(h, col: int, row: int):
    return SU.grid_addr(h, col, row)


def read_cell(h, col: int, row: int):
    return SU.read_cell(h, col, row)


def paint_bit(h, cells: list[tuple[int, int]], bit: int):
    """OR bit onto cells. Returns [(addr, old, col, row, new)]."""
    saved = []
    for col, row in cells:
        a = grid_addr(h, col, row)
        if a is None:
            print(f"  skip ({col},{row}): out of range", flush=True)
            continue
        old = read_cell(h, col, row)
        if old is None:
            continue
        new = old | bit
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
    if sp and vt in (VT_DROWN, VT_TYPE2E):
        ab = read_mem(h, sp + 8, 8)
        if ab and len(ab) == 8:
            acc = struct.unpack("<d", ab)[0]
    side = struct.unpack_from("<I", buf, 0x374)[0]
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
        "side": side,
        "flag5f": buf[0x5F] if len(buf) > 0x5F else 0,
    }


def lemmings(h) -> list[int]:
    return [p for p in actor_ptrs(h) if u32(h, p) in (LEMMING_VT, OTHER_VT)]


def live_bit_cells(h, bit: int) -> set[tuple[int, int]]:
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


def fluid_list_count(h) -> int | None:
    """List head at 0x004C89D8 — same shape as other object vectors.

    Anti-grav / fluid probes call 0x004157F0 with ecx=list to get count.
    observed layout for sibling lists: +0 count-ish via method; raw peek
    of first dwords for the harness log.
    """
    raw = read_mem(h, FLUID_LIST, 0x20)
    if not raw or len(raw) != 0x20:
        return None
    words = struct.unpack("<8I", raw)
    return words


def audit() -> int:
    lvls = levels_from_box(Path(r"C:\progs\Lemmings Revolution"))
    ea, ew, fluid, acid_g, lava_g, water_g = (
        set(), set(), set(), set(), set(), set())
    b40, b100, z0 = set(), set(), set()
    for name, data in lvls.items():
        if b"Entrance_acid" in data:
            ea.add(name)
        if b"Entrance_water" in data:
            ew.add(name)
        if b"Fluid" in data:
            fluid.add(name)
        if b"^acid_glow" in data or b"^acid" in data:
            acid_g.add(name)
        if b"^lava_glow" in data or b"^lava" in data:
            lava_g.add(name)
        if b"^water_glow" in data or b"^water" in data:
            water_g.add(name)
        _off, cells = find_grid(data)
        if any(c & BIT40 for c in cells):
            b40.add(name)
        if any(c & BIT100 for c in cells):
            b100.add(name)
        if any(c & ZONE0 for c in cells):
            z0.add(name)
    print(f"levels {len(lvls)}")
    print(f"Entrance_acid.a3d: {len(ea)} -> {sorted(ea)}")
    print(f"Entrance_water.a3d: {len(ew)}")
    print(f"Fluid: {len(fluid)}")
    print(f"^acid*: {len(acid_g)} -> {sorted(acid_g)}")
    print(f"^lava*: {len(lava_g)} -> {sorted(lava_g)}")
    print(f"^water*: {len(water_g)}")
    print(f"any cell & 0x40: {len(b40)} -> {sorted(b40)}")
    print(f"any cell & 0x100: {len(b100)} -> {sorted(b100)}")
    print(f"Entrance_acid & Fluid: {sorted(ea & fluid)}")
    print(f"Entrance_acid & 0x40: {sorted(ea & b40)}")
    print(f"Entrance_acid & 0x100: {sorted(ea & b100)}")
    print(f"Entrance_acid & 0x200: {sorted(ea & z0)}")
    print(f"^acid* & 0x40: {sorted(acid_g & b40)}")
    print(f"^acid* & 0x100: {sorted(acid_g & b100)}")
    print("NOTE: LemmingsAcid sheet is [+0x4c] index (0x0044384C), not hazard.")
    print("NOTE: Fluid glow subtype 2 selects ^acid_glow (0x0042A8EA).")
    return 0


def watch(h, seconds: float, hz: float, painted: set[tuple[int, int]]):
    period = 1.0 / hz if hz > 0 else 0.0
    t0 = time.perf_counter()
    last = {}
    drowns = []
    type2e = []
    deaths = []
    flag5f = []
    print(f"watching {seconds}s at {hz} Hz; painted {sorted(painted)[:24]}"
          f"{'...' if len(painted) > 24 else ''}", flush=True)
    fl = fluid_list_count(h)
    print(f"  fluid list 0x4c89d8 words: "
          f"{[hex(w) for w in fl] if fl else None}", flush=True)
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
                      f" under={under} on_paint={under in painted}"
                      f" 5f={s['flag5f']}",
                      flush=True)
            elif prev["vt"] != s["vt"]:
                print(f"  t{t:7.3f}s {p:#010x} STATE {prev['name']}->"
                      f" {s['name']}  x={s['x']:.3f} y={s['y']:.3f}"
                      f" vdir={s['vdir']:+d} +0x4c={s['type']}"
                      f" stype={s['stype']:#x} acc={s['acc']:.3f}"
                      f" strip {s['strip']}"
                      f" under={under} on_paint={under in painted}"
                      f" 5f={s['flag5f']}",
                      flush=True)
                if s["vt"] == VT_DROWN:
                    drowns.append(s)
                if s["vt"] == VT_TYPE2E:
                    type2e.append(s)
            elif s["vt"] in (VT_DROWN, VT_TYPE2E) and (
                    s["acc"] - prev["acc"]) >= 0.15:
                print(f"  t{t:7.3f}s {p:#010x} {s['name'].upper()} acc "
                      f"{prev['acc']:.3f}->{s['acc']:.3f}"
                      f" x={s['x']:.3f} y={s['y']:.3f}", flush=True)
            elif not prev["flag5f"] and s["flag5f"]:
                flag5f.append(s)
                print(f"  t{t:7.3f}s {p:#010x} FLAG5f 0->1"
                      f" x={s['x']:.3f} y={s['y']:.3f} {s['name']}"
                      f" under={under}", flush=True)
            last[p] = s
        for p, prev in list(last.items()):
            if p not in alive and prev["vt"] in (VT_DROWN, VT_TYPE2E):
                deaths.append(prev)
                print(f"  t{t:7.3f}s {p:#010x} LEFT actor list after"
                      f" {prev['name']} (last acc={prev['acc']:.3f})",
                      flush=True)
                del last[p]
            elif p not in alive:
                del last[p]
        if period:
            time.sleep(period)
    print(f"\n{len(drowns)} drown, {len(type2e)} type2e,"
          f" {len(deaths)} hazard removals, {len(flag5f)} flag5f",
          flush=True)
    return drowns, type2e, deaths, flag5f


def do_paint(h, cols: list[int], row: int, bit: int,
             watch_s: float, hz: float, wait: float = 20.0) -> int:
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
    if bit == BIT40:
        pred = ("PREDICTION: ground cell with 0x40 and empty Fluid list"
                " attaches drown 0x004B0570; with a matching Fluid object,"
                " type 0x2e / 0x004B04D0. Falsifier: neither attach, or"
                " attach while under cell clear of the bit.")
    else:
        pred = ("PREDICTION: ground cell with 0x100 (0x40 clear) does NOT"
                " attach drown/type2e; may set lem[+0x5f]. Falsifier:"
                " drown or type2e attach on the painted cell.")
    print(pred, flush=True)
    for c, r in cells:
        word = read_cell(h, c, r)
        print(f"  before ({c},{r}) = "
              f"{word:#010x}" if word is not None else "<none>", flush=True)
    saved = paint_bit(h, cells, bit)
    painted = {(s[2], s[3]) for s in saved}
    try:
        drowns, type2e, deaths, flag5f = watch(h, watch_s, hz, painted)
    finally:
        restore(h, saved)
        for c, r in cells:
            word = read_cell(h, c, r)
            print(f"  after  ({c},{r}) = "
                  f"{word:#010x}" if word is not None else
                  f"  after  ({c},{r}) = <unreadable>", flush=True)
    if bit == BIT40:
        ok = len(drowns) + len(type2e) > 0
    else:
        ok = len(drowns) == 0 and len(type2e) == 0
    print(f"verdict: {'PASS' if ok else 'FAIL'} "
          f"(drown={len(drowns)} type2e={len(type2e)} deaths={len(deaths)}"
          f" flag5f={len(flag5f)})", flush=True)
    return 0 if ok else 2


def main() -> int:
    ap = argparse.ArgumentParser(
        description=__doc__.splitlines()[0],
        formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--audit", action="store_true")
    ap.add_argument("--enter", metavar="LEVEL")
    ap.add_argument("--paint", action="store_true")
    ap.add_argument("--bit", default="0x40",
                    help="cell bit to OR (0x40 or 0x100)")
    ap.add_argument("--cols", default="32,33")
    ap.add_argument("--row", type=int, default=31)
    ap.add_argument("--watch", type=float, default=12.0)
    ap.add_argument("--hz", type=float, default=80.0)
    ap.add_argument("--watch-only", type=float, default=0.0)
    args = ap.parse_args()

    if args.audit:
        return audit()

    bit = int(args.bit, 0)
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
            for bit_v, label in ((BIT40, "0x40"), (BIT100, "0x100"),
                                 (ZONE0, "0x200"), (ZONE1, "0x2000"),
                                 (ZONE2, "0x4000")):
                z = live_bit_cells(h, bit_v)
                print(f"live {label} cells: {len(z)} e.g. {sorted(z)[:12]}",
                      flush=True)
            painted = live_bit_cells(h, BIT40) | live_bit_cells(h, BIT100)
            drowns, type2e, deaths, flag5f = watch(
                h, args.watch_only, args.hz, painted)
            print(f"verdict: drown={len(drowns)} type2e={len(type2e)}"
                  f" deaths={len(deaths)} flag5f={len(flag5f)}", flush=True)
            return 0
        if args.paint:
            cols = [int(x) for x in args.cols.split(",") if x.strip()]
            return do_paint(h, cols, args.row, bit, args.watch, args.hz)
        print("nothing to do; pass --audit, --watch-only, or --paint")
        return 0
    finally:
        k32.CloseHandle(h)


if __name__ == "__main__":
    raise SystemExit(main())
