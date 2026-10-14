"""Drive the original from the frontend into the first playable level.

Entry is memory-driven, not vision-driven: `lrstate.ui_state` says which
screen we are on (3 = frontend, 0 = playing, 5 = results) and
`drive.cursor` says where the pointer is, so nothing here depends on
`PrintWindow` -- which returns a black bitmap whenever the window is not
foreground and has produced every wrong conclusion in this area.

The route, all of it established in PLAN.md 1.6:

  * The stock 640x480 `Lemmings Revolution.exe` only. Under the resolution
    patch the level map's gold nodes are 3D-picked at the wrong scale and
    respond to no click at all.
  * Menus are German. RETURN (held ~20 polled frames; short taps are
    ignored) activates "Neues Spiel starten", RETURN again confirms "Ja",
    and the level map appears -- still ui_state 3.
  * The main menu has five items and the fifth is "Beenden". A blind RETURN
    closed the game during this work, so `find_menu_index` locates the
    highlight variable first (it is on the heap, so it moves every run) and
    drives it to item 0 before pressing anything.
  * `0x004C416C` is the frontend's own screen id: 3 = main menu, 4 = the
    Ja/Nein confirm, 5 = the level map. That is the only way to tell those
    apart here -- `ui_state` is 3 for all of them and both capture paths
    return black.
  * On the map the arrow keys do nothing and RETURN means "back to the main
    menu", so the node must be clicked. The first gold node is at render
    pixel (206, 204), measured by clicking: PLAN.md's (825, 612) was taken
    under the 2560x1440 build and scales as (x*640/2560, y*480/1440), not
    through the dgVoodoo letterbox.

Usage:
    python goto_level.py            # navigate, report the ui_state reached
"""
from __future__ import annotations

import sys
import time
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
from drive import aim_at, cursor  # noqa: E402
from lrinput import DIK, LRInput  # noqa: E402
from lrstate import attach, i32, ui_state  # noqa: E402
from memdump import k32  # noqa: E402

NODE = (206, 204)      # first gold node, render pixels (640x480)
SCREEN_ID = 0x004C416C  # frontend screen: 3 menu, 4 confirm, 5 level map
PLAYING = 0
FRONTEND = 3
RESULTS = 5


def focus(pid: int) -> bool:
    """Raise the game window and report whether Windows actually allowed it.

    A plain `SetForegroundWindow` from a background console is refused, which
    leaves the game polling at ~15-32 Hz instead of 144 and makes every timed
    measurement meaningless. Attaching to the target's input queue first
    removes the refusal.
    """
    import ctypes
    import ctypes.wintypes as wt

    user32 = ctypes.WinDLL("user32")
    k = ctypes.WinDLL("kernel32")
    found: list[int] = []

    @ctypes.WINFUNCTYPE(ctypes.c_bool, wt.HWND, wt.LPARAM)
    def cb(hwnd, _lp):
        p = wt.DWORD()
        user32.GetWindowThreadProcessId(hwnd, ctypes.byref(p))
        if p.value == pid and user32.IsWindowVisible(hwnd):
            if user32.GetWindowTextLengthW(hwnd):
                found.append(int(hwnd))
        return True

    user32.EnumWindows(cb, 0)
    for hwnd in found:
        p = wt.DWORD()
        tid = user32.GetWindowThreadProcessId(hwnd, ctypes.byref(p))
        mine = k.GetCurrentThreadId()
        user32.AttachThreadInput(mine, tid, True)
        user32.ShowWindow(hwnd, 9)
        user32.BringWindowToTop(hwnd)
        user32.SetForegroundWindow(hwnd)
        user32.AttachThreadInput(mine, tid, False)
    time.sleep(0.5)
    return bool(found) and user32.GetForegroundWindow() in found


def hide_console() -> None:
    """Minimise our own console so the game can hold the foreground.

    `SetForegroundWindow` on the game succeeds but Windows hands the
    foreground straight back to the console host that owns this process,
    which drops the engine from 144 Hz to about 30 and skews any timing read
    while it is running. Minimising the console is what actually keeps the
    game in front.
    """
    import ctypes

    k = ctypes.WinDLL("kernel32")
    con = k.GetConsoleWindow()
    if con:
        ctypes.WinDLL("user32").ShowWindow(con, 6)   # SW_MINIMIZE


