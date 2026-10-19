#!/usr/bin/env python3
"""Reversible draw isolation on an explicitly selected, paused stock process.

Run from a hidden worker (pythonw) so its console cannot steal game focus.
Requires the retained MM_CLIMB_002 camera. Launch/entry and wrapper setup are
separate; this tool leaves the selected game paused and alive. All temporary
code writes are logged before application, restored and re-read in finally.
No debugger, breakpoints, save writes, camera writes, or persistent patches.
"""
import argparse
import ctypes
import ctypes.wintypes as wt
import hashlib
import json
from pathlib import Path
import time

from PIL import Image, ImageChops
from acquire_game_foreground import acquire
from goto_level import loaded_level
from lrstate import PROCESS_VM_OPERATION, PROCESS_VM_WRITE, ui_state
from memdump import PROCESS_QUERY_INFORMATION, PROCESS_VM_READ, k32, read_mem
from screengrab import desktop_shot

ROOT = Path(__file__).resolve().parents[2]
AUTHORITY = "32675bdb49866e76502713251fdfd2b09078c2481c72ca22ff68ebf4b3737a2f"
TARGETS = (0x43a4ec, 0x4622fd, 0x4623a5, 0x462404, 0x462465)


def run(pid, output):
    output = Path(output).resolve()
    assert (ROOT / "build/tmp").resolve() in output.parents
    assert not output.exists() or not any(output.iterdir()), "use a new capture directory"
    output.mkdir(parents=True, exist_ok=True)
    access = PROCESS_QUERY_INFORMATION | PROCESS_VM_READ | PROCESS_VM_WRITE | PROCESS_VM_OPERATION
    h = k32.OpenProcess(access, False, pid)
    assert h
    user = ctypes.WinDLL("user32", use_last_error=True)
    user.SetProcessDPIAware()
    user.SetWindowPos.argtypes = [wt.HWND, wt.HWND, ctypes.c_int, ctypes.c_int, ctypes.c_int, ctypes.c_int, wt.UINT]
    user.GetWindowThreadProcessId.argtypes = [wt.HWND, ctypes.POINTER(wt.DWORD)]
    user.GetWindowLongW.argtypes = [wt.HWND, ctypes.c_int]
    user.GetClientRect.argtypes = [wt.HWND, ctypes.POINTER(wt.RECT)]
    user.ClientToScreen.argtypes = [wt.HWND, ctypes.POINTER(wt.POINT)]
    k32.FlushInstructionCache.argtypes = [wt.HANDLE, ctypes.c_void_p, ctypes.c_size_t]
    log = dict(pid=pid, writes=[], captures={})
    originals = {}

    def save():
        (output / "isolation.json").write_text(json.dumps(log, indent=2)+"\n", encoding="utf-8")

    def rd(va, size):
        data = read_mem(h, va, size)
        assert data is not None and len(data) == size
        return data

    def write(va, value, why):
        row = dict(address=hex(va), before=rd(va, 1).hex(), after=value.hex(), reason=why)
        log["writes"].append(row)
        save()
        n = ctypes.c_size_t()
        assert k32.WriteProcessMemory(h, ctypes.c_void_p(va), value, 1, ctypes.byref(n)) and n.value == 1
        assert k32.FlushInstructionCache(h, ctypes.c_void_p(va), 1)
        row["readback_ok"] = rd(va, 1) == value
        assert row["readback_ok"]
        save()

    def capture(name):
        focus = acquire(pid)
        hwnd = focus["hwnd"]
        assert focus["foreground_after_pid"] == pid
        assert rd(0x4cbfc8, len(camera)) == camera and ui_state(h) == 1
        was_topmost = bool(user.GetWindowLongW(hwnd, -20) & 8)
        try:
            assert user.SetWindowPos(hwnd, -1, 0, 0, 0, 0, 0x13)
            time.sleep(.5)
            rect, point = wt.RECT(), wt.POINT(0, 0)
            assert user.GetClientRect(hwnd, ctypes.byref(rect))
            assert user.ClientToScreen(hwnd, ctypes.byref(point))
            assert (rect.right, rect.bottom) == (640, 480)
            im = desktop_shot().crop((point.x, point.y, point.x+640, point.y+480)).convert("RGB")
            im.save(output / f"{name}.png")
            assert rd(0x4cbfc8, len(camera)) == camera
            log["captures"][name] = dict(focus=focus, sha256=hashlib.sha256((output / f"{name}.png").read_bytes()).hexdigest())
            save()
            return im
        finally:
            if not was_topmost:
                user.SetWindowPos(hwnd, -2, 0, 0, 0, 0, 0x13)

    def score(a, b, roi):
        diff = ImageChops.difference(a.crop(roi), b.crop(roi))
        return sum((i % 256)*n for i, n in enumerate(diff.histogram()))/(255*3*diff.width*diff.height)

    try:
        k32.QueryFullProcessImageNameW.argtypes = [wt.HANDLE, wt.DWORD, wt.LPWSTR, ctypes.POINTER(wt.DWORD)]
        pathbuf, size = ctypes.create_unicode_buffer(32768), wt.DWORD(32768)
        assert k32.QueryFullProcessImageNameW(h, 0, pathbuf, ctypes.byref(size))
        log["image_path"] = pathbuf.value
        assert hashlib.sha256(Path(pathbuf.value).read_bytes()).hexdigest() == AUTHORITY
        assert loaded_level(h).lower() == "compiled\\mm_climb_002.lvl"
        assert ui_state(h) == 1
        camera = bytes.fromhex(json.loads((ROOT / "tools/harness/fixtures/camera_a1/reference-state.json").read_text())["camera"]["raw_hex"])
        assert rd(0x4cbfc8, len(camera)) == camera
        for va in TARGETS:
            originals[va] = rd(va, 1)
            assert originals[va] == b"\x55"
        baseline = capture("baseline")
        comparisons = {}
        for name, targets in (("no-walker", TARGETS[:1]), ("no-cell-routes", TARGETS[1:])):
            try:
                for va in targets:
                    write(va, b"\xc3", name)
                time.sleep(.4)
                changed = capture(name)
            finally:
                for va in targets:
                    write(va, originals[va], f"restore {name}")
            time.sleep(.4)
            restored = capture(f"restored-{name}")
            comparisons[name] = {"upper_band_mae": score(baseline, changed, (100,0,540,12)),
                                 "tube_roi_mae": score(baseline, changed, (100,80,280,410)),
                                 "restored_tube_roi_mae": score(baseline, restored, (100,80,280,410)),
                                 "restored_upper_band_mae": score(baseline, restored, (100,0,540,12)),
                                 "restored_full_frame_difference_bbox": ImageChops.difference(baseline, restored).getbbox()}
        log["comparisons"] = comparisons
        assert all(v["upper_band_mae"] == 0 and v["tube_roi_mae"] > .02 and
                   v["restored_tube_roi_mae"] == 0 and v["restored_upper_band_mae"] == 0
                   for v in comparisons.values())
        log["success"] = True
    except Exception as exc:
        log["error"] = repr(exc)
        raise
    finally:
        try:
            for va, value in originals.items():
                if rd(va, 1) != value:
                    write(va, value, "unconditional final restore")
            log["code_restored"] = all(rd(va, 1) == value for va, value in originals.items())
            log["paused_at_finish"] = ui_state(h) == 1
            if "camera" in locals():
                log["camera_unchanged"] = rd(0x4cbfc8, len(camera)) == camera
            save()
        finally:
            k32.CloseHandle(h)
    return log


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--pid", type=int, required=True)
    parser.add_argument("--output", type=Path, required=True)
    args = parser.parse_args()
    run(args.pid, args.output)
