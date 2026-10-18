"""Round 11: the exit save test is object-overlap AABB, not a guessed rectangle.

Static read (stock exe, image base 0x400000), before any live write — see
docs/scratch/phase3-round11-exit.md for the full prediction table.

    overlap predicate 0x00430BA1: point in [obj+0x48..+0x70] on X/Y/Z
    AABB setter        0x00430FE1: forces Z to [0, 1.0]
    exit object        vtable 0x004B0F90, step 0x0042688D
    save state         vtable 0x004B0B28, type 0x2A
    saved counter      word at 0x004CBAA4 (inc when state+8 >= 1.0)

Modes
-----
    python tools/harness/measure_exit.py --dump
        Scan heap for exit vtables; print pose + AABB + margins.

    python tools/harness/measure_exit.py --enter MM_CLIMB_002 --watch 40 --hz 100
        Follow every lemming; print the first entry into the save state and
        every rise of 0x004CBAA4, with (X,Y,Z) and inside-AABB flags.

    python tools/harness/measure_exit.py --enter MM_CLIMB_002 --tight
        After the first natural save, report the lemming's (X,Y) against each
        dumped AABB edge (the binary-search anchor).

Do not put UCRT64 bin on PATH here. `lrstate.attach` returns (handle, pid).
"""
from __future__ import annotations

import argparse
import os
import struct
import sys
import time
from pathlib import Path

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))

from diffscan import regions  # noqa: E402
from lrstate import attach, i32, u32, ui_state  # noqa: E402
from measure_spawn import LEMMING_VT, OTHER_VT, actor_ptrs, f64  # noqa: E402
from memdump import k32, read_mem  # noqa: E402

EXIT_OBJ_VT = 0x004B0F90
SAVE_STATE_VT = 0x004B0B28
SAVE_TYPE = 0x2A
SAVED_WORD = 0x004CBAA4
OFF_X, OFF_Y, OFF_Z = 0x08, 0x10, 0x18
OFF_STATE = 0x340
OFF_TYPE = 0x4C
LEM_READ = 0x344

VT_WALK = 0x004B0D28
VT_FALL = 0x004B0A20
VT_NAMES = {
    VT_WALK: "walk", VT_FALL: "fall", SAVE_STATE_VT: "exit-save",
    0x004B0960: "climb", 0x004B0AF8: "flip", 0x004B0668: "splat",
    0x004B07B8: "block", 0x004B0CA8: "cartwheel",
}


def i16(h, addr):
    b = read_mem(h, addr, 2)
    return struct.unpack("<h", b)[0] if b and len(b) == 2 else None


def saved_count(h) -> int | None:
    return i16(h, SAVED_WORD)


def find_exits(h) -> list[int]:
    needle = struct.pack("<I", EXIT_OBJ_VT)
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


def read_aabb(h, ptr: int) -> dict | None:
    """Pose at +8/+10/+18 and AABB doubles at +0x48..+0x70."""
    buf = read_mem(h, ptr, 0x228)
    if not buf or len(buf) < 0x78:
        return None
    if struct.unpack_from("<I", buf, 0)[0] != EXIT_OBJ_VT:
        return None

    def d(off):
        return struct.unpack_from("<d", buf, off)[0]

    out = {
        "ptr": ptr,
        "x": d(OFF_X), "y": d(OFF_Y), "z": d(OFF_Z),
        "min_x": d(0x48), "min_y": d(0x50), "min_z": d(0x58),
        "max_x": d(0x60), "max_y": d(0x68), "max_z": d(0x70),
        "team": struct.unpack_from("<I", buf, 0x98)[0],
    }
    if len(buf) >= 0x228:
        out["off_min_x"] = d(0x208)
        out["off_min_y"] = d(0x210)
        out["off_max_x"] = d(0x218)
        out["off_max_y"] = d(0x220)
    return out


def inside(box: dict, x: float, y: float, z: float) -> bool:
    return (box["min_x"] <= x <= box["max_x"]
            and box["min_y"] <= y <= box["max_y"]
            and box["min_z"] <= z <= box["max_z"])


def dump_exits(h) -> list[dict]:
    ptrs = find_exits(h)
    print(f"{len(ptrs)} exit-object vtables (0x{EXIT_OBJ_VT:08x})", flush=True)
    boxes = []
    for p in ptrs:
        b = read_aabb(h, p)
        if not b:
            continue
        boxes.append(b)
        w = b["max_x"] - b["min_x"]
        hgt = b["max_y"] - b["min_y"]
        print(f"  exit {p:#010x} pose ({b['x']:.3f},{b['y']:.3f},{b['z']:.3f})"
              f" team={b['team']}", flush=True)
        print(f"    AABB X[{b['min_x']:.3f} .. {b['max_x']:.3f}] "
              f"Y[{b['min_y']:.3f} .. {b['max_y']:.3f}] "
              f"Z[{b['min_z']:.3f} .. {b['max_z']:.3f}] "
              f"size ({w:.3f} x {hgt:.3f})", flush=True)
        print(f"    pose-relative: "
              f"X[{b['min_x']-b['x']:+.3f} .. {b['max_x']-b['x']:+.3f}] "
              f"Y[{b['min_y']-b['y']:+.3f} .. {b['max_y']-b['y']:+.3f}]",
              flush=True)
        if "off_min_x" in b:
            print(f"    +0x208.. margins "
                  f"({b['off_min_x']:+.3f},{b['off_min_y']:+.3f}).."
                  f"({b['off_max_x']:+.3f},{b['off_max_y']:+.3f})",
                  flush=True)
    return boxes


