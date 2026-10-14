"""Check our rasterised terrain mask against the original's own lemmings.

This supersedes `live_terrain_check.py`, whose result PLAN.md withdrew. That
module called anything on the tube that "moves smoothly at constant height" a
walker, which on a turning tube also describes transformed surface vertices.
`find_lemmings.py` answered that with the spin: every surface vertex shares one
angular velocity, so a lemming is an outlier against the median.

Running it showed the spin argument is sound but incomplete in two ways, both
of which this module fixes.

  * **The tube is not always turning.** On MM_CLIMB_002 the median angular
    velocity over all ~700 tube points is 0.00000 rad/s: the camera sits
    still. So the median-spin outlier test degenerates to "moves at all",
    which is nearly the filter that was withdrawn.
  * **A bare f64 triple is phase-ambiguous.** The engine's vertex arrays are
    three f64 per vertex, and a window taken one f64 off the record boundary
    still passes a "lies on the tube" test while reporting coordinates that
    belong to two different vertices. Several of `find_lemmings`' outliers are
    exactly that.

The decisive discriminator is the *simulation clock*. The original steps its
lemmings on a flat 200 ms period (PLAN.md 2.2) while it rewrites its vertex
buffers at the display rate, 144 Hz here. So:

    a lemming is a billboard whose position changes on a 200 ms lattice
    and by exactly one of the surface's 256 columns per change.

Nothing in the scenery does that -- scenery is either static or rewritten
every frame, and a camera orbit moves it by a frame's worth of angle, not by
exactly 1/256 of a turn every fifth of a second. The quad shape (four
vertices, one shared height, all at tube radius) fixes the phase, so the
coordinates are read off record boundaries.

What is then tested is sharper than "is this cell solid". A walker sweeps
columns, so one track visits many cells; and where it turns round is where
the engine says there is a wall. So the report gives

  * the fraction of visited cells whose column has a floor in our raster,
    against the fraction of *all* columns that do, which is the rate a
    randomly placed point would score -- without that baseline the headline
    number means nothing;
  * whether any lemming was ever seen in a column our raster calls empty
    from top to bottom, which would falsify the raster outright;
  * the rows the lemmings occupy against the raster's floor rows;
  * the columns where a walker reversed, against the raster's walls.

    python check_mask.py [seconds] [--no-restart]
"""
from __future__ import annotations

import json
import math
import statistics
import struct
import sys
import time
from collections import defaultdict
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
from find_fallers import heap_regions  # noqa: E402
from live_terrain_check import MASK_H, MASK_W, PIXELS_PER_UNIT, REF, WORLD_H  # noqa: E402
from lrstate import attach, i32, read_mem, ui_state  # noqa: E402
from measure_motion import centroid, column, quads, wrap  # noqa: E402
from memdump import k32  # noqa: E402

RELEASED = 0x004CBA78
CLOCK = 0x004CBAAC
STEP_MS = 200.0
STEP_TOL = 0.03          # seconds; 200 ms +- 30
COL_TOL = 0.08           # columns; one column per step, +- this


def load_mask() -> bytes:
    if not REF.exists():
        raise SystemExit(f"missing {REF}; write it with lrtool lvlmask first")
    return REF.read_bytes()


def solid(mask: bytes, px: int, py: int) -> int:
    px %= MASK_W
    py = max(0, min(MASK_H - 1, py))
    return mask[py * MASK_W + px]


def floor_rows(mask: bytes) -> dict[int, int]:
    """Topmost solid row of the lowest solid run in each column, or -1."""
    out = {}
    for x in range(MASK_W):
        rows = [y for y in range(MASK_H) if mask[y * MASK_W + x]]
        if not rows:
            out[x] = -1
            continue
        y = rows[-1]
        while y - 1 in rows:
            y -= 1
        out[x] = y
    return out


