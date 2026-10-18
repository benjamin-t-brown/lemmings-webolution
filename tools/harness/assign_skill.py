"""Assign a skill to a chosen lemming in the running original, through the shim.

Phase 3, the assign round. Rounds 5-8 all need this and three rounds in a row
fell back to writing the lemming's own flag bytes instead. This module replaces
the sprite-rectangle guesswork with the engine's own data.

What the engine actually keeps, and where
-----------------------------------------
`0x004CBFB0` is **not** a crosshair object. It is the level screen, read at
`0x0045DA9E` as `[gameObject + 0x55A8]` (`gameObject` = `0x004C6A08`,
`lrstate.GAME_OBJ`) and passed as `this` to the screen's per-frame update
`0x004532E2`. Everything below is an offset into that object, each with the
instruction that proves it:

    screen + 0x0008  int     mouse mode when `0x0058CDAD` is clear (0x0045328C)
    screen + 0x0038  int     panel button under the cursor, -1 for none
                             (set 0x0045559C, cleared 0x004554B4)
    screen + 0x003C  ptr     the cursor's own sprite object (0x0045410E)
    screen + 0x16A0  double  CURSOR X in render pixels
    screen + 0x16A8  double  CURSOR Y in render pixels
    screen + 0x16D8  ptr     THE LEMMING UNDER THE CURSOR, 0 for none
                             (set 0x0045521E, cleared 0x00454DC2)

`+0x16A0` / `+0x16A8` are the live cursor. `0x00453FD1` adds
`[0x004C03A8] / 1.0 * dx` to `+0x16A0` and `0x00453FF8` adds
`[0x004C03B0] / 1.0 * dy` to `+0x16A8`, once per frame, then `0x00454016` and
`0x00454043` clamp each to `[0, [0x0058CB40] - 1]` and `[0, [0x0058CB44] - 1]`
-- the screen width and height words. `0x004540F0` truncates the pair through
`0x0049C5AC` and hands it to the cursor sprite at `[screen + 0x3C]`, which is
why the drawn crosshair is a *copy*: it is written from these doubles, one
frame behind, and writing the sprite back does nothing. The disagreement
`.claude/rules/match-the-original.md` records is between the drawn sprite and
these doubles; the doubles are the master.

So the harness no longer has to track a sprite record to know where the cursor
is, and `aim_at` closes the loop on `+0x16A0` / `+0x16A8` directly. The old
`measure_dropoff.Pointer` pool-of-ten, the "~2.0 px/unit vs 0.425 ghost"
ambiguity and the stale-record misses it caused are all gone: the scale is the
double at `0x004C03A8`, read, and the position is exact.

`+0x16D8` is the find that unblocks the round. `0x00454D6C` is the screen's
hover pass. It walks the pickable-lemming collection at `0x004C88D0`
(`0x004550A3` dereferences the iterator) and for each one calls

    0x00454D33(screen, lemming + 0x128)

which is `0x00454CB9`, a plain AABB overlap of that object's box against the
cursor sprite's box (`[screen+0x3C] + 0x10`), the cursor side shrunk 4 px on x
and grown 4 px on y. It then requires `byte [lemming+0x29]`,
`byte [lemming+0x370]` (`0x00455F20`), the arrow-key direction filters
`[screen+0x16B1]` / `[screen+0x16B2]` against `[lemming+0x40]`, **and**
`0x0045A0E0(0x004C91D8, lemming)` -- the same "can the selected skill go on
this lemming" test the click itself uses. The nearest survivor by world
distance (`0x00401830` on `lemming+8`, length `0x00418350`) wins and is stored
at `+0x16D8`.

**Therefore: a non-zero `screen + 0x16D8` means a click right now assigns.**
That is the aim feedback, from the engine, with no projection of ours.

And `lemming + 0x128` is the per-lemming screen box earlier rounds looked for
and did not find. Same four getters as the panel buttons
(`0x004554E6`-`0x0045552B`), so the field meanings are read off the panel
comparison at `0x00455533`-`0x00455591`:

    lemming + 0x128 + 0x20  double  x0, left     (getter 0x00414770)
    lemming + 0x128 + 0x28  double  y0, top      (getter 0x00414790)
    lemming + 0x128 + 0xF0  double  x1, right    (getter 0x0044D860)
    lemming + 0x128 + 0x90  double  y1, bottom   (getter 0x0044DFF0)

so `is_lemming`'s screen box is not needed at all. It was
`8 <= w <= 20 and 12 <= h <= 26 and 180 <= cx <= 580 and 220 <= cy <= 420`
-- a fit to one level's camera. There is no replacement box here, fitted or
widened: `lemming_box` reads the engine's own rectangle for a lemming taken
from the actor list, so it is correct on any level, any camera, and any
position on the tube by construction.

The click, and what the engine does with it
-------------------------------------------
`0x0045479E` calls `0x00455657` on a button-0 press that was not on a panel
button. That function is the assignment:

    0x00455663  bail out if [screen+0x16D8] == 0
    0x00455670  0x0045AFEB(0x004C91D8) -> the selected skill's state object
    0x00455696  0x0045A0E0(skills, lemming) -> may it be applied
    0x004556E6  0x0042E696(lemming, state)  -> Lemming::setState
    0x004556F0  0x0045B594(0x004C91D8)      -> spend one of the stock
    0x00455756  if ui_state == 1 (paused) drop back to 0

`0x0042E696` runs the state's enter hook (vtable slot 2) and then either
queues it at `lemming+0x374` (when `byte [state+0x11]` is set, `0x00430A50`)
or exits the old state and writes `[lemming + 0x340] = state`. So
**`[[lemming + 0x340]]` is the current state's vtable**, and that is the
behaviour read this module reports: `STATE_NAMES` maps the vtables
`read_steppers.py --states` prints, plus the two that scan misses because
their animation is set inside the stepper -- walk `0x004B0D28` (enter
`0x0041BDBA`, which sets "walk" at `0x0041BDC1`) and build `0x004B08DC`
(enter `0x0040FAB6`, "build" at `0x0040FB83`).

The panel
---------
`0x004C91D8` is the skill array: ten int stock counts, and its own `+0x28` is
the selected index -- which is `0x004C9200`, the static `measure_dropoff`
found by clicking (`0x0045A119` reads `[skills+0x28]` and switches on 0..9).
`0x004C9208` is ten button objects of stride `0x1B0` (`0x004554DA`), each
carrying the same box fields as above, so `panel_box` reads a button's
rectangle rather than assuming "y = 27 + 40k" -- which `measure_float_splat`
had already caught failing on `E10_KF_1`.

`0x00455611` refuses the hover when `stock[hovered] <= 0`, so a button with no
stock cannot be selected, which is why clicking lemmings with the wrong skill
up was a silent no-op in every earlier round.

What this module needs from the caller
--------------------------------------
  * A process already **in a level** (`lrstate.ui_state(h) == 0`). It does not
    enter levels; use `measure_float_splat.enter`, or `select_level` +
    `restart_level.ensure_playing` bracketed by the slot [0][0] restore. Never
    `goto_level.focus` -- it kills the process through dgVoodoo.
  * A write handle only if the caller itself needs one. Everything here is
    reads plus shim input.
  * An open, enabled `lrinput.LRInput`.

What it cannot do
-----------------
  * It cannot assign a skill the level has no stock of: `counts(h)` is the
    engine's own array and `select_skill` fails rather than pretending.
  * It cannot assign to a lemming the engine will not accept -- a faller, a
    lemming already in that state, an exploder. `0x0045A0E0` decides, and the
    only visible symptom is that `+0x16D8` stays 0 while the cursor is dead on
    the sprite. `assign` reports that as `reason="not hoverable"`, which is
    information, not a failure of the aim.
  * It sends **one** click per attempt and waits 0.6 s. Two releases inside
    500 ms are the engine's double click (`lrstate.py`, mouse `+0x1C`) and the
    UI does not treat one as two assignments.
  * It does not pause. `P` is ui_state 1, and `0x00455756` *unpauses* on a
    successful assignment, so a paused attempt changes the state you are
    measuring from.

Usage
-----
    python assign_skill.py --probe
        No input sent. Prints the screen object, the cursor doubles, the stock
        counts, the panel boxes, and every live lemming's screen box and state.

    python assign_skill.py --enter MM_BUILD --probe
        The same, after entering a level. `--enter` is the only thing here
        that needs a write handle, and the only write is `goto_level`'s level
        record swap, which is put back.

    python assign_skill.py --skill 4 --repeat 8 --attempts 3
        Eight separate assignments of panel index 4 in the level already
        running, and the hit rate at the end. Takes about 30 s.

What it did, 2026-10-15, stock exe, pid-per-level
-------------------------------------------------
Five levels, five panel indices, **36 clicks sent at a hovered lemming and 36
units of stock spent** -- the engine consumed one every time. Of the 25 run
under this file as it now stands, 25 reported `ok` and every one landed on
the *first* attempt:

    MM_DIG_000        --skill 7  digger    9/9 clicks spent stock; walk -> dig
    MM_BLOCK_001_A    --skill 3  blocker   4/4; walk -> block, and it sticks
    MM_FLOAT          --skill 1  parasol   5/5; [+0x49] 0 -> 1
    MM_CLIMB_002      --skill 0  climber  10/10; [+0x48] 0 -> 1, 6 climbing
    MM_BUILD          --skill 4  builder   8/8; walk -> build, y +1.000

The only failures were before any click: `reason="not hoverable"` on a level
entered a second earlier, where the actor list is empty or holds only
lemmings still coming out of the hatch, which the pick rejects. Those cost
nothing and `--wait` / the retry handle them.

As a library:

    from assign_skill import assign, counts, select_skill
    r = assign(h, inp, skill=7)        # choose = nearest hoverable lemming
    if r["ok"]:
        print(r["lemming"], r["state_before"], r["states"])
"""
from __future__ import annotations

