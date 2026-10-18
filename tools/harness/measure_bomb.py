"""How long the bomber's fuse is, when Ohno plays, and what the blast removes.

Phase 3 round 7, live. This tests `docs/scratch/phase3-round7.md` (the static
read of the fuse and the Ohno cue) and carries that read two steps further,
because the thing round 7 could not find -- the explosion footprint -- turns
out not to be a radius at all. Every address below is against the stock exe
(SHA-256 32675bdb...a2f, image base 0x400000).

The chain, four objects deep
----------------------------
1. **The countdown**, vtable `0x004B0990`, built by the bomb factory
   `0x0045B197` (entry [2] of the skill table at `0x0045B570`, so panel index
   2 -- the index round 8 measured):

       0045b1b3  6a 05                 push 5
       0045b1b8  e8 83 8b fb ff        call 0x413d40     ; the ctor
       00413d74  db 45 08              fild dword ptr [ebp + 8]
       00413d7a  dd 58 08              fstp qword ptr [eax + 8]   ; 5.0

   Its `[this+0x11]` is 1 (`0x00413D8A`), so `0x0042E696` pushes it onto the
   lemming's `+0x374` LIST (`0x0042E6CA`) instead of replacing `+0x340`: a
   primed lemming keeps walking and keeps its own state. `countdown_of` walks
   that list live and reads the double at `+8` -- the remaining fuse, in
   seconds.

2. **The event.** The countdown's enter `0x00413DF9` schedules a key at
   `clock + trunc(5.0) * 1000` ms (`[0x004CBF80]` holds 1000, written
   `mov [edx], 0x3e8` at `0x004657B2`; the clock is `[0x004CBF88]`). The
   scheduler runs it from `0x0043F008` at `0x0045DC1D`. The helper step
   `0x00414370` plays the registered `Ohno` sample -- GATED on the byte at
   `0x004CBB40` being zero:

       00414420  8a 15 40 bb 4c 00     mov dl, byte ptr [0x4cbb40]
       00414426  85 d2                 test edx, edx
       00414428  75 1e                 jne 0x414448          ; skip the play
       00414435  e8 c6 25 ff ff        call 0x406a00         ; the sound id
       00414443  e8 13 0f 03 00        call 0x44535b         ; play it

   and then installs the flapper whatever the byte says. `--probe` reads that
   byte out of the running process, which is the one thing the static read
   could not do.

3. **The flapper**, vtable `0x004B0A58`, enter `0x0041695C` -> animation
   `explode` (`0x004BF3E0`). Its step `0x0041699E` is animation plus a fall
   (`call 0x416038`, the faller's own speed accessor, when the cell below
   reads 0), and when the strip finishes (`call [eax+0x44]` at `0x00416AC6`)
   it builds the next object with `0x004081B0` and installs it
   (`0x00416B56`). So the flapper is a whole animation cycle long and the
   lemming is still falling through it.

4. **The blast**, vtable `0x004B0550` (`0x004081C2`), type tag 2 =
   `EXPLODE`. Enter `0x004081FC` plays `[0x004E32C0]` (a different sound id
   from Ohno's `[0x004E32CC]`). Its STEP `0x00408227` is the crater:

       00408273  rep movsd                      ; pos = lemming's x, y, z
       00408287  fild [0x4b10bc] (2) ; fdiv 2.0 ; fadd -> x + 1.0
       00408299  fild [0x4b10b8] (3) ; fdiv 2.0 ; fadd -> y + 1.5
       004082b6  call 0x43b31e                  ; the VISUAL, on x+1, y+1.5
       004082bb  fsub [0x4b02d0] (1.0)          ; x back to the lemming's x
       004082d7  call 0x43f510                  ; the TERRAIN, arg2 = 0

The footprint is a stencil, not a radius
----------------------------------------
`0x0043F510` is the only thing that touches the grid, and it is a stored
bitmap:

    0043f530  cmp dword ptr [ebp + 0xc], 0
    0043f536  mov [ebp-0x20], 7  ; mov [ebp-0x1c], 0x4b1528   <- arg2 == 0
    0043f546  mov [ebp-0x20], 9  ; mov [ebp-0x1c], 0x4b155c   <- arg2 != 0
    0043f557  fild [ebp-0x20] ; fdiv 2.0 ; fsubr [eax+8]      ; row0 = y - n/2
    0043f57b  fild [ebp-0x20] ; fdiv 2.0 ; fsubr [ecx]        ; col0 = x - n/2
    0043f5e9  mov al, [edx] ; add [ebp-0x1c], 1               ; one mask byte
    0043f5fb  je  0x43f7fe                                    ;   0 -> skip
    0043f60e  call 0x40de60                                   ; the cell
    0043f61b  and edx, 2     ; jne skip      (indestructible)
    0043f62b  and ecx, 0x200 ; jne skip      (the zone bit)
    0043f63e  and eax, 0x800 ; jne skip      (a blocker's field)
    0043f650  and edx, 1     ; je  0x43f7f5  (air: clear it anyway)
    0043f65e/672  col & 3 == 0 and row & 3 == 0 -> a debris object
    0043f7f5  mov dword ptr [edx], 0                          ; THE REMOVAL

So a lemming's bomb is the **7 x 7** stencil at `0x004B1528`, 37 of 49 bytes
set, read row-major, one byte per cell, with

    col0 = trunc(wrap(X - 3.5))     rows wrap the cylinder (`and eax, 0xff`)
    row0 = trunc(clamp(Y + 1.5 - 3.5)) = trunc(clamp(Y - 2.0))

The 9 x 9 at `0x004B155C` (61 of 81) belongs to the OTHER caller,
`0x0041DF89`, which passes 1 -- an object with its own countdown at `[+0xa0]`,
not a lemming. The removal is `mov dword ptr [edx], 0`, the whole dword, the
same instruction shape round 5 measured for the bash / dig / mine bite.

The stencil at 0x004B1528, as the loops read it (row 0 first):

      . . # # # . .
      . # # # # # .
      # # # # # # #
      # # # # # # #
      # # # # # # #
      . # # # # # .
      . . # # # . .

Re-run
------
Offline, no process -- the stencil, the masks' byte counts, and the predicted
footprint for a given position:

    python tools/harness/measure_bomb.py --stencil
    python tools/harness/measure_bomb.py --predict 170.066 77.0

Live. Restart the process per level; there is no route from inside a level
back to the map. Level ids are case-sensitive in the live table:

    python tools/harness/measure_bomb.py --enter MM_BOMB --probe
    python tools/harness/measure_bomb.py --bomb --seconds 9
    python tools/harness/measure_bomb.py --bomb --seconds 9 --panel 2

`--probe` and `--stencil` send no input. `--enter` writes only
`goto_level`'s level-record swap and puts it back. No lemming byte and no
grid cell is ever written.

`--bomb` presses the button from inside the diff loop, the way
`measure_blocker.tight_stamp` does, so the click, the countdown double, the
state changes and the grid all sit on one clock. Nothing is slept through.
`--col N` restricts the aim to a lemming whose trunc(X) is N, which is how a
blocker with neighbours inside the blast window was picked out.

What it measured, 2026-10-15
----------------------------
Six bombs, two levels, ~650 Hz whole-grid diff through every one:

    fuse      click -> flapper  4.9967 4.9985 4.9981 4.9976 4.9998 4.9980 s
              and the countdown double itself fell 4.9940 -> 0.1330 over
              4.8616 s, i.e. 1.000 s of fuse per second of wall clock
    flap      flapper -> blast  1.0211 1.0193 1.0226 1.0206 1.0191 1.0201 s
              against the read 17 frames x 0.06 / 1.0 = 1.020 s (strip 6)
    drop      the world Y is one unit LOWER in the blast state than in the
              flapper: 70 -> 69, 78 -> 77, four of four runs that sampled
              both. That is 0x00416B5B, after the install.
    crater    75 cells, every one 0x00000001 -> 0x00000000, and in five of
              six runs the changed set was EXACTLY the stencil cells that
              had held a non-zero word. Nothing outside the window, ever.
    gate      [0x004CBB40] reads 0x00 on both levels, before and after every
              bomb. It sits in .data's uninitialised tail (virtual size
              0x1185B8, raw size 0x7000), so the loader zero-fills it and
              the "0xFF in the image" in the static note was a read of the
              file past that section's raw bytes. Ohno always plays.
    walk      a primed walker went X 144.770 -> 119.770 in the five seconds,
              exactly 25.000 columns: the countdown does not interrupt it.
"""
from __future__ import annotations

