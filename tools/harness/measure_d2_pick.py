"""D2b: 20 live cursor positions on MM_CLIMB_002 versus pickHoveredLemming.

One Popen of Lemmings Revolution.exe. If a Lemmings process is already
running, or that launch fails, this exits without a second start.

The pick call is the landed port (HEAD snapshot compiled into
build/tmp/d2b-pick-capture/pick_driver.exe). This file does not edit
assign_skill.py or measure_d1_release_trace.py; it imports the cursor,
hover, lemming-box and aim helpers and copies the one-Popen launch shape.

No skill click, no lemming write, no camera write, no debugger.
"""
from __future__ import annotations

import argparse
import hashlib
import json
import os
import struct
import subprocess
import sys
import time
from pathlib import Path

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))

from acquire_game_foreground import acquire, foreground_pid  # noqa: E402
from lrinput import DIK, LRInput  # noqa: E402
from lrstate import attach, i32, read_mem, u32, ui_state  # noqa: E402
from memdump import find_pids, k32  # noqa: E402
import assign_skill as A  # noqa: E402
import measure_spawn as SP  # noqa: E402
import measure_steppers as S  # noqa: E402

ROOT = Path(__file__).resolve().parents[2]
OUT = ROOT / "build" / "tmp" / "d2b-pick-capture"
DRIVER = OUT / "pick_driver.exe"
PREDICTION = OUT / "prediction.md"
REPAIR_PREDICTION = OUT / "repair-prediction.md"
STOCK_EXE = Path(r"C:\progs\Lemmings Revolution\Lemmings Revolution.exe")
STOCK_SHA256 = "32675bdb49866e76502713251fdfd2b09078c2481c72ca22ff68ebf4b3737a2f"
INSTALL = STOCK_EXE.parent
WINDOWED_CONF = ROOT / "build" / "tmp" / "a1-zoom-elevation" / "dgVoodoo.windowed.conf"
UCRT_BIN = Path(r"C:\progs\msys2\ucrt64\bin")

# Widget-list screen point, read off 0x0045634E / 0x0040EB50 / 0x0040EAE0.
# matrix = [[[widget+0x44]+0xB4] + 8] + 0x48
# x,y,w = M * v + t, v at 0x0058D3D8, t at +0x48
# sx = dword[0x0058D294] + trunc(x/w); sy = dword[0x0058D298] - trunc(y/w)
# The hit box passed to 0x00454CB9 is that point expanded by 0x14.
VIEW_VEC = 0x0058D3D8
CENTER_X = 0x0058D294
CENTER_Y = 0x0058D298
WIDGET_LISTS = (
    (0x004C88F8, True),   # also requires word [obj+0x98] > 0
    (0x004C8908, False),
)
WIDGET_CALL_GATE = 0x004CBF88  # caller at 0x004537E4 pushes 1 when this dword > 0
LEMMING_VTS = (SP.LEMMING_VT, SP.OTHER_VT)


def sha256_file(path: Path):
    if not path.exists():
        return None
    return hashlib.sha256(path.read_bytes()).hexdigest()


def install_hashes() -> dict:
    names = ("Lemmings Revolution.exe", "dgVoodoo.conf", "SavedGame.dat")
    return {n: sha256_file(INSTALL / n) for n in names}


def trunc_i32(value: float) -> int:
    """MSVC (int)double via 0x0049C5AC: round toward zero, result in int32."""
    if value != value or value in (float("inf"), float("-inf")):
        raise ValueError(f"non-finite {value}")
    n = int(value)
    if n < -2147483648 or n > 2147483647:
        raise ValueError(f"out of int32 {n}")
    return n


def read_doubles(h, addr: int, n: int):
    raw = read_mem(h, addr, 8 * n)
    if not raw or len(raw) != 8 * n:
        return None
    return list(struct.unpack("<" + "d" * n, raw))


def read_byte(h, addr: int):
    raw = read_mem(h, addr, 1)
    if not raw or len(raw) != 1:
        return None
    return raw[0]


def vector_ptrs(h, list_addr: int) -> list[int]:
    count = i32(h, list_addr)
    arr = u32(h, list_addr + 0x0C)
    if count is None or arr is None or not 0 < count <= 400 or arr < 0x10000:
        return []
    raw = read_mem(h, arr, count * 4)
    if not raw or len(raw) != count * 4:
        return []
    return list(struct.unpack(f"<{count}I", raw))


def box_from_doubles(quad):
    if quad is None:
        return None
    try:
        left, top, right, bottom = (trunc_i32(v) for v in quad)
    except ValueError:
        return None
    return {"left": left, "top": top, "right": right, "bottom": bottom}


