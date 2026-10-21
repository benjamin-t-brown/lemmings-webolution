"""D1a: release and walk-out capture on MM_CLIMB_002, no input, for the diff
against `webo::sim::World` (`src/tools/SimReleaseTrace.cpp`,
`build/tmp/d1a-release-walkout-trace/diff_trace.py`).

PLAN.md "Phase D -- reconnect the simulation", D1's gate: from level start,
with no input at all once the level is playing, do every lemming's cell
X/Y/facing/state match the original each tick for 150 ticks (30 s), and does
the released count/order match? This script captures the original's side of
that question; it never writes to a lemming, the grid or a save target.

What it reuses, not reimplements
---------------------------------
Everything that touches live memory goes through modules this brief is
allowed to import and not edit: `lrstate` (attach/i32/u32/ui_state),
`memdump` (k32, process/region helpers), `measure_spawn` (actor_ptrs,
loaded_level, LEMMING_VT/OTHER_VT), `measure_steppers` (enter -- the safe
select_level + restart_level.ensure_playing level-entry pattern, NEVER
goto_level.focus(); lemmings(); sample(); CLOCK/RELEASED; live_cell(), the
same X/Y -> cell rounding testRetailSpawn and measure_steppers.compare()
use). `acquire_game_foreground.acquire()` is the documented safe
foreground helper (match-the-original.md, "Acquire foreground
automatically"), used once before the one input batch that enters the
level; no input follows it.

Read lemmings from the actor list (0x004CBB18) only -- measure_steppers
.lemmings()/.sample() already do, never a billboard quad (LESSONS.md,
"A plausible series can come from the wrong object").

D1j fix (this file only, queue id `d1j-harness-throttle-fix`): `capture()`'s
re-enumeration of that actor list used to be gated on a fixed 1.0 s wall
timer, so a newly-released lemming could go unsampled for up to ~1.02 s past
its true appearance (see docs/scratch/phase-d1-release-to-actor.md,
"Implication for D1g's spawn-tick-drift residual" -- D1i proved there is no
such gate in the original, so that lag was purely this script's own
artifact). `capture()` and the new `discover_new_actors()` now trigger
discovery off the RELEASED counter's own unthrottled per-pass read (already
happening every pass for the release-event log) and do a targeted tail read
of only the newly appended pointers, not a full vector re-scan -- see
`capture()`'s own docstring for the cost argument and
`build/tmp/d1j-harness-throttle-fix/prediction.md` for the full reasoning
written before the live re-check.

The new work: generalizing a tick clock across several live lemmings
------------------------------------------------------------------
Round 1/10 found that a WALKER's X, and a CLIMBER's Y, land on an exact
integer at the engine's own probe instant (`0x0041BECE`/`0x0041BF61` snap
X; the climb stepper does the same to Y -- LESSONS.md, "The simulation").
`measure_steppers.tick_series` uses that for one lemming's first
contiguous run of one state. This capture needs a tick index that is
GLOBAL (every lemming's row at tick T must mean the same wall-clock
instant, including lemmings that have not spawned yet, or have splattered,
or are mid-fall where neither X nor Y necessarily lands on an integer): the
whole point of the gate is comparing released order and count too, not
just one lemming's path.

The period is not re-derived here -- it is the measured constant
TICK_SECONDS = 0.2 s (Rules::ticksPerSecond = 5, measured three ways,
LESSONS.md). What this script derives per capture is the *phase*: which
wall-clock instants are tick boundaries. Two independent families of events
sit exactly on a tick boundary and both are sampled in the same pass, so
both contribute:

  * every walker-X / climber-Y probe-instant snap, from every lemming
    alive during the capture (not just the first one, and not stopped at
    the first state-run the way tick_series is);
  * every RELEASED-counter increment (a release happens inside the same
    tick's step as everything else -- it does not matter here *which*
    tick number a release lands on, only that it lands on *a* tick
    boundary, which is assumed, not re-derived, from the existing
    measure_release.py / LESSONS.md finding that one accumulator drives
    the whole simulation).

Pooling every such instant's `t mod TICK_SECONDS` and taking the median
(the residuals cluster tightly around one value if the assumption holds;
reported in the metadata so this is checkable, not asserted) gives the
phase. Tick index T=1 is then the first point of the resulting grid at or
after the capture's own start (the wall-clock instant `enter()` confirmed
ui_state 0, i.e. play beginning with no further input) -- the same
definition World::tick() uses (its first call IS tick 1). Every
subsequent tick is phase-exact at +0.2 s. For each (tick, lemming) cell,
the raw sample closest in time to that nominal instant is used, converted
to a cell with `measure_steppers.live_cell`-style rounding; a sample more
than half a tick (0.1 s) away is reported as missing rather than guessed.

This is a capture-time inference, not a byte read, and is reported as
exactly that: the aligned CSV's sidecar `*.meta.json` carries the phase,
how many anchors of each kind supported it, and the per-row `sample_dt`
(how far the matched raw sample actually was from the nominal tick time),
so a reader (or diff_trace.py, or a human re-deriving one row by hand from
the raw dump) can audit the alignment without re-running the game.

Usage
-----
    python tools/harness/measure_d1_release_trace.py --run-live \\
        --out-dir build/tmp/d1a-release-walkout-trace

One Popen of the stock exe (an authorized, one-shot start -- no retry once
it launches). `--seconds`/`--hz`/`--ticks` override the capture window,
sample rate and reported tick count. `--level` overrides the level (default
MM_CLIMB_002). If anything goes wrong after the process launches, this
reports the exact failure and still runs its teardown (restore
dgVoodoo.conf, terminate the process); it does not relaunch.
"""
from __future__ import annotations

