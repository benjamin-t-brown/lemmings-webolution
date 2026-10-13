"""Get into a playable level from cold start / Continue / level map."""
from __future__ import annotations

import ctypes
import ctypes.wintypes as wt
import sys
import time
from pathlib import Path

from PIL import Image

sys.path.insert(0, str(Path(__file__).resolve().parent))
from lrinput import LRInput, DIK
from memdump import (
    find_pids,
    read_mem,
    k32,
    PROCESS_VM_READ,
    PROCESS_QUERY_INFORMATION,
    scan_masks,
)
from enter_play import level_info

OUT = Path(r"C:\progs\lemmings-webolution\build\live_dump")
user32 = ctypes.WinDLL("user32")
gdi32 = ctypes.WinDLL("gdi32")


def find_hwnd(pid: int) -> int | None:
    found: list[int] = []

    @ctypes.WINFUNCTYPE(ctypes.c_bool, wt.HWND, wt.LPARAM)
    def cb(hwnd, _lp):
        if user32.IsWindowVisible(hwnd):
            p = wt.DWORD()
            user32.GetWindowThreadProcessId(hwnd, ctypes.byref(p))
            if p.value == pid and user32.GetWindowTextLengthW(hwnd) > 0:
                found.append(int(hwnd))
        return True

    user32.EnumWindows(cb, 0)
    return found[0] if found else None


def shot(hwnd: int, path: Path | None = None) -> Image.Image:
    rect = wt.RECT()
    user32.GetClientRect(hwnd, ctypes.byref(rect))
    w, h = rect.right, rect.bottom
    hdc = user32.GetDC(hwnd)
    memdc = gdi32.CreateCompatibleDC(hdc)
    hbmp = gdi32.CreateCompatibleBitmap(hdc, w, h)
    old = gdi32.SelectObject(memdc, hbmp)
    user32.PrintWindow(hwnd, memdc, 2)

    class BIH(ctypes.Structure):
        _fields_ = [
            ("biSize", wt.DWORD),
            ("biWidth", wt.LONG),
            ("biHeight", wt.LONG),
            ("biPlanes", wt.WORD),
            ("biBitCount", wt.WORD),
            ("biCompression", wt.DWORD),
            ("biSizeImage", wt.DWORD),
            ("biXPelsPerMeter", wt.LONG),
            ("biYPelsPerMeter", wt.LONG),
            ("biClrUsed", wt.DWORD),
            ("biClrImportant", wt.DWORD),
        ]

    bih = BIH()
    bih.biSize = 40
    bih.biWidth = w
    bih.biHeight = -h
    bih.biPlanes = 1
    bih.biBitCount = 24
    row = (w * 3 + 3) & ~3
    buf = (ctypes.c_ubyte * (row * h))()
    gdi32.GetDIBits(memdc, hbmp, 0, h, buf, ctypes.byref(bih), 0)
    gdi32.SelectObject(memdc, old)
    gdi32.DeleteObject(hbmp)
    gdi32.DeleteDC(memdc)
    user32.ReleaseDC(hwnd, hdc)
    img = Image.frombytes("RGB", (w, h), bytes(buf), "raw", "BGR", row, 1)
    if path:
        img.resize((640, 360)).save(path, quality=70)
    return img


def mean_rgb(img: Image.Image) -> float:
    small = img.resize((80, 45))
    px = list(small.getdata())
    return sum(sum(p) for p in px) / (len(px) * 3)


def is_continue(img: Image.Image) -> bool:
    # dark screen + "Continue" area mostly black
    return mean_rgb(img) < 40


