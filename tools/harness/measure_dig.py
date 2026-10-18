"""What bash, dig and mine remove from the grid, and when -- live.

Phase 3 round 5, the live half. `docs/scratch/phase3-round5.md` is the static
read this builds on and does not repeat: `bash`, `dig` and `mine` share slot-1
stepper `0x0041B077`, that stepper clears no cell itself, and the removal in
every bite is a store of dword 0 through the pointer the collision-grid probe
`0x0040DE60` returns. Phase 2 proved the engine collides against the stored
256-wide `u32` grid and that the live grid is byte-identical to the `.LVL`
bytes, so the removal is **directly observable**: snapshot the grid, click,
and diff it per sample.

What this reads, with the instruction for each
----------------------------------------------
The grid, from `read_grid.py`: row count `0x004C6E28`, data pointer
`0x004C6E38`, row-major `u32`, 256 wide (`and eax, 0xff` at `0x0040DE6A`).

The skill state, from `[lemming + 0x340]` (`0x0042E6FE` writes it):

    state + 0x00    the state vtable: dig 0x004B09F0, bash 0x004B06AC,
                    mine 0x004B0B68 (read_steppers.py --states)
    state + 0x18    the record list. `[+0x00]` is its count (0x0041B440,
                    called through 0x0041B3A0 at 0x0041B0A4) and `[+0x0C]`
                    its array of record pointers (0x0041B42A/0x0041B430).
    state + 0x28    the list cursor. Incremented at 0x0041B254 only when a
                    record fires, wrapped to 0 at 0x0041B26B.
    state + 0x2C    THE ANIMATION PERCENT the stepper last saw, initialised
                    to -1 at 0x0041AFC3 and stored at 0x0041B290/0x0041B29B,
                    taken mod the dword 100 at 0x004BF4C8 (0x0041B2B2).

    record + 0x08   the frame (= integer percent) this record fires at.
                    Compared for EQUALITY with the cursor at 0x0041B21E.
    record + 0x0C   the function. Called as (state, lemming, record+0x10)
                    at 0x0041B248; a non-zero return stops the tick
                    (0x0041B1F7).
    record + 0x10   its argument.

So `state + 0x2C` is the engine's own clock for a skill, and the record frames
are the engine's own "when". Both are read here rather than converted into a
tick interval we invented.

The dig bite, `0x00415A43`, which is the geometry this script predicts
--------------------------------------------------------------------
    00415a86  c7 45 f0 10 00 00 00   mov  [ebp-0x10], 0x10     ; refusal mask
    00415a90  83 79 44 ff            cmp  [ecx+0x44], -1       ; vertical dir
    00415a96  c7 45 f0 20 00 00 00   mov  [ebp-0x10], 0x20     ;   if inverted
    00415aa0  db 42 44               fild [edx+0x44]
    00415aa3  dc 6d 84               fsubr [ebp-0x7c]          ; Y = posY - vdir
    00415ab7  c7 45 d4 ff ff ff ff   mov  [ebp-0x2c], -1       ; i = -1
    00415ac9  83 7d d4 02            cmp  [ebp-0x2c], 2        ; while i < 2
    00415ad9  dc 41 08               fadd [ecx+8]              ; X = i + lemX
    00415b08  e8 53 83 ff ff         call 0x40de60             ; the cell
    00415b1b  83 e0 02               and  eax, 2               ; indestructible?
    00415b2a  23 55 f0               and  edx, [ebp-0x10]      ; the mask
    00415b2f  74 4d                  je   0x415b7e             ;   clear -> write
    00415b84  c7 02 00 00 00 00      mov  dword ptr [edx], 0   ; THE REMOVAL

`posY` is `0x0042F183`'s Y: `trunc([lem+0x10]) + [lem+0x368]`. Its X is
`trunc([lem+8]) + [lem+0x360]` and the bite does **not** use it -- the column
loop adds `i` to the raw double `[lem+8]` (`0x00415AD9`) and truncates after
wrapping. Both wrap/clamp helpers are the engine's: `0x0040DEC0` folds X by
the 256.0 at `0x004B07A8`, `0x0040DF50` clamps Y to [0, height-1].

Frame 30's record `0x00415DD2` writes `clampY([lem+0x10] - [lem+0x44])`
through `0x00423275` (`0x00415E28` is the `fsubr`), then calls `0x0042E913`
and, when that returns 0, switches the lemming to the state built by
`0x00415FA0` -- whose vtable store is `mov [eax], 0x004B0A20` at `0x00415FB2`,
the **faller**. So "the digger fell through" is a state change to the fall,
read rather than assumed.

The refusal bits, and why they matter before any click
------------------------------------------------------
`CollisionGrid.h` lists `0x4`, `0x8` and `0x20` as cell bits with no
instruction anywhere. They have one: they are the bites' refusal masks, and
they are DIRECTIONAL.

    dig   0x00415A86 / 0x00415A96   mask 0x10, or 0x20 when [lem+0x44] == -1
    bash  0x0040C0DC / 0x0040C0EC   mask 0x4,  or 0x8  when [lem+0x40] == -1
    mine  0x004189B0 / 0x004189C0   mask 0x4,  or 0x8  when [lem+0x40] == -1

plus `0x2` (indestructible) unconditionally in all three. A set bit means the
bite refuses the cell, returns 1, and the skill gives up.

That is why `--predict` exists and why it runs offline: on MM_DIG_000 every
solid cell is either `0x3` (solid|indestructible, 7732 of them) or `0x21`
(solid|0x20, 876), and `0x21` is diggable upright because the upright mask is
`0x10`. A digger put anywhere else on that level must give up without
touching the grid -- which is the "digger gives up after one tick" the assign
round saw and did not explain.

The animation clock
-------------------
`0x0041B0CB` adds 0x128 and calls the duration / current pair, which read the
animation controller:

    00468b27  call 0x45b8f0            ; the frame count, = [strip + 0]
    00468b32  fmul [0x4b1940]          ; 0.06 seconds per animation frame
    00468b56  fdiv [eax+0x10]          ; the strip's own divisor
    00468b69  fild [eax+0x1cc]         ; the CURRENT frame index
    00468b78  fadd [ecx+0x1c0]         ; plus the time inside that frame

`0x00468D00` resolves the strip as `[controller+0x1F8 + 0x0C] +
[controller+0x1C8] * 0x98`. The divisor cancels in the percent, so one whole
cycle is `frames * 0.06` seconds. `anim_block` reads all of it.

What it measured, 2026-10-15
----------------------------
MEASURED live, and what `Rules` now carries:

    dig   3 cells, trunc(X)-1..+1, in the row BELOW the feet, then Y -1.
          Bite at percent 25, Y step at 30. Strip 4, 20 frames, cycle
          1.2005 s = 6.00 ticks. Four diggers on MM_DIG_000, 12 cells each.
    bash  the leading column only, trunc(X)+facing, FOUR rows from the feet
          row upward, at percents 12, 16, 60, 64 in row order +0,+1,+3,+2.
          One column a cycle. Strip 0, 24 frames, 1.440 s = 7.20 ticks.
          Measured both facings on MM_BASH_002.
    mine  columns +1 and +2 ahead, rows feet-1..feet+3, minus the cell the
          engine skips (arg 2 with dx 1). Then X +1, Y -1. Percents 12, 12,
          16, 16, 20. Strip 9, 25 frames, 1.500 s = 7.50 ticks. MM_MINE.

So an interval in ticks does not exist: only dig divides into the measured
200 ms tick. Every cell cleared went to a whole `0x00000000` -- 38 of 38,
from `0x21`, `0x5` and `0x1` -- which excludes the `and 0xf77c` reading by
observation. docs/scratch/phase3-round5-live.md has the series.

Usage
-----
Offline, no process. `--predict` applies the refusal masks to the shipped
`.LVL` grid, which is what makes the live run a test rather than a hunt:

    python tools/harness/measure_dig.py --predict MM_DIG_000  --row 58
    python tools/harness/measure_dig.py --predict MM_BASH_002 --row 24

Live. The process must already be in a level, or pass `--enter`. Restart the
process between levels; there is no route from inside a level back to the map:

    python tools/harness/measure_dig.py --enter MM_DIG_000 --probe
    python tools/harness/measure_dig.py --skill 7 --rows 0 --repeat 2 --seconds 4
    python tools/harness/measure_dig.py --skill 7 --refuse --seconds 2
    python tools/harness/measure_dig.py --enter MM_BASH_002 --skill 5 --any --rows 0
    python tools/harness/measure_dig.py --enter MM_MINE --skill 6 --any --rows 0

`--skill` with no value picks the level's own stocked skill. `--rows 0`
watches the whole grid (about 650 Hz) and proves nothing moved outside the
window; a row window runs at about 4,500 Hz. `--any` drops the dig prediction,
which bash and mine need since their geometry is recorded here, not predicted.
`--refuse` aims at a lemming the read says the bite will refuse.

Nothing here writes to the game except `--enter`'s level-record swap, which is
put back. No lemming byte and no grid cell is ever written: the skill goes on
through the panel, by `assign_skill.py`.

Two traps that cost runs
------------------------
  * `[state+0x2C]` can read above 100. `0x0041B290` stores the raw percent and
    `0x0041B2B2` reduces it mod the dword 100 at `0x004BF4C8` a few
    instructions later, so a kHz sample can land between them and see 144 or
    1488. Not corruption.
  * Two lemmings in a skill at once contaminate a whole-grid diff, and the
    `d(col)` / `d(row)` offsets printed are then relative to the wrong
    lemming. One miner at a time.
"""
from __future__ import annotations