import argparse
import csv
import hashlib
import json
import os
import statistics
import struct
import subprocess
import sys
import time
from pathlib import Path

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))

from lrstate import attach, i32, u32, ui_state  # noqa: E402
from memdump import k32, read_mem  # noqa: E402
import measure_spawn as SP  # noqa: E402
import measure_steppers as S  # noqa: E402

ROOT = Path(__file__).resolve().parents[2]
STOCK_EXE = Path(r"C:\progs\Lemmings Revolution\Lemmings Revolution.exe")
STOCK_SHA256 = "32675bdb49866e76502713251fdfd2b09078c2481c72ca22ff68ebf4b3737a2f"
INSTALL = STOCK_EXE.parent
WINDOWED_CONF = ROOT / "build/tmp/a1-zoom-elevation/dgVoodoo.windowed.conf"

RELEASED = S.RELEASED
CLOCK = S.CLOCK
ACTOR_LIST = SP.ACTOR_LIST
TICK_SECONDS = 0.2   # Rules::ticksPerSecond = 5, measured (docs/LESSONS.md)
ANCHOR_TOL = 0.06    # world units; LESSONS "the walk probe snaps X to an
                     # exact integer before every test" -- generous of the
                     # ~0.03-unit render-frame step the same section notes
GRID_HEIGHT = 96     # terrain rows, every retail level (LESSONS.md)
GRID_WIDTH = 256

# measure_steppers.VT_NAMES labels these by strip/animation name, not by
# webo::sim::State's own string (stateName() in src/sim/World.cpp). Translate
# so diff_trace.py compares like-for-like against SimReleaseTrace's "state"
# column. Anything not in this table (a gadget vtable this no-input,
# no-skill run should never reach) prints as "unknown_vt_<hex>" rather than
# being silently mapped to something plausible.
VT_TO_SIM_STATE = {
    0x004B0D28: "walker",
    0x004B0960: "climber",
    0x004B0AF8: "hoister",
    0x004B0A20: "faller",
    0x004B0668: "splatter",
    0x004B07B8: "blocker",
    0x004B08DC: "builder",
    0x004B0C78: "shrugger",
    0x004B06AC: "basher",
    0x004B09F0: "digger",
    0x004B0B68: "miner",
    0x004B0A58: "exploder",
    0x004B0810: "bounce",     # trampoline, round 13 -- S.VT_NAMES: fall-810
    0x004B0570: "drown",      # water/acid, round 14/15 -- not in S.VT_NAMES
}
VT_WALK = 0x004B0D28
VT_CLIMB = 0x004B0960


