"""Round 17: what does a SpeedPad do, and how is it detected?

Stock exe, actor list 0x004CBB18. No PIXELS_PER_UNIT doubling.

Modes
-----
    python tools/harness/measure_speedpad.py --audit
        Offline census: speedup_two_way.a3d / pressure_pad.a3d vs cell bits.

    python tools/harness/measure_speedpad.py --enter H10_MT_1 --watch 45 --hz 80
        Enter level, verify path, watch +0x50 / FASTER / walk rate.

    python tools/harness/measure_speedpad.py --dump-objects
        Heap-scan object-shaped vtables; print pose + AABB (needs playing).

Predictions in docs/scratch/phase3-round17-speedpad.md before each live run.
Restore every live write.
"""
from __future__ import annotations

import argparse
import math
import os
import struct
import sys
import time
from collections import defaultdict
from pathlib import Path

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))

from diffscan import regions  # noqa: E402
from goto_level import loaded_level  # noqa: E402
from lrstate import attach, u32, ui_state  # noqa: E402
from measure_spawn import LEMMING_VT, OTHER_VT, actor_ptrs  # noqa: E402
from memdump import k32, read_mem  # noqa: E402
from read_grid_siblings import levels_from_box  # noqa: E402
from read_lvl_grid import find_grid  # noqa: E402

OFF_X, OFF_Y, OFF_Z = 0x08, 0x10, 0x18
OFF_FACING, OFF_VDIR = 0x40, 0x44
OFF_SPEED_MODE = 0x50
OFF_TYPE = 0x4C
OFF_STATE = 0x340
OFF_PARLIST = 0x374
LEM_READ = 0x390

VT_FASTER = 0x004B0A38
VT_SLOWER = 0x004B0C90
VT_WALK = 0x004B0D28

OBJECT_VTS = [
    0x004B0D80,  # anti-grav
    0x004B0F50,  # hatch
    0x004B0F90,  # exit
    0x004B1148,  # AABB message pad (candidate SpeedPad)
    0x004B1178,
    0x004B11E0,
    0x004B1250,  # teleport
    0x004B1288,  # lever (out of scope; dump only)
    0x004B12B4,  # candidate
]

VT_NAMES = {
    VT_WALK: "walk",
    0x004B0A20: "fall",
    VT_FASTER: "faster",
    VT_SLOWER: "slower",
    0x004B0960: "climb",
    0x004B0810: "bounce",
    0x004B0B28: "exit-save",
}

SPEEDUP = b"speedup_two_way.a3d"
PRESSURE = b"pressure_pad.a3d"
SPEED_ONLY = {"H10_MT_1", "SHOW_MT3", "VH21_JC"}


def f64_at(buf: bytes, off: int) -> float:
    return struct.unpack_from("<d", buf, off)[0]


def pose_after(data: bytes, a3d_at: int):
    """Match LevelObjects::poseAfterA3d (translation only)."""
    limit = min(len(data), a3d_at + 80)
    obj = None
    for i in range(a3d_at, max(a3d_at, limit - 13)):
        if (struct.unpack_from("<I", data, i)[0] == 9
                and data[i + 4:i + 13] == b"Object 09"):
            obj = i
            break
    if obj is None:
        return None
    if struct.unpack_from("<I", data, obj + 13)[0] != 0:
        return None
    start = obj + 17
    if start + 12 * 8 > len(data):
        return None
    tx = struct.unpack_from("<d", data, start + 9 * 8)[0]
    ty = struct.unpack_from("<d", data, start + 10 * 8)[0]
    tz = struct.unpack_from("<d", data, start + 11 * 8)[0]
    r = math.hypot(tx, tz)
    if r < 20.0 or r > 100.0 or ty < -10.0 or ty > 200.0:
        return None
    col = (math.atan2(tz, tx) + math.pi) / (2 * math.pi) * 256.0
    return tx, ty, tz, col


def find_a3d_poses(data: bytes, name: bytes) -> list:
    low = data.lower()
    needle = name.lower()
    out = []
    idx = 0
    while True:
        i = low.find(needle, idx)
        if i < 0:
            break
        if i >= 4:
            plen = struct.unpack_from("<I", data, i - 4)[0]
            if plen == len(name):
                p = pose_after(data, i - 4)
                if p:
                    out.append(p)
        idx = i + 1
    return out


