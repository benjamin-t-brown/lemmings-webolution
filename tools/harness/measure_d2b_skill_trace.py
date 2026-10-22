"""D2f: one live climber assignment on MM_CLIMB_002, 50-tick actor-list trace.

PLAN.md, D2 part (b): "Climber, floater, blocker and digger each assigned
once: the 50 ticks after each assignment match the actor list exactly, and
the skill counts decrement the same way." This round is climber only (the
first of four future rounds on the same gate).

What it reuses, not reimplements
---------------------------------
Nothing here pokes a lemming's bytes or re-derives the click path. The
assignment mechanism -- panel index 0 = climber, cursor-doubles write path,
`[screen+0x16D8]` hover/accept, `lemming+0x128` pick box, stock array
`0x004C91D8` -- was proven 38/38 live in `old/docs/scratch/phase3-assign-skill.md`
and lives in `tools/harness/assign_skill.py`, imported here as `A` and never
edited. This file composes `A.wait_for_lemmings`, `A.select_skill`,
`A.nearest_hoverable` and a plain press/release (the same two calls
`A.single_click`/`A.assign` make) so that the post-click sampling can begin
at the click itself rather than after `A.assign()`'s own internal 0.8 s
state-settle window and 0.6 s double-click wait -- the brief asks for "the
50 ticks after the assignment," and starting the sample window after those
internal sleeps would silently donate ~1.4 s of the window to dead time
rather than lemming motion. This is still "the same proven path": every
memory address and every click primitive is `assign_skill`'s own, called in
the same order it already validates, just not routed through the single
`assign()` convenience wrapper that also sleeps.

The post-click sampling loop, the tick-grid phase inference and the output
file shapes (`live_raw.jsonl`, `live_trace.csv`, `live_trace.meta.json`) are
all `measure_d1_release_trace.py`'s own functions (`capture`, `find_anchors`,
`infer_phase`, `release_residuals`, `build_table`, `write_outputs`,
`write_raw_dump`, `live_cell`, `VT_TO_SIM_STATE`), imported as `D1` and never
edited. D1i/D1j's throttle fix (unthrottled RELEASED-delta discovery trigger
instead of a fixed re-scan timer) is inherited for free because `D1.capture`
is called directly, not copied.

Launch/teardown (hash check, refuse-if-already-running, windowed dgVoodoo
config swap, ONE Popen, `acquire_game_foreground.acquire`, restore-and-verify
on the way out) mirror `measure_d1_release_trace.run_live` for the same
reason D1a gave: this keeps the one authorized start on an already-proven
launch path rather than inventing a new one. `D1.STOCK_EXE`, `D1.STOCK_SHA256`,
`D1.INSTALL` and `D1.WINDOWED_CONF` are reused by reference, not duplicated.

What is new
-----------
Only the composition: click once (recording the click's own wall time as the
sampling window's t=0), then run `D1.capture()` immediately after, then
independently re-read the stock count a second time (decoupled from the
state-change observation per the brief's acceptance criterion), then run the
existing D1 alignment/output functions over the result.

Usage
-----
    python tools/harness/measure_d2b_skill_trace.py --run-live \\
        --out-dir build/tmp/d2f-skill-assign-capture

One `subprocess.Popen` of the stock exe (the single authorized start -- no
retry once it launches). `--level` defaults to `MM_CLIMB_002`.
`--post-seconds`/`--post-ticks`/`--hz` control the post-assignment capture
window (default 14 s / 50 ticks / 50 Hz -- 4 s of margin over the 50-tick /
10 s the brief asks for, to absorb aiming/retry time before the window that
actually counts).
"""
from __future__ import annotations

import argparse
import json
import os
import subprocess
import sys
import time
from pathlib import Path

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))

from lrstate import attach, i32, ui_state  # noqa: E402
from memdump import k32  # noqa: E402
import measure_spawn as SP  # noqa: E402
import measure_steppers as S  # noqa: E402
import assign_skill as A  # noqa: E402
import measure_d1_release_trace as D1  # noqa: E402