def sim_state_name(vt: int) -> str:
    return VT_TO_SIM_STATE.get(vt, f"unknown_vt_{vt:#010x}")


def install_hashes() -> dict[str, str]:
    names = ("Lemmings Revolution.exe", "dgVoodoo.conf")
    out = {}
    for n in names:
        p = INSTALL / n
        out[n] = hashlib.sha256(p.read_bytes()).hexdigest() if p.exists() else None
    return out


# --- capture -----------------------------------------------------------------


def discover_new_actors(h, scanned: int):
    """Targeted tail read: examine only raw array slots not yet scanned.

    D1j fix (see docs/scratch/phase-d1-release-to-actor.md, "Implication for
    D1g's spawn-tick-drift residual"). The old mechanism re-scanned the
    *whole* actor-list vector every re-enumeration, including every slot
    already examined, costing one `u32` vtable-check read per already-seen
    slot on every re-scan. The array is append-only (D1i's exe read:
    `0x00423146`/`0x004233A0`, grow-if-full then unconditional
    `array[count] = ptr; count += 1`, never shrinks, never reorders), so
    anything at raw index < `scanned` has already been examined and
    classified -- there is no reason to touch it again.

    `scanned` is the number of raw array *slots* already read and vtable-
    checked (by any previous call), not the number of lemmings accepted --
    the two differ whenever a slot's `u32` read does not come back as a
    lemming actor vtable (observed live: MM_CLIMB_002's actor list holds a
    couple of non-lemming entries ahead of/interleaved with the lemmings,
    rejected by the filter below but still occupying raw slots). Passing
    `len(order)` instead of a true raw-scanned count here would under-count
    how far the array has actually been read, and the next call would
    needlessly re-examine already-rejected (or already-accepted) slots --
    still correct (duplicates are caught by the caller's `first_seen`
    dedup, and a filtered-out vtable never flips to a lemming one), but a
    few more `u32` reads than the minimum. This function takes the raw
    count explicitly so the caller can track it precisely.

    Returns (new_ptrs, count, scanned_now):
      new_ptrs    newly classified lemming-actor pointers in this window,
                  already vtable-filtered, in array order (callers must
                  still dedup against anything already known -- this
                  function does not see the caller's `first_seen`)
      count       the array's own current size (for logging)
      scanned_now how many raw slots have now been examined (== count when
                  the read succeeded; == `scanned` unchanged on any read
                  failure, so a caller never thinks it scanned something it
                  could not actually read)
    """
    count = i32(h, ACTOR_LIST)
    if count is None or count <= scanned:
        return [], count, scanned
    arr = u32(h, ACTOR_LIST + 0x0C)
    if arr is None or arr < 0x10000:
        return [], count, scanned
    n_window = count - scanned
    raw_tail = read_mem(h, arr + scanned * 4, n_window * 4)
    if not raw_tail or len(raw_tail) != n_window * 4:
        return [], count, scanned
    candidates = struct.unpack(f"<{n_window}I", raw_tail)
    new_ptrs = [p for p in candidates if u32(h, p) in (SP.LEMMING_VT, SP.OTHER_VT)]
    return new_ptrs, count, count


