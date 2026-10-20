"""A4j: break 0x4409E0 before unpause, watch new controller+0x40 to first non-255 write."""
from __future__ import annotations

import argparse
import json
import re
import struct
import subprocess
import sys
import time
import traceback
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
STOCK_EXE = Path(r"C:\progs\Lemmings Revolution\Lemmings Revolution.exe")
STOCK_SHA256 = "32675bdb49866e76502713251fdfd2b09078c2481c72ca22ff68ebf4b3737a2f"
INSTALL = STOCK_EXE.parent
ACTOR_LIST = 0x004CBB18
CONTROLLER_INIT = 0x004409E0
LEMMING_VT = 0x004B10E8
OTHER_VT = 0x004B0F6C
MAX_HITS = 15
SPAWN_BP_SEC = 45.0
WATCH_SEC = 30.0

sys.path.insert(0, str(ROOT / "tools" / "harness"))
from acquire_game_foreground import acquire  # noqa: E402
from goto_level import loaded_level  # noqa: E402
from lrinput import LRInput  # noqa: E402
from lrstate import attach, i32, ui_state, u32  # noqa: E402
from memdump import find_pids, k32, read_mem  # noqa: E402
from measure_spawn import actor_ptrs  # noqa: E402

from watch_actor_tint_a4i import (  # noqa: E402
    MI,
    advancement_snapshot,
    disassemble_at,
    f64,
    gameplay_advanced,
    install_hashes,
    load_enter_fast,
    read_exact,
)


def known_controllers(handle) -> set[int]:
    return {a + 0x128 for a in actor_ptrs(handle)}


def find_actor_for_controller(handle, controller: int) -> int | None:
    for a in actor_ptrs(handle):
        if a + 0x128 == controller:
            return a
    return None


def actor_state(handle, actor: int) -> dict:
    c = actor + 0x128
    return {
        "actor": hex(actor),
        "controller": hex(c),
        "vtable": hex(u32(handle, actor) or 0),
        "action": i32(handle, c + 0x1C8),
        "temporal": i32(handle, c + 0x1CC),
        "direction": i32(handle, c + 0x1B8),
        "controller_plus_0x40": f64(handle, c + 0x40),
        "draw_position_xyz": list(struct.unpack("<3d", read_exact(handle, actor + 0x108, 24))),
    }


def read_ecx(mi: MI) -> int:
    text = mi.console_collect("print/x $ecx")
    for m in re.findall(r"0x[0-9a-fA-F]+", text):
        v = int(m, 16)
        if 0x10000 <= v < 0x80000000:
            return v
    raise RuntimeError("ecx parse failed: " + repr(text))


def record_watch_hit(
    handle,
    mi: MI,
    watch_addr: int,
    hits: list[dict],
    old_val: float,
) -> float:
    pc, bt = mi.pc_and_backtrace()
    new_val = f64(handle, watch_addr)
    dis = disassemble_at(pc)
    hits.append(
        {
            "hit_index": len(hits),
            "eip": hex(pc),
            "instruction_bytes_disasm": dis,
            "old_value": old_val,
            "new_value": new_val,
            "watch_address": hex(watch_addr),
            "backtrace": bt,
        }
    )
    return new_val


def append_note(path: Path, text: str) -> None:
    with path.open("a", encoding="utf-8") as f:
        f.write(text)
        if not text.endswith("\n"):
            f.write("\n")


