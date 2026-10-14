"""Watch what changes as the frontend moves from main menu to level map.

`ui_state` stays 3 across the whole frontend and both screen-capture paths
are black on this machine, so the harness is blind between the main menu and
the level map. This records three snapshots -- main menu, after the first
RETURN, after the second -- and prints the small-int fields that differ, so a
screen or page variable can be picked out and used as an oracle afterwards.

The main-menu highlight is driven to 0 ("Neues Spiel starten") first, using
`goto_level.find_menu_index`: the fifth item is "Beenden" and a blind RETURN
has already closed the game once during this work.
"""
from __future__ import annotations

import sys
import time
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
from diffscan import changed, snap  # noqa: E402
from goto_level import find_menu_index, focus  # noqa: E402
from lrinput import DIK, LRInput  # noqa: E402
from lrstate import attach, i32, ui_state  # noqa: E402
from memdump import k32  # noqa: E402


def main() -> int:
    h, pid = attach()
    if not h:
        print("not running")
        return 1
    focus(pid)
    with LRInput() as inp:
        inp.enable()
        sel = find_menu_index(h, inp)
        print("menu index", hex(sel) if sel else None, flush=True)
        if sel is None:
            return 1
        for _ in range(6):
            if i32(h, sel) == 0:
                break
            inp.tap(DIK["DOWN"], hold_frames=20)
            time.sleep(0.5)
        shots = [("menu", snap(h))]
        for i in range(2):
            inp.tap(DIK["RETURN"], hold_frames=20)
            time.sleep(2.0)
            shots.append((f"after RETURN {i}", snap(h)))
            print(f"{shots[-1][0]}: ui_state {ui_state(h)} sel {i32(h, sel)}",
                  flush=True)
        inp.release()

    for (na, a), (nb, b) in zip(shots, shots[1:]):
        c = changed(a, b, lo=-2, hi=300)
        print(f"\n{na} -> {nb}: {len(c)} small-int changes")
        for addr, (x, y) in sorted(c.items())[:40]:
            print(f"  {addr:#010x}  {x} -> {y}")
    k32.CloseHandle(h)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