def capture(h, seconds: float, hz: float):
    """Sample every lemming plus RELEASED/CLOCK at a capped rate.

    Returns (order, first_seen, raw, released_events, discovery_log):
      order         actor pointers, first-seen order == spawn order (spawn()
                    only ever appends -- the same identity World::lemmings()
                    uses for its own vector index)
      first_seen    {ptr: t} the wall time each pointer was first observed
      raw           {ptr: [(t, x, y, facing, vt), ...]}
      released_events  [(t, before, after, clock), ...] on every RELEASED change
      discovery_log [(t, reason, n_new, array_count), ...] every time new
                    pointer(s) were added to `order`/`live`, and why (see
                    "D1j fix" below)

    D1j fix: detecting a newly-released lemming no longer waits for a fixed
    1.0 s re-enumeration timer (which could delay first sampling of a brand
    new pointer by up to ~1.02 s past its true appearance -- see
    docs/scratch/phase-d1-release-to-actor.md). `RELEASED`
    (`0x004CBA78`) is already read every pass below, unthrottled, for the
    release-event log; this reuses that exact read as the discovery
    trigger, at zero marginal read cost on the passes where nothing
    changed. D1i's exe read showed the actor-list `push_back` executes
    strictly before this same counter's increment, inside one synchronous
    constructor call with no clock/animation-frame check between them -- so
    by the pass this poll observes `RELEASED` having grown, the new
    pointer is already in the array; there is no race to wait out. On a
    trigger, `discover_new_actors` does a targeted tail read (cost bounded
    by how many pointers are actually new, not by how many are already
    known) instead of re-scanning the whole vector.

    A full `S.lemmings(h)` re-scan (the old, `O(total_count)` mechanism)
    is kept as a low-frequency (5 s), non-authoritative diagnostic backstop:
    logged if it ever disagrees with what the delta trigger already found,
    not trusted over it, and not on the sampling hot path. 5 s is slower
    than the old 1.0 s throttle (so this backstop alone would still have the
    old lag), but it no longer needs to be fast -- the delta trigger is now
    the thing that has to be prompt, and it is driven by a read that already
    happens every pass, not by this backstop's own timer.
    """
    order: list[int] = []
    first_seen: dict[int, float] = {}
    raw: dict[int, list] = {}
    released_events = []
    discovery_log = []
    last_released = i32(h, RELEASED)
    t0 = time.perf_counter()
    last_backstop = -1.0
    live: list[int] = []
    scanned = 0  # raw actor-list slots examined so far -- see
                 # discover_new_actors's docstring on why this is tracked
                 # separately from len(order) (not every raw slot is a
                 # lemming actor)
    gap = 1.0 / hz if hz > 0 else 0.0
    passes = 0

    def add_new(ptrs, t):
        n_added = 0
        for p in ptrs:
            if p not in first_seen:
                first_seen[p] = t
                order.append(p)
                live.append(p)
                n_added += 1
        return n_added

    while True:
        t = time.perf_counter() - t0
        if t >= seconds:
            break
        rel = i32(h, RELEASED)
        new_release = rel is not None and rel != last_released
        # Discovery trigger: the unthrottled RELEASED read above (not a new
        # read added by this fix), plus a bootstrap check before the very
        # first lemming exists (cheap: 1 extra i32 read/pass, only while
        # `live` is still empty).
        if new_release or not live:
            new_ptrs, count, scanned = discover_new_actors(h, scanned)
            if new_ptrs:
                n_added = add_new(new_ptrs, t)
                if n_added:
                    discovery_log.append((
                        t, "released_delta" if new_release else "bootstrap",
                        n_added, count,
                    ))
        for p in live:
            smp = S.sample(h, p, t)
            if smp is None:
                continue
            raw.setdefault(p, []).append((t, smp.x, smp.y, smp.facing, smp.vt))
        if new_release:
            released_events.append((t, last_released, rel, i32(h, CLOCK)))
            last_released = rel
        # Diagnostic-only backstop (see docstring): full O(total_count)
        # re-scan at most every 5 s, cross-checked against `order` and
        # logged -- never silently trusted -- rather than relied on for
        # sampling or for the discovery timing this round is fixing.
        if t - last_backstop > 5.0:
            full = S.lemmings(h)
            missed = [p for p in full if p not in first_seen]
            if missed:
                print(f"  WARNING: backstop scan at t={t:.3f}s found"
                      f" {len(missed)} pointer(s) the delta trigger missed;"
                      " adding them now (late)", flush=True)
                add_new(missed, t)
                discovery_log.append((t, "backstop_missed", len(missed), None))
            last_backstop = t
        passes += 1
        if gap:
            # Capping the rate matters -- measure_steppers.py's own warning:
            # an uncapped single-lemming loop drove ~300k ReadProcessMemory/s
            # and killed the game twice. The pass rate here is still capped
            # at `hz` exactly as before; this fix changes what happens
            # *inside* a pass (a handful of extra, population-bounded reads
            # only on the passes a release/backstop actually fires), not
            # whether passes themselves are rate-limited. Up to ~10 lemmings
            # * 2 reads each + a small, bounded discovery/backstop overhead,
            # * hz passes/s; hz 50 is comfortably under the danger threshold
            # above (see prediction.md's cost argument for the arithmetic).
            nxt = t0 + passes * gap
            while time.perf_counter() < nxt:
                pass
    n_samples = sum(len(v) for v in raw.values())
    print(f"  {passes} passes, {n_samples} lemming samples, {len(order)} lemmings"
          f" seen, {len(released_events)} release events,"
          f" {len(discovery_log)} discovery trigger(s) over {seconds:.1f}s"
          f" ({n_samples / max(seconds, 1e-9):.0f} reads/s)", flush=True)
    return order, first_seen, raw, released_events, discovery_log


