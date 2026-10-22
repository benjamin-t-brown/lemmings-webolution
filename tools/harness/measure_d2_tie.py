"""D2d: one live two-lemming overlap, priority then squared distance.

One Popen of Lemmings Revolution.exe. If a Lemmings process is already
running, or that launch fails, this exits without a second start.

The launch and main-menu entry are the shape in measure_d2_pick.py that
reached MM_CLIMB_002. This file does not edit that module. It does not call
goto_level.focus, click a skill, write a lemming, or write the camera.

No debugger. No Frida.
"""
from __future__ import annotations

import argparse
import json
import os
import struct
import subprocess
import sys
import time
from pathlib import Path

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))

from acquire_game_foreground import acquire, foreground_pid  # noqa: E402
from lrinput import LRInput  # noqa: E402
from lrstate import attach, i32, read_mem, u32, ui_state  # noqa: E402
from memdump import find_pids, k32  # noqa: E402
import assign_skill as A  # noqa: E402
import measure_d2_pick as P  # noqa: E402
import measure_spawn as SP  # noqa: E402
import measure_steppers as S  # noqa: E402

ROOT = Path(__file__).resolve().parents[2]
OUT = ROOT / "build" / "tmp" / "d2d-tie-live"
PREDICTION = OUT / "prediction.md"
STOCK_EXE = P.STOCK_EXE
STOCK_SHA256 = P.STOCK_SHA256
INSTALL = P.INSTALL
WINDOWED_CONF = P.WINDOWED_CONF

PRIORITY_TABLE = 0x004C05D0
# Settled file dwords 0..19. Index 0x12 is the 19th of these and is 0.
SETTLED_HEAD = [2, 1, 1, 4, 2, 3, 3, 3, 3, 1, 0, 2, 2, 2, 2, 2, 2, 0, 0, 2]
WALKER_VT = 0x004B0D28
WALKER_INDEX = 0x12

MAX_CURSORS = 30
SEARCH_SECONDS = 60.0
ELIGIBLE_WAIT_SECONDS = 45.0

LOG_FILE = None


def say(msg: str) -> None:
    print(msg, flush=True)
    if LOG_FILE is not None:
        LOG_FILE.write(msg + "\n")
        LOG_FILE.flush()


def dump_json(path: Path, payload: dict) -> None:
    path.write_bytes((json.dumps(payload, indent=2, allow_nan=False) + "\n").encode("utf-8"))


def pe_sections(data: bytes):
    e_lfanew = struct.unpack_from("<I", data, 0x3C)[0]
    if data[e_lfanew:e_lfanew + 4] != b"PE\0\0":
        raise ValueError("not a PE")
    nsections = struct.unpack_from("<H", data, e_lfanew + 6)[0]
    opt_size = struct.unpack_from("<H", data, e_lfanew + 20)[0]
    magic = struct.unpack_from("<H", data, e_lfanew + 24)[0]
    if magic != 0x10B:
        raise ValueError(f"PE magic {magic:#x}")
    image_base = struct.unpack_from("<I", data, e_lfanew + 24 + 28)[0]
    sec = e_lfanew + 24 + opt_size
    out = []
    for i in range(nsections):
        off = sec + i * 40
        name = data[off:off + 8].split(b"\0", 1)[0].decode("ascii", "replace")
        vsize, vrva, rawsize, rawptr = struct.unpack_from("<IIII", data, off + 8)
        out.append({
            "name": name,
            "vsize": vsize,
            "rva": vrva,
            "rawsize": rawsize,
            "rawptr": rawptr,
        })
    return image_base, out


def va_file_offset(data: bytes, va: int):
    """File offset of a VA, or None when that byte is past the section raw size."""
    image_base, sections = pe_sections(data)
    rva = va - image_base
    for sec in sections:
        span = max(sec["vsize"], sec["rawsize"])
        if sec["rva"] <= rva < sec["rva"] + span:
            delta = rva - sec["rva"]
            if delta >= sec["rawsize"]:
                return None, sec, image_base
            return sec["rawptr"] + delta, sec, image_base
    return None, None, image_base


def table_va(index_u: int) -> int:
    return (PRIORITY_TABLE + (index_u & 0xFFFFFFFF) * 4) & 0xFFFFFFFF


def file_i32(data: bytes, va: int):
    off, sec, image_base = va_file_offset(data, va)
    if off is None:
        return None, off, sec, image_base
    return struct.unpack_from("<i", data, off)[0], off, sec, image_base


