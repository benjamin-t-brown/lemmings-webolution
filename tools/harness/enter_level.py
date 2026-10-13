"""Drive the frontend toward a playable level via the dinput shim.

Heuristic: from the title/main menu, ENTER often starts or advances;
DOWN walks the highlight. After each action we re-read the level object
and print whether pointer fields look more "level-like".

Run with the game already open and focused.
"""
from __future__ import annotations

import struct
import sys
import time

sys.path.insert(0, r"C:\progs\lemmings-webolution\tools\harness")
from lrinput import LRInput, DIK
from memdump import find_pids, read_mem, read_u32, k32, PROCESS_VM_READ, PROCESS_QUERY_INFORMATION

LEVEL_PTR = 0x004CBF84


def snapshot(h):
    level = read_u32(h, LEVEL_PTR)
    if not level:
        return None
    blob = read_mem(h, level, 0x80)
    words = struct.unpack_from("<32I", blob)
    return level, words


def main():
    procs = find_pids("Lemmings")
    if not procs:
        print("start the game first")
        return 1
    pid = procs[0][0]
    h = k32.OpenProcess(PROCESS_VM_READ | PROCESS_QUERY_INFORMATION, False, pid)

    with LRInput() as inp:
        inp.enable()
        print("shim enabled; sequence: ENTER, DOWN x3, ENTER, wait...")
        before = snapshot(h)
        print("before", before[0] if before else None, before[1][:8] if before else None)

        # Wake / leave attract
        for _ in range(2):
            inp.tap(DIK["RETURN"])
            time.sleep(0.4)

        # Walk menu and try enter
        for i in range(8):
            inp.tap(DIK["DOWN"])
            time.sleep(0.25)
            if i in (1, 3, 5):
                inp.tap(DIK["RETURN"])
                time.sleep(1.0)
            snap = snapshot(h)
            if snap:
                lvl, words = snap
                # Heuristic: more non-zero heap ptrs after +0x28
                ptrs = sum(1 for w in words[8:24] if 0x01000000 < w < 0x7FFF0000)
                print(f"  step {i}: level={lvl:#x} mid_ptrs={ptrs} w16={words[16]:#x} w17={words[17]:#x}")

        time.sleep(2)
        after = snapshot(h)
        print("after", after[0] if after else None)
        if after:
            print("words", [hex(x) for x in after[1][:24]])
        inp.release()
    k32.CloseHandle(h)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