import argparse
import math
import struct
import sys
import time
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
from lrstate import attach, u32, ui_state  # noqa: E402
from memdump import k32, read_mem  # noqa: E402

import assign_skill as A  # noqa: E402
import measure_blocker as B  # noqa: E402
import measure_build as BU  # noqa: E402
import measure_dig as D  # noqa: E402

GRID_WIDTH = 256

PANEL_BOMBER = 2            # MEASURED, docs/scratch/phase3-round8-blocker.md

VT_COUNTDOWN = 0x004B0990   # the fuse object, ctor 0x00413D40
VT_FLAPPER = 0x004B0A58     # animation `explode`, enter 0x0041695C
VT_BLAST = 0x004B0550       # type tag 2 = EXPLODE, ctor 0x004081B0

GATE_BYTE = 0x004CBB40      # `jne` past the Ohno play at 0x00414428
EVENT_CLOCK_PTR = 0x004CBF88
EVENT_MULT_PTR = 0x004CBF80  # -> the dword 1000 (0x004657B2)
OHNO_ID_PTR = 0x004E32CC    # 0x004C6A08 + 0x1C8C4, registered at 0x0045E5B8
BLAST_ID_PTR = 0x004E32C0   # 0x004C6A08 + 0x1C8B8, the blast enter's sound