def overlaps(box, cursor) -> bool:
    """Same four compares as pickBoxOverlaps. Used only to reject UI hits."""
    return (
        box["left"] <= cursor["right"] - 4
        and box["right"] >= cursor["left"] + 4
        and box["top"] <= cursor["bottom"] + 4
        and box["bottom"] >= cursor["top"] - 4
    )


def project_widget(h, obj: int, require_word98: bool):
    """Screen hit-box of one widget, or a reason string when it is inactive
    or the pointer chain cannot be read."""
    flag = read_byte(h, obj + 0x29)
    if flag is None:
        return None, "byte+0x29 unreadable"
    if flag == 0:
        return None, "inactive"
    if require_word98:
        raw = read_mem(h, obj + 0x98, 2)
        if not raw or len(raw) != 2:
            return None, "word+0x98 unreadable"
        word = struct.unpack("<h", raw)[0]
        if word <= 0:
            return None, "word+0x98<=0"
    inner = u32(h, obj + 0x44)
    if not inner or inner < 0x10000:
        return None, "inner pointer missing"
    link = u32(h, inner + 0xB4)
    if not link or link < 0x10000:
        return None, "matrix link missing"
    matrix = read_doubles(h, link + 8 + 0x48, 9)
    view = read_doubles(h, VIEW_VEC, 3)
    trans = read_doubles(h, VIEW_VEC + 0x48, 3)
    if matrix is None or view is None or trans is None:
        return None, "matrix or view unreadable"
    x = matrix[0] * view[0] + matrix[3] * view[1] + matrix[6] * view[2] + trans[0]
    y = matrix[1] * view[0] + matrix[4] * view[1] + matrix[7] * view[2] + trans[1]
    w = matrix[2] * view[0] + matrix[5] * view[1] + matrix[8] * view[2] + trans[2]
    if w == 0.0 or abs(w) < 1e-12:
        return None, "w~0"
    cx = i32(h, CENTER_X)
    cy = i32(h, CENTER_Y)
    if cx is None or cy is None:
        return None, "viewport center unreadable"
    try:
        sx = cx + trunc_i32(x / w)
        sy = cy - trunc_i32(y / w)
    except ValueError as exc:
        return None, f"trunc {exc}"
    box = {
        "left": sx - 0x14,
        "top": sy - 0x14,
        "right": sx + 0x14,
        "bottom": sy + 0x14,
        "sx": sx,
        "sy": sy,
    }
    return box, "ok"


def widget_boxes(h) -> tuple[list[dict], list[dict]]:
    boxes = []
    notes = []
    for list_addr, require_word in WIDGET_LISTS:
        ptrs = vector_ptrs(h, list_addr)
        notes.append({"list": f"{list_addr:#010x}", "count": len(ptrs)})
        for obj in ptrs:
            box, reason = project_widget(h, obj, require_word)
            rec = {"list": f"{list_addr:#010x}", "obj": f"{obj:#010x}", "reason": reason}
            if box is not None:
                rec.update(box)
                if abs(box["sx"]) <= 5000 and abs(box["sy"]) <= 5000:
                    boxes.append(box)
                else:
                    rec["reason"] = "projected far outside the screen"
            notes.append(rec)
    return boxes, notes


def panel_boxes(h) -> list[dict]:
    out = []
    for i in range(A.PANEL_COUNT):
        quad = A.panel_box(h, i)
        box = box_from_doubles(quad) if quad else None
        if box is not None:
            box["index"] = i
            out.append(box)
    return out


def cursor_box_live(h, scr: int, cx: float, cy: float):
    sprite = u32(h, scr + A.OFF_CURSOR_SPRITE)
    raw = None
    if sprite and sprite >= 0x10000:
        raw = A.box_of(h, sprite + 0x10)
    box = box_from_doubles(raw) if raw else None
    if box is not None:
        return box, "live-sprite", list(raw), f"{sprite:#010x}"
    left = trunc_i32(cx) - 16
    top = trunc_i32(cy) - 16
    synth = {"left": left, "top": top, "right": left + 32, "bottom": top + 32}
    return synth, "synthesized", None, f"{sprite:#010x}" if sprite else "0x0"