import argparse
import math
import os
import struct
import sys
import time
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
from lrstate import attach, i32, u32, ui_state  # noqa: E402
from memdump import k32, read_mem  # noqa: E402

import assign_skill as A  # noqa: E402
from measure_stepup import GRID_DATA_PTR, ROW_COUNT, grid_of  # noqa: E402

GRID_WIDTH = 256
CORPUS = "build/lvl_dump/levels/COMPILED"

OFF_X, OFF_Y = 0x08, 0x10
OFF_DIR, OFF_VDIR = 0x40, 0x44       # horizontal facing, vertical direction
OFF_XBIAS, OFF_YBIAS = 0x360, 0x368  # 0x0042F183's two biases
OFF_STATE = 0x340

ST_LIST = 0x18
ST_CURSOR = 0x28
ST_PERCENT = 0x2C

TEXT_LO, TEXT_HI = 0x00401000, 0x004B0000

# The record functions named in docs/scratch/phase3-round5.md, so a live
# record list prints as the thing the constructor registered.
FN_NAMES = {
    0x0040C357: "bash-bite",
    0x0040C6E8: "bash-commit",
    0x0040C7F6: "bash-nudge-x",
    0x00415A43: "dig-bite",
    0x00415DD2: "dig-step-y",
    0x004188F6: "mine-bite",
    0x0041905F: "mine-nudge-xy",
    0x004190F3: "mine-commit",
}

