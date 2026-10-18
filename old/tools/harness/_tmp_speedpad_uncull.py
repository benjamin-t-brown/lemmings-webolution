"""Move SpeedPad near walkers; check stepping + FASTER attach; restore."""
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


def find_pad(h) -> int | None:
    needle = struct.pack("<I", 0x004B1178)
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
                return base + j
            i = j + 4
    return None


def main() -> int:
    h, pid = attach(write=True)
    focus(pid)
    print("ui", ui_state(h))
    obj = find_pad(h)
    saved = read_mem(h, obj, 0xE0)
    print("pad", hex(obj))

    # pick walker
    target = None
    for p in actor_ptrs(h):
        if u32(h, p) not in (LEMMING_VT, OTHER_VT):
            continue
        b = read_mem(h, p, 0x60)
        x, y = struct.unpack_from("<dd", b, 8)
        mode = struct.unpack_from("<i", b, 0x50)[0]
        if abs(y - 29) < 0.2 and mode == 1 and 100 < x < 140:
            target = p
            break
    b = read_mem(h, target, 0x60)
    x, y, z = struct.unpack_from("<ddd", b, 8)
    print(f"target {target:#x} x={x:.3f}")

    # move pose + AABB to lemming and keep there
    for _ in range(5):
        b = read_mem(h, target, 0x60)
        x, y, z = struct.unpack_from("<ddd", b, 8)
        write_mem(h, obj + 8, struct.pack("<ddd", x, y, 0.0))
        write_mem(
            h,
            obj + 0x48,
            struct.pack("<dddddd", x - 1.5, y, 0.0, x + 1.5, y + 6.0, 1.0),
        )
        time.sleep(0.1)

    # test stepping
    write_mem(h, obj + 0xA8, struct.pack("<d", 3.0))
    time.sleep(0.5)
    tmr = struct.unpack_from("<d", read_mem(h, obj, 0xB0), 0xA8)[0]
    print(f"after relocate: timer 3.0 -> {tmr:.3f}")

    write_mem(h, obj + 0xA8, struct.pack("<d", 0.0))
    write_mem(h, obj + 0xB8, struct.pack("<i", 0))

    hit = False
    t0 = time.perf_counter()
    while time.perf_counter() - t0 < 5:
        b = read_mem(h, target, 0x60)
        x, y, z = struct.unpack_from("<ddd", b, 8)
        write_mem(h, obj + 8, struct.pack("<ddd", x, y, 0.0))
        write_mem(
            h,
            obj + 0x48,
            struct.pack("<dddddd", x - 1.5, y, 0.0, x + 1.5, y + 6.0, 1.0),
        )
        write_mem(h, obj + 0xA8, struct.pack("<d", 0.0))
        mode = struct.unpack_from("<i", b, 0x50)[0]
        ost = struct.unpack_from("<i", read_mem(h, obj, 0xC0), 0xB8)[0]
        head = u32(h, target + 0x374) or 0
        has = False
        node = head
        for _ in range(6):
            if not node:
                break
            s = u32(h, node) or 0
            if s and (u32(h, s) or 0) == VT_FASTER:
                has = True
            node = u32(h, node + 4) or 0
        if ost == 1 or mode == 2 or has:
            print(f"HIT t={time.perf_counter()-t0:.2f} mode={mode} ost={ost} hasF={has}")
            x0 = x
            time.sleep(0.5)
            x1 = struct.unpack_from("<d", read_mem(h, target, 0x60), 8)[0]
            print(f"rate={abs(x1 - x0) / 0.5:.3f}")
            hit = True
            break
        time.sleep(0.05)

    write_mem(h, obj, saved)
    print("restored; hit", hit)
    k32.CloseHandle(h)
    return 0 if hit else 2


if __name__ == "__main__":
    raise SystemExit(main())