# --- tick-grid inference -----------------------------------------------------


def find_anchors(raw: dict[int, list], released_events: list) -> list[float]:
    """Probe-instant candidate times: walker-X / climber-Y near-integer snaps.

    RELEASED-counter transitions are measured separately (see
    `release_residuals`) and are NOT pooled in here: this capture measured
    them landing on an almost-constant ~2.887 s gap (~14.43 ticks, not a
    whole number), confirming -- same shape as the already-landed finding
    that RELEASERATE.PC holds literal seconds, not a tick count -- that a
    release is a continuous real-time countdown, unrelated to the 200 ms
    walker/climber tick grid. Treating it as a tick-boundary anchor was
    this script's own first-draft bug; the near-full-period residual spread
    it produced is what caught it (see round-note.md).
    """
    anchors: list[float] = []
    per_lemming = 0
    for _ptr, samples in raw.items():
        last_t = -1.0
        for t, x, y, _facing, vt in samples:
            if vt == VT_WALK:
                v = x
            elif vt == VT_CLIMB:
                v = y
            else:
                continue
            d = abs(v - round(v))
            if d < ANCHOR_TOL and t - last_t > 0.15:
                anchors.append(t)
                last_t = t
                per_lemming += 1
    anchors.sort()
    return anchors, per_lemming


def release_residuals(released_events: list) -> dict:
    """Diagnostic only: how release-event times sit against the 0.2 s grid.

    Not used for `infer_phase` (see `find_anchors`); kept and reported so
    the "releases are not tick-aligned" finding is checkable from the
    metadata, not just asserted.
    """
    times = [t for t, _b, _a, _c in released_events]
    if not times:
        return {"n": 0}
    residuals = [t - TICK_SECONDS * round(t / TICK_SECONDS) for t in times]
    gaps = [b - a for a, b in zip(times, times[1:])]
    return {
        "n": len(times),
        "residual_min": min(residuals), "residual_max": max(residuals),
        "residual_spread": max(residuals) - min(residuals),
        "mean_gap_seconds": statistics.mean(gaps) if gaps else None,
        "mean_gap_ticks": (statistics.mean(gaps) / TICK_SECONDS) if gaps else None,
        "first_release_t": times[0],
        "first_release_ticks": times[0] / TICK_SECONDS,
    }


def infer_phase(anchors: list[float]):
    """Circular median of `t mod TICK_SECONDS`: the tick grid's phase.

    Returns (phase, residual_spread, fallback) where `fallback` is True when
    there were no anchors at all and phase defaults to 0.0 (the grid aligned
    to the capture's own t=0) -- a reported limitation, not a silent guess.
    """
    if not anchors:
        return 0.0, None, True
    residuals = [t - TICK_SECONDS * round(t / TICK_SECONDS) for t in anchors]
    phase = statistics.median(residuals)
    spread = (max(residuals) - min(residuals)) if len(residuals) > 1 else 0.0
    return phase, spread, False


def live_cell(x: float, y: float) -> tuple[int, int]:
    """(column, Y-down terrain row), the testRetailSpawn/compare() rounding."""
    col = int(round(x)) % GRID_WIDTH
    row = GRID_HEIGHT - 1 - int(round(y))
    row = max(0, min(GRID_HEIGHT - 1, row))
    return col, row


