"""Wait for the current level to finish, start it again, and hold.

`restart_level` can only re-enter from the results screen or the map; while a
level is still running there is no way back in. MM_CLIMB_002 has a 300 second
limit, so the simplest reliable way to get a *fresh* level -- one where
lemmings are still dropping out of the entrance -- is to let the clock run
out. `0x004CBAAC` is that clock, in whole seconds.
"""
from __future__ import annotations

import sys
import time
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
from goto_level import focus  # noqa: E402
from lrinput import LRInput  # noqa: E402
from lrstate import attach, i32, ui_state  # noqa: E402
from memdump import k32  # noqa: E402
from restart_level import ensure_playing  # noqa: E402

CLOCK = 0x004CBAAC          # level clock, whole seconds remaining


def wait_for_end(h, timeout=330.0):
    t0 = time.time()
    while ui_state(h) == 0 and time.time() - t0 < timeout:
        print(f"  clock {i32(h, CLOCK)}", flush=True)
        time.sleep(5.0)
    return ui_state(h)


def main() -> int:
    h, pid = attach()
    if not h:
        print("not running")
        return 1
    focus(pid)
    if ui_state(h) == 0:
        print("a level is running; waiting for the clock")
        wait_for_end(h)
    with LRInput() as inp:
        inp.enable()
        ok = ensure_playing(h, inp)
        inp.release()
    print("playing, clock", i32(h, CLOCK) if ok else "-")
    k32.CloseHandle(h)
    return 0 if ok else 2


if __name__ == "__main__":
    raise SystemExit(main())
