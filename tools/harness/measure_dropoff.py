"""Read `Rules::fallSpeed` off a climber dropping from MM_CLIMB_002's y = 48 ring.

Why this script and not `measure_fall.py` / `catch_fall.py`
-----------------------------------------------------------
Those two go after the hatch, and the hatch does not yield the number: on this
level the new billboard is written at y = 32.190 and stays there. The only
descent the level offers is the one at the top of the climb, so this script
makes a climber on purpose and watches it come down.

The earlier look at that descent is the false start this replaces. It saw
y = 48.190 and then y = 32.190 about 0.39 s later with nothing in between, and
16 world units over 0.39 s is not a rate -- there were no intermediate heights
to divide. Two things were wrong with how it was sampled: it polled several
billboard addresses round-robin out of one loop, so the per-address interval
was tens of milliseconds rather than the loop's, and it only ever watched the
one quad it had locked on to.

So `follow` polls a single quad address in a tight loop -- a 96-byte
`ReadProcessMemory`, which runs at a few kHz -- and keeps every distinct value
with a timestamp. A torn read (the engine rewriting the quad while we read it)
is discarded by requiring all four vertices to share one y, which is the
billboard's defining property.

Where the lemmings are on screen
--------------------------------
Everything the engine draws is a sprite record: four corner (x, y) f64 pairs
0x68 apart, in render pixels. The crosshair object behind `0x004CBFB0` is laid
out exactly that way (corners at +0xa0, +0x108, +0x170, +0x1d8, and the
sprite's centre as two ints at +0x29c). So scanning for 2-by-2 rectangles of
f64 inside 640x480 finds every drawn sprite, and that is how this script knows
where to click: lemmings are the 9-13 x 19-20 px ones in the play area, and the
skill panel is the vertical strip of ten ~18x19 ones at x ~ 38, y 26..386 --
which is in *static* memory (0x004ca210 .. 0x004cb590), not the heap.

The rectangles are not matched to particular lemmings and do not need to be.
An earlier attempt tried to pair each f64 pair with the billboard it sits in
front of (the pairs really are at a stable offset, quad-0x2a8 ... quad-0x170)
and the centres did not track the lemming's column: one walker crossing
columns 135 -> 150 gave centres 454, 468, 479, 481, 466, 451. The draw list
looks depth-sorted, so a record holds a different lemming from frame to frame.
Clicking the centre of *some* lemming is all that is wanted.

Three other things that do not work
-----------------------------------
  * **The cursor statics do not drive the in-level hit test.** Writing
    0x004C36A0 / 0x004C36A4 sticks -- nothing in play overwrites them -- but a
    click with a lemming's screen position written there assigns nothing, and a
    cursor sweep over 1,323 positions moved no other byte in the crosshair
    object or the statics around it. In play that pair is dead memory holding
    the last level-map click. PLAN.md's handoff was wrong about this.
  * **The crosshair's integer position is a copy, not the master.** Shim mouse
    motion moves the corners and the ints together, so the crosshair is the
    in-level pointer -- but writing the ints leaves the corners where they
    were. The position the engine integrates lives elsewhere, so the pointer
    cannot be teleported by write; it has to be glided, with the centre of the
    four corners as feedback (`glide_to`).
  * **The pointer's own sprite record is not at a fixed offset.** It was at
    `[0x004CBFB0] + 0xa0` in two process runs and somewhere else in a third,
    where that offset held a sprite that never moved -- so `glide_to` span its
    fourteen iterations without the cursor leaving the centre of the screen
    and every click of that run landed on (320, 240). `find_pointer` injects a
    delta and keeps the record that follows it, which is the only way to be
    sure. Two records move: one at about 2.0 px per mouse unit and one at
    0.425. The fast one is the pointer (the frontend's cursor moves ~1.7
    px/unit through the same shim).
  * **Clicking a lemming with the wrong skill selected is a silent no-op.**
    MM_CLIMB_002 ships ten climbers and zero of everything else, so only one
    panel button can assign anything, and the original does *not* auto-select
    it (PLAN.md's handoff was wrong about that too): a sweep of 1,000 clicks
    across the whole play area, and clicks dead on the centre of every
    lemming-sized sprite rectangle, assigned nothing at all. `P` pauses the
    level (ui_state 1, not 2), which is how in-level input was confirmed to
    arrive at the screen code before any of this was blamed on the clicks.

The panel, and the one static that proves a click landed
--------------------------------------------------------
`0x004C9200` is the selected panel index. Clicking down the x = 38 strip while
paused walks it 3, 5, 7 for y = 147, 228, 307 -- one index per 40 px, so index
k sits at y ~ 27 + 40k and the strip is ten buttons from y 27 to 386. It is
the only in-level static that moves with a panel click, and reading it back is
how `select_skill` knows the click was not thrown away.

Do not call `goto_level.focus` from here
----------------------------------------
It kills the game. `focus` only raises the window when it first joins the
current foreground thread's input queue, and the activation that then succeeds
faults inside dgVoodoo's `DDraw.dll` -- `0xc0000005` at offset 0x2f3e1, five
times out of five, with `FullScreenMode = true` and
`FullscreenAttributes = fake` in `dgVoodoo.conf`. Every individual piece of
`focus` is harmless on its own (`SetForegroundWindow`, which Windows refuses;
`ShowWindow`; `BringWindowToTop`; `AttachThreadInput`); the combination that
actually activates the window is what crashes. It is not needed: the shim
serves input from shared memory whatever has the foreground, and it polled at
**144 Hz** with the game in the background throughout this work.

What it found
-------------
See PLAN.md 2.2 and Phase 3. The script prints the whole height series, so the
step can be read rather than inferred.

    python measure_dropoff.py [follow-seconds]
"""
from __future__ import annotations

