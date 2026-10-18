"""Proof v2: walk-state target, one-shot activate, dump +0x340/+0x374."""
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


def dump_sides(h, lem: int) -> None:
    main = u32(h, lem + 0x340) or 0
    mvt = u32(h, main) if main else 0
    print(f"  main +0x340={main:#x} vt={mvt:#x}")
    for off in (0x374, 0x378):
        raw = u32(h, lem + off) or 0
        print(f"  +{off:#x}={raw:#x}")
    # walk as intrusive list of nodes [state*, next]
    head = u32(h, lem + 0x374) or 0
    node = head
    for i in range(8):
        if not node:
            break
        st = u32(h, node) or 0
        nxt = u32(h, node + 4) or 0
        vt = u32(h, st) if st else 0
        typ = struct.unpack_from("<i", read_mem(h, st, 0x20), 0x14)[0] if st else -1
        tmr = (
            struct.unpack_from("<d", read_mem(h, st, 0x20), 0x18)[0] if st else 0
        )
        print(f"  list[{i}] node={node:#x} st={st:#x} vt={vt:#x} type={typ} tmr={tmr:.3f}")
        # also try: node IS the state (no wrapper)
        if st and vt == 0:
            vt2 = u32(h, node) or 0
            print(f"    alt node-as-state vt={vt2:#x}")
        node = nxt
        if node == head:
            break


def col_of(x: float, z: float) -> float:
    return (math.atan2(z, x) + math.pi) / (2 * math.pi) * 256.0


def main() -> int:
    h, pid = attach(write=True)
    focus(pid)
    print("ui", ui_state(h), "level", loaded_level(h), "pid", pid)
    pads = find_pads(h)
    obj = None
    for p in pads:
        write_mem(h, p + 0xA8, struct.pack("<d", 1.2))
        time.sleep(0.25)
        t1 = struct.unpack_from("<d", read_mem(h, p, 0xB0), 0xA8)[0]
        print(f"pad {p:#x} step 1.2->{t1:.3f}")
        if t1 < 1.15:
            obj = p
            break
    if not obj:
        obj = pads[0] if pads else None
    if not obj:
        k32.CloseHandle(h)
        return 1

    saved = read_mem(h, obj, 0xE0)
    msg = struct.unpack_from("<iiiiii", saved, 0xC0)
    print("msg", msg, "face_filt", struct.unpack_from("<i", saved, 0xBC)[0])

    # Find walking actor on the interaction list.
    target = None
    t0 = time.perf_counter()
    while time.perf_counter() - t0 < 60:
        on_list = set(listed(h))
        for lem in actor_ptrs(h):
            if lem not in on_list:
                continue
            if u32(h, lem) not in (LEMMING_VT, OTHER_VT):
                continue
            st = u32(h, lem + 0x340) or 0
            if not st or (u32(h, st) or 0) != VT_WALK:
                continue
            b = read_mem(h, lem, 0x60)
            mode = struct.unpack_from("<i", b, 0x50)[0]
            face = struct.unpack_from("<i", b, 0x40)[0]
            vdir = struct.unpack_from("<i", b, 0x44)[0]
            if mode != 1 or vdir != 1:
                continue
            x, y, z = struct.unpack_from("<ddd", b, 8)
            # need some motion
            time.sleep(0.25)
            b2 = read_mem(h, lem, 0x60)
            x2, y2, z2 = struct.unpack_from("<ddd", b2, 8)
            dc = abs(col_of(x2, z2) - col_of(x, z))
            if dc > 128:
                dc = 256 - dc
            if dc < 0.05:
                continue
            target = lem
            print(
                f"walker {lem:#x} face={face} col_delta/0.25={dc:.3f} "
                f"y={y:.2f}->{y2:.2f}"
            )
            break
        if target:
            break
        time.sleep(0.3)
    if not target:
        print("no moving walker on list; trying any listed walk-state")
        for lem in listed(h):
            st = u32(h, lem + 0x340) or 0
            if st and (u32(h, st) or 0) == VT_WALK:
                target = lem
                break
    if not target:
        print("FAIL no target")
        write_mem(h, obj, saved)
        k32.CloseHandle(h)
        return 2

    b = read_mem(h, target, 0x60)
    x, y, z = struct.unpack_from("<ddd", b, 8)
    c0 = col_of(x, z)
    time.sleep(0.4)
    b = read_mem(h, target, 0x60)
    x, y, z = struct.unpack_from("<ddd", b, 8)
    c1 = col_of(x, z)
    dc = abs(c1 - c0)
    if dc > 128:
        dc = 256 - dc
    base = dc / 0.4
    print(f"baseline col/s={base:.3f} mode={struct.unpack_from('<i', b, 0x50)[0]}")
    dump_sides(h, target)

    # One-shot: park pad on walker, clear latch once, zero timer, wait frames.
    write_mem(h, obj + 8, struct.pack("<ddd", x, y, z))
    write_mem(
        h,
        obj + 0x48,
        struct.pack("<dddddd", x - 4, y - 3, z - 4, x + 4, y + 12, z + 4),
    )
    write_mem(h, obj + 0xB8, struct.pack("<i", 0))
    write_mem(h, obj + 0xA4, struct.pack("<I", 0))
    write_mem(h, obj + 0xA8, struct.pack("<d", 0.0))

    hit = False
    for frame in range(40):
        b = read_mem(h, target, 0x60)
        x, y, z = struct.unpack_from("<ddd", b, 8)
        # keep AABB glued without clearing latch every time
        write_mem(h, obj + 8, struct.pack("<ddd", x, y, z))
        write_mem(
            h,
            obj + 0x48,
            struct.pack("<dddddd", x - 4, y - 3, z - 4, x + 4, y + 12, z + 4),
        )
        if frame < 3:
            write_mem(h, obj + 0xA8, struct.pack("<d", 0.0))
            write_mem(h, obj + 0xB8, struct.pack("<i", 0))

        mode = struct.unpack_from("<i", b, 0x50)[0]
        latch = struct.unpack_from("<i", read_mem(h, obj, 0xC0), 0xB8)[0]
        main = u32(h, target + 0x340) or 0
        mvt = u32(h, main) if main else 0
        if mode == 2 or latch == 1 or mvt == VT_FASTER:
            print(f"HIT frame={frame} mode={mode} latch={latch} main_vt={mvt:#x}")
            dump_sides(h, target)
            # rate
            c0 = col_of(x, z)
            time.sleep(0.5)
            b2 = read_mem(h, target, 0x60)
            x2, _, z2 = struct.unpack_from("<ddd", b2, 8)
            c2 = col_of(x2, z2)
            dc = abs(c2 - c0)
            if dc > 128:
                dc = 256 - dc
            boost = dc / 0.5
            print(
                f"boost col/s={boost:.3f} mode={struct.unpack_from('<i', b2, 0x50)[0]} "
                f"ratio={boost / base if base > 0.05 else float('nan'):.2f}"
            )
            dump_sides(h, target)
            hit = True
            break
        time.sleep(0.05)

    write_mem(h, obj, saved)
    print("restored; hit", hit)
    k32.CloseHandle(h)
    return 0 if hit else 3


if __name__ == "__main__":
    raise SystemExit(main())
