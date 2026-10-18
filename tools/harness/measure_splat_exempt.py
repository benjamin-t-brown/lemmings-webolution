"""Round 29: splat exemption via zone bits 0x200 / 0x4000 vs lemming [+0x4c].

Static (stock exe, image base 0x400000), before any live write:

    Fall land after splatDistance (0x004165FC / 0x004B10A8 = 30.0) and the
    floater byte [+0x49] (0x00416615):

        00416623  and edx, 0x200
        00416630  cmp dword ptr [eax+0x4c], 1
        00416634  je 0x4166a8          ; SURVIVE (same as floater arm)

        00416639  and ecx, 0x4000
        00416646  cmp dword ptr [edx+0x4c], 2
        0041664a  je 0x4166a8          ; SURVIVE

    Spawn copies hatch type into [+0x4c] at 0x0042E05F.
    Sheet table 0x004BFFC8: 0 Lemmings, 1 LemmingsWater, 2 LemmingsAcid.

    Fluid ctor FUN_00428a47 stamps object[+0x128] 0/1/2 -> 0x200/0x2000/0x4000
    at 0x00428D63 / 0x00428D84 / 0x00428DA5 (and sibling sites in the same
    Fluid path). Glow: ^water_glow / ^lava_glow / ^acid_glow.

Modes
-----
    python tools/harness/measure_splat_exempt.py --audit
    python tools/harness/measure_splat_exempt.py --enter e10_kf_1 --paint \\
            --bit 0x200 --type 1 --cols 94,112,130,74 --rows 6,7,8,11 \\
            --watch 20 --hz 60 --poke-every 2
    python tools/harness/measure_splat_exempt.py --enter e10_kf_1 --paint \\
            --bit 0x4000 --type 2 --cols 94,112,130,74 --rows 6,7,8,11 \\
            --watch 20 --hz 60 --poke-every 2

Cap --hz. Do not put UCRT64 bin on PATH. Actor list 0x004CBB18 only.
Restore every painted cell before exit.
"""
from __future__ import annotations

import argparse
import os
import struct
import sys
import time
from collections import Counter, defaultdict
from pathlib import Path

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))

from lrstate import attach, i32, u32, ui_state  # noqa: E402
from measure_spawn import LEMMING_VT, OTHER_VT, actor_ptrs, hatch_words  # noqa: E402
from memdump import k32, read_mem  # noqa: E402
from read_grid_siblings import game_dir, levels_from_box  # noqa: E402
from read_lvl_grid import find_grid  # noqa: E402
import measure_stepup as SU  # noqa: E402

ZONE0, ZONE1, ZONE2 = 0x200, 0x2000, 0x4000
OFF_X, OFF_Y = 0x08, 0x10
OFF_FACING, OFF_VDIR = 0x40, 0x44
OFF_TYPE = 0x4C
OFF_FLOAT = 0x49
OFF_STATE = 0x340
LEM_READ = 0x344