import ctypes
import ctypes.wintypes as wt
import statistics
import struct
import sys
import time
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
from find_fallers import heap_regions  # noqa: E402
from live_terrain_check import PIXELS_PER_UNIT  # noqa: E402
from lrstate import attach, i32, read_mem, u32, ui_state  # noqa: E402
from measure_motion import column, wrap  # noqa: E402
from memdump import k32  # noqa: E402

RELEASED = 0x004CBA78
CLOCK = 0x004CBAAC
CROSS_PTR = 0x004CBFB0
CROSS_CORNER = 0xA0        # four corner (x, y) f64 pairs, 0x68 apart
SELECTED = 0x004C9200      # the selected panel index, 0..9
PANEL_X = 38.0             # the panel strip's x, in render pixels
PANEL_Y0 = 27.0            # button 0's y; one button every PANEL_DY
PANEL_DY = 39.9
CLIMBER_INDEX = 0          # top button of the strip; index 1 is the parasol
SPRITE_STRIDE = 0x68 // 8  # in doubles
SPRITE_BYTES = 3 * 0x68 + 16
QUAD = 12 * 8
RING = 32.190              # the height MM_CLIMB_002's walkers patrol at
PANEL = (0x004C8000, 0x4000)

k32.WriteProcessMemory.argtypes = [
    wt.HANDLE, ctypes.c_void_p, ctypes.c_void_p, ctypes.c_size_t,
    ctypes.POINTER(ctypes.c_size_t),
]
k32.WriteProcessMemory.restype = wt.BOOL


def quad(h, addr):
    """The twelve doubles of a billboard, or None if the read looks torn."""
    d = read_mem(h, addr, QUAD)
    if not d or len(d) != QUAD:
        return None
    v = struct.unpack("<12d", d)
    if not (v[2] == v[5] == v[8] == v[11]):
        return None
    return v


def height(v) -> float:
    return v[2]


def rect_of(buf: bytes, off: int = 0, margin: float = 0.0):
    """The sprite rectangle starting at `off`, or None if it is not one.

    `margin` widens the screen box. The pointer needs it: the engine lets the
    cursor's centre go right to the edge, so its 32x32 corners leave 640x480
    and a strict box rejects the live record exactly when the pointer has been
    glided too far. That is what made `glide_to` report "landed None" and then
    click wherever the cursor had been left -- three runs' worth.
    """
    try:
        pts = [struct.unpack_from("<2d", buf, off + k * 0x68) for k in range(4)]
    except struct.error:
        return None
    xs = {round(p[0], 3) for p in pts}
    ys = {round(p[1], 3) for p in pts}
    if len(xs) != 2 or len(ys) != 2:
        return None
    if not all(-margin <= x <= 640.0 + margin for x in xs):
        return None
    if not all(-margin <= y <= 480.0 + margin for y in ys):
        return None
    lo_x, hi_x = min(xs), max(xs)
    lo_y, hi_y = min(ys), max(ys)
    return ((lo_x + hi_x) / 2.0, (lo_y + hi_y) / 2.0, hi_x - lo_x, hi_y - lo_y)


