"""A4i: hardware write watch on controller+0x40 (record0 R) for walk/block tint."""
from __future__ import annotations

import argparse
import hashlib
import importlib.util
import json
import queue
import re
import struct
import subprocess
import sys
import threading
import time
import traceback
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
STOCK_EXE = Path(r"C:\progs\Lemmings Revolution\Lemmings Revolution.exe")
STOCK_SHA256 = "32675bdb49866e76502713251fdfd2b09078c2481c72ca22ff68ebf4b3737a2f"
INSTALL = STOCK_EXE.parent
ACTOR_LIST = 0x004CBB18
LEMMING_VT = 0x004B10E8
OTHER_VT = 0x004B0F6C
RECORD_CONSTRUCTOR = 0x00440A10
MAX_PHASE1_SEC = 10.0
MAX_HITS = 15
MAX_PHASE1_HITS = 5
PHASE2_SPAWN_SEC = 30.0
NOTE_PATH = ROOT / "build/tmp/a4i-tint-write-watch/round-note.md"

sys.path.insert(0, str(ROOT / "tools" / "harness"))
from acquire_game_foreground import acquire  # noqa: E402
from goto_level import loaded_level  # noqa: E402
from lrinput import LRInput  # noqa: E402
from lrstate import attach, i32, ui_state, u32  # noqa: E402
from memdump import find_pids, k32, read_mem  # noqa: E402
from measure_spawn import actor_ptrs  # noqa: E402


def sha_file(path: Path) -> str:
    return hashlib.sha256(path.read_bytes()).hexdigest()


def install_hashes() -> dict[str, str]:
    names = ("Lemmings Revolution.exe", "SavedGame.dat", "dgVoodoo.conf")
    return {n: sha_file(INSTALL / n) for n in names}