ROOT = Path(__file__).resolve().parents[2]
RELEASED = SP.RELEASED
CLOCK = S.CLOCK


def install_hashes() -> dict:
    return D1.install_hashes()


# --- the composed click, with its own t=0 -----------------------------------


def click_and_capture(h, inp, scr: int, skill: int, post_seconds: float,
                       hz: float, click_attempts: int = 4,
                       verbose: bool = True):
    """Select, aim, click once, then sample continuously from the click.

    Returns a dict with the click bookkeeping plus whatever `D1.capture`
    returned, so the caller can run the same alignment/output functions D1a
    uses. Only the FIRST landed click is sampled -- this round assigns one
    climber, once, per the brief.
    """
    out = dict(ok=False, reason="", lemming=0, attempts=0)
    if A.selected(h) != skill and not A.select_skill(h, inp, skill):
        out["reason"] = f"panel {skill} not selected (0x004C9200={A.selected(h)})"
        return out

    tried = set()
    for n in range(click_attempts):
        out["attempts"] = n + 1
        target, aim = A.nearest_hoverable(h, inp, scr, exclude=tried)
        if not target:
            if verbose:
                print(f"  attempt {n}: no hoverable lemming yet"
                      f" ({len(A.lemmings(h))} live); retrying", flush=True)
            time.sleep(0.6)
            tried.clear()
            continue
        tried.add(target)
        before = A.read_lem(h, target)
        sb = A.state_of(h, target)
        counts_before = A.counts(h)

        t_click = time.perf_counter()
        inp.button(0, True)
        inp.wait_frames(4)
        inp.button(0, False)

        # The climber attribute flip is documented as instant
        # (old/docs/scratch/phase3-assign-skill.md, Observation 6); confirm it
        # landed with a short synchronous re-read before committing the one
        # long capture window to this lemming. This is a correctness check,
        # not the brief's required sampling -- the real 50-tick sample is
        # D1.capture() below, which starts immediately after.
        time.sleep(0.05)
        after_quick = A.read_lem(h, target)
        counts_after = A.counts(h)
        landed = bool(
            after_quick and before and
            (after_quick["climber"] != before["climber"] or
             (counts_before and counts_after and
              counts_after[skill] == counts_before[skill] - 1)))
        if verbose:
            print(f"  attempt {n}: {target:#010x} climber"
                  f" {before['climber'] if before else '?'} ->"
                  f" {after_quick['climber'] if after_quick else '?'}"
                  f"  stock[{skill}] {counts_before[skill] if counts_before else '?'}"
                  f" -> {counts_after[skill] if counts_after else '?'}"
                  f"  landed={landed}", flush=True)
        if not landed:
            out["reason"] = "clicked on a hovered lemming, no attribute/stock change seen"
            continue

        out.update(ok=True, lemming=target, reason="climber attribute or stock moved",
                   state_before=sb[1], t_click=t_click,
                   counts_before=counts_before, counts_after_quick=counts_after,
                   climber_before=before["climber"] if before else None,
                   climber_after_quick=after_quick["climber"] if after_quick else None,
                   x_before=before["x"] if before else None,
                   y_before=before["y"] if before else None)
        if verbose:
            print(f"  landed on {target:#010x}; starting {post_seconds:.1f}s"
                  f" post-assignment capture now (no further aiming)", flush=True)
        order, first_seen, raw, released_events, discovery_log = D1.capture(
            h, post_seconds, hz)
        out.update(order=order, first_seen=first_seen, raw=raw,
                   released_events=released_events, discovery_log=discovery_log)
        counts_final = A.counts(h)
        after_final = A.read_lem(h, target)
        out.update(counts_after_final=counts_final,
                   climber_after_final=after_final["climber"] if after_final else None)
        return out
    return out


# --- live run ----------------------------------------------------------------