OFF_EXTRA_LIST = 0x374      # the list 0x0042E6CA pushes a `[+0x11]` state onto
OFF_STATE = 0x340

STENCIL_7 = 0x004B1528      # 7 x 7, the lemming bomb (arg2 == 0)
STENCIL_9 = 0x004B155C      # 9 x 9, 0x0041DF89's caller (arg2 == 1)

# Name the two new states so `A.state_of` prints something useful. The map
# already calls 0x004B0A58 "explode"; this round needs to tell the flapper
# apart from the blast, so both are renamed here rather than in that file.
A.STATE_NAMES[VT_FLAPPER] = "flapper"
A.STATE_NAMES[VT_BLAST] = "blast"
A.STATE_NAMES[VT_COUNTDOWN] = "countdown"
D.SKILL_VT[VT_FLAPPER] = "flapper"
D.SKILL_VT[VT_BLAST] = "blast"


def trunc(v: float) -> int:
    """0x0049C5AC chops toward zero."""
    return int(math.trunc(v))


def stencil(size: int, addr: int):
    """The mask bytes at `addr`, as a list of `size` rows of `size` bytes.

    Offline: straight out of the exe's .rdata. `0x0043F5E9` reads one byte per
    inner iteration and never resets the pointer, so the bitmap is row-major
    with the outer loop (the grid row, counting up) first.
    """
    import read_steppers as RS
    exe = RS.DEFAULT_EXE
    data = Path(exe).read_bytes()
    off = addr - 0x4B0000 + 0xB0000
    raw = data[off:off + size * size]
    return [list(raw[r * size:(r + 1) * size]) for r in range(size)]


