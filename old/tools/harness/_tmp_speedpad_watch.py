"""Focus + natural watch for SpeedPad trigger on H10_MT_1."""
from __future__ import annotations

import struct
import sys
import time

sys.path.insert(0, ".")
from diffscan import regions  # noqa: E402
from goto_level import focus  # noqa: E402
from lrstate import attach, u32  # noqa: E402
from measure_spawn import LEMMING_VT, OTHER_VT, actor_ptrs  # noqa: E402
from memdump import k32, read_mem  # noqa: E402

VT_FASTER = 0x004B0A38


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
    h, pid = attach(write=False)
    if not h:
        print("no game")
        return 1
    print("focus", focus(pid), "pid", pid)
    obj = find_pad(h)
    print("pad", hex(obj) if obj else None)
    t0 = time.perf_counter()
    last = 0.0
    hit = False
    best = None
    while time.perf_counter() - t0 < 55:
        t = time.perf_counter() - t0
        ob = read_mem(h, obj, 0xC0)
        if not ob:
            break
        ost = struct.unpack_from("<i", ob, 0xB8)[0]
        best = None
        for p in actor_ptrs(h):
            if u32(h, p) not in (LEMMING_VT, OTHER_VT):
                continue
            b = read_mem(h, p, 0x60)
            if not b:
                continue
            x, y, z = struct.unpack_from("<ddd", b, 8)
            mode = struct.unpack_from("<i", b, 0x50)[0]
            face = struct.unpack_from("<i", b, 0x40)[0]
            if abs(y - 29) > 1:
                continue
            head = u32(h, p + 0x374) or 0
            has = False
            node = head
            for _ in range(6):
                if not node:
                    break
                s = u32(h, node) or 0
                if s and (u32(h, s) or 0) == VT_FASTER:
                    has = True
                node = u32(h, node + 4) or 0
            if best is None or x > best[0]:
                best = (x, mode, face, has, p)
            if mode == 2 or has or ost == 1:
                print(
                    f"HIT t={t:.2f} x={x:.2f} mode={mode} hasF={has} "
                    f"ost={ost} p={p:#x}"
                )
                x0 = x
                time.sleep(0.5)
                b = read_mem(h, p, 0x60)
                x1 = struct.unpack_from("<d", b, 8)[0]
                m1 = struct.unpack_from("<i", b, 0x50)[0]
                ob2 = read_mem(h, obj, 0xC0)
                print(
                    f"rate={abs(x1 - x0) / 0.5:.3f} mode={m1} "
                    f"ost={struct.unpack_from('<i', ob2, 0xB8)[0]}"
                )
                hit = True
                break
        if hit:
            break
        if t - last >= 3 and best:
            print(
                f"t={t:.1f} leadx={best[0]:.2f} face={best[2]:+d} "
                f"mode={best[1]} ost={ost}"
            )
            last = t
        time.sleep(0.04)
    print("done hit", hit, "best", best)
    k32.CloseHandle(h)
    return 0 if hit else 2


if __name__ == "__main__":
    raise SystemExit(main())
