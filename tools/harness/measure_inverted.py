"""Round 30: inverted step-up / climb / dig after anti-grav ([+0x44]==-1).

Static leads (stock exe, image base 0x400000):

    dig bite 0x00415A43:
        00415a86  mov [ebp-0x10], 0x10
        00415a90  cmp [ecx+0x44], -1
        00415a96  mov [ebp-0x10], 0x20          ; inverted mask
        00415b2a  and edx, [ebp-0x10]           ; refuse if set
        dig row = posY - vdir (0x00415AA0)
        dig step-Y 0x00415DD2: Y := Y - vdir

    walk probe step-up 0x0041C48E / 0x0041C528: row += [+0x44]
    climb stepper 0x00413869: Y += vdir * 5.0 * dt
    hoist 0x00417D6F: Y := trunc(Y) + 3*vdir; exit subtracts vdir -> net +2*vdir
    flip 0x0042F5E3: negate [+0x44]; upright [+0x358]=3 [+0x368]=0;
                     inverted [+0x358]=0 [+0x368]=3

Zero of 102 retail grids carry bit 0x10; eight carry 0x20 (MM_DIG_000 etc).

Modes
-----
    python tools/harness/measure_inverted.py --audit
        Offline: 0x10 / 0x20 / 0x8000 census.

    python tools/harness/measure_inverted.py --enter m06_kf_1 --watch-step 40
        Natural anti-grav flip, then watch inverted walkers for step-up
        (world Y changes of +/-1 or +/-2 while vdir==-1 and state==walk).

    python tools/harness/measure_inverted.py --enter m06_kf_1 --paint-step \\
            --cols 70,71 --rise 1 --watch 25
        OR solid into the air just below the ceiling platform so inverted
        walkers meet a 1- or 2-cell step; restore every cell.

    python tools/harness/measure_inverted.py --enter m06_kf_1 --poke-climb \\
            --watch 30
        After flip, set [+0x48]=1 on inverted walkers near a wall column;
        restore the byte.

    python tools/harness/measure_inverted.py --enter MM_DIG_000 --dig \\
            --force-vdir -1 --seconds 3
        Force [+0x44] and the bias doubles 0x0042F5E3 writes, assign digger,
        restore lemming bytes. Prediction: 0x21 cells REFUSE when inverted.

    python tools/harness/measure_inverted.py --enter MM_DIG_000 --dig \\
            --force-vdir -1 --paint-mask 0x11 --seconds 3
        Same, but paint the three dig-target cells to solid|0x10 first
        (restore after). Prediction: inverted dig CLEARS 0x11.

Do not put UCRT64 bin on PATH here.
"""
from __future__ import annotations

import argparse
import os
import struct
import sys
import time
from collections import Counter
from pathlib import Path

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))

import assign_skill as A  # noqa: E402
import measure_dig as D  # noqa: E402
import measure_stepup as SU  # noqa: E402
from lrstate import attach, i32, u32, ui_state  # noqa: E402
from measure_spawn import LEMMING_VT, OTHER_VT, actor_ptrs  # noqa: E402
from memdump import k32, read_mem  # noqa: E402
from read_grid_siblings import levels_from_box  # noqa: E402
from read_lvl_grid import GRID_WIDTH, find_grid  # noqa: E402

OFF_X, OFF_Y = 0x08, 0x10
OFF_FACING, OFF_VDIR, OFF_CLIMB = 0x40, 0x44, 0x48
OFF_YBIAS_CLIMB = 0x358   # double; upright 3.0, inverted 0.0
OFF_YBIAS_WALK = 0x368    # double; upright 0.0, inverted 3.0
OFF_STATE = 0x340

VT_WALK = 0x004B0D28
VT_FALL = 0x004B0A20
VT_CLIMB = 0x004B0960
VT_FLIP = 0x004B0AF8
VT_DIG = 0x004B09F0
VT_NAMES = {
    VT_WALK: "walk", VT_FALL: "fall", VT_CLIMB: "climb", VT_FLIP: "hoist",
    VT_DIG: "dig", 0x004B0CA8: "cartwheel",
}