def scan(h):
    """One pass over the heap for both things this script needs.

    Returns the addresses of the drawn sprites' corner records and a map of
    billboard quad address -> its current height. Both used to be their own
    full scan (`sprite_slots` and `measure_motion.quads`), which at 180 MB a
    pass cost most of a 300-second level before a single click was sent.

    The sprite pool is allocated once, so the addresses stay put for the level
    and only their values change; re-reading them later costs a few
    milliseconds where a re-scan costs tens of seconds.
    """
    sprites = []
    bill = {}
    import math

    for base, size in heap_regions(h):
        d = read_mem(h, base, size)
        if not d:
            continue
        n = len(d) // 8
        if n < 3 * SPRITE_STRIDE + 2:
            continue
        v = struct.unpack_from(f"<{n}d", d, 0)
        for i in range(n - 3 * SPRITE_STRIDE - 2):
            x0 = v[i]
            if 0.0 <= x0 <= 640.0:
                xs = (x0, v[i + SPRITE_STRIDE], v[i + 2 * SPRITE_STRIDE],
                      v[i + 3 * SPRITE_STRIDE])
                ys = (v[i + 1], v[i + SPRITE_STRIDE + 1],
                      v[i + 2 * SPRITE_STRIDE + 1], v[i + 3 * SPRITE_STRIDE + 1])
                if (all(0.0 <= x <= 640.0 for x in xs)
                        and all(0.0 <= y <= 480.0 for y in ys)
                        and len({round(x, 3) for x in xs}) == 2
                        and len({round(y, 3) for y in ys}) == 2):
                    sprites.append(base + i * 8)
            # The billboard test, as measure_motion.quads does it.
            y = v[i + 2]
            if not (0.5 < y < 95.0):
                continue
            if v[i + 5] != y or v[i + 8] != y or v[i + 11] != y:
                continue
            if all(40.0 < math.hypot(v[i + k], v[i + k + 1]) < 70.0
                   for k in (0, 3, 6, 9)):
                bill[base + i * 8] = y
    return sprites, bill


def rects_now(h, slots):
    out = []
    for a in slots:
        d = read_mem(h, a, SPRITE_BYTES)
        if not d:
            continue
        r = rect_of(d)
        if r:
            out.append((a,) + r)
    return out


def is_lemming(cx, cy, w, ht) -> bool:
    """SUPERSEDED. A fit to MM_CLIMB_002's camera; do not reuse it.

    This box is unchanged, because `assign_climber` below is the one path it
    is known to work on and that is now the regression case. It is a **fit**,
    not a rule: the screen box `180 <= cx <= 580 and 220 <= cy <= 420` was
    taken from one level's camera, and lemmings elsewhere fall straight out of
    it. Measured 2026-10-15 with `assign_skill.py --probe`:

        MM_DIG_000   20 walkers, boxes at x  92..361, y  77..126
        MM_BUILD     14 lemmings, boxes at x  98..322, y 182..306
        MM_FLOAT      2 walkers,  boxes at x 344..429, y  80..133

    Every one of those is outside the `cy` window, which is exactly why the
    float/splat round's `lemming_slots` found no target on MM_FLOAT and every
    click was a no-op before it was sent.

    There is no wider box here, because the engine stores the rectangle:
    `assign_skill.lemming_box` reads `lemming + 0x128`, the same four fields
    the picking routine itself compares against the cursor
    (`0x00454D33` / `0x00454CB9`). Use `assign_skill.assign` for any new work.
    """
    return (8.0 <= w <= 20.0 and 12.0 <= ht <= 26.0
            and 180.0 <= cx <= 580.0 and 220.0 <= cy <= 420.0)


def lemming_slots(h, slots):
    """The handful of sprite records currently drawing a lemming.

    Re-reading these costs about a millisecond where re-reading all 13,000
    records costs 400 ms -- and that latency is the difference between a click
    that lands and one that does not: a lemming crosses its own 12-pixel
    sprite in under a second.
    """
    return [a for a, cx, cy, w, ht in rects_now(h, slots)
            if is_lemming(cx, cy, w, ht)]


def lemming_targets(h, slots, seen_tol=8.0):
    """Centres of the lemming-sized sprites in the play area, de-duplicated."""
    out = []
    for _a, cx, cy, w, ht in rects_now(h, slots):
        if not is_lemming(cx, cy, w, ht):
            continue
        if any(abs(cx - px) < seen_tol and abs(cy - py) < seen_tol
               for px, py in out):
            continue
        out.append((cx, cy))
    return out


def click_lemming(h, inp, ptr, fast_slots, avoid=None):
    """Aim at a lemming, re-aim once it has been approached, then click.

    The second aim is the point: the first glide takes about a second, over
    which the lemming has walked 15 screen pixels, which is more than the
    sprite is wide.
    """
    targets = lemming_targets(h, fast_slots)
    if avoid is not None:
        targets = [t for t in targets
                   if abs(t[0] - avoid[0]) > 10 or abs(t[1] - avoid[1]) > 10]
    if not targets:
        return None, None
    here = ptr.pos(h) or (320.0, 240.0)
    target = min(targets, key=lambda t: abs(t[0] - here[0]) + abs(t[1] - here[1]))
    glide_to(h, inp, ptr, target)
    again = lemming_targets(h, fast_slots)
    if again:
        here = ptr.pos(h) or target
        target = min(again,
                     key=lambda t: abs(t[0] - here[0]) + abs(t[1] - here[1]))
        glide_to(h, inp, ptr, target, tries=4, tol=2.0)
    # One click, and then wait. Five quick clicks in a cross around the centre
    # assigned nothing at all in 300 tries, where single clicks a second apart
    # had worked twice: releases less than 500 ms apart are the engine's
    # double-click (lrstate.py, mouse +0x1c), and the UI evidently does not
    # treat a double click as two assignments.
    inp.button(0, True)
    inp.wait_frames(4)
    inp.button(0, False)
    inp.wait_frames(2)
    time.sleep(0.6)
    return target, ptr.pos(h)


