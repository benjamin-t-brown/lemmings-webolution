"""Drive the original from the frontend into any named level.

Entry is memory-driven, not vision-driven: `lrstate.ui_state` says which
screen we are on (3 = frontend, 0 = playing, 5 = results) and
`drive.cursor` says where the pointer is, so nothing here depends on
`PrintWindow` -- which returns a black bitmap whenever the window is not
foreground and has produced every wrong conclusion in this area.

Picking a level without knowing where its node is
-------------------------------------------------
The level map is a 3D scene the camera orbits, so a node's screen position is
a projection and not a stored coordinate. Neither `LEVELMAP.DAT` nor
`LEVELS.MAP` carries one -- they hold the id, the display name and the three
counts and nothing else -- and the pair (206, 204) appears nowhere in the
executable. So there is no node table to look up, and finding each node by
clicking would mean 102 fragile calibrations.

What the engine does keep is a flat table of level records, and the map only
ever indexes it:

    0x004E35D4              record table, 5 groups x 25 slots
    + group * 0x1B58        group stride (25 records)
    + slot  * 0x118         slot stride, and the record size
    +0x000  char[128]       level id, e.g. "MM_CLIMB_002"
    +0x080  char[128]       display name, e.g. "Just Climb"
    +0x100  u32 u32 u32     time limit seconds, total lemmings, save target

The table is zero in the image and filled at startup from `LEVELMAP.DAT`.
Clicking a node resolves to (group, slot), reads that record, and hands its id
to the path builder:

    0049b190  call 0x00460610         ; group index, the global 0x005D5A00
    0049b19d  call 0x00460630         ; slot index, the global 0x005D5A04
    0049b1a8  lea  ecx, [esi+eax+0x4e35d4]
    0049b1b5  call 0x0048e530         ; store the id pointer at 0x005D59F0
    0045d891  call 0x004605f0         ; read it back
    0045d89d  call 0x0045c9ab         ; sprintf "Compiled\\%s%s" -> 0x004CBBF8

So the whole selection funnels through one record. `select_level` copies the
record of the level you asked for over slot [0][0] -- the one node whose
screen position is known -- and the click then loads that level. The copy is
the whole 0x118 bytes, not just the id, so the clock and the lemming counts
come from the real level too.

**Do not try to write the index globals instead.** `0x0049B14F` and
`0x0049B15B` set slot and group from the picked node immediately before
`0x0049B190` reads them back, so a value written from outside is overwritten
within the same click. The record swap has no such race: it is read once, at
click time, long after the write.

The swap mutates the game's own table, so `enter_level` restores slot [0][0]
afterwards and the table is never left altered. A level entered this way is
the genuine level, checked three independent ways on MM_DIG_000: the path
buffer at `0x004CBBF8` reads `Compiled\\MM_DIG_000.lvl`, the save target at
`0x004CBAB0` reads 10 where MM_CLIMB_002 reads 1, and the release counter
reached 18 on a level `LEVELMAP.DAT` gives 20 lemmings.

Quirks, all of them the original's behaviour and none of them worked around
by pretending otherwise:

  * The stock 640x480 `Lemmings Revolution.exe` only. Under the resolution
    patch the level map's gold nodes are 3D-picked at the wrong scale and
    respond to no click at all.
  * Menus are German. RETURN (held ~20 polled frames; short taps are
    ignored) activates "Neues Spiel starten", RETURN again confirms "Ja",
    and the level map appears -- still ui_state 3.
  * The main menu has five items and the fifth is "Beenden". A blind RETURN
    closed the game during this work, so `find_menu_index` locates the
    highlight variable first (it is on the heap, so it moves every run) and
    drives it to item 0 before pressing anything. RETURN only at exactly 0.
  * `0x004C416C` is the frontend's own screen id: 3 = main menu, 4 = the
    Ja/Nein confirm, 5 = the level map. That is the only way to tell those
    apart here -- `ui_state` is 3 for all of them and both capture paths
    return black.
  * On the map the arrow keys do nothing and RETURN means "back to the main
    menu", so the node must be clicked. The first gold node is at render
    pixel (206, 204), measured by clicking: PLAN.md's (825, 612) was taken
    under the 2560x1440 build and scales as (x*640/2560, y*480/1440), not
    through the dgVoodoo letterbox.
  * `consume_deltas` is 0 in the shim block, so the last relative motion is
    re-read every frame and the cursor slides until it clamps. `drive.aim_at`
    writes a zero delta before returning; do not click without aiming first.
  * A 25-frame ESC can fail to leave pause; 40 frames after `focus()` works.
    ESC from a post-level screen can quit the process.

Usage:
    python goto_level.py                 # the first node, as before
    python goto_level.py MM_DIG_000      # any level in the table
    python goto_level.py --list          # what the live table holds
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

# The level record table the map indexes; see the module docstring.
LEVEL_TABLE = 0x004E35D4
GROUP_STRIDE = 0x1B58
SLOT_STRIDE = 0x118
RECORD_SIZE = 0x118
GROUPS, SLOTS = 5, 25
# Where the loader leaves the path it built, and the level's own counts.
PATH_BUFFER = 0x004CBBF8
TIME_LIMIT = 0x004CBAA8
SAVE_TARGET = 0x004CBAB0


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
    mine = k.GetCurrentThreadId()
    # Attaching only to the target is not enough when this process did not
    # receive the last input: Windows then refuses SetForegroundWindow.
    # Joining the *current* foreground thread's queue is what lifts that.
    fg = user32.GetForegroundWindow()
    fg_tid = user32.GetWindowThreadProcessId(fg, None) if fg else 0
    if fg_tid and fg_tid != mine:
        user32.AttachThreadInput(fg_tid, mine, True)
    for hwnd in found:
        p = wt.DWORD()
        tid = user32.GetWindowThreadProcessId(hwnd, ctypes.byref(p))
        user32.AttachThreadInput(mine, tid, True)
        user32.ShowWindow(hwnd, 9)
        user32.BringWindowToTop(hwnd)
        user32.SetForegroundWindow(hwnd)
        user32.AttachThreadInput(mine, tid, False)
    if fg_tid and fg_tid != mine:
        user32.AttachThreadInput(fg_tid, mine, False)
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


def foreground_pid() -> int:
    """Pid that owns the foreground window, or -1 when it cannot be asked.

    Safe off Windows (no `user32`) and when the call fails, so diagnostics
    never turn into the failure they are meant to explain.
    """
    try:
        import ctypes
        import ctypes.wintypes as wt

        user32 = ctypes.WinDLL("user32")
        owner = wt.DWORD()
        user32.GetWindowThreadProcessId(user32.GetForegroundWindow(), ctypes.byref(owner))
        return int(owner.value)
    except Exception:
        return -1


def _tee(log, verbose: bool = True):
    """A one-argument writer: stdout when `verbose`, and always `log` if given.

    `log` is a callable taking the line, or a file-like with `write`. The
    scheduled task that runs the probes does not capture stdout, so anything
    that matters for a failed entry has to reach a caller-supplied log.
    """
    def emit(line: str) -> None:
        if verbose:
            print(line, flush=True)
        if log is None:
            return
        if callable(log):
            log(line)
        else:
            log.write(line + "\n")
            if hasattr(log, "flush"):
                log.flush()
    return emit


def find_menu_index(h, inp, verbose: bool = True, attempts: int = 3,
                    log=None) -> int | None:
    """Locate the main menu's highlight index, live, by diffing on DOWN.

    It is a heap variable, so its address changes every run and cannot be
    hard-coded. Pressing RETURN without knowing it is dangerous: the menu has
    five items and the last is "Beenden", so a blind RETURN closed the game
    outright during this work. Finding the index first makes entry
    deterministic.

    `log` (callable or file-like) receives every diagnostic line as well as
    stdout; see `_tee`. `attempts` bounds the retry of an empty scan.
    """
    from diffscan import changed, snap

    emit = _tee(log, verbose)
    # A scan that finds nothing is retried: the extra DOWN taps only move the
    # highlight, and the (v + 1) % 5 test does not care where it starts. Each
    # press logs how many keyboard polls the game consumed and who held the
    # foreground, so a failed scan says whether DOWN reached the game at all.
    keep = None
    for attempt in range(attempts):
        keep = None
        for press in range(3):
            fg_before = foreground_pid()
            a = snap(h)
            k0 = inp.key_frame
            inp.tap(DIK["DOWN"], hold_frames=20)
            polls = inp.key_frame - k0
            time.sleep(0.6)
            b = snap(h)
            c = {k: v for k, v in changed(a, b, lo=0, hi=4).items()
                 if v[1] == (v[0] + 1) % 5}
            keep = c if keep is None else {k: v for k, v in c.items() if k in keep}
            emit(f"  menu scan {attempt + 1}.{press + 1}: key polls {polls}, "
                 f"foreground {fg_before}->{foreground_pid()}, "
                 f"mod-5 {len(c)}, kept {len(keep)}")
        if keep:
            break
    emit(f"  menu-index candidates: {len(keep or {})}")
    if not keep:
        return None
    return sorted(keep)[0]


def snapshot_idle(h, inp=None, seconds: float = 15.0, interval: float = 1.0,
                  log=None, verbose: bool = True, sleep=time.sleep,
                  now=time.monotonic) -> list[dict]:
    """Sample the frontend with NO input for `seconds`, one line per sample.

    Reads `SCREEN_ID` and `ui_state` (and the shim's keyboard poll counter and
    the foreground pid when available) about every `interval` s. Sends no key
    and writes nothing. It separates "the menu is not interactive yet / the
    game is not polling" from "keys were sent and lost": if screen and poll
    counter are live here and a later DOWN scan still finds nothing, the keys
    are what failed. Returns the samples; `sleep`/`now` are injectable.
    """
    emit = _tee(log, verbose)
    out: list[dict] = []
    t0 = now()
    emit(f"  idle snapshot: {seconds:g}s, every {interval:g}s, no input")
    while True:
        t = now() - t0
        row = {"t": round(t, 2), "screen": i32(h, SCREEN_ID), "ui_state": ui_state(h),
               "key_frame": inp.key_frame if inp is not None else None,
               "foreground": foreground_pid()}
        out.append(row)
        emit(f"  idle t={row['t']:.1f}s screen {row['screen']} ui_state {row['ui_state']} "
             f"key_frame {row['key_frame']} foreground {row['foreground']}")
        if t + interval > seconds:
            break
        sleep(interval)
    return out


def write_mem(h, addr: int, data: bytes) -> bool:
    """Write into the target. Needs a handle from `attach(write=True)`."""
    import ctypes

    n = ctypes.c_size_t(0)
    ok = k32.WriteProcessMemory(h, ctypes.c_void_p(addr), data, len(data), ctypes.byref(n))
    return bool(ok) and n.value == len(data)


def level_table(h) -> dict[str, tuple[int, int, int]]:
    """Every named record in the live table: id -> (group, slot, address).

    Empty slots carry the literal id "blank" and are skipped; the retail data
    fills 102 of the 125, which is exactly the number of `.LVL` files.
    """
    from memdump import read_mem

    out: dict[str, tuple[int, int, int]] = {}
    for g in range(GROUPS):
        for s in range(SLOTS):
            addr = LEVEL_TABLE + g * GROUP_STRIDE + s * SLOT_STRIDE
            raw = read_mem(h, addr, 16)
            if not raw:
                continue
            name = raw.split(b"\0")[0].decode("latin1", "replace")
            if name and name != "blank":
                out[name] = (g, s, addr)
    return out


def select_level(h, name: str, verbose: bool = True):
    """Point the one clickable node at `name`. Returns slot [0][0]'s old bytes.

    The caller must put those bytes back -- `enter_level` does. Raises
    LookupError if the live table has no such level, which is better than
    entering some other level and measuring it by mistake.
    """
    from memdump import read_mem

    table = level_table(h)
    if name not in table:
        raise LookupError(f"{name!r} is not in the live level table ({len(table)} entries)")
    g, s, src = table[name]
    slot0 = LEVEL_TABLE
    saved = read_mem(h, slot0, RECORD_SIZE)
    record = read_mem(h, src, RECORD_SIZE)
    if not saved or not record or len(record) != RECORD_SIZE:
        raise OSError("could not read the level records")
    if not write_mem(h, slot0, record):
        raise OSError("could not write slot [0][0] -- is the handle writable?")
    if verbose:
        print(f"  {name} is group {g} slot {s}; copied its record over slot [0][0]")
    return saved


def loaded_level(h) -> str:
    """The path the loader built, e.g. `Compiled\\MM_DIG_000.lvl`.

    This is the independent answer to "which level is actually running": the
    engine wrote it, not us, and it reflects the file it opened.
    """
    from memdump import read_mem

    raw = read_mem(h, PATH_BUFFER, 64)
    return raw.split(b"\0")[0].decode("latin1", "replace") if raw else ""


def enter_level(h, inp, name: str | None = None, verbose: bool = True) -> int:
    """Enter `name` (or the first node when None) and restore the table after.

    The restore is in a finally, so an exception or a failed entry still
    leaves the game's own level table exactly as it was found.
    """
    saved = None
    try:
        if name:
            saved = select_level(h, name, verbose=verbose)
        return enter(h, inp, verbose=verbose)
    finally:
        if saved is not None:
            ok = write_mem(h, LEVEL_TABLE, saved)
            if verbose:
                print(f"  slot [0][0] restored: {ok}")


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
    for _ in range(8):
        v = i32(h, sel)
        if v == 0:
            break
        # The last item is "Beenden". A value outside 0..4 means this slot
        # is not the highlight, and RETURN from here has closed the game.
        if v is None or not 0 <= v <= 4:
            say(f"  highlight {v} is outside the menu; refusing RETURN")
            return state
        inp.tap(DIK["DOWN"], hold_frames=20)
        time.sleep(0.5)
    if i32(h, sel) != 0:
        say(f"  highlight stayed {i32(h, sel)}; refusing RETURN")
        return state
    say("  highlight now 0 (Neues Spiel starten)")

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
    args = [a for a in sys.argv[1:] if a]
    want_list = "--list" in args
    level = next((a for a in args if not a.startswith("-")), None)

    # A write handle is only needed when a level was named; asking for one
    # unconditionally would make the plain case fail on a locked-down process.
    h, pid = attach(write=bool(level))
    if not h:
        print("the game is not running")
        return 1

    if want_list:
        table = level_table(h)
        print(f"{len(table)} levels in the live table")
        for name, (g, s_, addr) in sorted(table.items(), key=lambda kv: kv[1]):
            print(f"  group {g} slot {s_:2d}  {addr:08x}  {name}")
        k32.CloseHandle(h)
        return 0

    focus(pid)
    with LRInput() as inp:
        rate = poll_rate(inp)
        print(f"pid {pid}, poll {rate:.0f} Hz "
              f"({'foreground' if rate > 60 else 'BACKGROUND -- clicks may miss'})")
        inp.enable()
        try:
            state = enter_level(h, inp, level)
        except LookupError as e:
            print(f"  {e}")
            inp.release()
            k32.CloseHandle(h)
            return 2
        inp.release()
    print(f"final ui_state {state}")
    if state == PLAYING:
        # The engine's own answer to "which level is this", not ours.
        print(f"loaded: {loaded_level(h)}")
        print(f"  time limit {i32(h, TIME_LIMIT)}s, save target {i32(h, SAVE_TARGET)}")
    k32.CloseHandle(h)
    return 0 if state == PLAYING else 2


if __name__ == "__main__":
    raise SystemExit(main())
