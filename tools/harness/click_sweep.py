"""Find the clickable level-map node by sweeping clicks over the screen.

The map's gold nodes are 3D-picked, so their screen position cannot be read
out of any widget list, and both capture paths are black on this machine --
`PrintWindow` and a desktop `BitBlt` alike, because dgVoodoo presents through
a D3D11 swapchain in fake-fullscreen. That leaves brute force: click a grid
of render pixels and watch `ui_state` for the jump to 0 (playing).

The main menu's highlight index doubles as a screen oracle: it only responds
to DOWN while the main menu is up, so the sweep can tell whether a stray
click dropped it back out of the map, and stop rather than wander into
"Beenden".
"""
from __future__ import annotations

import sys
import time
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
from drive import aim_at, cursor  # noqa: E402
from goto_level import PLAYING, focus  # noqa: E402
from lrinput import DIK, LRInput  # noqa: E402
from lrstate import attach, i32, ui_state  # noqa: E402
from memdump import k32  # noqa: E402


def on_main_menu(h, inp, sel: int) -> bool:
    before = i32(h, sel)
    inp.tap(DIK["DOWN"], hold_frames=20)
    time.sleep(0.4)
    return i32(h, sel) != before


def main() -> int:
    sel = int(sys.argv[1], 0) if len(sys.argv) > 1 else None
    step = int(sys.argv[2]) if len(sys.argv) > 2 else 40
    h, pid = attach()
    if not h:
        print("not running")
        return 1
    focus(pid)
    with LRInput() as inp:
        inp.enable()
        if sel is not None:
            print("on main menu:", on_main_menu(h, inp, sel), flush=True)
        hit = None
        for y in range(step // 2, 480, step):
            for x in range(step // 2, 640, step):
                aim_at(h, inp, (x, y), tol=8)
                inp.button(0, True)
                inp.wait_frames(10)
                inp.button(0, False)
                inp.wait_frames(4)
                time.sleep(0.45)
                st = ui_state(h)
                if st != 3:
                    print(f"  ({x:3d},{y:3d}) -> ui_state {st}", flush=True)
                if st == PLAYING:
                    hit = (x, y)
                    break
            if hit:
                break
            print(f"row y={y} done, cursor {cursor(h)}", flush=True)
        inp.release()
    print("hit:", hit, "final ui_state", ui_state(h))
    k32.CloseHandle(h)
    return 0 if hit else 2


if __name__ == "__main__":
    raise SystemExit(main())
