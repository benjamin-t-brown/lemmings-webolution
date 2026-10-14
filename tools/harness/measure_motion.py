"""Measure a live lemming's motion: step period, walking speed, fall speed.

What the engine exposes
-----------------------
There is no lemming struct to read -- no symbols, and the state the simulation
keeps is not distinguishable by value from the rest of the heap. What *is*
findable is the billboard the engine builds for each lemming: four vertices,
three f64 each, laid out `(x, z, y)` with the same `y` in all four, all four
at tube radius. It is rebuilt whenever the lemming's appearance changes, so a
camera move rewrites it every rendered frame -- but with the camera still it
changes only when the lemming moves, which is what makes it a usable probe of
the simulation rather than of the renderer.

What is measured
----------------
Taking the centroid of the four vertices removes the billboard's own
orientation, leaving the lemming's position:

  * **step period** -- how long the centroid holds still between changes.
  * **walk speed** -- the change in angle around the tube per step, converted
    into columns of our 256-wide terrain mask by the same unroll
    `SurfaceMesh` uses: `px = atan2(-x, z) / 2pi * 256`.
  * **fall speed** -- the change in height per step, in world units and in
    mask rows (`PIXELS_PER_UNIT = 2`).

Run it on a running level; `fresh_level.py` puts one up.
"""
from __future__ import annotations

import math
import statistics
import struct
import sys
import time
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
from find_fallers import heap_regions  # noqa: E402
from live_terrain_check import MASK_W, PIXELS_PER_UNIT  # noqa: E402
from lrstate import attach, i32, read_mem, ui_state  # noqa: E402
from memdump import k32  # noqa: E402

RELEASED = 0x004CBA78
CLOCK = 0x004CBAAC
QUAD = 12 * 8          # four vertices, three f64 each


def quads(h, regs):
    """Addresses that look like a lemming billboard: 4 tube points, one height."""
    out = []
    for base, size in regs:
        d = read_mem(h, base, size)
        if not d:
            continue
        n = len(d) // 8
        if n < 12:
            continue
        v = struct.unpack_from(f"<{n}d", d, 0)
        for i in range(n - 11):
            y = v[i + 2]
            if not (0.5 < y < 95.0):
                continue
            if v[i + 5] != y or v[i + 8] != y or v[i + 11] != y:
                continue
            rs = [math.hypot(v[i + k], v[i + k + 1]) for k in (0, 3, 6, 9)]
            if all(40.0 < r < 70.0 for r in rs):
                out.append(base + i * 8)
    return out


def centroid(h, addr):
    d = read_mem(h, addr, QUAD)
    if not d or len(d) != QUAD:
        return None
    v = struct.unpack("<12d", d)
    x = (v[0] + v[3] + v[6] + v[9]) / 4.0
    z = (v[1] + v[4] + v[7] + v[10]) / 4.0
    return x, z, v[2]


def column(x: float, z: float) -> float:
    a = math.atan2(-x, z) / (2.0 * math.pi)
    if a < 0:
        a += 1.0
    return a * MASK_W


def wrap(d: float) -> float:
    while d > MASK_W / 2:
        d -= MASK_W
    while d < -MASK_W / 2:
        d += MASK_W
    return d


def main() -> int:
    seconds = float(sys.argv[1]) if len(sys.argv) > 1 else 8.0
    h, _pid = attach()
    if not h:
        print("not running")
        return 1
    if ui_state(h) != 0:
        print("not in a level; run fresh_level.py first")
        return 1
    regs = heap_regions(h)
    print(f"clock {i32(h, CLOCK)}s, released {i32(h, RELEASED)}", flush=True)
    want = int(sys.argv[2]) if len(sys.argv) > 2 else 1
    from find_fallers import wait_for_release
    if (i32(h, RELEASED) or 0) < want:
        print(f"waiting for release #{want}", flush=True)
        wait_for_release(h, want, timeout=120.0)
    cands = quads(h, regs)
    print(f"{len(cands)} billboard-shaped quads", flush=True)

    # Keep the ones that actually move: scenery quads exist too.
    live = []
    for a in cands:
        c0 = centroid(h, a)
        time.sleep(0.0)
        if c0:
            live.append((a, c0))
    time.sleep(0.5)
    movers = [a for a, c0 in live
              if (c := centroid(h, a)) and c != c0]
    print(f"{len(movers)} of them moved in half a second\n", flush=True)

    results = []
    for a in movers[:40]:
        series = []
        t0 = time.perf_counter()
        while time.perf_counter() - t0 < seconds / 4:
            c = centroid(h, a)
            if c:
                series.append((time.perf_counter() - t0, c))
        edges, prev = [], series[0][1]
        for t, c in series:
            if c != prev:
                edges.append((t, prev, c))
                prev = c
        if len(edges) < 4:
            continue
        gaps = [edges[i + 1][0] - edges[i][0] for i in range(len(edges) - 1)]
        dcol = [wrap(column(*b[:2]) - column(*p[:2])) for _t, p, b in edges]
        dy = [b[2] - p[2] for _t, p, b in edges]
        results.append((max(abs(d) for d in dy), a, gaps, dcol, dy))

    results.sort(reverse=True)
    for _rank, a, gaps, dcol, dy in results[:8]:
        print(f"  {a:#010x}  {len(dcol)} steps")
        print(f"    step period  median {statistics.median(gaps) * 1000:7.2f} ms"
              f"  mean {statistics.mean(gaps) * 1000:7.2f} ms"
              f"  -> {1 / statistics.median(gaps):5.2f} steps/s")
        print(f"    mask columns per step   median {statistics.median(dcol):+7.4f}"
              f"  (|.| max {max(abs(d) for d in dcol):.4f})")
        print(f"    height per step  world median {statistics.median(dy):+7.4f}"
              f"  min {min(dy):+.4f} max {max(dy):+.4f}"
              f"  mask rows min {min(dy) * PIXELS_PER_UNIT:+.4f}")
        print("    gaps ms: " + " ".join(f"{g * 1000:.0f}" for g in gaps[:14]))
        print("    d-col:   " + " ".join(f"{d:+.3f}" for d in dcol[:14]))
        print("    d-y:     " + " ".join(f"{d:+.3f}" for d in dy[:14]))
        print()
    k32.CloseHandle(h)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