def check_file_table(data: bytes) -> dict:
    head = []
    sections_seen = []
    for i in range(20):
        value, off, sec, image_base = file_i32(data, PRIORITY_TABLE + 4 * i)
        if value is None:
            raise RuntimeError(f"file dword {i} at {PRIORITY_TABLE + 4 * i:#010x} is not in raw bytes")
        head.append(value)
        if sec is not None and sec["name"] not in [s["name"] for s in sections_seen]:
            sections_seen.append(sec)
    if head != SETTLED_HEAD:
        raise RuntimeError(f"file table head {head} != settled {SETTLED_HEAD}")
    idx_value, idx_off, idx_sec, image_base = file_i32(data, table_va(WALKER_INDEX))
    if idx_value != 0:
        raise RuntimeError(f"file dword index 0x12 is {idx_value}, settled value is 0")
    info = {
        "image_base": f"{image_base:#010x}",
        "section": None if idx_sec is None else idx_sec["name"],
        "section_vsize": None if idx_sec is None else idx_sec["vsize"],
        "section_rawsize": None if idx_sec is None else idx_sec["rawsize"],
        "section_rva": None if idx_sec is None else f"{idx_sec['rva']:#010x}",
        "index_0x12_file_offset": None if idx_off is None else f"{idx_off:#x}",
        "index_0x12_file_i32": idx_value,
        "head_20": head,
        "within_rawsize": True,
    }
    return info


def dbl_pack(vals: tuple[float, float, float] | None, raw: bytes | None):
    if vals is None or raw is None:
        return None, None
    return [repr(v) for v in vals], raw.hex()


def read_three_doubles(h, addr: int):
    raw = read_mem(h, addr, 24)
    if not raw or len(raw) != 24:
        return None, None
    return struct.unpack("<3d", raw), raw


def distance_squared(lem, ref) -> float:
    return (lem[0] - ref[0]) ** 2 + (lem[1] - ref[1]) ** 2 + (lem[2] - ref[2]) ** 2


def compute_winner(cands: list[dict]):
    """Higher signed priority, then strict smaller distance, else the earlier index.

    Same sentinels as 0x00454D7D (-1) and 0x00455069 (100000000.0). A missing
    key is not replaced with 0; the caller must not pass such a candidate.
    """
    best = None
    best_p = -1
    best_d = 100000000.0
    for c in cands:
        pri = c["priority_used"]
        dist = c["distance_squared"]
        if pri is None or dist is None:
            raise ValueError("refusing to rank a candidate with a missing key")
        if pri < best_p:
            continue
        if pri == best_p and not (dist < best_d):
            continue
        best = c
        best_p = pri
        best_d = dist
    return best


def explain_winner(cands: list[dict], winner: dict | None) -> str:
    if not cands:
        return "no overlapping eligible lemming"
    if winner is None:
        return "no candidate beat the original sentinels"
    pris = [c["priority_used"] for c in cands]
    if len(set(pris)) > 1:
        top = max(pris)
        tied = [c for c in cands if c["priority_used"] == top]
        if len(tied) == 1:
            return "priorities differ; the higher priority wins and distance was not the decider"
        dists = [c["distance_squared"] for c in tied]
        if all(d == dists[0] for d in dists):
            return ("priorities differ; among the equal highest priority the distances "
                    "tie and the earlier actor-list index wins")
        return ("priorities differ; lower priorities are eliminated and the strict "
                "smaller squared distance selects among the highest")
    dists = [c["distance_squared"] for c in cands]
    if all(d == dists[0] for d in dists):
        return "priorities match; distances tie; the earlier actor-list index wins"
    return "priorities match and the strict smaller squared distance decides"


def _self_test_winner() -> None:
    a = {"index": 2, "priority_used": 0, "distance_squared": 5.0}
    b = {"index": 6, "priority_used": 0, "distance_squared": 3.0}
    if compute_winner([a, b])["index"] != 6:
        raise AssertionError("smaller distance should win an equal priority")
    tied = {"index": 6, "priority_used": 0, "distance_squared": 5.0}
    if compute_winner([a, tied])["index"] != 2:
        raise AssertionError("an exact tie should keep the earlier index")
    higher = {"index": 2, "priority_used": 1, "distance_squared": 100.0}
    if compute_winner([higher, b])["index"] != 2:
        raise AssertionError("higher priority should beat a nearer lower priority")
    if compute_winner([b, higher])["index"] != 2:
        raise AssertionError("higher priority later in the list should still win")
    try:
        compute_winner([{"index": 1, "priority_used": None, "distance_squared": 1.0}])
    except ValueError:
        pass
    else:
        raise AssertionError("a missing priority must not be ranked")