def audit() -> int:
    lvls = levels_from_box(Path(r"C:\progs\Lemmings Revolution"))
    speed, pressure = set(), set()
    bit_levels = defaultdict(set)
    poses = {}
    for name, data in lvls.items():
        low = data.lower()
        if SPEEDUP in low:
            speed.add(name)
            poses[name] = find_a3d_poses(data, SPEEDUP)
        if PRESSURE in low:
            pressure.add(name)
        _off, cells = find_grid(data)
        present = set()
        for c in cells:
            w = c
            b = 1
            while w:
                if w & 1:
                    present.add(b)
                w >>= 1
                b <<= 1
        for b in present:
            bit_levels[b].add(name)

    print(f"levels {len(lvls)}")
    print(f"speedup_two_way.a3d: {len(speed)} levels")
    n_place = sum(len(v) for v in poses.values())
    print(f"speedup placements with pose: {n_place}")
    print(f"pressure_pad.a3d: {len(pressure)} levels")
    both = speed & pressure
    only_s = speed - pressure
    print(f"co-occur: {len(both)}; speed-only: {sorted(only_s)}")
    exclusive = []
    for b, names in sorted(bit_levels.items()):
        in_s = names & speed
        in_o = names - speed
        if in_s == speed and not in_o:
            exclusive.append(b)
    print(f"bits exclusive to all speedup levels: {[hex(b) for b in exclusive]}")
    for name in sorted(speed):
        ps = poses.get(name, [])
        print(f"  {name}: {len(ps)} pads", flush=True)
        for i, (tx, ty, tz, col) in enumerate(ps):
            print(f"    S{i}: ty={ty:.3f} col~{col:.3f} "
                  f"(tx={tx:.3f} tz={tz:.3f})", flush=True)
    ok = len(speed) == 9 and n_place == 16 and not exclusive
    print(f"P1 {'holds' if ok else 'FAILS'}", flush=True)
    return 0 if ok else 1


def find_objects(h, vtable: int) -> list[int]:
    needle = struct.pack("<I", vtable)
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


def dump_objects(h) -> None:
    print("object vtable heap scan:", flush=True)
    for vt in OBJECT_VTS:
        ptrs = find_objects(h, vt)
        print(f"  {vt:#010x}: {len(ptrs)}", flush=True)
        for p in ptrs[:12]:
            buf = read_mem(h, p, 0xE0)
            if not buf or len(buf) < 0x78:
                continue
            x, y, z = f64_at(buf, 8), f64_at(buf, 0x10), f64_at(buf, 0x18)
            mn = [f64_at(buf, o) for o in (0x48, 0x50, 0x58)]
            mx = [f64_at(buf, o) for o in (0x60, 0x68, 0x70)]
            st = struct.unpack_from("<i", buf, 0xB8)[0] if len(buf) >= 0xBC else -99
            print(f"    {p:#010x} pose=({x:.3f},{y:.3f},{z:.3f}) "
                  f"AABB X[{mn[0]:.3f}..{mx[0]:.3f}] "
                  f"Y[{mn[1]:.3f}..{mx[1]:.3f}] "
                  f"Z[{mn[2]:.3f}..{mx[2]:.3f}] "
                  f"+0xb8={st}", flush=True)


def parallel_vts(h, lem: int) -> list[int]:
    """Walk +0x374/+0x378 0xC-node chains; return state vtables.

    MEASURED round 7/17: FUN_00430590 allocates 0xC nodes; head/tail sit at
    lemming+0x374 / +0x378. FASTER (0x004B0A38) lands here, not at +0x340.
    """
    out = []
    seen_vt = set()
    for head_off in (0x00, 0x04):
        node = u32(h, lem + OFF_PARLIST + head_off) or 0
        seen = set()
        while node and node not in seen and len(seen) < 16:
            seen.add(node)
            d = read_mem(h, node, 0x0C)
            if not d or len(d) != 0x0C:
                break
            val, _a, nxt = struct.unpack("<III", d)
            if val:
                vt = u32(h, val) or 0
                if vt and vt not in seen_vt:
                    seen_vt.add(vt)
                    out.append(vt)
            node = nxt
    return out


def sample(h, ptr: int, t: float):
    buf = read_mem(h, ptr, LEM_READ)
    if not buf or len(buf) < LEM_READ:
        return None
    if struct.unpack_from("<I", buf, 0)[0] not in (LEMMING_VT, OTHER_VT):
        return None
    sp = struct.unpack_from("<I", buf, OFF_STATE)[0]
    vt = (u32(h, sp) or 0) if sp else 0
    mode = struct.unpack_from("<i", buf, OFF_SPEED_MODE)[0]
    pvts = parallel_vts(h, ptr)
    return {
        "t": t,
        "ptr": ptr,
        "x": f64_at(buf, OFF_X),
        "y": f64_at(buf, OFF_Y),
        "z": f64_at(buf, OFF_Z),
        "facing": struct.unpack_from("<i", buf, OFF_FACING)[0],
        "vdir": struct.unpack_from("<i", buf, OFF_VDIR)[0],
        "type": struct.unpack_from("<i", buf, OFF_TYPE)[0],
        "mode": mode,
        "vt": vt,
        "name": VT_NAMES.get(vt, f"{vt:#010x}"),
        "par": pvts,
        "has_faster": VT_FASTER in pvts,
        "has_slower": VT_SLOWER in pvts,
    }