SKILL_VT = {0x004B06AC: "bash", 0x004B09F0: "dig", 0x004B0B68: "mine"}


# --- the refusal masks, read off the three bites ---------------------------


def dig_mask(vdir: int) -> int:
    """0x00415A86 stores 0x10; 0x00415A96 stores 0x20 when [+0x44] == -1."""
    return 0x20 if vdir == -1 else 0x10


def horiz_mask(dir_: int) -> int:
    """bash 0x0040C0DC/0x0040C0EC and mine 0x004189B0/0x004189C0."""
    return 0x08 if dir_ == -1 else 0x04


def refused(cell: int, mask: int) -> bool:
    """`and eax, 2` then `and edx, mask`: either one refuses the cell."""
    return bool(cell & 0x02) or bool(cell & mask)


def trunc(v: float) -> int:
    """0x0049C5AC chops toward zero (PHASE3_HANDOFF, round 3)."""
    return int(math.trunc(v))


def bash_front(x: float, y: float, dir_: int, xbias: float) -> int:
    """The leading column the bash bite probes, from `0x0040C357`.

        0040c39d  fild [ecx+0x40]          ; the facing
        0040c3a0  fadd [ebp-0x38]          ; + 0x0042F22E's x = X + [+0x360]
        0040c3ac  call 0x40dec0            ; wrap into [0, 256)
        0040c3b1  call 0x49c5ac            ; truncate
        0040c3d6  cmp [ecx+0x40], -1
        0040c3da  sete dl
        0040c3e0  add eax, edx             ; +1 when facing left

    `[+0x360]` is -1.0 facing left and 0.0 facing right (0x0042F66B), so the
    two corrections cancel and the column is `trunc(X) + dir`.
    """
    return (trunc((x + xbias + dir_) % 256.0)
            + (1 if dir_ == -1 else 0)) & 0xFF


def dig_target(x: float, y: float, vdir: int, ybias: float = 0.0):
    """(row, [col-1, col, col+1]) the dig bite probes, in live-grid cells.

    Y-up, as the file and the live grid are: row `trunc(y)` is the row the
    world Y falls in and the floor under standing feet is `row - 1`, which is
    what `posY - vdir` comes to for an upright lemming.
    """
    row = trunc(trunc(y) + ybias - vdir)
    return row, [trunc(i + x) & 0xFF for i in (-1, 0, 1)]


# --- offline ---------------------------------------------------------------


