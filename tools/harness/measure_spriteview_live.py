"""One owned, logged A3 stock-game observation and view-0 comparison."""
from __future__ import annotations

import ctypes
import hashlib
import importlib.util
import json
import argparse
from pathlib import Path
import struct
import subprocess
import sys
import time
import traceback

ROOT = Path(r"C:\progs\lemmings-webolution")
parser = argparse.ArgumentParser(description=__doc__)
parser.add_argument("--out", type=Path, default=ROOT / "build/tmp/a3-sprite-view/live")
WORK = parser.parse_args().out.resolve()
INSTALL = Path(r"C:\progs\Lemmings Revolution")
WORK.mkdir(parents=True, exist_ok=True)
sys.path.insert(0, str(ROOT / "tools/harness"))
from acquire_game_foreground import acquire  # noqa: E402
from goto_level import loaded_level  # noqa: E402
from lrinput import DIK, LRInput  # noqa: E402
from lrstate import attach, i32, ui_state  # noqa: E402
from memdump import find_pids, k32, read_mem  # noqa: E402
from measure_spawn import actor_ptrs  # noqa: E402


def load(name, path):
    spec = importlib.util.spec_from_file_location(name, path)
    mod = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(mod)
    return mod


cap = load("a3_capture", ROOT / "build/tmp/a1-zoom-elevation/capture_branches.py")
entry = load("a3_entry", ROOT / "build/tmp/a1-zoom-elevation/enter_fast.py")
entry.WORK = WORK

log = {"writes": [], "samples": [], "visual": {}, "camera_inputs": []}
proc = None
handle = None
suspend_handle = None
backups = {}
ntdll = ctypes.WinDLL("ntdll")
ntdll.NtSuspendProcess.argtypes = [ctypes.c_void_p]
ntdll.NtSuspendProcess.restype = ctypes.c_long
ntdll.NtResumeProcess.argtypes = [ctypes.c_void_p]
ntdll.NtResumeProcess.restype = ctypes.c_long
process_suspended = False
conf_path = INSTALL / "dgVoodoo.conf"
conf_original = conf_path.read_bytes()
install_hashes = {n: hashlib.sha256((INSTALL / n).read_bytes()).hexdigest()
                  for n in ("Lemmings Revolution.exe", "SavedGame.dat", "dgVoodoo.conf")}
stock_hash = "32675bdb49866e76502713251fdfd2b09078c2481c72ca22ff68ebf4b3737a2f"
camera_addr = 0x004CBFC8
camera_size = 0x168
actor_list = 0x004CBB18
visual_policy = {
    "written_before_capture": True,
    "crop": "project actor+0x108 origin by live 12-double composed camera/projection matrix at 0x0058D3D8; screen=(319+x/z,239-y/z); fixed 64x64 ROI centered there, clipped to 640x480",
    "metric": "RGB mean absolute pixel error over that crop, selected view versus force-view-0 screenshot",
    "pass_threshold": "normalized RGB MAE strictly greater than 0.02; changed crop pixels must be nonzero; lower values fail",
    "mutation_requirement": "with the native chooser temporarily replaced by a stack-correct RET 4 stub, changing only the actor view-index dword changes the sprite-local crop; restore the exact prior index and chooser bytes",
    "no_rescale_or_alignment": True,
    "actor_identity": "actor-list pointer read from 0x004CBB18; selected by nonzero live view index and current directional-entry count 5 or 8",
}
(WORK / "visual-policy.json").write_text(json.dumps(visual_policy, indent=2) + "\n")


class FocusedInput(LRInput):
    def tap(self, dik, hold_frames=2):
        log.setdefault("foreground", []).append(acquire(proc.pid))
        return super().tap(dik, hold_frames)


entry.LRInput = FocusedInput
old_find = entry.find_menu_index


def find_menu_index(h, inp):
    log.setdefault("foreground", []).append(acquire(proc.pid))
    inp._set_u32(32, 1)
    entry.aim_at(h, inp, (620, 460))
    return old_find(h, inp)


entry.find_menu_index = find_menu_index


def save():
    (WORK / "live-oracle.json").write_text(json.dumps(log, indent=2) + "\n")


def rd(addr, size):
    data = read_mem(handle, addr, size)
    if data is None or len(data) != size:
        raise RuntimeError(f"incomplete read {addr:#x}/{size}")
    return data