def write_mem(h, addr: int, data: bytes) -> bool:
    return SU.write_mem(h, addr, data)


def lemmings(h) -> list[int]:
    return [p for p in actor_ptrs(h) if u32(h, p) in (LEMMING_VT, OTHER_VT)]


def sample_lem(h, ptr: int):
    buf = read_mem(h, ptr, 0x370)
    if not buf or len(buf) < 0x370:
        return None
    if struct.unpack_from("<I", buf, 0)[0] not in (LEMMING_VT, OTHER_VT):
        return None
    sp = struct.unpack_from("<I", buf, OFF_STATE)[0]
    vt = (u32(h, sp) or 0) if sp else 0
    return {
        "ptr": ptr,
        "x": struct.unpack_from("<d", buf, OFF_X)[0],
        "y": struct.unpack_from("<d", buf, OFF_Y)[0],
        "facing": struct.unpack_from("<i", buf, OFF_FACING)[0],
        "vdir": struct.unpack_from("<i", buf, OFF_VDIR)[0],
        "climb": buf[OFF_CLIMB],
        "vt": vt,
        "name": VT_NAMES.get(vt, f"{vt:#010x}"),
        "ybias_climb": struct.unpack_from("<d", buf, OFF_YBIAS_CLIMB)[0],
        "ybias_walk": struct.unpack_from("<d", buf, OFF_YBIAS_WALK)[0],
    }


def set_vdir(h, ptr: int, vdir: int) -> list[tuple[int, bytes]]:
    """Write [+0x44] and the bias pair 0x0042F5E3 stores. Returns restores."""
    restores = []
    for off, size in ((OFF_VDIR, 4), (OFF_YBIAS_CLIMB, 8), (OFF_YBIAS_WALK, 8)):
        old = read_mem(h, ptr + off, size)
        if old:
            restores.append((ptr + off, old))
    write_mem(h, ptr + OFF_VDIR, struct.pack("<i", vdir))
    if vdir == 1:
        write_mem(h, ptr + OFF_YBIAS_CLIMB, struct.pack("<d", 3.0))
        write_mem(h, ptr + OFF_YBIAS_WALK, struct.pack("<d", 0.0))
    else:
        write_mem(h, ptr + OFF_YBIAS_CLIMB, struct.pack("<d", 0.0))
        write_mem(h, ptr + OFF_YBIAS_WALK, struct.pack("<d", 3.0))
    return restores


def restore_bytes(h, saved: list[tuple[int, bytes]]) -> None:
    for addr, old in reversed(saved):
        write_mem(h, addr, old)
    print(f"restored {len(saved)} byte spans", flush=True)


def paint_cells(h, cells: list[tuple[int, int]], word: int | None = None,
                or_bit: int | None = None):
    """Write absolute word or OR a bit. Returns [(addr, old, col, row), ...]."""
    saved = []
    for col, row in cells:
        a = SU.grid_addr(h, col, row)
        if a is None:
            continue
        old = SU.read_cell(h, col, row)
        if old is None:
            continue
        if word is not None:
            new = word
        else:
            new = old | (or_bit or 0)
        write_mem(h, a, struct.pack("<I", new))
        saved.append((a, old, col, row, new))
        print(f"  paint ({col},{row}) {old:#010x} -> {new:#010x}", flush=True)
    return saved


def restore_cells(h, saved) -> None:
    for a, old, col, row, _new in reversed(saved):
        write_mem(h, a, struct.pack("<I", old or 0))
        print(f"  restore ({col},{row}) -> {old:#010x}", flush=True)
    print(f"restored {len(saved)} grid cells", flush=True)