def panel_buttons(h):
    """The ten skill-panel buttons, top to bottom, from static memory."""
    found = {}
    base, size = PANEL
    d = read_mem(h, base, size)
    if not d:
        return []
    for off in range(0, size - SPRITE_BYTES, 8):
        r = rect_of(d, off)
        if not r:
            continue
        cx, cy, w, ht = r
        if cx < 70.0 and 10.0 < cy < 470.0 and w > 9.0 and ht > 9.0:
            found[(round(cx, 1), round(cy, 1))] = base + off
    return sorted(found)


CURSOR_POOL = 10           # sprite records the cursor rotates through


class Pointer:
    """The in-level mouse pointer, tracked by which record just changed.

    The cursor is drawn out of a pool of ten sprite records at
    `[0x004CBFB0] + 0xa0 + k * 0x238`, and only one of them is live at a
    time. Reading a fixed one is what wrecked every closed-loop aim in this
    script's history: `+0xa0` follows a big glide exactly (2.0 px per mouse
    unit, which is the real scale) and then goes stale, freezing at
    (338, 297) while `+0x2d8` walked 342, 346, 350 ... 398 in step with eight
    2-unit nudges. Feeding a stale record back made the aim miss by 25-30 px
    and the assignment land about one time in eight.

    So `pos` reads all ten and believes the one that moved since the last
    call, which is unambiguous because nothing else writes them between two
    of our own glides.
    """

    def __init__(self, h, cross: int):
        self.slots = [cross + CROSS_CORNER + k * 0x238
                      for k in range(CURSOR_POOL)]
        self.prev = self._read(h)
        self.last = None
        for r in self.prev.values():
            if not (r[0] < 30.0 and r[1] < 30.0):
                self.last = (r[0], r[1])

    def _read(self, h):
        out = {}
        for a in self.slots:
            d = read_mem(h, a, SPRITE_BYTES)
            if d:
                r = rect_of(d, margin=64.0)
                if r:
                    out[a] = r
        return out

    def pos(self, h):
        cur = self._read(h)
        best = None
        for a, r in cur.items():
            b = self.prev.get(a)
            if b is None or (abs(r[0] - b[0]) < 0.5 and abs(r[1] - b[1]) < 0.5):
                continue
            d = abs(r[0] - b[0]) + abs(r[1] - b[1])
            if best is None or d > best[0]:
                best = (d, (r[0], r[1]))
        self.prev = cur
        if best is not None:
            self.last = best[1]
        return self.last


def find_pointer(h, inp, cross, units: int = 20):
    """Sanity-check the pool: nudge and report what the tracker sees.

    Prefer the pool slot whose x moved in the nudge direction at ~2 px/unit.
    A slower ~0.4 slot and recycled non-cursor rectangles both move; picking
    max |delta| alone has returned scale -0.2 / 0.53 while the real cursor
    sat unused in another slot.
    """
    p = Pointer(h, cross)
    before_map = p._read(h)
    inp.glide(units, 0, steps=units, per_step_frames=1)
    inp.move(0, 0)
    after_map = p._read(h)
    best = None
    for a, r1 in after_map.items():
        r0 = before_map.get(a)
        if r0 is None:
            continue
        dx = r1[0] - r0[0]
        if dx < units * 0.8:
            continue
        sc = dx / units
        # In-level pointer is ~2.0 px/unit; reject the ~0.4 ghost.
        if 1.2 <= sc <= 3.0 and (best is None or sc > best[0]):
            best = (sc, (r0[0], r0[1]), (r1[0], r1[1]), a)
    inp.glide(-units, 0, steps=units, per_step_frames=1)
    inp.move(0, 0)
    p.prev = p._read(h)
    if best is not None:
        p.last = best[2]
        return p, best[1], best[2], best[0]
    # Fall back to the old max-motion read so a quiet screen still reports.
    before = None
    for r in before_map.values():
        if not (r[0] < 30.0 and r[1] < 30.0):
            before = (r[0], r[1])
            break
    after = p.pos(h)
    scale = None
    if before and after:
        scale = (after[0] - before[0]) / units
    return p, before, after, scale


