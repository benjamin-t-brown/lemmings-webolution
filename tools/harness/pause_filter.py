"""Separate simulation state from rendering state by pausing the game.

Everything in this process changes constantly: the engine rewrites a
transformed vertex buffer every one of its 144 rendered frames, the graphics
driver churns its own heaps, and the tube can be spinning. A plain memory
diff over a second therefore returns tens of thousands of slots and says
nothing about the lemmings.

Pausing is the clean filter. Diff twice while paused -- that is rendering and
driver noise, whatever it is. Diff twice while running. What changes only
when the simulation runs is simulation state, and that is a few hundred slots
instead of tens of thousands.

Usage:
    python pause_filter.py [KEY]     # KEY defaults to P
"""
from __future__ import annotations

import struct
import sys
import time
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
from diffscan import snap  # noqa: E402
from goto_level import focus  # noqa: E402
from lrinput import DIK, LRInput  # noqa: E402
from lrstate import attach, ui_state  # noqa: E402
from memdump import k32  # noqa: E402


def changed_set(a, b) -> set[int]:
    out = set()
    for base, da in a.items():
        db = b.get(base)
        if not da or not db or len(da) != len(db) or da == db:
            continue
        for off in range(0, len(da) - 4, 4):
            if da[off:off + 4] != db[off:off + 4]:
                out.add(base + off)
    return out


def busy(h, gap: float = 1.0) -> set[int]:
    a = snap(h)
    time.sleep(gap)
    return changed_set(a, snap(h))


def main() -> int:
    key = sys.argv[1].upper() if len(sys.argv) > 1 else "P"
    h, pid = attach()
    if not h:
        print("not running")
        return 1
    focus(pid)
    if ui_state(h) != 0:
        print("not in a level")
        return 1

    with LRInput() as inp:
        inp.enable()
        running = busy(h) | busy(h)
        print(f"running: {len(running)} slots change")
        inp.tap(DIK[key], hold_frames=20)
        time.sleep(1.0)
        paused = busy(h) | busy(h) | busy(h)
        print(f"after {key}: {len(paused)} slots change")
        inp.tap(DIK[key], hold_frames=20)
        time.sleep(1.0)
        again = busy(h) | busy(h)
        print(f"after {key} again: {len(again)} slots change")
        inp.release()

    sim = (running & again) - paused
    print(f"\nchanges only while the simulation runs: {len(sim)}")
    for addr in sorted(sim)[:60]:
        from memdump import read_mem
        d = read_mem(h, addr, 8)
        if d:
            i, = struct.unpack_from("<i", d, 0)
            f, = struct.unpack_from("<f", d, 0)
            g, = struct.unpack_from("<d", d, 0)
            print(f"  {addr:#010x}  i32={i:<12d} f32={f:<14.4f} f64={g:.4f}")
    k32.CloseHandle(h)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