def show_console() -> None:
    import ctypes

    k = ctypes.WinDLL("kernel32")
    con = k.GetConsoleWindow()
    if con:
        ctypes.WinDLL("user32").ShowWindow(con, 9)   # SW_RESTORE


def poll_rate(inp: LRInput, seconds: float = 0.5) -> float:
    f0 = inp.mouse_frame
    time.sleep(seconds)
    return (inp.mouse_frame - f0) / seconds


def find_menu_index(h, inp, verbose: bool = True) -> int | None:
    """Locate the main menu's highlight index, live, by diffing on DOWN.

    It is a heap variable, so its address changes every run and cannot be
    hard-coded. Pressing RETURN without knowing it is dangerous: the menu has
    five items and the last is "Beenden", so a blind RETURN closed the game
    outright during this work. Finding the index first makes entry
    deterministic.
    """
    from diffscan import changed, snap

    keep = None
    for _ in range(3):
        a = snap(h)
        inp.tap(DIK["DOWN"], hold_frames=20)
        time.sleep(0.6)
        b = snap(h)
        c = {k: v for k, v in changed(a, b, lo=0, hi=4).items()
             if v[1] == (v[0] + 1) % 5}
        keep = c if keep is None else {k: v for k, v in c.items() if k in keep}
    if verbose:
        print(f"  menu-index candidates: {len(keep or {})}")
    if not keep:
        return None
    return sorted(keep)[0]


def enter(h, inp, verbose: bool = True) -> int:
    """From wherever we are, try to reach ui_state 0. Returns the state."""
    def say(*a):
        if verbose:
            print(*a, flush=True)

    state = ui_state(h)
    say(f"ui_state {state}")
    if state == PLAYING:
        return state

    sel = find_menu_index(h, inp)
    if sel is None:
        say("  could not find the menu index; refusing to press RETURN blind")
        return state
    say(f"  menu index at {sel:#010x} = {i32(h, sel)}")
    for _ in range(6):
        if i32(h, sel) == 0:
            break
        inp.tap(DIK["DOWN"], hold_frames=20)
        time.sleep(0.5)
    say(f"  highlight now {i32(h, sel)} (0 = Neues Spiel starten)")

    # "Neues Spiel starten" -> "Ja" -> level map. Exactly two RETURNs: a
    # third is read by the map as "back to the main menu", which is how an
    # earlier run silently undid its own navigation.
    for i in range(2):
        inp.tap(DIK["RETURN"], hold_frames=20)
        time.sleep(1.5)
        say(f"  RETURN {i}: ui_state {ui_state(h)} cursor {cursor(h)}")
        if ui_state(h) == PLAYING:
            return PLAYING

    # On the map: click the first gold node.
    for attempt in range(4):
        pos = aim_at(h, inp, NODE)
        say(f"  aim {attempt}: cursor {pos}")
        inp.button(0, True)
        inp.wait_frames(8)
        inp.button(0, False)
        inp.wait_frames(8)
        time.sleep(2.5)
        state = ui_state(h)
        say(f"  click {attempt}: ui_state {state}")
        if state == PLAYING:
            return state
        # A second click sometimes confirms a "play this level?" prompt.
        inp.button(0, True)
        inp.wait_frames(8)
        inp.button(0, False)
        time.sleep(2.5)
        state = ui_state(h)
        say(f"  confirm {attempt}: ui_state {state}")
        if state == PLAYING:
            return state
    return ui_state(h)


def main() -> int:
    h, pid = attach()
    if not h:
        print("the game is not running")
        return 1
    focus(pid)
    with LRInput() as inp:
        rate = poll_rate(inp)
        print(f"pid {pid}, poll {rate:.0f} Hz "
              f"({'foreground' if rate > 60 else 'BACKGROUND -- clicks may miss'})")
        inp.enable()
        state = enter(h, inp)
        inp.release()
    print(f"final ui_state {state}")
    k32.CloseHandle(h)
    return 0 if state == PLAYING else 2


if __name__ == "__main__":
    raise SystemExit(main())