def predict(level: str, row: int | None) -> int:
    path = os.path.join(CORPUS, level.upper() + ".LVL")
    g = grid_of(path)
    if not g:
        print(f"no grid in {path}")
        return 1
    cells, height = g
    print(f"{level}  {GRID_WIDTH} x {height}   (offline, {path})")
    hist: dict[int, int] = {}
    for c in cells:
        hist[c] = hist.get(c, 0) + 1
    print("\ncell word        count   dig up  dig inv  bash R  bash L")
    for word, n in sorted(hist.items(), key=lambda kv: -kv[1]):
        if n < 8:
            continue
        print(f"  {word:#010x} {n:>8}"
              f"   {'REFUSE' if refused(word, dig_mask(1)) else 'clear ':>6}"
              f"  {'REFUSE' if refused(word, dig_mask(-1)) else 'clear ':>7}"
              f"  {'REFUSE' if refused(word, horiz_mask(1)) else 'clear ':>6}"
              f"  {'REFUSE' if refused(word, horiz_mask(-1)) else 'clear ':>6}")
    if row is None:
        return 0

    def spans(pred):
        out, s = [], None
        for x in range(GRID_WIDTH):
            if pred(x):
                s = x if s is None else s
            elif s is not None:
                out.append((s, x - 1))
                s = None
        if s is not None:
            out.append((s, GRID_WIDTH - 1))
        return out

    r = cells[row * GRID_WIDTH:(row + 1) * GRID_WIDTH]
    print(f"\nrow {row}, as the upright dig bite sees it:")
    print(f"  diggable (written)  {spans(lambda x: r[x] and not refused(r[x], 0x10))}")
    print(f"  refused             {spans(lambda x: refused(r[x], 0x10))}")
    print(f"  empty               {spans(lambda x: r[x] == 0)}")
    return 0


# --- live ------------------------------------------------------------------


def grid_shape(h):
    return i32(h, ROW_COUNT), u32(h, GRID_DATA_PTR)


def read_rows(h, base: int, r0: int, n: int):
    return read_mem(h, base + r0 * GRID_WIDTH * 4, n * GRID_WIDTH * 4)


def full_grid(h):
    rows, base = grid_shape(h)
    if not rows or not base:
        return None, None
    return rows, read_rows(h, base, 0, rows)


def lem_core(h, lem: int):
    """x, y, [+0x40], [+0x44] and both biases, in two reads."""
    d = read_mem(h, lem + OFF_X, 0x40)
    if not d or len(d) != 0x40:
        return None
    out = dict(
        x=struct.unpack_from("<d", d, 0)[0],
        y=struct.unpack_from("<d", d, OFF_Y - OFF_X)[0],
        dir=struct.unpack_from("<i", d, OFF_DIR - OFF_X)[0],
        vdir=struct.unpack_from("<i", d, OFF_VDIR - OFF_X)[0],
        xbias=0.0, ybias=0.0,
    )
    b = read_mem(h, lem + OFF_XBIAS, 0x10)
    if b and len(b) == 0x10:
        out["xbias"] = struct.unpack_from("<d", b, 0)[0]
        out["ybias"] = struct.unpack_from("<d", b, OFF_YBIAS - OFF_XBIAS)[0]
    return out


def records(h, state: int):
    """The state's record list: [(frame, fn, arg)], in list order.

    `[state+0x18]` is the count (0x0041B440) and `[state+0x18+0x0C]` the
    array of record pointers (0x0041B42A). Each record carries its frame at
    +8, its function at +0xC and its argument at +0x10 (0x0041B21B /
    0x0041B248).
    """
    lst = state + ST_LIST
    n = i32(h, lst)
    data = u32(h, lst + 0x0C)
    if n is None or not data or not (0 <= n <= 256):
        return []
    blob = read_mem(h, data, 4 * n)
    if not blob or len(blob) != 4 * n:
        return []
    out = []
    for p in struct.unpack(f"<{n}I", blob):
        rec = read_mem(h, p, 0x14) if p else None
        if not rec or len(rec) != 0x14:
            out.append(None)
            continue
        frame, fn, arg = struct.unpack_from("<iIi", rec, 8)
        out.append((frame, fn, arg))
    return out


def print_records(h, state: int, label: str = "") -> None:
    recs = records(h, state)
    vt = u32(h, state) or 0
    print(f"  record list of {state:#010x} vt {vt:#010x}"
          f" ({SKILL_VT.get(vt, A.STATE_NAMES.get(vt, '?'))}) {label}:"
          f" {len(recs)} entries", flush=True)
    for k, r in enumerate(recs):
        if r is None:
            print(f"    [{k:2d}] unreadable")
            continue
        frame, fn, arg = r
        print(f"    [{k:2d}] frame {frame:3d}  fn {fn:#010x}"
              f" {FN_NAMES.get(fn, ''):<14} arg {arg}")