def sample_lem(h, ptr: int, t: float):
    buf = read_mem(h, ptr, LEM_READ)
    if not buf or len(buf) != LEM_READ:
        return None
    if struct.unpack_from("<I", buf, 0)[0] not in (LEMMING_VT, OTHER_VT):
        return None
    sp = struct.unpack_from("<I", buf, OFF_STATE)[0]
    vt = (u32(h, sp) or 0) if sp else 0
    stype = (u32(h, sp + 0x14) or 0) if sp else 0
    return {
        "t": t, "ptr": ptr,
        "x": struct.unpack_from("<d", buf, OFF_X)[0],
        "y": struct.unpack_from("<d", buf, OFF_Y)[0],
        "z": struct.unpack_from("<d", buf, OFF_Z)[0],
        "lem_type": struct.unpack_from("<i", buf, OFF_TYPE)[0],
        "vt": vt,
        "state_type": stype,
        "name": VT_NAMES.get(vt, f"{vt:#010x}"),
        "state_ptr": sp,
    }


def watch(h, seconds: float, hz: float, boxes: list[dict]):
    period = 1.0 / hz if hz > 0 else 0.0
    t0 = time.perf_counter()
    last = {}
    prev_saved = saved_count(h)
    events = []
    print(f"watching {seconds}s at {hz} Hz; saved={prev_saved} "
          f"exits={len(boxes)}", flush=True)
    while time.perf_counter() - t0 < seconds:
        t = time.perf_counter() - t0
        sc = saved_count(h)
        if sc is not None and prev_saved is not None and sc != prev_saved:
            print(f"  t{t:7.3f}s SAVED {prev_saved} -> {sc}", flush=True)
            events.append(("saved", t, sc))
            prev_saved = sc
        for p in actor_ptrs(h):
            s = sample_lem(h, p, t)
            if not s:
                continue
            prev = last.get(p)
            flags = []
            for i, b in enumerate(boxes):
                if inside(b, s["x"], s["y"], s["z"]):
                    flags.append(f"in#{i}")
            tag = ",".join(flags) if flags else "out"
            if prev is None:
                print(f"  t{t:7.3f}s {p:#010x} birth "
                      f"x={s['x']:.3f} y={s['y']:.3f} z={s['z']:.3f} "
                      f"{s['name']} typ={s['state_type']} {tag}", flush=True)
            elif prev["vt"] != s["vt"] or prev["state_type"] != s["state_type"]:
                print(f"  t{t:7.3f}s {p:#010x} STATE {prev['name']}->"
                      f"{s['name']} typ {prev['state_type']}->{s['state_type']} "
                      f"x={s['x']:.3f} y={s['y']:.3f} z={s['z']:.3f} {tag}",
                      flush=True)
                events.append(("state", t, s))
                if s["vt"] == SAVE_STATE_VT or s["state_type"] == SAVE_TYPE:
                    for i, b in enumerate(boxes):
                        print(f"           vs exit#{i}: "
                              f"dx_lo={s['x']-b['min_x']:+.3f} "
                              f"dx_hi={b['max_x']-s['x']:+.3f} "
                              f"dy_lo={s['y']-b['min_y']:+.3f} "
                              f"dy_hi={b['max_y']-s['y']:+.3f} "
                              f"inside={inside(b, s['x'], s['y'], s['z'])}",
                              flush=True)
            last[p] = s
        if period:
            time.sleep(period)
    print(f"\n{len(events)} events; final saved={saved_count(h)}", flush=True)
    return events


def main() -> int:
    ap = argparse.ArgumentParser(
        description=__doc__.splitlines()[0],
        formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--enter", metavar="LEVEL")
    ap.add_argument("--dump", action="store_true",
                    help="scan and print exit AABBs")
    ap.add_argument("--watch", type=float, default=0.0,
                    help="seconds to watch saves / state changes")
    ap.add_argument("--hz", type=float, default=100.0)
    ap.add_argument("--tight", action="store_true",
                    help="dump + watch; emphasise first save edges")
    args = ap.parse_args()

    h, pid = attach(write=bool(args.enter))
    if not h:
        print("the game is not running")
        return 1
    print(f"pid {pid}  ui_state {ui_state(h)}", flush=True)
    try:
        if args.enter:
            from measure_float_splat import enter
            from goto_level import loaded_level
            rc = enter(h, args.enter)
            # Round 11: goto_level can silently fail; path at 0x004CBBF8 is
            # the independent check. Refuse to dump if the wrong level loaded.
            path = loaded_level(h)
            print(f"VERIFY loaded_level={path!r}", flush=True)
            if args.enter.lower() not in path.lower().replace("\\", "/"):
                print(f"REFUSING dump: wanted {args.enter!r}, got {path!r}",
                      flush=True)
                return 3
            if rc:
                return rc
            time.sleep(1.0)
        boxes = dump_exits(h) if (args.dump or args.watch or args.tight) else []
        if args.dump and not (args.watch or args.tight):
            return 0 if boxes else 2
        if args.watch or args.tight:
            if not boxes:
                boxes = dump_exits(h)
            print("PREDICTION: a save / type-0x2a attach fires only while the "
                  "lemming's raw (X,Y,Z) is inside an exit AABB; Z slab is "
                  "[0,1].", flush=True)
            events = watch(h, args.watch or 40.0, args.hz, boxes)
            saves = [e for e in events if e[0] == "saved"]
            enters = [e for e in events
                      if e[0] == "state"
                      and (e[2]["vt"] == SAVE_STATE_VT
                           or e[2]["state_type"] == SAVE_TYPE)]
            print(f"verdict anchors: {len(enters)} exit-state entries, "
                  f"{len(saves)} saved-counter rises", flush=True)
            return 0 if (enters or saves) else 2
        print("nothing to do; pass --dump and/or --watch")
        return 0
    finally:
        k32.CloseHandle(h)


if __name__ == "__main__":
    raise SystemExit(main())
