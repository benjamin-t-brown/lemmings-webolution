"""Dump the engine's own collision grid for several levels in one run.

Why a driver rather than running `goto_level` and `read_grid` by hand: the
only reliable way to change level is to restart the process. From inside a
running level there is no route back to the map that does not go through ESC,
and ESC is the one key in this game that has quit the process outright (see
`goto_level`'s docstring). Relaunching costs about fifteen seconds and never
surprises anyone, so that is what this does between levels.

Each level is entered through `goto_level.enter_level`, which copies the
level's record over the one node whose screen position is known and puts the
table back afterwards. The level that actually loaded is then confirmed from
the engine's own path buffer before anything is dumped, so a mis-navigation
cannot be recorded as a measurement of the wrong level.

Usage:
    python dump_grids.py                       # the four task-C levels
    python dump_grids.py MM_BASH_002 MM_MINE   # any levels in the table
"""
from __future__ import annotations

import struct
import subprocess
import sys
import time
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
from goto_level import (  # noqa: E402
    PLAYING,
    enter_level,
    focus,
    loaded_level,
    poll_rate,
)
from lrinput import LRInput  # noqa: E402
from lrstate import attach  # noqa: E402
from memdump import k32  # noqa: E402
from read_grid import GRID_WIDTH, read_grid, solid, write_png  # noqa: E402

EXE = Path(r"C:\progs\Lemmings Revolution\Lemmings Revolution.exe")
DEFAULT_LEVELS = ["MM_CLIMB_002", "MM_DIG_000", "MM_BOMB", "MM_BUILD", "MM_FLOAT"]
OUT_DIR = Path("build/live_dump")


def restart_game(wait: float = 14.0) -> None:
    """Kill the game if it is up and start it again, then wait for the menu."""
    subprocess.run(
        ["powershell", "-c", "Stop-Process -Name 'Lemmings Revolution' -Force"],
        capture_output=True,
    )
    time.sleep(3.0)
    subprocess.run(
        ["powershell", "-c",
         f"Start-Process -FilePath '{EXE}' -WorkingDirectory '{EXE.parent}'"],
        capture_output=True,
    )
    time.sleep(wait)


def dump_one(level: str) -> dict | None:
    """Enter `level` from a fresh process and dump its grid. None on failure."""
    restart_game()
    h, pid = attach(write=True)
    if not h:
        print(f"{level}: the game did not come up")
        return None
    try:
        focus(pid)
        with LRInput() as inp:
            poll_rate(inp)
            inp.enable()
            try:
                state = enter_level(h, inp, level, verbose=False)
            except LookupError as e:
                print(f"{level}: {e}")
                return None
            finally:
                inp.release()
        if state != PLAYING:
            print(f"{level}: did not reach a level, ui_state {state}")
            return None
        # The engine's own answer, not ours. A mis-navigation stops here
        # rather than being written out under the wrong name.
        path = loaded_level(h)
        if level.lower() not in path.lower():
            print(f"{level}: the engine loaded {path!r} instead; refusing to dump")
            return None
        grid, why = read_grid(h)
        if grid is None:
            print(f"{level}: {why}")
            return None
        rows, cells = grid
        OUT_DIR.mkdir(parents=True, exist_ok=True)
        stem = OUT_DIR / f"grid_{level}"
        stem.with_suffix(".bin").write_bytes(struct.pack(f"<{len(cells)}I", *cells))
        rgb = []
        for y in range(rows):
            line = bytearray()
            for x in range(GRID_WIDTH):
                c = cells[y * GRID_WIDTH + x]
                line += b"\xff\xff\xff" if solid(c) else (b"\x60\x60\x60" if c else b"\x00\x00\x00")
            rgb.append(line)
        write_png(stem.with_suffix(".png"), GRID_WIDTH, rows, rgb)
        n_solid = sum(1 for c in cells if solid(c))
        return {
            "level": level,
            "path": path,
            "rows": rows,
            "solid_pct": 100.0 * n_solid / len(cells),
        }
    finally:
        k32.CloseHandle(h)


def main() -> int:
    levels = sys.argv[1:] or DEFAULT_LEVELS
    rows = []
    for lv in levels:
        print(f"=== {lv} ===", flush=True)
        info = dump_one(lv)
        if info:
            rows.append(info)
            print(f"  {info['path']}  solid {info['solid_pct']:.2f}%", flush=True)
    print()
    print(f"{'level':<16} {'engine solid':>12}")
    for r in rows:
        print(f"{r['level']:<16} {r['solid_pct']:>11.2f}%")
    return 0 if len(rows) == len(levels) else 2


if __name__ == "__main__":
    raise SystemExit(main())