# The animation controller is the object at `lemming + 0x128` -- the same
# object whose box `assign_skill` picks with. `0x0041B0CB` adds 0x128 and calls
# the duration/current pair:
#
#   00468b27  call 0x45b8f0            ; frame count, = [strip + 0]
#   00468b32  fmul [0x4b1940]          ; 0.06 seconds per animation frame
#   00468b56  fdiv [eax+0x10]          ; the strip's own divisor
#   00468b69  fild [eax+0x1cc]         ; the CURRENT frame index
#   00468b78  fadd [ecx+0x1c0]         ; plus the time inside that frame
#
# and `0x00468D00` resolves the strip: `[controller+0x1F8 + 0x0C] +
# [controller+0x1C8] * 0x98`. The divisor cancels in the percent, so the
# percent is `100 * (frame + frac/0.06) / frames` and one whole cycle is
# `frames * 0.06 / rate` seconds, where the rate is the double at
# `controller + 0x1D8` (`0x004683E9` stores it; dig's enter hook passes 0.75
# at `0x004159CA`, bash's and mine's never call it).
ANIM = 0x128
AN_FRAC = 0x1C0        # double, time inside the current frame
AN_STRIP_IX = 0x1C8    # int, which strip
AN_FRAME = 0x1CC       # int, current frame index
AN_RATE = 0x1D8        # double, the rate dig sets to 0.75
AN_TABLE = 0x1F8       # the strip container
SEC_PER_FRAME = 0.06   # the double at 0x004B1940


def anim_block(h, lem: int):
    """frames, divisor, rate, frame index and fraction, read live."""
    c = lem + ANIM
    d = read_mem(h, c + 0x1C0, 0x40)
    if not d or len(d) != 0x40:
        return None
    frac = struct.unpack_from("<d", d, 0)[0]
    strip_ix = struct.unpack_from("<i", d, AN_STRIP_IX - 0x1C0)[0]
    frame = struct.unpack_from("<i", d, AN_FRAME - 0x1C0)[0]
    rate = struct.unpack_from("<d", d, AN_RATE - 0x1C0)[0]
    table = struct.unpack_from("<I", d, AN_TABLE - 0x1C0)[0]
    out = dict(frac=frac, strip=strip_ix, frame=frame, rate=rate,
               frames=None, div=None, cycle=None, table=table, data=None)
    base = u32(h, table + 0x0C) if table else None
    out["data"] = base
    if base:
        strip = base + strip_ix * 0x98
        sd = read_mem(h, strip, 0x18)
        if sd and len(sd) == 0x18:
            out["frames"] = struct.unpack_from("<i", sd, 0)[0]
            out["div"] = struct.unpack_from("<d", sd, 0x10)[0]
            if out["frames"] and rate:
                out["cycle"] = out["frames"] * SEC_PER_FRAME / rate
    return out


def state_block(h, lem: int):
    sp = u32(h, lem + OFF_STATE) or 0
    if not sp:
        return 0, 0, None, None
    d = read_mem(h, sp, 0x30)
    if not d or len(d) != 0x30:
        return sp, 0, None, None
    vt = struct.unpack_from("<I", d, 0)[0]
    cursor = struct.unpack_from("<i", d, ST_CURSOR)[0]
    pct = struct.unpack_from("<i", d, ST_PERCENT)[0]
    return sp, vt, cursor, pct