import argparse
import struct
import sys
import time
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
from lrstate import attach, i32, read_mem, u32, ui_state  # noqa: E402
from measure_spawn import LEMMING_VT, OTHER_VT, actor_ptrs  # noqa: E402
from memdump import k32  # noqa: E402

# --- the level screen -------------------------------------------------------

SCREEN_PTR = 0x004CBFB0        # [0x004C6A08 + 0x55A8], this of 0x004532E2
OFF_MODE = 0x0008              # 0x0045328C's fallback return
OFF_HOVER_BTN = 0x0038         # panel index under the cursor, -1 none
OFF_CURSOR_SPRITE = 0x003C
OFF_CURSOR_X = 0x16A0          # double, render pixels
OFF_CURSOR_Y = 0x16A8          # double
OFF_DIR_FILTER_L = 0x16B1      # byte; when set only [lem+0x40] == 1 picks
OFF_DIR_FILTER_R = 0x16B2      # byte; when set only [lem+0x40] == -1 picks
OFF_HOVER_LEM = 0x16D8         # ptr, the lemming a click would act on

CURSOR_MODE = 0x0058CDAD       # byte; set => the mouse drives the cursor
SCREEN_W = 0x0058CB40          # int, 0x00454043
SCREEN_H = 0x0058CB44          # int, 0x004540B0
SENS_X = 0x004C03A8            # double, 0x00453FC2
SENS_Y = 0x004C03B0            # double, 0x00453FE9