def build_table(order, first_seen, raw, phase: float, ticks: int,
                 match_tol: float = TICK_SECONDS / 2.0):
    """One row per (tick, lemming_index) with the nearest raw sample."""
    # Tick 1 is the first grid point >= 0 (the capture's own t=0, the wall
    # instant sampling began right after enter() confirmed ui_state 0).
    n_min = 0
    while phase + n_min * TICK_SECONDS < -1e-9:
        n_min += 1
    rows = []
    for T in range(1, ticks + 1):
        t_t = phase + (n_min + T - 1) * TICK_SECONDS
        # Independent of whether any lemming has a matching sample this
        # tick: released count is "how many pointers were first seen by
        # this nominal tick time", which is well-defined even with zero
        # live lemmings yet (e.g. every tick before the first release).
        released_by_t = sum(1 for p in order if first_seen[p] <= t_t + 1e-9)
        n_rows_this_tick = 0
        for idx, ptr in enumerate(order):
            samples = raw.get(ptr, [])
            if not samples or first_seen[ptr] > t_t + match_tol:
                continue
            best = min(samples, key=lambda s: abs(s[0] - t_t))
            dt = best[0] - t_t
            if abs(dt) > match_tol:
                continue
            _t, x, y, facing, vt = best
            col, row = live_cell(x, y)
            rows.append({
                "tick": T, "lemming_index": idx, "x": col, "y": row,
                "facing": int(facing), "state": sim_state_name(vt),
                "released": released_by_t, "sample_dt": round(dt, 4),
            })
            n_rows_this_tick += 1
        if n_rows_this_tick == 0:
            # A marker row (empty lemming fields, matching SimReleaseTrace's
            # own tick-0 convention) so "released" is never silently
            # dropped for a tick with no matched lemming sample -- without
            # this, a tick before the first release looked like "no live
            # data" instead of "released 0", which it genuinely is.
            rows.append({
                "tick": T, "lemming_index": "", "x": "", "y": "",
                "facing": "", "state": "", "released": released_by_t,
                "sample_dt": "",
            })
    return rows, n_min


def write_outputs(out_dir: Path, rows, order, first_seen, released_events,
                   anchors, per_lemming_anchors, phase,
                   spread, fallback, n_min, rel_resid: dict, meta_extra: dict,
                   discovery_log: list | None = None):
    out_dir.mkdir(parents=True, exist_ok=True)
    csv_path = out_dir / "live_trace.csv"
    with open(csv_path, "w", newline="") as f:
        w = csv.writer(f)
        w.writerow(["tick", "lemming_index", "x", "y", "facing", "state",
                   "released", "sample_dt"])
        for r in rows:
            w.writerow([r["tick"], r["lemming_index"], r["x"], r["y"],
                       r["facing"], r["state"], r["released"], r["sample_dt"]])
    meta = {
        "tick_seconds": TICK_SECONDS,
        "anchor_tol_world_units": ANCHOR_TOL,
        "phase_seconds": phase,
        "phase_residual_spread_seconds": spread,
        "phase_fallback_no_anchors": fallback,
        "n_anchors_walker_climber": per_lemming_anchors,
        "n_min_grid_points_before_capture_start": n_min,
        "lemmings_seen": len(order),
        "first_seen_seconds": {str(i): first_seen[p] for i, p in enumerate(order)},
        "released_events": [
            {"t": t, "before": b, "after": a, "clock": c}
            for t, b, a, c in released_events
        ],
        "release_residuals_diagnostic_not_used_for_phase": rel_resid,
        # D1j: when/why each pointer was added to `order` -- "released_delta"
        # is the fix's own trigger (the already-unthrottled RELEASED read
        # noticed growth), "bootstrap" is the pre-first-lemming fallback
        # check, "backstop_missed" would mean the delta trigger missed a
        # pointer the 5s diagnostic re-scan later found (expected to be
        # empty; not silently assumed empty -- see round-note.md).
        "discovery_log": [
            {"t": t, "reason": reason, "n_new": n, "array_count": c}
            for t, reason, n, c in (discovery_log or [])
        ],
    }
    meta.update(meta_extra)
    meta_path = out_dir / "live_trace.meta.json"
    meta_path.write_text(json.dumps(meta, indent=2))
    print(f"  wrote {csv_path} ({len(rows)} rows) and {meta_path}", flush=True)
    return csv_path, meta_path