def audit() -> int:
    lvls = levels_from_box(Path(r"C:\progs\Lemmings Revolution"))
    n10 = n20 = n8000 = 0
    levels_10, levels_20, levels_ag = [], [], []
    for name, data in lvls.items():
        _off, cells = find_grid(data)
        c10 = sum(1 for w in cells if w & 0x10)
        c20 = sum(1 for w in cells if w & 0x20)
        cag = sum(1 for w in cells if w & 0x8000)
        if c10:
            levels_10.append((name, c10))
            n10 += c10
        if c20:
            levels_20.append((name, c20))
            n20 += c20
        if cag:
            levels_ag.append((name, cag))
            n8000 += cag
    print(f"levels {len(lvls)}")
    print(f"bit 0x10 cells: {n10} across {len(levels_10)} levels")
    for x in levels_10[:12]:
        print(f"  {x}")
    print(f"bit 0x20 cells: {n20} across {len(levels_20)} levels")
    for x in sorted(levels_20, key=lambda t: -t[1])[:12]:
        print(f"  {x}")
    print(f"bit 0x8000 cells: {n8000} across {len(levels_ag)} levels")
    print("prediction: inverted dig mask is 0x20 (0x00415A96); upright 0x10.")
    print("MM_DIG_000 0x21 digs upright, refuses inverted.")
    print("No retail 0x10: inverted dig of 0x10 needs a paint.")
    return 0


def watch_step(h, seconds: float, hz: float = 80.0):
    """Watch inverted walkers for integer Y adopts (step-up / step-down)."""
    period = 1.0 / hz if hz > 0 else 0.0
    t0 = time.perf_counter()
    last = {}
    events = []
    print(f"watching step series {seconds}s @ {hz} Hz", flush=True)
    while time.perf_counter() - t0 < seconds:
        t = time.perf_counter() - t0
        for p in lemmings(h):
            s = sample_lem(h, p)
            if not s:
                continue
            prev = last.get(p)
            if prev is None:
                if s["vdir"] == -1:
                    print(f"  t{t:7.3f}s {p:#010x} inv x={s['x']:.3f}"
                          f" y={s['y']:.3f} {s['name']}"
                          f" climb_bias={s['ybias_climb']}"
                          f" walk_bias={s['ybias_walk']}", flush=True)
            elif s["vdir"] == -1 and prev["vdir"] == -1:
                dy = s["y"] - prev["y"]
                dx = s["x"] - prev["x"]
                if s["name"] != prev["name"]:
                    print(f"  t{t:7.3f}s {p:#010x} STATE {prev['name']}->"
                          f"{s['name']} x={s['x']:.3f} y={s['y']:.3f}"
                          f" vdir=-1", flush=True)
                    events.append(("state", s, prev))
                elif s["name"] == "walk" and abs(dy) >= 0.95:
                    kind = "step-up-inv" if dy < 0 else "step-down-inv"
                    # inverted: against gravity is -Y in Y-up (step-up)
                    print(f"  t{t:7.3f}s {p:#010x} {kind} dy={dy:+.3f}"
                          f" dx={dx:+.3f} x={s['x']:.3f} y={s['y']:.3f}"
                          f"-> {s['y']:.3f}", flush=True)
                    events.append((kind, s, prev, dy))
                elif s["name"] in ("climb", "hoist") and abs(dy) >= 0.5:
                    print(f"  t{t:7.3f}s {p:#010x} {s['name']} dy={dy:+.3f}"
                          f" x={s['x']:.3f} y={s['y']:.3f}", flush=True)
                    events.append((s["name"], s, prev, dy))
            elif prev["vdir"] != s["vdir"]:
                print(f"  t{t:7.3f}s {p:#010x} VDIR {prev['vdir']:+d}->"
                      f"{s['vdir']:+d} x={s['x']:.3f} y={s['y']:.3f}"
                      f" {prev['name']}->{s['name']}", flush=True)
                events.append(("flip", s, prev))
            last[p] = s
        if period:
            time.sleep(period)
    ups = [e for e in events if e[0] == "step-up-inv"]
    downs = [e for e in events if e[0] == "step-down-inv"]
    climbs = [e for e in events if e[0] in ("climb", "hoist")]
    print(f"\nsummary: {len(ups)} inv step-up, {len(downs)} inv step-down,"
          f" {len(climbs)} climb/hoist dy events", flush=True)
    for e in ups[:12]:
        dy = e[3]
        print(f"  step-up dy={dy:+.3f} (expect -1 or -2 for rise 1/2)",
              flush=True)
    return events