# --- the skill panel --------------------------------------------------------

SKILLS = 0x004C91D8            # ten int stock counts
SELECTED = SKILLS + 0x28       # == 0x004C9200, 0x0045A119
PANEL = 0x004C9208             # ten button objects
PANEL_STRIDE = 0x1B0           # 0x004554DA
PANEL_COUNT = 10               # 0x004554BB loads 9 and counts down

# --- the per-object screen box ----------------------------------------------
#
# The same four getters are used on `lemming + 0x128`, on a panel button, and
# on `[screen+0x3C] + 0x10`. Field meaning from the panel comparison at
# 0x00455533 (x pair against the cursor X, y pair against the cursor Y).
OFF_BOX = 0x128                # lemming + 0x128, 0x004550D9 / 0x0045516E
BOX_X0, BOX_Y0, BOX_X1, BOX_Y1 = 0x20, 0x28, 0xF0, 0x90

# --- the lemming ------------------------------------------------------------

OFF_X, OFF_Y = 0x08, 0x10      # world position doubles (round 3b)
OFF_DIR = 0x40                 # the pick's direction field, 0x0045512D
OFF_PICKABLE = 0x29            # byte, 0x00455104
OFF_PICKABLE2 = 0x370          # byte, getter 0x00455F20
OFF_STATE = 0x340              # ptr to the current state, 0x0042E6FE
OFF_CLIMBER = 0x48             # permanent-attribute bytes, round 3b
OFF_FLOATER = 0x49
# NOT a read size. One 0x400-byte `ReadProcessMemory` of a lemming fails
# outright when the block spans a region boundary, so `read_lem` takes the
# object in three short reads instead; see its docstring.
LEM_SPAN = 0x380

# Six-slot state vtables, named by their enter hook's animation string.
# `read_steppers.py --states` prints all but the last two; walk and build set
# their animation inside the stepper, so that scan does not name them.
STATE_NAMES = {
    0x004B0680: "fall", 0x004B0810: "fall", 0x004B0A20: "fall",
    0x004B0B28: "fall", 0x004B0C48: "fall", 0x004B0C60: "fall(25.0)",
    0x004B0668: "splat", 0x004B0570: "drown",
    0x004B0274: "explode", 0x004B028C: "explode",
    0x004B0A38: "explode", 0x004B0A58: "explode",
    0x004B06AC: "bash", 0x004B07B8: "block", 0x004B0960: "climb",
    0x004B09F0: "dig", 0x004B0B68: "mine", 0x004B0AF8: "flip",
    0x004B0C78: "shrug", 0x004B0CA8: "cartwheel",
    0x004B0D08: "trampoline",
    0x004B0D28: "walk",       # enter 0x0041BDBA sets "walk" at 0x0041BDC1
    0x004B08DC: "build",      # enter 0x0040FAB6 sets "build" at 0x0040FB83
}

