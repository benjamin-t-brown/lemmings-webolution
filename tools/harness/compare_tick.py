"""Compare one live lemming to webo::sim::World, one 200 ms tick at a time.

SUPERSEDED, and its numbers must not be quoted. Round 10 showed that the quad
this script locks is NOT a lemming: on MM_CLIMB_002 the actor list
(`0x004CBB18`) held eight lemmings at X 28.264 to 63.033, all at Y exactly
32.000, while this script locked a quad at column 168.042 and Y 32.190 whose
column drifted at 3.8 world units a second -- not the 5.0 a walker moves --
and no lemming's X shadowed it with a constant offset over a second of
simultaneous sampling. So round 1's "walk matched 8 ticks" and its "diverge at
tick 8" were both artefacts of that quad.

Use `measure_steppers.py --compare walker|climber`, which drives the same
printer off the actor list and matched 24 walk ticks and 16 climb ticks.
This file is kept unchanged as round 1's regression case.
docs/scratch/phase3-round10-steppers.md, part 4.

The live side is the billboard probe from measure_motion.py: four vertices,
one shared Y, centroid, column = atan2(-x, z) / 2π * 256. Height is that
world Y. One cell is one world unit. The integer part of the column and of
Y is compared to the sim cell. The fractional part of Y is printed and is
not fed back into the sim.

The sim side is compare_tick.exe: Terrain::fromCollisionGrid for the named
level, one lemming inserted with World::place at the starting integer cell,
then the same number of World::tick calls. World::spawn is not the start,
and Rules are untouched.

Stops at the first cell disagreement. A match for N ticks and a divergence
at tick T are both finished results.

Usage:
    python tools/harness/compare_tick.py
    python tools/harness/compare_tick.py --ticks 8
"""
from __future__ import annotations

import argparse
import math
import struct
import subprocess
import sys
import time
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
from find_fallers import heap_regions  # noqa: E402
from goto_level import PATH_BUFFER, loaded_level  # noqa: E402
from lrstate import attach, read_mem, ui_state  # noqa: E402
from memdump import find_pids, k32  # noqa: E402

COLUMNS = 256
QUAD = 12 * 8
GAME_DIR = Path(r"C:\progs\Lemmings Revolution")
LEVEL = "MM_CLIMB_002"
PRINTER = Path(__file__).resolve().parents[2] / "build" / "cmake" / "dev-debug" / "compare_tick.exe"

# Already measured on this level (PLAN.md, Phase 3 handoff). Not remeasured
# here in order to replace them: a walker steps one column per 200 ms and
# holds Y; a climber on the column-128 wall holds the column and steps Y by 1.
FLOOR_Y = 32.190
PEN = (128.0, 168.0)
TICK_S = 0.200


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
    return a * COLUMNS


def wrap_delta(d: float) -> float:
    while d > COLUMNS / 2:
        d -= COLUMNS
    while d < -COLUMNS / 2:
        d += COLUMNS
    return d


def sample(h, addr):
    c = centroid(h, addr)
    if not c:
        return None
    return column(*c[:2]), c[2]


# The collision grid in the file grows Y upward (engine world Y). Our Terrain
# flips that on load so y increases downward, matching worldToMaskPixel and
# the steppers. Live billboard Y must take the same map or the walker is
# placed in open air above the flipped floor (phase3-round1 tick-2 miss).
GRID_HEIGHT = 96


def integer_cell(col: float, y: float) -> tuple[int, int]:
    """Column and Terrain row for one live billboard sample.

    Column is the integer part of the unrolled angle. Row is
    floor(GRID_HEIGHT - y), the same map as worldToMaskPixel with
    pixelsPerUnit=1. The fractional part of Y is printed, not passed in.
    """
    col = col % COLUMNS
    return int(col), int(GRID_HEIGHT - y)


def fraction(y: float) -> float:
    return y - math.floor(y)


# The printer is a UCRT64 binary. Its DLLs live in that bin directory.
# Python itself must keep the default PATH: capstone is installed there,
# and putting ucrt64 first hides it. The directory is added only on the
# printer's own environment.
UCRT_BIN = Path(r"C:\progs\msys2\ucrt64\bin")


