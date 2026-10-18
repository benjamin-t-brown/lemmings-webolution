# Phase 3 round 7, live — the fuse, the Ohno cue, and what a bomb removes

> **One citation in this note is withdrawn (added 2026-10-16).** Where it cites
> the climb series' `.190` fraction as evidence that the world X fraction is
> real, the source of that number is retracted — round 10 showed the billboard
> quad it came from is not a lemming. The *conclusion* survives on better
> evidence: actor-list X values are genuinely fractional (28.264, 36.102,
> 42.696 ...). Everything else in this note stands.

2026-10-15. Stock `C:\progs\Lemmings Revolution\Lemmings Revolution.exe`,
SHA-256 `32675bdb49866e76502713251fdfd2b09078c2481c72ca22ff68ebf4b3737a2f`,
image base `0x400000`, driven through `tools/dinput-shim`.

This tests `docs/scratch/phase3-round7.md` (the static read) and does not
repeat it. That note's **fuse is right** — 5.0 s, and a time rather than a
tick count — and its **Ohno timing is right**: the cue and the start of the
explode animation are the same instant. It is wrong in **one** place, and the
error is a file-reading one rather than a reasoning one: the gate byte at
`0x004CBB40` is **not** `0xFF`, and Ohno is **not** suppressed. See "The gate
byte" below; the short version is that the byte lives past the end of
`.data`'s raw bytes, so there is no image byte to read.

It also answers the question that note left open. **The explosion is not a
circle and there is no radius.** It is a stored 7 × 7 bitmap, and the blast is
a whole animation cycle *after* the fuse ends.

Harness: `tools/harness/measure_bomb.py` (new). Every address in it is in its
docstring.

---

## SUMMARY

| Thing | Status | Value |
| --- | --- | --- |
| **the fuse** | **READ + MEASURED, 6 of 6** | **5000 ms.** `push 5` at `0x0045B1B3`, `fild`'d to a double at `0x00413D74`, × the dword 1000 at `0x004657B2`. Measured click → flapper: 4.9967, 4.9985, 4.9981, 4.9976, 4.9998, 4.9980 s. `bomberFuse = 5 * 5` ticks → **`bomberFuseMs = 5000`**. |
| **the fuse really is a clock** | **MEASURED** | the countdown object's own double, read live off the lemming's `+0x374` list, fell **4.9940 → 0.1330 over 4.8616 s** — 1.000 s per second, in ~0.195 s steps. This closes the static note's "the path to `0x00413F15` was not traced". |
| **Ohno's moment** | **MEASURED, 6 of 6** | the **end** of the fuse, not one second before it. The 5000 ms event is one function (`0x00414370`) that plays the sample **and** installs the flapper, so the cue and the explode animation start together. Our `fuse == ticksPerSecond` cue was a second early **by construction** and is gone. |
| **the gate byte** | **MEASURED** | `[0x004CBB40]` = **`0x00`** on both levels, before and after every bomb. `test edx, edx / jne` at `0x00414426` means zero is the arm. **Ohno always plays.** No quirk to preserve. |
| **the blast is not the fuse** | **MEASURED, 6 of 6** | the fuse installs the **flapper** (`0x004B0A58`, animation `explode`); the crater comes **one whole animation cycle later**: 1.0211, 1.0193, 1.0226, 1.0206, 1.0191, 1.0201 s. Read: strip **6, 17 frames, divisor 1.0** → `17 × 0.06 / 1.0` = **1.020 s**. New `Rules::exploderCycleMs = 1020`. |
| **total** | **MEASURED** | **6.018–6.021 s** from the click to the crater. |
| **the footprint** | **READ + MEASURED, 5 exact set matches** | a stored **7 × 7 bitmap at `0x004B1528`**, 37 of 49 bytes set, anchored at `col0 = trunc(X − 3.5)`, `row0 = trunc(Y + 1.5 − 3.5)`. **`explosionRadius = 5` and `clearCircle` are both wrong** and both are gone; `Terrain::clearExplosion` is the stencil. |
| **the value written** | **MEASURED, 75 of 75 cells** | `0x00000001` → `0x00000000`. `mov dword ptr [edx], 0` at `0x0043F7F5` — the whole dword, the same instruction shape as round 5's bite. |
| **the drop at the install** | **MEASURED, 4 of 4** | the world Y is **one unit lower** in the blast state than in the flapper (70→69, 78→77, twice each). `0x00416B5B`, after `0x0042E696`. It is why the crater straddles the feet row symmetrically. |
| **a primed lemming keeps its state** | **MEASURED** | the countdown's `[this+0x11]` is 1, so `0x0042E696` pushes it onto `+0x374` rather than replacing `+0x340`. A primed walker went X 144.770 → 119.770 — exactly **25.000** columns — and a primed blocker went on blocking for the whole five seconds. |
| **a bombed blocker's field** | **MEASURED, 2 of 2** | cleared **3.7 ms and 4.2 ms after the flapper arrived**, i.e. at the END of the fuse, **1.02 s before** the crater. Round 8 saw the same 2 ms gap and the same one-second lag; this round pins which of the two moments it belongs to. |
| **nothing outside the window** | **MEASURED, 6 of 6** | every run diffed the whole 256 × 96 grid; the only cells that ever moved were inside the 7 × 7 and (once) two neighbouring blockers' own four-cell fields. |
| **panel index 2 is the bomber** | **independently confirmed** | `MM_BOMB` ships `[0,0,20,0,0,0,0,0,0,1]` and the engine pre-selects index 2. Round 8 measured this from the state and the crater; this is the stock table saying it. |

