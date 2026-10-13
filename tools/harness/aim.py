"""Point the original's cursor at a screen position, reliably.

Aiming is harder here than it looks, and each of these details cost a round
trip to establish. Written down so the next attempt starts from them.

**Pointer acceleration is on, so there is no single scale.** Measured in a
2560x1440 window under dgVoodoo:

    burst of   5 DirectInput units  ->  ~1.0 screen px per unit
    burst of  30 units              ->  ~2.7 px per unit
    burst of 300 units              ->  ~8.2 px per unit

A fixed factor therefore oscillates: correct a 30 px error with a "30 px"
move and you overshoot by 50, then overshoot back. `aim_at` estimates the
scale from what actually happened on the previous step instead, and converges
in two or three moves. PLAN.md's earlier note of "2.5-3.8 units per pixel"
was a mid-range sample of this same curve, and its reciprocal at that.

**One unit per step, frame-synced.** The shim holds a single pending delta
which the game must poll before the next is written; writing faster simply
overwrites. `LRInput.glide(dx, dy, steps=n, per_step_frames=1)` waits a frame
per step, so pass `steps = max(|dx|, |dy|)` to keep each step at one unit.
Calling `move()` in a tight loop does *not* work -- the deltas coalesce, and
the acceleration curve above turns a 60-unit nudge into a 500 px jump.

**The cursor can vanish.** Parking hard into a corner with a large negative
glide can leave the game not drawing its crosshair at all, and small positive
moves will not bring it back; a large one does. If it cannot be found, sweep
rather than nudge.

**Measure while enabled.** Releasing the shim hands the device back to the
real mouse and the cursor drifts, so a position measured before a release is
stale. Aim and click inside one enabled session.

Colour beats differencing for finding the cursor, once you know the cursor is
actually on screen: it is the only strongly green-dominant thing on either
the dark menu or the pale level map. (`enter_and_dump.crosshair` is tuned for
the menu alone and finds nothing on the map, which is what sent an earlier
attempt down the differencing route.)
"""
from __future__ import annotations

import ctypes
import time


class BlankFrame(RuntimeError):
    """Raised when a capture came back black, which is not a game state.

    PrintWindow on this Direct3D window returns an all-black bitmap whenever
    the window is not the foreground one. Nothing about that is distinguishable
    from a legitimately dark screen by brightness alone, and treating it as one
    is how an earlier attempt concluded it was at the main menu while a level
    was actually running. Every vision helper here goes through
    `require_live_frame`, so a blank capture stops the run instead of quietly
    steering it.
    """


def is_blank(img, threshold: float = 0.5) -> bool:
    small = img.convert("L").resize((64, 36))
    px = list(small.getdata())
    return (sum(px) / len(px)) < threshold


def focus(hwnd) -> bool:
    """Brings the game forward. Windows may refuse, so the result is checked."""
    u = ctypes.windll.user32
    u.ShowWindow(hwnd, 9)          # SW_RESTORE
    u.SetForegroundWindow(hwnd)
    time.sleep(0.4)
    return u.GetForegroundWindow() == hwnd


def require_live_frame(shot_fn, hwnd=None, tries: int = 3):
    """A capture that is actually showing something, or BlankFrame."""
    for _ in range(tries):
        img = shot_fn()
        if not is_blank(img):
            return img
        if hwnd is not None:
            focus(hwnd)
        time.sleep(0.6)
    raise BlankFrame(
        "capture is black: the game window is probably not foreground. "
        "Vision results from such a frame mean nothing."
    )


def find_cursor(img, step: int = 2):
    """The game's green crosshair, or None when it is not being drawn."""
    px = img.convert("RGB").load()
    w, h = img.size
    pts = [
        (x, y)
        for y in range(0, h, step)
        for x in range(0, w, step)
        if px[x, y][1] > 100
        and px[x, y][1] - px[x, y][0] > 40
        and px[x, y][1] - px[x, y][2] > 40
    ]
    if not pts:
        return None
    return (sum(p[0] for p in pts) // len(pts), sum(p[1] for p in pts) // len(pts))


def gold_nodes(img, step: int = 2, min_pixels: int = 100):
    """The playable (gold) nodes on the level map, ordered top to bottom."""
    px = img.convert("RGB").load()
    w, h = img.size
    pts = [
        (x, y)
        for y in range(0, h, step)
        for x in range(0, w, step)
        if px[x, y][0] > 190 and 140 < px[x, y][1] < 210 and px[x, y][2] < 80
    ]
    clusters: list[list[tuple[int, int]]] = []
    for p in pts:
        for c in clusters:
            if abs(c[0][0] - p[0]) < 40 and abs(c[0][1] - p[1]) < 40:
                c.append(p)
                break
        else:
            clusters.append([p])
    nodes = [
        (sum(q[0] for q in c) // len(c), sum(q[1] for q in c) // len(c))
        for c in clusters
        if len(c) >= min_pixels
    ]
    return sorted(nodes, key=lambda n: n[1])


def sweep_for_cursor(shot_fn, inp, settle: float = 0.5):
    """Brings the crosshair back when the game has stopped drawing it."""
    for dx, dy in ((-3000, -3000), (400, 300), (1200, 800), (-1200, -800)):
        inp.glide(dx, dy, steps=40, per_step_frames=1)
        time.sleep(settle)
        pos = find_cursor(shot_fn())
        if pos:
            return pos
    return None


def aim_at(shot_fn, inp, target, tries: int = 10, tol: int = 8, settle: float = 0.35):
    """Walks the cursor onto `target`, learning the scale as it goes.

    Returns the final cursor position, or None if it could not be found.
    """
    scale = 1.2  # px per unit, refined below from what actually happens
    prev_pos = None
    prev_units = 0

    for _ in range(tries):
        pos = find_cursor(shot_fn())
        if pos is None:
            pos = sweep_for_cursor(shot_fn, inp)
            if pos is None:
                return None
        if prev_pos and prev_units:
            moved = pos[0] - prev_pos[0]
            if abs(moved) > 2:
                scale = max(0.6, min(9.0, abs(moved / prev_units)))

        ex = target[0] - pos[0]
        ey = target[1] - pos[1]
        if abs(ex) <= tol and abs(ey) <= tol:
            return pos

        ux = int(ex / scale) or (1 if ex > 0 else -1 if ex else 0)
        uy = int(ey / scale)
        prev_pos, prev_units = pos, ux
        inp.glide(ux, uy, steps=max(1, abs(ux), abs(uy)), per_step_frames=1)
        time.sleep(settle)

    return find_cursor(shot_fn())