def lemming_tie_fields(h, cand: dict, exe: bytes, ref) -> dict:
    ptr = cand["pointer_int"]
    state = u32(h, ptr + A.OFF_STATE)
    state_vt = u32(h, state) if state else 0
    rec = {
        "index": cand["index"],
        "pointer": f"{ptr:#010x}",
        "lemming_plus_0x340": None if state is None else f"{state:#010x}",
        "state_vtable": None if not state_vt else f"{state_vt:#010x}",
        "eligible": cand["eligible"],
        "box": cand["box"],
        "byte_0x29": cand["byte_0x29"],
        "byte_0x370": cand["byte_0x370"],
        "state_plus_0x14": None,
        "state_plus_0x14_i32": None,
        "priority_address": None,
        "priority_dword_live": None,
        "priority_dword_file": None,
        "priority_dword_live_matches_file": None,
        "priority_rule": None,
        "priority_used": None,
        "lemming_plus_8": None,
        "lemming_plus_8_hex": None,
        "distance_squared": None,
        "distance_squared_repr": None,
    }
    vals, raw = read_three_doubles(h, ptr + 8)
    shown, hx = dbl_pack(vals, raw)
    rec["lemming_plus_8"] = shown
    rec["lemming_plus_8_hex"] = hx
    if state == 0:
        rec["priority_rule"] = "state pointer is 0; the 0x004550AB call is skipped; priority local stays 0"
        rec["priority_used"] = 0
    elif state is None:
        rec["priority_rule"] = "state pointer unreadable; priority not substituted"
    else:
        raw_idx = read_mem(h, state + 0x14, 4)
        if not raw_idx or len(raw_idx) != 4:
            rec["priority_rule"] = "state+0x14 unreadable; priority not substituted"
        else:
            index_u = struct.unpack("<I", raw_idx)[0]
            index_s = struct.unpack("<i", raw_idx)[0]
            rec["state_plus_0x14"] = f"{index_u:#010x}"
            rec["state_plus_0x14_i32"] = index_s
            addr = table_va(index_u)
            rec["priority_address"] = f"{addr:#010x}"
            live = i32(h, addr)
            file_value, _off, _sec, _base = file_i32(exe, addr)
            rec["priority_dword_live"] = live
            rec["priority_dword_file"] = file_value
            if live is None:
                rec["priority_rule"] = "live priority dword unreadable; priority not substituted"
            else:
                rec["priority_dword_live_matches_file"] = live == file_value
                rec["priority_rule"] = "signed dword at 0x004C05D0 + 4 * [state+0x14]"
                rec["priority_used"] = live
    if vals is not None and ref is not None and rec["priority_used"] is not None:
        dist = distance_squared(vals, ref)
        rec["distance_squared"] = dist
        rec["distance_squared_repr"] = repr(dist)
    return rec