def crater_cells(x: float, y: float, rows: int, size: int = 7,
                 addr: int = STENCIL_7):
    """The cells `0x0043F510` visits, in live-grid (col, row) pairs.

    row0 = trunc(clamp(Y + 1.5 - size/2)), col0 = trunc(wrap(X - size/2)).
    The Y+1.5 is `0x00408299`'s `fild [0x4b10b8] (3) / 2.0`; the X+1.0 the
    visual gets at `0x00408287` is subtracted again at `0x004082BB`, so the
    terrain call sees the lemming's own X.
    """
    mask = stencil(size, addr)
    half = size / 2.0
    col0 = trunc(math.fmod(x - half, 256.0))
    if col0 < 0:
        col0 += 256
    row0 = trunc(max(0.0, min(float(rows - 1), y + 1.5 - half)))
    out = []
    for i in range(size):
        r = row0 + i
        if r >= rows:
            break
        for j in range(size):
            if mask[i][j]:
                out.append(((col0 + j) & 0xFF, r))
    return out


# --- offline ---------------------------------------------------------------


def do_stencil() -> int:
    for size, addr in ((7, STENCIL_7), (9, STENCIL_9)):
        m = stencil(size, addr)
        n = sum(sum(1 for b in row if b) for row in m)
        who = ("the LEMMING bomb -- 0x00408227 pushes 0"
               if size == 7 else
               "0x0041DF89's object -- it pushes 1, not a lemming")
        print(f"{addr:#010x}  {size} x {size}  {n} of {size * size} bytes set"
              f"   {who}")
        for row in m:
            print("    " + " ".join("#" if b else "." for b in row))
        print("")
    print("row 0 is the FIRST outer iteration, i.e. the LOWEST grid row"
          " (row0 = trunc(Y - 2.0) in the Y-up live grid).")
    print("Both masks are symmetric top-to-bottom, so a flipped reading of"
          " the rows would give the same cells.")
    return 0


def do_predict(x: float, y: float, rows: int = 96) -> int:
    cells = crater_cells(x, y, rows)
    half = 7 / 2.0
    print(f"a lemming at ({x}, {y}) in a {GRID_WIDTH} x {rows} grid")
    print(f"  col0 = trunc({x} - {half}) = {trunc(x - half)}")
    print(f"  row0 = trunc({y} + 1.5 - {half}) = {trunc(y + 1.5 - half)}")
    print(f"  {len(cells)} cells: {cells}")
    cols = sorted({c for c, _r in cells})
    rws = sorted({r for _c, r in cells})
    print(f"  columns {cols[0]}..{cols[-1]}  rows {rws[0]}..{rws[-1]}")
    print(f"  relative to the lemming's own cell"
          f" ({trunc(x)}, {trunc(y)}):"
          f" d(col) {cols[0] - trunc(x)}..{cols[-1] - trunc(x)}"
          f"  d(row) {rws[0] - trunc(y)}..{rws[-1] - trunc(y)}")
    return 0


# --- live ------------------------------------------------------------------


def gate(h) -> int | None:
    d = read_mem(h, GATE_BYTE, 1)
    return d[0] if d else None


def countdown_of(h, lem: int):
    """The live fuse: walk `lemming+0x374` for the 0x004B0990 object.

    The list is a 0xC-byte-node chain (`0x00430590` allocates 0xC and links
    `[list+4]` as the tail), so this walks a bounded number of nodes from both
    `[+0x374]` and `[+0x378]` and accepts whichever dword points at an object
    whose vtable is the countdown's. Returns (ptr, remaining seconds).
    """
    for head_off in (0x00, 0x04):
        node = u32(h, lem + OFF_EXTRA_LIST + head_off) or 0
        seen = set()
        while node and node not in seen and len(seen) < 16:
            seen.add(node)
            d = read_mem(h, node, 0x0C)
            if not d or len(d) != 0x0C:
                break
            val, _a, nxt = struct.unpack("<III", d)
            if val:
                vt = u32(h, val) or 0
                if vt == VT_COUNTDOWN:
                    raw = read_mem(h, val + 8, 8)
                    rem = struct.unpack("<d", raw)[0] if raw and len(raw) == 8 else None
                    return val, rem
            node = nxt
    return 0, None


def state_vt(h, lem: int):
    sp = u32(h, lem + OFF_STATE) or 0
    if not sp:
        return 0, 0, "none"
    vt = u32(h, sp) or 0
    return sp, vt, A.STATE_NAMES.get(vt, f"{vt:#010x}")


