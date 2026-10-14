"""Poll a few addresses as fast as ReadProcessMemory allows, and time the steps.

Used to read the simulation's step period straight off a moving lemming: the
engine's render loop runs at the display rate, but the values it moves a
lemming by change only when the simulation steps, so the plateaus between
changes are the step period and the jump across each plateau is the per-step
displacement.

    python sample_fast.py ADDR [COUNT] [SECONDS]

ADDR is the first of COUNT consecutive f64 slots.
"""
from __future__ import annotations

import statistics
import struct
import sys
import time
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
from lrstate import attach, read_mem  # noqa: E402
from memdump import k32  # noqa: E402


def main() -> int:
    addr = int(sys.argv[1], 0)
    count = int(sys.argv[2]) if len(sys.argv) > 2 else 1
    seconds = float(sys.argv[3]) if len(sys.argv) > 3 else 3.0
    h, pid = attach()
    if not h:
        print("not running")
        return 1
    from goto_level import focus
    from lrinput import LRInput
    focus(pid)
    with LRInput() as inp:
        f0 = inp.mouse_frame
        time.sleep(1.0)
        hz = inp.mouse_frame - f0
    print(f"engine loop {hz:.0f} Hz")

    span = count * 8
    series = []
    t0 = time.perf_counter()
    while time.perf_counter() - t0 < seconds:
        d = read_mem(h, addr, span)
        if d and len(d) == span:
            series.append((time.perf_counter() - t0,
                           struct.unpack(f"<{count}d", d)))
    print(f"{len(series)} samples in {seconds:.1f}s "
          f"({len(series) / seconds:.0f}/s)")

    for i in range(count):
        edges = []
        prev = series[0][1][i]
        for t, vals in series:
            if vals[i] != prev:
                edges.append((t, prev, vals[i]))
                prev = vals[i]
        if len(edges) < 3:
            print(f"\nslot {i} @ {addr + i * 8:#010x}: {len(edges)} changes")
            continue
        gaps = [edges[j + 1][0] - edges[j][0] for j in range(len(edges) - 1)]
        steps = [b - a for _t, a, b in edges]
        print(f"\nslot {i} @ {addr + i * 8:#010x}: {len(edges)} changes")
        print(f"  step period: mean {statistics.mean(gaps) * 1000:7.2f} ms"
              f"  median {statistics.median(gaps) * 1000:7.2f} ms"
              f"  -> {1 / statistics.median(gaps):6.2f} /s")
        print(f"  step size:   mean {statistics.mean(steps):+8.5f}"
              f"  median {statistics.median(steps):+8.5f}"
              f"  min {min(steps):+8.5f} max {max(steps):+8.5f}")
        print("  first gaps: " + " ".join(f"{g * 1000:.0f}" for g in gaps[:14]))
        print("  first steps:" + " ".join(f"{s:+.4f}" for s in steps[:10]))
    k32.CloseHandle(h)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
