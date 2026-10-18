"""Round 9: anti-grav is cell bit 0x8000 — identity, trigger, gravity flip.

Static read (stock exe, image base 0x400000), before any live write:

    walk probe 0x0041BFFC saves the ground cell at (trunc(X), trunc(Y)-vdir)
    into [ebp-0x18] at 0x0041C10B, then:

        0041c154  and edx, 0x8000
        0041c162  push 0x2b ; call 0x42ecc9     ; already cartwheeling?
        0041c245  call 0x41ad90                 ; vtable 0x004B0CA8 type 0x2b
        0041c276  call 0x42e696                 ; attach -> cartwheel enter
        0041c2b5  call 0x415fa0                 ; fall 0x004B0A20
        0041c2ec  call 0x42e696

    cartwheel enter 0x0041ADDC calls 0x0042F5E3 before the animation string:

        0042f5f0  imul ecx, ecx, -1
        0042f5f6  mov  [edx+0x44], ecx          ; flip vertical direction

Corpus: every retail level with any 0x8000 cell embeds anti_grav.a3d, and
every level with anti_grav.a3d has at least one 0x8000 cell (18 of 18).

Modes
-----
    python tools/harness/measure_antigrav.py --audit
        Offline. Bit ↔ anti_grav.a3d over all 102 levels.

    python tools/harness/measure_antigrav.py --enter MM_CLIMB_002 --paint \\
            --cols 40,41 --row 31 --watch 8 --hz 200
        OR 0x8000 into the named ground cells under the pen walkers, sample
        every lemming's [+0x44] / state / Y, then restore every cell written.
        Prediction (written in docs/scratch/phase3-round9-antigrav.md before
        the run): a walker whose ground cell is painted flips +1 -> -1 and
        starts falling with world Y increasing.

Cap --hz. An uncapped single-lemming sampler has killed the process before.

`lrstate.attach` returns (handle, pid). Do not put UCRT64 bin on PATH here.
"""
from __future__ import annotations

import argparse
import os
import struct
import sys
import time
from collections import defaultdict
from pathlib import Path

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))

from lrstate import attach, i32, u32, ui_state  # noqa: E402
from measure_spawn import LEMMING_VT, OTHER_VT, actor_ptrs  # noqa: E402
from memdump import k32, read_mem  # noqa: E402
from read_grid_siblings import levels_from_box  # noqa: E402
from read_lvl_grid import find_grid  # noqa: E402
import measure_stepup as SU  # noqa: E402

ANTIGRAV_BIT = 0x8000
OFF_X, OFF_Y = 0x08, 0x10
OFF_FACING, OFF_VDIR = 0x40, 0x44
OFF_TYPE = 0x4C
OFF_STATE = 0x340
LEM_READ = 0x344

VT_WALK = 0x004B0D28
VT_FALL = 0x004B0A20
VT_CARTWHEEL = 0x004B0CA8
VT_NAMES = {
    VT_WALK: "walk", VT_FALL: "fall", VT_CARTWHEEL: "cartwheel",
    0x004B0960: "climb", 0x004B0AF8: "flip", 0x004B0668: "splat",
    0x004B0810: "fall-810", 0x004B0680: "fall-680",
}


def write_mem(h, addr: int, data: bytes) -> bool:
    return SU.write_mem(h, addr, data)


def grid_addr(h, col: int, row: int):
    return SU.grid_addr(h, col, row)


def read_cell(h, col: int, row: int):
    return SU.read_cell(h, col, row)