def walkers(h):
    """Every live lemming with its core, for the aim pass."""
    out = []
    for p in A.lemmings(h):
        c = B.lem_core(h, p)
        if c is None:
            continue
        _sp, _vt, name = state_vt(h, p)
        box = A.lemming_box(h, p)
        on = bool(box and 0 <= (box[0] + box[2]) / 2 <= 640
                  and 0 <= (box[1] + box[3]) / 2 <= 480)
        out.append((p, c, name, box, on))
    return out


def do_probe(h) -> int:
    rows, base = B.grid_shape(h)
    g = gate(h)
    print(f"ui_state {ui_state(h)}  grid {GRID_WIDTH} x {rows}")
    print(f"stock {A.counts(h)}   selected {A.selected(h)}")
    print(f"the Ohno gate byte [{GATE_BYTE:#010x}] = "
          f"{'?' if g is None else f'{g:#04x}'}"
          f"   -> Ohno {'is NOT played' if g else 'IS played'} at"
          f" 0x00414420")
    print(f"event clock [{EVENT_CLOCK_PTR:#010x}] = {u32(h, EVENT_CLOCK_PTR)}"
          f"   multiplier *[{EVENT_MULT_PTR:#010x}] ="
          f" {u32(h, u32(h, EVENT_MULT_PTR) or 0)}")
    print(f"sound ids: Ohno [{OHNO_ID_PTR:#010x}] = {u32(h, OHNO_ID_PTR)}"
          f"   blast [{BLAST_ID_PTR:#010x}] = {u32(h, BLAST_ID_PTR)}")
    for p, c, name, box, on in walkers(h):
        cp, rem = countdown_of(h, p)
        fuse = "" if cp == 0 else f"  FUSE {cp:#010x} {rem:.4f}s"
        print(f"  {p:#010x} x {c['x']:8.3f} y {c['y']:7.3f}"
              f" dir {c['dir']:+d} {name:<9}"
              f" {'on-screen' if on else 'off-screen'}{fuse}")
    lems = A.lemmings(h)
    if lems:
        a, table = BU.strips(h, lems[0], 20)
        if table:
            print("animation strips, live (index, frames, divisor,"
                  " frames*0.06/divisor):")
            for i, frames, div in table:
                print(f"  [{i:2d}] {frames:3d}f  div {div:g}"
                      f"  = {frames * 0.06 / (div or 1.0):.3f} s")
    return 0


SKIP_BITS = 0x2 | 0x200 | 0x800   # 0x0043F61B / 0x0043F62B / 0x0043F63E