def glide_to(h, inp, ptr, target, tries: int = 14, tol: float = 4.0):
    """Closed loop onto a screen position, the way drive.aim_at does.

    Pointer acceleration is on, so the scale is re-estimated from the previous
    step instead of assumed; frame-synced single-unit steps keep it near
    linear.
    """
    px_per_unit = 2.0          # measured: the in-level pointer moves 2 px/unit
    prev = prev_units = None
    pos = None
    for _ in range(tries):
        pos = ptr.pos(h)
        if pos is None:
            continue
        if prev and prev_units:
            moved = max(abs(pos[0] - prev[0]), abs(pos[1] - prev[1]))
            units = max(abs(prev_units[0]), abs(prev_units[1]))
            if moved > 6 and units > 3:
                px_per_unit = max(0.3, min(4.0, moved / units))
        ex, ey = target[0] - pos[0], target[1] - pos[1]
        if abs(ex) <= tol and abs(ey) <= tol:
            return pos
        ux = max(-300, min(300, round(ex / px_per_unit)))
        uy = max(-300, min(300, round(ey / px_per_unit)))
        if ux == 0 and uy == 0:
            return pos
        prev, prev_units = pos, (ux, uy)
        inp.glide(ux, uy, steps=max(1, abs(ux), abs(uy)), per_step_frames=1)
        inp.move(0, 0)
    return ptr.pos(h)


def click_at(h, inp, cross, target, hold=4):
    pos = glide_to(h, inp, cross, target)
    inp.button(0, True)
    inp.wait_frames(hold)
    inp.button(0, False)
    inp.wait_frames(2)
    return pos


def riser(h, base, lo: float = RING + 1.0, hi: float = 62.0):
    """A quad that has left the walking ring upwards since the baseline."""
    for a, y0 in base.items():
        v = quad(h, a)
        if v is None:
            continue
        y = height(v)
        if lo < y < hi and abs(y - y0) > 0.01:
            return a, y
    return None, None


def readable_block(h, lo: int, hi: int):
    """A (lo, size) that ReadProcessMemory actually accepts.

    A block that spans a region boundary makes the call fail for the *whole*
    range rather than partially, which showed up once as "0 block reads in
    60.0 s".
    """
    size = (hi - lo) & ~7
    while size > QUAD and read_mem(h, lo, size) is None:
        size -= 0x1000
    return (lo, size) if size > QUAD else None


def _frac190(y: float) -> bool:
    """Live lemmings on this level hold a .190 height fraction; scenery often .000."""
    return abs((y % 1.0) - 0.190) < 0.02