def lemming_candidates(h):
    ptrs = SP.actor_ptrs(h)
    cands = []
    unread = []
    for index, ptr in enumerate(ptrs):
        vt = u32(h, ptr)
        if vt not in LEMMING_VTS:
            continue
        quad = A.lemming_box(h, ptr)
        b29 = read_byte(h, ptr + A.OFF_PICKABLE)
        b370 = read_byte(h, ptr + A.OFF_PICKABLE2)
        direction_raw = read_mem(h, ptr + A.OFF_DIR, 4)
        state = u32(h, ptr + A.OFF_STATE)
        state_vt = u32(h, state) if state else 0
        if quad is None or b29 is None or b370 is None or direction_raw is None:
            unread.append(f"{ptr:#010x}")
            continue
        box = box_from_doubles(quad)
        if box is None:
            unread.append(f"{ptr:#010x}")
            continue
        cands.append({
            "index": index,
            "pointer": f"{ptr:#010x}",
            "pointer_int": ptr,
            "box": box,
            "box_doubles": [quad[0], quad[1], quad[2], quad[3]],
            "eligible": b29 != 0 and b370 != 0,
            "byte_0x29": b29,
            "byte_0x370": b370,
            "direction": struct.unpack("<i", direction_raw)[0],
            "state_vt": f"{state_vt:#010x}" if state_vt else "0x0",
            "priority": 0,
            "distance_squared": 0.0,
        })
    return ptrs, cands, unread


def call_driver(cursor_box: dict, cands: list[dict]) -> tuple:
    lines = [
        "cursor {left} {top} {right} {bottom}".format(**cursor_box)
    ]
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
    overlaps_ids = []
    for line in text.splitlines():
        if line.startswith("pick "):
            tok = line.split()[1]
            pick_id = "none" if tok == "none" else int(tok)
        elif line.startswith("overlaps"):
            parts = line.split()[1:]
            overlaps_ids = [int(p) for p in parts]
    if pick_id is None:
        raise RuntimeError(f"pick_driver produced no pick line: {text!r}")
    return pick_id, overlaps_ids


def on_panel(cx: float, cy: float, cursor_box: dict, panels: list[dict], hover_btn) -> bool:
    if hover_btn != -1:
        return True
    for box in panels:
        if overlaps(box, cursor_box):
            return True
        if (box["left"] - 2) <= cx <= (box["right"] + 2) and (box["top"] - 2) <= cy <= (box["bottom"] + 2):
            return True
    return False


def on_widget(cursor_box: dict, widgets: list[dict]) -> bool:
    for box in widgets:
        if overlaps(box, cursor_box):
            return True
    return False


def accept_class(kept: list[dict], is_on: bool) -> bool:
    if len(kept) >= 20:
        return False
    on = sum(1 for row in kept if row["original_index"] != "none")
    none = len(kept) - on
    slots = 20 - len(kept)
    if is_on and none < 5 and slots <= (5 - none):
        return False
    if (not is_on) and on < 5 and slots <= (5 - on):
        return False
    return True


def public_candidate(c: dict) -> dict:
    return {
        "index": c["index"],
        "pointer": c["pointer"],
        "box": c["box"],
        "box_doubles": c["box_doubles"],
        "eligible": c["eligible"],
        "byte_0x29": c["byte_0x29"],
        "byte_0x370": c["byte_0x370"],
        "direction": c["direction"],
        "state_vt": c["state_vt"],
        "priority": 0,
        "distance_squared": 0.0,
    }


def snapshot(h, scr: int):
    pos = A.cursor(h, scr)
    if pos is None:
        return None, "cursor unreadable"
    cx, cy = pos
    cursor_box, source, raw_doubles, sprite = cursor_box_live(h, scr, cx, cy)
    hover = A.hovered(h, scr)
    ptrs, cands, unread = lemming_candidates(h)
    if unread:
        return None, f"unreadable lemming fields {unread}"
    hover_again = A.hovered(h, scr)
    if hover_again != hover:
        hover = hover_again
    if hover:
        try:
            original_index = ptrs.index(hover)
        except ValueError:
            original_index = "not-in-list"
    else:
        original_index = "none"
    pick_id, overlap_ids = call_driver(cursor_box, cands)
    by_index = {c["index"]: c for c in cands}
    if pick_id == "none":
        pick_pointer = "none"
    else:
        picked = by_index.get(pick_id)
        pick_pointer = picked["pointer"] if picked else "missing-candidate"
    filters = (
        read_byte(h, scr + A.OFF_DIR_FILTER_L),
        read_byte(h, scr + A.OFF_DIR_FILTER_R),
    )
    row = {
        "cursor_x": cx,
        "cursor_y": cy,
        "cursor_box": cursor_box,
        "cursor_box_source": source,
        "cursor_box_doubles": raw_doubles,
        "cursor_sprite": sprite,
        "original_pointer": f"{hover:#010x}" if hover else "0x0",
        "original_index": original_index,
        "pick_id": pick_id,
        "pick_pointer": pick_pointer,
        "hovered_button": A.hovered_button(h, scr),
        "facing_filter_16b1": filters[0],
        "facing_filter_16b2": filters[1],
        "selected_skill": i32(h, A.SELECTED),
        "widget_call_gate": i32(h, WIDGET_CALL_GATE),
        "overlaps": overlap_ids,
        "n_overlapping": len(overlap_ids),
        "priority": 0,
        "distance_squared": 0.0,
        "candidates": [public_candidate(c) for c in cands],
        "actor_list": [f"{p:#010x}" for p in ptrs],
        "level": SP.loaded_level(h),
    }
    row["agree"] = row["original_index"] == row["pick_id"]
    return row, None