def snapshot(h, exe: bytes):
    scr = A.screen(h)
    if not scr:
        return None, "screen pointer missing"
    pos = A.cursor(h, scr)
    if pos is None:
        return None, "cursor unreadable"
    cx, cy = pos
    cursor_box, source, raw_doubles, sprite = P.cursor_box_live(h, scr, cx, cy)
    ref_vals, ref_raw = read_three_doubles(h, scr + 0x58)
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
        ref_vals, ref_raw = read_three_doubles(h, scr + 0x58)
        pos = A.cursor(h, scr)
        if pos is None:
            return None, "cursor unreadable on retry"
        cx, cy = pos
        cursor_box, source, raw_doubles, sprite = P.cursor_box_live(h, scr, cx, cy)
    if hover:
        try:
            original_index = ptrs.index(hover)
        except ValueError:
            original_index = "not-in-list"
    else:
        original_index = None
    ref_shown, ref_hex = dbl_pack(ref_vals, ref_raw)
    detailed = [lemming_tie_fields(h, c, exe, ref_vals) for c in cands]
    hits = []
    for c in detailed:
        if c["eligible"] and P.overlaps(c["box"], cursor_box):
            hits.append(c)
    filters = (
        P.read_byte(h, scr + A.OFF_DIR_FILTER_L),
        P.read_byte(h, scr + A.OFF_DIR_FILTER_R),
    )
    row = {
        "cursor_x": repr(cx),
        "cursor_y": repr(cy),
        "cursor_box": cursor_box,
        "cursor_box_source": source,
        "cursor_box_doubles_repr": None if raw_doubles is None else [repr(v) for v in raw_doubles],
        "cursor_sprite": sprite,
        "cursor_mode": A.cursor_mode(h, scr),
        "screen": f"{scr:#010x}",
        "screen_plus_0x58": ref_shown,
        "screen_plus_0x58_hex": ref_hex,
        "original_pointer": f"{hover:#010x}" if hover else "0x00000000",
        "original_index": original_index,
        "hovered_button": A.hovered_button(h, scr),
        "facing_filter_16b1": filters[0],
        "facing_filter_16b2": filters[1],
        "widget_call_gate": i32(h, P.WIDGET_CALL_GATE),
        "level": SP.loaded_level(h),
        "overlapping_eligible": hits,
        "n_overlapping_eligible": len(hits),
        "n_eligible": sum(1 for c in detailed if c["eligible"]),
    }
    return row, None


def acceptance(box: dict) -> dict:
    """Integer cursor point that hits `box` for a 32x32 sprite at trunc(c)-16.

    The sprite size is the landed 32x32 at 0x0045578C. This only chooses where
    to aim. The kept row uses the live sprite box.
    """
    return {
        "left": box["left"] - 12,
        "right": box["right"] + 12,
        "top": box["top"] - 20,
        "bottom": box["bottom"] + 20,
    }


def intersect(a: dict, b: dict):
    rect = {
        "left": max(a["left"], b["left"]),
        "right": min(a["right"], b["right"]),
        "top": max(a["top"], b["top"]),
        "bottom": min(a["bottom"], b["bottom"]),
    }
    if rect["left"] <= rect["right"] and rect["top"] <= rect["bottom"]:
        return rect
    return None


def box_separation(a: dict, b: dict) -> tuple[int, int]:
    if a["right"] < b["left"]:
        dx = b["left"] - a["right"]
    elif b["right"] < a["left"]:
        dx = a["left"] - b["right"]
    else:
        dx = 0
    if a["bottom"] < b["top"]:
        dy = b["top"] - a["bottom"]
    elif b["bottom"] < a["top"]:
        dy = a["top"] - b["bottom"]
    else:
        dy = 0
    return dx, dy


def aim_points_for(rect: dict) -> list[tuple[float, float]]:
    cx = (rect["left"] + rect["right"]) / 2.0
    cy = (rect["top"] + rect["bottom"]) / 2.0
    return [
        (cx, cy),
        (float(rect["left"]), float(rect["top"])),
        (float(rect["right"]), float(rect["bottom"])),
        (float(rect["left"]), float(rect["bottom"])),
        (float(rect["right"]), float(rect["top"])),
    ]


def point_blocked_by_panel(x: float, y: float, panels: list[dict]) -> bool:
    fake = {
        "left": int(x) - 16,
        "top": int(y) - 16,
        "right": int(x) + 16,
        "bottom": int(y) + 16,
    }
    return P.on_panel(x, y, fake, panels, -1)


def already_aimed(x: float, y: float, tried: list[tuple[float, float]]) -> bool:
    return any(abs(x - px) < 6.0 and abs(y - py) < 6.0 for px, py in tried)


def choose_aim(cands: list[dict], panels: list[dict], tried: list[tuple[float, float]]):
    eligible = [c for c in cands if c["eligible"]]
    ranked = []
    closest = None
    for i in range(len(eligible)):
        for j in range(i + 1, len(eligible)):
            a = eligible[i]
            b = eligible[j]
            dx, dy = box_separation(a["box"], b["box"])
            gap = max(dx, dy)
            if closest is None or gap < closest:
                closest = gap
            rect = intersect(acceptance(a["box"]), acceptance(b["box"]))
            if rect is not None:
                ranked.append((1, (rect["right"] - rect["left"] + 1) * (rect["bottom"] - rect["top"] + 1), rect))
            elif gap <= 48:
                ax = (a["box"]["left"] + a["box"]["right"]) / 2.0
                ay = (a["box"]["top"] + a["box"]["bottom"]) / 2.0
                bx = (b["box"]["left"] + b["box"]["right"]) / 2.0
                by = (b["box"]["top"] + b["box"]["bottom"]) / 2.0
                ranked.append((0, -gap, {
                    "left": int((ax + bx) / 2.0),
                    "right": int((ax + bx) / 2.0),
                    "top": int((ay + by) / 2.0),
                    "bottom": int((ay + by) / 2.0),
                }))
    ranked.sort(key=lambda item: (item[0], item[1]), reverse=True)
    for _kind, _area, rect in ranked:
        for x, y in aim_points_for(rect):
            if not (8.0 <= x <= 632.0 and 8.0 <= y <= 460.0):
                continue
            if point_blocked_by_panel(x, y, panels):
                continue
            if already_aimed(x, y, tried):
                continue
            return x, y, closest
    return None


