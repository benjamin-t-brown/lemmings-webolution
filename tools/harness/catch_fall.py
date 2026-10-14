"""Catch one release and report the billboard's height, not its x or z.

A raw scan of every f64 in 0..96 treats the horizontal coordinates as height
whenever a walker walks through that band, and those move on the same 200 ms
lattice. The billboard is the thing that has one shared height across four
vertices, which is what `measure_motion.quads` already isolates.
"""
from __future__ import annotations

import math
import statistics
import struct
import sys
import time
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
from find_fallers import wait_for_release  # noqa: E402
from lrstate import attach, i32, read_mem, ui_state  # noqa: E402
from measure_fall import CLOCK, RELEASED, billboard_window, capture  # noqa: E402
from measure_motion import column  # noqa: E402
from memdump import k32  # noqa: E402


def billboards(buf: bytes, base: int):
    """Quad addresses inside one snapshot, matching measure_motion.quads."""
    n = len(buf) // 8
    if n < 12:
        return []
    v = struct.unpack_from(f"<{n}d", buf, 0)
    out = []
    for i in range(n - 11):
        y = v[i + 2]
        if not (0.5 < y < 95.0):
            continue
        if v[i + 5] != y or v[i + 8] != y or v[i + 11] != y:
            continue
        if not all(40.0 < math.hypot(v[i + k], v[i + k + 1]) < 70.0
                   for k in (0, 3, 6, 9)):
            continue
        x = (v[i] + v[i + 3] + v[i + 6] + v[i + 9]) / 4.0
        z = (v[i + 1] + v[i + 4] + v[i + 7] + v[i + 10]) / 4.0
        out.append((base + i * 8, x, z, y))
    return out


def tracks(shots, base: int):
    """One series per billboard address. Phase-shifted copies are dropped."""
    series: dict[int, list] = {}
    stride = max(1, len(shots) // 400)
    for t, d in shots[::stride]:
        for addr, x, z, y in billboards(d, base):
            series.setdefault(addr, []).append((t, x, z, y))
    kept = []
    for addr, s in series.items():
        if len(s) < 8:
            continue
        if any(abs(addr - k) < 24 and len(series[k]) >= len(s) and addr != k
               for k in kept):
            continue
        kept.append(addr)
    out = []
    for addr in kept:
        s = series[addr]
        ys = [y for _t, _x, _z, y in s]
        if max(ys) - min(ys) < 0.2:
            continue
        edges = []
        prev = s[0]
        for sample in s[1:]:
            if sample[3] != prev[3]:
                edges.append((sample[0], prev, sample))
                prev = sample
        drops = [a[3] - b[3] for _t, a, b in edges]
        out.append(dict(addr=addr, samples=s, drops=drops,
                        top=max(ys), bottom=min(ys)))
    out.sort(key=lambda f: -(f["top"] - f["bottom"]))
    return out


def main() -> int:
    h, pid = attach()
    if not h:
        print("the game is not running")
        return 1
    from goto_level import focus, hide_console
    from lrinput import LRInput
    from restart_level import ensure_playing

    hide_console()
    focus(pid)
    rel = i32(h, RELEASED) or 0
    clock = i32(h, CLOCK) or 0
    if ui_state(h) != 0 or rel >= 8 or clock < 40:
        print(f"restarting (ui {ui_state(h)} released {rel} clock {clock})", flush=True)
        with LRInput() as inp:
            inp.enable()
            ok = ensure_playing(h, inp)
            inp.release()
        if not ok:
            print("could not enter a level")
            k32.CloseHandle(h)
            return 1
    print(f"clock {i32(h, CLOCK)} released {i32(h, RELEASED)}", flush=True)
    while (i32(h, RELEASED) or 0) < 1:
        time.sleep(0.02)
    # The padded window around the walkers is a slice of a much larger region,
    # and a newly released lemming is allocated elsewhere in that region.
    _probe, _size = billboard_window(h)
    from find_fallers import heap_regions
    base, size = _probe, _size
    for rb, rs in heap_regions(h):
        if rb <= _probe < rb + rs:
            base, size = rb, rs
            break
    print(f"window {base:#010x} + {size // 1024} KB", flush=True)
    now = i32(h, RELEASED) or 0
    trigger = now + 1
    shots = capture(h, base, size, 4.0, trigger)
    print(f"{len(shots)} snapshots", flush=True)
    # A lemming born during the capture has an address the early frames lack.
    early, late = set(), set()
    nshot = len(shots)
    step = max(1, nshot // 40)
    for _t, d in shots[: nshot // 5 : step]:
        early.update(a for a, *_r in billboards(d, base))
    for _t, d in shots[-nshot // 5 :: step]:
        late.update(a for a, *_r in billboards(d, base))
    born = sorted(late - early)
    print(f"{len(born)} billboard addresses appeared during the capture", flush=True)
    for addr in born[:6]:
        ys = []
        for t, d in shots[::step]:
            off = addr - base
            if off < 0 or off + 96 > len(d):
                continue
            v = struct.unpack_from("<12d", d, off)
            y = v[2]
            if ys and ys[-1][1] == y:
                continue
            ys.append((t, y))
        if not ys:
            continue
        print(f"  born {addr:#010x}  " +
              " ".join(f"{y:.3f}" for _t, y in ys[:18]))

    found = tracks(shots, base)
    print(f"{len(found)} billboards changed height")
    for f in found[:8]:
        downs = [d for d in f["drops"] if d > 0.02]
        s0, s1 = f["samples"][0], f["samples"][-1]
        gaps = []
        prev_t = None
        prev_y = None
        for t, _x, _z, y in f["samples"]:
            if prev_y is not None and y < prev_y - 0.02 and prev_t is not None:
                gaps.append(t - prev_t)
            if prev_y is None or abs(y - prev_y) > 0.02:
                prev_t = t
                prev_y = y
        gap = statistics.median(gaps) * 1000 if gaps else float("nan")
        md = statistics.median(downs) if downs else float("nan")
        print(f"  {f['addr']:#010x}  y {f['top']:.4f} -> {f['bottom']:.4f}"
              f"  col {column(s0[1], s0[2]):.1f} -> {column(s1[1], s1[2]):.1f}"
              f"  down-steps {len(downs)} median {md:.4f} gap {gap:.1f} ms")
        print("    " + " ".join(repr(round(d, 6)) for d in downs[:14]))
    if not found:
        seen = {}
        for _t, d in shots[:: max(1, len(shots) // 20)]:
            for addr, _x, _z, y in billboards(d, base):
                seen[addr] = y
        ys = sorted({round(y, 3) for y in seen.values()})
        print(f"  {len(seen)} billboards, heights {ys[:12]}")
    k32.CloseHandle(h)
    return 0 if found else 2


if __name__ == "__main__":
    raise SystemExit(main())