def aim_and_snap(h, pid: int, x: float, y: float):
    fg = acquire(pid)
    with LRInput() as inp:
        inp.clear()
        inp.enable()
        pos = A.aim_at(h, inp, x, y, tol=2.0, tries=16)
        inp.move(0, 0)
        time.sleep(0.25)
        scr = A.screen(h)
        if not scr:
            return None, "screen pointer missing after aim", fg, pos
        row, err = snapshot(h, scr)
        inp.release()
    return row, err, fg, pos


def lemming_aim_points(cands: list[dict]) -> list[tuple[float, float]]:
    points = []
    for c in cands:
        if not c["eligible"]:
            continue
        x0, y0, x1, y1 = c["box_doubles"]
        cx = (x0 + x1) / 2.0
        cy = (y0 + y1) / 2.0
        if 90.0 <= cx <= 620.0 and 20.0 <= cy <= 420.0:
            points.append((cx, cy))
    return points


def empty_aim_points() -> list[tuple[float, float]]:
    points = []
    for y in (48, 96, 144, 192, 240, 288, 336, 384):
        for x in (130, 210, 290, 370, 450, 530, 600):
            points.append((float(x), float(y)))
    for y in (70, 160, 250, 340):
        for x in (160, 320, 480):
            points.append((float(x), float(y)))
    return points


def write_jsonl(path: Path, rows: list[dict]) -> None:
    text = "".join(json.dumps(row, allow_nan=False) + "\n" for row in rows)
    path.write_bytes(text.encode("utf-8"))


def _count_changed_bytes(before: dict, after: dict) -> int:
    """Byte differences between two diffscan.snap dictionaries."""
    total = 0
    for base, old in before.items():
        new = after.get(base)
        if not old or not new or len(old) != len(new) or old == new:
            continue
        mv_old = memoryview(old)
        mv_new = memoryview(new)
        start = 0
        n = len(mv_old)
        while start < n:
            end = min(start + 65536, n)
            if mv_old[start:end] != mv_new[start:end]:
                total += sum(a != b for a, b in zip(mv_old[start:end], mv_new[start:end]))
            start = end
    return total


def _press_down(inp) -> dict:
    """Same hold as find_menu_index's Down tap: 20 polled frames, then release."""
    k0 = inp.key_frame
    m0 = inp.mouse_frame
    inp.key(DIK["DOWN"], True)
    at_down = foreground_pid()
    held_ok = inp.wait_frames(20)
    key_while_held = inp.key_frame - k0
    mouse_while_held = inp.mouse_frame - m0
    after_hold = foreground_pid()
    inp.key(DIK["DOWN"], False)
    inp.wait_frames(1)
    return {
        "wait_ok": bool(held_ok),
        "key_frame_delta": inp.key_frame - k0,
        "mouse_frame_delta": inp.mouse_frame - m0,
        "key_frames_while_held": key_while_held,
        "mouse_frames_while_held": mouse_while_held,
        "foreground_at_down": at_down,
        "foreground_after_hold": after_hold,
    }


def _write_down_diff(probe: dict) -> None:
    presses = probe.get("presses") or []
    total = sum(p["bytes_changed"] for p in presses if isinstance(p.get("bytes_changed"), int))
    lines = [
        f"menu-index candidates: {probe.get('candidates')}",
        "Return was not sent." if not probe.get("return_sent") else "Return was sent.",
        "No cursor was sampled.",
        f"bytes_changed_sum: {total}",
        f"downs_sent: {sum(1 for p in presses if p.get('down_sent'))}",
    ]
    if probe.get("note"):
        lines.append(str(probe["note"]))
    addrs = probe.get("intersection_addrs") or []
    lines.append("intersection_addrs: " + (", ".join(addrs) if addrs else "none"))
    for rec in presses:
        def cell(key, rec=rec):
            value = rec.get(key)
            return "n/a" if value is None else str(value)
        lines.append(
            f"press {rec.get('n')}: down_sent={cell('down_sent')} "
            f"foreground_before={cell('foreground_before')} method={cell('method')} "
            f"foreground_after_print={cell('foreground_after_print')} "
            f"foreground_at_down={cell('foreground_at_down')} "
            f"foreground_after_hold={cell('foreground_after_hold')} "
            f"key_frames_while_held={cell('key_frames_while_held')} "
            f"key_frame_delta={cell('key_frame_delta')} "
            f"mouse_frame_delta={cell('mouse_frame_delta')} "
            f"bytes_changed={cell('bytes_changed')} "
            f"i32_in_0_4={cell('i32_in_0_4')} "
            f"mod5_this_press={cell('mod5_this_press')}"
        )
    for err in probe.get("acquire_errors") or []:
        lines.append(f"acquire_error: {err}")
    (OUT / "down-diff.txt").write_bytes(("\n".join(lines) + "\n").encode("utf-8"))


