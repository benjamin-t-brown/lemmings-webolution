"""Round 13: trampoline detection is cell bit 0x1000 → bounce state.

Static (stock exe, image base 0x400000), before any live write:

    fall land 0x00416433:
        00416560  and edx, 0x1000
        00416572  push "Bounce added in FALL..."
        004165xx  call 0x40ec7e          ; vtable 0x004B0810 type 0x21

    bounce collide 0x0040f1e9:
        0040f309  and eax, 0x1000
        → re-attach bounce; "Bounce hit trampoline"

    trampoline enter 0x0041b838 paints |0x883 / |0x1083 (incl. 0x1000);
    exit 0x0041b683 clears with and 0xf77c.

Offline: 0/102 .LVL grids carry 0x1000. pressure_pad.a3d is in 23 levels
(candidate object only).

Modes
-----
    python tools/harness/measure_trampoline.py --audit
        Offline. 0x1000 vs pressure_pad.a3d over all 102 levels.

    python tools/harness/measure_trampoline.py --enter MM_CLIMB_002 --paint \\
            --cols 32,33 --row 31 --watch 12 --hz 100
        OR 0x1000 into named ground cells, sample actor-list states, restore.
        Prediction (scratch note, before the run): a faller whose landing
        cell is painted attaches bounce 0x004B0810 and Y rises.

    python tools/harness/measure_trampoline.py --enter H01_KF_1 --watch-only 40
        No writes. Discover live 0x1000 cells; watch bounce / trampoline.

Cap --hz. Do not put UCRT64 bin on PATH here. `lrstate.attach` returns
(handle, pid). Lemmings from actor list 0x004CBB18 only.
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

TRAMP_BIT = 0x1000
OFF_X, OFF_Y = 0x08, 0x10
OFF_FACING, OFF_VDIR = 0x40, 0x44
OFF_TYPE = 0x4C
OFF_STATE = 0x340
LEM_READ = 0x344

VT_WALK = 0x004B0D28
VT_FALL = 0x004B0A20
VT_BOUNCE = 0x004B0810
VT_TRAMPOLINE = 0x004B0D08
VT_NAMES = {
    VT_WALK: "walk",
    VT_FALL: "fall",
    VT_BOUNCE: "bounce",
    VT_TRAMPOLINE: "trampoline",
    0x004B0960: "climb",
    0x004B0AF8: "flip",
    0x004B0668: "splat",
    0x004B0CA8: "cartwheel",
    0x004B0680: "fall-680",
}


def write_mem(h, addr: int, data: bytes) -> bool:
    return SU.write_mem(h, addr, data)


def grid_addr(h, col: int, row: int):
    return SU.grid_addr(h, col, row)


def read_cell(h, col: int, row: int):
    return SU.read_cell(h, col, row)


def paint(h, cells: list[tuple[int, int]], mode: str = "bit"):
    """Paint trampoline markers. Returns [(addr, old_word, col, row, new)].

    mode \"bit\": OR 0x1000 only (fall→bounce still fires; bounce then treats
    a solid cell as floor — see scratch). mode \"pad\": match trampoline enter
    0x0041B838's 0x1083 cell — OR 0x1083 then clear bit 0 (solid), so bounce
    collision takes the 0x1000 arm when t > 1.4 at 0x004c55a8.
    """
    saved = []
    for col, row in cells:
        a = grid_addr(h, col, row)
        if a is None:
            print(f"  skip ({col},{row}): out of range", flush=True)
            continue
        old = read_cell(h, col, row)
        if old is None:
            continue
        if mode == "pad":
            new = (old | 0x1083) & 0xFFFE
        else:
            new = old | TRAMP_BIT
        saved.append((a, old, col, row, new))
        write_mem(h, a, struct.pack("<I", new))
        print(f"  paint ({col},{row}) {old:#010x} -> {new:#010x} ({mode})",
              flush=True)
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
        "name": VT_NAMES.get(vt, f"{vt:#010x}"),
        "strip": struct.unpack_from("<i", buf, 0x128 + 0x1C8)[0],
    }


def lemmings(h) -> list[int]:
    return [p for p in actor_ptrs(h) if u32(h, p) in (LEMMING_VT, OTHER_VT)]


def live_bit_cells(h, bit: int = TRAMP_BIT) -> set[tuple[int, int]]:
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
    pad, bit = set(), set()
    for name, data in lvls.items():
        if b"pressure_pad.a3d" in data:
            pad.add(name)
        _off, cells = find_grid(data)
        if any(c & TRAMP_BIT for c in cells):
            bit.add(name)
    print(f"levels {len(lvls)}")
    print(f"pressure_pad.a3d: {len(pad)}")
    print(f"any cell & 0x1000: {len(bit)}")
    print(f"match: {pad == bit}")
    if pad - bit:
        print(f"  pad without bit: {sorted(pad - bit)}")
    if bit - pad:
        print(f"  bit without pad: {sorted(bit - pad)}")
    print("NOTE: 0x1000 is expected absent from .LVL (runtime paint by"
          " trampoline enter 0x0041B838).")
    return 0


def watch(h, seconds: float, hz: float, painted: set[tuple[int, int]]):
    period = 1.0 / hz if hz > 0 else 0.0
    t0 = time.perf_counter()
    last = {}
    bounces = []
    tramps = []
    print(f"watching {seconds}s at {hz} Hz; painted {sorted(painted)}",
          flush=True)
    while time.perf_counter() - t0 < seconds:
        t = time.perf_counter() - t0
        for p in lemmings(h):
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
                      f" dy={s['y'] - prev['y']:+.3f} vdir={s['vdir']:+d}"
                      f" stype={s['stype']:#x} strip {s['strip']}"
                      f" under={under} on_paint={under in painted}",
                      flush=True)
                if s["vt"] == VT_BOUNCE:
                    bounces.append(s)
                if s["vt"] == VT_TRAMPOLINE:
                    tramps.append(s)
            elif s["vt"] == VT_BOUNCE and abs(s["y"] - prev["y"]) >= 0.25:
                print(f"  t{t:7.3f}s {p:#010x} BOUNCE y {prev['y']:.3f}->"
                      f" {s['y']:.3f} x={s['x']:.3f} {s['name']}",
                      flush=True)
            last[p] = s
        if period:
            time.sleep(period)
    print(f"\n{len(bounces)} bounce attaches, {len(tramps)} trampoline"
          f" attaches", flush=True)
    return bounces, tramps


def do_paint(h, cols: list[int], row: int, watch_s: float, hz: float,
             mode: str = "bit", wait: float = 20.0) -> int:
    import assign_skill as A

    print(f"waiting up to {wait}s for lemmings...", flush=True)
    A.wait_for_lemmings(h, wait)
    for p in lemmings(h)[:8]:
        s = sample(h, p, 0.0)
        if s:
            print(f"  pre {p:#010x} x={s['x']:.3f} y={s['y']:.3f}"
                  f" face={s['facing']:+d} vdir={s['vdir']:+d} {s['name']}",
                  flush=True)
    cells = [(c & 0xFF, row) for c in cols]
    print("PREDICTION: a faller whose landing cell is one of"
          f" {cells} attaches bounce vtable 0x004B0810"
          f" (paint mode={mode}).", flush=True)
    for c, r in cells:
        word = read_cell(h, c, r)
        print(f"  before ({c},{r}) = "
              f"{word:#010x}" if word is not None else "<none>", flush=True)
    saved = paint(h, cells, mode=mode)
    painted = {(s[2], s[3]) for s in saved}
    try:
        bounces, tramps = watch(h, watch_s, hz, painted)
    finally:
        restore(h, saved)
        for c, r in cells:
            word = read_cell(h, c, r)
            print(f"  after  ({c},{r}) = "
                  f"{word:#010x}" if word is not None else
                  f"  after  ({c},{r}) = <unreadable>", flush=True)
    ok = len(bounces) > 0
    print(f"verdict: {'PASS' if ok else 'FAIL'} "
          f"({len(bounces)} bounce, {len(tramps)} trampoline)", flush=True)
    return 0 if ok else 2


def main() -> int:
    ap = argparse.ArgumentParser(
        description=__doc__.splitlines()[0],
        formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--audit", action="store_true")
    ap.add_argument("--enter", metavar="LEVEL")
    ap.add_argument("--paint", action="store_true")
    ap.add_argument("--pad", action="store_true",
                    help="with --paint: clear solid bit like trampoline enter")
    ap.add_argument("--cols", default="32,33")
    ap.add_argument("--row", type=int, default=31)
    ap.add_argument("--watch", type=float, default=12.0)
    ap.add_argument("--hz", type=float, default=100.0)
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
            print(f"live 0x1000 cells: {len(painted)} e.g. {sorted(painted)[:12]}",
                  flush=True)
            bounces, tramps = watch(h, args.watch_only, args.hz, painted)
            ok = len(bounces) > 0 or len(painted) > 0 or len(tramps) > 0
            print(f"verdict: {'PASS' if ok else 'FAIL'} "
                  f"(bit_cells={len(painted)} bounce={len(bounces)}"
                  f" tramp={len(tramps)})", flush=True)
            return 0 if ok else 2
        if args.paint:
            cols = [int(x) for x in args.cols.split(",") if x.strip()]
            mode = "pad" if args.pad else "bit"
            return do_paint(h, cols, args.row, args.watch, args.hz, mode=mode)
        print("nothing to do; pass --audit, --watch-only, or --paint")
        return 0
    finally:
        k32.CloseHandle(h)


if __name__ == "__main__":
    raise SystemExit(main())
