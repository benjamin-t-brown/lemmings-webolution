"""Compare SpeedPad vs teleport: are they stepped? Try VH21_JC."""
from __future__ import annotations

import ctypes
import struct
import sys
import time

sys.path.insert(0, ".")
from diffscan import regions  # noqa: E402
from goto_level import focus  # noqa: E402
from lrstate import attach, u32, ui_state  # noqa: E402
from memdump import k32, read_mem  # noqa: E402


def write_mem(h, addr: int, data: bytes) -> bool:
    n = ctypes.c_size_t(0)
    return bool(
        k32.WriteProcessMemory(
            h, ctypes.c_void_p(addr), data, len(data), ctypes.byref(n)
        )
    )


def find_vt(h, vt: int) -> list[int]:
    needle = struct.pack("<I", vt)
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


def probe(h, ptr: int, label: str) -> None:
    buf = read_mem(h, ptr, 0xC0)
    print(
        f"{label} {ptr:#x} +0x40={buf[0x40]:02x} +0xa0={buf[0xa0]:02x} "
        f"+0xb8={struct.unpack_from('<i', buf, 0xb8)[0]} "
        f"+0xa8={struct.unpack_from('<d', buf, 0xa8)[0]:.3f}"
    )
    write_mem(h, ptr + 0xA8, struct.pack("<d", 3.0))
    time.sleep(0.4)
    tmr = struct.unpack_from("<d", read_mem(h, ptr, 0xB0), 0xA8)[0]
    print(f"  timer 3.0 -> {tmr:.3f}")


def main() -> int:
    h, pid = attach(write=True)
    focus(pid)
    print("ui", ui_state(h))
    pads = find_vt(h, 0x004B1178)
    teles = find_vt(h, 0x004B1250)
    levers = find_vt(h, 0x004B1288)
    print(f"pads {len(pads)} teles {len(teles)} levers {len(levers)}")
    if pads:
        probe(h, pads[0], "pad")
    if teles:
        probe(h, teles[0], "tele")
    if levers:
        probe(h, levers[0], "lever")
    k32.CloseHandle(h)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
