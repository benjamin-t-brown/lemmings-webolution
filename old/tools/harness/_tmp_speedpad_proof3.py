"""Proof v3: walk +0x378 list; move walker onto pad; rate + FASTER node."""
from __future__ import annotations

import ctypes
import math
import struct
import sys
import time

sys.path.insert(0, ".")
from diffscan import regions  # noqa: E402
from goto_level import focus, loaded_level  # noqa: E402
from lrstate import attach, u32, ui_state  # noqa: E402
from measure_spawn import LEMMING_VT, OTHER_VT, actor_ptrs  # noqa: E402
from memdump import k32, read_mem  # noqa: E402

VT_FASTER = 0x004B0A38
VT_WALK = 0x004B0D28
VT_PAD = 0x004B1178


def write_mem(h, addr: int, data: bytes) -> bool:
    n = ctypes.c_size_t(0)
    return bool(
        k32.WriteProcessMemory(
            h, ctypes.c_void_p(addr), data, len(data), ctypes.byref(n)
        )
    )


def find_pads(h):
    needle = struct.pack("<I", VT_PAD)
    out = []
    for base, size in regions(h):
        blob = read_mem(h, base, size)
        if not blob:
            continue
        i = 0
        while True:
            j = blob.find(needle, i)
            if j < 0:
                break
            if j % 4 == 0:
                out.append(base + j)
            i = j + 4
    return out


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


def side_states(h, lem: int):
    """Walk +0x374/+0x378 0xC-node chains; return [(st, vt, tmr)]."""
    found = []
    for head_off in (0x00, 0x04):
        node = u32(h, lem + 0x374 + head_off) or 0
        seen = set()
        while node and node not in seen and len(seen) < 16:
            seen.add(node)
            d = read_mem(h, node, 0x0C)
            if not d or len(d) != 0x0C:
                break
            val, _a, nxt = struct.unpack("<III", d)
            if val:
                vt = u32(h, val) or 0
                raw = read_mem(h, val + 0x18, 8)
                tmr = struct.unpack("<d", raw)[0] if raw and len(raw) == 8 else None
                found.append((val, vt, tmr))
            node = nxt
    return found


def col_of(x: float, z: float) -> float:
    return (math.atan2(z, x) + math.pi) / (2 * math.pi) * 256.0