def follow(h, base, addr: int, seconds: float, settle: float = 12.0,
           dense: int = 0xC000, floor: float = RING + 0.8, ceil: float = 70.0,
           hz: float = 40.0, jump_col: float = 6.0, jump_y: float = 24.0):
    """Follow whatever billboard is above the walking ring, not one address.

    Following the single record the climber started in does not work. The
    climb itself stays in one record, but the moment the lemming tops out the
    record stops holding it: in one run it climbed 41.190 -> 58.190, hoisted
    to 60.190, and then that address sat unchanged for 1.75 s before reading
    32.190 at a column 3.45 to the left -- a different lemming's billboard in
    a recycled record, not a fall. A 14 KB window round the climber lost the
    descent the same way, and so did a 91 KB one.

    So each sample looks at two things: every 8-byte-aligned offset in a dense
    window around the climber, and every billboard address the opening scan
    found anywhere else. On MM_CLIMB_002 only one lemming at a time is off the
    ring, so the highest hit is the one being measured.

    "Highest above the ring" is not enough on its own: the entrance hatch
    animation is a billboard at y = 78.000, column 68, so it wins every
    sample and the series comes out as the hatch twitching by 0.190 over 8.6
    seconds. The band is therefore capped below it and the pick is by
    *continuity* -- the candidate nearest the previous accepted sample in
    column and height -- which is also what makes a change of record
    invisible, since the new record holds the same lemming in the same place.

    Round 2: prefer the same address while it still holds a .190 billboard,
    reject .000 heights that latch onto scenery, and after the y≈48 ledge
    tighten the column jump so a walker 5 columns away cannot steal the lock.
    Poll at ~40 Hz so a 4.0-unit / 200 ms fall still shows intermediates
    without slowing the sim the way multi-kHz block reads did.
    """
    # Clamp the dense window to the region holding the record before trying
    # to read it: a `lo` in an unreadable region fails every call however far
    # the size is shrunk ("could not read a window around the climber").
    lo, hi = max(0x10000, (addr - dense) & ~7), addr + dense
    for rb, rs in heap_regions(h):
        if rb <= addr < rb + rs:
            lo, hi = max(rb, lo), min(rb + rs, hi)
            break
    win = readable_block(h, lo, hi)
    if win is None:
        print("  could not read a window around the climber", flush=True)
        return []
    far = []
    addrs = sorted(a for a in base if not (win[0] <= a < win[0] + win[1]))
    if addrs:
        cur = [addrs[0], addrs[0] + QUAD]
        for a in addrs[1:]:
            if a - cur[1] < 0x8000:
                cur[1] = a + QUAD
            else:
                far.append((cur[0], cur[1], [x - cur[0] for x in addrs
                                             if cur[0] <= x < cur[1]]))
                cur = [a, a + QUAD]
        far.append((cur[0], cur[1], [x - cur[0] for x in addrs
                                     if cur[0] <= x < cur[1]]))
    blocks = []
    for lo, hi, offs in far:
        blk = readable_block(h, lo, hi)
        if blk:
            offs = [o for o in offs if o + QUAD <= blk[1]]
            if offs:
                blocks.append((blk[0], blk[1], offs))
    dense_offs = list(range(0, win[1] - QUAD, 8))
    total = win[1] + sum(b[1] for b in blocks)
    print(f"  window {win[0]:#x} + {win[1] // 1024} KB dense,"
          f" {len(blocks)} further blocks, {total // 1024} KB a sample",
          flush=True)

    # Seed the tracker from the climber we already identified. Picking the
    # highest candidate instead latched onto scenery that is permanently
    # above the ring -- the entrance hatch billboard at y = 78.000, column 68,
    # and another fixed one at y = 63.000, column 76 -- and reported their
    # twitching as the fall.
    v0 = quad(h, addr)
    last = None
    last_addr = addr
    if v0 is not None:
        last = (column((v0[0] + v0[3] + v0[6] + v0[9]) / 4.0,
                       (v0[1] + v0[4] + v0[7] + v0[10]) / 4.0), v0[2])
        print(f"  seeded at column {last[0]:.2f}, y {last[1]:.3f}", flush=True)
    series = []
    seen_above = False
    gone_at = None
    # climb -> ledge (y~48) -> fall. Without the ledge lock, a second
    # climber at 43 steals the 48 track as a fake 5-unit drop.
    phase = "climb"
    reads = 0
    t0 = time.perf_counter()
    period = 1.0 / hz
    next_at = 0.0
    while True:
        now = time.perf_counter() - t0
        if now > seconds:
            break
        if now < next_at:
            continue
        next_at = now + period
        recovering = bool(gone_at is not None)
        if phase == "climb":
            col_lim, y_lim = jump_col, jump_y
        elif phase == "ledge":
            col_lim = 3.5 if recovering else 2.5
            y_lim = 0.15  # walk holds y; fall starts below
        else:  # fall
            col_lim = 3.5 if recovering else 2.5
            y_lim = 5.5
        best = None
        for lo, size, offs in [(win[0], win[1], dense_offs)] + blocks:
            d = read_mem(h, lo, size)
            if not d or len(d) != size:
                continue
            for off in offs:
                y = struct.unpack_from("<d", d, off + 16)[0]
                if y <= floor or y >= ceil:
                    continue
                if not _frac190(y):
                    continue
                v = struct.unpack_from("<12d", d, off)
                if v[5] != y or v[8] != y or v[11] != y:
                    continue
                col = column((v[0] + v[3] + v[6] + v[9]) / 4.0,
                             (v[1] + v[4] + v[7] + v[10]) / 4.0)
                cand = lo + off
                if last is None:
                    score = -y
                else:
                    dc = abs(wrap(col - last[0]))
                    dy = abs(y - last[1])
                    if phase == "ledge":
                        # Stay on the ring, or take the first ~4-unit fall step.
                        on_ring = abs(y - last[1]) <= 0.15
                        first_fall = (3.5 <= (last[1] - y) <= 4.5
                                      and dc <= col_lim)
                        if not (on_ring or first_fall):
                            continue
                        if not on_ring and not first_fall:
                            continue
                        if dc > col_lim and on_ring:
                            continue
                        same = 0.0 if cand == last_addr else 0.75
                        score = dc + (0.0 if on_ring else 0.1) + same
                    elif phase == "fall":
                        if y > last[1] + 0.05 or dc > col_lim or dy > y_lim:
                            continue
                        drop = last[1] - y
                        # Prefer a single 4-unit tick over a stranger 5+ away.
                        if drop > 0.2 and not (3.5 <= drop <= 4.5):
                            if drop > 4.5:
                                continue
                        same = 0.0 if cand == last_addr else 0.75
                        score = dc + abs(drop - 4.0) + same
                    else:
                        if dc > col_lim or dy > y_lim:
                            continue
                        same = 0.0 if cand == last_addr else 0.75
                        score = dc + dy + same
                if best is None or score < best[0]:
                    best = (score, cand, v, col, y)
        reads += 1
        # Full heap rescan is ~12 s; only try every 0.5 s while recovering.
        if (best is None and phase in ("ledge", "fall") and last is not None
                and gone_at is not None and now - getattr(follow, "_resc", -9) > 0.5):
            follow._resc = now
            # Billboard records recycle on top-out / fall; the opening scan's
            # address list goes stale. Rescan for a .190 quad near last.
            for rb, rs in heap_regions(h):
                if rs > 4 * 1024 * 1024:
                    continue
                d = read_mem(h, rb, min(rs, 1 * 1024 * 1024))
                if not d:
                    continue
                n = len(d) // 8
                for i in range(0, n - 11, 3):
                    y = struct.unpack_from("<d", d, i * 8 + 16)[0]
                    if not (floor < y < ceil) or not _frac190(y):
                        continue
                    v = struct.unpack_from("<12d", d, i * 8)
                    if v[5] != y or v[8] != y or v[11] != y:
                        continue
                    col = column((v[0] + v[3] + v[6] + v[9]) / 4.0,
                                 (v[1] + v[4] + v[7] + v[10]) / 4.0)
                    dc = abs(wrap(col - last[0]))
                    drop = last[1] - y
                    if phase == "ledge":
                        if dc <= 10.0 and (abs(y - last[1]) <= 0.15
                                           or 3.5 <= drop <= 4.5):
                            best = (dc, rb + i * 8, v, col, y)
                            break
                    else:
                        if dc <= 10.0 and -0.05 <= drop <= 5.5:
                            best = (dc + abs(drop - 4.0), rb + i * 8, v, col, y)
                            break
                if best is not None:
                    print(f"  reacquired {best[1]:#010x} y={best[4]:.3f}"
                          f" col={best[3]:.2f} via rescan", flush=True)
                    break
        if best is None:
            if seen_above:
                gone_at = gone_at or now
                if now - gone_at > settle:
                    break
            continue
        seen_above = True
        gone_at = None
        last = (best[3], best[4])
        last_addr = best[1]
        if phase == "climb" and last[1] >= 47.5:
            phase = "ledge"
            print(f"  phase ledge at t={now * 1000:.0f} ms y={last[1]:.3f}"
                  f" col={last[0]:.2f}", flush=True)
        elif phase == "ledge" and last[1] < 47.5:
            phase = "fall"
            period = 1.0 / 80.0  # denser while falling so 4.0 steps show up
            print(f"  phase fall at t={now * 1000:.0f} ms y={last[1]:.3f}"
                  f" col={last[0]:.2f}", flush=True)
        if not series or series[-1][2] != best[2]:
            series.append((now, best[1], best[2]))
        # Keep following through the fall for a bit after the last sample
        # rather than settling out as soon as one reacquire misses.
        if phase == "fall" and last[1] <= floor + 0.5:
            print(f"  landed-ish y={last[1]:.3f}; stopping", flush=True)
            break
    el = max(1e-6, min(seconds, time.perf_counter() - t0))
    print(f"  {reads} samples in {el:.1f} s = {reads / el:.0f} Hz  phase={phase}",
          flush=True)
    return series