def explain(pre: bytes, observed, rows: int, size: int = 7,
            addr: int = STENCIL_7) -> None:
    """Which (col0, row0) reproduces the observed changes exactly.

    `0x0043F510` writes a whole dword 0 into every stencil cell that carries
    none of 0x2 / 0x200 / 0x800, so the cells that CHANGE are exactly the
    stencil cells whose prior word was non-zero and carried none of those
    three. That makes the footprint falsifiable: for each candidate anchor,
    the predicted change set is computed from the grid as it was BEFORE the
    click, and compared as a set.

    The anchor is swept over a window rather than taken from one position,
    because the flapper falls and the blast step reads the position on the
    frame it runs -- so the search says what the anchor was, and the caller
    compares that with the sampled X and Y.
    """
    mask = stencil(size, addr)
    obs = set(observed)
    if not obs:
        return

    def word(col, r):
        i = (r * GRID_WIDTH + (col & 0xFF)) * 4
        return struct.unpack_from("<I", pre, i)[0] if pre else 0

    cols = sorted({c for c, _r in obs})
    rws = sorted({r for _c, r in obs})
    best = []
    for col0 in range(cols[0] - size, cols[-1] + 1):
        for row0 in range(max(0, rws[0] - size), rws[-1] + 1):
            pred = set()
            for i in range(size):
                r = row0 + i
                if r >= rows:
                    continue
                for j in range(size):
                    if not mask[i][j]:
                        continue
                    col = (col0 + j) & 0xFF
                    w = word(col, r)
                    if w and not (w & SKIP_BITS):
                        pred.add((col, r))
            if pred == obs:
                best.append((col0, row0))
    print(f"    anchors (col0, row0) whose stencil predicts the observed"
          f" {len(obs)} changes EXACTLY: {best}")
    if best:
        col0, row0 = best[0]
        print(f"    the whole {size} x {size} window at"
              f" (col0 {col0}, row0 {row0}), word BEFORE the click,"
              f" '-' where the stencil byte is 0, '*' where the cell changed:")
        for i in reversed(range(size)):       # print the top row first
            r = row0 + i
            row = []
            for j in range(size):
                col = (col0 + j) & 0xFF
                if not mask[i][j]:
                    row.append("     -    ")
                    continue
                w = word(col, r)
                mark = "*" if (col, r) in obs else " "
                row.append(f"{w:#010x}{mark}")
            print(f"      row {r:3d} " + " ".join(row))
        skipped = [(col0 + j & 0xFF, row0 + i, word((col0 + j) & 0xFF, row0 + i))
                   for i in range(size) for j in range(size)
                   if mask[i][j] and ((col0 + j) & 0xFF, row0 + i) not in obs
                   and row0 + i < rows]
        nonzero = [(c, r, w) for c, r, w in skipped if w]
        print(f"      stencil cells that did not change: {len(skipped)},"
              f" of which {len(nonzero)} held a non-zero word"
              + (f": {[(c, r, hex(w)) for c, r, w in nonzero]}"
                 if nonzero else " (so every solid cell in the window went)"))
    if not best:
        # Nearest miss, so a disagreement is reported rather than hidden.
        scored = []
        for col0 in range(cols[0] - size, cols[-1] + 1):
            for row0 in range(max(0, rws[0] - size), rws[-1] + 1):
                pred = set()
                for i in range(size):
                    r = row0 + i
                    if r >= rows:
                        continue
                    for j in range(size):
                        if not mask[i][j]:
                            continue
                        col = (col0 + j) & 0xFF
                        w = word(col, r)
                        if w and not (w & SKIP_BITS):
                            pred.add((col, r))
                scored.append((len(pred ^ obs), col0, row0, pred))
        scored.sort()
        d, col0, row0, pred = scored[0]
        print(f"    NO exact anchor. nearest is (col0 {col0}, row0 {row0}),"
              f" {d} cells of symmetric difference:")
        print(f"      predicted-not-seen {sorted(pred - obs)}")
        print(f"      seen-not-predicted {sorted(obs - pred)}")