def begin_menu_probe(pid: int) -> dict:
    """Caller-side menu scan. Does not edit goto_level or restart_level.

    Acquire and print the foreground pid immediately before each Down, then
    send the key only when that pid is the game. Return is not sent here.
    """
    import restart_level
    import lrinput
    from diffscan import changed, snap
    from goto_level import SCREEN_ID
    from lrstate import i32, ui_state

    probe = {
        "presses": [],
        "candidates": None,
        "return_sent": False,
        "address": None,
        "note": "",
        "acquire_errors": [],
        "intersection_addrs": [],
    }
    orig_find = restart_level.find_menu_index
    orig_tap = lrinput.LRInput.tap

    def probed_find(h, inp, verbose=True):
        keep = None
        print(
            f"  menu scan start ui_state {ui_state(h)} screen {i32(h, SCREEN_ID)}",
            flush=True,
        )
        for n in range(1, 4):
            before = snap(h)
            sent = False
            method = None
            owner = None
            owner_after_print = None
            frames = None
            try:
                fg = acquire(pid)
                method = fg.get("method")
            except Exception as exc:
                probe["acquire_errors"].append(f"down {n}: {exc!r}")
                print(
                    f"  foreground acquire failed before Down {n}: {exc!r}; Down not sent",
                    flush=True,
                )
            else:
                owner = foreground_pid()
                print(
                    f"  foreground immediately before Down {n}: owner {owner} "
                    f"method {method} game {pid}",
                    flush=True,
                )
                owner_after_print = foreground_pid()
                if owner != pid:
                    print(f"  Down {n} not sent; foreground owner is not the game", flush=True)
                else:
                    frames = _press_down(inp)
                    sent = True
                    time.sleep(0.6)
            if sent:
                after = snap(h)
                moved = changed(before, after, lo=0, hi=4)
                step = {
                    k: v for k, v in moved.items() if v[1] == (v[0] + 1) % 5
                }
                print(f"  Down {n} counting changed bytes", flush=True)
                nbytes = _count_changed_bytes(before, after)
                keep = step if keep is None else {k: v for k, v in step.items() if k in keep}
                i32_n = len(moved)
                mod5_n = len(step)
            else:
                nbytes = None
                i32_n = None
                mod5_n = None
            rec = {
                "n": n,
                "down_sent": sent,
                "foreground_before": owner,
                "method": method,
                "foreground_after_print": owner_after_print,
                "foreground_at_down": None if frames is None else frames["foreground_at_down"],
                "foreground_after_hold": None if frames is None else frames["foreground_after_hold"],
                "key_frames_while_held": None if frames is None else frames["key_frames_while_held"],
                "key_frame_delta": None if frames is None else frames["key_frame_delta"],
                "mouse_frame_delta": None if frames is None else frames["mouse_frame_delta"],
                "bytes_changed": nbytes,
                "i32_in_0_4": i32_n,
                "mod5_this_press": mod5_n,
                "wait_ok": None if frames is None else frames["wait_ok"],
            }
            probe["presses"].append(rec)
            inter = 0 if keep is None else len(keep)
            print(
                f"  Down {n} down_sent {sent} bytes_changed {nbytes} "
                f"i32_in_0_4 {i32_n} mod5 {mod5_n} running_intersection {inter} "
                f"key_frames_while_held {rec['key_frames_while_held']}",
                flush=True,
            )
        downs_sent = sum(1 for p in probe["presses"] if p["down_sent"])
        if downs_sent == 3:
            probe["candidates"] = len(keep or {})
            probe["intersection_addrs"] = [
                f"{addr:#010x}" for addr in sorted(keep or {})[:8]
            ]
        else:
            probe["candidates"] = 0
            probe["note"] = f"three-press scan incomplete: downs_sent={downs_sent}"
            probe["intersection_addrs"] = []
        if verbose:
            print(f"  menu-index candidates: {probe['candidates']}", flush=True)
        if downs_sent != 3 or not keep:
            probe["address"] = None
            return None
        addr = sorted(keep)[0]
        probe["address"] = addr
        print(f"  menu index at {addr:#010x}", flush=True)
        return addr

    def tap_with_foreground(self, dik, hold_frames=2):
        try:
            fg = acquire(pid)
        except Exception as exc:
            probe["acquire_errors"].append(f"tap {dik}: {exc!r}")
            print(
                f"  foreground acquire failed before tap {dik}: {exc!r}; tap not sent",
                flush=True,
            )
            return False
        owner = foreground_pid()
        print(
            f"  foreground immediately before tap {dik}: owner {owner} "
            f"method {fg.get('method')} game {pid}",
            flush=True,
        )
        if owner != pid:
            print(f"  tap {dik} not sent; foreground owner is not the game", flush=True)
            return False
        if dik == DIK["RETURN"]:
            probe["return_sent"] = True
            print("  RETURN is being sent", flush=True)
        return orig_tap(self, dik, hold_frames=hold_frames)

    restart_level.find_menu_index = probed_find
    lrinput.LRInput.tap = tap_with_foreground
    probe["_restore"] = (restart_level, orig_find, lrinput, orig_tap)
    return probe