def aim_and_read(h, pid: int, exe: bytes, x: float, y: float):
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


def public_lemming(c: dict) -> dict:
    """Raw fields the facilitator recomputes from. No substituted priority."""
    return {
        "index": c["index"],
        "pointer": c["pointer"],
        "lemming_plus_0x340": c["lemming_plus_0x340"],
        "state_vtable": c["state_vtable"],
        "state_plus_0x14": c["state_plus_0x14"],
        "state_plus_0x14_i32": c["state_plus_0x14_i32"],
        "priority_address": c["priority_address"],
        "priority_dword_live": c["priority_dword_live"],
        "priority_dword_file": c["priority_dword_file"],
        "priority_dword_live_matches_file": c["priority_dword_live_matches_file"],
        "priority_rule": c["priority_rule"],
        "lemming_plus_8": c["lemming_plus_8"],
        "lemming_plus_8_hex": c["lemming_plus_8_hex"],
        "box": c["box"],
        "byte_0x29": c["byte_0x29"],
        "byte_0x370": c["byte_0x370"],
    }


def rank_hits(hits: list[dict]):
    missing = [c["index"] for c in hits if c["priority_used"] is None or c["distance_squared"] is None]
    if missing:
        return None, f"missing key on actor-list index {missing}; winner not computed"
    winner = compute_winner(hits)
    return winner, explain_winner(hits, winner)


def overlap_payload(row: dict, cursors_tried: int, elapsed: float, attempts: list[dict]) -> dict:
    hits = row["overlapping_eligible"]
    winner, explanation = rank_hits(hits)
    winner_index = None if winner is None else winner["index"]
    pris = [c["priority_used"] for c in hits if c["priority_used"] is not None]
    priorities_differ = len(set(pris)) > 1 if pris else None
    payload = {
        "result": "overlap",
        "cursor": {
            "x": row["cursor_x"],
            "y": row["cursor_y"],
            "box": row["cursor_box"],
            "box_source": row["cursor_box_source"],
            "sprite": row["cursor_sprite"],
        },
        "original_pointer": row["original_pointer"],
        "original_index": row["original_index"],
        "screen": row["screen"],
        "screen_plus_0x58": row["screen_plus_0x58"],
        "screen_plus_0x60_and_0x68_follow_in_the_three_doubles": True,
        "screen_plus_0x58_hex": row["screen_plus_0x58_hex"],
        "facing_filter_16b1": row["facing_filter_16b1"],
        "facing_filter_16b2": row["facing_filter_16b2"],
        "hovered_button": row["hovered_button"],
        "level": row["level"],
        "lemmings": [public_lemming(c) for c in hits],
        "worker_computed": {
            "formula": (
                "priority is the signed live dword, or 0 when [lemming+0x340] is 0; "
                "distance is the sum of squares of the three (lemming+8 minus screen+0x58) "
                "component subtractions; higher priority wins; a strict smaller distance "
                "wins only on an equal priority; an exact tie keeps the earlier actor-list index. "
                "Facing filters and the skill-stock gate are not applied."
            ),
            "per_lemming": [
                {
                    "index": c["index"],
                    "pointer": c["pointer"],
                    "priority_used": c["priority_used"],
                    "distance_squared": c["distance_squared_repr"],
                }
                for c in hits
            ],
            "winner_index": winner_index,
            "winner_pointer": None if winner is None else winner["pointer"],
            "matches_original_index": winner_index == row["original_index"],
            "priorities_differ": priorities_differ,
            "explanation": explanation,
        },
        "cursors_tried": cursors_tried,
        "seconds_after_two_eligible": elapsed,
        "attempts": attempts,
    }
    return payload


