"""Is the SpeedPad object being stepped? Does forced activate attach FASTER?"""
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


def pars(h, p):
    out = []
    node = u32(h, p + 0x374) or 0
    for _ in range(8):
        if not node:
            break
        s = u32(h, node) or 0
        if s:
            out.append(u32(h, s) or 0)
        node = u32(h, node + 4) or 0
    return out


def main() -> int:
    h, pid = attach(write=True)
    focus(pid)
    print("ui", ui_state(h))
    obj = find_pad(h)
    print("pad", hex(obj))
    saved = read_mem(h, obj, 0xE0)

    # 1) timer countdown proves step runs
    write_mem(h, obj + 0xA8, struct.pack("<d", 2.0))
    time.sleep(0.5)
    tmr = struct.unpack_from("<d", read_mem(h, obj, 0xB0), 0xA8)[0]
    print(f"timer 2.0 -> {tmr:.3f} after 0.5s (stepping if < 2)")

    # 2) place AABB on grounded walker + force +0xd8 entry manually
    target = None
    for p in actor_ptrs(h):
        if u32(h, p) not in (LEMMING_VT, OTHER_VT):
            continue
        b = read_mem(h, p, 0x60)
        x, y = struct.unpack_from("<dd", b, 8)
        if abs(y - 29) < 0.2 and 90 < x < 140:
            target = p
            break
    print("target", hex(target) if target else None)
    b = read_mem(h, target, 0x60)
    x, y, z = struct.unpack_from("<ddd", b, 8)

    write_mem(h, obj + 8, struct.pack("<ddd", x, y, 0.0))
    write_mem(
        h,
        obj + 0x48,
        struct.pack("<dddddd", x - 2, y - 1, -1.0, x + 2, y + 8, 2.0),
    )
    write_mem(h, obj + 0xA8, struct.pack("<d", 0.0))
    write_mem(h, obj + 0xB8, struct.pack("<i", 0))

    # manually seed +0xd4 with lemming and try setting +0xb8 via
    # writing the message's target then calling activate by
    # flipping through the deactivate/activate timer path:
    # if overlap works, ost becomes 1.
    t0 = time.perf_counter()
    while time.perf_counter() - t0 < 2.5:
        b = read_mem(h, target, 0x60)
        x, y, z = struct.unpack_from("<ddd", b, 8)
        write_mem(h, obj + 8, struct.pack("<ddd", x, y, 0.0))
        write_mem(
            h,
            obj + 0x48,
            struct.pack("<dddddd", x - 2, y - 1, -1.0, x + 2, y + 8, 2.0),
        )
        write_mem(h, obj + 0xA8, struct.pack("<d", 0.0))
        # also keep +0xbc = 0 (any face) and +0xa0 = 0
        write_mem(h, obj + 0xBC, struct.pack("<i", 0))
        write_mem(h, obj + 0xA0, b"\x00")
        ob = read_mem(h, obj, 0xC0)
        ost = struct.unpack_from("<i", ob, 0xB8)[0]
        mode = struct.unpack_from("<i", b, 0x50)[0]
        ps = pars(h, target)
        if ost == 1 or mode == 2 or VT_FASTER in ps:
            print(
                f"HIT ost={ost} mode={mode} par={[hex(v) for v in ps]} "
                f"x={x:.2f}"
            )
            break
        time.sleep(0.05)
    else:
        print("no natural activate")

    # 3) Forge: write +0xd4 = target, then set fields and poke
    #    FUN_00433b2e path by writing a one-shot: call activate via
    #    setting +0xb8 from 0 with a non-empty +0xd8 is hard without
    #    allocator. Directly attach by writing mode=2 is already MEASURED.
    #    Instead: write message +0xd4 and use known skill-assign helper if any.

    # Restore full object
    write_mem(h, obj, saved)
    print("restored vt", hex(u32(h, obj) or 0))
    k32.CloseHandle(h)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