VT_WALK = 0x004B0D28
VT_FALL = 0x004B0A20
VT_SPLAT = 0x004B0668
VT_NAMES = {
    VT_WALK: "walk",
    VT_FALL: "fall",
    VT_SPLAT: "splat",
    0x004B0960: "climb",
    0x004B0AF8: "flip",
    0x004B0CA8: "cartwheel",
    0x004B0810: "bounce",
    0x004B0570: "drown",
    0x004B0680: "fall-680",
}
SHEETS = {0: "Lemmings", 1: "LemmingsWater", 2: "LemmingsAcid", 3: "EvilRat"}
# Fall-state vtable (round 3b). Accumulator at +0x18 is the splat distance.
FALLER_STATE_VT = 0x004B0A20


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
    return {
        "t": t,
        "ptr": ptr,
        "x": struct.unpack_from("<d", buf, OFF_X)[0],
        "y": struct.unpack_from("<d", buf, OFF_Y)[0],
        "facing": struct.unpack_from("<i", buf, OFF_FACING)[0],
        "vdir": struct.unpack_from("<i", buf, OFF_VDIR)[0],
        "type": struct.unpack_from("<i", buf, OFF_TYPE)[0],
        "float": buf[OFF_FLOAT],
        "vt": vt,
        "name": VT_NAMES.get(vt, f"{vt:#010x}"),
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


def audit() -> int:
    lvls = levels_from_box(game_dir())
    type_c: Counter[int] = Counter()
    model_type: dict[str, Counter[int]] = defaultdict(Counter)
    z0, z1, z2 = set(), set(), set()
    fluid = set()
    for name, data in lvls.items():
        if b"Fluid" in data:
            fluid.add(name)
        cells = find_grid(data)[1]
        if any(c & ZONE0 for c in cells):
            z0.add(name)
        if any(c & ZONE1 for c in cells):
            z1.add(name)
        if any(c & ZONE2 for c in cells):
            z2.add(name)
        for h in hatch_words(data):
            type_c[h["type"]] += 1
            model_type[h["model"]][h["type"]] += 1

    print(f"levels {len(lvls)}")
    print(f"hatch type counts: {dict(type_c)}")
    print("hatch model -> types:")
    for m, c in sorted(model_type.items()):
        print(f"  {m!r}: {dict(c)}")
    print(f"Fluid: {len(fluid)}")
    print(f"zone 0x200: {len(z0)}  Fluid&z0: {len(fluid & z0)}  z0-Fluid: {len(z0 - fluid)}")
    print(f"zone 0x2000: {len(z1)} Fluid&z1: {len(fluid & z1)} z1-Fluid: {len(z1 - fluid)}")
    print(f"zone 0x4000: {len(z2)} Fluid&z2: {len(fluid & z2)} z2-Fluid: {len(z2 - fluid)}")
    print("sheet table 0x004BFFC8: 0 Lemmings, 1 LemmingsWater, 2 LemmingsAcid")
    print("landing: 0x200+[+0x4c]==1 OR 0x4000+[+0x4c]==2 -> survive (0x0041662B/41)")
    print("NOTE: 0x2000 (lava) has no splat-exemption arm.")
    return 0


def poke_type(h, ptr: int, typ: int) -> None:
    write_mem(h, ptr + OFF_TYPE, struct.pack("<i", typ))


def process_alive(h) -> bool:
    """False when the game has exited (reads go dead)."""
    return ui_state(h) is not None


def fall_states(h) -> list[int]:
    """Live fall-state object addresses (vtable 0x004B0A20)."""
    from measure_float_splat import state_slots
    return state_slots(h)


_fall_slots: list[int] = []


def clamp_fall_acc(h, max_acc: float, rescan: bool = False,
                   raise_to: bool = False) -> int:
    """Hold or raise every airborne lemming's fall [state+0x18].

    Prefers the lemming's own state pointer at +0x340 (vtable 0x004B0A20)
    over a heap scan — short hatch falls are gone before a scan lands.
    """
    n = 0
    for p in lemmings(h):
        sp = u32(h, p + OFF_STATE)
        if not sp:
            continue
        vt = u32(h, sp) or 0
        if vt != FALLER_STATE_VT:
            continue
        buf = read_mem(h, sp + 0x18, 8)
        if not buf or len(buf) != 8:
            continue
        acc = struct.unpack("<d", buf)[0]
        if raise_to:
            if abs(acc - max_acc) > 0.01:
                if write_mem(h, sp + 0x18, struct.pack("<d", max_acc)):
                    n += 1
        elif acc > max_acc:
            if write_mem(h, sp + 0x18, struct.pack("<d", max_acc)):
                n += 1
    return n


def force_fall(h, ptr: int, x: float | None, y: float) -> None:
    """Teleport a lemming to (x,y) so the next steps fall from there."""
    if x is not None:
        write_mem(h, ptr + OFF_X, struct.pack("<d", x))
    write_mem(h, ptr + OFF_Y, struct.pack("<d", y))


def watch_paint(h, seconds: float, hz: float, painted: set[tuple[int, int]],
                want_type: int | None, poke_every: int,
                treated: set[int] | None = None,
                clamp_acc: float | None = None,
                raise_acc: bool = False,
                control_type: int | None = None,
                force_y: float | None = None,
                force_x: float | None = None,
                force_every: int = 1) -> dict:
    """Follow landings. Poke sheet type on every Nth new lemming.

    `treated` if given is the set of pointers already marked for poking
    (so a re-enter can keep re-applying the dword each sample).
    """
    period = 1.0 / hz if hz > 0 else 0.0
    t0 = time.perf_counter()
    last: dict[int, dict] = {}
    poked: set[int] = set() if treated is None else set(treated)
    seen_order: list[int] = []
    survivors: list[dict] = []
    deaths: list[dict] = []
    landings: list[dict] = []
    treat = set() if treated is None else set(treated)

    clamped_total = 0
    sample_i = 0
    if clamp_acc is not None:
        print(f"acc drive: {'raise' if raise_acc else 'hold'} to {clamp_acc} "
              f"via lemming+0x340 fall state", flush=True)
    while time.perf_counter() - t0 < seconds:
        if not process_alive(h):
            print(f"  t{time.perf_counter() - t0:7.3f}s process gone "
                  f"(ui_state unreadable) — stop watch", flush=True)
            break
        t = time.perf_counter() - t0
        if clamp_acc is not None:
            clamped_total += clamp_fall_acc(
                h, clamp_acc, raise_to=raise_acc)
        sample_i += 1
        alive = set(lemmings(h))
        for p in list(alive):
            if p not in poked:
                seen_order.append(p)
                poked.add(p)
                idx = len(seen_order) - 1
                do_force = (force_y is not None and
                            (force_every <= 1 or idx % force_every == 0))
                if do_force:
                    force_fall(h, p, force_x, force_y)
                    print(f"  t{t:7.3f}s force-y={force_y} x={force_x} "
                          f"{p:#010x}", flush=True)
                if want_type is not None and (
                        poke_every <= 1 or idx % poke_every == 0):
                    treat.add(p)
                    poke_type(h, p, want_type)
                    print(f"  t{t:7.3f}s poke +0x4c={want_type} {p:#010x}",
                          flush=True)
                elif control_type is not None and want_type is not None:
                    poke_type(h, p, control_type)
                    print(f"  t{t:7.3f}s control poke +0x4c={control_type} "
                          f"{p:#010x}", flush=True)
                elif not do_force:
                    print(f"  t{t:7.3f}s seen {p:#010x}", flush=True)
            elif p in treat and want_type is not None:
                poke_type(h, p, want_type)
            # keep forced X sticky while falling (walk facing will drag otherwise)
            if force_x is not None and p in poked:
                s0 = sample(h, p, t)
                if s0 and s0["name"] == "fall" and force_y is not None:
                    write_mem(h, p + OFF_X, struct.pack("<d", force_x))
            s = sample(h, p, t)
            if not s:
                continue
            prev = last.get(p)
            if prev and prev["name"] == "fall" and s["name"] == "walk":
                under = (int(s["x"]) & 0xFF, int(s["y"]) - s["vdir"])
                rec = {
                    "ptr": p, "t": t, "type": s["type"],
                    "under": under, "on_paint": under in painted,
                    "y": s["y"], "x": s["x"],
                }
                landings.append(rec)
                survivors.append(rec)
                print(
                    f"  t{t:7.3f}s LAND-WALK {p:#010x} +0x4c={s['type']}"
                    f" under={under} on_paint={under in painted}"
                    f" y={s['y']:.3f}",
                    flush=True,
                )
            if prev and prev["name"] == "fall" and s["name"] == "splat":
                under = (int(s["x"]) & 0xFF, int(s["y"]) - s["vdir"])
                rec = {
                    "ptr": p, "t": t, "type": s["type"],
                    "under": under, "on_paint": under in painted,
                    "y": s["y"], "x": s["x"], "how": "splat-vt",
                }
                deaths.append(rec)
                print(
                    f"  t{t:7.3f}s SPLAT-VT {p:#010x} +0x4c={s['type']}"
                    f" under={under} on_paint={under in painted}",
                    flush=True,
                )
            last[p] = s
        for p, prev in list(last.items()):
            if p not in alive and prev["name"] in ("fall", "splat"):
                under = (int(prev["x"]) & 0xFF, int(prev["y"]) - prev["vdir"])
                # only count as death if still in-level and near a floor
                near_floor = prev["y"] < 20.0
                rec = {
                    "ptr": p, "t": t, "type": prev["type"],
                    "under": under, "on_paint": under in painted,
                    "y": prev["y"], "x": prev["x"], "how": "left-list",
                }
                if near_floor:
                    deaths.append(rec)
                print(
                    f"  t{t:7.3f}s LEFT {p:#010x} +0x4c={prev['type']}"
                    f" under={under} on_paint={under in painted}"
                    f" last={prev['name']} y={prev['y']:.3f}"
                    f"{'' if near_floor else ' (mid-fall, ignored)'}",
                    flush=True,
                )
                del last[p]
            elif p not in alive:
                del last[p]
        if period:
            time.sleep(period)

    if clamp_acc is not None:
        print(f"accumulator held at {clamp_acc} by {clamped_total} writes",
              flush=True)
    return {
        "survivors": survivors,
        "deaths": deaths,
        "landings": landings,
        "seen": seen_order,
    }


def do_paint(h, bit: int, want_type: int | None, cols: list[int], rows: list[int],
             watch_s: float, hz: float, poke_every: int,
             clamp_acc: float | None = None,
             raise_acc: bool = False,
             control_type: int | None = None,
             force_y: float | None = None,
             force_x: float | None = None,
             force_every: int = 1) -> int:
    import assign_skill as A

    cells: list[tuple[int, int]] = []
    if len(rows) == 1:
        cells = [(c & 0xFF, rows[0]) for c in cols]
    elif len(rows) == len(cols):
        cells = [(c & 0xFF, r) for c, r in zip(cols, rows)]
    else:
        cells = [(c & 0xFF, r) for c in cols for r in rows]

    print(
        f"PREDICTION: fallen > 30 onto a cell with {bit:#x} AND [+0x4c]=={want_type}"
        f" -> walk (FUN_00416433 / 0x0041662B/41); mismatch or clear zone -> splat.",
        flush=True,
    )
    saved = []
    if cells:
        for c, r in cells:
            word = read_cell(h, c, r)
            print(
                f"  before ({c},{r}) = {word:#010x}" if word is not None else
                f"  before ({c},{r}) = <none>",
                flush=True,
            )
        saved = paint_bit(h, cells, bit)
    painted = {(s[2], s[3]) for s in saved}
    if not painted and bit:
        # natural zone: discover live cells after enter
        painted = live_bit_cells(h, bit)
        print(f"natural live {bit:#x} cells: {len(painted)} "
              f"e.g. {sorted(painted)[:8]}", flush=True)
    if force_y is None and clamp_acc is None:
        print(f"waiting up to 20s for lemmings...", flush=True)
        A.wait_for_lemmings(h, 20.0)
    else:
        print("starting watch immediately (force/raise needs the hatch fall)",
              flush=True)
        A.wait_for_lemmings(h, 8.0)
    try:
        result = watch_paint(
            h, watch_s, hz, painted, want_type, poke_every,
            clamp_acc=clamp_acc, raise_acc=raise_acc,
            control_type=control_type,
            force_y=force_y, force_x=force_x, force_every=force_every)
    finally:
        if saved and process_alive(h):
            restore(h, saved)
            for c, r in cells:
                word = read_cell(h, c, r)
                print(
                    f"  after  ({c},{r}) = {word:#010x}" if word is not None else
                    f"  after  ({c},{r}) = <unreadable>",
                    flush=True,
                )
        elif saved:
            print("skip restore: process already gone", flush=True)

    if want_type is not None:
        surv_match = [s for s in result["survivors"]
                      if s["type"] == want_type and s["on_paint"]]
    else:
        surv_match = [s for s in result["survivors"] if s["on_paint"]]
    surv_any = result["survivors"]
    death_match = [d for d in result["deaths"] if d["on_paint"]]
    print(
        f"LAND-WALK total={len(surv_any)} match+paint={len(surv_match)}  "
        f"deaths on paint={len(death_match)}",
        flush=True,
    )
    ok = len(surv_match) > 0
    print(f"verdict: {'PASS' if ok else 'FAIL'}", flush=True)
    return 0 if ok else 2


def main() -> int:
    ap = argparse.ArgumentParser(
        description=__doc__.splitlines()[0],
        formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--audit", action="store_true")
    ap.add_argument("--enter", metavar="LEVEL")
    ap.add_argument("--paint", action="store_true")
    ap.add_argument("--bit", default="0x200",
                    help="zone bit to OR (0x200 or 0x4000)")
    ap.add_argument("--type", type=int, default=None, dest="sheet",
                    help="[+0x4c] value to poke on treated lemmings "
                         "(omit to leave hatch type alone)")
    ap.add_argument("--force-x", type=float, default=None,
                    help="with --force-y, also set X (column over a zone floor)")
    ap.add_argument("--cols", default="94,112,130,74")
    ap.add_argument("--rows", default="6,7,8,11",
                    help="one row for all cols, or paired with cols, or cartesian")
    ap.add_argument("--watch", type=float, default=18.0)
    ap.add_argument("--hz", type=float, default=60.0)
    ap.add_argument("--poke-every", type=int, default=2,
                    help="1=all poked; 2=alternate poke/control")
    ap.add_argument("--clamp-acc", type=float, default=None,
                    help="drive fall [state+0x18] to V each sample")
    ap.add_argument("--raise-acc", action="store_true",
                    help="with --clamp-acc, raise acc up to V (force splat arm "
                         "on a short drop); default holds acc down")
    ap.add_argument("--control-type", type=int, default=None,
                    help="sheet type written on control half (e.g. 0)")
    ap.add_argument("--natural", action="store_true",
                    help="no paint: use live zone cells of --bit")
    ap.add_argument("--force-y", type=float, default=None,
                    help="write lemming Y to this (force a long fall onto zone)")
    ap.add_argument("--force-every", type=int, default=1,
                    help="force-y on every Nth lemming (rest are untouched)")
    ap.add_argument("--watch-only", type=float, default=0.0)
    args = ap.parse_args()

    if args.audit:
        return audit()

    bit = int(args.bit, 0)
    cols = [int(x) for x in args.cols.split(",") if x.strip()] if args.cols else []
    rows = [int(x) for x in args.rows.split(",") if x.strip()] if args.rows else []

    h, pid = attach(write=bool(args.paint or args.enter or args.natural
                               or args.clamp_acc is not None))
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
            for bit_v, label in ((ZONE0, "0x200"), (ZONE1, "0x2000"),
                                 (ZONE2, "0x4000")):
                z = live_bit_cells(h, bit_v)
                print(f"live {label} cells: {len(z)} e.g. {sorted(z)[:8]}",
                      flush=True)
            types = Counter()
            for p in lemmings(h)[:20]:
                s = sample(h, p, 0.0)
                if s:
                    types[s["type"]] += 1
                    print(
                        f"  {p:#010x} x={s['x']:.3f} y={s['y']:.3f}"
                        f" +0x4c={s['type']} ({SHEETS.get(s['type'], '?')})"
                        f" {s['name']}",
                        flush=True,
                    )
            print(f"+0x4c histogram: {dict(types)}", flush=True)
            return 0
        if args.paint or args.natural:
            if args.natural:
                cols, rows = [], []
            return do_paint(h, bit, args.sheet, cols, rows,
                            args.watch, args.hz, args.poke_every,
                            clamp_acc=args.clamp_acc,
                            raise_acc=args.raise_acc,
                            control_type=args.control_type,
                            force_y=args.force_y, force_x=args.force_x,
                            force_every=args.force_every)
        print("nothing to do; pass --audit, --watch-only, --paint, or --natural")
        return 0
    finally:
        k32.CloseHandle(h)


if __name__ == "__main__":
    raise SystemExit(main())