class MI:
    """Minimal GDB/MI wrapper (copied from trace_cell_emission.py)."""

    def __init__(self, gdb: Path, pid: int, log_path: Path):
        self.proc = subprocess.Popen(
            [str(gdb), "--nx", "--quiet", "--interpreter=mi2"],
            stdin=subprocess.PIPE,
            stdout=subprocess.PIPE,
            stderr=subprocess.STDOUT,
            text=True,
            encoding="utf-8",
            errors="replace",
            creationflags=subprocess.CREATE_NO_WINDOW,
        )
        self.messages: queue.Queue[str] = queue.Queue()
        self.lines: list[str] = []
        self.token = 0
        self.attached = False
        self.running = False
        self.log_path = log_path
        threading.Thread(target=self._reader, daemon=True).start()
        self.pid = pid

    def _reader(self) -> None:
        assert self.proc.stdout is not None
        for line in self.proc.stdout:
            line = line.rstrip()
            self.lines.append(line)
            self.messages.put(line)

    def command(self, text: str, timeout: float = 20) -> str:
        assert self.proc.stdin is not None
        self.token += 1
        prefix = str(self.token)
        self.proc.stdin.write(prefix + text + "\n")
        self.proc.stdin.flush()
        deadline = time.monotonic() + timeout
        while time.monotonic() < deadline:
            try:
                line = self.messages.get(timeout=0.1)
            except queue.Empty:
                if self.proc.poll() is not None:
                    raise RuntimeError("GDB exited")
                continue
            if line.startswith(prefix + "^"):
                if line.startswith(prefix + "^error"):
                    raise RuntimeError(line)
                return line
        raise TimeoutError(text)

    def console(self, text: str, timeout: float = 20) -> str:
        return self.command("-interpreter-exec console " + json.dumps(text), timeout)

    def console_collect(self, text: str, timeout: float = 10) -> str:
        self.token += 1
        prefix = str(self.token)
        assert self.proc.stdin is not None
        self.proc.stdin.write(prefix + "-interpreter-exec console " + json.dumps(text) + "\n")
        self.proc.stdin.flush()
        deadline = time.monotonic() + timeout
        chunks: list[str] = []
        while time.monotonic() < deadline:
            try:
                line = self.messages.get(timeout=0.1)
            except queue.Empty:
                continue
            if line.startswith("~"):
                chunks.append(json.loads(line[1:]))
            if line.startswith(prefix + "^"):
                break
        return "".join(chunks)

    def wait_any_stop(self, timeout: float = 10) -> str:
        deadline = time.monotonic() + timeout
        while time.monotonic() < deadline:
            try:
                line = self.messages.get(timeout=0.1)
            except queue.Empty:
                if self.proc.poll() is not None:
                    raise RuntimeError("GDB exited waiting for stop")
                continue
            if line.startswith("*stopped"):
                self.running = False
                return line
        raise TimeoutError("inferior stop")

    @staticmethod
    def stop_reason(line: str) -> str:
        if "reason=\"watchpoint-trigger\"" in line:
            return "watchpoint"
        if "reason=\"breakpoint-hit\"" in line:
            return "breakpoint"
        if "reason=\"signal-received\"" in line:
            return "signal"
        if line.strip() == "*stopped":
            return "unknown"
        return "other"

    def resume(self) -> None:
        self.command("-exec-continue", timeout=5)
        self.running = True

    def interrupt_landed(self, timeout: float = 10) -> str:
        self.command("-exec-interrupt --all", timeout=10)
        return self.wait_any_stop(timeout=timeout)

    def breakpoint_exec(self, address: int) -> str:
        line = self.command(f"-break-insert -h *{address:#x}")
        m = re.search(r'number="(\d+)"', line)
        if not m:
            raise RuntimeError("no breakpoint number: " + line)
        return m.group(1)

    def watch_write_double(self, address: int) -> str:
        expr = f"*(double*){address:#x}"
        line = self.command(f"-break-watch {json.dumps(expr)}")
        m = re.search(r'number="(\d+)"', line)
        if not m:
            raise RuntimeError("no watchpoint number: " + line)
        return m.group(1)

    def delete_all_breakpoints(self) -> None:
        try:
            self.command("-break-delete", timeout=10)
        except RuntimeError as exc:
            if "No breakpoint" in str(exc) or "not found" in str(exc).lower():
                return
            raise

    def break_list_mi(self) -> str:
        return self.command("-break-list", timeout=10)

    def break_list_human(self) -> str:
        return self.console_collect("info break", timeout=10)

    def pc_and_backtrace(self) -> tuple[int, list[str]]:
        pc_line = self.console("print/x $pc")
        m = re.search(r"0x[0-9a-fA-F]+", pc_line)
        pc = int(m.group(0), 16) if m else 0
        self.console("bt 4")
        time.sleep(0.15)
        bt = []
        for line in self.lines[-40:]:
            if line.startswith("~"):
                text = json.loads(line[1:]).strip()
                if text.startswith("#"):
                    bt.append(text)
        return pc, bt

    def cleanup(self, report: dict) -> None:
        errors: list[str] = []
        try:
            if self.attached and self.running:
                try:
                    self.interrupt_landed(timeout=10)
                except Exception as exc:
                    errors.append("interrupt: " + repr(exc))
            if self.attached:
                try:
                    self.delete_all_breakpoints()
                    report["breakpoint_list_after_delete_mi"] = self.break_list_mi()
                    report["breakpoint_list_after_delete_human"] = self.break_list_human()
                except Exception as exc:
                    errors.append("delete breakpoints: " + repr(exc))
                try:
                    self.command("-target-detach", timeout=10)
                    self.attached = False
                except Exception as exc:
                    errors.append("detach: " + repr(exc))
            if not self.attached and self.proc.poll() is None:
                try:
                    self.command("-gdb-exit", timeout=5)
                    self.proc.wait(timeout=5)
                except Exception as exc:
                    errors.append("exit: " + repr(exc))
        finally:
            report["debugger_attached_after_cleanup"] = self.attached
            report["gdb_exit_code"] = self.proc.poll()
            if errors:
                report["gdb_cleanup_errors"] = errors
            self.log_path.write_text("\n".join(self.lines), encoding="utf-8")

    def start(self) -> None:
        for setting in ("pagination off", "confirm off", "auto-solib-add off", "mi-async on"):
            self.command("-gdb-set " + setting)
        self.command("-environment-cd " + json.dumps(self.log_path.parent.as_posix()))
        self.command(f"-target-attach {self.pid}", timeout=30)
        self.attached = True
        self.console("set can-use-hw-watchpoints 1")