def sample(h, lem: int, seconds: float, r0: int, nrows: int, label: str = ""):
    """Per-sample state / position / grid-window series around one lemming.

    Four `ReadProcessMemory` calls an iteration: the state pointer, the state
    block, the lemming's own first 0x40 bytes from +8, and the grid window.
    Nothing is written. A row is printed when the state, the animation
    percent, the integer world Y, or any cell in the window changes, so a
    bite that happens inside one 200 ms tick is still caught with its own
    timestamp and percent.
    """
    rows, base = grid_shape(h)
    r0 = max(0, min(rows - 1, r0))
    nrows = max(1, min(rows - r0, nrows))
    prev = read_rows(h, base, r0, nrows)
    print(f"  watching rows {r0}..{r0 + nrows - 1} of {rows}"
          f" ({len(prev)} bytes){label}", flush=True)
    series, diffs = [], []
    t0 = time.perf_counter()
    n = 0
    last = None
    shown = False
    while (t := time.perf_counter() - t0) < seconds:
        n += 1
        sp, vt, cursor, pct = state_block(h, lem)
        core = lem_core(h, lem)
        g = read_rows(h, base, r0, nrows)
        changed = []
        if g and prev and g != prev:
            for i in range(0, len(g), 4):
                if g[i:i + 4] != prev[i:i + 4]:
                    old = struct.unpack_from("<I", prev, i)[0]
                    new = struct.unpack_from("<I", g, i)[0]
                    cell = i // 4
                    changed.append((r0 + cell // GRID_WIDTH,
                                    cell % GRID_WIDTH, old, new))
            prev = g
        if vt in SKILL_VT and not shown:
            # The record list, out of the running process: the live answer to
            # "which frames does this skill act on", against the constructor
            # read in docs/scratch/phase3-round5.md.
            print_records(h, sp, f"at t {t:.3f}s")
            a = anim_block(h, lem)
            if a:
                cyc = ("%.4f s = %.3f ticks" % (a["cycle"], a["cycle"] / 0.2)
                       if a["cycle"] else "unreadable")
                print(f"    animation strip {a['strip']} frames {a['frames']}"
                      f" divisor {a['div']} rate {a['rate']}"
                      f" table {a['table']:#010x} data {a['data']}"
                      f"  => frames * 0.06 / rate = {cyc}"
                      f"  (frame {a['frame']} frac {a['frac']:.4f})", flush=True)
            shown = True
        if core is None:
            continue
        key = (vt, cursor, pct, trunc(core["y"]), trunc(core["x"]))
        if changed or key != last:
            row = dict(t=t, state=sp, vt=vt, cursor=cursor, pct=pct,
                       x=core["x"], y=core["y"], dir=core["dir"],
                       vdir=core["vdir"], changed=changed)
            series.append(row)
            diffs.extend((t, pct) + c for c in changed)
            name = SKILL_VT.get(vt, A.STATE_NAMES.get(vt, f"{vt:#010x}"))
            cells = "".join(
                f"\n          cell (col {c[1]:3d}, row {c[0]:3d})"
                f"  {c[2]:#010x} -> {c[3]:#010x}"
                f"   d(col)={((c[1] - trunc(core['x']) + 128) % 256) - 128:+d}"
                f" d(row)={c[0] - trunc(core['y']):+d}"
                for c in changed)
            print(f"  {t:6.3f}s {name:<5} pct {pct if pct is not None else '?':>4}"
                  f" cur {cursor if cursor is not None else '?':>2}"
                  f"  x {core['x']:8.3f} y {core['y']:7.3f}"
                  f" dir {core['dir']:+d} vdir {core['vdir']:+d}{cells}",
                  flush=True)
            last = key
    print(f"  {n} samples in {seconds:.1f}s ({n / seconds:.0f} Hz),"
          f" {len(diffs)} cell changes", flush=True)
    return series, diffs


def candidates(h, want_refuse: bool, skill_kind: str):
    """Live lemmings, with the bite's own prediction for each one.

    The prediction is the static read applied to the live grid: for dig, the
    three cells `0x00415A43` would probe and whether `0x2` or the direction
    mask refuses them. Choosing by it is what makes the live series a test of
    the read rather than a fishing trip.
    """
    rows, base = grid_shape(h)
    out = []
    for p in A.lemmings(h):
        core = lem_core(h, p)
        if core is None:
            continue
        row, cols = dig_target(core["x"], core["y"], core["vdir"], core["ybias"])
        if not (0 <= row < rows):
            continue
        words = []
        for c in cols:
            w = read_mem(h, base + (row * GRID_WIDTH + c) * 4, 4)
            words.append(struct.unpack("<I", w)[0] if w and len(w) == 4 else None)
        mask = dig_mask(core["vdir"])
        will_refuse = any(w is None or refused(w, mask) for w in words)
        front = bash_front(core["x"], core["y"], core["dir"], core["xbias"])
        frow = trunc(core["y"] + core["ybias"])
        fronts = []
        for dr in range(0, 7):
            w = read_mem(h, base + (((frow + dr) % rows) * GRID_WIDTH
                                    + (front & 0xFF)) * 4, 4)
            fronts.append(struct.unpack("<I", w)[0] if w and len(w) == 4 else None)
        vt, name = A.state_of(h, p)
        box = A.lemming_box(h, p)
        on_screen = bool(box and 0 <= (box[0] + box[2]) / 2 <= 640
                         and 0 <= (box[1] + box[3]) / 2 <= 480)
        out.append(dict(ptr=p, core=core, row=row, cols=cols, words=words,
                        refuse=will_refuse, state=name, on_screen=on_screen,
                        front=front, front_row=frow, front_words=fronts,
                        wall=any(w for w in fronts)))
    return out


def print_candidates(h, cands) -> None:
    for c in cands:
        ws = " ".join("?" if w is None else f"{w:#06x}" for w in c["words"])
        print(f"  {c['ptr']:#010x} x {c['core']['x']:8.3f} y {c['core']['y']:7.3f}"
              f" dir {c['core']['dir']:+d} vdir {c['core']['vdir']:+d}"
              f" bias ({c['core']['xbias']:+.1f},{c['core']['ybias']:+.1f})"
              f"  dig row {c['row']:3d} cols {c['cols']}"
              f" words [{ws}]  {'REFUSE' if c['refuse'] else 'CLEAR '}"
              f"  {c['state']:<5} {'on-screen' if c['on_screen'] else 'off-screen'}"
              f"  front col {c['front']:3d} rows {c['front_row']}..+6 ["
              + " ".join("?" if w is None else f"{w:#05x}" for w in c["front_words"])
              + "]", flush=True)