def printer_env() -> dict[str, str]:
    import os
    env = os.environ.copy()
    env["PATH"] = str(UCRT_BIN) + os.pathsep + env.get("PATH", "")
    return env


def run_printer(state: str, x: int, y: int, facing: str, ticks: int) -> list[tuple[int, int, str]]:
    if not PRINTER.is_file():
        raise FileNotFoundError(PRINTER)
    proc = subprocess.run(
        [str(PRINTER), str(GAME_DIR), LEVEL, state, str(x), str(y), facing, str(ticks)],
        check=False,
        capture_output=True,
        text=True,
        env=printer_env(),
    )
    if proc.stderr.strip():
        print(proc.stderr.rstrip(), flush=True)
    if proc.returncode != 0:
        raise RuntimeError(f"compare_tick exited {proc.returncode}: {proc.stderr.strip()}")
    rows = []
    for line in proc.stdout.splitlines():
        tick_s, xs, ys, st = line.split()
        rows.append((int(xs), int(ys), st))
        if int(tick_s) != len(rows) - 1:
            raise RuntimeError(f"printer tick out of order: {line}")
    if len(rows) != ticks + 1:
        raise RuntimeError(f"printer returned {len(rows)} lines, wanted {ticks + 1}")
    return rows


def classify(h) -> list[dict]:
    """One short pair of samples per billboard, so a walker and a climber separate."""
    regs = heap_regions(h)
    addrs = quads(h, regs)
    first = []
    for a in addrs:
        s = sample(h, a)
        if s:
            first.append((a, s))
    time.sleep(0.45)
    out = []
    for a, (c0, y0) in first:
        s = sample(h, a)
        if not s:
            continue
        c1, y1 = s
        out.append({
            "addr": a,
            "col": c1,
            "y": y1,
            "dcol": wrap_delta(c1 - c0),
            "dy": y1 - y0,
        })
    return out


def reacquire(h, col: float, y: float):
    """Find the billboard nearest a lost one. Appearance changes relocate it."""
    best = None
    best_d = 3.0
    for a in quads(h, heap_regions(h)):
        s = sample(h, a)
        if not s:
            continue
        dc = abs(wrap_delta(s[0] - col))
        dy = abs(s[1] - y)
        d = dc + dy
        if dc <= 1.5 and dy <= 0.5 and d < best_d:
            best = (a, s)
            best_d = d
    return best


def _near(a: tuple[float, float], b: tuple[float, float]) -> bool:
    return abs(wrap_delta(a[0] - b[0])) < 0.05 and abs(a[1] - b[1]) < 0.05


def _delta(prev: tuple[float, float], cur: tuple[float, float]) -> tuple[float, float]:
    return abs(wrap_delta(cur[0] - prev[0])), abs(cur[1] - prev[1])


def step_ok(prev: tuple[float, float], cur: tuple[float, float]) -> bool:
    """One simulation tick of motion: about 1 column, or about 1 world unit of Y.

    A jump of ~2 columns is a missed hold between 200 ms steps (phase3-round1),
    not walkSpeed=2. Those must not enter the series. The window is wide
    enough for a live sample that landed at 0.82 (same note).
    """
    dc, dy = _delta(prev, cur)
    if dy < 0.1:
        return 0.75 <= dc <= 1.25
    if dc < 0.1:
        return 0.75 <= dy <= 1.25
    return False


def missed_hold(prev: tuple[float, float], cur: tuple[float, float]) -> bool:
    """About two ticks of motion with nothing recorded in between."""
    dc, dy = _delta(prev, cur)
    if dy < 0.1:
        return 1.6 <= dc <= 2.4
    if dc < 0.1:
        return 1.6 <= dy <= 2.4
    return False


