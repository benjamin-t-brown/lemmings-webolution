"""Verify pad pose/AABB stick across frames; try activate by writing +0xb8 path."""
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
    obj = find_pad(h)
    saved = read_mem(h, obj + 8, 0x70)
    print("pad", hex(obj))

    # pick lemming on list: first from 0x4c88d0 walk
    node = u32(h, 0x004C88D4) or 0
    target = u32(h, node) if node else 0
    print("list lem", hex(target) if target else None)
    b = read_mem(h, target, 0x60)
    x, y, z = struct.unpack_from("<ddd", b, 8)
    print(f"at x={x:.3f} y={y:.3f} z={z:.3f}")

    write_mem(h, obj + 8, struct.pack("<ddd", x, y, 0.0))
    write_mem(
        h,
        obj + 0x48,
        struct.pack("<dddddd", x - 2, y - 1, 0.0, x + 2, y + 8, 2.0),
    )
    time.sleep(0.2)
    ob = read_mem(h, obj, 0x78)
    print("after 0.2s pose", struct.unpack_from("<ddd", ob, 8))
    print("AABB mn", struct.unpack_from("<ddd", ob, 0x48))
    print("AABB mx", struct.unpack_from("<ddd", ob, 0x60))

    # manually put lemming ptr into +0xd8 list and call activate by
    # setting up state: write +0xd4 and invoke message via writing
    # FASTER onto lemming through known attach: set +0x50=2 and
    # allocate is hard — instead poke message target and +0xb8.
    # Direct behavioural measure: set +0x50=2, measure rate, clear.
    print("--- direct +0x50=2 rate test ---")
    mode0 = struct.unpack_from("<i", b, 0x50)[0]
    x0 = x
    write_mem(h, target + 0x50, struct.pack("<i", 2))
    time.sleep(0.5)
    b1 = read_mem(h, target, 0x60)
    x1 = struct.unpack_from("<d", b1, 8)[0]
    m1 = struct.unpack_from("<i", b1, 0x50)[0]
    print(f"mode {mode0}->{m1} rate={abs(x1 - x0) / 0.5:.3f}")
    write_mem(h, target + 0x50, struct.pack("<i", 1))
    time.sleep(0.5)
    b2 = read_mem(h, target, 0x60)
    x2 = struct.unpack_from("<d", b2, 8)[0]
    m2 = struct.unpack_from("<i", b2, 0x50)[0]
    print(f"mode back {m2} rate={abs(x2 - x1) / 0.5:.3f}")

    # Try forcing activate: push lem into overlap list storage at +0xd8
    # by writing +0xb8=0, zero timer, and see if continuous AABB stick works
    # with larger box for 3s while printing ost each 0.25s
    print("--- sticky AABB 3s ---")
    t0 = time.perf_counter()
    while time.perf_counter() - t0 < 3:
        b = read_mem(h, target, 0x60)
        x, y, z = struct.unpack_from("<ddd", b, 8)
        write_mem(h, obj + 8, struct.pack("<ddd", x, y, 0.0))
        write_mem(
            h,
            obj + 0x48,
            struct.pack("<dddddd", x - 3, y - 2, -1.0, x + 3, y + 10, 3.0),
        )
        write_mem(h, obj + 0xA8, struct.pack("<d", 0.0))
        ob = read_mem(h, obj, 0xC0)
        ost = struct.unpack_from("<i", ob, 0xB8)[0]
        mode = struct.unpack_from("<i", b, 0x50)[0]
        if ost or mode == 2:
            print("FLIP", ost, mode)
            break
        time.sleep(0.05)
    else:
        ob = read_mem(h, obj, 0xC0)
        print(
            "no flip; ost",
            struct.unpack_from("<i", ob, 0xB8)[0],
            "pose",
            struct.unpack_from("<ddd", ob, 8),
        )

    write_mem(h, obj + 8, saved)
    print("restored")
    k32.CloseHandle(h)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
