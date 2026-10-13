"""Assume already on level map; click mid yellow node; scan if level loads."""
from __future__ import annotations

import sys
import time
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
from enter_and_dump import (
    OUT,
    crosshair,
    hwnd_for,
    is_main_menu,
    long_click,
    mean_rgb,
    shot,
    yellow_nodes,
)
from enter_play import level_info
from lrinput import LRInput
from memdump import (
    PROCESS_QUERY_INFORMATION,
    PROCESS_VM_READ,
    find_pids,
    k32,
    read_mem,
    scan_masks,
)
import ctypes

user32 = ctypes.WinDLL("user32")


def main():
    OUT.mkdir(parents=True, exist_ok=True)
    procs = find_pids("Lemmings")
    if not procs:
        print("no game")
        return 1
    pid = procs[0][0]
    hwnd = hwnd_for(pid)
    user32.ShowWindow(hwnd, 9)
    user32.SetForegroundWindow(hwnd)
    hproc = k32.OpenProcess(PROCESS_VM_READ | PROCESS_QUERY_INFORMATION, False, pid)

    with LRInput() as inp:
        inp.enable()
        img = shot(hwnd)
        t = yellow_nodes(img)
        print("map?", t, "mean", mean_rgb(img), flush=True)
        if not t:
            print("not on map")
            return 1

        c = crosshair(img)
        sx = 2.5
        if c:
            inp.glide(20, 0, steps=5)
            time.sleep(0.1)
            c2 = crosshair(shot(hwnd))
            if c2:
                sx = (c2[0] - c[0]) / 20.0
            print("sx", sx, c, c2, flush=True)
            inp.glide(-20, 0, steps=5)
        else:
            print("no cursor at start; sweeping", flush=True)
            inp.glide(-200, -200, steps=20)
            inp.glide(400, 400, steps=25)

        clicked = False
        for attempt in range(20):
            img = shot(hwnd, OUT / f"z{attempt}.jpg")
            t = yellow_nodes(img)
            c = crosshair(img)
            if not t:
                print("left map", flush=True)
                break
            if not c:
                print("lost cursor; nudge", flush=True)
                inp.glide(30, 20, steps=6)
                continue
            dx_px, dy_px = t[0] - c[0], t[1] - c[1]
            dist = (dx_px * dx_px + dy_px * dy_px) ** 0.5
            print(
                f"z{attempt} dist={dist:.1f} c={c[0]:.0f},{c[1]:.0f} t={t[0]:.0f},{t[1]:.0f}",
                flush=True,
            )
            if dist < 40:
                print("CLICK", flush=True)
                long_click(inp)
                time.sleep(0.4)
                long_click(inp)
                time.sleep(3)
                clicked = True
                break
            gain = 0.35 if dist < 80 else 0.55
            dx = int(gain * dx_px / sx)
            dy = int(gain * dy_px / sx)
            if dx == 0 and abs(dx_px) > 3:
                dx = 1 if dx_px > 0 else -1
            if dy == 0 and abs(dy_px) > 3:
                dy = 1 if dy_px > 0 else -1
            dx = max(-40, min(40, dx))
            dy = max(-40, min(40, dy))
            inp.glide(dx, dy, steps=max(4, (abs(dx) + abs(dy)) // 8))
            time.sleep(0.12)

        if not clicked:
            print("no click")
            return 1

        for i in range(15):
            img = shot(hwnd, OUT / f"P{i}.jpg")
            nodes = yellow_nodes(img)
            menu = is_main_menu(img)
            cols = list(img.resize((100, 56)).getdata())
            uniq = len(set(cols))
            m = mean_rgb(img)
            info = level_info(hproc)
            print(
                f"P{i} m={m:.1f} uniq={uniq} nodes={nodes} menu={menu} bufs={info.get('bufs')}",
                flush=True,
            )
            if nodes is None and not menu and uniq > 1200 and m > 40:
                print("IN LEVEL — scan", flush=True)
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
                break
            time.sleep(0.5)
        else:
            print("never entered", flush=True)
        inp.release()
    k32.CloseHandle(hproc)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