def record(h, addr: int, ticks: int, timeout: float) -> list[tuple[float, float]]:
    """Stable positions at the start and after each one-cell step.

    The billboard holds still between 200 ms steps, but the centroid can
    drift by a few tenths of a column while "still" (phase3-round1 after
    the first harden). Drift below half a column is ignored. A change is
    recorded only after it has held for 40 ms and is about one cell. A
    jump of about two cells resyncs the series start. Anything else that
    is not a step is dropped without clearing the series. If the quad
    address goes dead, the nearest billboard with the same column and
    height is taken up instead.
    """
    start = sample(h, addr)
    if not start:
        return []
    series = [start]
    held = start
    pending = None
    pending_since = 0.0
    miss_since = None
    t0 = time.perf_counter()
    deadline = t0 + timeout
    while len(series) < ticks + 1 and time.perf_counter() < deadline:
        s = sample(h, addr)
        now = time.perf_counter()
        if s is None:
            if miss_since is None:
                miss_since = now
            elif now - miss_since > 0.12:
                found = reacquire(h, held[0], held[1])
                miss_since = None
                if not found:
                    time.sleep(0.005)
                    continue
                addr, s = found
                print(f"  reacquired {addr:#010x} col {s[0]:.3f} y {s[1]:.3f}", flush=True)
            else:
                time.sleep(0.005)
                continue
        else:
            miss_since = None

        dc, dy = _delta(held, s)
        # Billboard noise between ticks — stay on the last committed cell.
        if dc < 0.5 and dy < 0.15:
            pending = None
            time.sleep(0.005)
            continue

        if pending is None or not _near(s, pending):
            pending = s
            pending_since = now
            time.sleep(0.005)
            continue

        if now - pending_since < 0.04:
            time.sleep(0.005)
            continue

        # Pending has held for 40 ms.
        if step_ok(held, pending):
            series.append(pending)
            held = pending
            print(
                f"  step {len(series) - 1}  col {pending[0]:.3f}  y {pending[1]:.3f}",
                flush=True,
            )
        elif missed_hold(held, pending):
            dcol = wrap_delta(pending[0] - held[0])
            dyy = pending[1] - held[1]
            print(
                f"  resync: dcol {dcol:+.3f} dy {dyy:+.3f} — missed hold, new start "
                f"col {pending[0]:.3f} y {pending[1]:.3f}",
                flush=True,
            )
            series = [pending]
            held = pending
        else:
            dcol = wrap_delta(pending[0] - held[0])
            dyy = pending[1] - held[1]
            print(
                f"  ignore: dcol {dcol:+.3f} dy {dyy:+.3f} — not a one-cell step",
                flush=True,
            )
            # Address reused by another quad — find the walker we were on.
            if abs(dcol) > 2.4 or abs(dyy) > 2.4:
                found = reacquire(h, held[0], held[1])
                if found:
                    addr, s = found
                    print(
                        f"  reacquired {addr:#010x} col {s[0]:.3f} y {s[1]:.3f}",
                        flush=True,
                    )
        pending = None
    return series


# On MM_CLIMB_002 the file grid is solid at world Y 32 for columns 128–158
# (the climb wall's mass). Live walkers still appear there at Y 32.190; our
# Terrain treats those cells as walls, so a compare started there sticks.
# Do not fall back into that band — wait_for keeps polling until a walker
# is on empty floor (file Y 32 empty from ~160 upward in the pen, and in
# other empty runs). docs/scratch/phase3-round1.md.
WALL_MASS = (128.0, 158.0)
# Empty pen floor on MM_CLIMB_002 at Y 32.190: solid at 159 and below;
# OPEN_FLOOR is the empty band used for lock/runway (phase3-round1).
OPEN_FLOOR = (160.0, 171.0)


def _on_open_floor(col: float) -> bool:
    return not (WALL_MASS[0] <= col <= WALL_MASS[1])