def main() -> int:
    h, pid = attach(write=True)
    focus(pid)
    print("ui", ui_state(h), "level", loaded_level(h), "pid", pid)
    pads = find_pads(h)
    obj = None
    for p in pads:
        write_mem(h, p + 0xA8, struct.pack("<d", 0.8))
        time.sleep(0.2)
        t1 = struct.unpack_from("<d", read_mem(h, p, 0xB0), 0xA8)[0]
        print(f"pad {p:#x} step->{t1:.3f}")
        if t1 < 0.78:
            obj = p
            break
    if not obj:
        obj = pads[0]
    saved_pad = read_mem(h, obj, 0xE0)
    px, py, pz = struct.unpack_from("<ddd", saved_pad, 8)
    aabb = saved_pad[0x48:0x78]
    msg = struct.unpack_from("<iiii", saved_pad, 0xC4)
    print(f"using pad {obj:#x} pose=({px:.2f},{py:.2f},{pz:.2f}) msg{msg}")

    # Wait for a walking listed lemming.
    target = None
    t0 = time.perf_counter()
    while time.perf_counter() - t0 < 90:
        for lem in listed(h):
            if u32(h, lem) not in (LEMMING_VT, OTHER_VT):
                continue
            st = u32(h, lem + 0x340) or 0
            if not st or (u32(h, st) or 0) != VT_WALK:
                continue
            mode = struct.unpack_from("<i", read_mem(h, lem, 0x54), 0x50)[0]
            vdir = struct.unpack_from("<i", read_mem(h, lem, 0x48), 0x44)[0]
            if mode == 1 and vdir == 1:
                target = lem
                break
        if target:
            break
        time.sleep(0.25)
    if not target:
        print("no walker")
        write_mem(h, obj, saved_pad)
        k32.CloseHandle(h)
        return 2

    saved_lem_xyz = read_mem(h, target + 8, 24)
    print(f"target {target:#x} sides before={side_states(h, target)}")

    # Baseline rate at current spot.
    b = read_mem(h, target, 0x60)
    x0, y0, z0 = struct.unpack_from("<ddd", b, 8)
    c0 = col_of(x0, z0)
    time.sleep(0.5)
    b = read_mem(h, target, 0x60)
    x1, y1, z1 = struct.unpack_from("<ddd", b, 8)
    c1 = col_of(x1, z1)
    dc = abs(c1 - c0)
    if dc > 128:
        dc = 256 - dc
    base = dc / 0.5
    print(f"baseline col/s={base:.3f} at y={y1:.2f}")

    # Place feet inside pad AABB (natural pad, not moved).
    # Pad AABB from dump: X[px-1..px+1] Y[py..py+6] Z[0..1]
    # Put walker at pad pose Y = py (feet on pad).
    write_mem(h, target + 8, struct.pack("<ddd", px, py, 0.5))
    # Ensure pad ready: latch 0, timer 0
    write_mem(h, obj + 0xB8, struct.pack("<i", 0))
    write_mem(h, obj + 0xA4, struct.pack("<I", 0))
    write_mem(h, obj + 0xA8, struct.pack("<d", 0.0))

    hit = False
    for frame in range(60):
        # keep on pad
        write_mem(h, target + 8, struct.pack("<ddd", px, py, 0.5))
        if frame < 2:
            write_mem(h, obj + 0xA8, struct.pack("<d", 0.0))
            write_mem(h, obj + 0xB8, struct.pack("<i", 0))

        mode = struct.unpack_from("<i", read_mem(h, target, 0x54), 0x50)[0]
        latch = struct.unpack_from("<i", read_mem(h, obj, 0xC0), 0xB8)[0]
        sides = side_states(h, target)
        has_f = any(vt == VT_FASTER for _, vt, _ in sides)
        if mode == 2 or latch == 1 or has_f:
            print(f"HIT frame={frame} mode={mode} latch={latch} hasF={has_f}")
            print("  sides:", [(hex(s), hex(v), t) for s, v, t in sides])
            # rate while mode 2 — stop forcing X so they can walk
            # only pin Y near pad for a moment then free
            write_mem(h, target + 8, saved_lem_xyz)  # restore XY so walk resumes where they were
            # re-set mode if restore somehow... no, xyz only
            time.sleep(0.05)
            # if mode cleared by leaving, re-poke via pad once more at original
            # Instead: measure with mode still 2 at restored position
            b = read_mem(h, target, 0x60)
            mode = struct.unpack_from("<i", b, 0x50)[0]
            x0, y0, z0 = struct.unpack_from("<ddd", b, 8)
            c0 = col_of(x0, z0)
            time.sleep(0.5)
            b = read_mem(h, target, 0x60)
            x1, y1, z1 = struct.unpack_from("<ddd", b, 8)
            c1 = col_of(x1, z1)
            dc = abs(c1 - c0)
            if dc > 128:
                dc = 256 - dc
            boost = dc / 0.5
            mode2 = struct.unpack_from("<i", b, 0x50)[0]
            sides2 = side_states(h, target)
            print(
                f"boost col/s={boost:.3f} mode={mode2} "
                f"ratio={boost / base if base > 0.05 else float('nan'):.2f}"
            )
            print("  sides after:", [(hex(s), hex(v), t) for s, v, t in sides2])
            hit = True
            break
        time.sleep(0.05)

    # Direct +0x50 poke rate (behavioural of the mode bit), then restore.
    print("--- direct mode poke rate ---")
    b = read_mem(h, target, 0x60)
    x0, y0, z0 = struct.unpack_from("<ddd", b, 8)
    mode0 = struct.unpack_from("<i", b, 0x50)[0]
    write_mem(h, target + 0x50, struct.pack("<i", 2))
    time.sleep(0.5)
    b = read_mem(h, target, 0x60)
    x1, y1, z1 = struct.unpack_from("<ddd", b, 8)
    c0, c1 = col_of(x0, z0), col_of(x1, z1)
    dc = abs(c1 - c0)
    if dc > 128:
        dc = 256 - dc
    r2 = dc / 0.5
    write_mem(h, target + 0x50, struct.pack("<i", 1))
    time.sleep(0.5)
    b = read_mem(h, target, 0x60)
    x2, y2, z2 = struct.unpack_from("<ddd", b, 8)
    c2 = col_of(x2, z2)
    dc = abs(c2 - c1)
    if dc > 128:
        dc = 256 - dc
    r1 = dc / 0.5
    print(f"poke mode2 col/s={r2:.3f} mode1 col/s={r1:.3f} (was mode {mode0})")

    write_mem(h, obj, saved_pad)
    write_mem(h, target + 8, saved_lem_xyz)
    write_mem(h, target + 0x50, struct.pack("<i", 1))
    print("restored pad+lem; hit", hit)
    k32.CloseHandle(h)
    return 0 if hit else 3


if __name__ == "__main__":
    raise SystemExit(main())
