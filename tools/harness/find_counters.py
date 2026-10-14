"""Find the original's monotonic counters, and read the simulation rate off them.

The question this answers is "how many simulation steps per second does the
original run", which everything else measured in ticks depends on.

It cannot be read off the frame rate: the render and input loop is locked to
the display (144 Hz here, confirmed by the shim's `GetDeviceState` counter and
by how often the engine rewrites its transformed vertex buffer), and a game
that stepped its lemmings 144 times a second would be unplayable. So the
simulation is decoupled, and something in memory must be counting its steps.

Method: snapshot all writable memory, wait, snapshot again, and keep every
4-byte slot that increased. Do it over two different durations. A counter
advancing at a fixed rate shows the same increments-per-second in both,
within the sampling error; everything else -- timers in milliseconds, frame
counters, random noise -- either disagrees or lands on a rate we can name.
The surviving rates, histogrammed, are the candidate tick rates.
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


def rising(a, b, lo=1, hi=100000):
    """Slots whose u32 rose by a sane amount, as {addr: (old, new)}."""
    out = {}
    for base, da in a.items():
        db = b.get(base)
        if not da or not db or len(da) != len(db) or da == db:
            continue
        for off in range(0, len(da) - 4, 4):
            if da[off:off + 4] == db[off:off + 4]:
                continue
            x = struct.unpack_from("<I", da, off)[0]
            y = struct.unpack_from("<I", db, off)[0]
            if lo <= y - x <= hi:
                out[base + off] = (x, y)
    return out


def measure(h, seconds: float):
    a = snap(h)
    t0 = time.perf_counter()
    time.sleep(seconds)
    b = snap(h)
    dt = time.perf_counter() - t0
    return rising(a, b), dt


def main() -> int:
    h, _pid = attach()
    if not h:
        print("not running")
        return 1
    print("ui_state", ui_state(h))
    short, dts = measure(h, 2.0)
    print(f"{len(short)} rising slots over {dts:.2f}s")
    long, dtl = measure(h, 6.0)
    print(f"{len(long)} rising slots over {dtl:.2f}s")

    rates = {}
    for addr, (x, y) in short.items():
        if addr not in long:
            continue
        r1 = (y - x) / dts
        r2 = (long[addr][1] - long[addr][0]) / dtl
        if r1 <= 0 or r2 <= 0:
            continue
        if abs(r1 - r2) <= 0.06 * max(r1, r2) + 0.6:
            rates[addr] = (r1 + r2) / 2

    print(f"\n{len(rates)} slots advancing at a steady rate")
    hist = Counter(round(r) for r in rates.values())
    for rate, n in sorted(hist.items(), key=lambda kv: -kv[1])[:25]:
        example = next(a for a, r in rates.items() if round(r) == rate)
        print(f"  {rate:7d} /s   x{n:<5d}  e.g. {example:#010x} "
              f"= {long[example][1]}")
    k32.CloseHandle(h)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
