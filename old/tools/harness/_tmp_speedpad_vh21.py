"""On VH21_JC: put walker on stepped SpeedPad; watch FASTER; restore."""
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


def find_pads(h) -> list[int]:
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


def main() -> int:
    h, pid = attach(write=True)
    focus(pid)
    print("ui", ui_state(h))
    pads = find_pads(h)
    print("pads", [hex(p) for p in pads])
    # prefer pad near X=223
    obj = None
    for p in pads:
        x = struct.unpack_from("<d", read_mem(h, p, 0x20), 8)[0]
        if x > 100:
            obj = p
            break
    if not obj:
        obj = pads[0]
    saved = read_mem(h, obj, 0xE0)
    ox = struct.unpack_from("<d", saved, 8)[0]
    oy = struct.unpack_from("<d", saved, 0x10)[0]
    print(f"using pad {obj:#x} at ({ox:.1f},{oy:.1f})")

    # confirm stepping
    write_mem(h, obj + 0xA8, struct.pack("<d", 2.0))
    time.sleep(0.4)
    tmr = struct.unpack_from("<d", read_mem(h, obj, 0xB0), 0xA8)[0]
    print(f"stepping: 2.0 -> {tmr:.3f}")

    target = None
    orig = None
    deadline = time.perf_counter() + 20
    while time.perf_counter() < deadline and not target:
        for p in actor_ptrs(h):
            if u32(h, p) not in (LEMMING_VT, OTHER_VT):
                continue
            b = read_mem(h, p, 0x60)
            if not b:
                continue
            x, y, z = struct.unpack_from("<ddd", b, 8)
            mode = struct.unpack_from("<i", b, 0x50)[0]
            vdir = struct.unpack_from("<i", b, 0x44)[0]
            if mode == 1 and vdir == 1:
                target = p
                orig = (x, y, z)
                print(f"target {p:#x} x={x:.3f} y={y:.3f} vdir={vdir}")
                break
        if not target:
            time.sleep(0.2)
    if not target:
        print("no target")
        write_mem(h, obj, saved)
        k32.CloseHandle(h)
        return 1

    # teleport walker onto pad center
    write_mem(h, target + 8, struct.pack("<d", ox))
    write_mem(h, target + 0x10, struct.pack("<d", oy))
    write_mem(h, target + 0x18, struct.pack("<d", 0.75))
    write_mem(h, obj + 0xA8, struct.pack("<d", 0.0))
    write_mem(h, obj + 0xB8, struct.pack("<i", 0))
    print("placed on pad")

    t0 = time.perf_counter()
    hit = False
    while time.perf_counter() - t0 < 6:
        # keep on pad
        write_mem(h, target + 8, struct.pack("<d", ox))
        write_mem(h, target + 0x10, struct.pack("<d", oy))
        write_mem(h, target + 0x18, struct.pack("<d", 0.75))
        write_mem(h, obj + 0xA8, struct.pack("<d", 0.0))
        b = read_mem(h, target, 0x60)
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
            print(
                f"HIT t={time.perf_counter()-t0:.2f} mode={mode} "
                f"ost={ost} hasF={has}"
            )
            # release hold; measure rate
            time.sleep(0.05)
            x0 = struct.unpack_from("<d", read_mem(h, target, 0x60), 8)[0]
            time.sleep(0.5)
            b = read_mem(h, target, 0x60)
            x1 = struct.unpack_from("<d", b, 8)[0]
            m1 = struct.unpack_from("<i", b, 0x50)[0]
            print(f"rate={abs(x1 - x0) / 0.5:.3f} mode={m1}")
            hit = True
            break
        time.sleep(0.05)

    # restore lem + pad
    if orig:
        write_mem(h, target + 8, struct.pack("<d", orig[0]))
        write_mem(h, target + 0x10, struct.pack("<d", orig[1]))
        write_mem(h, target + 0x18, struct.pack("<d", orig[2]))
    write_mem(h, obj, saved)
    # clear mode if still 2
    if target:
        write_mem(h, target + 0x50, struct.pack("<i", 1))
    print("restored; hit", hit)
    k32.CloseHandle(h)
    return 0 if hit else 2


if __name__ == "__main__":
    raise SystemExit(main())