def is_level_map(img: Image.Image) -> bool:
    # parchment mid-tones with yellow nodes
    m = mean_rgb(img)
    if m < 80 or m > 200:
        return False
    px = img.load()
    w, h = img.size
    yel = 0
    for y in range(h // 3, 2 * h // 3, 4):
        for x in range(w // 5, w // 2, 4):
            r, g, b = px[x, y]
            if r > 190 and g > 150 and b < 100:
                yel += 1
    return yel > 40


def find_crosshair(img: Image.Image):
    px = img.load()
    w, h = img.size
    xs, ys = [], []
    for y in range(0, h, 2):
        for x in range(0, w, 2):
            r, g, b = px[x, y]
            if g > 180 and g > r + 60 and g > b + 60 and r < 140 and b < 140:
                xs.append(x)
                ys.append(y)
    if len(xs) < 8:
        return None
    return sum(xs) / len(xs), sum(ys) / len(ys)


def find_yellow_mid(img: Image.Image):
    px = img.load()
    w, h = img.size
    pts = []
    for y in range(0, h, 2):
        for x in range(0, w // 2, 2):
            r, g, b = px[x, y]
            if r > 190 and g > 150 and b < 100 and r > b + 80:
                pts.append((x, y))
    if len(pts) < 20:
        return None
    xmin = min(x for x, _ in pts)
    left = [p for p in pts if p[0] < xmin + 80]
    lefts = sorted(left, key=lambda p: p[1])
    mid = lefts[len(lefts) // 3 : 2 * len(lefts) // 3]
    return sum(x for x, _ in mid) / len(mid), sum(y for _, y in mid) / len(mid)


def main():
    OUT.mkdir(parents=True, exist_ok=True)
    procs = find_pids("Lemmings")
    if not procs:
        print("no game")
        return 1
    pid = procs[0][0]
    hwnd = find_hwnd(pid)
    print("pid", pid, "hwnd", hwnd, flush=True)
    user32.ShowWindow(hwnd, 9)
    user32.SetForegroundWindow(hwnd)
    h = k32.OpenProcess(PROCESS_VM_READ | PROCESS_QUERY_INFORMATION, False, pid)

    with LRInput() as inp:
        f0 = inp.mouse_frame
        time.sleep(0.3)
        print("poll", (inp.mouse_frame - f0) / 0.3, flush=True)
        inp.enable()

        # Dismiss intro / Continue
        for i in range(20):
            img = shot(hwnd, OUT / f"nav_{i}.jpg")
            m = mean_rgb(img)
            print(f"nav {i} mean={m:.1f} continue={is_continue(img)} map={is_level_map(img)}", flush=True)
            if is_level_map(img):
                break
            inp.tap(DIK["RETURN"])
            time.sleep(0.6)
            if i % 3 == 2:
                inp.tap(DIK["SPACE"])
                time.sleep(0.4)
        else:
            print("never reached level map")
            inp.release()
            return 1

        # Move crosshair onto mid yellow node
        for attempt in range(8):
            img = shot(hwnd, OUT / f"aim_{attempt}.jpg")
            c = find_crosshair(img)
            t = find_yellow_mid(img)
            print(f"aim {attempt} cursor={c} target={t}", flush=True)
            if not c or not t:
                inp.glide(0, 40, steps=5)
                continue
            dx, dy = int(t[0] - c[0]), int(t[1] - c[1])
            if abs(dx) < 25 and abs(dy) < 25:
                print("on target; click", flush=True)
                inp.click()
                time.sleep(0.4)
                inp.click()
                time.sleep(2.0)
                inp.tap(DIK["RETURN"])
                time.sleep(3.0)
                break
            # scale: if previous move undershot, multiply
            steps = max(15, (abs(dx) + abs(dy)) // 30)
            print(f"  glide {dx},{dy} steps={steps}", flush=True)
            inp.glide(dx, dy, steps=steps, per_step_frames=1)
            time.sleep(0.15)
        else:
            print("failed to aim")
            inp.release()
            return 1

        img = shot(hwnd, OUT / "in_level.jpg")
        print(
            "after enter mean",
            mean_rgb(img),
            "map",
            is_level_map(img),
            "info",
            level_info(h),
            flush=True,
        )
        if is_level_map(img):
            print("still on map — mouse may not be moving; trying consume=0 burst")
            c = find_crosshair(img)
            t = find_yellow_mid(img)
            if c and t:
                dx, dy = int(t[0] - c[0]), int(t[1] - c[1])
                inp._set_u32(32, 0)  # no consume
                inp._set_i32(16, max(-80, min(80, dx // 10)))
                inp._set_i32(20, max(-80, min(80, dy // 10)))
                time.sleep(1.5)
                inp._set_i32(16, 0)
                inp._set_i32(20, 0)
                inp._set_u32(32, 1)
                img = shot(hwnd, OUT / "burst.jpg")
                print("after burst cursor", find_crosshair(img), flush=True)
                inp.click()
                time.sleep(3)

        img = shot(hwnd, OUT / "final.jpg")
        info = level_info(h)
        print("final", mean_rgb(img), is_level_map(img), info, flush=True)

        if not is_level_map(img):
            print("scanning (in-level candidate)...", flush=True)
            hits = scan_masks(h)
            print("hits", len(hits), flush=True)
            for sc, addr, ww, ht, ones in hits[:20]:
                print(f"  {sc:.4f} {addr:#x} {ww}x{ht} ones~{ones}", flush=True)
            if hits:
                sc, addr, ww, ht, ones = hits[0]
                blob = read_mem(h, addr, ww * ht)
                if blob:
                    p = OUT / f"live_mask_{ww}x{ht}_{addr:08x}.bin"
                    p.write_bytes(blob)
                    print("wrote", p, "nz", sum(1 for b in blob if b), flush=True)

        inp.release()
    k32.CloseHandle(h)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
