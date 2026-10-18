"""Wait for a listed walker; move pad AABB onto it; watch FASTER; restore."""
from __future__ import annotations

import ctypes
import struct
import sys
import time

sys.path.insert(0, ".")
from diffscan import regions  # noqa: E402
from goto_level import focus  # noqa: E402
from lrstate import attach, u32, ui_state  # noqa: E402
from measure_spawn import LEMMING_VT, OTHER_VT, actor_ptrs  # noqa: E402
from memdump import k32, read_mem  # noqa: E402

VT_FASTER = 0x004B0A38


def write_mem(h, addr: int, data: bytes) -> bool:
    n = ctypes.c_size_t(0)
    return bool(
        k32.WriteProcessMemory(
            h, ctypes.c_void_p(addr), data, len(data), ctypes.byref(n)
        )
    )


def find_pads(h):
    needle = struct.pack("<I", 0x004B1178)
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


def main() -> int:
    h, pid = attach(write=True)
    focus(pid)
    print("ui", ui_state(h), "pid", pid)
    pads = find_pads(h)
    # use a stepped pad (either)
    obj = pads[0]
    saved = read_mem(h, obj, 0xE0)
    print("pad", hex(obj), "step check...")
    write_mem(h, obj + 0xA8, struct.pack("<d", 1.5))
    time.sleep(0.35)
    tmr = struct.unpack_from("<d", read_mem(h, obj, 0xB0), 0xA8)[0]
    print(f"timer 1.5 -> {tmr:.3f}")

    target = None
    t0 = time.perf_counter()
    while time.perf_counter() - t0 < 45:
        for lem in listed(h):
            if u32(h, lem) not in (LEMMING_VT, OTHER_VT):
                continue
            b = read_mem(h, lem, 0x60)
            if not b:
                continue
            mode = struct.unpack_from("<i", b, 0x50)[0]
            vdir = struct.unpack_from("<i", b, 0x44)[0]
            face = struct.unpack_from("<i", b, 0x40)[0]
            x, y, z = struct.unpack_from("<ddd", b, 8)
            if mode == 1 and vdir == 1:
                target = lem
                print(
                    f"got listed {lem:#x} x={x:.2f} y={y:.2f} "
                    f"face={face} vdir={vdir}"
                )
                break
        if target:
            break
        time.sleep(0.15)
    if not target:
        print("timeout waiting for listed walker")
        write_mem(h, obj, saved)
        k32.CloseHandle(h)
        return 1

    hit = False
    t1 = time.perf_counter()
    while time.perf_counter() - t1 < 8:
        b = read_mem(h, target, 0x60)
        if not b:
            break
        x, y, z = struct.unpack_from("<ddd", b, 8)
        write_mem(h, obj + 8, struct.pack("<ddd", x, y, 0.0))
        write_mem(
            h,
            obj + 0x48,
            struct.pack("<dddddd", x - 2, y - 1, -1.0, x + 2, y + 8, 2.0),
        )
        write_mem(h, obj + 0xA8, struct.pack("<d", 0.0))
        write_mem(h, obj + 0xB8, struct.pack("<i", 0))
        write_mem(h, obj + 0xBC, struct.pack("<i", 0))
        write_mem(h, obj + 0xA0, b"\x00")

        mode = struct.unpack_from("<i", b, 0x50)[0]
        ost = struct.unpack_from("<i", read_mem(h, obj, 0xC0), 0xB8)[0]
        head = u32(h, target + 0x374) or 0
        has = False
        node = head
        for _ in range(8):
            if not node:
                break
            s = u32(h, node) or 0
            if s and (u32(h, s) or 0) == VT_FASTER:
                has = True
            node = u32(h, node + 4) or 0
        if ost == 1 or mode == 2 or has:
            print(f"HIT mode={mode} ost={ost} hasF={has} x={x:.2f} y={y:.2f}")
            x0 = x
            time.sleep(0.5)
            b = read_mem(h, target, 0x60)
            x1 = struct.unpack_from("<d", b, 8)[0]
            m1 = struct.unpack_from("<i", b, 0x50)[0]
            print(f"rate={abs(x1 - x0) / 0.5:.3f} mode_now={m1}")
            hit = True
            break
        time.sleep(0.05)

    write_mem(h, obj, saved)
    if target:
        write_mem(h, target + 0x50, struct.pack("<i", 1))
    print("restored hit", hit)
    k32.CloseHandle(h)
    return 0 if hit else 2


if __name__ == "__main__":
    raise SystemExit(main())