def rejected_reason(row: dict, panels: list[dict], widgets: list[dict]) -> str | None:
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


def placed_cursors(attempts: list[dict]) -> int:
    return sum(1 for attempt in attempts if "overlapping_eligible" in attempt)


def search_overlap(h, pid: int, exe: bytes):
    mode = A.cursor_mode(h)
    say(f"  cursor_mode {mode} before search")
    if mode != 2:
        return {
            "result": "aim-not-sent",
            "reason": f"cursor_mode {mode} is not 2; mouse input would write screen+0x58; no cursor was sent",
            "cursors_tried": 0,
        }, f"cursor_mode {mode}"
    start = time.time()
    attempts = []
    tried_points: list[tuple[float, float]] = []
    fallback = None
    idle_note = "no pair came within the aim window"
    while placed_cursors(attempts) < MAX_CURSORS and (time.time() - start) < SEARCH_SECONDS:
        _ptrs, cands, unread = P.lemming_candidates(h)
        if unread:
            say(f"  lemming fields unreadable {unread}; not aiming")
            time.sleep(0.4)
            continue
        panels = P.panel_boxes(h)
        chosen = choose_aim(cands, panels, tried_points)
        if chosen is None:
            time.sleep(0.4)
            continue
        x, y, closest = chosen
        n = len(attempts) + 1
        say(f"  cursor {n} aim ({x:.1f}, {y:.1f}) closest_gap {closest} "
            f"elapsed {time.time() - start:.1f}s")
        try:
            row, err, fg, pos = aim_and_read(h, pid, exe, x, y)
        except Exception as exc:
            say(f"    aim failed: {exc!r}")
            attempts.append({
                "n": n, "target_x": repr(x), "target_y": repr(y),
                "error": repr(exc), "counted": False,
            })
            if "cursor_mode" in repr(exc):
                break
            continue
        if err and "cursor_mode" in err:
            say(f"    {err}")
            return {
                "result": "aim-not-sent",
                "reason": err,
                "cursors_tried": placed_cursors(attempts),
                "seconds_after_two_eligible": time.time() - start,
                "attempts": attempts,
            }, err
        if err or row is None:
            attempts.append({
                "n": n, "target_x": repr(x), "target_y": repr(y),
                "error": err, "counted": False,
            })
            say(f"    not a cursor read: {err}")
            time.sleep(0.3)
            continue
        tried_points.append((float(row["cursor_x"]), float(row["cursor_y"])))
        attempts.append({
            "n": n,
            "target_x": repr(x),
            "target_y": repr(y),
            "cursor_x": row["cursor_x"],
            "cursor_y": row["cursor_y"],
            "overlapping_eligible": row["n_overlapping_eligible"],
            "eligible": row["n_eligible"],
            "box_source": row["cursor_box_source"],
            "foreground": None if fg is None else fg.get("method"),
            "aim": None if pos is None else [repr(pos[0]), repr(pos[1])],
        })
        widgets, _notes = P.widget_boxes(h)
        panels = P.panel_boxes(h)
        reason = rejected_reason(row, panels, widgets)
        attempts[-1]["rejected"] = reason
        say(f"    overlaps {row['n_overlapping_eligible']} original {row['original_index']} "
            f"source {row['cursor_box_source']} rejected {reason}")
        if reason is not None or row["n_overlapping_eligible"] < 2:
            continue
        payload = overlap_payload(row, placed_cursors(attempts), time.time() - start, attempts)
        if row["n_overlapping_eligible"] == 2:
            return payload, None
        if fallback is None:
            fallback = payload
            say("    saved a more-than-two overlap and kept looking for exactly two")
    elapsed = time.time() - start
    if fallback is not None:
        fallback["cursors_tried"] = placed_cursors(attempts)
        fallback["seconds_after_two_eligible"] = elapsed
        fallback["attempts"] = attempts
        fallback["note"] = "no cursor had exactly two overlapping eligible lemmings; this cursor had more"
        return fallback, None
    return {
        "result": "no-overlap",
        "cursors_tried": placed_cursors(attempts),
        "seconds_after_two_eligible": elapsed,
        "search_seconds_cap": SEARCH_SECONDS,
        "cursor_cap": MAX_CURSORS,
        "note": idle_note if not attempts else "cursors were placed; none had two overlapping eligible boxes",
        "attempts": attempts,
    }, None


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
        f"hashes_match {payload.get('hashes_match')}",
        f"hashes_before {payload.get('hashes_before')}",
        f"hashes_after {payload.get('hashes_after')}",
        f"failure {payload.get('failure')}",
        f"result {payload.get('result')}",
    ]
    (OUT / "restoration.log").write_bytes(("\n".join(lines) + "\n").encode("utf-8"))