def pick_walker(rows: list[dict]) -> dict | None:
    """A lemming on empty floor whose column moves by about 1 per tick.

    Returns None while the only floor walkers sit inside WALL_MASS, so the
    caller keeps waiting instead of comparing inside solid.
    """
    walking = []
    for r in rows:
        if abs(r["dy"]) > 0.05:
            continue
        # classify() gaps 0.45 s, so a walker may show ~1 or ~2 columns.
        ad = abs(r["dcol"])
        if abs(ad - 1.0) > 0.05 and abs(ad - 2.0) > 0.08:
            continue
        if abs(r["y"] - FLOOR_Y) > 0.05:
            continue
        if not _on_open_floor(r["col"]):
            continue
        walking.append(r)
    if not walking:
        return None
    open_ledge = [r for r in walking if OPEN_FLOOR[0] <= r["col"] <= OPEN_FLOOR[1]]
    # Eight-tick open path is leftward from the right apex: 168→160. Left
    # from 167 hits solid 159 at tick 8; right from 160 reverses near 167
    # before tick 8. Live often sits at >=168 still classified facing right,
    # then reverses into the leftward run — lock the apex either way.
    apex = [r for r in open_ledge if r["col"] >= 168.0]
    if not apex:
        return None
    apex.sort(key=lambda r: (-r["col"], 0 if r["dcol"] < 0 else 1))
    return apex[0]


def pick_climber(rows: list[dict]) -> dict | None:
    """Column frozen, Y rising by about one world unit per sample gap.

    The 0.45 s gap covers two 200 ms ticks, so dy is about 2 when the
    climber is mid-wall. One tick caught at the edge is about 1.
    """
    climbing = []
    for r in rows:
        if abs(r["dcol"]) > 0.05:
            continue
        if not (0.9 <= r["dy"] <= 2.2):
            continue
        climbing.append(r)
    if not climbing:
        return None
    wall = [r for r in climbing if abs(r["col"] - 128.0) < 2.0]
    pool = wall or climbing
    pool.sort(key=lambda r: abs(r["col"] - 128.0))
    return pool[0]


def wait_for(h, kind: str, timeout: float) -> dict | None:
    pick = pick_walker if kind == "walker" else pick_climber
    deadline = time.time() + timeout
    while time.time() < deadline:
        if ui_state(h) != 0:
            print(f"left the level, ui_state {ui_state(h)}", flush=True)
            return None
        rows = classify(h)
        found = pick(rows)
        if found:
            return found
        print(f"  no {kind} yet ({len(rows)} billboards)", flush=True)
    return None


def report(kind: str, live: list[tuple[float, float]], ours: list[tuple[int, int, str]]) -> tuple[str, int | None]:
    """Print one line per tick. Stop at the first cell disagreement.

    Returns ("match", n) or ("diverge", tick).
    """
    n = min(len(live), len(ours))
    print(f"{kind}: {n - 1} live steps, sim stepped to match", flush=True)
    for i in range(n):
        col, y = live[i]
        ic, iy = integer_cell(col, y)
        ox, oy, st = ours[i]
        frac = fraction(y)
        print(
            f"tick {i}  live {ic},{iy}  ours {ox},{oy}  frac {frac:.3f}"
            f"  live_col {col:.3f}  live_y {y:.3f}  ours_state {st}",
            flush=True,
        )
        if (ic, iy) != (ox, oy):
            print(
                f"diverge tick {i}  live {ic},{iy}  ours {ox},{oy}  frac {frac:.3f}",
                flush=True,
            )
            return "diverge", i
    matched = n - 1
    print(f"{kind}: matched for {matched} ticks", flush=True)
    return "match", matched


def facing_from_delta(dcol: float) -> str:
    """Observation: the sign of the column step is the walk direction.

    Facing::Right advances our x, which is the same column index.
    """
    return "right" if dcol > 0 else "left"