def end_menu_probe(probe: dict | None) -> None:
    if not probe or "_restore" not in probe:
        return
    restart_level, orig_find, lrinput, orig_tap = probe.pop("_restore")
    restart_level.find_menu_index = orig_find
    lrinput.LRInput.tap = orig_tap


def run_self_test() -> int:
    if not DRIVER.exists():
        print(f"error: {DRIVER} missing", file=sys.stderr)
        return 2
    env = os.environ.copy()
    env["PATH"] = str(UCRT_BIN) + os.pathsep + env.get("PATH", "")
    proc = subprocess.run([str(DRIVER), "--self-test"], stdout=subprocess.PIPE,
                          stderr=subprocess.PIPE, env=env, check=False)
    sys.stdout.buffer.write(proc.stdout)
    sys.stderr.buffer.write(proc.stderr)
    if proc.returncode != 0:
        return proc.returncode
    # stdin protocol: tie keeps the earlier id; a miss is none.
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
    print("protocol self-test ok", flush=True)
    return 0


def run_live() -> int:
    if not PREDICTION.exists():
        print("error: prediction.md is missing; refusing to launch", file=sys.stderr)
        return 2
    if not REPAIR_PREDICTION.exists():
        print("error: repair-prediction.md is missing; refusing to launch", file=sys.stderr)
        return 2
    if not WINDOWED_CONF.exists():
        print(f"error: windowed config missing: {WINDOWED_CONF}", file=sys.stderr)
        return 2
    rc = run_self_test()
    if rc != 0:
        print("error: pick driver self-test failed; not launching", file=sys.stderr)
        return rc

    before = install_hashes()
    if before["Lemmings Revolution.exe"] != STOCK_SHA256:
        print("error: installed exe does not match the authority hash", file=sys.stderr)
        return 2
    existing = find_pids("Lemmings")
    if existing:
        print("error: a Lemmings process is already running; not launching", file=sys.stderr)
        print(existing, file=sys.stderr)
        _write_restoration({
            "prediction_mtime": PREDICTION.stat().st_mtime,
            "launched": False,
            "blocked": "Lemmings process already running",
            "existing": [{"pid": pid, "exe": exe} for pid, exe in existing],
            "hashes_before": before,
        })
        return 3

    pred_mtime = PREDICTION.stat().st_mtime
    repair_mtime = REPAIR_PREDICTION.stat().st_mtime
    dgv_path = INSTALL / "dgVoodoo.conf"
    save_path = INSTALL / "SavedGame.dat"
    dgv_bytes = dgv_path.read_bytes()
    save_bytes = save_path.read_bytes() if save_path.exists() else None
    owned = None
    handle = None
    failure = None
    rows: list[dict] = []
    attempts: list[dict] = []
    launched = False
    launch_at = None
    pid = None
    probe = None
    accept_picks = False
    try:
        dgv_path.write_bytes(WINDOWED_CONF.read_bytes())
        print("launching Lemmings Revolution.exe (ONE start)", flush=True)
        launched = True
        launch_at = time.time()
        owned = subprocess.Popen([str(STOCK_EXE)], cwd=str(INSTALL))
        pid = owned.pid
        print(
            f"  pid {pid} started at {launch_at:.3f}; prediction mtime {pred_mtime:.3f}; "
            f"repair prediction mtime {repair_mtime:.3f}",
            flush=True,
        )
        if pred_mtime >= launch_at or repair_mtime >= launch_at:
            failure = "prediction file is not older than the process"
            return 2
        time.sleep(12.0)
        handle, attached = attach(write=True)
        if not handle or attached != pid:
            failure = f"attach failed (handle={bool(handle)} attached={attached} owned={pid})"
            return 1
        print(f"attached pid {attached} ui_state {ui_state(handle)}", flush=True)
        from goto_level import SCREEN_ID
        menu_deadline = time.time() + 20.0
        state = ui_state(handle)
        screen = i32(handle, SCREEN_ID)
        while not (state == 3 and screen == 3) and time.time() < menu_deadline:
            print(f"  waiting for main menu ui_state {state} screen {screen}", flush=True)
            time.sleep(1.0)
            state = ui_state(handle)
            screen = i32(handle, SCREEN_ID)
        print(f"  before scan ui_state {state} screen {screen}", flush=True)
        if state != 3 or screen != 3:
            probe = {
                "presses": [],
                "candidates": 0,
                "return_sent": False,
                "note": (
                    f"three-press scan not run; ui_state {state} screen {screen}; "
                    "Down was not sent"
                ),
                "acquire_errors": [],
                "intersection_addrs": [],
            }
            failure = probe["note"] + "; Return was not sent"
            return 2
        probe = begin_menu_probe(pid)
        try:
            enter_rc = S.enter(handle, "MM_CLIMB_002")
        finally:
            end_menu_probe(probe)
        loaded = SP.loaded_level(handle)
        state = ui_state(handle)
        print(
            f"  enter rc {enter_rc} ui_state {state} loaded {loaded!r} "
            f"menu-index candidates {probe.get('candidates')} "
            f"return_sent {probe.get('return_sent')}",
            flush=True,
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

        deadline = time.time() + 35.0
        while time.time() < deadline:
            _ptrs, cands, _unread = lemming_candidates(handle)
            eligible = [c for c in cands if c["eligible"]]
            print(f"  waiting lemmings: {len(cands)} boxed, {len(eligible)} eligible,"
                  f" released {i32(handle, SP.RELEASED)}", flush=True)
            if len(eligible) >= 4:
                break
            time.sleep(1.0)

        scr = A.screen(handle)
        widgets, widget_notes = widget_boxes(handle)
        (OUT / "widget-probe.json").write_bytes(
            json.dumps({"gate": i32(handle, WIDGET_CALL_GATE), "notes": widget_notes,
                        "usable_boxes": widgets}, indent=2).encode("utf-8")
        )
        print(f"  widget usable boxes {len(widgets)} gate {i32(handle, WIDGET_CALL_GATE)}",
              flush=True)

        on_points = lemming_aim_points(cands)
        empty_points = empty_aim_points()
        on_i = 0
        empty_i = 0
        attempt_n = 0
        while len(rows) < 20 and attempt_n < 80:
            attempt_n += 1
            on_count = sum(1 for row in rows if row["original_index"] != "none")
            none_count = len(rows) - on_count
            want_on = on_count < 5 or (on_count <= none_count and on_count < 10)
            if want_on and on_i >= len(on_points):
                _ptrs, cands, _unread = lemming_candidates(handle)
                on_points = lemming_aim_points(cands)
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
            print(f"  attempt {attempt_n} {kind} -> ({x:.1f}, {y:.1f})"
                  f" kept {len(rows)} on {on_count} none {none_count}", flush=True)
            try:
                row, err, fg, pos = aim_and_snap(handle, pid, x, y)
            except Exception as exc:
                failure = f"aim/acquire failed: {exc!r}"
                print(f"  {failure}", flush=True)
                break
            if err or row is None:
                attempts.append({"n": attempt_n, "kind": kind, "x": x, "y": y, "error": err})
                print(f"    discarded: {err}", flush=True)
                continue
            panels = panel_boxes(handle)
            widgets, _notes = widget_boxes(handle)
            rejected = None
            if "MM_CLIMB_002" not in (row["level"] or ""):
                rejected = f"level changed to {row['level']!r}"
            elif on_panel(row["cursor_x"], row["cursor_y"], row["cursor_box"], panels, row["hovered_button"]):
                rejected = "on a skill-panel widget"
            elif row["widget_call_gate"] and row["widget_call_gate"] > 0 and on_widget(row["cursor_box"], widgets):
                rejected = "on a list widget"
            is_on = row["original_index"] != "none"
            if rejected is None and not accept_class(rows, is_on):
                rejected = "quota reserved the remaining slots for the other class"
            attempts.append({
                "n": attempt_n, "kind": kind, "target_x": x, "target_y": y,
                "cursor_x": row["cursor_x"], "cursor_y": row["cursor_y"],
                "original_index": row["original_index"], "pick_id": row["pick_id"],
                "rejected": rejected, "foreground": fg.get("method"),
                "aim": None if pos is None else [pos[0], pos[1]],
            })
            if rejected:
                print(f"    discarded: {rejected} original {row['original_index']} pick {row['pick_id']}",
                      flush=True)
                continue
            row["sample"] = len(rows) + 1
            row["aim_kind"] = kind
            row["aim_target"] = [x, y]
            rows.append(row)
            print(f"    kept {row['sample']} original {row['original_index']} pick {row['pick_id']}"
                  f" agree {row['agree']} overlaps {row['n_overlapping']}"
                  f" box {row['cursor_box_source']}", flush=True)

        if attempts:
            write_jsonl(OUT / "attempts.jsonl", attempts)
        on_count = sum(1 for row in rows if row["original_index"] != "none")
        none_count = len(rows) - on_count
        agree = sum(1 for row in rows if row["agree"])
        print(
            f"kept {len(rows)} on {on_count} none {none_count} "
            f"agree {agree} disagree {len(rows) - agree}",
            flush=True,
        )
        gate_ok = (
            failure is None and len(rows) == 20 and on_count >= 5 and none_count >= 5
        )
        if gate_ok:
            write_jsonl(OUT / "picks.jsonl", rows)
            accept_picks = True
            stale = OUT / "down-diff.txt"
            if stale.exists():
                stale.unlink()
            return 0
        if failure is None:
            failure = (f"sample missed the 20/5/5 gate: rows={len(rows)}"
                       f" original-lemming={on_count} original-none={none_count}")
        return 4
    except Exception as exc:
        failure = f"exception: {exc!r}"
        import traceback
        traceback.print_exc()
        return 1
    finally:
        if probe is not None:
            end_menu_probe(probe)
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
                print(f"  terminate warning: {exc}", flush=True)
            if pid is not None and any(p == pid for p, _exe in find_pids("Lemmings")):
                subprocess.run(["taskkill", "/PID", str(pid), "/T", "/F"],
                               check=False, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
                try:
                    owned.wait(timeout=10)
                except Exception:
                    pass
            time.sleep(0.5)
            gone = not any(p == pid for p, _exe in find_pids("Lemmings"))
            print(f"  process {pid} gone={gone} returncode {owned.returncode}", flush=True)
        if dgv_bytes is not None:
            dgv_path.write_bytes(dgv_bytes)
        if save_bytes is not None:
            save_path.write_bytes(save_bytes)
        after = install_hashes()
        restored = after == before
        print(f"  install hashes match before-state: {restored}", flush=True)
        if not restored:
            print(f"  before={before}", file=sys.stderr)
            print(f"  after ={after}", file=sys.stderr)
        picks_path = OUT / "picks.jsonl"
        if accept_picks:
            stale = OUT / "down-diff.txt"
            if stale.exists():
                stale.unlink()
        else:
            if picks_path.exists():
                picks_path.unlink()
            if probe is not None and probe.get("candidates") == 0:
                try:
                    _write_down_diff(probe)
                except Exception as exc:
                    print(f"  down-diff write failed: {exc!r}", flush=True)
        menu_presses = None if probe is None else probe.get("presses")
        _write_restoration({
            "prediction_path": str(PREDICTION),
            "prediction_mtime": pred_mtime,
            "prediction_before_process": (
                launch_at > pred_mtime if launched and launch_at is not None else None
            ),
            "repair_prediction_path": str(REPAIR_PREDICTION),
            "repair_prediction_mtime": repair_mtime,
            "repair_prediction_before_process": (
                launch_at > repair_mtime if launched and launch_at is not None else None
            ),
            "launched": launched,
            "pid_started": pid,
            "pid_gone": gone,
            "remaining_lemmings": [
                {"pid": p, "exe": exe} for p, exe in find_pids("Lemmings")
            ],
            "hashes_before": before,
            "hashes_after": after,
            "hashes_match": restored,
            "failure": failure,
            "kept_rows": len(rows),
            "menu_candidates": None if probe is None else probe.get("candidates"),
            "return_sent": None if probe is None else probe.get("return_sent"),
            "menu_presses": menu_presses,
            "accept_picks": accept_picks,
        })
        if failure:
            print(f"FAILED: {failure}", file=sys.stderr)
            (OUT / "FAILED.txt").write_text(failure + "\n", encoding="utf-8", newline="\n")


def _write_restoration(payload: dict) -> None:
    OUT.mkdir(parents=True, exist_ok=True)
    (OUT / "restoration.json").write_bytes(
        json.dumps(payload, indent=2).encode("utf-8")
    )
    lines = [
        f"prediction_mtime {payload.get('prediction_mtime')}",
        f"prediction_before_process {payload.get('prediction_before_process')}",
        f"repair_prediction_mtime {payload.get('repair_prediction_mtime')}",
        f"repair_prediction_before_process {payload.get('repair_prediction_before_process')}",
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


def main() -> int:
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument("--run-live", action="store_true")
    ap.add_argument("--self-test", action="store_true")
    args = ap.parse_args()
    OUT.mkdir(parents=True, exist_ok=True)
    if args.self_test and not args.run_live:
        return run_self_test()
    if not args.run_live:
        print("pass --run-live for the one authorized capture, or --self-test")
        return 0
    return run_live()


if __name__ == "__main__":
    raise SystemExit(main())
