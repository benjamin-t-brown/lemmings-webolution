"""Get back into MM_CLIMB_002 from wherever the last run left the game.

The level has a 300 second limit (LEVELMAP.DAT), so a measurement session
runs out and lands on the results screen; this puts it back. The route is
read off `ui_state` and the frontend screen id rather than guessed:

    ui_state 5          results      -> RETURN
    ui_state 3 screen 3 main menu    -> highlight item 0, RETURN, RETURN
    ui_state 3 other    level map    -> click the first gold node
    ui_state 0                       -> playing
"""
from __future__ import annotations

import sys
import time
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
from drive import aim_at  # noqa: E402
from goto_level import NODE, SCREEN_ID, find_menu_index, focus  # noqa: E402
from lrinput import DIK, LRInput  # noqa: E402
from lrstate import attach, i32, ui_state  # noqa: E402
from memdump import k32  # noqa: E402

MAIN_MENU = 3


def click(h, inp, target):
    aim_at(h, inp, target)
    time.sleep(0.2)
    inp.button(0, True)
    inp.wait_frames(10)
    inp.button(0, False)
    inp.wait_frames(5)
    time.sleep(2.0)


def ensure_playing(h, inp, verbose=True, tries=8) -> bool:
    for _ in range(tries):
        st, screen = ui_state(h), i32(h, SCREEN_ID)
        if verbose:
            print(f"  ui_state {st} screen {screen}", flush=True)
        if st == 0:
            return True
        if st == 5:
            inp.tap(DIK["RETURN"], hold_frames=20)
            time.sleep(2.0)
            continue
        if screen == MAIN_MENU:
            sel = find_menu_index(h, inp, verbose=verbose)
            if sel is None:
                return False
            for _ in range(6):
                if i32(h, sel) == 0:
                    break
                inp.tap(DIK["DOWN"], hold_frames=20)
                time.sleep(0.5)
            for _ in range(2):
                inp.tap(DIK["RETURN"], hold_frames=20)
                time.sleep(1.8)
            continue
        click(h, inp, NODE)
    return ui_state(h) == 0


def main() -> int:
    h, pid = attach()
    if not h:
        print("not running")
        return 1
    focus(pid)
    with LRInput() as inp:
        inp.enable()
        ok = ensure_playing(h, inp)
        inp.release()
    print("playing" if ok else "failed to enter")
    k32.CloseHandle(h)
    return 0 if ok else 2


if __name__ == "__main__":
    raise SystemExit(main())
