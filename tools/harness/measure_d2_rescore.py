"""D2e: 20 live cursor positions on MM_CLIMB_002, scored with the real tie
keys instead of the forced-zero substitute D2b used.

For every eligible lemming the priority is the live signed dword at
`0x004C05D0 + 4 * [state+0x14]` (`state` = `[lemming+0x340]`), read exactly
as D2c/D2d established, and the squared distance is the sum of squares of
`(lemming+8) - (screen+0x58, +0x60, +0x68)`. Neither key is ever forced to
0 here; a sample where either key is unreadable on an eligible candidate is
discarded, not padded with a guess.

The pick itself is not reimplemented: every row is scored by the landed
`pickHoveredLemming` (`src/app/Pick.cpp`), called through a tiny stdin/stdout
driver built from an unedited HEAD snapshot of that file and `Pick.h`
(`build/tmp/d2e-rescore/snap/app/`), compiled to
`build/tmp/d2e-rescore/pick_driver.exe`.

One Popen of Lemmings Revolution.exe. If a Lemmings process is already
running, or that launch fails, this exits without a second start.

This file imports (does not edit) `measure_d2_pick.py` for the launch shape,
the lemming/cursor/widget/panel readers and the 20-position accept-class
quota, and `measure_d2_tie.py` for the live tie-key reader
(`lemming_tie_fields`), the file-table cross-check and the close-pair aim
search. It does not edit `assign_skill.py`, `measure_spawn.py`,
`measure_steppers.py`, `measure_d2_pick.py`, `measure_d2_tie.py`, or
`src/app/Pick.cpp`.

No skill click, no lemming write, no camera write, no debugger, no Frida,
no `goto_level.focus()`.
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

from acquire_game_foreground import acquire, foreground_pid  # noqa: E402
from lrinput import LRInput  # noqa: E402
from lrstate import attach, i32, ui_state  # noqa: E402
from memdump import find_pids, k32  # noqa: E402
import assign_skill as A  # noqa: E402
import measure_d2_pick as P  # noqa: E402
import measure_d2_tie as T  # noqa: E402
import measure_spawn as SP  # noqa: E402
import measure_steppers as S  # noqa: E402

ROOT = Path(__file__).resolve().parents[2]
OUT = ROOT / "build" / "tmp" / "d2e-rescore"
DRIVER = OUT / "pick_driver.exe"
PREDICTION = OUT / "prediction.md"
STOCK_EXE = P.STOCK_EXE
STOCK_SHA256 = P.STOCK_SHA256
INSTALL = P.INSTALL
WINDOWED_CONF = P.WINDOWED_CONF
UCRT_BIN = P.UCRT_BIN

MAX_ATTEMPTS = 150
SAMPLING_SECONDS_CAP = 480.0
ELIGIBLE_WAIT_SECONDS = 45.0

LOG_FILE = None


def say(msg: str) -> None:
    print(msg, flush=True)
    if LOG_FILE is not None:
        LOG_FILE.write(msg + "\n")
        LOG_FILE.flush()


def dump_json(path: Path, payload) -> None:
    path.write_bytes((json.dumps(payload, indent=2, allow_nan=False) + "\n").encode("utf-8"))


def write_jsonl(path: Path, rows: list[dict]) -> None:
    text = "".join(json.dumps(row, allow_nan=False) + "\n" for row in rows)
    path.write_bytes(text.encode("utf-8"))


def call_driver(cursor_box: dict, cands: list[dict]) -> tuple:
    """Same stdin/stdout protocol as measure_d2_pick.call_driver, pointed at
    this task's own driver (build/tmp/d2e-rescore/pick_driver.exe) instead of
    measure_d2_pick's. Candidates carry the real priority/distance computed
    this round, not forced zeros."""
    lines = ["cursor {left} {top} {right} {bottom}".format(**cursor_box)]
    for c in cands:
        b = c["box"]
        lines.append(
            f"cand {c['index']} {b['left']} {b['top']} {b['right']} {b['bottom']} "
            f"{1 if c['eligible'] else 0} {c['priority']} {c['distance_squared']}"
        )
    lines.append("end")
    payload = ("\n".join(lines) + "\n").encode("ascii")
    env = os.environ.copy()
    env["PATH"] = str(UCRT_BIN) + os.pathsep + env.get("PATH", "")
    proc = subprocess.run(
        [str(DRIVER)], input=payload, stdout=subprocess.PIPE, stderr=subprocess.PIPE,
        env=env, check=False,
    )
    text = proc.stdout.decode("ascii", "replace")
    if proc.returncode != 0:
        raise RuntimeError(
            f"pick_driver exit {proc.returncode}: {proc.stderr.decode('ascii', 'replace')} {text}"
        )
    pick_id = None
    overlap_ids = []
    for line in text.splitlines():
        if line.startswith("pick "):
            tok = line.split()[1]
            pick_id = "none" if tok == "none" else int(tok)
        elif line.startswith("overlaps"):
            parts = line.split()[1:]
            overlap_ids = [int(p) for p in parts]
    if pick_id is None:
        raise RuntimeError(f"pick_driver produced no pick line: {text!r}")
    return pick_id, overlap_ids


def run_self_test() -> int:
    if not DRIVER.exists():
        print(f"error: {DRIVER} missing", file=sys.stderr)
        return 2
    env = os.environ.copy()
    env["PATH"] = str(UCRT_BIN) + os.pathsep + env.get("PATH", "")
    proc = subprocess.run([str(DRIVER), "--self-test"], stdout=subprocess.PIPE,
                          stderr=subprocess.PIPE, env=env, check=False)
    say(proc.stdout.decode("ascii", "replace").rstrip("\n"))
    if proc.stderr:
        say(proc.stderr.decode("ascii", "replace").rstrip("\n"))
    if proc.returncode != 0:
        return proc.returncode
    # Tie on both keys keeps the earlier id.
    pick, overlaps_ids = call_driver(
        {"left": 90, "top": 90, "right": 150, "bottom": 150},
        [
            {"index": 3, "box": {"left": 110, "top": 110, "right": 120, "bottom": 120},
             "eligible": True, "priority": 0, "distance_squared": 0.0},
            {"index": 4, "box": {"left": 110, "top": 110, "right": 120, "bottom": 120},
             "eligible": True, "priority": 0, "distance_squared": 0.0},
        ],
    )
    if pick != 3 or overlaps_ids != [3, 4]:
        print(f"protocol self-test failed: pick={pick} overlaps={overlaps_ids}", file=sys.stderr)
        return 2
    # Higher priority wins outright even against a much smaller distance.
    pick2, _ = call_driver(
        {"left": 90, "top": 90, "right": 150, "bottom": 150},
        [
            {"index": 1, "box": {"left": 110, "top": 110, "right": 120, "bottom": 120},
             "eligible": True, "priority": 0, "distance_squared": 1.0},
            {"index": 2, "box": {"left": 110, "top": 110, "right": 120, "bottom": 120},
             "eligible": True, "priority": 5, "distance_squared": 100.0},
        ],
    )
    if pick2 != 2:
        print(f"priority self-test failed: pick={pick2}", file=sys.stderr)
        return 2
    print("protocol self-test ok", flush=True)
    return 0


def public_candidate(c: dict) -> dict:
    """Raw per-candidate fields: the fields measure_d2_tie.public_lemming
    already reports plus the two live tie keys it computed but does not
    publish, so the facilitator can recompute the winner independently."""
    base = T.public_lemming(c)
    base["priority_used"] = c["priority_used"]
    base["distance_squared"] = c["distance_squared"]
    base["distance_squared_repr"] = c["distance_squared_repr"]
    return base


def snapshot(h, exe: bytes):
    scr = A.screen(h)
    if not scr:
        return None, "screen pointer missing"
    pos = A.cursor(h, scr)
    if pos is None:
        return None, "cursor unreadable"
    cx, cy = pos
    cursor_box, source, raw_doubles, sprite = P.cursor_box_live(h, scr, cx, cy)
    ref_vals, ref_raw = T.read_three_doubles(h, scr + 0x58)
    hover = A.hovered(h, scr)
    ptrs, cands, unread = P.lemming_candidates(h)
    if unread:
        return None, f"unreadable lemming fields {unread}"
    hover_again = A.hovered(h, scr)
    if hover_again != hover:
        hover = hover_again
        ptrs, cands, unread = P.lemming_candidates(h)
        if unread:
            return None, f"unreadable lemming fields {unread}"
        ref_vals, ref_raw = T.read_three_doubles(h, scr + 0x58)
        pos = A.cursor(h, scr)
        if pos is None:
            return None, "cursor unreadable on retry"
        cx, cy = pos
        cursor_box, source, raw_doubles, sprite = P.cursor_box_live(h, scr, cx, cy)
    if ref_vals is None:
        return None, "screen+0x58 unreadable"
    if hover:
        try:
            original_index = ptrs.index(hover)
        except ValueError:
            original_index = "not-in-list"
    else:
        original_index = "none"

    detailed = [T.lemming_tie_fields(h, c, exe, ref_vals) for c in cands]
    missing = [c["index"] for c in detailed if c["eligible"] and
               (c["priority_used"] is None or c["distance_squared"] is None)]
    if missing:
        return None, f"missing live tie-key on eligible actor-list index {missing}"

    driver_cands = [
        {
            "index": c["index"],
            "box": c["box"],
            "eligible": c["eligible"],
            "priority": c["priority_used"] if c["priority_used"] is not None else 0,
            "distance_squared": (
                c["distance_squared"] if c["distance_squared"] is not None else 0.0
            ),
        }
        for c in detailed
    ]
    pick_id, overlap_ids = call_driver(cursor_box, driver_cands)
    by_index = {c["index"]: c for c in detailed}
    if pick_id == "none":
        pick_pointer = "none"
    else:
        picked = by_index.get(pick_id)
        pick_pointer = picked["pointer"] if picked else "missing-candidate"

    hits = [c for c in detailed if c["eligible"] and P.overlaps(c["box"], cursor_box)]

    filters = (
        P.read_byte(h, scr + A.OFF_DIR_FILTER_L),
        P.read_byte(h, scr + A.OFF_DIR_FILTER_R),
    )
    ref_shown, ref_hex = T.dbl_pack(ref_vals, ref_raw)
    row = {
        "cursor_x": repr(cx),
        "cursor_y": repr(cy),
        "cursor_box": cursor_box,
        "cursor_box_source": source,
        "cursor_box_doubles_repr": None if raw_doubles is None else [repr(v) for v in raw_doubles],
        "cursor_sprite": sprite,
        "screen": f"{scr:#010x}",
        "screen_plus_0x58": ref_shown,
        "screen_plus_0x58_hex": ref_hex,
        "original_pointer": f"{hover:#010x}" if hover else "0x00000000",
        "original_index": original_index,
        "pick_id": pick_id,
        "pick_pointer": pick_pointer,
        "hovered_button": A.hovered_button(h, scr),
        "facing_filter_16b1": filters[0],
        "facing_filter_16b2": filters[1],
        "widget_call_gate": i32(h, P.WIDGET_CALL_GATE),
        "level": SP.loaded_level(h),
        "overlaps_from_driver": overlap_ids,
        "overlapping_eligible": [public_candidate(c) for c in hits],
        "n_overlapping_eligible": len(hits),
        "n_eligible": sum(1 for c in detailed if c["eligible"]),
        "all_candidates": [public_candidate(c) for c in detailed],
    }
    row["agree"] = row["original_index"] == row["pick_id"]
    return row, None


def aim_and_snap(h, pid: int, exe: bytes, x: float, y: float):
    mode = A.cursor_mode(h)
    if mode != 2:
        return None, f"cursor_mode {mode} is not 2; mouse not sent", None, None
    try:
        fg = acquire(pid)
    except Exception as exc:
        return None, f"acquire failed: {exc!r}; aim not sent", None, None
    with LRInput() as inp:
        inp.clear()
        inp.enable()
        owner = foreground_pid()
        say(f"  foreground immediately before aim: owner {owner} method {fg.get('method')} game {pid}")
        if owner != pid:
            try:
                fg = acquire(pid)
            except Exception as exc:
                inp.release()
                return None, f"acquire failed: {exc!r}; aim not sent", None, None
            owner = foreground_pid()
            say(f"  foreground rechecked before aim: owner {owner} method {fg.get('method')}")
        if owner != pid:
            inp.release()
            return None, f"foreground owner {owner} is not {pid}; aim not sent", fg, None
        if A.cursor_mode(h) != 2:
            inp.release()
            return None, "cursor_mode changed before aim; mouse not sent", fg, None
        pos = A.aim_at(h, inp, x, y, tol=2.0, tries=16)
        inp.move(0, 0)
        time.sleep(0.25)
        row, err = snapshot(h, exe)
        inp.release()
    return row, err, fg, pos


def rejected_reason(h, row: dict, panels: list[dict], widgets: list[dict]) -> str | None:
    if "MM_CLIMB_002" not in (row["level"] or ""):
        return f"level changed to {row['level']!r}"
    if row["cursor_box_source"] != "live-sprite":
        return f"cursor box source {row['cursor_box_source']}"
    if row["screen_plus_0x58"] is None:
        return "screen+0x58 unreadable"
    hover_btn = row["hovered_button"]
    if hover_btn is None:
        hover_btn = -1
    if P.on_panel(float(row["cursor_x"]), float(row["cursor_y"]), row["cursor_box"], panels, hover_btn):
        return "on a skill-panel widget"
    gate = row["widget_call_gate"]
    if gate and gate > 0 and P.on_widget(row["cursor_box"], widgets):
        return "on a list widget"
    return None


def sample_loop(h, pid: int, exe: bytes) -> tuple[list[dict], list[dict], str | None]:
    rows: list[dict] = []
    attempts: list[dict] = []
    pair_tried: list[tuple[float, float]] = []
    have_pair = False
    on_points: list[tuple[float, float]] = []
    on_i = 0
    empty_points = P.empty_aim_points()
    empty_i = 0
    attempt_n = 0
    deadline = time.time() + SAMPLING_SECONDS_CAP
    failure = None

    while len(rows) < 20 and attempt_n < MAX_ATTEMPTS and time.time() < deadline:
        attempt_n += 1
        _ptrs, cands, unread = P.lemming_candidates(h)
        if unread:
            say(f"  attempt {attempt_n}: unreadable lemming fields {unread}; skipping")
            time.sleep(0.3)
            continue
        panels = P.panel_boxes(h)
        pair_point = None if have_pair else T.choose_aim(cands, panels, pair_tried)
        if pair_point is not None:
            x, y, gap = pair_point
            kind = "pair-search"
        else:
            on_count = sum(1 for r in rows if r["original_index"] != "none")
            none_count = len(rows) - on_count
            want_on = on_count < 5 or (on_count <= none_count and on_count < 10)
            if want_on and on_i >= len(on_points):
                _ptrs, cands2, _unread2 = P.lemming_candidates(h)
                on_points = P.lemming_aim_points(cands2)
                on_i = 0
            if want_on and on_points:
                x, y = on_points[on_i % len(on_points)]
                on_i += 1
                kind = "lemming-center"
            else:
                if empty_i >= len(empty_points):
                    failure = "ran out of empty aim points before 20 kept rows"
                    break
                x, y = empty_points[empty_i]
                empty_i += 1
                kind = "empty-grid"
        say(f"  attempt {attempt_n} {kind} -> ({x:.1f}, {y:.1f}) kept {len(rows)} "
            f"have_pair {have_pair} elapsed {time.time() - (deadline - SAMPLING_SECONDS_CAP):.1f}s")
        try:
            row, err, fg, pos = aim_and_snap(h, pid, exe, x, y)
        except Exception as exc:
            failure = f"aim/acquire failed: {exc!r}"
            say(f"  {failure}")
            break
        if kind == "pair-search":
            pair_tried.append((x, y))
        if err or row is None:
            attempts.append({"n": attempt_n, "kind": kind, "x": x, "y": y, "error": err})
            say(f"    discarded: {err}")
            continue
        panels = P.panel_boxes(h)
        widgets, _notes = P.widget_boxes(h)
        reason = rejected_reason(h, row, panels, widgets)
        has_pair_now = row["n_overlapping_eligible"] >= 2
        force_pair = (not have_pair) and has_pair_now
        if reason is None and not force_pair:
            is_on = row["original_index"] != "none"
            if not P.accept_class(rows, is_on):
                reason = "quota reserved the remaining slots for the other class"
        attempts.append({
            "n": attempt_n, "kind": kind, "target_x": x, "target_y": y,
            "cursor_x": row["cursor_x"], "cursor_y": row["cursor_y"],
            "original_index": row["original_index"], "pick_id": row["pick_id"],
            "n_overlapping_eligible": row["n_overlapping_eligible"],
            "rejected": reason, "foreground": fg.get("method") if fg else None,
            "aim": None if pos is None else [repr(pos[0]), repr(pos[1])],
        })
        if reason:
            say(f"    discarded: {reason} original {row['original_index']} pick {row['pick_id']}")
            continue
        if has_pair_now:
            have_pair = True
        row["sample"] = len(rows) + 1
        row["aim_kind"] = kind
        row["aim_target"] = [x, y]
        rows.append(row)
        say(f"    kept {row['sample']} original {row['original_index']} pick {row['pick_id']}"
            f" agree {row['agree']} n_overlapping_eligible {row['n_overlapping_eligible']}"
            f" box {row['cursor_box_source']}")
    return rows, attempts, failure


def _write_restoration(payload: dict) -> None:
    OUT.mkdir(parents=True, exist_ok=True)
    dump_json(OUT / "restoration.json", payload)
    lines = [
        f"prediction_mtime {payload.get('prediction_mtime')}",
        f"prediction_before_process {payload.get('prediction_before_process')}",
        f"launched {payload.get('launched')}",
        f"pid_started {payload.get('pid_started')}",
        f"pid_gone {payload.get('pid_gone')}",
        f"remaining_lemmings {payload.get('remaining_lemmings')}",
        f"menu_candidates {payload.get('menu_candidates')}",
        f"return_sent {payload.get('return_sent')}",
        f"kept_rows {payload.get('kept_rows')}",
        f"hashes_match {payload.get('hashes_match')}",
        f"hashes_before {payload.get('hashes_before')}",
        f"hashes_after {payload.get('hashes_after')}",
        f"failure {payload.get('failure')}",
    ]
    (OUT / "restoration.log").write_bytes(("\n".join(lines) + "\n").encode("utf-8"))


def run_check_file() -> int:
    data = STOCK_EXE.read_bytes()
    digest = P.sha256_file(STOCK_EXE)
    info = T.check_file_table(data)
    T._self_test_winner()
    say(f"exe sha256 {digest}")
    say(f"file table {json.dumps(info)}")
    if digest != STOCK_SHA256:
        say("error: installed exe does not match the authority hash")
        return 2
    say("file-table check ok; winner self-test ok")
    return 0


def run_live_body() -> int:
    if not PREDICTION.exists():
        say("error: prediction.md is missing; refusing to launch")
        return 2
    if not WINDOWED_CONF.exists():
        say(f"error: windowed config missing: {WINDOWED_CONF}")
        return 2
    rc = run_self_test()
    if rc != 0:
        say("error: pick driver self-test failed; not launching")
        return rc
    exe = STOCK_EXE.read_bytes()
    try:
        table_info = T.check_file_table(exe)
        T._self_test_winner()
    except Exception as exc:
        say(f"error: file table / winner self-test failed: {exc!r}")
        return 2
    say(f"file table check {json.dumps(table_info)}")

    before = P.install_hashes()
    if before["Lemmings Revolution.exe"] != STOCK_SHA256:
        say("error: installed exe does not match the authority hash")
        return 2
    existing = find_pids("Lemmings")
    if existing:
        say("error: a Lemmings process is already running; not launching")
        say(str(existing))
        _write_restoration({
            "prediction_mtime": PREDICTION.stat().st_mtime,
            "launched": False,
            "blocked": "Lemmings process already running",
            "existing": [{"pid": pid, "exe": exe_path} for pid, exe_path in existing],
            "hashes_before": before,
        })
        return 3

    pred_mtime = PREDICTION.stat().st_mtime
    dgv_path = INSTALL / "dgVoodoo.conf"
    save_path = INSTALL / "SavedGame.dat"
    dgv_bytes = dgv_path.read_bytes()
    save_existed = save_path.exists()
    save_bytes = save_path.read_bytes() if save_existed else None
    owned = None
    handle = None
    failure = None
    launched = False
    launch_at = None
    pid = None
    probe = None
    rows: list[dict] = []
    attempts: list[dict] = []
    try:
        dgv_path.write_bytes(WINDOWED_CONF.read_bytes())
        say("launching Lemmings Revolution.exe (ONE start)")
        launched = True
        launch_at = time.time()
        owned = subprocess.Popen([str(STOCK_EXE)], cwd=str(INSTALL))
        pid = owned.pid
        say(f"  pid {pid} started at {launch_at:.3f}; prediction mtime {pred_mtime:.3f}")
        if pred_mtime >= launch_at:
            failure = "prediction file is not older than the process"
            return 2
        time.sleep(12.0)
        handle, attached = attach(write=True)
        if not handle or attached != pid:
            failure = f"attach failed (handle={bool(handle)} attached={attached} owned={pid})"
            return 1
        say(f"attached pid {attached} ui_state {ui_state(handle)}")
        from goto_level import SCREEN_ID
        menu_deadline = time.time() + 20.0
        state = ui_state(handle)
        screen = i32(handle, SCREEN_ID)
        while not (state == 3 and screen == 3) and time.time() < menu_deadline:
            say(f"  waiting for main menu ui_state {state} screen {screen}")
            time.sleep(1.0)
            state = ui_state(handle)
            screen = i32(handle, SCREEN_ID)
        say(f"  before scan ui_state {state} screen {screen}")
        if state != 3 or screen != 3:
            failure = (
                f"three-press scan not run; ui_state {state} screen {screen}; "
                "Down was not sent; Return was not sent"
            )
            return 2
        probe = P.begin_menu_probe(pid)
        try:
            enter_rc = S.enter(handle, "MM_CLIMB_002")
        finally:
            P.end_menu_probe(probe)
        loaded = SP.loaded_level(handle)
        state = ui_state(handle)
        say(
            f"  enter rc {enter_rc} ui_state {state} loaded {loaded!r} "
            f"menu-index candidates {probe.get('candidates')} "
            f"return_sent {probe.get('return_sent')}"
        )
        if probe.get("candidates") == 0:
            failure = (
                f"menu-index candidates 0; return_sent={probe.get('return_sent')}; "
                f"enter rc={enter_rc} ui_state={state} loaded={loaded!r}"
            )
            return 2
        if enter_rc != 0 or state != 0 or "MM_CLIMB_002" not in (loaded or ""):
            failure = (
                f"level entry failed rc={enter_rc} ui_state={state} loaded={loaded!r} "
                f"candidates={probe.get('candidates')} return_sent={probe.get('return_sent')}"
            )
            return 2

        wait_deadline = time.time() + ELIGIBLE_WAIT_SECONDS
        eligible_count = 0
        while time.time() < wait_deadline:
            _ptrs, cands, _unread = P.lemming_candidates(handle)
            eligible_count = sum(1 for c in cands if c["eligible"])
            say(f"  waiting lemmings: {len(cands)} boxed, {eligible_count} eligible, "
                f"released {i32(handle, SP.RELEASED)}")
            if eligible_count >= 4:
                break
            time.sleep(1.0)
        if eligible_count < 2:
            failure = f"level reached; eligible lemmings stayed {eligible_count}"
            return 4

        widgets, widget_notes = P.widget_boxes(handle)
        dump_json(OUT / "widget-probe.json", {
            "gate": i32(handle, P.WIDGET_CALL_GATE), "notes": widget_notes,
            "usable_boxes": widgets,
        })
        say(f"  widget usable boxes {len(widgets)} gate {i32(handle, P.WIDGET_CALL_GATE)}")

        rows, attempts, sample_failure = sample_loop(handle, pid, exe)
        if sample_failure:
            failure = sample_failure
        on_count = sum(1 for row in rows if row["original_index"] != "none")
        none_count = len(rows) - on_count
        pair_count = sum(1 for row in rows if row["n_overlapping_eligible"] >= 2)
        agree = sum(1 for row in rows if row["agree"])
        say(
            f"kept {len(rows)} on {on_count} none {none_count} pair_rows {pair_count} "
            f"agree {agree} disagree {len(rows) - agree}"
        )
        gate_ok = (
            len(rows) == 20 and none_count >= 5 and pair_count >= 1
        )
        if not gate_ok and not failure:
            failure = (
                f"sample missed the 20/5-none/1-pair gate: rows={len(rows)} "
                f"none={none_count} pair_rows={pair_count}"
            )
        return 0 if gate_ok else 4
    except Exception as exc:
        failure = f"exception: {exc!r}"
        import traceback
        traceback.print_exc()
        return 1
    finally:
        if probe is not None:
            P.end_menu_probe(probe)
        if handle:
            try:
                k32.CloseHandle(handle)
            except Exception:
                pass
        gone = None
        if owned is not None:
            try:
                owned.terminate()
                owned.wait(timeout=10)
            except Exception as exc:
                say(f"  terminate warning: {exc}")
            if pid is not None and any(p == pid for p, _exe in find_pids("Lemmings")):
                subprocess.run(
                    ["taskkill", "/PID", str(pid), "/T", "/F"],
                    check=False, stdout=subprocess.PIPE, stderr=subprocess.PIPE,
                )
                try:
                    owned.wait(timeout=10)
                except Exception:
                    pass
            time.sleep(0.5)
            gone = not any(p == pid for p, _exe in find_pids("Lemmings"))
            say(f"  process {pid} gone={gone} returncode {owned.returncode}")
        if dgv_bytes is not None:
            dgv_path.write_bytes(dgv_bytes)
        if save_existed and save_bytes is not None:
            save_path.write_bytes(save_bytes)
        elif not save_existed and save_path.exists():
            save_path.unlink()
        after = P.install_hashes()
        restored = after == before
        say(f"  install hashes match before-state: {restored}")
        if not restored:
            say(f"  before={before}")
            say(f"  after ={after}")
        if attempts:
            write_jsonl(OUT / "attempts.jsonl", attempts)
        picks_written = False
        if rows:
            write_jsonl(OUT / "picks.jsonl", rows)
            picks_written = True
        _write_restoration({
            "prediction_path": str(PREDICTION),
            "prediction_mtime": pred_mtime,
            "prediction_before_process": (
                launch_at > pred_mtime if launched and launch_at is not None else None
            ),
            "launched": launched,
            "pid_started": pid,
            "pid_gone": gone,
            "remaining_lemmings": [
                {"pid": p, "exe": exe_path} for p, exe_path in find_pids("Lemmings")
            ],
            "hashes_before": before,
            "hashes_after": after,
            "hashes_match": restored,
            "failure": failure,
            "kept_rows": len(rows),
            "menu_candidates": None if probe is None else probe.get("candidates"),
            "return_sent": None if probe is None else probe.get("return_sent"),
            "menu_presses": None if probe is None else probe.get("presses"),
            "file_table": table_info if "table_info" in dir() else None,
            "picks_written": picks_written,
        })
        if failure:
            say(f"FAILED: {failure}")
            (OUT / "FAILED.txt").write_bytes((failure + "\n").encode("utf-8"))


class _Tee:
    def __init__(self, stream, path: Path):
        self.stream = stream
        self.file = path.open("w", encoding="utf-8", newline="\n")

    def write(self, data):
        self.stream.write(data)
        self.file.write(data)
        self.file.flush()

    def flush(self):
        self.stream.flush()
        self.file.flush()


def run_live() -> int:
    OUT.mkdir(parents=True, exist_ok=True)
    stdout, stderr = sys.stdout, sys.stderr
    sys.stdout = _Tee(stdout, OUT / "run.log")
    sys.stderr = _Tee(stderr, OUT / "run.err")
    try:
        return run_live_body()
    finally:
        out, err = sys.stdout, sys.stderr
        sys.stdout, sys.stderr = stdout, stderr
        for stream in (out, err):
            closer = getattr(stream, "file", None)
            if closer is not None:
                closer.close()


def main() -> int:
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument("--run-live", action="store_true")
    ap.add_argument("--self-test", action="store_true")
    ap.add_argument("--check-file", action="store_true")
    args = ap.parse_args()
    OUT.mkdir(parents=True, exist_ok=True)
    if args.self_test and not args.run_live:
        return run_self_test()
    if args.check_file and not args.run_live:
        return run_check_file()
    if not args.run_live:
        print("pass --run-live for the one authorized capture, --self-test, or --check-file")
        return 0
    return run_live()


if __name__ == "__main__":
    raise SystemExit(main())