def write_raw_dump(out_dir: Path, order, first_seen, raw, released_events):
    """Every raw sample, for an independent re-derivation of one row by hand."""
    path = out_dir / "live_raw.jsonl"
    with open(path, "w") as f:
        for idx, ptr in enumerate(order):
            for t, x, y, facing, vt in raw.get(ptr, []):
                f.write(json.dumps({
                    "lemming_index": idx, "ptr": ptr, "t": t, "x": x, "y": y,
                    "facing": facing, "vt": vt,
                }) + "\n")
        for t, before, after, clock in released_events:
            f.write(json.dumps({
                "event": "released", "t": t, "before": before, "after": after,
                "clock": clock,
            }) + "\n")
    print(f"  wrote {path} (raw dump, {sum(len(v) for v in raw.values())}"
          f" lemming samples + {len(released_events)} release events)",
          flush=True)
    return path


# --- live run -----------------------------------------------------------------


def run_live(level: str, seconds: float, hz: float, ticks: int, out_dir: Path,
             boot_wait: float = 10.0) -> int:
    if not STOCK_EXE.exists():
        print(f"error: {STOCK_EXE} not found", file=sys.stderr)
        return 1
    before_hashes = install_hashes()
    if before_hashes["Lemmings Revolution.exe"] != STOCK_SHA256:
        print("error: installed exe does not match the authority hash",
              file=sys.stderr)
        return 1
    if attach()[0]:
        print("error: a Lemmings Revolution process is already running;"
              " refusing to launch a second one", file=sys.stderr)
        return 1

    out_dir.mkdir(parents=True, exist_ok=True)
    dgv_backup = INSTALL / "dgVoodoo.conf"
    dgv_saved_bytes = dgv_backup.read_bytes()
    owned = None
    h = None
    failure = None
    try:
        # Same windowed-config swap other landed rounds use (e.g. A4p
        # cmd_start_menu) -- not required for a pure memory read, but this
        # keeps the one authorized start on the same proven launch path
        # rather than inventing a new one.
        dgv_backup.write_bytes(WINDOWED_CONF.read_bytes())

        print(f"launching {STOCK_EXE} (ONE authorized start)", flush=True)
        owned = subprocess.Popen([str(STOCK_EXE)], cwd=INSTALL)
        time.sleep(boot_wait)

        h, pid = attach(write=True)
        if not h or pid != owned.pid:
            failure = f"attach failed after launch (handle={bool(h)}, pid={pid}, owned={owned.pid})"
            return 1

        print(f"attached pid {pid}, ui_state {ui_state(h)}", flush=True)

        try:
            from acquire_game_foreground import acquire
            fg = acquire(pid)
            print(f"  foreground: {fg.get('method')}", flush=True)
        except Exception as exc:  # pragma: no cover - best-effort only
            print(f"  foreground acquisition skipped/failed: {exc}"
                  " (gameplay input goes through the dinput shim regardless"
                  " of OS focus; proceeding)", flush=True)

        rc = S.enter(h, level)
        loaded = SP.loaded_level(h)
        state = ui_state(h)
        if rc != 0 or state != 0 or level not in loaded:
            failure = (f"level entry failed: enter() rc={rc} ui_state={state}"
                      f" loaded={loaded!r}")
            return 2

        released0 = i32(h, RELEASED)
        clock0 = i32(h, CLOCK)
        print(f"  entered {loaded!r}; released {released0}, clock {clock0}s",
              flush=True)
        if released0 != 0:
            failure = (f"level was not fresh or RELEASED unreadable:"
                      f" RELEASED={released0!r} (expected exactly 0 at a"
                      " clean level start)")
            return 2

        print(f"capturing {seconds:.1f}s / target {ticks} ticks at {hz:.0f} Hz,"
              " no further input ...", flush=True)
        t_start_wall = time.time()
        order, first_seen, raw, released_events, discovery_log = capture(h, seconds, hz)
        t_end_wall = time.time()

        after_state = ui_state(h)
        after_loaded = SP.loaded_level(h)
        print(f"  after capture: ui_state {after_state} loaded {after_loaded!r}"
              f" released {i32(h, RELEASED)} clock {i32(h, CLOCK)}", flush=True)

        anchors, per_lemming_anchors = find_anchors(raw, released_events)
        phase, spread, fallback = infer_phase(anchors)
        rel_resid = release_residuals(released_events)
        print(f"  tick-grid phase {phase:.4f}s (spread {spread},"
              f" {per_lemming_anchors} walker/climber anchors, fallback={fallback})",
              flush=True)
        print(f"  release residuals (diagnostic, NOT used for phase):"
              f" {rel_resid}", flush=True)
        backstop_misses = [d for d in discovery_log if d[1] == "backstop_missed"]
        print(f"  D1j discovery log: {len(discovery_log)} trigger(s)"
              f" ({sum(1 for d in discovery_log if d[1] == 'released_delta')}"
              f" released_delta, {len(backstop_misses)} backstop_missed"
              " -- backstop_missed>0 would mean the delta trigger lagged"
              " and the diagnostic backstop had to catch up)", flush=True)

        rows, n_min = build_table(order, first_seen, raw, phase, ticks)
        write_raw_dump(out_dir, order, first_seen, raw, released_events)
        write_outputs(
            out_dir, rows, order, first_seen, released_events, anchors,
            per_lemming_anchors, phase, spread, fallback,
            n_min, rel_resid,
            {
                "level": level, "requested_seconds": seconds,
                "requested_ticks": ticks, "hz": hz,
                "capture_wall_start": t_start_wall, "capture_wall_end": t_end_wall,
                "ui_state_after_entry": state, "ui_state_after_capture": after_state,
                "loaded_level_after_entry": loaded,
                "loaded_level_after_capture": after_loaded,
                "released_at_entry": released0, "clock_at_entry": clock0,
                "released_at_end": i32(h, RELEASED), "clock_at_end": i32(h, CLOCK),
                "pid": pid, "boot_wait_seconds": boot_wait,
            },
            discovery_log=discovery_log)
        return 0
    except Exception as exc:  # the one-shot rule: report, do not relaunch
        failure = f"exception during capture: {exc!r}"
        import traceback
        traceback.print_exc()
        return 1
    finally:
        if h:
            try:
                k32.CloseHandle(h)
            except Exception:
                pass
        if owned is not None:
            try:
                owned.terminate()
                owned.wait(timeout=10)
            except Exception as exc:
                print(f"  warning: could not cleanly terminate pid"
                      f" {owned.pid}: {exc}", flush=True)
            print(f"  process {owned.pid} terminated"
                  f" (returncode {owned.returncode})", flush=True)
        dgv_backup.write_bytes(dgv_saved_bytes)
        after_hashes = install_hashes()
        restored = after_hashes == before_hashes
        print(f"  dgVoodoo.conf restored; install hashes match before-state:"
              f" {restored}", flush=True)
        if not restored:
            print(f"  before={before_hashes}\n  after ={after_hashes}",
                  file=sys.stderr)
        if failure:
            print(f"FAILED: {failure}", file=sys.stderr)
            (out_dir / "FAILED.txt").write_text(failure + "\n")


def main() -> int:
    ap = argparse.ArgumentParser(description=__doc__,
                                 formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--run-live", action="store_true",
                    help="launch the stock game (ONE authorized start),"
                         " enter --level, capture, align and write the"
                         " aligned CSV + raw dump + metadata, then tear down")
    ap.add_argument("--level", default="MM_CLIMB_002")
    ap.add_argument("--seconds", type=float, default=30.0)
    ap.add_argument("--ticks", type=int, default=150)
    ap.add_argument("--hz", type=float, default=50.0)
    ap.add_argument("--boot-wait", type=float, default=10.0)
    ap.add_argument("--out-dir", default="build/tmp/d1a-release-walkout-trace")
    args = ap.parse_args()

    out_dir = (ROOT / args.out_dir) if not os.path.isabs(args.out_dir) else Path(args.out_dir)

    if not args.run_live:
        print("pass --run-live to launch the one authorized capture"
              " (see the module docstring)")
        return 0

    return run_live(args.level, args.seconds, args.hz, args.ticks, out_dir,
                    boot_wait=args.boot_wait)


if __name__ == "__main__":
    raise SystemExit(main())
