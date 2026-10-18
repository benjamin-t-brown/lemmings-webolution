"""What a blocker's cell bit `0x800` is, and what it does to a walker -- live.

Phase 3 round 8. The static read this tests, every address checked against the
stock exe (SHA-256 32675bdb...a2f, image base 0x400000):

The blocker is the state at vtable `0x004B07B8` (`read_steppers.py --states`):
step `0x0040E003`, **enter hook `0x0040E20C`**, **exit hook `0x0040E0DF`**,
animation "block" (the string pointer `0x004BF148` pushed into `0x0042FD25` at
`0x0040E22A`). Those three addresses have NO `e8` caller anywhere in `.text`;
they are reached only through the vtable slots at file offsets 0xB07BC / C0 /
C4, i.e. `0x004B07B8 + 4/8/0xC`. So the only code that can stamp the field is
entering the block state, and the only code that can erase it is leaving.

The stamp -- the enter hook, `0x0040E20C`:

    0040e22a  push 0x4bf148 ; call 0x42fd25   ; animation "block"
    0040e237  push 0x3fd00000 ; push 0        ; the rate 0.25
    0040e247  call 0x4683e9                   ;   onto lemming+0x128
    0040e253  call 0x42f22e                   ; pos = (X + [+0x360],
                                              ;        Y + [+0x368], Z)
    0040e26f  fld  [ebp-0x48] ; call 0x49c5ac ; trunc that X
    0040e288  call 0x40dec0   ; call 0x49c5ac ; wrap into [0,256), trunc
    0040e29a  cmp [edx+0x40], -1 ; sete al
    0040e2a4  add ecx, eax                    ;   +1 when facing LEFT   -> col
    0040e2a9  fld  [ebp-0x40] ; call 0x49c5ac ; trunc the Y            -> base
    0040e2c9  cmp eax, [0x4b10b8]             ; the dword 3: j = 0..3
    0040e2d7  imul edx, [ecx+0x44]            ; row = base + j * vdir
    0040e2f1  call 0x40df50                   ;   clamped to the grid
    0040e305  call 0x40de60                   ; the cell (col, row)
    0040e312  or   ah, 8                      ; SET 0x800  (AH is bits 8..15)
    0040e318  mov  [ecx], eax                 ;   written back

so FOUR cells, in ONE column, rows `trunc(Y) .. trunc(Y)+3` upward for an
upright lemming. `[+0x360]` is 0.0 facing right and -1.0 facing left
(`0x0042F66B`), and the `+1` cancels it, so the column is `trunc(X)` both
ways.

The erase -- the exit hook, `0x0040E0DF` -- is the same loop over the same
four cells with

    0040e1d5  25 ff f7 00 00    and eax, 0xf7ff      ; CLEAR 0x800, only 0x800

reading the position through `0x0042F183` (trunc-then-bias) rather than
`0x0042F22E` (bias-then-trunc); for a non-negative X and an integer bias the
two agree cell for cell. `0x0042E696` calls the OUTGOING state's slot-3 exit
hook at `0x0042E6F5` before storing the new state, so every departure from the
block state erases the field -- a bomb, a nuke, the step's own
`0x0042E913`-returned-0 fall, or another skill.

What reads it. A whole-`.text` scan for every instruction whose immediate is
0x800, 0xf7ff, or an `ah`/`ch`/`dh`/`bh` mask that lands on bits 8..15 finds
exactly these, outside the CRT:

    set    0x0040E312  or  ah, 8        fn 0x0040E20C  block enter
    clear  0x0040E1D5  and eax, 0xf7ff  fn 0x0040E0DF  block exit
    test   0x0040C110  and eax, 0x800   fn 0x0040C0AF  the BASH bite's cell
                       test -- the third of three, after 0x2 and the
                       directional 0x4/0x8; a set bit refuses the removal
    test   0x00418AF9  and edx, 0x800   fn 0x004188F6  the MINE bite, which
                       does not merely refuse: 0x00418B06 calls 0x0042F66B,
                       the TURN, and leaves the bite
    test   0x0041C430  and eax, 0x800   fn 0x0041BFFC  the walk probe, feet row
    test   0x0041C4C9  and edx, 0x800   fn 0x0041BFFC  the walk probe, feet+1
    test   0x0041C563  and ecx, 0x800   fn 0x0041BFFC  the walk probe, feet+2
    test   0x0043F63E  and eax, 0x800   fn 0x0043F510  called from 0x004082D7
                       and 0x0041DF89; not read this round
    (the dig bite, 0x00415A43, has NO 0x800 test)

and `0x0041BFFC` has exactly ONE caller, `0x0041BEE6` inside the walk stepper
`0x0041BDD4`, so the walker's turn is walk-only.

The walk probe, `0x0041BFFC`, in the order it tests:

    0041c069  call 0x42f22e              ; the biased position
    0041c099..0041c0c2                   ; col = trunc(wrap(trunc(X+[+0x360])))
    0041c0c5..0041c104                   ; cell A = (col, trunc(Y)-vdir), the
                                         ;   floor -- only 0x8000 is read off it
    0041c128..0041c147                   ; cell B = (col, trunc(Y)), the FEET
    0041c430  and eax, 0x800             ; B carries the field?
    0041c43c  cmp [ecx+0x4c], 3         ;   ... and I am not type 3
    0041c445  call 0x42f66b              ;   -> TURN, and return 0
    0041c47d  and edx, 1                 ; B solid? no -> the step-down path
    0041c48e  add ecx, [eax+0x44]        ; cell C = (col, trunc(Y)+vdir)
    0041c4c9  and edx, 0x800 ; 0041c4d6 cmp [eax+0x4c], 3 -> the same turn
    0041c517  and ecx, 1                 ; C solid? no -> commit a 1-cell rise
    0041c528  add eax, [edx+0x44]        ; cell D = (col, trunc(Y)+2*vdir)
    0041c563  and ecx, 0x800 ; 0041c570 cmp [edx+0x4c], 3 -> the same turn
    0041c5b1  and eax, 1                 ; D solid? no -> a 2-cell rise; yes
    0041c5c1  mov dl, [ecx+0x48]         ;   -> the climber flag, else 0x41c676

`0x0042F66B` is the turn, read in full: `[+0x40] = -[+0x40]`, then `[+0x350]`
and `[+0x360]` set to -1.0 when the new facing is -1 and 0.0 when it is +1.
The probe returns **0** after turning, and `0x0041BDD4`'s slice loop keeps
going on a 0 (`cmp [ebp-4], 0` at 0x0041BE13), so the walker reverses and
spends the rest of the tick walking back.

The column is therefore ASYMMETRIC in the facing, and that is the measurement
this script exists for: the probe column is `trunc(X)` going right and
`trunc(X) - 1` going left, against a field stamped at the blocker's own
`trunc(X)`. Walking right, a walker turns when its own `trunc(X)` reaches the
blocker's column; walking left, one column short of it.

What it measured, 2026-10-15
----------------------------
    stamp      three blockers, four cells each, every cell predicted from the
               read BEFORE the click and then matched: (76, 71..74) for a
               left-facing blocker at X 76.645, (170, 78..81) right-facing at
               170.066, (105, 78..81) left-facing at 105.898. All
               0x00000000 -> 0x00000800, inside 5 ms of the press, nothing
               else in the 256 x 96 grid ever touched.
    turn       ten walkers. Field column 76: four left-facing turns at the
               snapped X = 77. Field column 105: one at X = 106. Field column
               170: five right-facing turns at X = 170. In every one the probe
               column -- `trunc(snapped X + [+0x360])` -- was the field column.
    erase      a blocker bombed (panel 2 on MM_Block_Bomb_A) entered the
               explode state 4.997 s after the click and all four of its cells
               went 0x00000800 -> 0x00000000 two milliseconds later, a whole
               second before the crater.
    audit      0 cells of 2,506,752 in the 102 shipped grids carry 0x800.

Offline, no process -- and it is the first half of the answer:

    python tools/harness/measure_blocker.py --audit
    python tools/harness/measure_blocker.py --predict MM_BLOCK_001_A

Live. Restart the process per level; there is no route back to the map. Level
ids are case-sensitive in the live table and MM_Block_Bomb_A really is spelt
that way:

    python tools/harness/measure_blocker.py --enter MM_BLOCK_001_A --probe
    python tools/harness/measure_blocker.py --stamp  --seconds 3 --watch 20
    python tools/harness/measure_blocker.py --tight  --dir 1  --watch 30
    python tools/harness/measure_blocker.py --tight  --dir -1 --watch 30
    python tools/harness/measure_blocker.py --bomb 2 --seconds 10
    python tools/harness/measure_blocker.py --watch-only 30   # not run 2026-10-15

`--stamp` snapshots the whole 256x96 grid, puts a blocker on through the panel
(`assign_skill.assign`, so the proof is the stock going down), and diffs the
grid per sample at about 680 Hz. `--tight` does the same but presses the
button from inside the diff loop, which is what times the stamp: `assign`
spends about 1.4 s watching the state and sitting out the double-click window
before the first sample, and the stamp is long over by then. `--watch N`
follows every live lemming's X, Y, state and `[+0x40]` for N more seconds and
prints every facing flip with the X the probe saw, which is the turn
measurement. `--bomb PANEL` puts that panel's skill on a lemming that is
already blocking and watches the field through it.

Nothing is written to the game except `--enter`'s level record swap, which is
put back (`slot [0][0] restored: True` on all three entries this session). No
lemming byte and no grid cell was ever written.

Three traps this round hit
--------------------------
  * `--tight --col N` is the wrong tool for a rare column. It needs
    `[screen+0x16D8]` to name the lemming it chose, and with lemmings packed
    near the hatch the engine names a neighbour instead -- round 5 hit the
    same wall. Two 40-150 s hunts for a walker at column 128 found candidates
    every time and never got one hoverable, and the second hunt ran the level
    clock out (`ui_state 5`).
  * Panel index 9, the one every level stocks exactly one of, was clicked
    twice on a level with one blocker left and did nothing at all: no grid
    change, the blocker still blocking. Whatever it is, it is not a nuke that
    reaches a blocker in two clicks.
  * The X a sample sees just before a turn is NOT the X the probe tested.
    `0x0041BECE` snaps X to the column boundary the walker was heading for and
    only then probes, so the probe X is `ceil` of the last sample going right
    and `floor` of it going left. Computing the probe column off the raw
    sample reports a miss by one column; `watch_walkers` does the snap.
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

BLOCKER_BIT = 0x800
PANEL_BLOCKER = 3           # MEASURED, docs/scratch/phase3-assign-skill.md

OFF_X, OFF_Y = 0x08, 0x10
OFF_DIR, OFF_VDIR = 0x40, 0x44
OFF_TYPE = 0x4C             # the `cmp [..+0x4c], 3` the probe exempts
OFF_XBIAS, OFF_YBIAS = 0x360, 0x368
OFF_STATE = 0x340

VT_BLOCK = 0x004B07B8
# The dword 3 at 0x004B10B8, the stamp loop's bound (j <= 3).
STAMP_ROWS = 4


def trunc(v: float) -> int:
    """0x0049C5AC chops toward zero."""
    return int(math.trunc(v))


def stamp_cells(x: float, y: float, dir_: int, xbias: float, ybias: float,
                vdir: int, rows: int):
    """The four cells `0x0040E20C` stamps, in live-grid (col, row) pairs.

    col  = trunc(wrap(trunc(X + [+0x360]))) + (1 if [+0x40] == -1)
    base = trunc(Y + [+0x368])
    rows = base + j * [+0x44] for j = 0..3, each clamped to the grid
    """
    col = (trunc(math.fmod(trunc(x + xbias), 256.0))
           + (1 if dir_ == -1 else 0)) & 0xFF
    base = trunc(y + ybias)
    out = []
    for j in range(STAMP_ROWS):
        r = base + j * vdir
        out.append((col, max(0, min(rows - 1, r))))
    return out


# --- offline ---------------------------------------------------------------


def audit() -> int:
    """How many cells of every shipped grid carry 0x800. Expect none."""
    files = sorted(f for f in os.listdir(CORPUS) if f.upper().endswith(".LVL"))
    bad, total, carriers = 0, 0, []
    for f in files:
        g = grid_of(os.path.join(CORPUS, f))
        if not g:
            bad += 1
            continue
        cells, _h = g
        total += 1
        n = sum(1 for c in cells if c & BLOCKER_BIT)
        if n:
            carriers.append((f, n))
    print(f"{total} shipped grids read, {bad} unreadable")
    if carriers:
        print("levels whose .LVL grid already carries 0x800:")
        for f, n in carriers:
            print(f"  {f:<24} {n} cells")
    else:
        print("cells carrying 0x800 in any shipped grid: NONE, 0 of"
              f" {total * GRID_WIDTH * 96}")
        print("so 0x800 is not level authoring; it can only be runtime state.")
    return 0


def predict(level: str) -> int:
    path = os.path.join(CORPUS, level.upper() + ".LVL")
    g = grid_of(path)
    if not g:
        print(f"no grid in {path}")
        return 1
    cells, height = g
    hist: dict[int, int] = {}
    for c in cells:
        hist[c] = hist.get(c, 0) + 1
    print(f"{level}  {GRID_WIDTH} x {height}   (offline, {path})")
    print("\ncell word        count   has 0x800")
    for word, n in sorted(hist.items(), key=lambda kv: -kv[1]):
        if n < 8:
            continue
        print(f"  {word:#010x} {n:>8}   {'YES' if word & BLOCKER_BIT else 'no'}")
    return 0


# --- live ------------------------------------------------------------------


def grid_shape(h):
    return i32(h, ROW_COUNT), u32(h, GRID_DATA_PTR)


def read_rows(h, base: int, r0: int, n: int):
    return read_mem(h, base + r0 * GRID_WIDTH * 4, n * GRID_WIDTH * 4)


def lem_core(h, lem: int):
    """X, Y, facing, vertical direction, `[+0x4c]`, and both biases."""
    d = read_mem(h, lem + OFF_X, 0x48)
    if not d or len(d) != 0x48:
        return None
    out = dict(
        x=struct.unpack_from("<d", d, 0)[0],
        y=struct.unpack_from("<d", d, OFF_Y - OFF_X)[0],
        dir=struct.unpack_from("<i", d, OFF_DIR - OFF_X)[0],
        vdir=struct.unpack_from("<i", d, OFF_VDIR - OFF_X)[0],
        type=struct.unpack_from("<i", d, OFF_TYPE - OFF_X)[0],
        xbias=0.0, ybias=0.0,
    )
    b = read_mem(h, lem + OFF_XBIAS, 0x10)
    if b and len(b) == 0x10:
        out["xbias"] = struct.unpack_from("<d", b, 0)[0]
        out["ybias"] = struct.unpack_from("<d", b, OFF_YBIAS - OFF_XBIAS)[0]
    return out


def print_lemmings(h) -> None:
    rows, base = grid_shape(h)
    for p in A.lemmings(h):
        c = lem_core(h, p)
        if c is None:
            continue
        _vt, name = A.state_of(h, p)
        box = A.lemming_box(h, p)
        on = bool(box and 0 <= (box[0] + box[2]) / 2 <= 640
                  and 0 <= (box[1] + box[3]) / 2 <= 480)
        cells = stamp_cells(c["x"], c["y"], c["dir"], c["xbias"], c["ybias"],
                            c["vdir"], rows)
        words = []
        for col, r in cells:
            w = read_mem(h, base + (r * GRID_WIDTH + col) * 4, 4)
            words.append(struct.unpack("<I", w)[0] if w and len(w) == 4 else None)
        print(f"  {p:#010x} x {c['x']:8.3f} y {c['y']:7.3f}"
              f" dir {c['dir']:+d} vdir {c['vdir']:+d} +0x4c {c['type']}"
              f" bias ({c['xbias']:+.1f},{c['ybias']:+.1f})"
              f"  {name:<6} {'on-screen' if on else 'off-screen'}"
              f"  would stamp {cells} = ["
              + " ".join("?" if w is None else f"{w:#07x}" for w in words)
              + "]", flush=True)


def whole_grid(h):
    rows, base = grid_shape(h)
    if not rows or not base:
        return None, None, None
    return rows, base, read_rows(h, base, 0, rows)


def watch_grid(h, lem: int, seconds: float, t_click: float, label: str = ""):
    """Diff the whole grid per sample and report every cell that changes.

    Two `ReadProcessMemory` calls an iteration: the whole 256 x rows grid and
    the lemming's own core. Nothing is written. A `0x800` transition is
    reported with the cell's old and new word, the time since the click, and
    the cell's offset from the lemming's own truncated cell.
    """
    rows, base, prev = whole_grid(h)
    print(f"  watching the whole grid, {len(prev)} bytes{label}", flush=True)
    events = []
    t0 = time.perf_counter()
    n = 0
    last = None
    while (t := time.perf_counter() - t0) < seconds:
        n += 1
        g = read_rows(h, base, 0, rows)
        core = lem_core(h, lem)
        vt, name = A.state_of(h, lem)
        changed = []
        if g and prev and g != prev:
            for i in range(0, len(g), 4):
                if g[i:i + 4] != prev[i:i + 4]:
                    old = struct.unpack_from("<I", prev, i)[0]
                    new = struct.unpack_from("<I", g, i)[0]
                    cell = i // 4
                    changed.append((cell // GRID_WIDTH, cell % GRID_WIDTH,
                                    old, new))
            prev = g
        if core is None:
            continue
        key = (name, trunc(core["x"]), trunc(core["y"]), core["dir"])
        if changed or key != last:
            for r, c, old, new in changed:
                events.append((time.perf_counter() - t_click, name, r, c,
                               old, new))
            cells = "".join(
                f"\n          cell (col {c:3d}, row {r:3d})"
                f"  {old:#010x} -> {new:#010x}"
                f"   {'+0x800' if (new & BLOCKER_BIT) and not (old & BLOCKER_BIT) else ''}"
                f"{'-0x800' if (old & BLOCKER_BIT) and not (new & BLOCKER_BIT) else ''}"
                f"   d(col)={((c - trunc(core['x']) + 128) % 256) - 128:+d}"
                f" d(row)={r - trunc(core['y']):+d}"
                for r, c, old, new in changed)
            print(f"  t{time.perf_counter() - t_click:+7.3f}s {name:<6}"
                  f" x {core['x']:8.3f} y {core['y']:7.3f}"
                  f" dir {core['dir']:+d} +0x4c {core['type']}{cells}",
                  flush=True)
            last = key
    print(f"  {n} grid samples in {seconds:.1f}s ({n / seconds:.0f} Hz),"
          f" {len(events)} cell changes", flush=True)
    return events


def live_field(h):
    """Every cell in the live grid carrying 0x800, as (col, row)."""
    rows, base, g = whole_grid(h)
    out = []
    for i in range(0, len(g), 4):
        if struct.unpack_from("<I", g, i)[0] & BLOCKER_BIT:
            cell = i // 4
            out.append((cell % GRID_WIDTH, cell // GRID_WIDTH))
    return sorted(out)


def tight_stamp(h, inp, target: int, settle: float = 1.2):
    """Press the button from inside the grid-diff loop, so the stamp is timed.

    `assign_skill.assign` watches the lemming's state and then sleeps out the
    engine's double-click window, which puts about 1.4 s between the press and
    the first grid sample. The stamp is a single store inside the enter hook,
    so to time it the diff loop has to straddle the click: this presses at
    t = 0, keeps diffing the whole grid at about 700 Hz, and releases the
    button four frames later from the same loop.
    """
    rows, base, prev = whole_grid(h)
    pre = prev
    events = []
    t0 = time.perf_counter()
    inp.button(0, True)
    released = False
    n = 0
    while (t := time.perf_counter() - t0) < settle:
        n += 1
        if not released and t >= 4 / 60.0:
            inp.button(0, False)
            released = True
        g = read_rows(h, base, 0, rows)
        if g and prev and g != prev:
            for i in range(0, len(g), 4):
                if g[i:i + 4] != prev[i:i + 4]:
                    old = struct.unpack_from("<I", prev, i)[0]
                    new = struct.unpack_from("<I", g, i)[0]
                    cell = i // 4
                    events.append((t, cell // GRID_WIDTH,
                                   cell % GRID_WIDTH, old, new))
            prev = g
    if not released:
        inp.button(0, False)
    print(f"  tight stamp: {n} whole-grid samples in {settle:.2f}s"
          f" ({n / settle:.0f} Hz), {len(events)} cell changes", flush=True)
    for t, r, c, old, new in events:
        print(f"    t {t:+7.4f}s  (col {c:3d}, row {r:3d})"
              f"  {old:#010x} -> {new:#010x}", flush=True)
    return events


def watch_walkers(h, seconds: float, field, blocker: int, t_click: float):
    """Every live lemming's facing, and the X at which it flips.

    The walk probe fires only when `0x0041BDD4` has snapped X to an exact
    column (`0x0041BECE`), so the X at a flip is an integer boundary. What is
    recorded is that integer, the probe column the facing BEFORE the flip
    implies (`trunc(X)` right, `trunc(X) - 1` left), and whether that column
    is the stamped one.
    """
    cols = sorted({c for c, _r in field})
    rowset = sorted({r for _c, r in field})
    print(f"  field columns {cols} rows {rowset}; following every lemming"
          f" for {seconds:.1f}s", flush=True)
    prev: dict[int, dict] = {}
    flips = []
    t0 = time.perf_counter()
    n = 0
    lems = A.lemmings(h)
    while (t := time.perf_counter() - t0) < seconds:
        n += 1
        if n % 500 == 0:
            lems = A.lemmings(h)
        for p in lems:
            c = lem_core(h, p)
            if c is None:
                continue
            vt, name = A.state_of(h, p)
            c["state"] = name
            was = prev.get(p)
            prev[p] = c
            if was is not None and was.get("state") != name:
                # Every state change, with the cell it happened in. A faller
                # that stops on a field cell lands four rows high: the
                # landing probe `0x0041A99A` accepts any NON-ZERO cell with
                # 0x80 clear, and a stamped air cell reads 0x00000800.
                onf = (trunc(c["x"]) & 0xFF, trunc(c["y"])) in field
                print(f"  t{time.perf_counter() - t_click:+7.3f}s STATE"
                      f" {p:#010x} {was.get('state')} -> {name}"
                      f"  x {c['x']:8.3f} y {c['y']:7.3f}"
                      f"  cell ({trunc(c['x']):3d},{trunc(c['y']):3d})"
                      f"  {'IN the field' if onf else ''}", flush=True)
            if was is None or was["dir"] == c["dir"]:
                continue
            # `0x0041BECE` snaps X to `trunc(X)` the instant the slice loop
            # lands on a column, and only then calls the probe, so the X the
            # probe saw is the boundary the walker was heading for, not the
            # last X sampled before it: `ceil` going right, `floor` going
            # left. The probe column is then `trunc(X + [+0x360])`, which is
            # 0.0 right and -1.0 left (0x0042F66B).
            edge = (math.ceil(was["x"]) if was["dir"] > 0
                    else math.floor(was["x"]))
            probe_col = trunc(edge + was["xbias"]) & 0xFF
            hit = probe_col in cols
            flips.append(dict(t=time.perf_counter() - t_click, lem=p,
                              name=name, x_before=was["x"], x_after=c["x"],
                              y=c["y"], dir_before=was["dir"],
                              dir_after=c["dir"], probe_col=probe_col,
                              on_field=hit, row=trunc(c["y"]), edge=edge))
            print(f"  t{time.perf_counter() - t_click:+7.3f}s FLIP"
                  f" {p:#010x} {name:<6} dir {was['dir']:+d} -> {c['dir']:+d}"
                  f"  x {was['x']:8.3f} -> {c['x']:8.3f}"
                  f"  y {c['y']:7.3f}  snapped x {edge:3d}"
                  f"  probe col {probe_col:3d}"
                  f"  {'ON the field' if hit else 'not the field'}"
                  f"  {'(the blocker itself)' if p == blocker else ''}",
                  flush=True)
    print(f"  {n} walker samples in {seconds:.1f}s ({n / seconds:.0f} Hz),"
          f" {len(flips)} facing flips", flush=True)
    return flips


def do_tight(h, watch: float, choose_dir: int = 0,
             col: int | None = None) -> int:
    """The stamp, with the press inside the grid-diff loop so it is timed."""
    from lrinput import LRInput

    if ui_state(h) != 0:
        print(f"ui_state {ui_state(h)}, not in a level")
        return 2
    A.wait_for_lemmings(h, 25.0)
    rows, base = grid_shape(h)
    scr = A.screen(h)
    print(f"stock {A.counts(h)} selected {A.selected(h)};"
          f" field before: {live_field(h)}", flush=True)
    with LRInput() as inp:
        inp.clear()
        inp.enable()
        try:
            if not A.select_skill(h, inp, PANEL_BLOCKER):
                print("  panel 3 not selected")
                return 2
            target = 0
            deadline = time.perf_counter() + 40.0
            while not target and time.perf_counter() < deadline:
              for p in A.lemmings(h):
                c = lem_core(h, p)
                if c is None:
                    continue
                _vt, name = A.state_of(h, p)
                if name != "walk":
                    continue
                if choose_dir and c["dir"] != choose_dir:
                    continue
                if col is not None and trunc(c["x"]) != col:
                    continue
                b = A.lemming_box(h, p)
                if not b:
                    continue
                A.aim_at(h, inp, (b[0] + b[2]) / 2, (b[1] + b[3]) / 2)
                b = A.lemming_box(h, p)
                if b:
                    A.aim_at(h, inp, (b[0] + b[2]) / 2, (b[1] + b[3]) / 2,
                             tol=1.0, tries=6)
                if A.hovered(h, scr) == p:
                    target = p
                    break
              if col is None:
                break
            if not target:
                print("  no hoverable walker of the wanted facing/column")
                return 2
            before = lem_core(h, target)
            cb = A.counts(h)
            print(f"  aiming at {target:#010x} x {before['x']:.3f}"
                  f" y {before['y']:.3f} dir {before['dir']:+d};"
                  f" the read predicts "
                  + str(stamp_cells(before["x"], before["y"], before["dir"],
                                    before["xbias"], before["ybias"],
                                    before["vdir"], rows)), flush=True)
            events = tight_stamp(h, inp, target)
            ca = A.counts(h)
            after = lem_core(h, target)
            _vt, name = A.state_of(h, target)
            print(f"  stock[3] {cb[3]} -> {ca[3]}"
                  f"  spent={ca[3] == cb[3] - 1}  state now {name}"
                  f"  x {after['x']:.3f} y {after['y']:.3f}"
                  f" dir {after['dir']:+d} +0x4c {after['type']}", flush=True)
            print(f"  the read predicts "
                  + str(stamp_cells(after["x"], after["y"], after["dir"],
                                    after["xbias"], after["ybias"],
                                    after["vdir"], rows))
                  + f"; field now {live_field(h)}", flush=True)
            time.sleep(0.6)
            if watch > 0:
                watch_walkers(h, watch, live_field(h), target,
                              time.perf_counter())
            return 0
        finally:
            inp.release()


def do_bomb(h, panel: int, seconds: float) -> int:
    """Put `panel` on a lemming that is already blocking, and watch the field.

    The point is the exit hook. `0x0042E696` calls the OUTGOING state's slot-3
    hook at `0x0042E6F5` before it stores the new state, and the block state's
    slot 3 is `0x0040E0DF`, whose `and eax, 0xf7ff` at `0x0040E1D5` is the only
    instruction in `.text` that clears `0x800`. So any departure from the block
    state should erase exactly the four cells the enter hook wrote -- a claim
    that is directly observable in the grid.
    """
    from lrinput import LRInput

    rows, base = grid_shape(h)
    scr = A.screen(h)
    before_field = live_field(h)
    print(f"  field before: {before_field}", flush=True)
    blockers = []
    for p in A.lemmings(h):
        c = lem_core(h, p)
        if c is None:
            continue
        _vt, name = A.state_of(h, p)
        if name == "block":
            blockers.append((p, c))
            print(f"  blocker {p:#010x} x {c['x']:.3f} y {c['y']:.3f}"
                  f" dir {c['dir']:+d} stamped "
                  + str(stamp_cells(c["x"], c["y"], c["dir"], c["xbias"],
                                    c["ybias"], c["vdir"], rows)), flush=True)
    if not blockers:
        print("  no lemming is blocking")
        return 2
    with LRInput() as inp:
        inp.clear()
        inp.enable()
        try:
            if not A.select_skill(h, inp, panel):
                print(f"  panel {panel} not selected")
                return 2
            target = 0
            for p, _c in blockers:
                b = A.lemming_box(h, p)
                if not b:
                    continue
                A.aim_at(h, inp, (b[0] + b[2]) / 2, (b[1] + b[3]) / 2)
                b = A.lemming_box(h, p)
                if b:
                    A.aim_at(h, inp, (b[0] + b[2]) / 2, (b[1] + b[3]) / 2,
                             tol=1.0, tries=6)
                if A.hovered(h, scr) == p:
                    target = p
                    break
            if not target:
                print("  no blocker became hoverable")
                return 2
            cb = A.counts(h)
            _r, _b, prev = whole_grid(h)
            t0 = time.perf_counter()
            inp.button(0, True)
            inp.wait_frames(4)
            inp.button(0, False)
            print(f"  clicked panel {panel} on blocker {target:#010x}"
                  f" at t 0", flush=True)
            n = 0
            last = None
            while (t := time.perf_counter() - t0) < seconds:
                n += 1
                g = read_rows(h, base, 0, rows)
                c = lem_core(h, target)
                _vt, name = A.state_of(h, target)
                if g and prev and g != prev:
                    for i in range(0, len(g), 4):
                        if g[i:i + 4] != prev[i:i + 4]:
                            old = struct.unpack_from("<I", prev, i)[0]
                            new = struct.unpack_from("<I", g, i)[0]
                            cell = i // 4
                            tag = ""
                            if (old & BLOCKER_BIT) and not (new & BLOCKER_BIT):
                                tag = "   -0x800"
                            if (new & BLOCKER_BIT) and not (old & BLOCKER_BIT):
                                tag = "   +0x800"
                            print(f"  t {t:+7.3f}s {name:<6}"
                                  f" (col {cell % GRID_WIDTH:3d},"
                                  f" row {cell // GRID_WIDTH:3d})"
                                  f"  {old:#010x} -> {new:#010x}{tag}",
                                  flush=True)
                    prev = g
                if name != last:
                    print(f"  t {t:+7.3f}s state -> {name}"
                          f"  x {c['x']:.3f} y {c['y']:.3f}" if c else
                          f"  t {t:+7.3f}s state -> {name}", flush=True)
                    last = name
            ca = A.counts(h)
            print(f"  {n} grid samples in {seconds:.1f}s ({n / seconds:.0f} Hz)",
                  flush=True)
            print(f"  stock[{panel}] {cb[panel]} -> {ca[panel]}"
                  f"  spent={ca[panel] == cb[panel] - 1}", flush=True)
            print(f"  field after: {live_field(h)}", flush=True)
            return 0
        finally:
            inp.release()


def do_panel(h, index: int, clicks: int, seconds: float) -> int:
    """Click a panel button `clicks` times and diff the grid through it.

    Used to answer "what happens to the field when the blocker is removed".
    Panel index 9 is the one every level stocks exactly one of; what it is was
    not established before this run, so what it does is reported rather than
    named. The field's fate is read off the grid either way.
    """
    from lrinput import LRInput

    rows, base = grid_shape(h)
    print(f"  field before: {live_field(h)}", flush=True)
    for p in A.lemmings(h):
        c = lem_core(h, p)
        _vt, name = A.state_of(h, p)
        print(f"  {p:#010x} {name:<6} x {c['x']:.3f} y {c['y']:.3f}"
              f" +0x4c {c['type']}", flush=True)
    box = A.panel_box(h, index)
    print(f"  panel {index} box {box}", flush=True)
    with LRInput() as inp:
        inp.clear()
        inp.enable()
        try:
            A.aim_at(h, inp, (box[0] + box[2]) / 2, (box[1] + box[3]) / 2)
            _rows, _base, prev = whole_grid(h)
            t0 = time.perf_counter()
            fired = 0
            n = 0
            while (t := time.perf_counter() - t0) < seconds:
                n += 1
                if fired < clicks and t >= fired * 0.9:
                    inp.button(0, True)
                    inp.wait_frames(4)
                    inp.button(0, False)
                    fired += 1
                    print(f"  t {t:+7.3f}s  click {fired} on panel {index}",
                          flush=True)
                g = read_rows(h, base, 0, rows)
                if g and prev and g != prev:
                    for i in range(0, len(g), 4):
                        if g[i:i + 4] != prev[i:i + 4]:
                            old = struct.unpack_from("<I", prev, i)[0]
                            new = struct.unpack_from("<I", g, i)[0]
                            cell = i // 4
                            tag = ""
                            if (old & BLOCKER_BIT) and not (new & BLOCKER_BIT):
                                tag = "  -0x800"
                            if (new & BLOCKER_BIT) and not (old & BLOCKER_BIT):
                                tag = "  +0x800"
                            print(f"  t {t:+7.3f}s  (col {cell % GRID_WIDTH:3d},"
                                  f" row {cell // GRID_WIDTH:3d})"
                                  f"  {old:#010x} -> {new:#010x}{tag}",
                                  flush=True)
                    prev = g
            print(f"  {n} grid samples in {seconds:.1f}s ({n / seconds:.0f} Hz)",
                  flush=True)
            print(f"  ui_state {ui_state(h)}  field after: {live_field(h)}"
                  if ui_state(h) == 0 else f"  ui_state {ui_state(h)}",
                  flush=True)
            for p in A.lemmings(h):
                c = lem_core(h, p)
                _vt, name = A.state_of(h, p)
                if c:
                    print(f"  {p:#010x} {name:<6} x {c['x']:.3f}"
                          f" y {c['y']:.3f}", flush=True)
            return 0
        finally:
            inp.release()


def do_stamp(h, seconds: float, watch: float, choose_dir: int = 0,
             col: int | None = None) -> int:
    from lrinput import LRInput

    if ui_state(h) != 0:
        print(f"ui_state {ui_state(h)}, not in a level")
        return 2
    n = A.wait_for_lemmings(h, 25.0)
    rows, base = grid_shape(h)
    print(f"{n} lemmings; stock {A.counts(h)} selected {A.selected(h)};"
          f" grid {GRID_WIDTH} x {rows} at {base:#010x}", flush=True)
    print("live lemmings, and the four cells the stamp read says each would"
          " write:", flush=True)
    print_lemmings(h)

    pre_rows, pre_base, pre = whole_grid(h)
    pre_bit = sum(1 for i in range(0, len(pre), 4)
                  if struct.unpack_from("<I", pre, i)[0] & BLOCKER_BIT)
    print(f"  cells carrying 0x800 BEFORE the click: {pre_bit}", flush=True)

    def pick(handle, lems):
        best = None
        for p in lems:
            c = lem_core(handle, p)
            if c is None:
                continue
            _vt, name = A.state_of(handle, p)
            if name != "walk":
                continue
            if choose_dir and c["dir"] != choose_dir:
                continue
            box = A.lemming_box(handle, p)
            if not (box and 0 <= (box[0] + box[2]) / 2 <= 640
                    and 0 <= (box[1] + box[3]) / 2 <= 480):
                continue
            if col is not None and trunc(c["x"]) == col:
                return p
            if best is None:
                best = p
        return None if col is not None else best

    with LRInput() as inp:
        inp.clear()
        inp.enable()
        try:
            t_click = time.perf_counter()
            res = A.assign(h, inp, PANEL_BLOCKER, choose=pick, window=0.4)
            print(f"  assign: ok={res['ok']} {res.get('reason', '')}"
                  f" lemming {res.get('lemming', 0):#010x}"
                  f" stock {res.get('count_before')} ->"
                  f" {res.get('count_after')}"
                  f" states {res.get('states')}", flush=True)
            lem = res.get("lemming", 0)
            if not lem:
                return 2
            core = lem_core(h, lem)
            field = stamp_cells(core["x"], core["y"], core["dir"],
                                core["xbias"], core["ybias"], core["vdir"],
                                rows)
            print(f"  blocker at ({core['x']:.3f}, {core['y']:.3f})"
                  f" dir {core['dir']:+d} vdir {core['vdir']:+d}"
                  f" +0x4c {core['type']}: the read predicts {field}",
                  flush=True)
            events = watch_grid(h, lem, seconds, t_click)
            post_rows, post_base, post = whole_grid(h)
            got = []
            for i in range(0, len(post), 4):
                a = struct.unpack_from("<I", pre, i)[0]
                b = struct.unpack_from("<I", post, i)[0]
                if (a ^ b) == 0:
                    continue
                cell = i // 4
                got.append((cell % GRID_WIDTH, cell // GRID_WIDTH, a, b))
            print("\n  whole-grid before/after diff:", flush=True)
            for col, r, a, b in got:
                print(f"    (col {col:3d}, row {r:3d})  {a:#010x} -> {b:#010x}"
                      f"   d(col)={col - trunc(core['x']):+d}"
                      f" d(row)={r - trunc(core['y']):+d}", flush=True)
            gained = sorted((c, r) for c, r, a, b in got
                            if (b & BLOCKER_BIT) and not (a & BLOCKER_BIT))
            print(f"  cells that GAINED 0x800: {gained}", flush=True)
            print(f"  the read predicted        {sorted(field)}", flush=True)
            print(f"  MATCH: {gained == sorted(field)}", flush=True)
            flips = []
            if watch > 0:
                print("", flush=True)
                flips = watch_walkers(h, watch, live_field(h), lem, t_click)
            print("\n== summary", flush=True)
            print(f"  blocker {lem:#010x} at ({core['x']:.3f},"
                  f" {core['y']:.3f}) dir {core['dir']:+d}", flush=True)
            print(f"  stamped {gained}, predicted {sorted(field)},"
                  f" match {gained == sorted(field)}", flush=True)
            for e in events:
                print(f"    t{e[0]:+7.3f}s {e[1]:<6} (col {e[3]:3d},"
                      f" row {e[2]:3d})  {e[4]:#010x} -> {e[5]:#010x}",
                      flush=True)
            for f in flips:
                print(f"    t{f['t']:+7.3f}s flip {f['lem']:#010x}"
                      f" {f['name']:<6} {f['dir_before']:+d} ->"
                      f" {f['dir_after']:+d} at x {f['x_before']:.3f}"
                      f" y {f['y']:.3f} probe col {f['probe_col']}"
                      f" on_field={f['on_field']}", flush=True)
            return 0
        finally:
            inp.release()


def do_watch(h, seconds: float) -> int:
    """Follow every lemming with no click at all, against the live field."""
    rows, base = grid_shape(h)
    _r, _b, g = whole_grid(h)
    field = []
    for i in range(0, len(g), 4):
        if struct.unpack_from("<I", g, i)[0] & BLOCKER_BIT:
            cell = i // 4
            field.append((cell % GRID_WIDTH, cell // GRID_WIDTH))
    print(f"  the live field is {sorted(field)}", flush=True)
    watch_walkers(h, seconds, field, 0, time.perf_counter())
    return 0


def main() -> int:
    ap = argparse.ArgumentParser(description=__doc__.splitlines()[0],
                                 formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--audit", action="store_true",
                    help="offline: count 0x800 in every shipped grid")
    ap.add_argument("--predict", metavar="LEVEL",
                    help="offline: the cell words of one level")
    ap.add_argument("--enter", metavar="LEVEL", help="enter this level first")
    ap.add_argument("--probe", action="store_true",
                    help="read and print, send no input")
    ap.add_argument("--stamp", action="store_true",
                    help="put a blocker on through the panel and diff the grid")
    ap.add_argument("--tight", action="store_true",
                    help="the same, with the press inside the diff loop so the"
                         " stamp is timed relative to the click")
    ap.add_argument("--dir", type=int, default=0, choices=(-1, 0, 1),
                    help="only block a lemming facing this way")
    ap.add_argument("--seconds", type=float, default=3.0,
                    help="how long to diff the grid after the click")
    ap.add_argument("--watch", type=float, default=0.0,
                    help="then follow every lemming's facing for this long")
    ap.add_argument("--watch-only", type=float, default=0.0,
                    help="no click: follow every lemming against the live field")
    ap.add_argument("--panel", type=int, default=None,
                    help="click this panel button and diff the grid through it")
    ap.add_argument("--clicks", type=int, default=1)
    ap.add_argument("--col", type=int, default=None,
                    help="with --tight, only block a walker whose trunc(X) is"
                         " this column; waits up to 60 s for one")
    ap.add_argument("--bomb", type=int, default=None, metavar="PANEL",
                    help="put this panel's skill on a lemming that is already"
                         " blocking and watch the field (2 on MM_Block_Bomb_A)")
    args = ap.parse_args()

    if args.audit:
        return audit()
    if args.predict:
        return predict(args.predict)

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
        if args.probe:
            rows, base = grid_shape(h)
            print(f"ui_state {ui_state(h)}  grid {GRID_WIDTH} x {rows}"
                  f" at {base and hex(base)}  stock {A.counts(h)}"
                  f"  selected {A.selected(h)}")
            A.wait_for_lemmings(h, 25.0)
            print_lemmings(h)
            return 0
        if args.watch_only:
            return do_watch(h, args.watch_only)
        if args.bomb is not None:
            return do_bomb(h, args.bomb, args.seconds)
        if args.panel is not None:
            return do_panel(h, args.panel, args.clicks, args.seconds)
        if args.tight:
            return do_tight(h, args.watch, args.dir, args.col)
        if args.stamp:
            return do_stamp(h, args.seconds, args.watch, args.dir,
                            args.col)
        print("nothing to do; pass --audit, --probe, --stamp or --watch-only")
        return 0
    finally:
        k32.CloseHandle(h)


if __name__ == "__main__":
    raise SystemExit(main())
