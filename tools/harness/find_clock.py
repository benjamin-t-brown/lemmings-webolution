"""Find the level clock in the running original, and read the tick rate off it.

`LEVELMAP.DAT` gives each level's limit in whole seconds -- MM_CLIMB_002 is
300 -- and the game counts it down on screen. Whatever unit the engine stores
that countdown in *is* its simulation step: if the slot falls by 1 per second
the clock is in seconds and says nothing, but if it falls by N per second then
the engine is stepping N times a second and N is the tick rate, read directly
off the engine rather than inferred from frame timing.

Method: snapshot all writable memory twice over a measured interval and keep
the 4-byte slots that *fell* monotonically at a steady rate, then confirm over
a second, longer interval. The level clock is the slot whose remaining value
also divides sensibly into the known 300-second limit.
"""
from __future__ import annotations

import struct
import sys
import time
from collections import Counter
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
from diffscan import snap  # noqa: E402
from lrstate import attach, ui_state  # noqa: E402
from memdump import k32  # noqa: E402

LIMIT_SECONDS = 300          # MM_CLIMB_002, from LEVELMAP.DAT


def falling(a, b, lo=1, hi=100000):
    out = {}
    for base, da in a.items():
        db = b.get(base)
        if not da or not db or len(da) != len(db) or da == db:
            continue
        for off in range(0, len(da) - 4, 4):
            if da[off:off + 4] == db[off:off + 4]:
                continue
            x, = struct.unpack_from("<i", da, off)
            y, = struct.unpack_from("<i", db, off)
            if lo <= x - y <= hi and y >= 0:
                out[base + off] = (x, y)
    return out


def main() -> int:
    h, _pid = attach()
    if not h:
        print("not running")
        return 1
    if ui_state(h) != 0:
        print("not in a level (ui_state", ui_state(h), ")")
        return 1

    a = snap(h)
    t0 = time.perf_counter()
    time.sleep(3.0)
    b = snap(h)
    t1 = time.perf_counter()
    time.sleep(6.0)
    c = snap(h)
    t2 = time.perf_counter()

    first, second = falling(a, b), falling(b, c)
    d1, d2 = t1 - t0, t2 - t1
    print(f"falling slots: {len(first)} over {d1:.2f}s, {len(second)} over {d2:.2f}s")

    rates = {}
    for addr, (x, y) in first.items():
        if addr not in second:
            continue
        x2, y2 = second[addr]
        if y != x2:            # must be the same continuing countdown
            continue
        r1, r2 = (x - y) / d1, (x2 - y2) / d2
        if abs(r1 - r2) <= 0.08 * max(r1, r2) + 0.4:
            rates[addr] = ((r1 + r2) / 2, y2)

    print(f"{len(rates)} steady countdowns\n")
    hist = Counter(round(r) for r, _ in rates.values())
    for rate, n in sorted(hist.items(), key=lambda kv: -kv[1])[:15]:
        print(f"  {rate:7d} /s   x{n}")
    print()
    for addr, (r, v) in sorted(rates.items()):
        total = r * LIMIT_SECONDS
        print(f"  {addr:#010x}  {r:8.2f}/s  now {v:<10d} "
              f"  limit*rate = {total:.0f}"
              f"{'   <== clock-shaped' if 0 < v <= total * 1.05 else ''}")
    k32.CloseHandle(h)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