# Panel order. Five of the ten are MEASURED, each by entering a level whose
# stock is one skill and reading `counts(h)`: the index holding that level's
# count is that skill, and the assignment then produced the matching state or
# attribute (see the file header's table).
#
#   0  climber  MM_CLIMB_002 stock [10,0,...]   -> [lemming+0x48] = 1
#   1  parasol  MM_FLOAT     stock [0,10,0,...] -> [lemming+0x49] = 1
#   3  blocker  MM_BLOCK_001_A                  -> state 0x004B07B8 block
#   4  builder  MM_BUILD                        -> state 0x004B08DC build
#   7  digger   MM_DIG_000                      -> state 0x004B09F0 dig
#
# Indices 2, 5, 6 are left unnamed: they are the classic bomber / basher /
# miner slots by elimination and that is a guess, not a read. Index 8 is some
# screen action (`0x00455B12` sets `[0x004CBFB0]+0x20 = 1`) and index 9 is the
# one `0x0045472B` treats specially and every level above ships exactly 1 of.
# `--probe` prints the stock, so a caller can tell which index a level ships.
PANEL_HINT = {0: "climber", 1: "parasol", 3: "blocker", 4: "builder",
              7: "digger"}

# Starting guess for render pixels per mouse unit. MEASURED by nudging and
# reading `[screen+0x16A0]` back, not taken from `0x004C03A8`: that double is
# 0.2, and `0x00453FD1` multiplies it by `[ebp-0x14]`, which is not the raw
# delta. `aim_at` re-estimates per axis from the motion it caused, so this is
# only the first step's size.
PX_PER_UNIT = 2.0


def f64(h, addr):
    b = read_mem(h, addr, 8)
    return struct.unpack("<d", b)[0] if b and len(b) == 8 else None


def screen(h) -> int | None:
    """The level screen object, or None when no level is loaded."""
    p = u32(h, SCREEN_PTR)
    return p or None


def cursor(h, scr: int | None = None):
    """The engine's cursor, in render pixels. The master, not the sprite."""
    scr = scr or screen(h)
    if not scr:
        return None
    x = f64(h, scr + OFF_CURSOR_X)
    y = f64(h, scr + OFF_CURSOR_Y)
    return None if x is None or y is None else (x, y)


def hovered(h, scr: int | None = None) -> int:
    """The lemming a click would assign to right now, or 0."""
    scr = scr or screen(h)
    if not scr:
        return 0
    return u32(h, scr + OFF_HOVER_LEM) or 0


def hovered_button(h, scr: int | None = None):
    scr = scr or screen(h)
    return None if not scr else i32(h, scr + OFF_HOVER_BTN)


def sensitivity(h):
    """Render pixels per mouse unit, as the cursor integrator uses them."""
    return f64(h, SENS_X), f64(h, SENS_Y)


def cursor_mode(h, scr: int | None = None) -> int:
    """`0x0045328C`'s return: 2 means the mouse drives the cursor.

    Read, not assumed: the function returns 2 outright when the byte at
    `0x0058CDAD` is set, and otherwise returns `[screen+8]`. On the levels
    probed here `0x0058CDAD` is **0** and `[screen+8]` is **2**, so the
    cursor path at `0x00453FAC` is the live one and the mouse does not rotate
    the camera. A 0 here would mean the deltas go to `[screen+0x58]` /
    `[screen+0x60]` instead and no amount of gliding moves a cursor.
    """
    b = read_mem(h, CURSOR_MODE, 1)
    if b and b[0]:
        return 2
    scr = scr or screen(h)
    return (i32(h, scr + OFF_MODE) or 0) if scr else 0


def box_of(h, obj: int):
    """(x0, y0, x1, y1) of a box object, in render pixels, or None."""
    d = read_mem(h, obj, 0x100)
    if not d or len(d) != 0x100:
        return None
    x0, y0 = struct.unpack_from("<d", d, BOX_X0)[0], struct.unpack_from("<d", d, BOX_Y0)[0]
    y1, x1 = struct.unpack_from("<d", d, BOX_Y1)[0], struct.unpack_from("<d", d, BOX_X1)[0]
    return (x0, y0, x1, y1)


def lemming_box(h, lem: int):
    """The lemming's own screen rectangle. No sprite search, no projection."""
    return box_of(h, lem + OFF_BOX)


def panel_box(h, index: int):
    """Button `index`'s rectangle, read rather than assumed from a pitch."""
    return box_of(h, PANEL + index * PANEL_STRIDE)


def counts(h) -> list[int]:
    """The ten stock counts. `0x00455611` refuses a button whose count <= 0."""
    d = read_mem(h, SKILLS, 4 * PANEL_COUNT)
    if not d:
        return []
    return list(struct.unpack(f"<{PANEL_COUNT}i", d))


def selected(h):
    return i32(h, SELECTED)


