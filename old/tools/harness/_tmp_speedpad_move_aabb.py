"""Move SpeedPad AABB onto a walker, watch FASTER, restore pad bytes."""
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
    print("ui", ui_state(h), "focus", focus(pid))
    obj = find_pad(h)
    if not obj:
        print("no pad")
        return 1
    # save pose + AABB region (+8 .. +0x78)
    saved = read_mem(h, obj + 8, 0x70)
    print("pad", hex(obj), "saved", len(saved), "bytes")

    target = None
    for p in actor_ptrs(h):
        if u32(h, p) not in (LEMMING_VT, OTHER_VT):
            continue
        b = read_mem(h, p, 0x60)
        x, y, z = struct.unpack_from("<ddd", b, 8)
        mode = struct.unpack_from("<i", b, 0x50)[0]
        if abs(y - 29) < 0.5 and mode == 1 and 80 < x < 150:
            target = p
            print(f"target {p:#x} x={x:.3f} y={y:.3f} z={z:.3f}")
            break
    if not target:
        print("no target")
        k32.CloseHandle(h)
        return 1

    b = read_mem(h, target, 0x60)
    x, y, z = struct.unpack_from("<ddd", b, 8)
    # place pad AABB around lemming
    pose = struct.pack("<ddd", x, y, 0.0)
    aabb = struct.pack(
        "<dddddd",
        x - 1.0, y, 0.0,
        x + 1.0, y + 6.0, 1.0,
    )
    write_mem(h, obj + 8, pose)
    write_mem(h, obj + 0x48, aabb)
    write_mem(h, obj + 0xA8, struct.pack("<d", 0.0))
    write_mem(h, obj + 0xB8, struct.pack("<i", 0))  # allow re-fire
    print("pad AABB moved onto lemming")

    t0 = time.perf_counter()
    hit = False
    while time.perf_counter() - t0 < 8:
        # keep AABB tracking the lemming
        b = read_mem(h, target, 0x60)
        if not b:
            break
        x, y, z = struct.unpack_from("<ddd", b, 8)
        mode = struct.unpack_from("<i", b, 0x50)[0]
        write_mem(h, obj + 8, struct.pack("<ddd", x, y, 0.0))
        write_mem(
            h,
            obj + 0x48,
            struct.pack("<dddddd", x - 1, y, 0.0, x + 1, y + 6, 1.0),
        )
        write_mem(h, obj + 0xA8, struct.pack("<d", 0.0))

        ob = read_mem(h, obj, 0xC0)
        ost = struct.unpack_from("<i", ob, 0xB8)[0]
        head = u32(h, target + 0x374) or 0
        pars = []
        node = head
        for _ in range(8):
            if not node:
                break
            s = u32(h, node) or 0
            if s:
                pars.append(u32(h, s) or 0)
            node = u32(h, node + 4) or 0
        if mode == 2 or VT_FASTER in pars or ost == 1:
            hit = True
            print(
                f"HIT t={time.perf_counter()-t0:.2f} x={x:.3f} mode={mode} "
                f"ost={ost} par={[hex(v) for v in pars]}"
            )
            x0 = x
            time.sleep(0.5)
            b = read_mem(h, target, 0x60)
            x1 = struct.unpack_from("<d", b, 8)[0]
            m1 = struct.unpack_from("<i", b, 0x50)[0]
            print(f"rate={abs(x1 - x0) / 0.5:.3f} mode_now={m1}")
            # watch mode return to 1
            t1 = time.perf_counter()
            while time.perf_counter() - t1 < 8:
                b = read_mem(h, target, 0x60)
                m = struct.unpack_from("<i", b, 0x50)[0]
                if m != 2:
                    print(
                        f"mode left 2 at +{time.perf_counter()-t0:.2f}s "
                        f"now={m}"
                    )
                    break
                time.sleep(0.1)
            break
        time.sleep(0.05)

    # restore pad
    write_mem(h, obj + 8, saved)
    print("pad restored; verify vt", hex(u32(h, obj) or 0))
    ob = read_mem(h, obj + 8, 0x70)
    print("pose back", struct.unpack_from("<ddd", ob, 0))
    print("hit", hit)
    k32.CloseHandle(h)
    return 0 if hit else 2


if __name__ == "__main__":
    raise SystemExit(main())