def paint(h, cells: list[tuple[int, int]], bit: int = ANTIGRAV_BIT):
    """OR `bit` into each (col, row). Returns [(addr, old_word), ...] to restore."""
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
    return {
        "t": t,
        "ptr": ptr,
        "x": struct.unpack_from("<d", buf, OFF_X)[0],
        "y": struct.unpack_from("<d", buf, OFF_Y)[0],
        "facing": struct.unpack_from("<i", buf, OFF_FACING)[0],
        "vdir": struct.unpack_from("<i", buf, OFF_VDIR)[0],
        "type": struct.unpack_from("<i", buf, OFF_TYPE)[0],
        "vt": vt,
        "name": VT_NAMES.get(vt, f"{vt:#010x}"),
        "strip": struct.unpack_from("<i", buf, 0x128 + 0x1C8)[0],
    }


def lemmings(h) -> list[int]:
    return [p for p in actor_ptrs(h) if u32(h, p) in (LEMMING_VT, OTHER_VT)]


def audit() -> int:
    lvls = levels_from_box(Path(r"C:\progs\Lemmings Revolution"))
    anti, bit = set(), set()
    for name, data in lvls.items():
        if b"anti_grav.a3d" in data:
            anti.add(name)
        _off, cells = find_grid(data)
        if any(c & ANTIGRAV_BIT for c in cells):
            bit.add(name)
    print(f"levels {len(lvls)}")
    print(f"anti_grav.a3d: {len(anti)}")
    print(f"any cell & 0x8000: {len(bit)}")
    print(f"match: {anti == bit}")
    if anti - bit:
        print(f"  anti without bit: {sorted(anti - bit)}")
    if bit - anti:
        print(f"  bit without anti: {sorted(bit - anti)}")
    for name in sorted(bit)[:5]:
        _off, cells = find_grid(lvls[name])
        pads = [(i & 0xFF, i >> 8, cells[i])
                for i in range(len(cells)) if cells[i] & ANTIGRAV_BIT]
        print(f"  {name}: {len(pads)} cells e.g. {pads[:4]}")
    return 0 if anti == bit else 1


def watch(h, seconds: float, hz: float, painted: set[tuple[int, int]]):
    """Sample every lemming; print vdir / state changes and pad contacts."""
    period = 1.0 / hz if hz > 0 else 0.0
    t0 = time.perf_counter()
    last = {}
    flips = []
    print(f"watching {seconds}s at {hz} Hz; painted {sorted(painted)}",
          flush=True)
    while time.perf_counter() - t0 < seconds:
        t = time.perf_counter() - t0
        for p in lemmings(h):
            s = sample(h, p, t)
            if not s:
                continue
            prev = last.get(p)
            ground = (int(s["x"]) & 0xFF, int(s["y"]) - s["vdir"])
            # After flip vdir is -1 so "ground" for the probe is Y-(-1)=Y+1;
            # also report the cell under feet in upright sense for the paint.
            under = (int(s["x"]) & 0xFF, int(s["y"]) - 1)
            key = (s["vdir"], s["vt"], s["name"])
            if prev is None:
                print(f"  t{t:7.3f}s {p:#010x} birth x={s['x']:.3f}"
                      f" y={s['y']:.3f} vdir={s['vdir']:+d} +0x4c={s['type']}"
                      f" {s['name']} strip {s['strip']}"
                      f" under={under} on_paint={under in painted}",
                      flush=True)
            elif prev["vdir"] != s["vdir"]:
                flips.append(s)
                print(f"  t{t:7.3f}s {p:#010x} VDIR {prev['vdir']:+d} ->"
                      f" {s['vdir']:+d}  x={s['x']:.3f} y={s['y']:.3f}"
                      f" {prev['name']}->{s['name']} strip {s['strip']}"
                      f" under={under} on_paint={under in painted}"
                      f" ground_probe={ground}",
                      flush=True)
            elif prev["vt"] != s["vt"]:
                print(f"  t{t:7.3f}s {p:#010x} STATE {prev['name']}->"
                      f" {s['name']}  x={s['x']:.3f} y={s['y']:.3f}"
                      f" vdir={s['vdir']:+d} strip {s['strip']}"
                      f" under={under} on_paint={under in painted}",
                      flush=True)
            elif abs(s["y"] - prev["y"]) >= 0.5 and s["vdir"] == -1:
                # coarse inverted-fall progress
                if int(prev["y"]) != int(s["y"]):
                    print(f"  t{t:7.3f}s {p:#010x} Y {prev['y']:.3f}->"
                          f" {s['y']:.3f} vdir={s['vdir']:+d} {s['name']}",
                          flush=True)
            last[p] = s
        if period:
            time.sleep(period)
    print(f"\n{len(flips)} vdir flips", flush=True)
    return flips


