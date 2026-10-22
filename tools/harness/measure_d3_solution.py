"""D3d: find and record one scripted solution for MM_CLIMB_002, with the original's
own saved count and end time.

Question (`build/tmp/d3d-solution-discovery-brief.md`): what scripted sequence of skill
assignments completes `MM_CLIMB_002` in the original, and what are the original's own
saved count and end time (whole seconds) for that run? This is oracle-gathering only --
nothing in `src/` is read or changed by this file.

What this reuses, not reimplements
-----------------------------------
Every memory address and every click primitive here is already-landed harness code,
imported by reference:

    `assign_skill.py` (as `A`)           -- panel select / aim / click / verify, the
                                             exact mechanism `old/docs/scratch/
                                             phase3-assign-skill.md` proved 38/38 live.
    `measure_steppers.py` (as `S`)       -- `S.enter` (record-swap level entry,
                                             restoring slot [0][0]), `S.CLOCK`.
    `measure_spawn.py` (as `SP`)         -- `SP.RELEASED`, `SP.loaded_level`.
    `measure_exit.py` (as `E`)           -- `E.saved_count` / `E.SAVED_WORD`
                                             (`0x004CBAA4`, "inc when state+8 >= 1.0").
    `measure_float_splat.py` (as `FS`)   -- `FS.TIME_LIMIT` (`0x004CBAA8`) and
                                             `FS.SAVE_TARGET` (`0x004CBAB0`), the two
                                             fields next to `SAVED_WORD`/`CLOCK` in the
                                             same already-read in-level statics block.
    `measure_d1_release_trace.py` (as `D1`) -- launch/teardown: hash check,
                                             refuse-if-already-running, windowed
                                             dgVoodoo config swap, ONE `Popen`,
                                             `acquire_game_foreground.acquire`,
                                             restore-and-verify on the way out. Same
                                             shape every D2(b) round used.

`CLOCK` (`0x004CBAAC`) is the level's own countdown, already cross-checked live in
`build/tmp/d1g-full-gate-recheck/live_trace.meta.json`: `clock_at_entry=298`,
`clock_at_end=268` over a measured 30.0014s capture window -- a drop of exactly 30 over
30.0s of real time, i.e. **CLOCK decrements by 1 per elapsed real second**, already in
whole seconds. `end_time_seconds = time_limit_seconds - clock_at_level_end` is this
round's own reading of "the original's own end time in whole seconds," off a field four
separate already-landed harness modules already treat as live in-level state, not a new
address found this round.

Usage
-----
    python tools/harness/measure_d3_solution.py --run-live \
        --out-dir build/tmp/d3d-solution-discovery

One `subprocess.Popen` of the stock exe per invocation (the one-Popen rule; run this
at most twice per the brief's budget). `--level` defaults to `MM_CLIMB_002`.
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
import assign_skill as A  # noqa: E402
import measure_spawn as SP  # noqa: E402
import measure_steppers as S  # noqa: E402
import measure_exit as E  # noqa: E402
import measure_float_splat as FS  # noqa: E402
import measure_d1_release_trace as D1  # noqa: E402

ROOT = Path(__file__).resolve().parents[2]
RELEASED = SP.RELEASED
CLOCK = S.CLOCK
SAVED_WORD = E.SAVED_WORD
TIME_LIMIT = FS.TIME_LIMIT
SAVE_TARGET = FS.SAVE_TARGET
RESULTS_UI_STATE = 5   # `old/docs/scratch/phase3-round12-exit-models.md`:
                       # "pid 26872 was on ui_state 5 / results when restored"


def install_hashes() -> dict:
    out = D1.install_hashes()
    saved_game = D1.INSTALL / "SavedGame.dat"
    out["SavedGame.dat"] = (
        __import__("hashlib").sha256(saved_game.read_bytes()).hexdigest()
        if saved_game.exists() else None
    )
    return out


# --- the scripted solution: climber onto every lemming as it is hoverable ---


def assign_climber_to_every_lemming(h, inp, total_lemmings: int,
                                     attempts: int = 4, wait: float = 20.0,
                                     verbose: bool = True) -> list[dict]:
    """Repeat `assign_skill.assign(skill=0)` up to `total_lemmings` times.

    Reuses `A.assign` exactly as `assign_skill.py --skill 0 --repeat N` already proved
    10/10 live on this level (`old/docs/scratch/phase3-assign-skill.md`, Observation 6).
    No new click mechanism; this is the composition the brief's "Required reading"
    points at.
    """
    results = []
    for r in range(total_lemmings):
        if verbose:
            print(f"-- assignment {r} of 0..{total_lemmings - 1} (skill 0, climber)",
                  flush=True)
        res = A.assign(h, inp, skill=0, attempts=attempts, window=0.4,
                        wait=wait, verbose=verbose)
        results.append(res)
        if verbose:
            print(f"   -> ok={res['ok']} lemming={res.get('lemming', 0):#010x}"
                  f" {res['reason']}", flush=True)
        if not res["ok"] and "no lemming in the actor list" in res["reason"]:
            # The hatch is done (or never started); further attempts would
            # just repeat the same block. Stop early rather than burn the
            # per-attempt waits for no reason.
            if verbose:
                print("   (hatch appears finished or stalled; stopping"
                      " the assignment loop early)", flush=True)
            break
    return results


# --- watch to a level-end state, no further input ---------------------------


def watch_to_level_end(h, poll_hz: float = 5.0, hard_cap_seconds: float = 330.0,
                       verbose: bool = True) -> dict:
    """Poll SAVED_WORD/CLOCK/RELEASED/ui_state/alive-count, no input sent.

    Stops on the first `ui_state != 0` (the level's own end state) or when
    `hard_cap_seconds` of wall time elapses (a safety bound well past the level's own
    300s time limit, to absorb this script's own assignment-phase latency plus a
    results-screen settle margin). Returns every sample plus the final reading.
    """
    t0 = time.perf_counter()
    samples = []
    prev_saved = None
    prev_ui = None
    period = 1.0 / poll_hz if poll_hz > 0 else 0.2
    while True:
        t = time.perf_counter() - t0
        sc = E.i16(h, SAVED_WORD)
        clk = i32(h, CLOCK)
        rel = i32(h, RELEASED)
        st = ui_state(h)
        alive = len(A.lemmings(h))
        row = dict(t=t, saved=sc, clock=clk, released=rel, ui_state=st, alive=alive)
        samples.append(row)
        if verbose and (sc != prev_saved or st != prev_ui):
            print(f"  t{t:7.2f}s saved={sc} clock={clk} released={rel}"
                  f" ui_state={st} alive={alive}", flush=True)
        prev_saved, prev_ui = sc, st
        if st is not None and st != 0:
            break
        if t >= hard_cap_seconds:
            if verbose:
                print(f"  hard cap {hard_cap_seconds:.0f}s reached without a"
                      " ui_state change; stopping the watch", flush=True)
            break
        time.sleep(period)
    return dict(samples=samples, final=samples[-1])


# --- live run -----------------------------------------------------------------


def run_live(level: str, out_dir: Path, boot_wait: float = 10.0,
             assign_attempts: int = 4, assign_wait: float = 20.0,
             poll_hz: float = 5.0, hard_cap_seconds: float = 330.0) -> int:
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
    dgv_path = D1.INSTALL / "dgVoodoo.conf"
    dgv_saved_bytes = dgv_path.read_bytes()
    save_path = D1.INSTALL / "SavedGame.dat"
    save_saved_bytes = save_path.read_bytes() if save_path.exists() else None

    owned = None
    h = None
    failure = None
    result: dict = {}
    try:
        dgv_path.write_bytes(D1.WINDOWED_CONF.read_bytes())

        print(f"launching {D1.STOCK_EXE} (ONE authorized start)", flush=True)
        owned = subprocess.Popen([str(D1.STOCK_EXE)], cwd=D1.INSTALL)
        time.sleep(boot_wait)

        h, pid = attach(write=True)
        if not h or pid != owned.pid:
            failure = (f"attach failed after launch (handle={bool(h)}, pid={pid},"
                      f" owned={owned.pid})")
            return 1

        print(f"attached pid {pid}, ui_state {ui_state(h)}", flush=True)

        try:
            from acquire_game_foreground import acquire, foreground_pid
            fg = acquire(pid)
            actual = foreground_pid()
            print(f"  foreground: {fg.get('method')}, verified owner pid={actual}"
                  f" (expected {pid})", flush=True)
            if actual != pid:
                print("  warning: foreground owner is not the attached pid after"
                      " acquire(); proceeding anyway since the dinput shim drives"
                      " input independent of OS focus", flush=True)
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
        saved0 = E.i16(h, SAVED_WORD)
        save_target0 = i32(h, SAVE_TARGET)
        time_limit0 = i32(h, TIME_LIMIT)
        print(f"  entered {loaded!r}; released={released0} clock={clock0}s"
              f" saved={saved0} save_target={save_target0}"
              f" time_limit={time_limit0}s", flush=True)
        if released0 != 0:
            failure = (f"level was not fresh or RELEASED unreadable:"
                      f" RELEASED={released0!r} (expected exactly 0 at a"
                      " clean level start)")
            return 2

        print("  waiting for the hatch to release at least one lemming"
              " (no input sent) ...", flush=True)
        n0 = A.wait_for_lemmings(h, seconds=20.0)
        if not n0:
            failure = "no lemming in the actor list within 20 s"
            return 2
        print(f"  {n0} lemming(s) live; stock before any click: {A.counts(h)}",
              flush=True)

        scr = A.screen(h)
        if not scr:
            failure = "no level screen object after entry"
            return 2
        if A.cursor_mode(h, scr) != 2:
            failure = "0x0045328C != 2: mouse is not driving a cursor on this screen"
            return 2

        total_lemmings = 10  # sweep.txt: MM_CLIMB_002 totalLemmings=10
        from lrinput import LRInput
        assign_results = []
        with LRInput() as inp:
            inp.clear()
            inp.enable()
            try:
                assign_results = assign_climber_to_every_lemming(
                    h, inp, total_lemmings, attempts=assign_attempts,
                    wait=assign_wait)
            finally:
                inp.release()

        landed = [r for r in assign_results if r["ok"]]
        distinct_lemmings = {r["lemming"] for r in landed}
        print(f"  {len(landed)} of {len(assign_results)} assignment attempts landed,"
              f" {len(distinct_lemmings)} distinct lemmings got climber;"
              f" stock now {A.counts(h)}", flush=True)

        print("  watching for a level-end state (no further input) ...",
              flush=True)
        watch = watch_to_level_end(h, poll_hz=poll_hz, hard_cap_seconds=hard_cap_seconds)
        final = watch["final"]

        end_clock = final["clock"]
        end_time_seconds = (
            (time_limit0 - end_clock) if (time_limit0 is not None
                                          and end_clock is not None) else None
        )

        result = dict(
            level=level, pid=pid,
            total_lemmings=total_lemmings,
            save_target=save_target0, time_limit_seconds=time_limit0,
            released_at_entry=released0, clock_at_entry=clock0,
            saved_at_entry=saved0,
            assign_results=[{k: v for k, v in r.items() if k != "state_series"}
                            for r in assign_results],
            distinct_lemmings_assigned=[hex(p) for p in distinct_lemmings],
            stock_after_assignment=A.counts(h),
            watch_sample_count=len(watch["samples"]),
            final_sample=final,
            end_ui_state=final["ui_state"],
            end_saved_count=final["saved"],
            end_clock=end_clock,
            end_time_seconds=end_time_seconds,
            end_released=final["released"],
            end_alive_in_actor_list=final["alive"],
            won=bool(save_target0 is not None and final["saved"] is not None
                    and final["saved"] >= save_target0),
            reached_level_end_state=bool(final["ui_state"] not in (0, None)),
        )
        (out_dir / "solution.json").write_text(json.dumps(result, indent=2, default=str))
        (out_dir / "watch_samples.json").write_text(
            json.dumps(watch["samples"], indent=2, default=str))
        print(f"\nRESULT: ui_state {final['ui_state']} (results screen ="
              f" {RESULTS_UI_STATE})  saved {final['saved']} / target"
              f" {save_target0}  end_time_seconds={end_time_seconds}"
              f"  won={result['won']}", flush=True)
        print(f"  wrote {out_dir / 'solution.json'}", flush=True)
        return 0 if result["reached_level_end_state"] else 4
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
        dgv_path.write_bytes(dgv_saved_bytes)
        if save_saved_bytes is not None:
            save_path.write_bytes(save_saved_bytes)
        after_hashes = install_hashes()
        restored = after_hashes == before_hashes
        print(f"  dgVoodoo.conf + SavedGame.dat restored; install hashes match"
              f" before-state: {restored}", flush=True)
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
                    help="launch the stock game (ONE authorized start) and run")
    ap.add_argument("--level", default="MM_CLIMB_002")
    ap.add_argument("--out-dir", type=Path,
                    default=ROOT / "build/tmp/d3d-solution-discovery")
    ap.add_argument("--boot-wait", type=float, default=10.0)
    ap.add_argument("--assign-attempts", type=int, default=4)
    ap.add_argument("--assign-wait", type=float, default=20.0)
    ap.add_argument("--poll-hz", type=float, default=5.0)
    ap.add_argument("--hard-cap-seconds", type=float, default=330.0,
                    help="safety bound on the post-assignment watch, past the"
                         " level's own 300s time limit")
    args = ap.parse_args()

    if not args.run_live:
        print("pass --run-live to launch the stock game and run the scripted"
              " solution; this file has no offline mode (it is a live oracle"
              " capture, not a unit test)")
        return 0

    return run_live(args.level, args.out_dir, boot_wait=args.boot_wait,
                    assign_attempts=args.assign_attempts,
                    assign_wait=args.assign_wait, poll_hz=args.poll_hz,
                    hard_cap_seconds=args.hard_cap_seconds)


if __name__ == "__main__":
    raise SystemExit(main())