def run_live(level: str, post_seconds: float, post_ticks: int, hz: float,
             out_dir: Path, boot_wait: float = 10.0,
             click_attempts: int = 4) -> int:
    if not D1.STOCK_EXE.exists():
        print(f"error: {D1.STOCK_EXE} not found", file=sys.stderr)
        return 1
    before_hashes = install_hashes()
    if before_hashes["Lemmings Revolution.exe"] != D1.STOCK_SHA256:
        print("error: installed exe does not match the authority hash",
              file=sys.stderr)
        return 1
    if attach()[0]:
        print("error: a Lemmings Revolution process is already running;"
              " refusing to launch a second one (one Popen rule)",
              file=sys.stderr)
        return 1

    out_dir.mkdir(parents=True, exist_ok=True)
    dgv_backup = D1.INSTALL / "dgVoodoo.conf"
    dgv_saved_bytes = dgv_backup.read_bytes()
    owned = None
    h = None
    failure = None
    click_result = None
    try:
        dgv_backup.write_bytes(D1.WINDOWED_CONF.read_bytes())

        print(f"launching {D1.STOCK_EXE} (ONE authorized start)", flush=True)
        owned = subprocess.Popen([str(D1.STOCK_EXE)], cwd=D1.INSTALL)
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
                      " clean level start, matching D1a's own precondition)")
            return 2

        print("  waiting for the hatch to release at least one lemming"
              " (no input sent) ...", flush=True)
        n0 = A.wait_for_lemmings(h, seconds=20.0)
        if not n0:
            failure = "no lemming in the actor list within 20 s"
            return 2
        print(f"  {n0} lemming(s) live; counts before any click:"
              f" {A.counts(h)}", flush=True)

        scr = A.screen(h)
        if not scr:
            failure = "no level screen object after entry"
            return 2
        if A.cursor_mode(h, scr) != 2:
            failure = "0x0045328C != 2: mouse is not driving a cursor on this screen"
            return 2

        from lrinput import LRInput
        with LRInput() as inp:
            inp.clear()
            inp.enable()
            try:
                click_result = click_and_capture(
                    h, inp, scr, skill=0, post_seconds=post_seconds, hz=hz,
                    click_attempts=click_attempts)
            finally:
                inp.release()

        if not click_result.get("ok"):
            failure = (f"climber assignment never landed in"
                      f" {click_result.get('attempts')} attempts:"
                      f" {click_result.get('reason')}")
            return 3

        print(f"  assignment landed on {click_result['lemming']:#010x}"
              f"  climber {click_result['climber_before']} ->"
              f" {click_result['climber_after_quick']} (quick) ->"
              f" {click_result['climber_after_final']} (after capture)"
              f"  stock[0] {click_result['counts_before'][0]} ->"
              f" {click_result['counts_after_quick'][0]} (quick) ->"
              f" {click_result['counts_after_final'][0]} (after capture)",
              flush=True)

        after_state = ui_state(h)
        after_loaded = SP.loaded_level(h)
        print(f"  after capture: ui_state {after_state} loaded {after_loaded!r}"
              f" released {i32(h, RELEASED)} clock {i32(h, CLOCK)}", flush=True)

        order = click_result["order"]
        first_seen = click_result["first_seen"]
        raw = click_result["raw"]
        released_events = click_result["released_events"]
        discovery_log = click_result["discovery_log"]

        anchors, per_lemming_anchors = D1.find_anchors(raw, released_events)
        phase, spread, fallback = D1.infer_phase(anchors)
        rel_resid = D1.release_residuals(released_events)
        print(f"  tick-grid phase {phase:.4f}s (spread {spread},"
              f" {per_lemming_anchors} walker/climber anchors, fallback={fallback})",
              flush=True)

        # The assigned lemming's own anchors, isolated, per the prediction's
        # falsifiable claim that a per-lemming phase is more trustworthy than
        # the pooled one when several lemmings are in flight at once (D1a's
        # own observation). Diagnostic only; `phase` above (pooled across
        # every live lemming, exactly as D1.find_anchors/infer_phase do it)
        # is what build_table actually uses, for consistency with D1's own
        # convention and so this file does not invent a second tick grid.
        target = click_result["lemming"]
        target_raw = {target: raw.get(target, [])}
        t_anchors, t_per = D1.find_anchors(target_raw, [])
        t_phase, t_spread, t_fallback = D1.infer_phase(t_anchors)
        print(f"  assigned lemming's OWN phase (diagnostic only):"
              f" {t_phase:.4f}s (spread {t_spread}, {t_per} anchors,"
              f" fallback={t_fallback})", flush=True)

        rows, n_min = D1.build_table(order, first_seen, raw, phase, post_ticks)
        D1.write_raw_dump(out_dir, order, first_seen, raw, released_events)
        D1.write_outputs(
            out_dir, rows, order, first_seen, released_events, anchors,
            per_lemming_anchors, phase, spread, fallback, n_min, rel_resid,
            {
                "level": level, "skill_index": 0, "skill_name": "climber",
                "requested_post_seconds": post_seconds,
                "requested_post_ticks": post_ticks, "hz": hz,
                "ui_state_after_entry": state, "ui_state_after_capture": after_state,
                "loaded_level_after_entry": loaded,
                "loaded_level_after_capture": after_loaded,
                "released_at_entry": released0, "clock_at_entry": clock0,
                "released_at_end": i32(h, RELEASED), "clock_at_end": i32(h, CLOCK),
                "pid": pid, "boot_wait_seconds": boot_wait,
                "assigned_lemming_ptr": target,
                "assigned_lemming_index_in_order": (
                    order.index(target) if target in order else None),
                "assign_attempts": click_result["attempts"],
                "t_click_perf_counter": click_result["t_click"],
                "counts_before_click": click_result["counts_before"],
                "counts_after_quick": click_result["counts_after_quick"],
                "counts_after_capture": click_result["counts_after_final"],
                "climber_byte_before": click_result["climber_before"],
                "climber_byte_after_quick": click_result["climber_after_quick"],
                "climber_byte_after_capture": click_result["climber_after_final"],
                "x_before_click": click_result["x_before"],
                "y_before_click": click_result["y_before"],
                "assigned_lemming_own_phase_seconds_diagnostic": t_phase,
                "assigned_lemming_own_phase_spread_diagnostic": t_spread,
                "assigned_lemming_own_phase_n_anchors_diagnostic": t_per,
            },
            discovery_log=discovery_log)

        # Save the click bookkeeping itself (not just the aligned table) so a
        # reader can see the raw before/after counts/attribute reads without
        # cross-referencing the console log.
        (out_dir / "assign_result.json").write_text(json.dumps({
            k: v for k, v in click_result.items()
            if k not in ("order", "first_seen", "raw", "released_events",
                         "discovery_log")
        }, indent=2, default=str))
        print(f"  wrote {out_dir / 'assign_result.json'}", flush=True)

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
                         " enter --level, wait for lemmings, assign climber"
                         " once, sample 50+ ticks, write artifacts, tear down")
    ap.add_argument("--level", default="MM_CLIMB_002")
    ap.add_argument("--post-seconds", type=float, default=14.0,
                    help="post-click capture window, real seconds"
                         " (default 14s: 10s/50 ticks the brief asks for,"
                         " plus margin)")
    ap.add_argument("--post-ticks", type=int, default=50)
    ap.add_argument("--hz", type=float, default=50.0)
    ap.add_argument("--boot-wait", type=float, default=10.0)
    ap.add_argument("--click-attempts", type=int, default=4)
    ap.add_argument("--out-dir", default="build/tmp/d2f-skill-assign-capture")
    args = ap.parse_args()

    out_dir = (ROOT / args.out_dir) if not os.path.isabs(args.out_dir) else Path(args.out_dir)

    if not args.run_live:
        print("pass --run-live to launch the one authorized capture"
              " (see the module docstring)")
        return 0

    return run_live(args.level, args.post_seconds, args.post_ticks, args.hz,
                    out_dir, boot_wait=args.boot_wait,
                    click_attempts=args.click_attempts)


if __name__ == "__main__":
    raise SystemExit(main())