def run_check_file() -> int:
    data = STOCK_EXE.read_bytes()
    digest = P.sha256_file(STOCK_EXE)
    info = check_file_table(data)
    _self_test_winner()
    say(f"exe sha256 {digest}")
    say(f"file table {json.dumps(info)}")
    if digest != STOCK_SHA256:
        say("error: installed exe does not match the authority hash")
        return 2
    say("file-table check ok; winner self-test ok")
    return 0


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


def run_live_body() -> int:
    if not PREDICTION.exists():
        say("error: prediction.md is missing; refusing to launch")
        return 2
    if not WINDOWED_CONF.exists():
        say(f"error: windowed config missing: {WINDOWED_CONF}")
        return 2
    try:
        _self_test_winner()
    except Exception as exc:
        say(f"error: winner self-test failed: {exc!r}")
        return 2
    exe = STOCK_EXE.read_bytes()
    try:
        table_info = check_file_table(exe)
    except Exception as exc:
        say(f"error: file table check failed: {exc!r}")
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
            "result": "not-launched",
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
    result_name = None
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
            result_name = "prediction-not-older"
            return 2
        time.sleep(12.0)
        handle, attached = attach(write=True)
        if not handle or attached != pid:
            failure = f"attach failed (handle={bool(handle)} attached={attached} owned={pid})"
            result_name = "attach-failed"
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
            result_name = "menu-not-reached"
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
            result_name = "menu-scan-failed"
            return 2
        if enter_rc != 0 or state != 0 or "MM_CLIMB_002" not in (loaded or ""):
            failure = (
                f"level entry failed rc={enter_rc} ui_state={state} loaded={loaded!r} "
                f"candidates={probe.get('candidates')} return_sent={probe.get('return_sent')}"
            )
            result_name = "level-not-reached"
            return 2

        wait_deadline = time.time() + ELIGIBLE_WAIT_SECONDS
        eligible_count = 0
        while time.time() < wait_deadline:
            _ptrs, cands, _unread = P.lemming_candidates(handle)
            eligible_count = sum(1 for c in cands if c["eligible"])
            say(f"  waiting lemmings: {len(cands)} boxed, {eligible_count} eligible, "
                f"released {i32(handle, SP.RELEASED)}")
            if eligible_count >= 2:
                break
            time.sleep(1.0)
        if eligible_count < 2:
            failure = f"level reached; eligible lemmings stayed {eligible_count}"
            result_name = "fewer-than-two-eligible"
            dump_json(OUT / "overlap.json", {
                "result": "fewer-than-two-eligible",
                "eligible": eligible_count,
                "cursors_tried": 0,
                "level": SP.loaded_level(handle),
            })
            return 4

        payload, search_failure = search_overlap(handle, pid, exe)
        result_name = payload.get("result")
        dump_json(OUT / "overlap.json", payload)
        if search_failure:
            failure = search_failure
            return 4
        say(f"  search result {result_name}")
        return 0
    except Exception as exc:
        failure = f"exception: {exc!r}"
        result_name = result_name or "exception"
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
            "result": result_name,
            "menu_candidates": None if probe is None else probe.get("candidates"),
            "return_sent": None if probe is None else probe.get("return_sent"),
            "menu_presses": None if probe is None else probe.get("presses"),
            "file_table": table_info,
        })
        if launched and not (OUT / "overlap.json").exists():
            dump_json(OUT / "overlap.json", {
                "result": result_name or "not-measured",
                "failure": failure,
                "cursors_tried": 0,
            })
        if failure:
            say(f"FAILED: {failure}")
            (OUT / "FAILED.txt").write_bytes((failure + "\n").encode("utf-8"))


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
    ap.add_argument("--check-file", action="store_true")
    args = ap.parse_args()
    if args.check_file and not args.run_live:
        return run_check_file()
    if not args.run_live:
        print("pass --run-live for the one authorized capture, or --check-file")
        return 0
    return run_live()


if __name__ == "__main__":
    raise SystemExit(main())
