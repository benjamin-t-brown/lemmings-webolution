"""Drive the original with memory feedback instead of screenshots.

The harness used to aim by finding the green crosshair in a `PrintWindow`
capture, and to decide what screen it was on from the frame's brightness.
Both are unreliable here: the capture comes back all black whenever the game
window is not foreground, and nothing distinguishes that from a dark screen.

The engine answers both questions directly, so this module asks it:

    0x004C36A0 / 0x004C36A4   cursor x, y in render pixels, clamped to the
                              render size (2560x1440 after lr_patch.py)
    0x004CC134                UI state, the switch variable of the main loop
    InputManager -> mouse     where the shim's bytes actually land

All three were pinned down by experiment, not inference: the cursor pair by
injecting a marker delta and diffing .data across a known move, and the mouse
object by searching the whole address space for that same marker.

`aim_at` is then a plain closed loop. Pointer acceleration is still on, so the
scale is re-estimated from the previous step exactly as the screenshot aimer
did -- the difference is only that the feedback now costs a 4-byte read and
cannot silently be a black frame.
"""
from __future__ import annotations

import struct
import sys
import time
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
from lrstate import attach, read_mem, ui_state  # noqa: E402

CURSOR_X = 0x004C36A0
CURSOR_Y = 0x004C36A4


def cursor(h) -> tuple[int, int]:
    b = read_mem(h, CURSOR_X, 8)
    return struct.unpack("<ii", b)


def aim_at(h, inp, target, tries: int = 30, tol: int = 6):
    """Walk the cursor onto `target` (render pixels). Returns where it ended.

    One loop, because frame-synced motion turns out to be linear. Pointer
    acceleration only bites when several units arrive in the same poll; a
    `glide` that delivers exactly one unit per polled frame moves a steady
    ~1.7 px per unit at any distance, so the whole screen is two or three
    iterations away and the ratio is re-estimated from what actually
    happened rather than assumed.

    The earlier screenshot aimer could not do this: it had to spend a capture
    per iteration, and a capture can silently be a black frame. Reading the
    engine's own cursor costs four bytes and cannot lie.
    """
    px_per_unit = 1.7
    prev = None
    prev_units = (0, 0)
    for _ in range(tries):
        pos = cursor(h)
        if prev:
            moved = max(abs(pos[0] - prev[0]), abs(pos[1] - prev[1]))
            units = max(abs(prev_units[0]), abs(prev_units[1]))
            if moved > 8 and units > 4:
                px_per_unit = max(0.8, min(4.0, moved / units))
        ex, ey = target[0] - pos[0], target[1] - pos[1]
        if abs(ex) <= tol and abs(ey) <= tol:
            return pos
        ux = max(-400, min(400, round(ex / px_per_unit)))
        uy = max(-400, min(400, round(ey / px_per_unit)))
        if ux == 0 and uy == 0:
            return pos
        prev, prev_units = pos, (ux, uy)
        inp.glide(ux, uy, steps=max(1, abs(ux), abs(uy)), per_step_frames=1)
        time.sleep(0.05)
    return cursor(h)


def click_at(h, inp, target, hold_frames: int = 6, double: bool = False):
    """Aim, then click. Returns (final cursor, ui_state before, ui_state after)."""
    pos = aim_at(h, inp, target)
    before = ui_state(h)
    rounds = 2 if double else 1
    for _ in range(rounds):
        inp.button(0, True)
        inp.wait_frames(hold_frames)
        inp.button(0, False)
        inp.wait_frames(hold_frames)
    time.sleep(0.8)
    return pos, before, ui_state(h)
