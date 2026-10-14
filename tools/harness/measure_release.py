"""Time the original's lemming releases, and test what drives its simulation.

Two things fall out of one measurement.

*The release interval.* `RELEASERATE.PC` ships 99 doubles, seconds per
lemming, and our `LevelSetup::releaseIntervalTicks` is derived from it. Timing
the engine's own released counter against a wall clock says whether that
derivation is right, in seconds, without needing to find a single lemming in
memory.

*Whether there is a fixed simulation tick at all.* The render and input loop
is locked to the display -- 144 Hz with the window foreground, about 31 Hz
when it is not, both measured through the shim's `GetDeviceState` counter. So
running the same measurement in both states separates the two possibilities:
if the releases keep the same spacing in real seconds, the simulation is
driven by a real-time clock and the engine has no fixed tick for us to match;
if the spacing stretches by the frame-rate ratio, it is frame-locked and the
tick rate is the frame rate.

Statics used, all found by diffing live memory (see PLAN.md 2.2):

    0x004CBA78   lemmings released so far   (0 .. total)
    0x004CBAA8   the level's time limit, whole seconds
    0x004CBAAC   the clock, whole seconds remaining
"""
from __future__ import annotations

import sys
import time
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
from lrstate import attach, i32, ui_state  # noqa: E402
from memdump import k32  # noqa: E402

RELEASED = 0x004CBA78
LIMIT = 0x004CBAA8
CLOCK = 0x004CBAAC


def watch(h, inp=None, total=10, timeout=180.0):
    """Timestamp every increment of the released counter."""
    events = []
    last = i32(h, RELEASED)
    t0 = time.perf_counter()
    start_clock = i32(h, CLOCK)
    while time.perf_counter() - t0 < timeout:
        v = i32(h, RELEASED)
        if v is None:
            break
        if v != last:
            events.append((time.perf_counter() - t0, last, v, i32(h, CLOCK)))
            last = v
            if v >= total:
                break
        time.sleep(0.002)
    return events, start_clock


def unfocus() -> None:
    """Take the foreground away from the game, which drops it to ~31 Hz."""
    import ctypes

    user32 = ctypes.WinDLL("user32")
    k = ctypes.WinDLL("kernel32")
    con = k.GetConsoleWindow()
    if con:
        user32.ShowWindow(con, 9)
        user32.SetForegroundWindow(con)
    time.sleep(0.5)


def poll_hz() -> float:
    from lrinput import LRInput

    with LRInput() as inp:
        f0 = inp.mouse_frame
        time.sleep(1.0)
        return inp.mouse_frame - f0


def main() -> int:
    total = int(sys.argv[1]) if len(sys.argv) > 1 else 10
    background = "--background" in sys.argv
    h, pid = attach()
    if not h:
        print("not running")
        return 1
    if background:
        unfocus()
    else:
        from goto_level import focus
        focus(pid)
    print(f"engine loop: {poll_hz():.0f} Hz")
    if ui_state(h) != 0:
        print("not in a level; run fresh_level.py first")
        return 1
    print(f"limit {i32(h, LIMIT)}s, clock {i32(h, CLOCK)}s, "
          f"released {i32(h, RELEASED)}")
    events, _ = watch(h, total=total)
    print(f"\n{len(events)} releases")
    prev = 0.0
    gaps = []
    for t, a, b, clock in events:
        gaps.append(t - prev)
        print(f"  t={t:7.3f}s  {a} -> {b}   gap {t - prev:6.3f}s   clock {clock}")
        prev = t
    if len(gaps) > 2:
        body = gaps[1:]
        print(f"\nmean gap after the first: {sum(body) / len(body):.3f}s "
              f"(min {min(body):.3f}, max {max(body):.3f})")
    k32.CloseHandle(h)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