def report(series) -> int:
    print(f"\n{len(series)} distinct quad values", flush=True)
    if not series:
        return 2
    hs = []
    for t, addr, v in series:
        y = height(v)
        col = column((v[0] + v[3] + v[6] + v[9]) / 4.0,
                     (v[1] + v[4] + v[7] + v[10]) / 4.0)
        if not hs or hs[-1][1] != y:
            hs.append((t, y, col, addr))
    print("height changes (t ms, y, column, record):")
    for t, y, c, a in hs:
        print(f"  {t * 1000:9.1f}  y {y:8.3f}  col {c:7.2f}  {a:#010x}")
    steps = [(hs[i + 1][0] - hs[i][0], hs[i + 1][1] - hs[i][1],
              hs[i][1], hs[i + 1][1])
             for i in range(len(hs) - 1)]
    downs = [(g, d, y0, y1) for g, d, y0, y1 in steps if d < -0.001]
    # Climb steps are +1; a walk-off fall is a run of downward steps below
    # the upper ledge. Hatch / floor at ~32.190 is not this measurement.
    print(f"\n{len(downs)} downward height changes (world units; do not x{PIXELS_PER_UNIT})")
    if not downs:
        return 2
    print("  drop (world units): " + " ".join(f"{-d:.3f}" for _g, d, *_ in downs))
    print("  gap (ms):           " + " ".join(f"{g * 1000:.0f}" for g, *_ in downs))
    for g, d, y0, y1 in downs:
        ticks = g / 0.200
        per = (-d / ticks) if ticks > 0.25 else float("nan")
        print(f"    {y0:.3f} -> {y1:.3f}  gap {g * 1000:.0f} ms"
              f"  (~{ticks:.2f} ticks)  dY={-d:.3f}"
              f"  => {-d:.3f}/gap or ~{per:.3f}/tick if one step")
    # Fall-sized: ~4 world units. Gaps may stretch well past 200 ms when a
    # recycled billboard is reacquired late; a clean 4.000 drop still counts.
    fallish = [(g, d) for g, d, y0, y1 in downs
               if 3.5 <= -d <= 4.5 and y0 >= 33.0]
    near = [(g, d) for g, d, y0, y1 in downs
            if 0.8 <= -d <= 5.5 and y0 >= 33.0 and 0.12 <= g <= 0.45]
    use = fallish or near
    if use:
        md = statistics.median([-d for _g, d in use])
        print(f"  fall-sized steps: n={len(use)}"
              f"  median dY={md:.4f} world units/step")
        spread = max(abs(-d - md) for _g, d in use)
        print(f"  spread {spread:.4f} ->"
              f" {'repeated constant' if spread < 0.05 else 'NOT a repeated constant'}")
        if abs(md - 4.0) < 0.15:
            print("  agrees with predicted fallSpeed 4.0 world units/tick")
        else:
            print(f"  disagrees with predicted 4.0 (median {md:.4f})")
    else:
        print("  no fall-sized steps in this series"
              " (snap or non-fall descent)")
    return 0 if use else 2