def main() -> int:
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument("--ticks", type=int, default=8, help="ticks to record, unless they diverge sooner")
    ap.add_argument("--wait", type=float, default=90.0, help="seconds to wait for each behaviour")
    ap.add_argument("--only", choices=("both", "walker", "climber"), default="both")
    args = ap.parse_args()
    if args.ticks < 1:
        print("need at least one tick")
        return 2

    h, pid = attach()
    if not h:
        print("the game is not running")
        return 1
    state = ui_state(h)
    path = loaded_level(h)
    print(f"pid {pid}  ui_state {state}  path {path!r}", flush=True)
    if state != 0 or LEVEL.lower() not in path.lower():
        print(f"need ui_state 0 and {LEVEL} in the path buffer {PATH_BUFFER:#x}")
        k32.CloseHandle(h)
        return 1

    walk_result, walk_n = "skipped", None
    if args.only != "climber":
        print("walker: looking for a floor walker", flush=True)
        walker = wait_for(h, "walker", args.wait)
        if not walker:
            print("walker: none found")
            k32.CloseHandle(h)
            return 1
        in_pen = PEN[0] <= walker["col"] <= PEN[1]
        print(
            f"walker locked {walker['addr']:#010x}  col {walker['col']:.3f}"
            f"  y {walker['y']:.3f}  dcol {walker['dcol']:+.3f}  pen {in_pen}",
            flush=True,
        )
        live = record(h, walker["addr"], args.ticks, timeout=args.ticks * TICK_S + 8.0)
        if len(live) < 2:
            print(f"walker: only {len(live)} samples")
            k32.CloseHandle(h)
            return 1
        # Facing is the direction of the recorded series, not of the search
        # sample. A walker at a wall can reverse between those two.
        face = facing_from_delta(wrap_delta(live[1][0] - live[0][0]))
        print(f"walker facing {face} from the first recorded step", flush=True)
        ix, iy = integer_cell(*live[0])
        steps = len(live) - 1
        try:
            ours = run_printer("walker", ix, iy, face, steps)
        except (OSError, RuntimeError) as e:
            print(f"walker: printer failed: {e}")
            k32.CloseHandle(h)
            return 1
        walk_result, walk_n = report("walk", live, ours)

    climb_result, climb_n = "skipped", None
    if args.only != "walker":
        print("climber: looking for a frozen column and rising Y", flush=True)
        climber = wait_for(h, "climber", args.wait)
        if not climber:
            print("climber: none found")
            end_state = ui_state(h)
            end_path = loaded_level(h)
            print(
                f"SUMMARY walk {walk_result} {walk_n}  climb missing"
                f"  pid {pid}  ui_state {end_state}  path {end_path!r}",
                flush=True,
            )
            k32.CloseHandle(h)
            return 1
        # Column is frozen, so the step does not show facing. The measured wall
        # is column 128 and the pen is at greater columns, so the direction that
        # walks into that wall decreases the column. That is a hypothesis about
        # which way this particular climber faces, not a read of +0x40.
        climb_face = "left" if climber["col"] < 148.0 else "right"
        print(
            f"climber locked {climber['addr']:#010x}  col {climber['col']:.3f}"
            f"  y {climber['y']:.3f}  dcol {climber['dcol']:+.3f}  dy {climber['dy']:+.3f}"
            f"  facing {climb_face} (hypothesis: wall side of the pen)",
            flush=True,
        )
        live_c = record(h, climber["addr"], args.ticks, timeout=args.ticks * TICK_S + 8.0)
        if len(live_c) < 2:
            print(f"climber: only {len(live_c)} samples")
            k32.CloseHandle(h)
            print(f"SUMMARY walk {walk_result} {walk_n}")
            return 1
        cx, cy = integer_cell(*live_c[0])
        try:
            ours_c = run_printer("climber", cx, cy, climb_face, len(live_c) - 1)
        except (OSError, RuntimeError) as e:
            print(f"climber: printer failed: {e}")
            k32.CloseHandle(h)
            print(f"SUMMARY walk {walk_result} {walk_n}")
            return 1
        climb_result, climb_n = report("climb", live_c, ours_c)

    end_state = ui_state(h)
    end_path = loaded_level(h)
    end_pid = find_pids("Lemmings")
    print(
        f"SUMMARY walk {walk_result} {walk_n}  climb {climb_result} {climb_n}"
        f"  pid {end_pid[0][0] if end_pid else pid}  ui_state {end_state}  path {end_path!r}",
        flush=True,
    )
    k32.CloseHandle(h)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
