"""Enter a level from the map with calibrated mouse; dump mask candidates."""
from __future__ import annotations

import ctypes
import ctypes.wintypes as wt
import sys
import time
from pathlib import Path

from PIL import Image

sys.path.insert(0, str(Path(__file__).resolve().parent))
from enter_play import level_info
from lrinput import LRInput, DIK
from memdump import (
    PROCESS_QUERY_INFORMATION,
    PROCESS_VM_READ,
    find_pids,
    k32,
    read_mem,
    scan_masks,
)

OUT = Path(r"C:\progs\lemmings-webolution\build\live_dump")
user32 = ctypes.WinDLL("user32")
gdi32 = ctypes.WinDLL("gdi32")


def hwnd_for(pid: int) -> int | None:
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
    cols = list(img.resize((80, 45)).getdata())
    return sum(sum(p) for p in cols) / (len(cols) * 3)


def crosshair(img: Image.Image):
    px = img.load()
    w, h = img.size
    pts = []
    for y in range(0, h, 2):
        for x in range(0, w, 2):
            r, g, b = px[x, y]
            if g > 200 and g > r + 70 and g > b + 70 and r < 120 and b < 120:
                pts.append((x, y))
    if not pts:
        return None
    buckets: dict[tuple[int, int], list] = {}
    for x, y in pts:
        buckets.setdefault((x // 40, y // 40), []).append((x, y))
    best = None
    best_score = 1e9
    for cells in buckets.values():
        if len(cells) < 4 or len(cells) > 120:
            continue
        xs = [p[0] for p in cells]
        ys = [p[1] for p in cells]
        bw, bh = max(xs) - min(xs), max(ys) - min(ys)
        if bw < 4 or bh < 4 or bw > 60 or bh > 60:
            continue
        score = len(cells) + abs(bw - bh) * 2
        if score < best_score:
            best_score = score
            best = (sum(xs) / len(xs), sum(ys) / len(ys), len(cells))
    return best


def yellow_nodes(img: Image.Image):
    """Return mid yellow start-node if the screen is the level map."""
    m = mean_rgb(img)
    if m < 90 or m > 160:
        return None
    px = img.load()
    w, h = img.size
    pts = []
    for y in range(int(h * 0.35), int(h * 0.65), 2):
        for x in range(int(w * 0.34), int(w * 0.40), 2):
            r, g, b = px[x, y]
            if r > 200 and 150 < g < 210 and b < 90:
                pts.append((x, y))
    if len(pts) < 40:
        return None
    # Expect ~3 vertical bands of yellow (start column).
    ys = sorted(y for _, y in pts)
    span = ys[-1] - ys[0]
    if span < 120:
        return None
    lefts = sorted(pts, key=lambda p: p[1])
    mid = lefts[len(lefts) // 3 : 2 * len(lefts) // 3]
    return sum(x for x, _ in mid) / len(mid), sum(y for _, y in mid) / len(mid)


def is_main_menu(img: Image.Image) -> bool:
    return mean_rgb(img) < 45


def long_click(inp: LRInput):
    for _ in range(2):
        inp.button(0, True)
        inp.wait_frames(24)
        inp.button(0, False)
        inp.wait_frames(10)
        time.sleep(0.3)


def main():
    OUT.mkdir(parents=True, exist_ok=True)
    procs = find_pids("Lemmings")
    if not procs:
        print("no game")
        return 1
    pid = procs[0][0]
    hwnd = hwnd_for(pid)
    print("pid", pid, "hwnd", hwnd, flush=True)
    user32.ShowWindow(hwnd, 9)
    user32.SetForegroundWindow(hwnd)
    hproc = k32.OpenProcess(PROCESS_VM_READ | PROCESS_QUERY_INFORMATION, False, pid)

    with LRInput() as inp:
        inp.enable()

        # Reach level map: main menu -> Start New Game / Play Current -> map.
        target = None
        for i in range(24):
            img = shot(hwnd, OUT / f"nav_{i}.jpg")
            target = yellow_nodes(img)
            menu = is_main_menu(img)
            print(
                f"nav{i} mean={mean_rgb(img):.1f} menu={menu} nodes={target} ch={crosshair(img)}",
                flush=True,
            )
            if target:
                break
            if menu:
                # Ensure top item then ENTER (Start New Game), or DOWN once for Play Current.
                for _ in range(4):
                    inp.tap(DIK["UP"], hold_frames=6)
                    time.sleep(0.12)
                # Prefer Play Current Game (2nd item) so we keep unlocks.
                inp.tap(DIK["DOWN"], hold_frames=6)
                time.sleep(0.2)
                inp.tap(DIK["RETURN"], hold_frames=8)
                time.sleep(1.2)
            else:
                # Continue? / interstitial
                inp.tap(DIK["RETURN"], hold_frames=8)
                time.sleep(0.9)
        else:
            print("never reached level map")
            inp.release()
            return 1

        img = shot(hwnd, OUT / "map_now.jpg")
        c = crosshair(img)
        t = yellow_nodes(img)
        print("map c", c, "t", t, flush=True)
        if not c or not t:
            print("missing cursor or nodes")
            inp.release()
            return 1

        inp.glide(30, 0, steps=6)
        time.sleep(0.15)
        c2 = crosshair(shot(hwnd))
        sx = ((c2[0] - c[0]) / 30) if c and c2 else 2.5
        print("sx", sx, c, c2, flush=True)
        inp.glide(-30, 0, steps=6)
        time.sleep(0.1)
        scale = sx if abs(sx) > 0.5 else 2.5

        clicked = False
        for attempt in range(14):
            img = shot(hwnd, OUT / f"a{attempt}.jpg")
            if not yellow_nodes(img):
                print("left map during aim")
                break
            c = crosshair(img)
            t = yellow_nodes(img)
            print(f"a{attempt} c={c} t={t}", flush=True)
            if not c or not t:
                inp.glide(15, 15, steps=4)
                continue
            dx_px, dy_px = t[0] - c[0], t[1] - c[1]
            dist = (dx_px * dx_px + dy_px * dy_px) ** 0.5
            if dist < 22:
                print("LONG CLICK dist", dist, flush=True)
                long_click(inp)
                time.sleep(2.5)
                clicked = True
                break
            dx = int(0.55 * dx_px / scale)
            dy = int(0.55 * dy_px / scale)
            if dx == 0 and abs(dx_px) > 2:
                dx = 1 if dx_px > 0 else -1
            if dy == 0 and abs(dy_px) > 2:
                dy = 1 if dy_px > 0 else -1
            print(f"  move di={dx},{dy} dist={dist:.1f}", flush=True)
            inp.glide(dx, dy, steps=max(6, (abs(dx) + abs(dy)) // 15), per_step_frames=1)
            time.sleep(0.12)

        if not clicked:
            print("aim failed")
            inp.release()
            return 1

        # Wait for 3D — never RETURN on the map (Back to Main Menu).
        in_level = False
        for i in range(20):
            img = shot(hwnd, OUT / f"L{i}.jpg")
            nodes = yellow_nodes(img)
            menu = is_main_menu(img)
            cols = list(img.resize((100, 56)).getdata())
            uniq = len(set(cols))
            m = mean_rgb(img)
            print(
                f"L{i} mean={m:.1f} uniq={uniq} nodes={nodes} menu={menu} bufs={level_info(hproc).get('bufs')}",
                flush=True,
            )
            if nodes is None and not menu and uniq > 1200 and m > 40:
                print("LIKELY IN LEVEL", flush=True)
                in_level = True
                break
            if menu:
                print("bounced to main menu")
                break
            time.sleep(0.6)

        if in_level:
            print("SCAN", flush=True)
            hits = scan_masks(hproc)
            print("hits", len(hits), flush=True)
            for sc, addr, ww, ht, ones in hits[:30]:
                print(f"  {sc:.4f} {addr:#x} {ww}x{ht} ones~{ones}", flush=True)
            if hits:
                sc, addr, ww, ht, _ = hits[0]
                blob = read_mem(hproc, addr, ww * ht)
                if blob:
                    p = OUT / f"live_mask_{ww}x{ht}_{addr:08x}.bin"
                    p.write_bytes(blob)
                    print("wrote", p, "nz", sum(1 for b in blob if b), flush=True)
        else:
            print("not in level; no scan", flush=True)
        inp.release()
    k32.CloseHandle(hproc)
    print("done", flush=True)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