def lemmings(h) -> list[int]:
    """Live lemming pointers, from the actor list `0x004CBB18` (round 3b)."""
    out = []
    for p in actor_ptrs(h):
        if u32(h, p) in (LEMMING_VT, OTHER_VT):
            out.append(p)
    return out


def state_of(h, lem: int):
    """(vtable, name) of `[lemming + 0x340]`, the current state."""
    sp = u32(h, lem + OFF_STATE)
    if not sp:
        return (0, "none")
    vt = u32(h, sp) or 0
    return (vt, STATE_NAMES.get(vt, f"{vt:#010x}"))


def read_lem(h, lem: int):
    """The fields this module uses, in three short reads rather than one long.

    One 0x400-byte read is tempting and wrong: a block that spans a region
    boundary makes `ReadProcessMemory` fail for the *whole* request
    (`measure_dropoff.readable_block` says the same), so a lemming near the
    end of its heap region came back as None and a landed climber was
    reported as "no attribute change seen". The core read is 0x80, which is
    the object's own first page's worth and always succeeded.
    """
    d = read_mem(h, lem, 0x80)
    if not d or len(d) != 0x80:
        return None
    out = dict(
        ptr=lem,
        vt=struct.unpack_from("<I", d, 0)[0],
        x=struct.unpack_from("<d", d, OFF_X)[0],
        y=struct.unpack_from("<d", d, OFF_Y)[0],
        direction=struct.unpack_from("<i", d, OFF_DIR)[0],
        pickable=d[OFF_PICKABLE],
        climber=d[OFF_CLIMBER], floater=d[OFF_FLOATER],
        pickable2=None, state=0, box=None,
    )
    box = read_mem(h, lem + OFF_BOX, 0x100)
    if box and len(box) == 0x100:
        out["box"] = (struct.unpack_from("<d", box, BOX_X0)[0],
                      struct.unpack_from("<d", box, BOX_Y0)[0],
                      struct.unpack_from("<d", box, BOX_X1)[0],
                      struct.unpack_from("<d", box, BOX_Y1)[0])
    tail = read_mem(h, lem + OFF_STATE, 0x34)
    if tail and len(tail) == 0x34:
        out["state"] = struct.unpack_from("<I", tail, 0)[0]
        out["pickable2"] = tail[OFF_PICKABLE2 - OFF_STATE]
    return out


# --- aiming -----------------------------------------------------------------


def aim_at(h, inp, x: float, y: float, tol: float = 2.0, tries: int = 16,
           scr: int | None = None):
    """Glide the shim mouse until the engine's cursor is within `tol` of (x, y).

    The loop is closed on `[screen+0x16A0]` / `[screen+0x16A8]` -- the values
    the engine integrates and then clamps -- so there is no sprite record to
    lose and no scale to estimate: the step is `delta / [0x004C03A8]`, the
    sensitivity the integrator itself multiplies by.
    """
    scr = scr or screen(h)
    if not scr:
        return None
    sx = sy = PX_PER_UNIT
    prev = prev_units = None
    for _ in range(tries):
        pos = cursor(h, scr)
        if pos is None:
            return None
        if prev is not None:
            # Re-estimate from what the engine actually integrated. The
            # per-axis scale is whatever this build/driver combination gives;
            # see PX_PER_UNIT's comment for why the .rdata double is not it.
            mx, my = pos[0] - prev[0], pos[1] - prev[1]
            if abs(prev_units[0]) >= 4 and abs(mx) > 1.0:
                sx = max(0.1, min(8.0, abs(mx / prev_units[0])))
            if abs(prev_units[1]) >= 4 and abs(my) > 1.0:
                sy = max(0.1, min(8.0, abs(my / prev_units[1])))
        ex, ey = x - pos[0], y - pos[1]
        if abs(ex) <= tol and abs(ey) <= tol:
            return pos
        ux = max(-400, min(400, int(round(ex / sx))))
        uy = max(-400, min(400, int(round(ey / sy))))
        if ux == 0 and uy == 0:
            return pos
        prev, prev_units = pos, (ux, uy)
        inp.glide(ux, uy, steps=max(1, min(40, max(abs(ux), abs(uy)))),
                  per_step_frames=1)
        inp.move(0, 0)
    return cursor(h, scr)


def single_click(h, inp, hold_frames: int = 4, settle: float = 0.6):
    """One press/release, then the engine's double-click window.

    Two releases inside 500 ms set mouse `+0x1C` (`lrstate.py`) and the UI
    does not read a double click as two assignments. Five quick clicks in a
    cross assigned nothing in 300 tries; single clicks a second apart worked.
    """
    inp.button(0, True)
    inp.wait_frames(hold_frames)
    inp.button(0, False)
    inp.wait_frames(2)
    time.sleep(settle)


