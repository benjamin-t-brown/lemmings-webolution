"""From the level-map screen, click a starting (yellow) node and dump terrain."""
from __future__ import annotations

import struct
import sys
import time
from pathlib import Path

from PIL import Image

sys.path.insert(0, str(Path(__file__).resolve().parent))
from lrinput import LRInput, DIK
from memdump import (
    find_pids,
    read_mem,
    read_u32,
    k32,
    PROCESS_VM_READ,
    PROCESS_QUERY_INFORMATION,
    scan_masks,
)
from enter_play import level_info

OUT = Path(r"C:\progs\lemmings-webolution\build\live_dump")
LEVEL_PTR = 0x004CBF84


def screenshot(hwnd: int, path: Path) -> Image.Image:
    import ctypes
    import ctypes.wintypes as wt

    user32 = ctypes.WinDLL("user32")
    gdi32 = ctypes.WinDLL("gdi32")
    rect = wt.RECT()
    user32.GetClientRect(hwnd, ctypes.byref(rect))
    w, h = rect.right - rect.left, rect.bottom - rect.top
    hdc = user32.GetDC(hwnd)
    memdc = gdi32.CreateCompatibleDC(hdc)
    hbmp = gdi32.CreateCompatibleBitmap(hdc, w, h)
    old = gdi32.SelectObject(memdc, hbmp)
    user32.PrintWindow(hwnd, memdc, 2)

    class BITMAPINFOHEADER(ctypes.Structure):
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

    bih = BITMAPINFOHEADER()
    bih.biSize = ctypes.sizeof(BITMAPINFOHEADER)
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
    img.save(path)
    return img


def find_hwnd(pid: int) -> int | None:
    import ctypes
    import ctypes.wintypes as wt

    user32 = ctypes.WinDLL("user32")
    found = []

    @ctypes.WINFUNCTYPE(ctypes.c_bool, wt.HWND, wt.LPARAM)
    def cb(hwnd, _lp):
        if not user32.IsWindowVisible(hwnd):
            return True
        p = wt.DWORD()
        user32.GetWindowThreadProcessId(hwnd, ctypes.byref(p))
        if p.value == pid:
            n = user32.GetWindowTextLengthW(hwnd)
            if n:
                found.append(int(hwnd))
        return True

    user32.EnumWindows(cb, 0)
    return found[0] if found else None


def locate(img: Image.Image):
    w, h = img.size
    px = img.load()
    gxs, gys, yxs, yys = [], [], [], []
    step = 2
    for y in range(0, h, step):
        for x in range(0, w, step):
            r, g, b = px[x, y]
            if g > 140 and g > r + 40 and g > b + 40 and r < 180 and b < 180:
                gxs.append(x)
                gys.append(y)
            if r > 180 and g > 140 and b < 120 and r > b + 60 and g > b + 40:
                yxs.append(x)
                yys.append(y)
    if not gxs or not yxs:
        return None
    cursor = (sum(gxs) / len(gxs), sum(gys) / len(gys))
    xmin = min(yxs)
    left = [(x, y) for x, y in zip(yxs, yys) if x < xmin + 150]
    lefts = sorted(left, key=lambda p: p[1])
    mid = lefts[len(lefts) // 3 : 2 * len(lefts) // 3]
    target = (sum(x for x, _ in mid) / len(mid), sum(y for _, y in mid) / len(mid))
    return cursor, target


def main():
    OUT.mkdir(parents=True, exist_ok=True)
    procs = find_pids("Lemmings")
    if not procs:
        print("no game")
        return 1
    pid = procs[0][0]
    hwnd = find_hwnd(pid)
    if not hwnd:
        print("no hwnd")
        return 1
    import ctypes

    user32 = ctypes.WinDLL("user32")
    user32.ShowWindow(hwnd, 9)
    user32.SetForegroundWindow(hwnd)
    time.sleep(0.3)

    img = screenshot(hwnd, OUT / "pre_click.bmp")
    loc = locate(img)
    if not loc:
        print("could not locate cursor/nodes")
        return 1
    cursor, target = loc
    dx = int(target[0] - cursor[0])
    dy = int(target[1] - cursor[1])
    print(f"cursor={cursor} target={target} delta=({dx},{dy})")

    h = k32.OpenProcess(PROCESS_VM_READ | PROCESS_QUERY_INFORMATION, False, pid)
    print("before", level_info(h))

    with LRInput() as inp:
        f0 = inp.mouse_frame
        time.sleep(0.5)
        if inp.mouse_frame == f0:
            print("shim not polling")
            return 1
        inp.enable()
        # DI mickeys ≈ pixels under dgVoodoo; glide in chunks
        inp.glide(dx, dy, steps=max(20, abs(dx) // 20 + abs(dy) // 20), per_step_frames=1)
        time.sleep(0.2)
        img2 = screenshot(hwnd, OUT / "pre_click2.bmp")
        loc2 = locate(img2)
        print("after move locate", loc2)
        if loc2:
            c2, t2 = loc2
            dx2, dy2 = int(t2[0] - c2[0]), int(t2[1] - c2[1])
            print("correct", dx2, dy2)
            if abs(dx2) > 8 or abs(dy2) > 8:
                inp.glide(dx2, dy2, steps=15, per_step_frames=1)
        inp.click()
        time.sleep(0.5)
        inp.click()  # sometimes needs confirm
        time.sleep(3.0)
        print("after click", level_info(h))
        screenshot(hwnd, OUT / "post_click.bmp").save(OUT / "post_click.jpg", quality=70)
        # If still on map, try RETURN as confirm
        inp.tap(DIK["RETURN"])
        time.sleep(2.0)
        print("after return", level_info(h))
        screenshot(hwnd, OUT / "post_return.bmp")

        print("scanning masks...")
        hits = scan_masks(h)
        for sc, addr, ww, ht, ones in hits[:20]:
            print(f"  score={sc:.4f} {addr:#010x} {ww}x{ht} ones≈{ones}")
        if hits:
            sc, addr, ww, ht, ones = hits[0]
            blob = read_mem(h, addr, ww * ht)
            if blob:
                path = OUT / f"live_mask_{ww}x{ht}_{addr:08x}.bin"
                path.write_bytes(blob)
                print("wrote", path, "nonzero", sum(1 for b in blob if b))
        inp.release()
    k32.CloseHandle(h)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