def read_exact(handle, address: int, size: int) -> bytes:
    data = read_mem(handle, address, size)
    if data is None or len(data) != size:
        raise RuntimeError(f"incomplete READ at {address:#x}/{size:#x}")
    return data


def f64(handle, address: int) -> float:
    return struct.unpack("<d", read_exact(handle, address, 8))[0]


def advancement_snapshot(handle, actor: int) -> dict:
    pos = struct.unpack("<3d", read_exact(handle, actor + 0x108, 24))
    count = i32(handle, ACTOR_LIST)
    return {
        "ui_state": ui_state(handle),
        "actor_list_count": count,
        "draw_position_xyz": pos,
        "draw_position_hex": read_exact(handle, actor + 0x108, 24).hex(),
    }


def gameplay_advanced(before: dict, after: dict) -> bool:
    if before.get("actor_list_count") != after.get("actor_list_count"):
        return True
    return before.get("draw_position_hex") != after.get("draw_position_hex")


def disassemble_at(pc: int, length: int = 16) -> dict:
    script = ROOT / "tools" / "harness" / "read_steppers.py"
    out = subprocess.run(
        [sys.executable, str(script), "--at", hex(pc), "--len", hex(length)],
        capture_output=True,
        text=True,
        cwd=ROOT,
        timeout=30,
    )
    return {
        "returncode": out.returncode,
        "stdout": out.stdout[-4000:] if out.stdout else "",
        "stderr": out.stderr[-2000:] if out.stderr else "",
    }


def actor_vt(handle, actor: int) -> int | None:
    return u32(handle, actor)


def select_walker(handle) -> tuple[int, dict]:
    for actor in actor_ptrs(handle):
        if not actor or actor_vt(handle, actor) not in (LEMMING_VT, OTHER_VT):
            continue
        controller = actor + 0x128
        action = i32(handle, controller + 0x1C8)
        temporal = i32(handle, controller + 0x1CC)
        direction = i32(handle, controller + 0x1B8)
        cont = u32(handle, controller + 0x1F8)
        if not cont:
            continue
        arr = u32(handle, cont + 0x0C)
        if arr is None or action is None or temporal is None or direction is None:
            continue
        if action != 13:
            continue
        ta = u32(handle, arr + action * 0x98 + 0x0C)
        if not ta:
            continue
        temp = ta + temporal * 0x10
        count = u32(handle, temp)
        if count != 5 or direction <= 0:
            continue
        tint = f64(handle, controller + 0x40)
        pos = struct.unpack("<3d", read_exact(handle, actor + 0x108, 24))
        row = {
            "actor": hex(actor),
            "controller": hex(controller),
            "action": action,
            "temporal": temporal,
            "direction": direction,
            "count": count,
            "tint_r_record0": tint,
            "draw_position": pos,
        }
        return actor, row
    raise RuntimeError("no walk/action13/count5/direction>0 actor in actor list")


def load_enter_fast(owned_pid: int):
    path = ROOT / "build/tmp/a1-zoom-elevation/enter_fast.py"
    spec = importlib.util.spec_from_file_location("enter_fast", path)
    mod = importlib.util.module_from_spec(spec)
    assert spec.loader
    spec.loader.exec_module(mod)
    from drive import aim_at  # noqa: E402
    from lrinput import LRInput as BaseLRInput  # noqa: E402

    class FocusedInput(BaseLRInput):
        def tap(self, *a, **kw):
            acquire(owned_pid)
            return super().tap(*a, **kw)

    mod.LRInput = FocusedInput
    old_find = mod.find_menu_index

    def focused_find(h, inp):
        acquire(owned_pid)
        inp._set_u32(32, 1)
        aim_at(h, inp, (620, 460))
        return old_find(h, inp)

    mod.find_menu_index = focused_find
    return mod