def do_paint_step(h, cols: list[int], rise: int, ceiling_row: int,
                  watch_s: float, hz: float) -> int:
    """Hang `rise` solid cells below the ceiling into open air (Y-up)."""
    # Ceiling underside is `ceiling_row`; rise 1 paints ceiling_row-1, etc.
    cells = []
    for col in cols:
        for k in range(1, rise + 1):
            cells.append((col & 0xFF, ceiling_row - k))
    print(f"PREDICTION: inverted walker meeting a {rise}-cell hang from"
          f" row {ceiling_row} adopts rise {rise} (world Y -= {rise})"
          f" via add [+0x44] at 0x0041C48E.", flush=True)
    A.wait_for_lemmings(h, 25.0)
    # Wait for at least one inverted walker.
    t0 = time.perf_counter()
    while time.perf_counter() - t0 < 35.0:
        if any((s := sample_lem(h, p)) and s["vdir"] == -1
               for p in lemmings(h)):
            break
        time.sleep(0.2)
    saved = paint_cells(h, cells, word=0x1)
    try:
        events = watch_step(h, watch_s, hz)
    finally:
        restore_cells(h, saved)
    ups = [e for e in events if e[0] == "step-up-inv"]
    match = [e for e in ups if abs(abs(e[3]) - rise) < 0.05]
    print(f"verdict paint-step rise={rise}: "
          f"{'PASS' if match else 'FAIL'} "
          f"({len(match)} matching of {len(ups)} step-ups)", flush=True)
    return 0 if match else 1


def do_poke_climb(h, watch_s: float, hz: float) -> int:
    A.wait_for_lemmings(h, 25.0)
    t0 = time.perf_counter()
    poked = []
    while time.perf_counter() - t0 < 40.0 and len(poked) < 4:
        for p in lemmings(h):
            if p in [x[0] for x in poked]:
                continue
            s = sample_lem(h, p)
            if not s or s["vdir"] != -1 or s["name"] != "walk":
                continue
            old = read_mem(h, p + OFF_CLIMB, 1)
            if not old:
                continue
            write_mem(h, p + OFF_CLIMB, b"\x01")
            poked.append((p, old))
            print(f"  poke climb byte {p:#010x} was {old.hex()} "
                  f"x={s['x']:.3f} y={s['y']:.3f}", flush=True)
        time.sleep(0.3)
    try:
        events = watch_step(h, watch_s, hz)
    finally:
        for p, old in poked:
            write_mem(h, p + OFF_CLIMB, old)
        print(f"restored {len(poked)} climb bytes", flush=True)
    climbs = [e for e in events if e[0] in ("climb", "hoist")
              or (e[0] == "state" and e[1]["name"] in ("climb", "hoist"))]
    # Also count state transitions into climb
    to_climb = [e for e in events if e[0] == "state"
                and e[1]["name"] == "climb"]
    print(f"verdict poke-climb: {'PASS' if to_climb or climbs else 'SOFT'} "
          f"state->climb={len(to_climb)} dy_events={len(climbs)}",
          flush=True)
    return 0


def grid_diff(pre: bytes, post: bytes):
    out = []
    if not pre or not post or len(pre) != len(post):
        return out
    for i in range(0, len(pre), 4):
        if pre[i:i + 4] != post[i:i + 4]:
            cell = i // 4
            r, c = divmod(cell, GRID_WIDTH)
            out.append((c, r,
                        struct.unpack_from("<I", pre, i)[0],
                        struct.unpack_from("<I", post, i)[0]))
    return out