def wr(addr, data, reason):
    before = rd(addr, len(data))
    record = {"address": hex(addr), "before": before.hex(), "after": data.hex(), "reason": reason}
    log["writes"].append(record)
    save()
    n = ctypes.c_size_t(0)
    if not k32.WriteProcessMemory(handle, ctypes.c_void_p(addr), data, len(data), ctypes.byref(n)) or n.value != len(data):
        raise OSError(f"WriteProcessMemory failed at {addr:#x}")
    record["readback_ok"] = rd(addr, len(data)) == data
    if not record["readback_ok"]:
        raise RuntimeError(f"write readback failed at {addr:#x}")
    save()


def suspend_owned():
    global process_suspended
    if process_suspended:
        return True
    status = ntdll.NtSuspendProcess(suspend_handle or handle)
    process_suspended = status == 0
    log.setdefault("suspension_events", []).append({"action": "suspend", "status": status,
                                                     "success": process_suspended})
    save()
    return process_suspended


def resume_owned():
    global process_suspended
    if not process_suspended:
        return False
    status = ntdll.NtResumeProcess(suspend_handle or handle)
    process_suspended = status != 0
    log.setdefault("suspension_events", []).append({"action": "resume", "status": status,
                                                     "success": not process_suspended})
    save()
    return not process_suspended


def actor_rows():
    count = i32(handle, actor_list)
    ptr_array = struct.unpack("<I", rd(actor_list + 0x0C, 4))[0]
    if count is None or not 0 <= count <= 2000 or not ptr_array:
        raise RuntimeError(f"invalid actor list count={count}, ptr={ptr_array:#x}")
    rows = []
    log["actor_vector_entries"] = []
    for actor in actor_ptrs(handle):
        if not actor:
            continue
        vt = struct.unpack("<I", rd(actor, 4))[0]
        log["actor_vector_entries"].append({"actor": hex(actor), "vtable": hex(vt)})
        if vt not in (0x004B10E8, 0x004B0F6C):
            continue
        c = actor + 0x128
        fields = rd(c + 0x1B4, 0x114)
        index = struct.unpack_from("<i", fields, 4)[0]  # +1b8
        mirror = fields[0]
        strip = struct.unpack_from("<i", fields, 0x14)[0]  # +1c8
        frame = struct.unpack_from("<i", fields, 0x18)[0]  # +1cc
        rate = struct.unpack_from("<d", fields, 0x24)[0]  # +1d8
        container = struct.unpack_from("<I", fields, 0x44)[0]  # +1f8
        count_views = None
        record_addr = None
        action_addr = None
        temporal_base = None
        if container:
            entries = struct.unpack("<I", rd(container + 0x0C, 4))[0]
            if entries and strip >= 0 and frame >= 0:
                action_addr = entries + strip * 0x98
                temporal_base = struct.unpack("<I", rd(action_addr + 0x0C, 4))[0]
                record_addr = temporal_base + frame * 0x10 if temporal_base else None
                if record_addr:
                    count_raw = struct.unpack("<I", rd(record_addr, 4))[0]
                    count_views = count_raw if count_raw in (1, 5, 8) else None
        model = rd(actor + 0x108, 0x18)
        model_xyz = struct.unpack("<3d", model)
        facing_raw = rd(actor + 0x40, 4)
        facing = struct.unpack("<i", facing_raw)[0]
        gravity_raw = rd(actor + 0x44, 8)
        matrix = struct.unpack("<12d", rd(0x0058D3D8, 12 * 8))
        cx, cy, cz = model_xyz
        px = matrix[0] * cx + matrix[3] * cy + matrix[6] * cz + matrix[9]
        py = matrix[1] * cx + matrix[4] * cy + matrix[7] * cz + matrix[10]
        pz = matrix[2] * cx + matrix[5] * cy + matrix[8] * cz + matrix[11]
        screen_anchor = None if pz <= 0 else [319.0 + px / pz, 239.0 - py / pz]
        scores = None
        camera_eye = None
        if count_views in (1, 5, 8):
            camera_eye = struct.unpack("<3d", rd(0x004CC018, 0x18))
            toward_actor = tuple(model_xyz[k] - camera_eye[k] for k in range(3))
            magnitude = sum(v * v for v in toward_actor) ** 0.5
            if magnitude:
                toward_actor = tuple(v / magnitude for v in toward_actor)
            basis = struct.unpack("<3d", rd(c + 0x1E0, 0x18))
            vectors = []
            import math
            pi = 3.1415927410125732
            # Native code starts at angleField+0.0625, then adds 0.1875
            # for the second test vector (total angleField+0.25).
            first_angle = rate + 0.0625
            for a0 in (first_angle, first_angle + 0.1875):
                a = a0 * 2.0 * pi
                co, si = math.cos(a), math.sin(a)
                vx = si * basis[0] + co * basis[2]
                vz = co * basis[0] - si * basis[2]
                vectors.append((vz, basis[1], vx))
            scores = [sum(toward_actor[k] * vectors[j][k] for k in range(3)) for j in range(2)]
            scores = [s + 2.0 if s < -1.0 else s - 2.0 if s > 1.0 else s for s in scores]
        rows.append({
            "actor": hex(actor), "vtable": hex(vt), "position": list(model_xyz),
            "facing_raw_hex": facing_raw.hex(), "facing_i32": facing,
            "gravity_raw_hex": gravity_raw.hex(), "view_index": index,
            "mirror": mirror, "strip": strip, "frame": frame, "angle_field_1d8": rate,
            "container": hex(container), "directional_entry_count": count_views,
            "action_record": hex(action_addr) if action_addr else None,
            "temporal_array": hex(temporal_base) if temporal_base else None,
            "entry_record": hex(record_addr) if record_addr else None,
            "directional_entry_count_valid": count_views in (1, 5, 8),
            "projected_actor_origin": screen_anchor,
            "camera_eye_4cc018": list(camera_eye) if camera_eye else None,
            "orientation_basis_1e0": list(struct.unpack("<3d", rd(c + 0x1E0, 0x18))),
            "camera_to_actor_scores_candidate": scores,
            "selection_field_addresses": {"view": hex(c + 0x1B8), "mirror": hex(c + 0x1B4)},
        })
    return rows