def unpause_with_acquire(pid: int, handle) -> None:
    if ui_state(handle) != 1:
        return
    acquire(pid)
    with LRInput() as inp:
        inp.enable()
        inp.tap("P", 2)
        inp.wait_frames(3)
        inp.release()
    time.sleep(0.05)


def record_hit(
    handle,
    mi: MI,
    watch_address: int,
    hits: list[dict],
    phase: str,
    actor_row: dict,
    old_value: float | None = None,
) -> None:
    pc, bt = mi.pc_and_backtrace()
    new_val = f64(handle, watch_address)
    dis = disassemble_at(pc)
    hits.append(
        {
            "phase": phase,
            "hit_index": len(hits),
            "eip": hex(pc),
            "disassembly": dis,
            "old_value_at_watch": old_value,
            "new_value_at_watch": new_val,
            "watch_address": hex(watch_address),
            "actor": actor_row,
            "backtrace": bt,
            "stop_line": mi.lines[-1] if mi.lines else "",
        }
    )


def append_note(note_path: Path, text: str) -> None:
    with note_path.open("a", encoding="utf-8") as f:
        f.write(text)
        if not text.endswith("\n"):
            f.write("\n")


def run_phase1_watch(
    handle,
    mi: MI,
    owned_pid: int,
    actor: int,
    actor_row: dict,
    watch_address: int,
    report: dict,
) -> tuple[int, bool]:
    """Returns (hit_count, gameplay_advanced)."""
    before_attach = advancement_snapshot(handle, actor)
    report["phase1_before_attach"] = before_attach
    assert before_attach["ui_state"] == 0, "phase1 requires unpaused gameplay before GDB attach"

    mi.start()
    report["debugger_attached"] = True
    wp = mi.watch_write_double(watch_address)
    report["phase1_watch"] = {
        "number": wp,
        "address": hex(watch_address),
        "break_list_mi": mi.break_list_mi(),
        "break_list_human": mi.break_list_human(),
    }
    mi.resume()
    phase1_start = time.monotonic()
    while len(report["hits"]) < MAX_PHASE1_HITS and (time.monotonic() - phase1_start) < MAX_PHASE1_SEC:
        remaining = MAX_PHASE1_SEC - (time.monotonic() - phase1_start)
        try:
            stop_line = mi.wait_any_stop(timeout=max(0.05, remaining))
        except TimeoutError:
            break
        reason = MI.stop_reason(stop_line)
        report.setdefault("phase1_stops", []).append({"reason": reason, "line": stop_line})
        if reason == "watchpoint":
            record_hit(handle, mi, watch_address, report["hits"], "phase1", actor_row)
            if len(report["hits"]) >= MAX_PHASE1_HITS:
                break
            mi.resume()
            continue
        if reason == "signal":
            report.setdefault("notes", []).append("phase1_unexpected_signal_not_end")
            break
        mi.resume()

    report["phase1_seconds"] = time.monotonic() - phase1_start
    stop_line = mi.interrupt_landed(timeout=15)
    report["phase1_interrupt_stop"] = stop_line
    mi.delete_all_breakpoints()
    report["phase1_break_list_after_delete_mi"] = mi.break_list_mi()
    report["phase1_break_list_after_delete_human"] = mi.break_list_human()
    mi.command("-target-detach", timeout=10)
    mi.attached = False
    report["phase1_detached"] = True
    time.sleep(0.2)
    after_detach = advancement_snapshot(handle, actor)
    report["phase1_after_detach"] = after_detach
    advanced = gameplay_advanced(before_attach, after_detach)
    report["phase1_gameplay_advanced"] = advanced
    return len([h for h in report["hits"] if h["phase"] == "phase1"]), advanced


