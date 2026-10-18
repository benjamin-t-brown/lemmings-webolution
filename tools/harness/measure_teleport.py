"""Round 16: what does a teleport do, and how is it detected?

Stock exe, actor list 0x004CBB18. No PIXELS_PER_UNIT doubling.

Modes
-----
    python tools/harness/measure_teleport.py --audit
        Offline census: teleport.a3d / teleport_fx.a3d vs cell bits.

    python tools/harness/measure_teleport.py --enter MM_TELE_003 --watch 40 --hz 80
        Enter level, verify loaded path, watch actor-list (X,Y) for jumps.

    python tools/harness/measure_teleport.py --dump-objects
        Heap-scan object-shaped vtables; print pose + AABB (needs playing).

Predictions are written in docs/scratch/phase3-round16-teleport.md before
each live run. Restore every live write.
"""
from __future__ import annotations

import argparse
import math
import os
import struct
import sys
import time
from collections import Counter, defaultdict
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
OFF_TYPE = 0x4C
OFF_STATE = 0x340
LEM_READ = 0x350

# Object-shaped vtables (dtor, 0x41dc00, 0x41dc20, 0x41dc40, 0x4323eb, step).
OBJECT_VTS = [
    0x004B0D80,  # anti-grav object (cue AntiGrav)
    0x004B0DC8,
    0x004B0DE4,
    0x004B0E00,
    0x004B0F50,  # hatch
    0x004B0F90,  # exit
    0x004B1078,
    0x004B1110,
    0x004B112C,
    0x004B1148,
    0x004B1178,
    0x004B11E0,
    0x004B1250,  # teleport pad (MEASURED round 16)
    0x004B1288,
    0x004B12B4,
]

VT_TELEPORT_OUT = 0x004B0C1C  # type 0x28
VT_TELEPORT_SNAP = 0x004B0CE8  # type 0x2C

VT_NAMES = {
    0x004B0D28: "walk",
    0x004B0A20: "fall",
    0x004B0960: "climb",
    0x004B0AF8: "flip",
    0x004B0668: "splat",
    0x004B0CA8: "cartwheel",
    0x004B0B28: "exit-save",
    0x004B0570: "drown",
    0x004B0810: "bounce",
    VT_TELEPORT_OUT: "tele-out",
    VT_TELEPORT_SNAP: "tele-snap",
}


def f64_at(buf: bytes, off: int) -> float:
    return struct.unpack_from("<d", buf, off)[0]


def sample(h, ptr: int, t: float):
    buf = read_mem(h, ptr, LEM_READ)
    if not buf or len(buf) < LEM_READ:
        return None
    if struct.unpack_from("<I", buf, 0)[0] not in (LEMMING_VT, OTHER_VT):
        return None
    sp = struct.unpack_from("<I", buf, OFF_STATE)[0]
    vt = (u32(h, sp) or 0) if sp else 0
    return {
        "t": t,
        "ptr": ptr,
        "x": f64_at(buf, OFF_X),
        "y": f64_at(buf, OFF_Y),
        "z": f64_at(buf, OFF_Z),
        "facing": struct.unpack_from("<i", buf, OFF_FACING)[0],
        "vdir": struct.unpack_from("<i", buf, OFF_VDIR)[0],
        "type": struct.unpack_from("<i", buf, OFF_TYPE)[0],
        "vt": vt,
        "name": VT_NAMES.get(vt, f"{vt:#010x}"),
    }


def lemmings(h) -> list[int]:
    return [p for p in actor_ptrs(h) if u32(h, p) in (LEMMING_VT, OTHER_VT)]


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