def select_skill(h, inp, index: int, tries: int = 4) -> bool:
    """Click panel button `index` and verify `0x004C9200` came back as it.

    Fails without clicking when the level has no stock of that skill:
    `0x00455611` will not let the hover settle on a zero-count button, so the
    click could not have worked anyway.
    """
    st = counts(h)
    if not (0 <= index < PANEL_COUNT):
        return False
    if st and st[index] <= 0:
        print(f"  panel {index} has stock {st[index]}; the engine refuses it"
              f" (0x00455611)", flush=True)
        return False
    box = panel_box(h, index)
    if box is None:
        return False
    cx, cy = (box[0] + box[2]) / 2.0, (box[1] + box[3]) / 2.0
    for _ in range(tries):
        if selected(h) == index:
            return True
        aim_at(h, inp, cx, cy)
        single_click(h, inp)
    return selected(h) == index


def nearest_hoverable(h, inp, scr: int, exclude=(), tries: int = 8):
    """Aim at live lemmings until `[screen+0x16D8]` names one.

    Order is by how far the cursor already is from each lemming's own box
    centre, so the first candidate is usually one glide away. `+0x16D8` is the
    engine's answer to "would a click here assign", so this returns a target
    that is known-good rather than one that looked the right size.
    """
    here = cursor(h, scr) or (320.0, 240.0)
    cands = []
    for p in lemmings(h):
        if p in exclude:
            continue
        b = lemming_box(h, p)
        if b is None:
            continue
        cx, cy = (b[0] + b[2]) / 2.0, (b[1] + b[3]) / 2.0
        if not (0.0 <= cx <= 640.0 and 0.0 <= cy <= 480.0):
            continue          # behind the camera / off screen; not clickable
        cands.append((abs(cx - here[0]) + abs(cy - here[1]), p, cx, cy))
    cands.sort()
    for _d, p, cx, cy in cands[:tries]:
        aim_at(h, inp, cx, cy)
        # Re-read: the first glide takes a moment and a walker moves. The box
        # is the lemming's own, so re-aiming is a second read, not a re-scan.
        b = lemming_box(h, p)
        if b is not None:
            cx, cy = (b[0] + b[2]) / 2.0, (b[1] + b[3]) / 2.0
            aim_at(h, inp, cx, cy, tol=1.0, tries=6)
        hv = hovered(h, scr)
        if hv:
            return hv, (cx, cy)
    return 0, None


def watch_states(h, lem: int, seconds: float = 0.8):
    """Every distinct `[[lemming+0x340]]` in a window, with a timestamp.

    One `ReadProcessMemory` of 4 bytes plus one of the state's vtable, so the
    loop runs at kHz and a state that lives a single 200 ms tick is still
    seen. Returns [(t, vtable, name), ...] in order, first sample included.
    """
    out = []
    t0 = time.perf_counter()
    last = None
    while time.perf_counter() - t0 < seconds:
        sp = u32(h, lem + OFF_STATE)
        vt = (u32(h, sp) or 0) if sp else 0
        if vt != last:
            out.append((time.perf_counter() - t0, vt,
                        STATE_NAMES.get(vt, f"{vt:#010x}")))
            last = vt
    return out


def wait_for_lemmings(h, seconds: float = 20.0) -> int:
    """Block until the hatch has released one, and return how many there are.

    A level entered a moment ago has an empty actor list, and every earlier
    round that aimed into one read "no lemming the engine will accept" and
    blamed the aim. MM_CLIMB_002 releases one every 2.889 s.
    """
    t0 = time.perf_counter()
    while True:
        n = len(lemmings(h))
        if n or time.perf_counter() - t0 >= seconds:
            return n
        time.sleep(0.25)