def one_run(h, inp, skill: int, want_refuse: bool, seconds: float,
            nrows: int, any_target: bool = False, wall: bool = False,
            front_mask: int = 0, facing: int = 0) -> dict:
    """Assign `skill` to a lemming the static read says will (or will not)
    dig, then sample the grid around it.

    `any_target` drops the dig prediction and takes any walker on screen,
    which is what bash and mine need: their bites have their own geometry and
    this script does not predict it, it records it.
    """
    scr = A.screen(h)
    if not A.select_skill(h, inp, skill):
        return dict(ok=False, reason=f"panel {skill} not selected")
    def pick():
        out = []
        for c in candidates(h, want_refuse, "dig"):
            if not (c["on_screen"] and c["state"] == "walk"):
                continue
            if not any_target and c["refuse"] != want_refuse:
                continue
            if wall and not c["wall"]:
                continue
            if front_mask and not (c["front_words"][0] or 0) & front_mask:
                continue
            if facing and c["core"]["dir"] != facing:
                continue
            out.append(c)
        return out

    # A basher has to be standing at something to bash. Walkers reach a wall
    # on their own at one column per tick, so this waits for one rather than
    # assigning into open air and recording a cycle that removes nothing
    # (which is itself a result -- see the note -- but only needs doing once).
    t_wait = time.perf_counter()
    cands = pick()
    while not cands and (wall or front_mask) and time.perf_counter() - t_wait < 40.0:
        time.sleep(0.2)
        cands = pick()
    if not cands:
        return dict(ok=False, reason="no candidate of the wanted kind on screen")
    print(f"  {len(cands)} candidate(s) with refuse={want_refuse}:", flush=True)
    print_candidates(h, cands[:6])

    for c in cands[:6]:
        p = c["ptr"]
        b = A.lemming_box(h, p)
        if not b:
            continue
        A.aim_at(h, inp, (b[0] + b[2]) / 2, (b[1] + b[3]) / 2)
        b = A.lemming_box(h, p)
        if b:
            A.aim_at(h, inp, (b[0] + b[2]) / 2, (b[1] + b[3]) / 2, tol=1.0, tries=6)
        if A.hovered(h, scr) == p:
            break
    else:
        return dict(ok=False, reason="none of the candidates became hoverable")

    before = lem_core(h, p)
    rowbase, cols = dig_target(before["x"], before["y"], before["vdir"],
                               before["ybias"])
    cb = A.counts(h)
    rows_pre, pre = full_grid(h)
    print(f"  clicking {p:#010x}: predicted first bite row {rowbase}"
          f" cols {cols}, stock[{skill}] {cb[skill]}", flush=True)
    nrows_live = rows_pre if nrows <= 0 else nrows
    r0 = 0 if nrows <= 0 else rowbase - nrows + 3
    inp.button(0, True)
    inp.wait_frames(4)
    inp.button(0, False)
    series, diffs = sample(h, p, seconds, r0, nrows_live)
    window = (r0, r0 + nrows_live)
    ca = A.counts(h)
    rows_post, post = full_grid(h)
    spent = bool(cb and ca and ca[skill] == cb[skill] - 1)

    outside = []
    if pre and post and len(pre) == len(post):
        for i in range(0, len(pre), 4):
            if pre[i:i + 4] != post[i:i + 4]:
                cell = i // 4
                r, cc = cell // GRID_WIDTH, cell % GRID_WIDTH
                if not (window[0] <= r < window[1]):
                    outside.append((r, cc,
                                    struct.unpack_from("<I", pre, i)[0],
                                    struct.unpack_from("<I", post, i)[0]))
    print(f"  stock[{skill}] {cb[skill] if cb else '?'} ->"
          f" {ca[skill] if ca else '?'}  spent={spent}", flush=True)
    if outside:
        print(f"  {len(outside)} whole-grid changes OUTSIDE the window:"
              f" {outside[:12]}", flush=True)
    else:
        print("  whole-grid diff: no change outside the window", flush=True)
    time.sleep(0.6)   # the engine's double-click window, after the reads
    return dict(ok=spent, reason="", lemming=p, series=series, diffs=diffs,
                outside=outside, before=before, cols=cols, row=rowbase)