def find_tracks(h, seconds: float, verbose: bool = True):
    """Billboards whose position changes on the simulation's 200 ms lattice."""
    regs = heap_regions(h)
    cands = quads(h, regs)
    if verbose:
        print(f"{len(cands)} billboard-shaped quads", flush=True)
    first = {a: centroid(h, a) for a in cands}
    time.sleep(0.5)
    movers = []
    for a in cands:
        c = centroid(h, a)
        if first.get(a) and c and c != first[a]:
            movers.append(a)
    if verbose:
        print(f"{len(movers)} of them moved in half a second", flush=True)
    if not movers:
        return [], 0.0

    series: dict[int, list] = {a: [] for a in movers}
    t0 = time.perf_counter()
    while time.perf_counter() - t0 < seconds:
        t = time.perf_counter() - t0
        for a in movers:
            c = centroid(h, a)
            if c:
                series[a].append((t, c))
    rate = len(series[movers[0]]) / seconds if movers else 0.0
    if verbose:
        print(f"polled each of them at {rate:.0f} Hz for {seconds:.0f} s",
              flush=True)

    tracks = []
    for a, s in series.items():
        if len(s) < 10:
            continue
        edges, prev = [], s[0][1]
        for t, c in s:
            if c != prev:
                edges.append((t, prev, c))
                prev = c
        if len(edges) < 6:
            continue
        gaps = [edges[i + 1][0] - edges[i][0] for i in range(len(edges) - 1)]
        gaps = [g for g in gaps if g > 0.05]        # a step caught mid-write
        if len(gaps) < 4:
            continue
        gap = statistics.median(gaps)
        dcol = [wrap(column(b[0], b[1]) - column(p[0], p[1]))
                for _t, p, b in edges]
        dy = [b[2] - p[2] for _t, p, b in edges]
        unit = statistics.median([abs(d) for d in dcol])
        locked = (abs(gap - STEP_MS / 1000.0) < STEP_TOL
                  and abs(unit - 1.0) < COL_TOL)
        tracks.append(dict(addr=a, gap=gap, edges=edges, dcol=dcol, dy=dy,
                           locked=locked, unit=unit, pos=s[-1][1]))
    return tracks, rate


def spin(h, seconds: float = 1.5) -> float:
    """The tube's angular velocity, for the record (0 when the camera is still)."""
    from live_terrain_check import candidates

    addrs = candidates(h)
    a0 = {}
    for a in addrs:
        d = read_mem(h, a, 24)
        if d and len(d) == 24:
            x, _y, z = struct.unpack("<3d", d)
            a0[a] = math.atan2(-x, z)
    time.sleep(seconds)
    das = []
    for a, prev in a0.items():
        d = read_mem(h, a, 24)
        if not d or len(d) != 24:
            continue
        x, _y, z = struct.unpack("<3d", d)
        das.append((((math.atan2(-x, z) - prev + math.pi) % (2 * math.pi))
                    - math.pi) / seconds)
    return statistics.median(das) if das else float("nan")


def dedupe(tracks):
    """One entry per distinct lemming.

    The quad finder scans every f64 offset, so a single billboard is found
    four times over -- once per vertex boundary -- and the four windows report
    centroids a third of a column apart. Clustering on the column (and the
    height, since two lemmings can share a column) collapses them.
    """
    kept: list = []
    for t in sorted(tracks, key=lambda t: -len(t["edges"])):
        x, z, y = t["pos"]
        c = column(x, z)
        if any(abs(wrap(c - column(k["pos"][0], k["pos"][1]))) < 2.0
               and abs(y - k["pos"][2]) < 1.0 for k in kept):
            continue
        kept.append(t)
    return kept