def run_phase2_spawn_watch(
    handle,
    mi: MI,
    actor_row: dict,
    report: dict,
) -> None:
    before_attach = {
        "ui_state": ui_state(handle),
        "actor_list_count": i32(handle, ACTOR_LIST),
    }
    report["phase2_before_attach"] = before_attach
    assert before_attach["ui_state"] == 0, "phase2 requires unpaused gameplay before attach"

    known_controllers = {a + 0x128 for a in actor_ptrs(handle)}
    actor_count_0 = i32(handle, ACTOR_LIST)
    mi.start()
    bp = mi.breakpoint_exec(RECORD_CONSTRUCTOR)
    report["phase2"] = {
        "constructor_bp": bp,
        "actor_count_at_start": actor_count_0,
        "known_controllers": len(known_controllers),
        "break_list_mi": mi.break_list_mi(),
        "break_list_human": mi.break_list_human(),
    }
    mi.resume()
    target_controller = None
    deadline = time.monotonic() + PHASE2_SPAWN_SEC
    while time.monotonic() < deadline and target_controller is None:
        try:
            stop_line = mi.wait_any_stop(timeout=min(2.0, deadline - time.monotonic()))
        except TimeoutError:
            continue
        if MI.stop_reason(stop_line) != "breakpoint":
            mi.resume()
            continue
        pc, _ = mi.pc_and_backtrace()
        if pc != RECORD_CONSTRUCTOR:
            mi.resume()
            continue
        ecx = int(re.search(r"0x[0-9a-fA-F]+", mi.console("print/x $ecx")).group(0), 16)
        actors_now = actor_ptrs(handle)
        actor_count = i32(handle, ACTOR_LIST)
        for a in actors_now:
            if a + 0x128 == ecx and actor_vt(handle, a) in (LEMMING_VT, OTHER_VT):
                if ecx not in known_controllers or (actor_count or 0) > (actor_count_0 or 0):
                    target_controller = ecx
                    report["phase2"]["matched_actor"] = hex(a)
                    break
        if target_controller is None and ecx not in known_controllers:
            if actor_count and actor_count > (actor_count_0 or 0):
                target_controller = ecx
                report["phase2"]["match_reason"] = "new_ecx_and_actor_count_grew"
        if target_controller is None:
            mi.resume()
            continue
        report["phase2"]["matched_ecx"] = hex(target_controller)
        break

    mi.delete_all_breakpoints()
    if target_controller is None:
        report["phase2"]["error"] = f"no constructor hit within {PHASE2_SPAWN_SEC}s"
        mi.interrupt_landed(timeout=10)
        mi.delete_all_breakpoints()
        report["phase2_break_list_after_delete"] = mi.break_list_human()
        mi.command("-target-detach", timeout=10)
        mi.attached = False
        return

    watch_addr = target_controller + 0x40
    wp2 = mi.watch_write_double(watch_addr)
    report["phase2"]["watch"] = {
        "number": wp2,
        "address": hex(watch_addr),
        "break_list_mi": mi.break_list_mi(),
        "break_list_human": mi.break_list_human(),
    }
    mi.resume()
    p2_row = dict(actor_row)
    p2_row["controller"] = hex(target_controller)
    p2_deadline = time.monotonic() + 20.0
    while len(report["hits"]) < MAX_HITS and time.monotonic() < p2_deadline:
        try:
            stop_line = mi.wait_any_stop(timeout=min(2.0, p2_deadline - time.monotonic()))
        except TimeoutError:
            break
        if MI.stop_reason(stop_line) != "watchpoint":
            if MI.stop_reason(stop_line) == "signal":
                break
            mi.resume()
            continue
        record_hit(handle, mi, watch_addr, report["hits"], "phase2", p2_row)
        val = report["hits"][-1]["new_value_at_watch"]
        if val != 255.0:
            report["phase2"]["first_non_255"] = val
            break
        mi.resume()

    mi.interrupt_landed(timeout=15)
    mi.delete_all_breakpoints()
    report["phase2_break_list_after_delete_mi"] = mi.break_list_mi()
    report["phase2_break_list_after_delete_human"] = mi.break_list_human()
    mi.command("-target-detach", timeout=10)
    mi.attached = False


