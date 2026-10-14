"""Measure the original's simulation step rate off a live lemming.

End to end, with no hand-held steps: enter the level if it is not running,
wait for the first lemming to be released, find the memory the engine moves
for it, then poll that memory as fast as `ReadProcessMemory` allows and
report how long it stays still between changes.

Why this measures the simulation and not the renderer: the slots come from
`find_lemming_record`, which keeps only memory that did not exist before the
release and keeps moving after it, so it is per-lemming state. The engine
rewrites it whenever anything about the lemming's appearance changes, and at
144 Hz it demonstrably can -- a camera move rewrites it every 7 ms. When the
camera is still it changes on a flat 200 ms period regardless of whether the
game is running at 144, 30 or 27 frames a second, and the displacement across
each of those steps is constant. That period is the simulation step.

    python measure_tick.py [SECONDS]
"""
from __future__ import annotations

import statistics
import struct
import sys
import time
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
from find_fallers import heap_regions, shot, wait_for_release  # noqa: E402
from goto_level import focus, hide_console, show_console  # noqa: E402
from lrinput import LRInput  # noqa: E402
from lrstate import attach, i32, read_mem, ui_state  # noqa: E402
from memdump import k32  # noqa: E402
from restart_level import ensure_playing  # noqa: E402

RELEASED = 0x004CBA78
CLOCK = 0x004CBAAC
LO, HI = 0.2, 95.0


def born_slots(h, regs, n=1):
    """f64 slots that became tube-shaped exactly when lemming n was released."""
    before = shot(h, regs)
    wait_for_release(h, n)
    after = shot(h, regs)
    out = []
    for base, da in before.items():
        db = after.get(base)
        if not da or not db or len(da) != len(db):
            continue
        m = min(len(da), len(db)) // 8
        va = struct.unpack_from(f"<{m}d", da, 0)
        vb = struct.unpack_from(f"<{m}d", db, 0)
        for i in range(m):
            if va[i] != vb[i] and LO < vb[i] < HI and not LO < va[i] < HI:
                out.append(base + i * 8)
    return out


def still_moving(h, addrs, rounds=6, interval=0.1):
    tracks = {a: [] for a in addrs}
    for _ in range(rounds):
        for a in addrs:
            d = read_mem(h, a, 8)
            tracks[a].append(struct.unpack("<d", d)[0] if d else None)
        time.sleep(interval)
    out = []
    for a, vs in tracks.items():
        if any(v is None for v in vs) or not all(LO < v < HI for v in vs):
            continue
        if len(set(vs)) > 2:
            out.append(a)
    return out


def step_period(h, addr, seconds):
    series = []
    t0 = time.perf_counter()
    while time.perf_counter() - t0 < seconds:
        d = read_mem(h, addr, 8)
        if d and len(d) == 8:
            series.append((time.perf_counter() - t0, struct.unpack("<d", d)[0]))
    edges, prev = [], series[0][1]
    for t, v in series:
        if v != prev:
            edges.append((t, prev, v))
            prev = v
    gaps = [edges[i + 1][0] - edges[i][0] for i in range(len(edges) - 1)]
    steps = [b - a for _t, a, b in edges]
    return len(series), edges, gaps, steps


def main() -> int:
    seconds = float(sys.argv[1]) if len(sys.argv) > 1 else 6.0
    h, pid = attach()
    if not h:
        print("not running")
        return 1
    focus(pid)
    with LRInput() as inp:
        inp.enable()
        if not ensure_playing(h, inp, verbose=True):
            print("could not enter a level")
            return 1
        inp.release()
    focus(pid)

    regs = heap_regions(h)
    print(f"clock {i32(h, CLOCK)}s, released {i32(h, RELEASED)}", flush=True)
    if (i32(h, RELEASED) or 0) != 0:
        print("note: not at the start of the level; "
              "the born-slot filter needs release #1", flush=True)
    addrs = born_slots(h, regs, 1)
    print(f"{len(addrs)} slots born at the release", flush=True)
    movers = still_moving(h, addrs)
    print(f"{len(movers)} still moving", flush=True)
    if not movers:
        return 2

    hide_console()
    focus(pid)
    with LRInput() as inp:
        f0 = inp.mouse_frame
        time.sleep(1.0)
        hz = inp.mouse_frame - f0
    print(f"engine loop {hz:.0f} Hz "
          f"({'foreground' if hz > 60 else 'NOT foreground'})", flush=True)

    best = None
    for a in movers[:12]:
        n, edges, gaps, steps = step_period(h, a, seconds)
        if len(gaps) < 4:
            continue
        med = statistics.median(gaps)
        spread = statistics.pstdev(gaps)
        print(f"\n  {a:#010x}  {len(edges)} changes, {n} samples")
        print(f"    period median {med * 1000:7.2f} ms  "
              f"mean {statistics.mean(gaps) * 1000:7.2f} ms  "
              f"sd {spread * 1000:6.2f} ms  -> {1 / med:5.2f} steps/s")
        print(f"    displacement median {statistics.median(steps):+8.5f} "
              f"world units per step")
        print("    gaps: " + " ".join(f"{g * 1000:.0f}" for g in gaps[:16]))
        if best is None or spread < best[0]:
            best = (spread, a, med, statistics.median(steps))
    show_console()
    if best:
        _sd, a, med, disp = best
        print(f"\ncleanest track {a:#010x}: {1 / med:.2f} simulation steps/s "
              f"({med * 1000:.1f} ms), {disp:+.5f} world units per step")
    k32.CloseHandle(h)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
