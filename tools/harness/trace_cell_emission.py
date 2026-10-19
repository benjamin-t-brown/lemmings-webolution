"""Trace one paused MM_CLIMB_002 inline cell quad to its final Direct3D VB.

This is a READ-only GDB hardware-breakpoint trace. It requires an explicit PID
and a fresh output directory below build/tmp. It does not send game input or
modify process memory. A prediction JSON may be supplied for an ordered-field
comparison; the trace itself does not require one.
"""
from __future__ import annotations

import argparse
import hashlib
import json
import queue
import re
import struct
import subprocess
import sys
import threading
import time
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
STOCK_EXE = Path(r"C:\progs\Lemmings Revolution\Lemmings Revolution.exe")
STOCK_SHA256 = "32675bdb49866e76502713251fdfd2b09078c2481c72ca22ff68ebf4b3737a2f"
sys.path.insert(0, str(ROOT / "tools" / "harness"))
from acquire_game_foreground import acquire
from lrstate import attach, ui_state, u32
from memdump import k32, read_mem
import goto_level

MAIN = 0x004C6A08
WORLD = MAIN + 8
CAMERA = 0x004CBFC8
TARGET_GROUP = 3
TARGET_ENTRY = 2
TARGET_HELPER = 0x004599C0
HELPER_RETURN = 0x004599AF
EMITTER_BUILDER = 0x0046D460
EMITTER_CACHE_STORE = 0x0046D4F3
FINAL_D3D_CALL = 0x0046D5F3
FINAL_VERTEX_STRIDE = 32
EXPECTED_QUAD_SHA256 = "f150a763f292cf2e743e7c25cc1176bea4b4360ab32f575fd5b60d386546ad39"


def sha(data: bytes) -> str:
    return hashlib.sha256(data).hexdigest()


def f32(value: float) -> float:
    return struct.unpack("<f", struct.pack("<f", value))[0]


def parse_args() -> argparse.Namespace:
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument("--pid", type=int, required=True, help="PID of the already owned stock original")
    p.add_argument("--output", required=True, type=Path, help="new directory under build/tmp")
    p.add_argument("--prediction", type=Path, help="optional candidate prediction JSON for ordered comparison")
    p.add_argument("--expected-quad-sha256", default=EXPECTED_QUAD_SHA256)
    p.add_argument("--gdb", type=Path, default=Path(r"C:\progs\msys2\mingw32\bin\gdb.exe"))
    return p.parse_args()