def do_paint(h, cols: list[int], row: int, watch_s: float, hz: float,
             wait: float = 20.0) -> int:
    # Wait for walkers so the paint is under a path that is actually walked.
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
    print("PREDICTION (already in the scratch note): a walker whose"
          f" ground cell is one of {cells} flips [+0x44] +1 -> -1 and"
          " falls with world Y increasing.", flush=True)
    for c, r in cells:
        word = read_cell(h, c, r)
        print(f"  before ({c},{r}) = "
              f"{word:#010x}" if word is not None else "<none>", flush=True)
    saved = paint(h, cells)
    painted = {(s[2], s[3]) for s in saved}
    try:
        flips = watch(h, watch_s, hz, painted)
    finally:
        restore(h, saved)
        for c, r in cells:
            word = read_cell(h, c, r)
            print(f"  after  ({c},{r}) = "
                  f"{word:#010x}" if word is not None else
                  f"  after  ({c},{r}) = <unreadable>", flush=True)
    ok = any(f["vdir"] == -1 for f in flips)
    print(f"verdict: {'PASS' if ok else 'FAIL'} "
          f"({len(flips)} flips to inverted)", flush=True)
    return 0 if ok else 2


def main() -> int:
    ap = argparse.ArgumentParser(
        description=__doc__.splitlines()[0],
        formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--audit", action="store_true")
    ap.add_argument("--enter", metavar="LEVEL")
    ap.add_argument("--paint", action="store_true",
                    help="OR 0x8000 onto --cols at --row, watch, restore")
    ap.add_argument("--cols", default="40,41",
                    help="comma-separated file columns to paint")
    ap.add_argument("--row", type=int, default=31,
                    help="file row (Y-up); MM_CLIMB_002 pen floor is 31")
    ap.add_argument("--watch", type=float, default=8.0)
    ap.add_argument("--hz", type=float, default=200.0)
    ap.add_argument("--probe", action="store_true")
    ap.add_argument("--watch-only", type=float, default=0.0,
                    help="no writes: follow vdir/state for this many seconds")
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
        if args.probe:
            for p in lemmings(h)[:12]:
                s = sample(h, p, 0.0)
                if s:
                    print(f"  {p:#010x} x={s['x']:.3f} y={s['y']:.3f}"
                          f" vdir={s['vdir']:+d} +0x4c={s['type']}"
                          f" {s['name']}", flush=True)
            return 0
        if args.watch_only:
            import assign_skill as A
            A.wait_for_lemmings(h, 25.0)
            # Discover live 0x8000 cells so on_paint reports pad contact.
            rows = i32(h, SU.ROW_COUNT) or 96
            base = u32(h, SU.GRID_DATA_PTR)
            painted = set()
            if base:
                raw = read_mem(h, base, 256 * rows * 4)
                if raw:
                    for i in range(0, len(raw), 4):
                        if struct.unpack_from("<I", raw, i)[0] & ANTIGRAV_BIT:
                            painted.add((i // 4 & 0xFF, i // 4 >> 8))
            print(f"live 0x8000 cells: {len(painted)} e.g. {sorted(painted)[:8]}",
                  flush=True)
            flips = watch(h, args.watch_only, args.hz, painted)
            ok = any(f["vdir"] == -1 for f in flips)
            print(f"verdict: {'PASS' if ok else 'FAIL'} "
                  f"({len(flips)} flips)", flush=True)
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