def write_retry_note(report: dict, retry_cmd: str) -> None:
    lines = [
        "\n## Retry (2026-10-19)\n",
        f"\nCommand: `{retry_cmd}`\n",
    ]
    if report.get("phase1_before_attach"):
        b, a = report["phase1_before_attach"], report.get("phase1_after_detach", {})
        lines.append(
            f"\n**Advancement proof:** before attach count={b.get('actor_list_count')} "
            f"pos={b.get('draw_position_xyz')}; after detach count={a.get('actor_list_count')} "
            f"pos={a.get('draw_position_xyz')}; advanced={report.get('phase1_gameplay_advanced')}\n"
        )
    pw = report.get("phase1_watch") or {}
    if pw.get("break_list_human"):
        lines.append("\n**Phase 1 watch `-break-list` / info break after install:**\n```\n")
        lines.append(pw.get("break_list_human", "") + "\n```\n")
    if report.get("phase1_break_list_after_delete_human"):
        lines.append("\n**Phase 1 empty before detach:**\n```\n")
        lines.append(report["phase1_break_list_after_delete_human"] + "\n```\n")
    hits = report.get("hits") or []
    if hits:
        lines.append("\n**Hits:**\n")
        for h in hits:
            lines.append(
                f"- {h['phase']} EIP `{h['eip']}` wrote **{h['new_value_at_watch']}** "
                f"at `{h['watch_address']}`\n"
            )
            if h.get("backtrace"):
                for fr in h["backtrace"][:4]:
                    lines.append(f"  - `{fr}`\n")
            stdout = (h.get("disassembly") or {}).get("stdout", "")
            if stdout:
                lines.append(f"  - bytes/disasm:\n```\n{stdout[-800:]}\n```\n")
    else:
        lines.append("\n**Hits:** none\n")
    if report.get("phase2"):
        p2 = report["phase2"]
        if p2.get("watch", {}).get("break_list_human"):
            lines.append("\n**Phase 2 watch install:**\n```\n" + p2["watch"]["break_list_human"] + "\n```\n")
        if report.get("phase2_break_list_after_delete_human"):
            lines.append(
                "\n**Phase 2 empty before detach:**\n```\n"
                + report["phase2_break_list_after_delete_human"]
                + "\n```\n"
            )
    lines.append(
        f"\n**Cleanup:** process_stopped={report.get('process_stopped')} "
        f"alive_after_detach={report.get('process_alive_after_detach')} "
        f"install_match={report.get('install_hashes_match')}\n"
    )
    if report.get("error"):
        lines.append("\n**Error:**\n```\n" + report["error"] + "\n```\n")
    append_note(NOTE_PATH, "".join(lines))


