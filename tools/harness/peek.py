"""Snapshot: focus state, poll rate, ui_state, cursor, and a screenshot."""
from __future__ import annotations

import sys
import time
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
from drive import cursor  # noqa: E402
from goto_level import focus  # noqa: E402
from lrinput import LRInput  # noqa: E402
from lrstate import attach, ui_state  # noqa: E402
from memdump import k32  # noqa: E402

OUT = Path(r"C:\progs\lemmings-webolution\build\live_dump")


def main() -> int:
    h, pid = attach()
    if not h:
        print("not running")
        return 1
    ok = focus(pid)
    print("focus granted:", ok)
    with LRInput() as inp:
        f0 = inp.mouse_frame
        time.sleep(1.0)
        print("poll Hz:", inp.mouse_frame - f0)
    print("ui_state:", ui_state(h), "cursor:", cursor(h))
    try:
        from enter_and_dump import hwnd_for, shot

        OUT.mkdir(parents=True, exist_ok=True)
        img = shot(hwnd_for(pid))
        p = OUT / "peek.png"
        img.save(p)
        print("shot", img.size, "->", p)
        small = img.resize((40, 30))
        print("mean", sum(sum(c) for c in small.getdata()) / (1200 * 3))
    except Exception as exc:
        print("no shot:", exc)
    k32.CloseHandle(h)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
