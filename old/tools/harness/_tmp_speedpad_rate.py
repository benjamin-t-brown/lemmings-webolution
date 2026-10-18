"""MEASURED: +0x50==2 doubles walk rate on a grounded walker."""
from __future__ import annotations

import ctypes
import struct
import sys
import time

sys.path.insert(0, ".")
from goto_level import focus  # noqa: E402
from lrstate import attach, u32, ui_state  # noqa: E402
from measure_spawn import LEMMING_VT, OTHER_VT, actor_ptrs  # noqa: E402
from memdump import k32, read_mem  # noqa: E402


def write_mem(h, addr: int, data: bytes) -> bool:
    n = ctypes.c_size_t(0)
    return bool(
        k32.WriteProcessMemory(
            h, ctypes.c_void_p(addr), data, len(data), ctypes.byref(n)
        )
    )


def sample_rate(h, p: int, seconds: float = 0.6) -> tuple[float, int, float]:
    b = read_mem(h, p, 0x60)
    x0 = struct.unpack_from("<d", b, 8)[0]
    y0 = struct.unpack_from("<d", b, 0x10)[0]
    m0 = struct.unpack_from("<i", b, 0x50)[0]
    time.sleep(seconds)
    b = read_mem(h, p, 0x60)
    x1 = struct.unpack_from("<d", b, 8)[0]
    y1 = struct.unpack_from("<d", b, 0x10)[0]
    m1 = struct.unpack_from("<i", b, 0x50)[0]
    return abs(x1 - x0) / seconds, m1, y1


def main() -> int:
    h, pid = attach(write=True)
    print("ui", ui_state(h), "focus", focus(pid))
    target = None
    for p in actor_ptrs(h):
        if u32(h, p) not in (LEMMING_VT, OTHER_VT):
            continue
        b = read_mem(h, p, 0x60)
        x, y = struct.unpack_from("<dd", b, 8)
        mode = struct.unpack_from("<i", b, 0x50)[0]
        face = struct.unpack_from("<i", b, 0x40)[0]
        if abs(y - 29) < 0.2 and mode == 1 and 90 < x < 140:
            target = p
            print(f"pick {p:#x} x={x:.3f} y={y:.3f} face={face:+d}")
            break
    if not target:
        print("no grounded walker")
        k32.CloseHandle(h)
        return 1

    r1, m1, y1 = sample_rate(h, target, 0.6)
    print(f"baseline mode={m1} y={y1:.3f} |dx|/s={r1:.3f}")

    write_mem(h, target + 0x50, struct.pack("<i", 2))
    r2, m2, y2 = sample_rate(h, target, 0.6)
    print(f"mode2     mode={m2} y={y2:.3f} |dx|/s={r2:.3f}  ratio={r2 / r1 if r1 else 0:.3f}")

    write_mem(h, target + 0x50, struct.pack("<i", 3))
    r3, m3, y3 = sample_rate(h, target, 0.6)
    print(f"mode3     mode={m3} y={y3:.3f} |dx|/s={r3:.3f}  ratio={r3 / r1 if r1 else 0:.3f}")

    write_mem(h, target + 0x50, struct.pack("<i", 1))
    r4, m4, y4 = sample_rate(h, target, 0.6)
    print(f"restored  mode={m4} y={y4:.3f} |dx|/s={r4:.3f}")

    k32.CloseHandle(h)
    ok = m2 == 2 and r2 > r1 * 1.5 and r3 < r1 * 0.75
    print("PASS" if ok else "CHECK")
    return 0 if ok else 2


if __name__ == "__main__":
    raise SystemExit(main())
