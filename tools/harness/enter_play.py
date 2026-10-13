"""Drive main menu into a playable level; watch for terrain-sized buffers.

Avoid ESCAPE (quits). Prefer SPACE/RETURN and mouse clicks on menu items.
"""
from __future__ import annotations

import struct
import sys
import time
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
from lrinput import LRInput, DIK
from memdump import (
    find_pids,
    read_mem,
    read_u32,
    k32,
    PROCESS_VM_READ,
    PROCESS_QUERY_INFORMATION,
    MEMORY_BASIC_INFORMATION,
    MEM_COMMIT,
    PAGE_GUARD,
    PAGE_NOACCESS,
)

LEVEL_PTR = 0x004CBF84


def level_info(h):
    level = read_u32(h, LEVEL_PTR)
    if not level:
        return {"level": 0}
    blob = read_mem(h, level, 0x100)
    if not blob:
        return {"level": level}
    words = struct.unpack_from("<64I", blob)
    # Collect size,size,1,ptr descriptors hanging off the object
    bufs = []
    for i, w in enumerate(words):
        if not (0x01000000 < w < 0x7FFF0000):
            continue
        d = read_mem(h, w, 32)
        if not d:
            continue
        a = struct.unpack_from("<8I", d)
        if a[0] == a[1] and a[2] == 1 and 256 <= a[0] <= 2_000_000 and 0x10000 < a[3] < 0x7FFF0000:
            bufs.append((i, a[0], a[3]))
    return {"level": level, "bufs": bufs, "w0": words[0], "w6": words[6]}


def find_string(h, needle: bytes, limit=0x20000000):
    """Quick scan for a short ASCII needle in committed heap-ish ranges."""
    mbi = MEMORY_BASIC_INFORMATION()
    addr = 0x01000000
    while addr < limit:
        got = k32.VirtualQueryEx(h, ctypes.c_void_p(addr), ctypes.byref(mbi), ctypes.sizeof(mbi))
        if got == 0:
            break
        base = mbi.BaseAddress or 0
        size = mbi.RegionSize or 0
        if size == 0:
            break
        if mbi.State == MEM_COMMIT and not (mbi.Protect & PAGE_GUARD) and mbi.Protect != PAGE_NOACCESS:
            chunk = min(size, 4 * 1024 * 1024)
            data = read_mem(h, base, chunk)
            if data:
                off = data.find(needle)
                if off >= 0:
                    return base + off
        addr = base + size
        if addr <= base:
            break
    return None


def shim_alive(inp, seconds=1.0) -> bool:
    f0 = inp.mouse_frame
    time.sleep(seconds)
    return inp.mouse_frame > f0


def main():
    procs = find_pids("Lemmings")
    if not procs:
        print("start the game first")
        return 1
    pid = procs[0][0]
    h = k32.OpenProcess(PROCESS_VM_READ | PROCESS_QUERY_INFORMATION, False, pid)
    print("pid", pid, "before", level_info(h))

    with LRInput() as inp:
        if not shim_alive(inp):
            print("shim not polling")
            return 1
        inp.enable()
        print("shim live; skipping attract with SPACE/RETURN (no ESC)")

        for _ in range(8):
            inp.tap(DIK["SPACE"])
            time.sleep(0.35)
            inp.tap(DIK["RETURN"])
            time.sleep(0.35)

        # Walk each menu item and try RETURN — look for big buffers or level name
        for item in range(10):
            print(f"--- menu item {item} ---", level_info(h))
            inp.tap(DIK["RETURN"])
            time.sleep(1.5)
            info = level_info(h)
            big = [b for b in info.get("bufs", []) if b[1] >= 256 * 160]
            print("  after ENTER", info, "big", big)
            if big:
                print("FOUND large buffers — likely in level")
                break
            # back out one level of submenu if we dove in wrongly: LEFT or no ESC
            inp.tap(DIK["DOWN"])
            time.sleep(0.4)

        # Also try mouse: move down the screen in steps and click
        print("--- mouse sweep clicks ---")
        for step in range(12):
            inp.move(0, 18)
            time.sleep(0.08)
            if step % 2 == 1:
                inp.click()
                time.sleep(1.0)
                info = level_info(h)
                big = [b for b in info.get("bufs", []) if b[1] >= 256 * 160]
                print(f"  click {step}", info.get("bufs"), "big", big)
                if big:
                    break

        time.sleep(2)
        final = level_info(h)
        print("final", final)
        # Search for a known practice level stem
        for name in (b"MM_CLIMB", b"MM_FLOAT", b"MM_BASH", b"CLIMB"):
            at = find_string(h, name)
            print(f"  string {name!r} -> {hex(at) if at else None}")
        inp.release()
    k32.CloseHandle(h)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