def live(h, skill: int | None, repeat: int, want_refuse: bool,
         seconds: float, nrows: int, any_target: bool = False,
         wall: bool = False, front_mask: int = 0, facing: int = 0) -> int:
    from lrinput import LRInput

    if ui_state(h) != 0:
        print(f"ui_state {ui_state(h)}, not in a level")
        return 2
    n = A.wait_for_lemmings(h, 25.0)
    print(f"{n} lemmings in the actor list; stock {A.counts(h)}"
          f"  selected {A.selected(h)}", flush=True)
    if skill is None:
        st = A.counts(h)
        pick = [k for k in range(9) if st[k] > 0]
        if len(pick) != 1:
            print(f"cannot tell which skill this level ships: {st}")
            return 2
        skill = pick[0]
        print(f"the level's only stocked skill is index {skill}"
              f" ({A.PANEL_HINT.get(skill, 'unidentified')})", flush=True)
    rows, base = grid_shape(h)
    print(f"grid {GRID_WIDTH} x {rows} at {base:#010x}", flush=True)
    print("live lemmings, with the dig bite's prediction for each:", flush=True)
    print_candidates(h, candidates(h, want_refuse, "dig"))

    results = []
    with LRInput() as inp:
        inp.clear()
        inp.enable()
        try:
            for k in range(repeat):
                print(f"\n-- run {k}: skill {skill}, want_refuse={want_refuse}",
                      flush=True)
                r = one_run(h, inp, skill, want_refuse, seconds, nrows,
                            any_target, wall, front_mask, facing)
                if not r["ok"]:
                    print(f"   -> {r['reason']}", flush=True)
                results.append(r)
        finally:
            inp.release()

    print("\n== summary", flush=True)
    for k, r in enumerate(results):
        if not r.get("series"):
            print(f"  run {k}: {r['reason']}")
            continue
        cells = r["diffs"]
        print(f"  run {k}: lemming {r['lemming']:#010x}"
              f" start ({r['before']['x']:.3f},{r['before']['y']:.3f})"
              f" {len(cells)} cells cleared, stock spent={r['ok']}")
        for t, pct, row, col, old, new in cells:
            print(f"      t {t:6.3f}s pct {pct:>4}"
                  f"  (col {col:3d}, row {row:3d})  {old:#010x} -> {new:#010x}")
    return 0


def main() -> int:
    ap = argparse.ArgumentParser(description=__doc__.splitlines()[0],
                                 formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--predict", metavar="LEVEL",
                    help="offline: the cell words and what each bite does with them")
    ap.add_argument("--row", type=int, help="with --predict, spans of one row")
    ap.add_argument("--enter", metavar="LEVEL", help="enter this level first")
    ap.add_argument("--probe", action="store_true",
                    help="read and print, send no input")
    ap.add_argument("--skill", type=int, nargs="?", const=-1, default=None,
                    metavar="INDEX",
                    help="panel index to assign; bare --skill picks the"
                         " level's only stocked one")
    ap.add_argument("--repeat", type=int, default=1)
    ap.add_argument("--refuse", action="store_true",
                    help="aim at a lemming the read says the bite will refuse")
    ap.add_argument("--seconds", type=float, default=3.0,
                    help="how long to sample after the click")
    ap.add_argument("--rows", type=int, default=14,
                    help="how many grid rows to watch; 0 watches the whole"
                         " grid, which bash and mine want since this script"
                         " does not predict their geometry")
    ap.add_argument("--front-mask", default="0",
                    help="only take a lemming whose leading cell at its own"
                         " feet row has these bits set, e.g. 4 for a cell the"
                         " bash bite refuses facing right")
    ap.add_argument("--facing", type=int, default=0, choices=(-1, 0, 1),
                    help="only take a lemming facing this way")
    ap.add_argument("--wall", action="store_true",
                    help="only take a lemming whose leading column (the one"
                         " the bash bite probes) has something in it")
    ap.add_argument("--any", action="store_true", dest="any_target",
                    help="take any walker on screen rather than one the dig"
                         " prediction picks")
    args = ap.parse_args()

    if args.predict:
        return predict(args.predict, args.row)

    h, pid = attach(write=bool(args.enter))
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
            rows, base = grid_shape(h)
            print(f"ui_state {ui_state(h)}  grid {GRID_WIDTH} x {rows}"
                  f" at {base and hex(base)}  stock {A.counts(h)}"
                  f"  selected {A.selected(h)}")
            A.wait_for_lemmings(h, 25.0)
            print_candidates(h, candidates(h, False, "dig"))
            for p in A.lemmings(h):
                sp, vt, cur, pct = state_block(h, p)
                if vt in SKILL_VT:
                    print_records(h, sp, f"(lemming {p:#010x})")
            return 0
        skill = None if args.skill == -1 else args.skill
        return live(h, skill, args.repeat, args.refuse, args.seconds,
                    args.rows, args.any_target, args.wall,
                    int(args.front_mask, 16), args.facing)
    finally:
        k32.CloseHandle(h)


if __name__ == "__main__":
    raise SystemExit(main())
