"""Bulk-scan a release for a falling billboard.

The entrance sits above the ledge, so a release looked like a free fall
measurement. It is not. On MM_CLIMB_002 the new quad is written at
y = 32.190 and stays there (`catch_fall.py`). The hatch and a walk-off
share one fall speed, and this script does not produce it. Do not treat
a docstring, or a single 48.190 -> 32.190 snap, as that speed.

What it does: one bulk snapshot of the billboard pages across a release,
then which 8-byte slot fell. The earlier per-address scan missed new
slots for two mechanical reasons:

  * it polled single f64 addresses one `ReadProcessMemory` at a time, at 0.12 s
    per sample, so a fall that lasts a couple of seconds yields a handful of
    points and a slot that did not exist before the release is not in the
    candidate list at all;
  * it filtered candidates *before* the release, and the falling lemming's
    billboard is written into memory that held something implausible until the
    moment it spawned.

So this does it the other way round. It takes one bulk snapshot of the pages
the billboards live in -- a few hundred KB, one read each -- as fast as the
loop goes, for several seconds across a release, and only afterwards asks
which 8-byte slot fell. Nothing has to be identified in advance.

A slot that merely sits in the tube's 0..96 band and decreases is not a
height: a walker's x and z sweep that band. Height is the shared y of a
billboard (`measure_motion.centroid`). On the releases this was run
against, that y does not change.

    python measure_fall.py [release] [seconds]
"""
from __future__ import annotations

import statistics
import struct
import sys
import time
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
from find_fallers import heap_regions  # noqa: E402
from live_terrain_check import PIXELS_PER_UNIT  # noqa: E402
from lrstate import attach, i32, read_mem, ui_state  # noqa: E402
from measure_motion import quads  # noqa: E402
from memdump import k32  # noqa: E402

RELEASED = 0x004CBA78
CLOCK = 0x004CBAAC
PAGE = 0x1000
MAX_WINDOW = 1 << 18
PAD = 0x8000


def billboard_window(h) -> tuple[int, int]:
    """The address range the lemming billboards live in, rounded to pages.

    Taken from the billboards that already exist, which is why this runs after
    the first release rather than before it.
    """
    regs = heap_regions(h)
    # Billboard-*shaped* quads are mostly scenery, and the region holding most
    # of them is not the region holding lemmings. Locate real lemmings first
    # -- the ones stepping on the 200 ms simulation lattice -- and window on
    # those, since a newly released lemming's billboard is allocated beside
    # them.
    from check_mask import find_tracks

    tracks, _rate = find_tracks(h, 3.0, verbose=False)
    addrs = [t["addr"] for t in tracks if t["locked"]]
    if not addrs:
        addrs = quads(h, regs)
    if not addrs:
        raise SystemExit("no billboard-shaped quads; is a level running?")
    # Pick the region holding most of them and clip to it: a window spanning
    # two regions has an unreadable gap, and ReadProcessMemory then fails for
    # the whole call rather than partially -- which showed up as zero
    # snapshots and no error.
    best = None
    for rb, rs in regs:
        n = sum(1 for a in addrs if rb <= a < rb + rs)
        if n and (best is None or n > best[0]):
            best = (n, rb, rs)
    _n, rb, rs = best
    inside = [a for a in addrs if rb <= a < rb + rs]
    lo = max(rb, min(inside) - PAD) & ~(PAGE - 1)
    hi = min(rb + rs, (max(inside) + PAD + PAGE - 1) & ~(PAGE - 1))
    if hi - lo > MAX_WINDOW:
        hi = lo + MAX_WINDOW
    while hi > lo and read_mem(h, lo, hi - lo) is None:
        hi -= PAGE
    if hi <= lo:
        raise SystemExit("could not read a window around the billboards")
    return lo, hi - lo


def capture(h, base: int, size: int, seconds: float, trigger: int):
    """Snapshot the window as fast as the loop allows, across one release."""
    from find_fallers import wait_for_release

    print(f"  waiting for release #{trigger}", flush=True)
    if wait_for_release(h, trigger, timeout=120.0) is None:
        raise SystemExit("the release counter never reached that value")
    shots = []
    t0 = time.perf_counter()
    while time.perf_counter() - t0 < seconds:
        t = time.perf_counter() - t0
        d = read_mem(h, base, size)
        if d and len(d) == size:
            shots.append((t, d))
    return shots


def fallers(shots, base: int):
    """Slots whose f64 decreases on every change, inside the height band."""
    n = len(shots[0][1]) // 8
    out = []
    for i in range(n):
        off = i * 8
        series = []
        for t, d in shots:
            v = struct.unpack_from("<d", d, off)[0]
            if not (0.0 < v < 96.0):
                series = None
                break
            series.append((t, v))
        if not series:
            continue
        edges = []
        prev = series[0][1]
        for t, v in series:
            if v != prev:
                edges.append((t, prev, v))
                prev = v
        if len(edges) < 4:
            continue
        drops = [b - a for _t, a, b in edges]
        if not all(d < -0.05 for d in drops):
            continue
        gaps = [edges[i + 1][0] - edges[i][0] for i in range(len(edges) - 1)]
        gaps = [g for g in gaps if g > 0.05]
        out.append(dict(addr=base + off, edges=edges, drops=drops, gaps=gaps,
                        top=series[0][1], bottom=series[-1][1]))
    return out