def main() -> int:
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument("--output", type=Path, required=True)
    ap.add_argument(
        "--spawn-only",
        action="store_true",
        help="skip phase 1; break 0x440A10 soon after level entry (uses one game start)",
    )
    ap.add_argument("--gdb", type=Path, default=Path(r"C:\progs\msys2\mingw32\bin\gdb.exe"))
    args = ap.parse_args()
    out = args.output.resolve()
    out.mkdir(parents=True, exist_ok=True)
    report_path = out / "session-report.json"
    retry_cmd = f"python tools/harness/watch_actor_tint_a4i.py --output {out.relative_to(ROOT)}"

    retry_prediction = (
        "\n## Retry prediction (before live run)\n\n"
        "Phase 1 with proven advancement: still zero rewrites of 152.0 on the established walker. "
        "Phase 2: first non-255 write at `ecx+0x40` after `0x440A10` lands in init/fill with value 152.0.\n"
        "Falsifier: hw watch fails to install; no advancement across attach window; only 255 writes.\n"
    )
    if NOTE_PATH.exists() and "Retry prediction (before live run)" not in NOTE_PATH.read_text(encoding="utf-8"):
        append_note(NOTE_PATH, retry_prediction)

    before = install_hashes()
    if before["Lemmings Revolution.exe"] != STOCK_SHA256:
        raise SystemExit("stock exe hash mismatch")
    (out / "before.json").write_text(json.dumps(before, indent=2) + "\n", encoding="utf-8")

    report: dict = {"before": before, "hits": [], "retry": True}
    owned = None
    handle = None
    mi: MI | None = None
    conf_backup = None
    game_starts = 0

    try:
        if find_pids("Lemmings"):
            raise RuntimeError("exclusive live ownership required — game already running")
        conf_backup = (INSTALL / "dgVoodoo.conf").read_bytes()
        windowed = ROOT / "build/tmp/a1-zoom-elevation/dgVoodoo.windowed.conf"
        (INSTALL / "dgVoodoo.conf").write_bytes(windowed.read_bytes())

        owned = subprocess.Popen([str(STOCK_EXE)], cwd=INSTALL)
        game_starts += 1
        report["pid"] = owned.pid
        report["game_starts"] = game_starts
        time.sleep(5)
        enter = load_enter_fast(owned.pid)
        enter.WORK = out
        if enter.main() != 0:
            raise RuntimeError("enter_fast failed")

        handle, apid = attach()
        if not handle or apid != owned.pid:
            raise RuntimeError("attach failed")
        lvl = loaded_level(handle)
        report["loaded_level"] = lvl
        assert lvl.lower() == r"compiled\mm_climb_002.lvl"

        with LRInput() as inp:
            inp.enable()
            inp.clear()
            acquire(owned.pid)
            inp.tap("P", 2)
            assert ui_state(handle) == 0
            time.sleep(8)
            acquire(owned.pid)
            inp.tap("P", 2)
            inp.wait_frames(3)
            inp.release()
        assert ui_state(handle) == 1

        mi = MI(args.gdb, owned.pid, out / "gdb-mi.log")
        actor_row: dict = {"actor": None, "controller": None}

        if args.spawn_only:
            report["spawn_only"] = True
            actor_row["note"] = "spawn-only; no walker gate"
            unpause_with_acquire(owned.pid, handle)
            assert ui_state(handle) == 0
            run_phase2_spawn_watch(handle, mi, actor_row, report)
        else:
            actor, actor_row = select_walker(handle)
            watch_address = actor + 0x168
            actor_row["pre_watch_tint_r"] = f64(handle, watch_address)
            report["selected_actor"] = actor_row

            unpause_with_acquire(owned.pid, handle)
            assert ui_state(handle) == 0

            p1_hits, advanced = run_phase1_watch(
                handle, mi, owned.pid, actor, actor_row, watch_address, report
            )

            proc_alive = owned.poll() is None
            report["process_alive_after_phase1_detach"] = proc_alive

            if p1_hits == 0 and advanced and len(report["hits"]) < MAX_HITS:
                unpause_with_acquire(owned.pid, handle)
                assert ui_state(handle) == 0
                run_phase2_spawn_watch(handle, mi, actor_row, report)
            elif p1_hits == 0 and not advanced:
                report["phase2_skipped"] = "phase1 zero hits but gameplay did not advance"
            elif p1_hits > 0:
                report["phase2_skipped"] = "phase1 recorded hits"

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
        report_path.write_text(json.dumps(report, indent=2) + "\n", encoding="utf-8")
        write_retry_note(report, retry_cmd)

    ok = report.get("completed") and report.get("install_hashes_match")
    has_writer = any(h.get("phase") == "phase2" and h.get("new_value_at_watch") not in (255.0, None) for h in report.get("hits", []))
    has_writer = has_writer or any(
        h.get("phase") == "phase1" and h.get("new_value_at_watch") not in (None,) for h in report.get("hits", [])
    )
    return 0 if ok and (has_writer or report.get("phase1_gameplay_advanced") is not None) else 1


if __name__ == "__main__":
    raise SystemExit(main())