def do_dig(h, force_vdir: int | None, paint_mask: int | None,
           seconds: float, skill: int = 7,
           biases: bool = False) -> int:
    """Assign digger; optionally force vdir and paint dig-row cells.

    A floor walker forced to vdir=-1 digs toward +Y (into air above the
    floor). For refuse/clear series, pass --paint-mask so the dig-row the
    bite actually probes is the word under test (0x21 refuse / 0x11 clear).
    """
    A.wait_for_lemmings(h, 25.0)
    st = A.counts(h)
    print(f"stock {st} selected {A.selected(h)}", flush=True)
    if st[skill] <= 0:
        print(f"no dig stock at panel {skill}")
        return 1

    vdir_want = force_vdir if force_vdir is not None else 1
    mask = D.dig_mask(vdir_want)
    print(f"PREDICTION: dig mask={mask:#x} at vdir={vdir_want:+d}; "
          f"0x21 refuses when mask=0x20; 0x11 clears when mask=0x20; "
          f"dig row = trunc(Y)+ybias-vdir; Y step -= vdir.", flush=True)

    lem_restores: list[tuple[int, bytes]] = []
    cell_saved = []
    target = None
    expect_refuse = False
    try:
        for p in lemmings(h):
            s = sample_lem(h, p)
            if not s or s["name"] != "walk":
                continue
            if force_vdir is not None:
                if biases:
                    lem_restores.extend(set_vdir(h, p, force_vdir))
                else:
                    # Dig bite / step-Y read [+0x44] directly. Leaving the
                    # walk bias at 0 keeps dig_row = trunc(Y) - vdir next to
                    # a floor walker (Y+1 when inverted).
                    old = read_mem(h, p + OFF_VDIR, 4)
                    if old:
                        lem_restores.append((p + OFF_VDIR, old))
                    write_mem(h, p + OFF_VDIR, struct.pack("<i", force_vdir))
                s = sample_lem(h, p)
            ybias = s["ybias_walk"] if biases else 0.0
            row, cols = D.dig_target(s["x"], s["y"], s["vdir"], ybias)
            if paint_mask is not None:
                cells = [(c, row) for c in cols]
                cell_saved = paint_cells(h, cells, word=paint_mask)
                words = [paint_mask] * 3
            else:
                words = [SU.read_cell(h, c, row) for c in cols]
            refused = any(D.refused(w or 0, mask) for w in words if w is not None)
            solidish = any((w or 0) & 1 for w in words)
            print(f"  cand {p:#010x} x={s['x']:.3f} y={s['y']:.3f}"
                  f" vdir={s['vdir']:+d} dig_row={row} cols={cols}"
                  f" words={[hex(w) if w is not None else None for w in words]}"
                  f" refuse={refused}", flush=True)
            if solidish or paint_mask is not None:
                target = p
                expect_refuse = refused
                break
        if target is None:
            print("no dig candidate")
            return 1

        _rows_pre, pre = D.full_grid(h)
        from lrinput import LRInput
        with LRInput() as inp:
            inp.clear()
            inp.enable()
            if not A.select_skill(h, inp, skill):
                print("panel select failed")
                return 1
            b = A.lemming_box(h, target)
            if not b:
                print("no screen box")
                return 1
            A.aim_at(h, inp, (b[0] + b[2]) / 2, (b[1] + b[3]) / 2)
            scr = A.screen(h)
            if A.hovered(h, scr) != target:
                A.aim_at(h, inp, (b[0] + b[2]) / 2, (b[1] + b[3]) / 2,
                         tol=1.0, tries=8)
            cb = A.counts(h)
            print(f"  click {target:#010x} expect_refuse={expect_refuse}"
                  f" stock[{skill}]={cb[skill]}", flush=True)
            inp.button(0, True)
            inp.wait_frames(4)
            inp.button(0, False)
            t0 = time.perf_counter()
            y_series = []
            while time.perf_counter() - t0 < seconds:
                s = sample_lem(h, target)
                if s:
                    y_series.append((time.perf_counter() - t0, s["y"],
                                     s["vdir"], s["name"]))
                time.sleep(0.02)
            ca = A.counts(h)
            spent = bool(cb and ca and ca[skill] == cb[skill] - 1)
            _rows_post, post = D.full_grid(h)
            diffs = grid_diff(pre, post)
            print(f"  stock spent={spent}  grid_changes={len(diffs)}",
                  flush=True)
            for c, r, old, new in diffs[:20]:
                print(f"    ({c},{r}) {old:#010x} -> {new:#010x}", flush=True)
            dig_ys = [(t, y) for t, y, vd, n in y_series if n == "dig"]
            if len(dig_ys) >= 2:
                print(f"  dig Y: {dig_ys[0][1]:.3f} .. {dig_ys[-1][1]:.3f}"
                      f" delta={dig_ys[-1][1] - dig_ys[0][1]:+.3f}"
                      f" (n={len(dig_ys)})", flush=True)
            states = Counter(n for _t, _y, _vd, n in y_series)
            print(f"  states seen: {dict(states)}", flush=True)

            if expect_refuse:
                ok = spent and len(diffs) == 0
                print(f"verdict inverted-refuse: "
                      f"{'PASS' if ok else 'FAIL'} "
                      f"(spent={spent}, diffs={len(diffs)})", flush=True)
            else:
                ok = spent and len(diffs) > 0
                print(f"verdict inverted-dig: "
                      f"{'PASS' if ok else 'FAIL'} "
                      f"(spent={spent}, diffs={len(diffs)})", flush=True)
            return 0 if ok else 1
    finally:
        if cell_saved:
            restore_cells(h, cell_saved)
        if lem_restores:
            restore_bytes(h, lem_restores)