def main() -> int:
    trigger = int(sys.argv[1]) if len(sys.argv) > 1 else 3
    seconds = float(sys.argv[2]) if len(sys.argv) > 2 else 6.0
    h, pid = attach()
    if not h:
        print("the game is not running")
        return 1
    if ui_state(h) != 0:
        from goto_level import focus
        from lrinput import LRInput
        from restart_level import ensure_playing
        focus(pid)
        with LRInput() as inp:
            inp.enable()
            ensure_playing(h, inp)
            inp.release()
    if ui_state(h) != 0:
        print("not in a level")
        return 1
    print(f"clock {i32(h, CLOCK)} s, released {i32(h, RELEASED)}", flush=True)

    while (i32(h, RELEASED) or 0) < 1:
        time.sleep(0.05)
    base, size = billboard_window(h)
    print(f"billboard window {base:#010x} + {size // 1024} KB", flush=True)

    # Finding the window costs a heap scan, and releases are 2.9 s apart, so
    # the requested release has usually already happened by now. Trigger on
    # the next one instead of capturing a random moment: `wait_for_release`
    # returns immediately once the counter is past its argument.
    now = i32(h, RELEASED) or 0
    if trigger <= now:
        trigger = now + 1
        print(f"  released is already {now}; triggering on #{trigger}")
    shots = capture(h, base, size, seconds, trigger)
    rate = len(shots) / seconds
    print(f"  {len(shots)} snapshots, {rate:.0f} Hz", flush=True)

    found = fallers(shots, base)
    print(f"\n{len(found)} slots fell monotonically\n")
    for f in sorted(found, key=lambda f: -(f["top"] - f["bottom"]))[:12]:
        drops = f["drops"]
        gaps = f["gaps"]
        print(f"  {f['addr']:#010x}  {f['top']:7.3f} -> {f['bottom']:7.3f}"
              f"  total {f['top'] - f['bottom']:6.3f} units"
              f" = {(f['top'] - f['bottom']) * PIXELS_PER_UNIT:6.2f} mask rows")
        span = f["edges"][-1][0] - f["edges"][0][0]
        print(f"    {len(drops)} changes over {span * 1000:.0f} ms"
              f"  (first at t={f['edges'][0][0] * 1000:.0f} ms)"
              + (f", median gap {statistics.median(gaps) * 1000:.1f} ms"
                 f", min {min(gaps) * 1000:.1f}, max {max(gaps) * 1000:.1f}"
                 if gaps else ", no gap above 50 ms"))
        print(f"    -> {len(drops) / span:.1f} changes/s,"
              f" {(f['top'] - f['bottom']) / span:.3f} units/s")
        print("    drop per step (units): "
              + " ".join(f"{-d:.3f}" for d in drops[:16]))
        print("    drop per step (rows):  "
              + " ".join(f"{-d * PIXELS_PER_UNIT:.2f}" for d in drops[:16]))
        md = statistics.median([-d for d in drops])
        print(f"    median {md:.4f} units = {md * PIXELS_PER_UNIT:.3f} mask rows"
              f"; first {-drops[0]:.3f}, last {-drops[-1]:.3f}"
              f" -> {'constant' if abs(drops[0] - drops[-1]) < 0.05 else 'ACCELERATING'}")
        print()
    # Prove the faller is a lemming rather than scenery or a particle: the
    # billboard layout is (x, z, y) per vertex with the same y in all four, so
    # the slot that fell should be vertex 0's y, and after landing the same
    # quad should start stepping one mask column per 200 ms -- which is what a
    # walker does and nothing else in the level does.
    if found:
        from measure_motion import centroid, column, wrap

        best = max(found, key=lambda f: f["top"] - f["bottom"])
        qbase = best["addr"] - 16
        d = read_mem(h, qbase, 96)
        if d and len(d) == 96:
            v = struct.unpack("<12d", d)
            ys = (v[2], v[5], v[8], v[11])
            import math
            rs = [math.hypot(v[k], v[k + 1]) for k in (0, 3, 6, 9)]
            print(f"  quad at {qbase:#010x}: y {ys}, radii "
                  + " ".join(f"{r:.2f}" for r in rs))
        series = []
        t0 = time.perf_counter()
        while time.perf_counter() - t0 < 5.0:
            c = centroid(h, qbase)
            if c:
                series.append((time.perf_counter() - t0, c))
        cols, prev = [], None
        for t, c in series:
            if c != prev:
                cols.append((t, column(c[0], c[1]), c[2]))
                prev = c
        if len(cols) > 3:
            dc = [wrap(cols[i + 1][1] - cols[i][1]) for i in range(len(cols) - 1)]
            gp = [cols[i + 1][0] - cols[i][0] for i in range(len(cols) - 1)]
            gp = [g for g in gp if g > 0.05]
            print("  after landing: "
                  + (f"gap {statistics.median(gp) * 1000:.1f} ms, " if gp else "")
                  + f"median |dcol| {statistics.median([abs(x) for x in dc]):.3f}, "
                  f"y now {cols[-1][2]:.3f}")
    k32.CloseHandle(h)
    return 0 if found else 2


if __name__ == "__main__":
    raise SystemExit(main())