def lemming_ptrs():
    found = []
    for actor in actor_ptrs(handle):
        if actor and struct.unpack("<I", rd(actor, 4))[0] in (0x004B10E8, 0x004B0F6C):
            found.append(actor)
    return found


def shot(name, hwnd):
    log.setdefault("foreground", []).append(acquire(proc.pid))
    if ui_state(handle) != 1:
        raise RuntimeError(f"screenshot requires paused ui_state 1, got {ui_state(handle)}")
    image, origin = cap.shot(hwnd)
    if image.size != (640, 480):
        raise RuntimeError(f"capture client size {image.size}, expected 640x480")
    image.convert("RGB").save(WORK / f"{name}.png")
    log.setdefault("screenshots", {})[name] = {"origin": origin, "camera_sha256": hashlib.sha256(rd(camera_addr, camera_size)).hexdigest()}
    save()
    return image.convert("RGB")


try:
    assert hashlib.sha256((INSTALL / "Lemmings Revolution.exe").read_bytes()).hexdigest() == stock_hash
    assert not find_pids("Lemmings"), "another game process is running; refusing ownership"
    windowed = (ROOT / "build/tmp/a1-zoom-elevation/dgVoodoo.windowed.conf").read_bytes()
    conf_path.write_bytes(windowed)
    proc = subprocess.Popen([str(INSTALL / "Lemmings Revolution.exe")], cwd=INSTALL)
    log["pid"] = proc.pid
    save()
    time.sleep(5)
    if entry.main() != 0:
        raise RuntimeError("fast entry failed")
    handle, pid = attach(write=True)
    if not handle or pid != proc.pid:
        raise RuntimeError(f"attached PID {pid} != owned PID {proc.pid}")
    log["pid"] = pid
    suspend_handle = k32.OpenProcess(0x0800 | 0x0010 | 0x0020 | 0x0008 | 0x0400,
                                    False, pid)
    if not suspend_handle:
        raise OSError(f"OpenProcess(SUSPEND_RESUME|VM access) failed: {ctypes.get_last_error()}")
    log["stock_executable_sha256"] = hashlib.sha256((INSTALL / "Lemmings Revolution.exe").read_bytes()).hexdigest()
    log["loaded_level"] = loaded_level(handle)
    log["ui_state"] = ui_state(handle)
    if log["loaded_level"].lower() != r"compiled\mm_climb_002.lvl" or log["ui_state"] != 1:
        raise RuntimeError(f"unexpected level/pause state: {log['loaded_level']}, {log['ui_state']}")
    hwnd = cap.find_hwnd(pid)
    backups[camera_addr] = rd(camera_addr, camera_size)
    log["camera_baseline_sha256"] = hashlib.sha256(backups[camera_addr]).hexdigest()
    # The entry helper pauses at the first playable frame, before this level's
    # hatch has released an actor. Run at ordinary game speed until the first
    # actor is published, then pause immediately and read the actor list.
    if not lemming_ptrs():
        with LRInput() as inp:
            inp.enable()
            inp.clear()
            log.setdefault("foreground", []).append(acquire(pid))
            inp.tap("P", 2)
            deadline = time.monotonic() + 4.0
            while time.monotonic() < deadline and not lemming_ptrs():
                time.sleep(0.1)
            log["actor_wait_seconds"] = round(4.0 - max(0.0, deadline - time.monotonic()), 3)
            if lemming_ptrs():
                time.sleep(8.0)  # let transient level-start text clear
            inp.tap("P", 2)
            for _ in range(20):
                if ui_state(handle) == 1:
                    break
                time.sleep(0.05)
            inp.release()
        if ui_state(handle) != 1:
            raise RuntimeError(f"failed to repause after first actor wait, ui={ui_state(handle)}")
    log["actor_list_count"] = i32(handle, actor_list)
    log["actor_list_pointer"] = hex(struct.unpack("<I", rd(actor_list + 0x0C, 4))[0])
    rows = actor_rows()
    log["samples"].append({"label": "baseline", "actors": rows})
    candidate = next((r for r in rows if r["directional_entry_count"] in (5, 8)
                      and r["view_index"] != 0), None)
    if candidate:
        actor = int(candidate["actor"], 16)
        field = actor + 0x128 + 0x1B8
        saved_field = rd(field, 4)
        current_mirror = rd(field - 4, 1)
        # The visual policy was saved before any image; crop uses this actor's
        # four original projected corners and is fixed before force-view-0.
        if candidate["projected_actor_origin"] is None:
            raise RuntimeError("actor transform origin has nonpositive camera depth")
        sx, sy = candidate["projected_actor_origin"]
        rect = (max(0, int(sx // 1) - 32), max(0, int(sy // 1) - 32),
                min(640, int(sx // 1) + 32), min(480, int(sy // 1) + 32))
        if rect[2] <= rect[0] or rect[3] <= rect[1]:
            raise RuntimeError(f"invalid sprite-local crop {rect}")
        log["visual"]["actor"] = candidate
        log["visual"]["crop"] = list(rect)
        visual_policy["crop_rect_pixels"] = list(rect)
        (WORK / "visual-policy.json").write_text(json.dumps(visual_policy, indent=2) + "\n")
        save()
        image = shot("selected", hwnd)
        if not suspend_owned():
            raise RuntimeError("could not suspend owned process for selector-call patch")
        selector_entry = 0x0046840B
        original_entry = rd(selector_entry, 3)
        if original_entry != bytes.fromhex("558bec"):
            raise RuntimeError(f"unexpected selector entry bytes {original_entry.hex()}")
        wr(selector_entry, b"\xc2\x04\x00", "temporary RET 4 at selector entry to hold chosen view")
        wr(field, b"\0\0\0\0", "force view zero for same paused actor frame")
        if not resume_owned():
            raise RuntimeError("could not resume process for force-view render")
        log["visual"]["view_readback_before_capture"] = struct.unpack("<i", rd(field, 4))[0]
        with LRInput() as inp:
            inp.enable()
            inp.clear()
            inp.wait_frames(2)
            log["visual"]["view_readback_after_two_render_polls"] = struct.unpack("<i", rd(field, 4))[0]
            forced = shot("force-view-0", hwnd)
            inp.release()
        from PIL import ImageChops
        diff = ImageChops.difference(image.crop(rect), forced.crop(rect))
        log["visual"]["force_view_0_rgb_mae"] = sum((i % 256) * n for i, n in enumerate(diff.histogram())) / (255 * 3 * max(1, diff.width * diff.height))
        log["visual"]["changed_crop_pixels"] = sum(1 for p in diff.getdata() if p != (0, 0, 0))
        log["visual"]["passes_predeclared_gate"] = (log["visual"]["force_view_0_rgb_mae"] > 0.02 and
                                                   log["visual"]["changed_crop_pixels"] > 0)
        log["visual"]["crop_size"] = [diff.width, diff.height]
        if not suspend_owned():
            raise RuntimeError("could not suspend owned process to restore selector mutation")
        wr(field, saved_field, "restore actor view index")
        wr(selector_entry, original_entry, "restore selector entry bytes")
        if not resume_owned():
            raise RuntimeError("could not resume process after selector mutation restoration")
        log["visual"]["mirror_unmodified"] = rd(field - 4, 1) == current_mirror
        log["visual"]["view_restored"] = rd(field, 4) == saved_field
    else:
        (WORK / "visual-policy.json").write_text(json.dumps(visual_policy, indent=2) + "\n")
        shot("selected", hwnd)
        log["visual"]["unsettled"] = "No visible actor in the paused baseline had a nonzero five/eight-entry selector index."
    save()

    # Try a bounded 4-frame camera yaw step with the same actor list frozen.
    # NUMPAD4 is READ/MEASURED as left rotation when right-button grab is active.
    rows_before = actor_rows()
    with LRInput() as inp:
        inp.enable()
        inp.clear()
        log.setdefault("foreground", []).append(acquire(pid))
        inp.button(1, True)
        inp.wait_frames(2)
        inp.key(0x4B, True)  # DirectInput NUMPAD4 scan code.
        inp.wait_frames(4)
        inp.key(0x4B, False)
        inp.wait_frames(2)
        inp.button(1, False)
        inp.wait_frames(2)
        camera_rotated = rd(camera_addr, camera_size)
        rows_rotated = actor_rows()
        log["camera_inputs"].append({"action": "right-button hold + NUMPAD4 for four polls",
                                     "camera_changed": camera_rotated != backups[camera_addr],
                                     "camera_changed_offsets": [i for i, (a, b) in enumerate(zip(backups[camera_addr], camera_rotated)) if a != b],
                                     "camera_sha256": hashlib.sha256(camera_rotated).hexdigest(),
                                     "actors_before": rows_before, "actors_after": rows_rotated})
        inp.release()
    save()
    # Undo a measured key yaw with equal NUMPAD6 polls; if any camera byte
    # remains different, restore the full pre-input block and verify it.
    with LRInput() as inp:
        inp.enable()
        inp.clear()
        log.setdefault("foreground", []).append(acquire(pid))
        inp.key(0x4D, True)  # NUMPAD6
        inp.wait_frames(4)
        inp.key(0x4D, False)
        inp.wait_frames(2)
        inp.release()
    if rd(camera_addr, camera_size) != backups[camera_addr]:
        wr(camera_addr, backups[camera_addr], "restore complete camera block after balanced yaw inputs")
    log["camera_restored_exactly_after_live_input"] = rd(camera_addr, camera_size) == backups[camera_addr]
    log["baseline_actor_list_after_camera_restore"] = actor_rows()
    save()
    log["success"] = True
except Exception:
    log["error"] = traceback.format_exc()
finally:
    try:
        if handle and proc and proc.poll() is None:
            log["final_process_suspended"] = suspend_owned()
            if log.get("suspension_events"):
                log["final_suspend_status"] = log["suspension_events"][-1]["status"]
        if handle and camera_addr in backups and rd(camera_addr, camera_size) != backups[camera_addr]:
            wr(camera_addr, backups[camera_addr], "final unconditional camera restoration")
        if handle:
            initial = {}
            for write in log["writes"]:
                initial.setdefault(write["address"], bytes.fromhex(write["before"]))
            for address, original in backups.items():
                initial[hex(address)] = original
            for addr, expected in initial.items():
                if rd(int(addr, 16), len(expected)) != expected:
                    wr(int(addr, 16), expected, "final unconditional restoration of logged target")
            log["all_logged_write_targets_restored"] = all(
                rd(int(addr, 16), len(expected)) == expected for addr, expected in initial.items())
            if camera_addr in backups:
                camera_final = rd(camera_addr, camera_size)
                log["camera_restored_exactly"] = camera_final == backups[camera_addr]
                log["camera_final_diff_offsets"] = [i for i, (a, b) in enumerate(zip(backups[camera_addr], camera_final)) if a != b]
            else:
                log["camera_restored_exactly"] = True
            log["success"] = bool(log.get("success") and log["all_logged_write_targets_restored"] and
                                  log["camera_restored_exactly"])
            if process_suspended:
                resume_owned()
                log["final_resume_status"] = log["suspension_events"][-1]["status"]
            if suspend_handle:
                k32.CloseHandle(suspend_handle)
                suspend_handle = None
            k32.CloseHandle(handle)
    except Exception:
        log.setdefault("cleanup_errors", []).append(traceback.format_exc())
    try:
        with LRInput() as inp:
            inp.release()
    except Exception:
        pass
    if proc:
        proc.terminate()
        proc.wait(timeout=10)
        log["owned_process_stopped"] = proc.poll() is not None
    conf_path.write_bytes(conf_original)
    log["install_hashes_after"] = {n: hashlib.sha256((INSTALL / n).read_bytes()).hexdigest()
                                   for n in install_hashes}
    log["install_matches_before"] = log["install_hashes_after"] == install_hashes
    log["install_hashes_before"] = install_hashes
    save()
    print(json.dumps(log, indent=2))
