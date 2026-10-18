"""Force overlap: verify list membership + manual AABB math + seed +0xd8."""
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


def on_list(h, lem: int) -> bool:
    node = u32(h, 0x004C88D4) or 0
    for _ in range(64):
        if not node or node == 0x004C88DC:
            return False
        if (u32(h, node) or 0) == lem:
            return True
        node = u32(h, node + 4) or 0
    return False


def aabb_hit(ob: bytes, x, y, z) -> bool:
    mn = struct.unpack_from("<ddd", ob, 0x48)
    mx = struct.unpack_from("<ddd", ob, 0x60)
    return (
        x >= mn[0]
        and y >= mn[1]
        and z >= mn[2]
        and x <= mx[0]
        and y <= mx[1]
        and z <= mx[2]
    )


def main() -> int:
    h, pid = attach(write=True)
    focus(pid)
    pads = find_pads(h)
    obj = max(pads, key=lambda p: struct.unpack_from("<d", read_mem(h, p, 0x20), 8)[0])
    saved = read_mem(h, obj, 0xE0)
    ox = struct.unpack_from("<d", saved, 8)[0]
    oy = struct.unpack_from("<d", saved, 0x10)[0]
    print(f"pad {obj:#x} ({ox:.1f},{oy:.1f})")

    # find listed walker
    target = None
    for p in actor_ptrs(h):
        if u32(h, p) not in (LEMMING_VT, OTHER_VT):
            continue
        if on_list(h, p):
            b = read_mem(h, p, 0x60)
            x, y, z = struct.unpack_from("<ddd", b, 8)
            mode = struct.unpack_from("<i", b, 0x50)[0]
            face = struct.unpack_from("<i", b, 0x40)[0]
            vdir = struct.unpack_from("<i", b, 0x44)[0]
            print(
                f"listed {p:#x} x={x:.2f} y={y:.2f} z={z:.2f} "
                f"mode={mode} face={face} vdir={vdir}"
            )
            if mode == 1 and vdir == 1:
                target = p
                break
    if not target:
        print("no listed walker")
        k32.CloseHandle(h)
        return 1

    orig = struct.unpack_from("<ddd", read_mem(h, target, 0x60), 8)
    write_mem(h, target + 8, struct.pack("<d", ox))
    write_mem(h, target + 0x10, struct.pack("<d", oy))
    write_mem(h, target + 0x18, struct.pack("<d", 0.75))
    write_mem(h, obj + 0xA8, struct.pack("<d", 0.0))
    write_mem(h, obj + 0xB8, struct.pack("<i", 0))

    b = read_mem(h, target, 0x60)
    x, y, z = struct.unpack_from("<ddd", b, 8)
    ob = read_mem(h, obj, 0x78)
    print(
        f"placed x={x:.2f} y={y:.2f} z={z:.2f} "
        f"aabb_hit={aabb_hit(ob, x, y, z)} still_listed={on_list(h, target)}"
    )

    t0 = time.perf_counter()
    while time.perf_counter() - t0 < 5:
        write_mem(h, target + 8, struct.pack("<d", ox))
        write_mem(h, target + 0x10, struct.pack("<d", oy))
        write_mem(h, target + 0x18, struct.pack("<d", 0.75))
        write_mem(h, obj + 0xA8, struct.pack("<d", 0.0))
        b = read_mem(h, target, 0x60)
        mode = struct.unpack_from("<i", b, 0x50)[0]
        ost = struct.unpack_from("<i", read_mem(h, obj, 0xC0), 0xB8)[0]
        # check +0xd8 list count roughly via first ptr
        d8 = u32(h, obj + 0xD8) or 0
        if ost == 1 or mode == 2:
            print(f"HIT ost={ost} mode={mode} d8={d8:#x}")
            break
        time.sleep(0.05)
    else:
        print(
            f"miss ost={ost} mode={mode} listed={on_list(h, target)} "
            f"aabb={aabb_hit(read_mem(h, obj, 0x78), ox, oy, 0.75)}"
        )

    write_mem(h, target + 8, struct.pack("<d", orig[0]))
    write_mem(h, target + 0x10, struct.pack("<d", orig[1]))
    write_mem(h, target + 0x18, struct.pack("<d", orig[2]))
    write_mem(h, obj, saved)
    write_mem(h, target + 0x50, struct.pack("<i", 1))
    print("restored")
    k32.CloseHandle(h)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