def assign_climber(h, inp, ptr, base, slots, watch=15.0, clicks=10,
                   index=CLIMBER_INDEX, rounds=6):
    """Select the climber, click lemmings, and wait for one to leave the ring.

    Index 0 -- the top button of the strip -- is the climber. Index 1 is the
    parasol, which MM_CLIMB_002 has none of, so clicking lemmings with it
    selected does nothing at all and reports nothing: six rounds of ten
    clicks each assigned exactly nothing. The two runs that did produce a
    climber were both the round *after* one that had clicked the top button,
    which is the same evidence read the hard way.

    Everything here happens with the level *running*. Pausing (P) would stop
    the clock, which is tempting on a 300-second level, but while paused the
    pointer's sprite record stops being a readable rectangle, so `glide_to`
    has no feedback and every click of a paused round lands wherever the
    cursor was left. Two whole runs were lost to that.
    """
    for rnd in range(rounds):
        if (i32(h, CLOCK) or 0) < 20:
            print("  out of clock", flush=True)
            return None
        pos = click_at(h, inp, ptr, (PANEL_X, PANEL_Y0 + PANEL_DY * index))
        print(f"round {rnd}: panel click landed {pos},"
              f" 0x004C9200 = {i32(h, SELECTED)}", flush=True)
        fast = lemming_slots(h, slots)
        last = None
        for _ in range(clicks):
            last, landed = click_lemming(h, inp, ptr, fast, avoid=last)
            if last is None:
                fast = lemming_slots(h, slots)
                continue
            print(f"    aimed ({last[0]:.0f},{last[1]:.0f}) landed {landed}",
                  flush=True)
            a, y = riser(h, base)
            if a:
                print(f"  -> {a:#010x} at y {y:.3f} (during the clicks)",
                      flush=True)
                return a
        t0 = time.perf_counter()
        while time.perf_counter() - t0 < watch:
            a, y = riser(h, base)
            if a:
                print(f"  -> {a:#010x} at y {y:.3f}", flush=True)
                return a
        print(f"  nothing rose (clock {i32(h, CLOCK)})", flush=True)
    return None


def main() -> int:
    seconds = float(sys.argv[1]) if len(sys.argv) > 1 else 90.0
    h, _pid = attach(write=True)
    if not h:
        print("the game is not running")
        return 1
    from lrinput import LRInput
    from restart_level import ensure_playing

    with LRInput() as inp:
        inp.clear()
        inp.enable()
        if ui_state(h) != 0:
            if not ensure_playing(h, inp):
                print("could not enter a level")
                inp.release()
                k32.CloseHandle(h)
                return 1
        cross = u32(h, CROSS_PTR)
        # The released counter does not reset on restart, so wait on the
        # clock: the level starts at 300 and releases one lemming every
        # 2.889 s, and the sweep wants all ten of them on the ledge.
        while (i32(h, CLOCK) or 0) > 268:
            time.sleep(0.5)
        print(f"clock {i32(h, CLOCK)} cross {cross:#010x}", flush=True)
        t0 = time.perf_counter()
        slots, base = scan(h)
        print(f"{len(slots)} sprite records and {len(base)} billboard quads"
              f" in {time.perf_counter() - t0:.1f} s"
              f" (clock {i32(h, CLOCK)})", flush=True)
        ptr, before, after, scale = find_pointer(h, inp, cross)
        print(f"pointer {before} -> {after} = {scale} px per mouse unit",
              flush=True)
        if scale is None or scale < 0.5:
            print("the mouse moves no cursor; input is not reaching the level")
            inp.release()
            k32.CloseHandle(h)
            return 2
        a = assign_climber(h, inp, ptr, base, slots)
        inp.release()

    if a is None:
        print("no billboard left the ring")
        k32.CloseHandle(h)
        return 2
    print(f"following {a:#010x} for up to {seconds:.0f} s", flush=True)
    rc = report(follow(h, base, a, seconds))
    k32.CloseHandle(h)
    return rc


if __name__ == "__main__":
    raise SystemExit(main())