def main() -> int:
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument("--output", type=Path, required=True)
    ap.add_argument("--gdb", type=Path, default=Path(r"C:\progs\msys2\mingw32\bin\gdb.exe"))
    args = ap.parse_args()
    out = args.output.resolve()
    out.mkdir(parents=True, exist_ok=True)
    note = out / "round-note.md"
    report: dict = {"hits": [], "spawn_bp_skips": 0}

    before = install_hashes()
    if before["Lemmings Revolution.exe"] != STOCK_SHA256:
        raise SystemExit("stock exe hash mismatch")
    (out / "before.json").write_text(json.dumps(before, indent=2) + "\n", encoding="utf-8")

    owned = None
    handle = None
    mi: MI | None = None
    conf_backup = None

    try:
        if find_pids("Lemmings"):
            raise RuntimeError("exclusive live ownership required")
        conf_backup = (INSTALL / "dgVoodoo.conf").read_bytes()
        windowed = ROOT / "build/tmp/a1-zoom-elevation/dgVoodoo.windowed.conf"
        (INSTALL / "dgVoodoo.conf").write_bytes(windowed.read_bytes())

        game_starts = 0
        enter_rc = 1
        while game_starts < 2 and enter_rc != 0:
            if owned and owned.poll() is None:
                owned.terminate()
                owned.wait(timeout=10)
            owned = subprocess.Popen([str(STOCK_EXE)], cwd=INSTALL)
            game_starts += 1
            report["pid"] = owned.pid
            time.sleep(8 if game_starts == 1 else 10)
            enter = load_enter_fast(owned.pid)
            enter.WORK = out
            enter_rc = enter.main()
        report["game_starts"] = game_starts
        if enter_rc != 0:
            raise RuntimeError("enter_fast failed after 2 starts")

        handle, apid = attach()
        if not handle or apid != owned.pid:
            raise RuntimeError("attach failed")
        lvl = loaded_level(handle)
        report["loaded_level"] = lvl
        assert lvl.lower() == r"compiled\mm_climb_002.lvl"
        assert ui_state(handle) == 1, "must stay paused before GDB"

        pre_controllers = known_controllers(handle)
        report["before_attach"] = {
            "ui_state": ui_state(handle),
            "actor_list_count": i32(handle, ACTOR_LIST),
            "known_controllers": sorted(hex(x) for x in pre_controllers),
        }
        ref_actor = actor_ptrs(handle)[0] if actor_ptrs(handle) else None
        if ref_actor:
            report["before_attach"]["ref_actor_snapshot"] = advancement_snapshot(handle, ref_actor)

        mi = MI(args.gdb, owned.pid, out / "gdb-mi.log")
        mi.start()
        bp = mi.breakpoint_exec(CONTROLLER_INIT)
        report["spawn_breakpoint"] = {
            "number": bp,
            "address": hex(CONTROLLER_INIT),
            "break_list_mi": mi.break_list_mi(),
            "break_list_human": mi.break_list_human(),
        }
        mi.resume()
        report["inferior_running_before_unpause"] = True

        acquire(owned.pid)
        with LRInput() as inp:
            inp.enable()
            inp.tap("P", 2)
            inp.wait_frames(3)
            inp.release()
        report["unpaused_after_continue"] = ui_state(handle)
        if not mi.running:
            mi.resume()

        target_controller = None
        deadline = time.monotonic() + SPAWN_BP_SEC
        while time.monotonic() < deadline and target_controller is None:
            try:
                stop_line = mi.wait_any_stop(timeout=min(2.0, deadline - time.monotonic()))
            except TimeoutError:
                continue
            reason = MI.stop_reason(stop_line)
            if reason == "breakpoint":
                m_pc = re.search(r'addr="(0x[0-9a-fA-F]+)"', stop_line)
                pc = int(m_pc.group(1), 16) if m_pc else mi.pc_and_backtrace()[0]
                if pc != CONTROLLER_INIT:
                    mi.resume()
                    continue
                ecx = read_ecx(mi)
                _, bt = mi.pc_and_backtrace()
                report.setdefault("spawn_hits", []).append(
                    {"ecx": hex(ecx), "pc": hex(pc), "backtrace": bt}
                )
                if ecx in pre_controllers or ecx == CONTROLLER_INIT:
                    report["spawn_bp_skips"] += 1
                    pre_controllers.add(ecx)
                    mi.resume()
                    continue
                target_controller = ecx
                report["new_controller_ecx"] = hex(ecx)
                break
            if reason == "signal":
                report.setdefault("notes", []).append("unexpected signal during spawn wait")
                break
            mi.resume()

        if target_controller is None:
            raise RuntimeError(f"no new controller at {CONTROLLER_INIT:#x} within {SPAWN_BP_SEC}s")

        mi.delete_all_breakpoints()
        report["after_spawn_bp_delete"] = mi.break_list_human()

        watch_addr = target_controller + 0x40
        wp = mi.watch_write_double(watch_addr)
        report["tint_watch"] = {
            "number": wp,
            "address": hex(watch_addr),
            "break_list_mi": mi.break_list_mi(),
            "break_list_human": mi.break_list_human(),
        }
        last_val = f64(handle, watch_addr)
        report["tint_before_watch_continue"] = last_val
        mi.resume()

        watch_deadline = time.monotonic() + WATCH_SEC
        poll_initial = last_val
        while len(report["hits"]) < MAX_HITS and time.monotonic() < watch_deadline:
            if mi.running:
                try:
                    polled = f64(handle, watch_addr)
                except Exception:
                    polled = poll_initial
                if polled != poll_initial:
                    report["poll_detected_change"] = {"from": poll_initial, "to": polled}
                    try:
                        report["poll_interrupt_stop"] = mi.interrupt_landed(timeout=15)
                    except TimeoutError as exc:
                        report["poll_interrupt_timeout"] = repr(exc)
                    if not mi.running:
                        record_watch_hit(handle, mi, watch_addr, report["hits"], poll_initial)
                    last_val = polled
                    if polled != 255.0:
                        report["first_non_255"] = polled
                    break
                time.sleep(0.002)
                continue
            try:
                stop_line = mi.wait_any_stop(timeout=0.05)
            except TimeoutError:
                continue
            if MI.stop_reason(stop_line) == "watchpoint":
                old = last_val
                last_val = record_watch_hit(handle, mi, watch_addr, report["hits"], old)
                if last_val != 255.0:
                    report["first_non_255"] = last_val
                    break
                poll_initial = last_val
                mi.resume()
                continue
            if MI.stop_reason(stop_line) == "signal":
                break
            mi.resume()

        report["tint_after_watch_window"] = f64(handle, watch_addr)

        report["interrupt_stop"] = mi.interrupt_landed(timeout=15)
        mi.delete_all_breakpoints()
        report["break_list_after_delete_mi"] = mi.break_list_mi()
        report["break_list_after_delete_human"] = mi.break_list_human()
        mi.command("-target-detach", timeout=10)
        mi.attached = False
        report["detached"] = True
        time.sleep(0.15)

        report["after_detach"] = {
            "ui_state": ui_state(handle),
            "actor_list_count": i32(handle, ACTOR_LIST),
        }
        actor = find_actor_for_controller(handle, target_controller)
        if actor:
            report["spawned_actor"] = actor_state(handle, actor)
        else:
            report["spawned_actor"] = {"error": "no actor with actor+0x128 == ecx"}

        if ref_actor:
            report["after_detach"]["ref_actor_snapshot"] = advancement_snapshot(handle, ref_actor)
            report["gameplay_advanced"] = gameplay_advanced(
                report["before_attach"]["ref_actor_snapshot"],
                report["after_detach"]["ref_actor_snapshot"],
            )
        else:
            report["gameplay_advanced"] = (
                report["before_attach"]["actor_list_count"] != report["after_detach"]["actor_list_count"]
            )

        report["process_alive_after_detach"] = owned.poll() is None
        report["completed"] = True
    except Exception:
        report["error"] = traceback.format_exc()
    finally:
        if mi and mi.attached:
            try:
                mi.cleanup(report)
            except Exception as exc:
                report["cleanup_error"] = repr(exc)
        if handle:
            k32.CloseHandle(handle)
        if owned:
            try:
                if owned.poll() is None:
                    owned.terminate()
                    owned.wait(timeout=15)
                report["process_stopped"] = owned.poll() is not None
            except Exception as exc:
                report["process_stop_error"] = repr(exc)
        if conf_backup is not None:
            try:
                (INSTALL / "dgVoodoo.conf").write_bytes(conf_backup)
            except Exception as exc:
                report["dgVoodoo_restore_error"] = repr(exc)
        after = install_hashes()
        report["after"] = after
        report["install_hashes_match"] = after == before
        (out / "after.json").write_text(json.dumps(after, indent=2) + "\n", encoding="utf-8")
        (out / "session-report.json").write_text(json.dumps(report, indent=2) + "\n", encoding="utf-8")

        cmd = f"python tools/harness/watch_spawn_tint_a4j.py --output {out.relative_to(ROOT)}"
        section = f"\n## Run results\n\nCommand: `{cmd}`\n\n"
        if report.get("spawn_breakpoint", {}).get("break_list_human"):
            section += "**Spawn hw bp:**\n```\n" + report["spawn_breakpoint"]["break_list_human"] + "```\n"
        if report.get("tint_watch", {}).get("break_list_human"):
            section += "**Tint hw watch:**\n```\n" + report["tint_watch"]["break_list_human"] + "```\n"
        if report.get("hits"):
            section += "\n**Watch hits:**\n"
            for h in report["hits"]:
                section += (
                    f"- EIP `{h['eip']}` old={h['old_value']} new={h['new_value']}\n"
                )
                for fr in h.get("backtrace") or []:
                    section += f"  - `{fr}`\n"
                stdout = (h.get("instruction_bytes_disasm") or {}).get("stdout", "")
                if stdout:
                    section += f"```\n{stdout[-600:]}\n```\n"
        else:
            section += "\n**Watch hits:** none\n"
        if report.get("spawned_actor"):
            section += f"\n**Spawned actor:** `{json.dumps(report['spawned_actor'])}`\n"
        section += (
            f"\n**Advancement:** {report.get('gameplay_advanced')}\n"
            f"**Cleanup:** empty={report.get('break_list_after_delete_human')!r} "
            f"process_stopped={report.get('process_stopped')} hashes={report.get('install_hashes_match')}\n"
        )
        if report.get("error"):
            section += f"\n**Error:**\n```\n{report['error']}\n```\n"
        append_note(note, section)

    ok = report.get("completed") and report.get("install_hashes_match")
    has_writer = bool(report.get("first_non_255") is not None and report.get("hits"))
    return 0 if ok and has_writer else 1


if __name__ == "__main__":
    raise SystemExit(main())