def do_bomb(h, panel: int, seconds: float, target_state: str = "walk",
            col: int | None = None) -> int:
    """Click `panel` on a walker from inside the diff loop and watch it all.

    One iteration reads: the whole 256 x rows grid, the lemming's core, its
    state pointer and vtable, and the countdown double off the `+0x374` list.
    Nothing is written apart from the mouse button. The three moments this
    round needs -- the click, the flapper, the blast -- are all on this clock.
    """
    from lrinput import LRInput

    rows, base = B.grid_shape(h)
    scr = A.screen(h)
    if not scr:
        print("  no level screen")
        return 2
    cands = [w for w in walkers(h) if w[2] == target_state and w[4]]
    if col is not None:
        cands = [w for w in cands if trunc(w[1]["x"]) == col]
    print(f"  {len(cands)} on-screen {target_state}s of"
          f" {len(A.lemmings(h))} live lemmings"
          + (f" at column {col}" if col is not None else ""), flush=True)
    if not cands:
        print(f"  nothing in state {target_state} is on screen")
        return 2
    with LRInput() as inp:
        inp.clear()
        inp.enable()
        try:
            if not A.select_skill(h, inp, panel):
                print(f"  panel {panel} not selected"
                      f" (0x004C9200={A.selected(h)})")
                return 2
            target = 0
            for p, _c, _n, box, _on in cands:
                if not box:
                    continue
                A.aim_at(h, inp, (box[0] + box[2]) / 2, (box[1] + box[3]) / 2)
                box = A.lemming_box(h, p)
                if box:
                    A.aim_at(h, inp, (box[0] + box[2]) / 2,
                             (box[1] + box[3]) / 2, tol=1.0, tries=6)
                got = A.hovered(h, scr)
                if got:
                    target = got
                    break
            if not target:
                print("  no lemming became hoverable")
                return 2
            c0 = B.lem_core(h, target)
            print(f"  aiming at {target:#010x} x {c0['x']:.3f}"
                  f" y {c0['y']:.3f} dir {c0['dir']:+d}"
                  f"  gate byte {gate(h):#04x}", flush=True)
            cb = A.counts(h)
            _r, _b, prev = B.whole_grid(h)
            pre = prev

            events = []           # (t, label, col, row, old, new)
            states = []           # (t, name, x, y)
            fuses = []            # (t, remaining seconds)
            n = 0
            released = False
            last_name = None
            last_rem = None
            t_flap = None
            t_blast = None
            flap_pos = None
            blast_pos = None
            crater_pos = None
            crater_prev = None
            prev_pos = None
            t0 = time.perf_counter()
            inp.button(0, True)
            while (t := time.perf_counter() - t0) < seconds:
                n += 1
                if not released and t >= 4 / 60.0:
                    inp.button(0, False)
                    released = True
                g = B.read_rows(h, base, 0, rows)
                c = B.lem_core(h, target)
                _sp, vt, name = state_vt(h, target)
                _cp, rem = countdown_of(h, target)
                if rem is not None and (last_rem is None
                                        or abs(rem - last_rem) > 0.19):
                    fuses.append((t, rem))
                    last_rem = rem
                if name != last_name:
                    states.append((t, name, c["x"] if c else None,
                                   c["y"] if c else None))
                    print(f"  t {t:+7.4f}s state -> {name:<9}"
                          + (f" x {c['x']:8.3f} y {c['y']:7.3f}" if c else "")
                          + (f"  fuse {rem:.4f}s" if rem is not None else ""),
                          flush=True)
                    if vt == VT_FLAPPER and t_flap is None:
                        t_flap = t
                        flap_pos = (c["x"], c["y"]) if c else None
                        ab = D.anim_block(h, target)
                        if ab:
                            print(f"      flapper animation: strip"
                                  f" {ab['strip']} {ab['frames']}f div"
                                  f" {ab['div']} rate {ab['rate']}"
                                  f"  => frames*0.06/div ="
                                  f" {BU.cycle_s(ab):.3f} s", flush=True)
                        if c:
                            pred = crater_cells(c["x"], c["y"], rows)
                            print(f"      flapper at ({c['x']:.3f},"
                                  f" {c['y']:.3f}); if it blew up HERE the"
                                  f" stencil would hit {len(pred)} cells"
                                  f" cols {min(p for p, _ in pred)}.."
                                  f"{max(p for p, _ in pred)} rows"
                                  f" {min(r for _, r in pred)}.."
                                  f"{max(r for _, r in pred)}", flush=True)
                    if vt == VT_BLAST and t_blast is None:
                        t_blast = t
                        blast_pos = (c["x"], c["y"]) if c else None
                    last_name = name
                if g and prev and g != prev:
                    if t_blast is not None and crater_pos is None:
                        crater_pos = (c["x"], c["y"]) if c else None
                        crater_prev = prev_pos
                    for i in range(0, len(g), 4):
                        if g[i:i + 4] != prev[i:i + 4]:
                            old = struct.unpack_from("<I", prev, i)[0]
                            new = struct.unpack_from("<I", g, i)[0]
                            cell = i // 4
                            col, r = cell % GRID_WIDTH, cell // GRID_WIDTH
                            events.append((t, name, col, r, old, new))
                            dc = ((col - trunc(c["x"]) + 128) % 256) - 128 if c else 0
                            dr = r - trunc(c["y"]) if c else 0
                            print(f"  t {t:+7.4f}s {name:<9}"
                                  f" (col {col:3d}, row {r:3d})"
                                  f"  {old:#010x} -> {new:#010x}"
                                  f"   d(col)={dc:+d} d(row)={dr:+d}",
                                  flush=True)
                    prev = g
                if c:
                    prev_pos = (c["x"], c["y"])
            if not released:
                inp.button(0, False)
            ca = A.counts(h)
            print("", flush=True)
            print(f"  {n} whole-grid samples in {seconds:.1f}s"
                  f" ({n / seconds:.0f} Hz), {len(events)} cell changes",
                  flush=True)
            print(f"  stock[{panel}] {cb[panel]} -> {ca[panel]}"
                  f"  spent={ca[panel] == cb[panel] - 1}", flush=True)
            print(f"  gate byte [{GATE_BYTE:#010x}] = {gate(h):#04x}"
                  f" after the run", flush=True)
            if t_flap is not None:
                print(f"  FUSE  click -> flapper   {t_flap:.4f} s", flush=True)
            if t_flap is not None and t_blast is not None:
                print(f"  FLAP  flapper -> blast   {t_blast - t_flap:.4f} s",
                      flush=True)
            if t_blast is not None:
                print(f"  TOTAL click -> blast     {t_blast:.4f} s", flush=True)
            if fuses:
                print("  the countdown double on +0x374, sampled:")
                for t, rem in fuses:
                    print(f"    t {t:+7.4f}s  {rem:.4f}")
            # The crater: every change that landed at or after the blast.
            crater = [e for e in events
                      if t_blast is not None and e[0] >= t_blast - 0.002]
            if crater:
                cells = sorted({(c, r) for _t, _n, c, r, _o, _nw in crater})
                vals = sorted({(o, nw) for _t, _n, _c, _r, o, nw in crater})
                print(f"  CRATER {len(cells)} cells changed at the blast")
                print(f"    cells: {cells}")
                print("    value transitions: "
                      + ", ".join(f"{o:#010x} -> {nw:#010x}" for o, nw in vals))
                print(f"    position at the state change: {blast_pos}")
                print(f"    position at the crater sample: {crater_pos}")
                print(f"    position one sample earlier:   {crater_prev}")
                explain(pre, cells, rows)
            return 0
        finally:
            inp.release()