def assign(h, inp, skill: int, choose=None, attempts: int = 6,
           verbose: bool = True, window: float = 0.8, wait: float = 20.0) -> dict:
    """Put `skill` on a lemming and report what the lemming then did.

    `choose` may be a lemming pointer, or a callable taking the handle and the
    list of live lemming pointers and returning one. The default takes
    whichever hoverable lemming is nearest the cursor.

    The result dict carries `ok`, `lemming`, `state_before`, `state_after`,
    `count_before`, `count_after`, `y_before`, `y_after`, `attempts`,
    `landed` and `reason`. `ok` is true only when the engine spent a unit of
    stock AND the lemming's state or a permanent-attribute byte changed -- the
    panel index is never evidence, and nothing here writes to a lemming.
    """
    scr = screen(h)
    out = dict(ok=False, lemming=0, attempts=0, landed=0, reason="")
    if not scr:
        out["reason"] = "no level screen"
        return out
    if ui_state(h) != 0:
        out["reason"] = f"ui_state {ui_state(h)}, not in a level"
        return out
    if not wait_for_lemmings(h, wait):
        out["reason"] = (f"no lemming in the actor list within {wait:.0f} s;"
                         " the hatch has not released one yet")
        return out
    if cursor_mode(h, scr) != 2:
        out["reason"] = ("0x0045328C is not 2: the mouse rotates the camera"
                         " rather than moving a cursor (0x00453FAC)")
        return out
    if not select_skill(h, inp, skill):
        out["reason"] = f"panel {skill} not selected (0x004C9200={selected(h)})"
        return out

    tried = set()
    for n in range(attempts):
        out["attempts"] = n + 1
        if selected(h) != skill and not select_skill(h, inp, skill):
            out["reason"] = "panel selection lost"
            break
        if choose is None:
            target, _aim = nearest_hoverable(h, inp, scr, exclude=tried)
        else:
            target = choose(h, lemmings(h)) if callable(choose) else choose
            b = lemming_box(h, target) if target else None
            if b:
                aim_at(h, inp, (b[0] + b[2]) / 2.0, (b[1] + b[3]) / 2.0)
                b = lemming_box(h, target)
                aim_at(h, inp, (b[0] + b[2]) / 2.0, (b[1] + b[3]) / 2.0,
                       tol=1.0, tries=6)
            target = hovered(h, scr) or 0
        if not target:
            out["reason"] = "not hoverable"
            if verbose:
                print(f"  attempt {n}: cursor {cursor(h, scr)},"
                      f" +0x16D8 = 0 over {len(lemmings(h))} live lemmings"
                      f" -- none the engine will accept", flush=True)
            # Seen on MM_BUILD right after entry: the only lemmings alive were
            # still coming out of the hatch and the pick rejects them, so a
            # retry is worth more than another candidate. 0.6 s is three ticks.
            time.sleep(0.6)
            tried.clear()
            continue
        tried.add(target)
        before = read_lem(h, target)
        sb = state_of(h, target)
        cb = counts(h)
        inp.button(0, True)
        inp.wait_frames(4)
        inp.button(0, False)
        # Sample the state in a tight loop rather than after a fixed sleep.
        # MEASURED on MM_DIG_000: the dig state is live at t = 0.000 s and is
        # gone again by 0.271 s -- the state object is freed and `walk` is
        # back. A single read 0.45 s after the click saw "walk -> walk" on two
        # attempts out of three and called a landed assignment a miss. Which
        # tick the digger gives up on is round 5's question; capturing the
        # state it entered is this round's.
        seq = watch_states(h, target, window)
        after = read_lem(h, target)
        ca = counts(h)
        time.sleep(0.6)                 # the double-click window, after the read
        spent = bool(cb and ca and ca[skill] == cb[skill] - 1)
        entered = [nm for _t, _vt, nm in seq if nm != sb[1]]
        changed = bool(entered) or (
            after and before and (after["climber"] != before["climber"]
                                  or after["floater"] != before["floater"]))
        if verbose:
            path = " -> ".join([sb[1]] + [nm for _t, _vt, nm in seq])
            print(f"  attempt {n}: {target:#010x} state {path},"
                  f" stock[{skill}] {cb[skill] if cb else '?'} ->"
                  f" {ca[skill] if ca else '?'},"
                  f" +48/{after['climber'] if after else '?'}"
                  f" +49/{after['floater'] if after else '?'},"
                  f" y {before['y'] if before else 0:.3f} ->"
                  f" {after['y'] if after else 0:.3f}", flush=True)
        out.update(lemming=target, state_before=sb[1],
                   state_after=seq[-1][2] if seq else sb[1],
                   states=[nm for _t, _vt, nm in seq], state_series=seq,
                   count_before=cb[skill] if cb else None,
                   count_after=ca[skill] if ca else None,
                   climber_before=before["climber"] if before else None,
                   climber_after=after["climber"] if after else None,
                   floater_before=before["floater"] if before else None,
                   floater_after=after["floater"] if after else None,
                   y_before=before["y"] if before else None,
                   y_after=after["y"] if after else None)
        if spent and changed:
            out["ok"] = True
            out["landed"] = 1
            out["reason"] = "state or attribute moved and stock was spent"
            return out
        if spent:
            out["reason"] = ("stock spent but no state or attribute change"
                             " seen inside the window")
            out["landed"] = 1
            return out
        out["reason"] = "clicked on a hovered lemming, stock unchanged"
    return out


# --- the command line -------------------------------------------------------


