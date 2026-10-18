"""Trace one actual cylinder material/submission sequence in a paused stock frame.

READ evidence only: this does not accept camera constants or explain every pixel.
Requires 32-bit GDB and a game already at a retained reference camera. Run from
a hidden worker; foreground acquisition masks activation input from the game.
Raw dumps stay under ignored build/tmp and must not be committed as retail assets.
"""
import argparse
import json
from pathlib import Path
import queue
import subprocess
import threading
import time

from acquire_game_foreground import acquire
from lrstate import attach, ui_state, GAME_OBJ
from memdump import read_mem, k32
import goto_level

ROOT = Path(__file__).resolve().parents[2]


def trace(pid, case, output, gdb):
    output = output.resolve()
    output.relative_to((ROOT / "build/tmp").resolve())
    output.mkdir(parents=True, exist_ok=True)
    fixture = ROOT / "tools/harness/fixtures/camera_a1"
    name = "reference-state.json" if case == "initial" else "reference-rotation-key-left-state.json"
    expected = bytes.fromhex(json.loads((fixture / name).read_text())["camera"]["raw_hex"])
    focus = acquire(pid)
    handle, actual_pid = attach()
    if not handle or actual_pid != pid:
        if handle:
            k32.CloseHandle(handle)
        raise RuntimeError("Owned PID is not the attached stock game")
    try:
        if ui_state(handle) != 1 or goto_level.loaded_level(handle) != r"Compiled\MM_CLIMB_002.lvl":
            raise RuntimeError("Expected the paused reference level")
        if read_mem(handle, GAME_OBJ + 0x55C0, 0x168) != expected:
            raise RuntimeError("Camera does not match the requested retained reference")
    finally:
        k32.CloseHandle(handle)

    proc = subprocess.Popen([str(gdb), "--nx", "--quiet", "--interpreter=mi2"],
                            stdin=subprocess.PIPE, stdout=subprocess.PIPE, stderr=subprocess.STDOUT,
                            text=True, encoding="utf-8", errors="replace",
                            creationflags=subprocess.CREATE_NO_WINDOW)
    lines, messages = [], queue.Queue()
    token, attached, completed, camera_matches = 0, False, False, False

    def reader():
        for line in proc.stdout:
            lines.append(line.rstrip())
            messages.put(line.rstrip())
    threading.Thread(target=reader, daemon=True).start()

    def command(value, timeout=20):
        nonlocal token
        token += 1
        prefix = str(token)
        proc.stdin.write(prefix + value + "\n")
        proc.stdin.flush()
        deadline = time.monotonic() + timeout
        while time.monotonic() < deadline:
            try:
                line = messages.get(timeout=.1)
            except queue.Empty:
                if proc.poll() is not None:
                    raise RuntimeError("GDB exited before command completion")
                continue
            if line.startswith(prefix + "^"):
                if line.startswith(prefix + "^error"):
                    raise RuntimeError(line)
                return line
        raise TimeoutError(value)

    def console(value):
        return command("-interpreter-exec console " + json.dumps(value))

    def breakpoint(address, actions):
        import re
        response = command(f"-break-insert -h *{address:#x}")
        number = re.search(r'number="(\d+)"', response).group(1)
        command("-break-commands " + number + " " + " ".join(json.dumps(x) for x in actions))

    def dump(name, start, end):
        # This GDB's dump parser treats quotes as literal filename bytes.
        # Set its directory through MI, then use fixed whitespace-free leaves.
        return f"dump binary memory {name} {start} {end}"

    try:
        for value in ["pagination off", "confirm off", "auto-solib-add off", "mi-async on"]:
            command("-gdb-set " + value)
        command("-environment-cd " + json.dumps(output.as_posix()))
        command("-target-attach " + str(pid), timeout=30)
        attached = True
        console("set $seen_material=0")
        breakpoint(0x43A4EC, ["silent", 'printf "WALKER_THIS=%#x RET=%#x\\n", $ecx, *(unsigned int*)$esp',
                   dump("cylinder-live-object.bin", "$ecx", "$ecx+0x440"),
                   dump("camera-at-walker.bin", "0x4cbfc8", "0x4cc130"),
                   "disable 1", "enable 2", "continue"])
        breakpoint(0x43BD80, ["silent", 'printf "MATERIAL_THIS=%#x RET=%#x\\n", $ecx, *(unsigned int*)$esp',
                   dump("cylinder-live-material.bin", "$ecx", "$ecx+24"),
                   "set $seen_material=1", "enable 3", "continue"])
        breakpoint(0x46D460, ["silent", 'printf "DRAW_THIS=%#x RET=%#x\\n", $ecx, *(unsigned int*)$esp',
                   dump("cylinder-live-draw.bin", "$ecx", "$ecx+0x80"),
                   "bt 12", 'printf "TRACE_DONE\\n"', "disable 3"])
        command("-break-disable 2 3")
        command("-exec-continue")
        deadline = time.monotonic() + 30
        while time.monotonic() < deadline:
            try:
                line = messages.get(timeout=.1)
            except queue.Empty:
                continue
            if line.startswith("~") and json.loads(line[1:]).strip() == "TRACE_DONE":
                completed = True
                break
            if '*stopped,reason="signal-received"' in line:
                raise RuntimeError(line)
            if line.startswith("&"):
                raise RuntimeError("Breakpoint command failed: " + json.loads(line[1:]))
        if not completed:
            command("-exec-interrupt --all")
            raise TimeoutError("No complete walker/material/draw sequence")
        if (output / "camera-at-walker.bin").read_bytes() != expected:
            raise RuntimeError("Actual walker hit used a different camera")
        camera_matches = True
    finally:
        try:
            if attached:
                command("-target-detach")
                attached = False
            command("-gdb-exit", timeout=5)
            proc.wait(timeout=5)
        finally:
            (output / "gdb-mi.log").write_text("\n".join(lines))
            report = {"pid": pid, "case": case, "completed": completed, "debugger_attached": attached,
                      "camera_matches_reference_at_walker": camera_matches,
                      "gdb_exit_code": proc.poll(), "foreground": focus,
                      "scope": "One live READ sequence; no new calibrated value accepted"}
            (output / "trace-result.json").write_text(json.dumps(report, indent=2))
    return report


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("pid", type=int)
    parser.add_argument("--case", choices=["initial", "left"], required=True)
    parser.add_argument("--output", type=Path, default=ROOT / "build/tmp/cylinder-draw")
    parser.add_argument("--gdb", type=Path, default=Path(r"C:\progs\msys2\mingw32\bin\gdb.exe"))
    args = parser.parse_args()
    print(json.dumps(trace(args.pid, args.case, args.output, args.gdb), indent=2))
