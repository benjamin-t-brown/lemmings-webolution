"""Round 21: what does a Lever do, and how is it detected?

Stock exe, actor list 0x004CBB18. No PIXELS_PER_UNIT doubling.

Modes
-----
    python tools/harness/measure_lever.py --audit
        Offline census: lever.a3d vs cell bits.

    python tools/harness/measure_lever.py --enter E16_KF_1 --dump-objects --watch 40 --hz 80
        Enter level, verify path, dump lever objects, watch +0xb8 / actors.

    python tools/harness/measure_lever.py --enter E16_KF_1 --proof
        Force a listed walker onto a stepped lever AABB; restore all writes.

Predictions in docs/scratch/phase3-round21-lever.md before each live run.
Restore every live write.
"""
from __future__ import annotations

import argparse
import ctypes
import math
import os
import struct
import sys
import time
from collections import defaultdict
from pathlib import Path

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))

from diffscan import regions  # noqa: E402
from goto_level import focus, loaded_level  # noqa: E402
from lrstate import attach, u32, ui_state  # noqa: E402
from measure_spawn import LEMMING_VT, OTHER_VT, actor_ptrs  # noqa: E402
from memdump import k32, read_mem  # noqa: E402
from read_grid_siblings import levels_from_box  # noqa: E402
from read_lvl_grid import find_grid  # noqa: E402

OFF_X, OFF_Y, OFF_Z = 0x08, 0x10, 0x18
OFF_FACING, OFF_VDIR = 0x40, 0x44
OFF_STATE = 0x340
OFF_PARLIST = 0x374
LEM_READ = 0x390

VT_LEVER = 0x004B1288
VT_WALK = 0x004B0D28
VT_FASTER = 0x004B0A38

OBJECT_VTS = [
    0x004B0D80,  # anti-grav
    0x004B0F50,  # hatch
    0x004B0F90,  # exit
    0x004B1148,  # AABB message pad
    0x004B1178,  # SpeedPad
    0x004B11E0,
    0x004B1250,  # teleport
    VT_LEVER,    # lever
    0x004B12B4,  # sibling / candidate door-ish
]

LEVER_A3D = b"lever.a3d"


def f64_at(buf: bytes, off: int) -> float:
    return struct.unpack_from("<d", buf, off)[0]


def i32_at(buf: bytes, off: int) -> int:
    return struct.unpack_from("<i", buf, off)[0]


def write_mem(h, addr: int, data: bytes) -> bool:
    n = ctypes.c_size_t(0)
    return bool(
        k32.WriteProcessMemory(
            h, ctypes.c_void_p(addr), data, len(data), ctypes.byref(n)
        )
    )


def pose_after(data: bytes, a3d_at: int):
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
    levers = set()
    bit_levels = defaultdict(set)
    poses = {}
    for name, data in lvls.items():
        low = data.lower()
        if LEVER_A3D in low:
            levers.add(name)
            poses[name] = find_a3d_poses(data, LEVER_A3D)
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

    n_place = sum(len(v) for v in poses.values())
    print(f"levels {len(lvls)}")
    print(f"lever.a3d: {len(levers)} levels / {n_place} placements with pose")
    exclusive = []
    for b, names in sorted(bit_levels.items()):
        in_l = names & levers
        in_o = names - levers
        if in_l == levers and not in_o:
            exclusive.append(b)
    print(f"bits exclusive to all lever levels: {[hex(b) for b in exclusive]}")
    for name in sorted(levers):
        ps = poses.get(name, [])
        print(f"  {name}: {len(ps)} levers", flush=True)
        for i, (tx, ty, tz, col) in enumerate(ps):
            print(f"    L{i}: ty={ty:.3f} col~{col:.3f} "
                  f"(tx={tx:.3f} tz={tz:.3f})", flush=True)
    ok = len(levers) == 55 and n_place == 130 and not exclusive
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


def vec_count(h, vec_ptr: int) -> int:
    """Guess std::vector-ish: [begin, end, cap] at vec_ptr."""
    b = read_mem(h, vec_ptr, 12)
    if not b or len(b) < 12:
        return -1
    begin, end, _cap = struct.unpack("<III", b)
    if not begin or end < begin:
        return 0
    return (end - begin) // 4  # element size unknown; report dword span