def main() -> int:
    ap = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    ap.add_argument("--stencil", action="store_true",
                    help="dump both explosion masks out of the exe (offline)")
    ap.add_argument("--predict", nargs=2, type=float, metavar=("X", "Y"),
                    help="the cells the stencil would hit at (X, Y) (offline)")
    ap.add_argument("--rows", type=int, default=96,
                    help="grid height for --predict")
    ap.add_argument("--enter", metavar="LEVEL",
                    help="enter LEVEL by the record swap, then carry on")
    ap.add_argument("--probe", action="store_true",
                    help="read the gate byte, the stocks, the strips (no input)")
    ap.add_argument("--bomb", action="store_true",
                    help="click the bomber on a walker and watch it through")
    ap.add_argument("--panel", type=int, default=PANEL_BOMBER)
    ap.add_argument("--state", default="walk",
                    help="which state to aim at (walk, block, ...)")
    ap.add_argument("--col", type=int, default=None,
                    help="only aim at a lemming whose trunc(X) is this")
    ap.add_argument("--seconds", type=float, default=9.0)
    a = ap.parse_args()

    if a.stencil:
        return do_stencil()
    if a.predict:
        return do_predict(a.predict[0], a.predict[1], a.rows)

    h, pid = attach(write=True)
    if not h:
        print("the game is not running")
        return 1
    print(f"attached pid {pid}", flush=True)
    try:
        if a.enter:
            from measure_float_splat import enter as fs_enter
            rc = fs_enter(h, a.enter)
            if rc:
                return rc
        if a.probe:
            do_probe(h)
        if a.bomb:
            return do_bomb(h, a.panel, a.seconds, a.state, a.col)
        return 0
    finally:
        k32.CloseHandle(h)


if __name__ == "__main__":
    raise SystemExit(main())
