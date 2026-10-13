"""Try to get past AVI / into the main menu, then into play."""
import sys
import time

sys.path.insert(0, r"C:\progs\lemmings-webolution\tools\harness")
from lrinput import LRInput, DIK
from memdump import find_pids, read_u32, k32, PROCESS_VM_READ, PROCESS_QUERY_INFORMATION

LEVEL_PTR = 0x004CBF84


def main():
    procs = find_pids("Lemmings")
    if not procs:
        print("no game")
        return 1
    h = k32.OpenProcess(PROCESS_VM_READ | PROCESS_QUERY_INFORMATION, False, procs[0][0])
    with LRInput() as inp:
        inp.enable()
        # Skip cutscenes / attract
        for key in ("ESCAPE", "SPACE", "RETURN", "ESCAPE"):
            print("tap", key)
            inp.tap(DIK[key])
            time.sleep(0.5)
        # Click center-ish via relative moves + click (no absolute mouse in DI)
        print("mouse nudge + click")
        for _ in range(5):
            inp.move(5, 0)
            time.sleep(0.05)
        inp.click()
        time.sleep(1.0)
        # Menu walk: often first item is Play / Start
        for i in range(6):
            print("DOWN", i, "level", hex(read_u32(h, LEVEL_PTR) or 0))
            inp.tap(DIK["DOWN"])
            time.sleep(0.3)
        inp.tap(DIK["RETURN"])
        time.sleep(2)
        print("final level", hex(read_u32(h, LEVEL_PTR) or 0))
        # Try ENTER again in case of confirm
        inp.tap(DIK["RETURN"])
        time.sleep(3)
        print("after confirm", hex(read_u32(h, LEVEL_PTR) or 0))
        inp.release()
    k32.CloseHandle(h)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