def main() -> int:
    ap = argparse.ArgumentParser(
        description=__doc__.splitlines()[0],
        formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--audit", action="store_true")
    ap.add_argument("--enter", metavar="LEVEL")
    ap.add_argument("--watch-step", type=float, nargs="?", const=40.0,
                    metavar="SEC")
    ap.add_argument("--paint-step", action="store_true")
    ap.add_argument("--cols", default="70,71")
    ap.add_argument("--rise", type=int, default=1, choices=(1, 2))
    ap.add_argument("--ceiling-row", type=int, default=19)
    ap.add_argument("--poke-climb", action="store_true")
    ap.add_argument("--watch", type=float, default=25.0)
    ap.add_argument("--hz", type=float, default=80.0)
    ap.add_argument("--dig", action="store_true")
    ap.add_argument("--force-vdir", type=int, choices=(-1, 1), default=None)
    ap.add_argument("--paint-mask", type=lambda s: int(s, 0), default=None,
                    help="paint dig-row cells to this word (e.g. 0x11 / 0x21)")
    ap.add_argument("--biases", action="store_true",
                    help="also write [+0x358]/[+0x368] as 0x0042F5E3 does")
    ap.add_argument("--seconds", type=float, default=3.0)
    ap.add_argument("--skill", type=int, default=7)
    args = ap.parse_args()

    if args.audit:
        return audit()

    need_write = bool(args.enter or args.paint_step or args.poke_climb
                      or args.dig or args.force_vdir is not None)
    h, pid = attach(write=need_write)
    if not h:
        print("the game is not running")
        return 1
    print(f"pid {pid} ui_state {ui_state(h)}", flush=True)
    try:
        if args.enter:
            from measure_float_splat import enter
            rc = enter(h, args.enter)
            if rc:
                return rc
            print(f"entered; ui_state {ui_state(h)}", flush=True)
        if args.dig:
            return do_dig(h, args.force_vdir, args.paint_mask,
                          args.seconds, args.skill, args.biases)
        if args.paint_step:
            cols = [int(x) for x in args.cols.split(",") if x.strip()]
            return do_paint_step(h, cols, args.rise, args.ceiling_row,
                                 args.watch, args.hz)
        if args.poke_climb:
            return do_poke_climb(h, args.watch, args.hz)
        if args.watch_step is not None:
            A.wait_for_lemmings(h, 25.0)
            watch_step(h, args.watch_step, args.hz)
            return 0
        print("nothing to do; see --help")
        return 1
    finally:
        k32.CloseHandle(h)


if __name__ == "__main__":
    raise SystemExit(main())