class MI:
    def __init__(self, gdb: Path, pid: int, output: Path):
        self.proc = subprocess.Popen(
            [str(gdb), "--nx", "--quiet", "--interpreter=mi2"],
            stdin=subprocess.PIPE, stdout=subprocess.PIPE, stderr=subprocess.STDOUT,
            text=True, encoding="utf-8", errors="replace",
            creationflags=subprocess.CREATE_NO_WINDOW,
        )
        self.messages: queue.Queue[str] = queue.Queue()
        self.lines: list[str] = []
        self.token = 0
        self.attached = False
        self.running = False
        self.bp_numbers: list[str] = []
        self.output = output
        threading.Thread(target=self._reader, daemon=True).start()
        self.pid = pid

    def start(self) -> None:
        for setting in ("pagination off", "confirm off", "auto-solib-add off", "mi-async on"):
            self.command("-gdb-set " + setting)
        self.command("-environment-cd " + json.dumps(self.output.as_posix()))
        self.command(f"-target-attach {self.pid}", timeout=30)
        self.attached = True

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

    def wait_marker(self, marker: str, timeout: float = 30) -> None:
        deadline = time.monotonic() + timeout
        while time.monotonic() < deadline:
            try:
                line = self.messages.get(timeout=0.1)
            except queue.Empty:
                if self.proc.poll() is not None:
                    raise RuntimeError("GDB exited waiting for " + marker)
                continue
            if line.startswith("~") and marker == json.loads(line[1:]).strip():
                self.running = False
                return
            if line.startswith("*stopped"):
                self.running = False
                # This Windows GDB also emits a bare '*stopped' notification.
                # Only the actual console marker completes this wait; explicit
                # signals/exits are failures, while stop metadata alone is not.
                if 'reason="signal-received"' in line or 'reason="exited' in line:
                    raise RuntimeError("unexpected inferior stop: " + line)
            elif line.startswith("*running"):
                self.running = True
        raise TimeoutError(marker)

    def wait_any_stop(self, timeout: float = 10) -> str:
        deadline = time.monotonic() + timeout
        while time.monotonic() < deadline:
            try:
                line = self.messages.get(timeout=0.1)
            except queue.Empty:
                continue
            if line.startswith("*stopped"):
                self.running = False
                return line
        raise TimeoutError("inferior stop")

    def breakpoint(self, address: int, condition: str | None, actions: list[str]) -> str:
        line = self.command(f"-break-insert -h *{address:#x}")
        number = re.search(r'number="(\d+)"', line).group(1)
        if condition:
            self.command("-break-condition " + number + " " + json.dumps(condition))
        self.command("-break-commands " + number + " " + " ".join(json.dumps(a) for a in actions))
        self.bp_numbers.append(number)
        return number

    def resume(self) -> None:
        self.command("-exec-continue", timeout=5)
        self.running = True

    def cleanup(self, report: dict) -> None:
        errors = []
        try:
            if self.attached and self.running:
                try:
                    self.command("-exec-interrupt --all", timeout=10)
                    report["cleanup_stop_record"] = self.wait_any_stop(timeout=10)
                except Exception as exc:
                    errors.append("interrupt: " + repr(exc))
            if self.attached:
                # Remove debug-register breakpoints before detaching. A prior
                # resource-limit failure ended with an unhandled debug trap.
                try:
                    self.command("-break-delete", timeout=10)
                    report["breakpoints_deleted_before_detach"] = True
                except Exception as exc:
                    errors.append("delete breakpoints: " + repr(exc))
                try:
                    self.command("-target-detach", timeout=10)
                    self.attached = False
                except Exception as exc:
                    errors.append("detach: " + repr(exc))
            # Do not kill a debugger still attached to the original on failure.
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
            (self.output / "gdb-mi.log").write_text("\n".join(self.lines), encoding="utf-8")


def read_exact(handle, address: int, size: int) -> bytes:
    data = read_mem(handle, address, size)
    if data is None or len(data) != size:
        raise RuntimeError(f"incomplete process READ at {address:#x}/{size:#x}")
    return data