def audit() -> int:
    lvls = levels_from_box(Path(r"C:\progs\Lemmings Revolution"))
    tele, fx = set(), set()
    bit_levels = defaultdict(set)
    poses = {}
    for name, data in lvls.items():
        low = data.lower()
        if b"teleport.a3d" in low:
            tele.add(name)
        if b"teleport_fx.a3d" in low:
            fx.add(name)
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
        if name in tele or name.upper() == "MM_TELE_003":
            ts = []
            idx = 0
            while True:
                i = low.find(b"teleport.a3d", idx)
                if i < 0:
                    break
                if i >= 4 and struct.unpack_from("<I", data, i - 4)[0] == 12:
                    p = pose_after(data, i - 4)
                    if p:
                        ts.append(p)
                idx = i + 1
            poses[name] = ts

    print(f"levels {len(lvls)}")
    print(f"teleport.a3d: {len(tele)}")
    print(f"teleport_fx.a3d: {len(fx)}")
    print(f"same set: {tele == fx}")
    exclusive = []
    for b, names in sorted(bit_levels.items()):
        in_t = names & tele
        in_o = names - tele
        if in_t == tele and not in_o:
            exclusive.append(b)
        if in_t and not in_o:
            print(f"bit {b:#06x} ONLY in tele levels ({len(in_t)})")
    print(f"bits exclusive to all tele levels: {[hex(b) for b in exclusive]}")
    name = "MM_TELE_003"
    # box keys may be path-like
    key = next((k for k in poses if "MM_TELE_003" in k.upper()), None)
    if key:
        print(f"{key}: {len(poses[key])} teleport.a3d poses")
        for i, (tx, ty, tz, col) in enumerate(poses[key]):
            print(f"  T{i}: ty={ty:.3f} col~{col:.3f} (tx={tx:.3f} tz={tz:.3f})")
    return 0 if tele == fx and len(tele) == 18 else 1


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
        for p in ptrs[:8]:
            buf = read_mem(h, p, 0xE0)
            if not buf or len(buf) < 0x78:
                continue
            x, y, z = f64_at(buf, 8), f64_at(buf, 0x10), f64_at(buf, 0x18)
            mn = [f64_at(buf, o) for o in (0x48, 0x50, 0x58)]
            mx = [f64_at(buf, o) for o in (0x60, 0x68, 0x70)]
            print(f"    {p:#010x} pose=({x:.3f},{y:.3f},{z:.3f}) "
                  f"AABB X[{mn[0]:.3f}..{mx[0]:.3f}] "
                  f"Y[{mn[1]:.3f}..{mx[1]:.3f}] "
                  f"Z[{mn[2]:.3f}..{mx[2]:.3f}]", flush=True)


def watch(h, seconds: float, hz: float):
    period = 1.0 / hz if hz > 0 else 0.0
    t0 = time.perf_counter()
    last = {}
    jumps = []
    print(f"watching {seconds}s at {hz} Hz for |dX|>2.5 or |dY|>5", flush=True)
    print("PREDICTION P2: a walker overlapping a teleport.a3d pad relocates "
          "near the paired pad's (col, ty); facing preserved or flipped — "
          "falsified by no jump in 40s on MM_TELE_003.", flush=True)
    while time.perf_counter() - t0 < seconds:
        t = time.perf_counter() - t0
        for p in lemmings(h):
            s = sample(h, p, t)
            if not s:
                continue
            prev = last.get(p)
            if prev is None:
                print(f"  t{t:7.3f}s {p:#010x} birth x={s['x']:.3f} "
                      f"y={s['y']:.3f} face={s['facing']:+d} "
                      f"{s['name']}", flush=True)
            else:
                dx = s["x"] - prev["x"]
                dy = s["y"] - prev["y"]
                if abs(dx) > 2.5 or abs(dy) > 5.0:
                    jumps.append((t, s, prev))
                    print(f"  t{t:7.3f}s {p:#010x} JUMP "
                          f"({prev['x']:.3f},{prev['y']:.3f})->"
                          f"({s['x']:.3f},{s['y']:.3f}) "
                          f"d=({dx:+.3f},{dy:+.3f}) "
                          f"face {prev['facing']:+d}->{s['facing']:+d} "
                          f"{prev['name']}->{s['name']} "
                          f"typ {prev['type']}->{s['type']}", flush=True)
                elif prev["vt"] != s["vt"]:
                    print(f"  t{t:7.3f}s {p:#010x} STATE "
                          f"{prev['name']}->{s['name']} "
                          f"x={s['x']:.3f} y={s['y']:.3f}", flush=True)
            last[p] = s
        if period:
            time.sleep(period)
    print(f"{len(jumps)} jumps; {len(last)} lemmings seen", flush=True)
    return jumps


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
            jumps = watch(h, args.watch, args.hz)
            return 0 if jumps else 2
        if not args.dump_objects:
            print("nothing to do; pass --audit / --watch / --dump-objects")
            return 0
        return 0
    finally:
        k32.CloseHandle(h)


if __name__ == "__main__":
    raise SystemExit(main())