def lemmings(h) -> list[int]:
    return [p for p in actor_ptrs(h) if u32(h, p) in (LEMMING_VT, OTHER_VT)]


def watch(h, seconds: float, hz: float):
    period = 1.0 / hz if hz > 0 else 0.0
    t0 = time.perf_counter()
    last = {}
    rate_win = defaultdict(list)  # ptr -> [(t, x)]
    events = []
    print(f"watching {seconds}s at {hz} Hz for +0x50==2 / FASTER / rate",
          flush=True)
    print("PREDICTION P2: walker overlapping speedup_two_way gets "
          "+0x50=2 and ~2x walk for ~5s via FASTER 0x004B0A38 — "
          "falsified by no mode/rate change in 45s on H10_MT_1.", flush=True)
    while time.perf_counter() - t0 < seconds:
        t = time.perf_counter() - t0
        for p in lemmings(h):
            s = sample(h, p, t)
            if not s:
                continue
            prev = last.get(p)
            rate_win[p].append((t, s["x"]))
            # prune window to ~0.4 s
            while rate_win[p] and t - rate_win[p][0][0] > 0.4:
                rate_win[p].pop(0)
            rate = None
            if len(rate_win[p]) >= 2:
                t0w, x0w = rate_win[p][0]
                t1w, x1w = rate_win[p][-1]
                dt = t1w - t0w
                if dt > 0.05:
                    rate = abs(x1w - x0w) / dt
            if prev is None:
                print(f"  t{t:7.3f}s {p:#010x} birth x={s['x']:.3f} "
                      f"y={s['y']:.3f} mode={s['mode']} "
                      f"{s['name']} par={[hex(v) for v in s['par']]}",
                      flush=True)
            else:
                if prev["mode"] != s["mode"] or prev["has_faster"] != s["has_faster"]:
                    events.append(s)
                    print(f"  t{t:7.3f}s {p:#010x} MODE "
                          f"{prev['mode']}->{s['mode']} "
                          f"faster {prev['has_faster']}->{s['has_faster']} "
                          f"x={s['x']:.3f} y={s['y']:.3f} "
                          f"rate={rate if rate is not None else float('nan'):.2f} "
                          f"{s['name']} "
                          f"par={[hex(v) for v in s['par']]}", flush=True)
                elif s["mode"] == 2 and rate is not None and int(t * 2) != int(prev["t"] * 2):
                    # heartbeat while sped
                    if int(t) != int(prev["t"]):
                        print(f"  t{t:7.3f}s {p:#010x} FAST "
                              f"x={s['x']:.3f} rate={rate:.2f} "
                              f"mode={s['mode']}", flush=True)
            last[p] = s
        if period:
            time.sleep(period)
    n_fast = sum(1 for s in last.values() if s["mode"] == 2 or s["has_faster"])
    print(f"{len(events)} mode events; {n_fast} lemmings still fast/mode2; "
          f"{len(last)} lemmings seen", flush=True)
    return events


def main() -> int:
    ap = argparse.ArgumentParser(
        description=__doc__.splitlines()[0],
        formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--audit", action="store_true")
    ap.add_argument("--enter", metavar="LEVEL")
    ap.add_argument("--watch", type=float, default=0.0)
    ap.add_argument("--hz", type=float, default=80.0)
    ap.add_argument("--dump-objects", action="store_true")
    args = ap.parse_args()

    if args.audit:
        return audit()

    need_write = bool(args.enter)
    h, pid = attach(write=need_write)
    if not h:
        print("the game is not running")
        return 1
    print(f"pid {pid}  ui_state {ui_state(h)}", flush=True)
    try:
        if args.enter:
            from measure_float_splat import enter
            rc = enter(h, args.enter)
            path = loaded_level(h)
            print(f"VERIFY loaded_level={path!r}", flush=True)
            if args.enter.lower() not in path.lower().replace("\\", "/"):
                print(f"REFUSING: wanted {args.enter!r}, got {path!r}",
                      flush=True)
                return 3
            if rc:
                return rc
            time.sleep(1.0)
        if args.dump_objects:
            dump_objects(h)
        if args.watch:
            events = watch(h, args.watch, args.hz)
            return 0 if events else 2
        if not args.dump_objects:
            print("nothing to do; pass --audit / --watch / --dump-objects")
            return 0
        return 0
    finally:
        k32.CloseHandle(h)


if __name__ == "__main__":
    raise SystemExit(main())