def main() -> int:
    args = parse_args()
    output = args.output.resolve()
    tmp = (ROOT / "build" / "tmp").resolve()
    try:
        output.relative_to(tmp)
    except ValueError:
        raise SystemExit(f"--output must be below {tmp}")
    if output.exists():
        raise SystemExit(f"--output must be new: {output}")
    output.mkdir(parents=True, exist_ok=False)
    report: dict = {
        "pid": args.pid,
        "selected_record": {"group": TARGET_GROUP, "entry": TARGET_ENTRY},
        "scope": "READ-only; target-conditioned hardware breakpoints; final indirect D3D VB capture",
        "completed": False,
    }
    handle = None
    mi: MI | None = None
    try:
        authority_sha = sha(STOCK_EXE.read_bytes())
        if authority_sha != STOCK_SHA256:
            raise RuntimeError("installed executable differs from the stock authority")
        report["authority_sha256"] = authority_sha
        report["foreground"] = acquire(args.pid)
        handle, attached_pid = attach()
        if not handle or attached_pid != args.pid:
            raise RuntimeError(f"process attach PID mismatch: {attached_pid}")
        if ui_state(handle) != 1 or goto_level.loaded_level(handle).lower() != r"compiled\mm_climb_002.lvl":
            raise RuntimeError("expected paused MM_CLIMB_002 precondition")

        group_head = WORLD + 0x208 + TARGET_GROUP * 0x10
        group = read_exact(handle, group_head, 0x10)
        group_count, group_capacity, group_data = struct.unpack_from("<II4xI", group)
        if group_count <= TARGET_ENTRY:
            raise RuntimeError("selected group entry absent")
        record_address = group_data + TARGET_ENTRY * 0x100
        record = read_exact(handle, record_address, 0x100)
        if not record[0xF8]:
            raise RuntimeError("selected inline quad is inactive")
        quad = record[8:0x88]
        quad_hash = sha(quad)
        if quad_hash.lower() != args.expected_quad_sha256.lower():
            raise RuntimeError(f"selected quad hash mismatch: {quad_hash}")
        material_index = struct.unpack_from("<I", record, 0xC8)[0]
        descriptor_index = material_index + 24
        material_head = read_exact(handle, WORLD + 0x168, 0x10)
        material_count, material_capacity, material_data = struct.unpack_from("<II4xI", material_head)
        if descriptor_index >= material_count:
            raise RuntimeError("descriptor is outside the live material table")
        descriptor = read_exact(handle, material_data + descriptor_index * 24, 24)
        texture_id, blend = struct.unpack_from("<II", descriptor)
        uv_offsets = struct.unpack_from("<2d", descriptor, 8)

        camera = read_exact(handle, CAMERA, 0x168)
        ref_path = ROOT / "tools" / "harness" / "fixtures" / "camera_a1" / "reference-state.json"
        expected_camera = bytes.fromhex(json.loads(ref_path.read_text(encoding="utf-8"))["camera"]["raw_hex"])
        if camera != expected_camera:
            raise RuntimeError("complete live camera differs from reference fixture")
        control_object = u32(handle, 0x004CBFB0)
        control = read_exact(handle, control_object, 0x80)
        cursor = read_exact(handle, control_object + 0x16A0, 16)
        grab = read_exact(handle, control_object + 0x16B3, 1)[0]
        yaw = struct.unpack_from("<d", control, 0x40)[0]
        zoom = struct.unpack_from("<i", control, 0x10)[0]
        elevation = struct.unpack_from("<d", control, 0x18)[0]
        cursor_x, cursor_y = struct.unpack("<2d", cursor)
        if not (0 <= cursor_x < 640 and 0 <= cursor_y < 480) or yaw != 32.0 or zoom != 0 or elevation != 0.0 or grab != 0:
            raise RuntimeError("semantic controls/gameplay-cursor guard failed")
        report.update({
            "precondition": {"ui_state": 1, "loaded_level": goto_level.loaded_level(handle),
                "camera_sha256": sha(camera), "camera_raw_hex": camera.hex(),
                "control_object": hex(control_object), "yaw": yaw, "zoom": zoom, "elevation": elevation,
                "gameplay_cursor_address": hex(control_object + 0x16A0),
                "gameplay_cursor": list(struct.unpack("<2d", cursor)),
                "grab": grab},
            "quad": {"record_address": hex(record_address), "record_sha256": sha(record),
                "quad_address": hex(record_address + 8), "quad_sha256": quad_hash,
                "source_indices": [struct.unpack_from("<I", quad, 0x20 + i * 0x18)[0] for i in range(4)],
                "uv_doubles": [list(struct.unpack_from("<2d", quad, 0x28 + i * 0x18)) for i in range(4)],
                "material_index": material_index, "descriptor_index": descriptor_index,
                "descriptor_texture_id": texture_id, "descriptor_blend": blend,
                "descriptor_uv_offsets": list(uv_offsets), "descriptor_raw_hex": descriptor.hex()},
        })

        prediction = None
        if args.prediction:
            prediction = json.loads(args.prediction.read_text(encoding="utf-8"))
            report["prediction"] = {"path": str(args.prediction.resolve()), "sha256": sha(args.prediction.read_bytes())}
        mi = MI(args.gdb, args.pid, output)
        mi.start()
        report["debugger_attached"] = True
        quad_address = record_address + 8
        entry_bp = mi.breakpoint(TARGET_HELPER, f"$ecx == {quad_address:#x}", [
            "silent", f'printf "TARGET_ENTER quad=%#x ret=%#x\\n",$ecx,*(unsigned int*)$esp',
            'printf "TARGET_ENTER_STOP\\n"',
        ])
        mi.resume(); mi.wait_marker("TARGET_ENTER_STOP")
        entry_lines = [json.loads(x[1:]).strip() for x in mi.lines if x.startswith("~") and "TARGET_ENTER quad=" in json.loads(x[1:])]
        if not entry_lines:
            raise RuntimeError("selected target entry did not fire")
        ret = int(re.search(r"ret=(0x[0-9a-fA-F]+)", entry_lines[-1]).group(1), 16)
        mi.command("-break-delete " + entry_bp)
        mi.console("set $target_active=1"); mi.console("set $builder_seen=0"); mi.console("set $cache_seen=0")
        mi.breakpoint(ret, None, ["silent", "set $target_active=0", 'printf "TARGET_EXTENT_CLOSED\\n"', "continue"])
        builder_actions = [
            "silent", "if $builder_seen == 0",
            'printf "BUILDER_POLYGON ecx=%#x ret=%#x count=%u capacity=%u data=%#x\\n",$ecx,*(unsigned int*)$esp,*(unsigned int*)$ecx,*(unsigned int*)($ecx+4),*(unsigned int*)($ecx+12)',
            "dump binary memory polygon-array-header.bin $ecx $ecx+0x20",
            "set $poly_data=*(unsigned int*)($ecx+12)", "set $poly_count=*(unsigned int*)$ecx",
            "dump binary memory polygon-pointer-list.bin $poly_data $poly_data+$poly_count*4",
            "bt 16", "set $builder_seen=1", "end", "continue",
        ]
        mi.breakpoint(EMITTER_BUILDER, "$target_active == 1 && $builder_seen == 0", builder_actions)
        cache_actions = [
            "silent", "if $cache_seen < 4", "set $cache_idx=*(unsigned int*)($ebp-4)",
            # At 46D4F3 the following local store has not happened yet; EDX is
            # the current source record pointer, unlike [EBP-0xC].
            "set $cache_src=$edx", "set $cache_dst=*(unsigned int*)($ebp-8)",
            'printf "CACHE_USE index=%u src=%#x dst=%#x\\n",$cache_idx,$cache_src,$cache_dst',
        ]
        for i in range(4):
            cache_actions += [f"if $cache_idx == {i}", f"dump binary memory cache-used-{i}.bin $cache_src $cache_src+0x58", "end"]
        cache_actions += ["set $cache_seen=$cache_seen+1", "end", "continue"]
        mi.breakpoint(EMITTER_CACHE_STORE, "$target_active == 1", cache_actions)
        final_actions = [
            "silent", "set $device_arg=*(unsigned int*)$esp", "set $primitive=*(unsigned int*)($esp+4)",
            "set $fvf=*(unsigned int*)($esp+8)", "set $vb=*(unsigned int*)($esp+12)",
            "set $vertex_count=*(unsigned int*)($esp+16)", "set $flags=*(unsigned int*)($esp+20)",
            'printf "FINAL pc=%#x this=%#x esp=%#x device=%#x primitive=%u fvf=%#x vb=%#x count=%u flags=%#x\\n",$pc,$ecx,$esp,$device_arg,$primitive,$fvf,$vb,$vertex_count,$flags',
            "dump binary memory final-stack.bin $esp $esp+0x40",
            f"dump binary memory final-vertex-buffer.bin $vb $vb+$vertex_count*{FINAL_VERTEX_STRIDE}",
            "dump binary memory camera-at-final-call.bin 0x4cbfc8 0x4cc130",
            "dump binary memory uv-globals-at-final-call.bin 0x58d2a0 0x58d2b0",
            'printf "FINAL_VERTEX_STOP\\n"',
        ]
        mi.breakpoint(FINAL_D3D_CALL, "$target_active == 1", final_actions)
        mi.resume(); mi.wait_marker("FINAL_VERTEX_STOP")
        report["trace_console"] = [json.loads(x[1:]).strip() for x in mi.lines if x.startswith("~") and any(k in json.loads(x[1:]) for k in ("BUILDER_POLYGON", "CACHE_USE", "FINAL pc=", "TARGET_ENTER quad="))]
        report["builder_callstack"] = [json.loads(x[1:]).strip() for x in mi.lines if x.startswith("~") and "#0  0x0046d460" in json.loads(x[1:])]
        (output / "gdb-mi.log").write_text("\n".join(mi.lines), encoding="utf-8")

        stack = (output / "final-stack.bin").read_bytes()
        args_stack = struct.unpack_from("<8I", stack)
        vb = (output / "final-vertex-buffer.bin").read_bytes()
        count = args_stack[4]
        if (args_stack[1], args_stack[2], count, args_stack[5]) != (6, 0x1C4, 4, 0):
            raise RuntimeError("device arguments differ from this bounded quad fixture")
        if len(vb) != count * FINAL_VERTEX_STRIDE:
            raise RuntimeError("final VB byte count does not equal count * 32")
        vertices = []
        for i in range(count):
            x, y, z, rhw, diffuse, specular, u, v = struct.unpack_from("<4fII2f", vb, i * FINAL_VERTEX_STRIDE)
            vertices.append({"index": i, "xy": [x, y], "z": z, "rhw": rhw,
                "diffuse": f"0x{diffuse:08X}", "specular": f"0x{specular:08X}", "uv": [u, v],
                "raw_hex": vb[i * FINAL_VERTEX_STRIDE:(i + 1) * FINAL_VERTEX_STRIDE].hex()})
        report["device_call"] = {"device_argument": hex(args_stack[0]), "primitive": args_stack[1], "fvf": hex(args_stack[2]),
            "vb_pointer": hex(args_stack[3]), "vertex_count": count, "flags": hex(args_stack[5]),
            "stack_hex": stack.hex(), "vertex_buffer_sha256": sha(vb), "vertex_buffer_bytes": len(vb), "vertices": vertices}
        report["polygon_array"] = {"raw_hex": (output / "polygon-array-header.bin").read_bytes().hex(),
            "pointer_list_hex": (output / "polygon-pointer-list.bin").read_bytes().hex()}
        header = (output / "polygon-array-header.bin").read_bytes()
        poly_count, poly_capacity, poly_flags, poly_data = struct.unpack_from("<IIII", header)
        poly_list = struct.unpack("<" + "I" * poly_count, (output / "polygon-pointer-list.bin").read_bytes())
        report["polygon_array"].update(count=poly_count, capacity=poly_capacity, flags=poly_flags,
            data_pointer=hex(poly_data), entry_addresses=[hex(x) for x in poly_list])
        cache_captures = {}
        for i in range(4):
            raw = (output / f"cache-used-{i}.bin").read_bytes()
            cache_captures[str(i)] = {"sha256": sha(raw), "raw_hex": raw.hex(),
                "source_z_f64": struct.unpack_from("<d", raw, 0x10)[0],
                "screen_xy_f64": list(struct.unpack_from("<2d", raw, 0x18)),
                "uv_f64": list(struct.unpack_from("<2d", raw, 0x28)),
                "diffuse_rg_f64": list(struct.unpack_from("<2d", raw, 0x38)),
                "diffuse_ba_f64": list(struct.unpack_from("<2d", raw, 0x48))}
        report["emitter_cache_records"] = cache_captures
        cache_ptrs = []
        cache_dst_ptrs = []
        for line in report["trace_console"]:
            m = re.search(r"CACHE_USE index=(\d+) src=(0x[0-9a-f]+) dst=(0x[0-9a-f]+)", line)
            if m:
                cache_ptrs.append((int(m.group(1)), int(m.group(2), 16)))
                cache_dst_ptrs.append(int(m.group(3), 16))
        report["cache_list_order"] = {
            "cache_source_pointers": [hex(pointer) for _, pointer in cache_ptrs],
            "indices": [index for index, _ in cache_ptrs],
            "matches_polygon_pointer_list": [pointer for _, pointer in cache_ptrs] == list(poly_list),
            "destination_pointers": [hex(pointer) for pointer in cache_dst_ptrs],
        }
        report["camera_at_final_call_sha256"] = sha((output / "camera-at-final-call.bin").read_bytes())
        if (output / "camera-at-final-call.bin").read_bytes() != expected_camera:
            raise RuntimeError("camera changed at the actual device-call hit")
        if not report["cache_list_order"]["matches_polygon_pointer_list"]:
            raise RuntimeError("captured emitter records differ from polygon list order")
        report["uv_globals_hex"] = (output / "uv-globals-at-final-call.bin").read_bytes().hex()

        if prediction:
            predicted = prediction["corners"]
            comparisons = []
            expected_diffuse_match = re.search(r"0x[0-9a-fA-F]{8}", prediction.get("predicted_emitted_diffuse", ""))
            expected_diffuse = expected_diffuse_match.group(0).upper() if expected_diffuse_match else None
            if len(predicted) != len(vertices):
                report["candidate_comparison_error"] = f"candidate has {len(predicted)} corners but emitted VB has {len(vertices)} vertices"
            for i, actual in enumerate(vertices):
                exp = predicted[i]
                comparisons.append({"index": i, "source_index": exp["source_index"],
                    "xy_delta": [actual["xy"][j] - exp["candidate_float_xy"][j] for j in range(2)],
                    "xy_within_0_01": all(abs(actual["xy"][j] - exp["candidate_float_xy"][j]) <= 0.01 for j in range(2)),
                    "uv_delta": [actual["uv"][j] - exp["uv_binary64"][j] for j in range(2)],
                    "uv_within_1e_5": all(abs(actual["uv"][j] - exp["uv_binary64"][j]) <= 1e-5 for j in range(2)),
                    "z_expected_float32_from_emitter_cache": f32(cache_captures[str(i)]["source_z_f64"]),
                    "z_exact": actual["z"] == f32(cache_captures[str(i)]["source_z_f64"]),
                    "rhw_expected_float32_0_25_over_float32_cache_z": f32(0.25 / f32(cache_captures[str(i)]["source_z_f64"])),
                    "rhw_exact": actual["rhw"] == f32(0.25 / f32(cache_captures[str(i)]["source_z_f64"])),
                    "diffuse": actual["diffuse"], "candidate_diffuse": expected_diffuse,
                    "diffuse_match": actual["diffuse"].upper() == expected_diffuse if expected_diffuse else None})
            report["candidate_comparison"] = comparisons
        report["completed"] = True
    except Exception as exc:
        report["error"] = repr(exc)
    finally:
        if mi:
            try:
                mi.cleanup(report)
            except Exception as exc:
                report["gdb_cleanup_error"] = repr(exc)
        if handle:
            try:
                k32.CloseHandle(handle)
            except Exception:
                pass
        (output / "trace-result.json").write_text(json.dumps(report, indent=2), encoding="utf-8")
        print(json.dumps(report, indent=2), flush=True)
    clean = (not report.get("debugger_attached_after_cleanup", True)
             and report.get("gdb_exit_code") == 0
             and not report.get("gdb_cleanup_error")
             and not report.get("gdb_cleanup_errors"))
    return 0 if report.get("completed") and clean else 1


if __name__ == "__main__":
    raise SystemExit(main())
