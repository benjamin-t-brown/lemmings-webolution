"""Clean SpeedPad -> FASTER proof on current level. Restore all writes."""
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
from measure_spawn import LEMMING_VT, OTHER_VT  # noqa: E402
from memdump import k32, read_mem  # noqa: E402

VT_FASTER = 0x004B0A38
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


def par_vts(h, lem: int):
    head = u32(h, lem + 0x374) or 0
    out = []
    node = head
    for _ in range(12):
        if not node:
            break
        s = u32(h, node) or 0
        if s:
            out.append(u32(h, s) or 0)
        node = u32(h, node + 4) or 0
    return out


def col_of(x: float, z: float) -> float:
    return (math.atan2(z, x) + math.pi) / (2 * math.pi) * 256.0


def dump_pad(h, obj: int) -> None:
    b = read_mem(h, obj, 0xE0)
    x, y, z = struct.unpack_from("<ddd", b, 8)
    tmr = struct.unpack_from("<d", b, 0xA8)[0]
    period = struct.unpack_from("<d", b, 0x98)[0]
    a0 = b[0xA0]
    latch = struct.unpack_from("<i", b, 0xB8)[0]
    face_filt = struct.unpack_from("<i", b, 0xBC)[0]
    msg = struct.unpack_from("<iiiiii", b, 0xC0)
    print(
        f"pad {obj:#x} pose=({x:.2f},{y:.2f},{z:.2f}) "
        f"tmr={tmr:.3f} period={period:.3f} a0={a0} "
        f"latch={latch} face_filt={face_filt} msg={msg}"
    )


def main() -> int:
    h, pid = attach(write=True)
    focus(pid)
    print("ui", ui_state(h), "level", loaded_level(h), "pid", pid)
    pads = find_pads(h)
    print("pads", [hex(p) for p in pads])
    if not pads:
        k32.CloseHandle(h)
        return 1

    # Prefer a pad whose timer is ticking (in view / stepped).
    obj = pads[0]
    for p in pads:
        write_mem(h, p + 0xA8, struct.pack("<d", 1.2))
        time.sleep(0.25)
        t1 = struct.unpack_from("<d", read_mem(h, p, 0xB0), 0xA8)[0]
        print(f"  step-check {p:#x}: 1.2 -> {t1:.3f}")
        if t1 < 1.15:
            obj = p
            break

    saved = read_mem(h, obj, 0xE0)
    dump_pad(h, obj)

    target = None
    face_filt = struct.unpack_from("<i", saved, 0xBC)[0]
    a0 = saved[0xA0]
    t0 = time.perf_counter()
    while time.perf_counter() - t0 < 60:
        for lem in listed(h):
            if u32(h, lem) not in (LEMMING_VT, OTHER_VT):
                continue
            b = read_mem(h, lem, 0x60)
            if not b:
                continue
            mode = struct.unpack_from("<i", b, 0x50)[0]
            face = struct.unpack_from("<i", b, 0x40)[0]
            vdir = struct.unpack_from("<i", b, 0x44)[0]
            if mode != 1:
                continue
            if face_filt not in (0, face):
                continue
            if a0 == 0 and vdir != 1:
                continue
            if a0 != 0 and vdir != -1:
                continue
            target = lem
            x, y, z = struct.unpack_from("<ddd", b, 8)
            print(
                f"target {lem:#x} x={x:.2f} y={y:.2f} z={z:.2f} "
                f"face={face} vdir={vdir}"
            )
            break
        if target:
            break
        time.sleep(0.2)
    if not target:
        print("no matching walker on list 0x004C88D0")
        write_mem(h, obj, saved)
        k32.CloseHandle(h)
        return 2

    # Baseline walk rate (column units / s) before pad contact.
    b0 = read_mem(h, target, 0x60)
    x0, y0, z0 = struct.unpack_from("<ddd", b0, 8)
    c0 = col_of(x0, z0)
    time.sleep(0.4)
    b1 = read_mem(h, target, 0x60)
    x1, y1, z1 = struct.unpack_from("<ddd", b1, 8)
    c1 = col_of(x1, z1)
    base_rate = abs(c1 - c0) / 0.4
    # unwrap near 0/256
    if abs(c1 - c0) > 128:
        base_rate = (256.0 - abs(c1 - c0)) / 0.4
    print(f"baseline col_rate={base_rate:.3f} mode={struct.unpack_from('<i', b1, 0x50)[0]}")

    hit = False
    t1 = time.perf_counter()
    while time.perf_counter() - t1 < 10:
        b = read_mem(h, target, 0x60)
        if not b:
            break
        x, y, z = struct.unpack_from("<ddd", b, 8)
        # Move pad onto walker; do NOT touch message at +0xc0 or face filter.
        write_mem(h, obj + 8, struct.pack("<ddd", x, y, z))
        write_mem(
            h,
            obj + 0x48,
            struct.pack("<dddddd", x - 3, y - 2, z - 3, x + 3, y + 10, z + 3),
        )
        write_mem(h, obj + 0xA8, struct.pack("<d", 0.0))  # timer ready
        write_mem(h, obj + 0xB8, struct.pack("<i", 0))  # clear latch so activate can fire
        write_mem(h, obj + 0xA4, struct.pack("<I", 0))  # clear last-hit sticky

        mode = struct.unpack_from("<i", b, 0x50)[0]
        latch = struct.unpack_from("<i", read_mem(h, obj, 0xC0), 0xB8)[0]
        pvts = par_vts(h, target)
        has_f = VT_FASTER in pvts
        if latch == 1 or mode == 2 or has_f:
            print(
                f"HIT latch={latch} mode={mode} hasF={has_f} "
                f"pars={[hex(v) for v in pvts]}"
            )
            dump_pad(h, obj)
            # Measure boosted rate while mode stays 2.
            bx, by, bz = x, y, z
            bc = col_of(bx, bz)
            time.sleep(0.5)
            b2 = read_mem(h, target, 0x60)
            x2, y2, z2 = struct.unpack_from("<ddd", b2, 8)
            c2 = col_of(x2, z2)
            boost = abs(c2 - bc) / 0.5
            if abs(c2 - bc) > 128:
                boost = (256.0 - abs(c2 - bc)) / 0.5
            m2 = struct.unpack_from("<i", b2, 0x50)[0]
            pvts2 = par_vts(h, target)
            print(
                f"boost col_rate={boost:.3f} mode_now={m2} "
                f"hasF_now={VT_FASTER in pvts2} "
                f"ratio={boost / base_rate if base_rate > 0.01 else float('nan'):.2f}"
            )
            hit = True
            break
        time.sleep(0.05)

    write_mem(h, obj, saved)
    # Do not force-clear lemming mode — leave engine state; pad restored.
    print("restored pad; hit", hit)
    dump_pad(h, obj)
    k32.CloseHandle(h)
    return 0 if hit else 3


if __name__ == "__main__":
    raise SystemExit(main())