Re-run. `--stencil` and `--predict` are offline; `--probe` sends no input:

```powershell
python tools/harness/measure_bomb.py --stencil
python tools/harness/measure_bomb.py --predict 170.066 77.0

# restart the process before each entry; there is no route back to the map
python tools/harness/measure_bomb.py --enter MM_BOMB --probe
python tools/harness/measure_bomb.py --bomb --seconds 9

python tools/harness/measure_bomb.py --enter MM_Block_Bomb_A --probe
python tools/harness/measure_bomb.py --bomb --seconds 9
python tools/harness/measure_bomb.py --bomb --state block --seconds 9
python tools/harness/measure_bomb.py --bomb --state block --col 115 --seconds 9
```

`--enter` is the only thing that writes to the game, and its only write is
`goto_level`'s level-record swap, which is put back (`slot [0][0] restored:
True` on all four entries this session). **No lemming byte and no grid cell
was written.** The skill went on through the panel every time and the engine
spent a unit of stock on every click (6 of 6, `spent=True`).

---

## The chain, four objects deep

The static note had the first three of these. The fourth is the one that
matters for the terrain, and it is a whole second after the moment the note
(and our sim) treated as the explosion.

| # | object | vtable | what it does | when |
| --- | --- | --- | --- | --- |
| 1 | countdown | `0x004B0990` | holds the double 5.0, subtracts the frame's slice | the click |
| 2 | the 5000 ms event | helper `0x004B09A8`, step `0x00414370` | plays `Ohno`, installs the flapper | +5000 ms |
| 3 | flapper | `0x004B0A58`, enter `0x0041695C` | animation `explode`, and falls while it flaps | +5000 ms |
| 4 | **blast** | `0x004B0550`, ctor `0x004081B0`, step `0x00408227` | **the crater** | +6020 ms |

The bomb factory is entry `[2]` of the skill table at `0x0045B570`
(`0x0045B197`), confirming panel index 2 from the code as well as from the
stock table.

Live, one run end to end (run 6, `MM_BOMB`, a right-to-left walker):

```
  aiming at 0x027c4300 x 144.770 y 70.000 dir -1  gate byte 0x00
  t +0.0000s state -> walk      x  144.770 y  70.000
  t +4.9980s state -> flapper   x  119.770 y  70.000
      flapper animation: strip 6 17f div 1.0 rate 0.0  => 1.020 s
  t +6.0181s state -> blast     x  119.770 y  69.000
  t +6.0197s blast  12 cells (col 116..122, rows 68..69)  0x1 -> 0x0
  FUSE  click -> flapper   4.9980 s
  FLAP  flapper -> blast   1.0201 s
  TOTAL click -> blast     6.0181 s