def main() -> int:
    seconds = float(sys.argv[1]) if len(sys.argv) > 1 else 30.0
    h, pid = attach()
    if not h:
        print("the game is not running")
        return 1
    mask = load_mask()

    # A background window polls at ~15 Hz, and the 200 ms lattice test then
    # cannot tell a step from a late sample. Hide our console first: a bare
    # SetForegroundWindow is handed straight back to it.
    from goto_level import focus, hide_console, poll_rate
    from lrinput import LRInput

    hide_console()
    focus(pid)
    with LRInput() as inp:
        rate = poll_rate(inp)
        print(f"poll {rate:.0f} Hz", flush=True)
        if rate < 60:
            print("not foreground; refusing to sample")
            k32.CloseHandle(h)
            return 1
        if ui_state(h) != 0 and "--no-restart" not in sys.argv:
            from restart_level import ensure_playing
            inp.enable()
            ensure_playing(h, inp)
            inp.release()
    if ui_state(h) != 0:
        print("not in a level")
        k32.CloseHandle(h)
        return 1
    focus(pid)
    with LRInput() as inp:
        rate = poll_rate(inp)
    print(f"poll during play {rate:.0f} Hz", flush=True)
    if rate < 60:
        print("lost the foreground after entry; refusing to sample")
        k32.CloseHandle(h)
        return 1

    print(f"tube spin: {spin(h):+.5f} rad/s (median over all tube points)")
    print(f"clock {i32(h, CLOCK)} s, released {i32(h, RELEASED)}", flush=True)

    with LRInput() as inp:
        frame0 = inp.mouse_frame
        t_sample = time.perf_counter()
        tracks, _rate = find_tracks(h, seconds)
        sample_hz = (inp.mouse_frame - frame0) / (time.perf_counter() - t_sample)
    print(f"poll across the sample: {sample_hz:.0f} Hz", flush=True)
    locked = dedupe([t for t in tracks if t["locked"]])
    other = [t for t in tracks if not t["locked"]]
    print(f"\n{len(locked)} step-locked tracks (200 ms, one column per step)"
          f"; {len(other)} movers rejected\n")

    fl = floor_rows(mask)
    empty_cols = {x for x, y in fl.items() if y < 0}
    cols_with_floor = MASK_W - len(empty_cols)

    visits: list[tuple[int, int]] = []
    rev_cols: list[int] = []
    rows_seen: defaultdict[int, int] = defaultdict(int)
    for t in sorted(locked, key=lambda t: column(t["pos"][0], t["pos"][1])):
        _x, _z, y = t["pos"]
        cols = []
        for _tt, _p, b in t["edges"]:
            c = column(b[0], b[1])
            cols.append(c)
            px, py = int(c) % MASK_W, int((WORLD_H - b[2]) * PIXELS_PER_UNIT)
            visits.append((px, py))
            rows_seen[py] += 1
        for i in range(1, len(t["dcol"])):
            if t["dcol"][i] * t["dcol"][i - 1] < 0:
                rev_cols.append(int(cols[i]) % MASK_W)
        print(f"  {t['addr']:#010x} gap={t['gap'] * 1000:6.1f} ms "
              f"steps={len(t['edges']):3d} |dcol|={t['unit']:.3f} "
              f"y={y:7.3f} col {min(cols):6.1f}..{max(cols):6.1f} "
              f"dy max {max(abs(d) for d in t['dy']):.3f}")

    if not visits:
        print("no step-locked tracks; nothing to compare")
        k32.CloseHandle(h)
        return 2

    on_floor_col = sum(1 for px, _py in visits if fl[px] >= 0)
    here = sum(1 for px, py in visits if solid(mask, px, py))
    below = sum(1 for px, py in visits
                if sum(solid(mask, px, py + d) for d in (1, 2, 3)) >= 2)
    in_empty = sum(1 for px, _py in visits if px in empty_cols)

    print(f"\nvisited cells: {len(visits)} over "
          f"{len({v[0] for v in visits})} distinct columns")
    print(f"  column has a floor in our raster: {on_floor_col}/{len(visits)}"
          f"  (baseline {cols_with_floor}/{MASK_W} = "
          f"{100.0 * cols_with_floor / MASK_W:.1f}% of columns)")
    print(f"  cell itself solid:                {here}/{len(visits)}")
    print(f"  solid continues below:            {below}/{len(visits)}")
    print(f"  in a column our raster calls empty top to bottom: {in_empty}")
    print("  rows occupied: "
          + " ".join(f"{r}x{n}" for r, n in sorted(rows_seen.items())))
    print("  raster floor-top rows at those columns: "
          + " ".join(str(v) for v in sorted({fl[px] for px, _ in visits})))
    if rev_cols:
        print(f"  reversal columns: {sorted(set(rev_cols))}")

    # Is the disagreement a *rotation*? The billboards are read straight out
    # of the engine's own buffers, so our unroll's zero angle need not be the
    # engine's. Sweeping the offset separates "our raster is wrong" from "our
    # raster is right but turned", which are very different findings.
    best = []
    for k in range(MASK_W):
        hits = sum(1 for px, py in visits if solid(mask, px + k, py))
        best.append((hits, k))
    best.sort(reverse=True)
    print("\n  offset sweep (cell solid at column + k):")
    for hits, k in best[:5]:
        print(f"    k={k:3d}  {hits}/{len(visits)}")
    print(f"    k=  0  {sum(1 for px, py in visits if solid(mask, px, py))}"
          f"/{len(visits)}  (our convention)")
    # A wide tie is a platform the track can slide along, not a measured
    # origin. Printing only the five best k hides that.
    top_hits = best[0][0]
    tied = sorted(k for hits, k in best if hits == top_hits)
    bands: list[tuple[int, int]] = []
    start = prev = tied[0]
    for k in tied[1:]:
        if k == prev + 1:
            prev = k
            continue
        bands.append((start, prev))
        start = prev = k
    bands.append((start, prev))
    print(f"    {len(tied)} offsets tie at {top_hits}/{len(visits)}: "
          + ", ".join(f"{a}..{b}" for a, b in bands))

    out = REF.parent / "live_tracks.json"
    out.write_text(json.dumps(dict(
        level="MM_CLIMB_002", seconds=seconds,
        tracks=[dict(addr=t["addr"], gap=t["gap"],
                     cols=[column(b[0], b[1]) for _t, _p, b in t["edges"]],
                     ys=[b[2] for _t, _p, b in t["edges"]]) for t in locked],
    ), indent=1))
    print(f"\n  tracks written to {out}")
    k32.CloseHandle(h)
    return 0 if in_empty == 0 and on_floor_col == len(visits) else 2


if __name__ == "__main__":
    raise SystemExit(main())