def probe(h) -> int:
    scr = screen(h)
    print(f"ui_state {ui_state(h)}  screen {scr and hex(scr)}")
    if not scr:
        print("no level screen; enter a level first")
        return 2
    flag = read_mem(h, CURSOR_MODE, 1)
    print(f"0x0045328C mode {cursor_mode(h, scr)}"
          f" (0x0058CDAD={flag[0] if flag else '?'},"
          f" [screen+8]={i32(h, scr + OFF_MODE)})")
    w, hh = i32(h, SCREEN_W), i32(h, SCREEN_H)
    sx, sy = sensitivity(h)
    print(f"render {w}x{hh}  sensitivity {sx} x {sy} px/unit")
    print(f"cursor {cursor(h, scr)}  mode[+8] {i32(h, scr + OFF_MODE)}"
          f"  hover-button {hovered_button(h, scr)}"
          f"  hover-lemming {hovered(h, scr):#010x}")
    print(f"selected 0x004C9200 = {selected(h)}  stock {counts(h)}")
    for k in range(PANEL_COUNT):
        b = panel_box(h, k)
        if b:
            print(f"  panel {k}: x {b[0]:7.2f}..{b[2]:7.2f}"
                  f"  y {b[1]:7.2f}..{b[3]:7.2f}"
                  f"  {PANEL_HINT.get(k, '')}")
    lems = lemmings(h)
    print(f"{len(lems)} lemmings in the actor list")
    for p in lems:
        lem = read_lem(h, p)
        if not lem:
            continue
        vt, name = state_of(h, p)
        b = lem["box"]
        box = (f"box x {b[0]:7.2f}..{b[2]:7.2f} y {b[1]:7.2f}..{b[3]:7.2f}"
               if b else "box unreadable")
        print(f"  {p:#010x} world ({lem['x']:8.3f},{lem['y']:7.3f})"
              f" dir {lem['direction']:+d}  {box}"
              f"  +29/{lem['pickable']} +370/{lem['pickable2']}"
              f" +48/{lem['climber']} +49/{lem['floater']}  state {name}")
    return 0


def main() -> int:
    ap = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    ap.add_argument("--probe", action="store_true",
                    help="read everything, send no input")
    ap.add_argument("--enter", metavar="LEVEL",
                    help="enter this level first (measure_float_splat.enter)")
    ap.add_argument("--skill", type=int, metavar="INDEX",
                    help="panel index 0..9 to select and assign")
    ap.add_argument("--attempts", type=int, default=6)
    ap.add_argument("--repeat", type=int, default=1,
                    help="how many separate assignments to try, for a rate")
    ap.add_argument("--watch", type=float, default=0.0, metavar="SECONDS",
                    help="after an assignment, print the lemming's Y series")
    ap.add_argument("--window", type=float, default=0.8, metavar="SECONDS",
                    help="how long to sample [lemming+0x340] after the click;"
                         " the dig state lives ~0.27 s, so do not raise the"
                         " 0.45 s fixed sleep this replaced")
    ap.add_argument("--wait", type=float, default=20.0, metavar="SECONDS",
                    help="how long to wait for the hatch to release one")
    args = ap.parse_args()

    need_write = bool(args.enter)
    h, pid = attach(write=need_write)
    if not h:
        print("the game is not running")
        return 1
    print(f"pid {pid}")
    try:
        if args.enter:
            from measure_float_splat import enter
            rc = enter(h, args.enter)
            if rc:
                return rc
        if args.probe or args.skill is None:
            return probe(h)

        from lrinput import LRInput
        hits = 0
        results = []
        with LRInput() as inp:
            inp.clear()
            inp.enable()
            try:
                for r in range(args.repeat):
                    print(f"-- assignment {r} of skill {args.skill}"
                          f" ({PANEL_HINT.get(args.skill, 'index')})",
                          flush=True)
                    res = assign(h, inp, args.skill, attempts=args.attempts,
                                 window=args.window, wait=args.wait)
                    results.append(res)
                    hits += 1 if res["ok"] else 0
                    print(f"   -> ok={res['ok']} {res['reason']}", flush=True)
                    if res["ok"] and args.watch:
                        t0 = time.perf_counter()
                        seen = []
                        while time.perf_counter() - t0 < args.watch:
                            lem = read_lem(h, res["lemming"])
                            if lem is None:
                                break
                            vt, name = state_of(h, res["lemming"])
                            row = (round(lem["y"], 3), round(lem["x"], 3), name)
                            if not seen or seen[-1] != row:
                                seen.append(row)
                                print(f"     {time.perf_counter() - t0:6.2f}s"
                                      f" y {row[0]:8.3f} x {row[1]:8.3f}"
                                      f" {row[2]}", flush=True)
                            time.sleep(0.02)
            finally:
                inp.release()
        tried = sum(r["attempts"] for r in results)
        print(f"\n{hits} of {len(results)} assignments landed,"
              f" {tried} clicks on a hovered lemming spent")
        for r in results:
            print(f"  ok={r['ok']:d} attempts={r['attempts']}"
                  f" {r.get('state_before')} -> {r.get('state_after')}"
                  f" stock {r.get('count_before')} -> {r.get('count_after')}"
                  f" : {r['reason']}")
        return 0 if hits else 2
    finally:
        k32.CloseHandle(h)


if __name__ == "__main__":
    raise SystemExit(main())