```

`144.770 − 119.770 = 25.000` columns in five seconds: the primed walker keeps
walking at the measured `walkSpeed`, because the countdown is on the `+0x374`
list and not in `+0x340`.

### The countdown double, sampled live

Run 1, off the object `countdown_of` finds on the lemming's own list:

```
    t +0.0021s  4.9940      t +2.5296s  2.4660
    t +0.1973s  4.7990      t +2.9191s  2.0770
    t +0.5849s  4.4110      t +3.5036s  1.4930
    t +0.9741s  4.0220      t +4.0872s  0.9090
    t +1.5573s  3.4380      t +4.6691s  0.3260
    t +1.9461s  3.0490      t +4.8637s  0.1330
```

4.861 of fuse in 4.8616 s of wall clock, in steps of 0.194–0.195. Observation:
the fuse is decremented once per engine tick by that tick's own dt, which is
what `0x00413F53`'s `fsub qword ptr [ecx]` says it should be. The static note
could not establish that the double was being decremented at all; it is.

---

## The footprint — a stencil, not a radius

`0x00408227` (the blast's step) is the only thing on this path that touches
the grid, and it does it through one call:

```
00408273  rep movsd                        ; pos = the lemming's x, y, z
00408287  fild [0x4b10bc] (2) ; fdiv 2.0   ; pos.x += 1.0
00408299  fild [0x4b10b8] (3) ; fdiv 2.0   ; pos.y += 1.5
004082b6  call 0x43b31e                    ; the VISUAL, on (x+1, y+1.5)
004082bb  fsub [0x4b02d0] (1.0)            ; pos.x back to the lemming's x
004082d7  call 0x43f510                    ; the TERRAIN, second arg 0
```

and `0x0043F510` is a bitmap walk:

```
0043f530  cmp dword ptr [ebp + 0xc], 0
0043f536  mov [ebp-0x20], 7  ; mov [ebp-0x1c], 0x4b1528   <- arg2 == 0
0043f546  mov [ebp-0x20], 9  ; mov [ebp-0x1c], 0x4b155c   <- arg2 != 0
0043f557  fild [ebp-0x20] ; fdiv 2.0 ; fsubr [eax+8]      ; row0 = y - n/2
0043f57b  fild [ebp-0x20] ; fdiv 2.0 ; fsubr [ecx]        ; col0 = x - n/2
0043f5e9  mov al, [edx] ; add [ebp-0x1c], 1               ; ONE mask byte
0043f5fb  je  0x43f7fe                                    ;   0 -> skip
0043f60e  call 0x40de60                                   ; the cell
0043f61b  and edx, 2     ; jne skip        (indestructible)
0043f62b  and ecx, 0x200 ; jne skip        (the zone bit)
0043f63e  and eax, 0x800 ; jne skip        (a blocker's field)
0043f650  and edx, 1     ; je  0x43f7f5    (air: zero it anyway)
0043f65e/672  col & 3 == 0 and row & 3 == 0 -> a debris object
0043f7f5  mov dword ptr [edx], 0                          ; THE REMOVAL
0043f804  and eax, 0xff                                   ; the column wraps
```

The mask pointer advances once per inner iteration and is never reset, so the
bitmap is row-major: outer loop = grid row (counting up), inner = column. The
7 × 7 at `0x004B1528`, straight out of `.rdata`
(`measure_bomb.py --stencil`):

```
  . . # # # . .
  . # # # # # .
  # # # # # # #        37 of 49 bytes set
  # # # # # # #
  # # # # # # #
  . # # # # # .
  . . # # # . .
```

The 9 × 9 at `0x004B155C` (61 of 81) is the **other** caller's,
`0x0041DF89`, which pushes 1. That caller is an object with its own countdown
at `[+0xa0]`, not a lemming, so it is not the skill bomb. Named here so nobody
later picks the bigger one.

### Measured: five exact set matches

The test is falsifiable and was run as one. For each candidate anchor
`(col0, row0)` in a window, `measure_bomb.explain` computes the cells the
stencil *would* change — stencil cells whose word in the pre-click grid
snapshot was non-zero and carried none of `0x2` / `0x200` / `0x800` — and
compares that as a **set** with what actually changed.

| run | level | state bombed | blast (X, Y) | cells | anchor that matches exactly |
| --- | --- | --- | --- | --- | --- |
| 1 | MM_BOMB | walk | (112.921, 69.0) | 12 | **(109, 67)** |
| 2 | MM_BOMB | walk | (179.251, 49.0) | 10 | **(175, 47)** |
| 3 | MM_Block_Bomb_A | walk | (166.388, 77.0) | 13 | **(162, 75)** |
| 4 | MM_Block_Bomb_A | block | (88.158, 77.0) | 15 | **(84, 75)** |
| 6 | MM_BOMB | walk | (119.770, 69.0) | 12 | **(116, 67)** |

and the formula, applied to those positions:

```
  col0 = trunc(X - 3.5)      112.921 -> 109   179.251 -> 175   166.388 -> 162
                              88.158 ->  84   119.770 -> 116
  row0 = trunc(Y + 1.5 - 3.5) = trunc(Y - 2)   69 -> 67   49 -> 47   77 -> 75
```

Five of five, exactly. 75 cells across the six runs, **every one
`0x00000001` → `0x00000000`**, no other transition anywhere.

Run 6 printed the whole window, which is the clearest single piece of
evidence in the round — the stencil's shape in the pre-click words, with `*`
on each cell that went:

```
  row  73      -          -     0x00000000  0x00000000  0x00000000       -          -
  row  72      -     0x00000000  0x00000000  0x00000000  0x00000000  0x00000000       -
  row  71 0x00000000  0x00000000  0x00000000  0x00000000  0x00000000  0x00000000  0x00000000
  row  70 0x00000000  0x00000000  0x00000000  0x00000000  0x00000000  0x00000000  0x00000000
  row  69 0x00000001* 0x00000001* 0x00000001* 0x00000001* 0x00000001* 0x00000001* 0x00000001*
  row  68      -     0x00000001* 0x00000001* 0x00000001* 0x00000001* 0x00000001*      -
  row  67      -          -     0x00000000  0x00000000  0x00000000       -          -
  stencil cells that did not change: 25, of which 0 held a non-zero word
```

### Observation, and it matters for our integer X: the anchor is fractional

`trunc(X − 3.5)` is `trunc(X) − 4` when the fraction of X is below 0.5 and
`trunc(X) − 3` above it, so **the stencil's centre column is the lemming's own
column only about half the time**. Measured on four fractions:

| X | fraction | col0 | centre (col0 + 3) | vs `trunc(X)` |
| --- | --- | --- | --- | --- |
| 112.921 | .921 | 109 | 112 | `trunc(X)` |
| 179.251 | .251 | 175 | 178 | `trunc(X) − 1` |
| 166.388 | .388 | 162 | 165 | `trunc(X) − 1` |
| 88.158 | .158 | 84 | 87 | `trunc(X) − 1` |
| 119.770 | .770 | 116 | 119 | `trunc(X)` |

Our sim has no fraction — `l.x` *is* `trunc(X)` — so `Terrain::clearExplosion`
applies the original's own formula to the X it has, which gives
`col0 = l.x − 4`. That is the three-of-five case and it is the truncation this
project already uses everywhere else. **Stated rather than tuned**: an X whose
fraction is above 0.5 blows one column further right in the original than in
our sim.

### Vertically it is symmetric, and that is lucky

`row0 = trunc(Y_blast − 2)` with `Y_blast = Y_feet − 1` (the install's drop),
so in the engine's Y-up grid the rows are `Y_feet − 3 … Y_feet + 3`: the
stencil straddles the feet row. And the bitmap is **symmetric top to bottom**
(row 0 = row 6, row 1 = row 5, row 2 = row 4), so `Terrain`'s downward Y needs
no flip and the single most repeated mistake in this project is not available
here.

### The three skipped bits

READ at `0x0043F61B` / `0x0043F62B` / `0x0043F63E`. One of them was probed
live and the result is honest but inconclusive:

Run 5 bombed a blocker at column 115 with two more blockers at 112 and 113,
chosen so their `0x800` field cells sat **inside** the stencil
(`--col 115`). Those cells did go to zero — but so did `(112, 81)` and
`(113, 81)`, which are **outside** the stencil, and each of the two
neighbours lost **all four** of its cells. The crater had just taken the floor
out from under both of them, which drops a blocker out of the block state and
runs its exit hook (round 8). So:

```
    anchors whose stencil predicts the observed 21 changes EXACTLY: []
    nearest is (col0 111, row0 75), 8 cells of symmetric difference:
      predicted-not-seen []
      seen-not-predicted [(112,78) (112,79) (112,80) (112,81)
                          (113,78) (113,79) (113,80) (113,81)]
```

**Every cell the stencil predicted was seen**, and the extras are exactly two
complete four-cell fields. That is consistent with the `0x800` skip and with
nothing else being wrong — but the blast's write and the dying blockers'
erase landed inside one 1.5 ms sample, so this run **cannot separate them**.
Recorded as consistent-with-the-read, not as a measurement. `0x2` and `0x200`
were not exercised: no level in reach puts indestructible or zone cells inside
a bombable lemming's window.

---

## The gate byte — the static note's one error, and it is a file-offset one

The static note says the byte at `0x004CBB40` is `0xFF` in the image and that
nothing in `.text` stores it, so Ohno might never play. **The second half is
right and the first half is not.** The section headers:

```
  .text    va 0x00401000  vsize 0xae430    raw 0x1000    rawsize 0xaf000
  .rdata   va 0x004b0000  vsize 0xe1cd     raw 0xb0000   rawsize 0xf000
  .data    va 0x004bf000  vsize 0x1185b8   raw 0xbf000   rawsize 0x7000
  .rsrc    va 0x005d8000  vsize 0x9d3c     raw 0xc6000   rawsize 0xa000
```

`.data`'s virtual size is `0x1185B8` and its raw size is `0x7000`, so the file
only supplies `0x004BF000 … 0x004C6000`. `0x004CBB40` is **`0x5B40` past the
end of that**: it is in the uninitialised tail, the loader zero-fills it, and
the file byte the note read at offset `0xCBB40` is not this variable at all.

Live, `--probe` on both levels, before and after every bomb:

```
  the Ohno gate byte [0x004cbb40] = 0x00   -> Ohno IS played at 0x00414420
  sound ids: Ohno [0x004e32cc] = 7   blast [0x004e32c0] = 4
```

`test edx, edx` / `jne` at `0x00414426` skips the play when the byte is
**non**-zero, so zero is the arm. Nothing stores it, so it stays zero.
**Ohno always plays, and there is no quirk here to preserve.** (The same
section arithmetic explains the note's other "past the raw file" remark about
`0x004E32CC`: that is live heap-adjacent data, and `--probe` reads the id out
of it — 7 for Ohno, 4 for the blast, two different sounds.)

The gate's other reader, `0x0043B38D` inside `0x0043B31E`, is the **visual**
half of the blast — it builds a rectangle from `(X−5.5, Y−4.5)` to
`(X+2.5, Y+3.5)` and substitutes the double 3.0 for 20.0 when the byte is
set. Whatever it is, it is a presentation switch and it touches no cell.

---

## What moved in the tree

### `Rules`

| field | before | after | why |
| --- | --- | --- | --- |
| `bomberFuse` | `5 * 5` ticks | **gone** | the engine holds a 5 and a 1000, never a 25 |
| `bomberFuseMs` | — | **5000** | `0x0045B1B3` / `0x00413D74` / `0x004657B2`; six live fuses of 4.997–5.000 s |
| `exploderCycleMs` | — | **1020** | `explode` is strip 6, 17 frames, divisor 1.0; six live flaps of 1.019–1.023 s |
| `explosionRadius` | 5 | **gone** | it is not a circle. The footprint is the stencil at `0x004B1528` |

### `Terrain`

`clearCircle(cx, cy, radius)` is **deleted** and replaced by
`clearExplosion(x, y)`: the 49 stencil bytes are written out in `Terrain.cpp`
(they are stored data, not a formula), the window is columns `x−4 … x+2` and
rows `y−4 … y+2`, steel is skipped (`0x0043F61B`) and so is a cell carrying a
blocker's field (`0x0043F63E`). The `0x200` zone bit has no counterpart in
this `Terrain` and is noted rather than faked.

### The stepper

`State::Exploder` existed and was never entered. It is now the flapper:

- `step()`'s fuse block counts **milliseconds**, and at zero it cues `OhNo`,
  calls `leaveBlocker` (the measured erase moment) and enters `Exploder`.
  The old `if (l.fuse == rules_.ticksPerSecond) cue(OhNo)` is gone — it was a
  second early by construction and had no citation.
- `stepExploder` waits out `exploderCycleMs`, then drops Y by one (measured,
  `0x00416B5B`) and explodes.
- `explode()` is now only the blast: the stencil, `Dead`, and the `Explode`
  cue. `leaveBlocker` moved out of it, to the fuse's end where it was
  measured.
- `assign`'s bomber arm refuses on `fuseMs >= 0`, so a lemming that has
  already blown cannot be re-primed while it flaps.

Three within-tick simplifications, stated the way round 6 stated its three:

1. **Our tick grid is 200 ms and the blast is at 1020 ms into the flap**, so
   we act on the sixth tick — 1200 ms — up to one tick late. Same shape as
   round 6's shrug (1000 ms against a measured 966).
2. **The fuse divides exactly**: 5000 / 200 = 25 ticks, so `OhNo` lands on
   the right tick to the millisecond. That is the one place in this round
   where the 200 ms grid costs nothing.
3. **The flapper's fall is implemented but never measured.** `0x0041699E`
   tests the cell below and calls `0x00416038` — the faller's own speed
   accessor — on air. All six bombs were on solid ground, so this arm has no
   live series; it is in `stepExploder` labelled READ, because without it a
   lemming bombed in mid-air would hang for six ticks, which nothing in the
   original does.

### Tests

`test_world` gained `testBomberFuseAndCrater` and
`testBombedBlockerLosesItsFieldAtTheFuse`; `testSkillRules` gained the two
bomber fields; `testSoundCues`'s bomber section was rewritten. **Every
expectation is written out, not derived from `Rules`** — the old
`testSoundCues` computed its tick numbers from `bomberFuse` and
`ticksPerSecond` and *therefore could not fail* when the cue was in the wrong
place, which is exactly the trap round 4 named.

`test_terrain` gained the stencil's shape, the wrap, and the blocker-field
skip.

**Verified the way the facilitator verified rounds 4, 5, 6 and 8.** With
`bomberFuseMs` back to 25 ticks, `exploderCycleMs` 0, the old fuse block
(`--l.fuseMs`, `OhNo` at `== ticksPerSecond`, `explode()` at 0, no flapper),
`explode()` back on `(l.x, l.y - lemmingHeight/2)` and `clearExplosion` a
radius-5 circle, **`test_world` fails 17 of its 232 checks** and **15 of the
17 are behavioural**, not field asserts:

```
  FAIL  the fuse is 5000 ms (5 x 1000, not 25 ticks)              <- field
  FAIL  the explode strip is 17 animation frames of 60 ms         <- field
  FAIL  oh-no on tick 25 (was 20)
  FAIL  and the explode animation starts on the same tick (was -1)
  FAIL  not one second early, which is where it used to be
  FAIL  the blast on tick 31 (was 25)
  FAIL  and the terrain is untouched until then (first change on tick 25)
  FAIL  it walked through the whole fuse and stopped where it blew (141 vs -1)
  FAIL  15 cells of floor went (got 26)
  FAIL  row 80 is cleared seven wide, bombX-4 to bombX+2
  FAIL  row 81 is five wide, inset by one -- the stencil's . # # # # # .
  FAIL  row 82 is three wide -- . . # # # . .
  FAIL  and it dies on tick 31, six ticks later (was 25)
  FAIL  which is the 1020 ms explode animation between them
  FAIL  quiet for the first 24 ticks of the fuse
  FAIL  oh-no on tick 25, at the end of the fuse
  FAIL  and the blast on tick 31
```

and `test_terrain` fails **7 of its 35**:

```
  FAIL  the blast stencil is 37 cells (got 81)
  FAIL  and its corners are not
  FAIL  nor is the column either side of the window
  FAIL  nor the row above or below it
  FAIL  a blocker's field cell survives (got 81)
  FAIL  the guarded cell is still there
  FAIL  and stops there
```

Restored, the suite is **16 of 16**, `test_world` 232 checks / 0 failures and
`test_terrain` 35 / 0.

```powershell
.\scripts\Invoke-Ucrt64.ps1 "make -C src test"
```

---

## Clean negatives, and what was searched for

- **No radius anywhere on the path.** The countdown's step, the event helper,
  the flapper's enter and step, the blast's enter and step were all read in
  full. The only grid-touching call is `0x0043F510`, and its size comes from
  the `mov [ebp-0x20], 7` / `9` pair, not from a `.rdata` double. There is no
  number 5 and no number 9 to promote, and a circle of radius 5 would clear
  81 cells where the original clears 37.
- **Nothing outside the window, in any of six whole-grid diffs.** The only
  exception is run 5's two neighbouring blocker fields, accounted for above.
- **No cell value other than `0x00000001` → `0x00000000`**, across 75 crater
  cells. No partial mask, no `or`, no bit left standing: `mov dword ptr
  [edx], 0`.
- **The 9 × 9 mask is excluded for a lemming**, by the call site, not by the
  diff: `0x004082D1` pushes 0 and `0x0041DF80` pushes 1, and only the first is
  on a lemming's path.
- **The gate byte is not a mute.** Searched: both `.text` readers
  (`0x00414420`, `0x0043B38D`), and no store exists — which with a BSS
  address means it is zero for the life of the process. Live, 0x00 on two
  levels and four `--probe` reads.
- **`0x2` and `0x200` were not exercised live.** Stated rather than implied:
  no reachable level puts an indestructible or zone cell inside a bombable
  lemming's 7 × 7. The skips are reads.
- **Two levels, two bombed states.** `MM_BOMB` (stock `[0,0,20,…]`) and
  `MM_Block_Bomb_A` (`[0,0,15,15,…]`), walkers and blockers, three processes.
  Both levels agreed on the fuse, the flap and the anchor formula.

---

## Harness notes

- **`--bomb` presses the button from inside the diff loop.** `assign_skill`'s
  `assign` sleeps about 1.4 s for the double-click window, and the fuse is
  five seconds, so the click and the crater have to share one clock or the
  numbers are a sleep plus a read. One iteration reads the grid, the
  lemming's core, its state pointer and the countdown double — ~650 Hz, with
  nothing written but the mouse button.
- **The countdown is reachable live.** `countdown_of` walks the 0xC-byte node
  chain at `lemming+0x374` and accepts a node whose value has vtable
  `0x004B0990`. That is how the fuse became a *series* rather than a single
  interval.
- **A level ends on you.** One run came back `0 on-screen walks of 0 live
  lemmings` and the next probe read `ui_state 5`: `MM_Block_Bomb_A`'s 600 s
  clock, with most of its lemmings bombed. Restart; do not try to leave a
  results screen.
- **Aiming at a specific column works when the engine's pick cooperates.**
  `--col N` filters candidates by `trunc(X)`, which is how a blocker with
  neighbours inside its own blast window was selected. No arrow-key filter was
  needed this round — all six aims landed first try, probably because
  `MM_BOMB` has three lemmings on screen rather than twenty.
- `lrstate.attach` returns `(handle, pid)`. Still true.
- `goto_level.focus` was never called. Entry is `measure_float_splat.enter`
  (`select_level` + `restart_level.ensure_playing`), bracketed by the slot
  `[0][0]` restore, four times, all four `True`.

---

## Left the tree

| file | change |
| --- | --- |
| `tools/harness/measure_bomb.py` | **new.** `--stencil` / `--predict` offline; `--probe` read-only; `--bomb [--state S] [--col N]` is the series above. Every address in the docstring. It teaches `assign_skill`'s state map and `measure_dig`'s vtable map about the flapper and the blast at runtime rather than editing those files. |
| `src/sim/Terrain.h` | `clearCircle` → `clearExplosion(x, y)`, with the stencil, the anchor and the three skipped bits in the comment. |
| `src/sim/Terrain.cpp` | the 49 stencil bytes written out, the window, the steel and blocker-field skips, column wrap. |
| `src/sim/World.h` | `Rules`: `bomberFuse` and `explosionRadius` out; `bomberFuseMs = 5000` and `exploderCycleMs = 1020` in, with the addresses and the six series. `Lemming::fuse` → `fuseMs`. `stepExploder` declared. The round-7 comment block. |
| `src/sim/World.cpp` | the fuse block counts ms and ends in `OhNo` + `leaveBlocker` + `Exploder`; `stepExploder` added; `explode()` is the stencil only; `assign` and `nuke` use `fuseMs`. |
| `src/__test__/test_world.cpp` | `testBomberFuseAndCrater`, `testBombedBlockerLosesItsFieldAtTheFuse`, two rows in `testSkillRules`, `testSoundCues`'s bomber section rewritten with written-out ticks. |
| `src/__test__/test_terrain.cpp` | the stencil's shape, the seam wrap, the blocker-field skip; `clearCircle` references gone. |
| `src/main.cpp` | one line: `l.fuse` → `l.fuseMs` for the primed-lemming tint. |

Not touched: `fallSpeed`, `floatSpeed`, `floatAfter`, `splatDistance`,
`walkSpeed`, `climbSpeed`, `ticksPerSecond`, `maxStepUp`, `lemmingHeight`,
`blockerFieldHeight`, round 5's cycle and footprint fields, round 6's
`brickLength` / `builderBricks` / `builderCycleMs` / `builderBrickPercent` /
`builderEnterPercent` / `shrugCycleMs`, `World::spawn`, `findHatchRecords`,
`Terrain::blockerField` and the walk probe's field test,
`src/assets/LevelStats.cpp`, `worldToMaskPixel`, `test_corpus`'s numbers,
`test_collisiongrid`, `PLAN.md`, `docs/PHASE3_HANDOFF.md`, and
`docs/scratch/phase3-round7.md`. The four open stepper differences
(step-down before falling, climber top-out, hoist hold, delayed float
halving) are untouched. Round 9 was not started.

## Left the game

**Killed.** No process is running at the end of this session; `Get-Process`
for `*Lemming*` returns nothing. Four processes were used — pid 48372
(`MM_BOMB`, runs 1 and 2), 30160 (`MM_Block_Bomb_A`, runs 3 and 4), 45132
(`MM_Block_Bomb_A`, run 5 and the four blocker placements), 36612
(`MM_BOMB`, run 6) — each started by `dump_grids.restart_game` and killed
before the next entry. Four entries, four `slot [0][0] restored: True`. One
process was found at `ui_state 5` and answered with a restart; **no ESC was
pressed from a results screen.**

The shim is installed and inert (`dinput.dll` in the game folder, `enable`
left at 0 by `LRInput.release()`). A fresh launch comes up at `ui_state 3`,
the main menu.

## For round 9, and whoever lands PLAN.md

- **`0x0043F510` is the explosion, and its tail is unread.** Past the removal
  it allocates `0x748`-byte objects every fourth column and row
  (`col & 3 == 0 && row & 3 == 0`) and pushes them onto `0x004C88A8` — the
  flying debris. `0x0043B0A1` is then called with the window rectangle, which
  is probably the surface-mesh rebuild. Neither is simulation.
- **`0x0043B31E` is the blast's visual half** and is the second reader of the
  gate byte. It takes a rectangle `(X−5.5, Y−4.5) … (X+2.5, Y+3.5)` — an
  8 × 8 box, not the 7 × 7 — and ends in `0x00441E26` on the game object with
  a double that the gate byte switches between 20.0 and 3.0. A rendering
  round will want it; it touches no cell.
- **The `0x200` zone bit now has a third sighting.** The float/splat round
  found `and 0x200` in the landing path (`0x0041662B`); this round finds it
  as an explosion exemption (`0x0043F62B`). Two independent behaviours off
  one bit `CollisionGrid.h` names and ignores — a good entry point for the
  gadget round.
- **The nuke is still unidentified.** Round 8's negative on panel index 9
  stands. Our `World::nuke()` lights every fuse at once, which is now
  5000 ms + 1020 ms per lemming, but nothing live has been shown to do that.
- **The flapper falls and we have never seen it.** A bomb on a lemming over a
  hole would measure it in one run, and `0x00416A8D`'s use of the faller's own
  accessor says the step should be `fallSpeed`.
- **The fractional anchor is the one place our integer X loses information.**
  `trunc(X − 3.5)` is column-sensitive to the fraction, and the climb series
  already showed the fraction is real and non-zero (`.190`). This is the
  second time a fraction has decided a cell (the first was round 8's turn
  edges). If a future round adds sub-cell X, this is one of its tests.