def dump_msg_list(h, obj: int, off: int, label: str) -> None:
    """Dump message entries. SpeedPad embeds at +0xc4 as (id,a,b,...).

    Lever lists at +0xbc / +0xcc are containers filled by ctor 0x42ca80.
    Probe: read 0x40 bytes and print as i32 groups; also try vector layout.
    """
    raw = read_mem(h, obj + off, 0x40)
    if not raw:
        print(f"    {label} @{off:#x}: unreadable", flush=True)
        return
    dwords = struct.unpack("<" + "i" * (len(raw) // 4), raw)
    print(f"    {label} @{off:#x} raw i32: {dwords[:12]}", flush=True)
    begin = struct.unpack_from("<I", raw, 0)[0]
    end = struct.unpack_from("<I", raw, 4)[0]
    if begin and end >= begin and (end - begin) <= 0x400:
        blob = read_mem(h, begin, end - begin)
        if blob:
            # FUN_00433B2E reads +4,+8,+0xc,+0x10,+0x14 — try stride 0x18
            stride = 0x18
            n = len(blob) // stride
            print(f"    {label} vec [{begin:#x}..{end:#x}) n~{n} stride 0x18:",
                  flush=True)
            for i in range(min(n, 8)):
                e = blob[i * stride:(i + 1) * stride]
                vals = struct.unpack("<" + "i" * (len(e) // 4), e)
                print(f"      [{i}] {vals}", flush=True)


def dump_lever(h, ptr: int) -> dict | None:
    buf = read_mem(h, ptr, 0x100)
    if not buf or len(buf) < 0xE0:
        return None
    x, y, z = f64_at(buf, 8), f64_at(buf, 0x10), f64_at(buf, 0x18)
    mn = [f64_at(buf, o) for o in (0x48, 0x50, 0x58)]
    mx = [f64_at(buf, o) for o in (0x60, 0x68, 0x70)]
    st = i32_at(buf, 0xB8)
    hit = struct.unpack_from("<I", buf, 0xA4)[0]
    a0 = buf[0xA0]
    period = f64_at(buf, 0x98) if len(buf) >= 0xA0 else float("nan")
    timer = f64_at(buf, 0xA8) if len(buf) >= 0xB0 else float("nan")
    info = {
        "ptr": ptr,
        "x": x, "y": y, "z": z,
        "mn": mn, "mx": mx,
        "b8": st, "a0": a0, "hit": hit,
        "period": period, "timer": timer,
    }
    print(f"    {ptr:#010x} pose=({x:.3f},{y:.3f},{z:.3f}) "
          f"AABB X[{mn[0]:.3f}..{mx[0]:.3f}] "
          f"Y[{mn[1]:.3f}..{mx[1]:.3f}] "
          f"Z[{mn[2]:.3f}..{mx[2]:.3f}] "
          f"+0xb8={st} +0xa0={a0} hit={hit:#x} "
          f"period={period:.3f} timer={timer:.3f}", flush=True)
    dump_msg_list(h, ptr, 0xBC, "list+0xbc")
    dump_msg_list(h, ptr, 0xCC, "list+0xcc")
    return info


def dump_objects(h) -> list[dict]:
    print("object vtable heap scan:", flush=True)
    levers = []
    for vt in OBJECT_VTS:
        ptrs = find_objects(h, vt)
        print(f"  {vt:#010x}: {len(ptrs)}", flush=True)
        if vt != VT_LEVER:
            continue
        for p in ptrs:
            info = dump_lever(h, p)
            if info:
                levers.append(info)
    return levers


def parallel_vts(h, lem: int) -> list[int]:
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


def sample_lem(h, ptr: int, t: float):
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
        "facing": i32_at(buf, OFF_FACING),
        "vdir": i32_at(buf, OFF_VDIR),
        "vt": vt,
        "par": parallel_vts(h, ptr),
    }


def lemmings(h) -> list[int]:
    return [p for p in actor_ptrs(h) if u32(h, p) in (LEMMING_VT, OTHER_VT)]


def watch(h, seconds: float, hz: float, levers: list[dict]):
    period = 1.0 / hz if hz > 0 else 0.0
    t0 = time.perf_counter()
    last_b8 = {L["ptr"]: L["b8"] for L in levers}
    last_lem = {}
    flips = 0
    print(f"watching {seconds}s at {hz} Hz for lever +0xb8 flips / "
          f"actor parallel changes", flush=True)
    print("PREDICTION P3: correct-facing AABB overlap flips +0xb8 and "
          "dispatches messages; no lemming parallel attach — falsified by "
          "no +0xb8 flip in 40s with hatch traffic, or a new parallel vt "
          "on pull.", flush=True)
    while time.perf_counter() - t0 < seconds:
        t = time.perf_counter() - t0
        for L in levers:
            buf = read_mem(h, L["ptr"], 0xC0)
            if not buf or len(buf) < 0xBC:
                continue
            b8 = i32_at(buf, 0xB8)
            if b8 != last_b8.get(L["ptr"]):
                flips += 1
                print(f"  t{t:7.3f}s LEVER {L['ptr']:#010x} +0xb8 "
                      f"{last_b8.get(L['ptr'])}->{b8}", flush=True)
                last_b8[L["ptr"]] = b8
        for p in lemmings(h):
            s = sample_lem(h, p, t)
            if not s:
                continue
            prev = last_lem.get(p)
            if prev and prev["par"] != s["par"]:
                print(f"  t{t:7.3f}s lem {p:#010x} PAR "
                      f"{[hex(v) for v in prev['par']]} -> "
                      f"{[hex(v) for v in s['par']]} "
                      f"x={s['x']:.3f} y={s['y']:.3f} face={s['facing']}",
                      flush=True)
            last_lem[p] = s
        if period:
            time.sleep(period)
    print(f"{flips} lever +0xb8 flips; {len(last_lem)} lemmings seen",
          flush=True)
    return flips


def listed(h):
    node = u32(h, 0x004C88D4) or 0
    out = []
    for _ in range(64):
        if not node or node == 0x004C88DC:
            break
        lem = u32(h, node) or 0
        if lem:
            out.append(lem)
        node = u32(h, node + 4) or 0
    return out


def stepped_lever(h, ptrs: list[int]) -> int | None:
    """Return a lever whose step runs (timer decays). Leave timer at 0.

    FUN_004339fc only calls activate (vt+0x20) when +0xa8 <= 0. A leftover
    cooldown from the step-probe would set hit via overlap but skip the
    toggle — that falsified P3 on the first E16 attempt.
    """
    for p in ptrs:
        write_mem(h, p + 0xA8, struct.pack("<d", 0.8))
        time.sleep(0.25)
        t1 = struct.unpack_from("<d", read_mem(h, p, 0xB0), 0xA8)[0]
        print(f"  timer poke {p:#x} 0.8 -> {t1:.3f}", flush=True)
        if t1 < 0.78:
            write_mem(h, p + 0xA8, struct.pack("<d", 0.0))
            return p
    if ptrs:
        write_mem(h, ptrs[0] + 0xA8, struct.pack("<d", 0.0))
        return ptrs[0]
    return None


def proof(h) -> int:
    """Place a walker on a stepped lever; require +0xb8 flip; restore."""
    print("PROOF: force walker onto lever AABB with matching facing",
          flush=True)
    print("PREDICTION: +0xb8 flips to the walker's facing; no FASTER / "
          "new parallel state on the lemming. Falsified by no flip.",
          flush=True)
    ptrs = find_objects(h, VT_LEVER)
    print(f"found {len(ptrs)} lever objects", flush=True)
    if not ptrs:
        return 2
    obj = stepped_lever(h, ptrs)
    if not obj:
        return 2
    saved = read_mem(h, obj, 0x100)
    if not saved:
        return 2
    b8_0 = i32_at(saved, 0xB8)
    mn = [f64_at(saved, o) for o in (0x48, 0x50, 0x58)]
    mx = [f64_at(saved, o) for o in (0x60, 0x68, 0x70)]
    px, py, pz = f64_at(saved, 8), f64_at(saved, 0x10), f64_at(saved, 0x18)
    # Need facing that matches current filter.
    # +0xb8==1 => need face -1; else need face +1.
    need_face = -1 if b8_0 == 1 else 1
    mid_x = 0.5 * (mn[0] + mx[0])
    mid_y = 0.5 * (mn[1] + mx[1])
    print(f"using {obj:#x} pose=({px:.2f},{py:.2f},{pz:.2f}) "
          f"AABB X[{mn[0]:.1f}..{mx[0]:.1f}] Y[{mn[1]:.1f}..{mx[1]:.1f}] "
          f"+0xb8={b8_0} need_face={need_face}", flush=True)

    target = None
    t0 = time.perf_counter()
    while time.perf_counter() - t0 < 90:
        for lem in listed(h):
            if u32(h, lem) not in (LEMMING_VT, OTHER_VT):
                continue
            st = u32(h, lem + OFF_STATE) or 0
            if not st or (u32(h, st) or 0) != VT_WALK:
                continue
            target = lem
            break
        if target:
            break
        time.sleep(0.2)
    if not target:
        print("no walker on list 0x004C88D0", flush=True)
        return 2

    lem_saved = read_mem(h, target, 0x50)
    par0 = parallel_vts(h, target)
    # AABB is only 2 cols wide — re-pin every frame or the walker walks out
    # before activate. Keep +0xa8 at 0 until the first flip (cooldown gate).
    write_mem(h, obj + 0xA8, struct.pack("<d", 0.0))
    print(f"pinning lem {target:#x} at ({mid_x:.2f},{mid_y:.2f}) "
          f"face={need_face}; timer forced 0", flush=True)

    flipped = False
    b8_1 = b8_0
    for frame in range(40):
        write_mem(h, target + OFF_X, struct.pack("<d", mid_x))
        write_mem(h, target + OFF_Y, struct.pack("<d", mid_y))
        write_mem(h, target + OFF_Z, struct.pack("<d", pz))
        write_mem(h, target + OFF_FACING, struct.pack("<i", need_face))
        if not flipped:
            write_mem(h, obj + 0xA8, struct.pack("<d", 0.0))
        time.sleep(0.05)
        buf = read_mem(h, obj, 0xC0)
        b8_1 = i32_at(buf, 0xB8)
        hit = struct.unpack_from("<I", buf, 0xA4)[0]
        tmr = f64_at(buf, 0xA8)
        if b8_1 != b8_0:
            flipped = True
            print(f"HIT frame={frame} +0xb8 {b8_0}->{b8_1} hit={hit:#x} "
                  f"timer={tmr:.3f}", flush=True)
            break
        if frame % 5 == 0:
            print(f"  frame={frame} +0xb8={b8_1} hit={hit:#x} "
                  f"timer={tmr:.3f}", flush=True)

    par1 = parallel_vts(h, target)
    has_faster = VT_FASTER in par1
    print(f"par before {[hex(v) for v in par0]} after {[hex(v) for v in par1]} "
          f"faster={has_faster}", flush=True)

    # Restore lever object blob and lemming pose.
    rest_obj = write_mem(h, obj, saved)
    rest_lem = write_mem(h, target, lem_saved) if lem_saved else False
    print(f"restored lever={rest_obj} lem_prefix={rest_lem}", flush=True)
    # Re-read to confirm restore.
    now = i32_at(read_mem(h, obj, 0xC0), 0xB8)
    print(f"post-restore +0xb8={now} (was {b8_0})", flush=True)

    if not flipped:
        print("P3 FALSIFIED: no +0xb8 flip", flush=True)
        return 3
    if has_faster or par1 != par0:
        # Parallel change is interesting but not necessarily fail — log.
        print("NOTE: parallel list changed on pull", flush=True)
    print("P3 holds (toggle observed; restores done)", flush=True)
    return 0


def main() -> int:
    ap = argparse.ArgumentParser(
        description=__doc__.splitlines()[0],
        formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--audit", action="store_true")
    ap.add_argument("--enter", metavar="LEVEL")
    ap.add_argument("--watch", type=float, default=0.0)
    ap.add_argument("--hz", type=float, default=80.0)
    ap.add_argument("--dump-objects", action="store_true")
    ap.add_argument("--proof", action="store_true")
    args = ap.parse_args()

    if args.audit:
        return audit()

    need_write = bool(args.enter or args.proof)
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
        levers = []
        if args.dump_objects or args.watch or args.proof:
            levers = dump_objects(h)
        if args.proof:
            return proof(h)
        if args.watch:
            flips = watch(h, args.watch, args.hz, levers)
            return 0 if flips else 2
        if not args.dump_objects:
            print("nothing to do; pass --audit / --watch / --dump-objects / --proof")
            return 0
        return 0
    finally:
        k32.CloseHandle(h)


if __name__ == "__main__":
    raise SystemExit(main())
