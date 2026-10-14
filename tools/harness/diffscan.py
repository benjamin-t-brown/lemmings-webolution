"""Find the variable an input changes, by diffing all writable memory.

Used to locate the frontend's menu-highlight and screen variables, which do
not live in the game object and so cannot be read the way `ui_state` is.
The method is the classic one: snapshot, act, snapshot, intersect across
repeats so that per-frame noise (timers, counters, cursor) falls out.
"""
from __future__ import annotations

import ctypes
import struct
import sys
import time
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
from goto_level import focus  # noqa: E402
from lrinput import DIK, LRInput  # noqa: E402
from lrstate import attach  # noqa: E402
from memdump import (  # noqa: E402
    MEM_COMMIT,
    MEMORY_BASIC_INFORMATION,
    PAGE_GUARD,
    PAGE_NOACCESS,
    k32,
    read_mem,
)

LIMIT = 0x30000000


def regions(h):
    mbi = MEMORY_BASIC_INFORMATION()
    addr = 0x00400000
    out = []
    while addr < LIMIT:
        if not k32.VirtualQueryEx(h, ctypes.c_void_p(addr), ctypes.byref(mbi),
                                  ctypes.sizeof(mbi)):
            break
        base, size = mbi.BaseAddress or 0, mbi.RegionSize or 0
        if size == 0:
            break
        writable = mbi.Protect in (0x04, 0x08, 0x40, 0x80)
        if mbi.State == MEM_COMMIT and writable and not (mbi.Protect & PAGE_GUARD) \
                and mbi.Protect != PAGE_NOACCESS and size <= 32 * 1024 * 1024:
            out.append((base, size))
        addr = base + size
        if addr <= base:
            break
    return out


def snap(h):
    return {b: read_mem(h, b, s) for b, s in regions(h)}


def changed(a, b, lo=0, hi=64):
    """Addresses whose little-endian i32 changed, both ends in [lo, hi]."""
    out = {}
    for base, da in a.items():
        db = b.get(base)
        if not da or not db or len(da) != len(db):
            continue
        if da == db:
            continue
        for off in range(0, len(da) - 4, 4):
            if da[off:off + 4] == db[off:off + 4]:
                continue
            x = struct.unpack_from("<i", da, off)[0]
            y = struct.unpack_from("<i", db, off)[0]
            if lo <= x <= hi and lo <= y <= hi:
                out[base + off] = (x, y)
    return out


def main() -> int:
    key = sys.argv[1] if len(sys.argv) > 1 else "DOWN"
    rounds = int(sys.argv[2]) if len(sys.argv) > 2 else 3
    h, pid = attach()
    if not h:
        print("not running")
        return 1
    focus(pid)
    keep = None
    with LRInput() as inp:
        inp.enable()
        for r in range(rounds):
            a = snap(h)
            inp.tap(DIK[key], hold_frames=20)
            time.sleep(1.0)
            b = snap(h)
            c = changed(a, b)
            print(f"round {r}: {len(c)} candidates")
            keep = c if keep is None else {k: v for k, v in c.items() if k in keep}
        inp.release()
    print(f"\nstable across {rounds} presses of {key}: {len(keep)}")
    for addr, (x, y) in sorted(keep.items())[:80]:
        print(f"  {addr:#010x}  {x} -> {y}")
    k32.CloseHandle(h)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
