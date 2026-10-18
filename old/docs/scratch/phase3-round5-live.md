# Phase 3 round 5, live — what bash, dig and mine remove, and when

2026-10-15. Stock `C:\progs\Lemmings Revolution\Lemmings Revolution.exe`,
SHA-256 `32675bdb49866e76502713251fdfd2b09078c2481c72ca22ff68ebf4b3737a2f`,
image base `0x400000`, driven through `tools/dinput-shim`.

This builds on `docs/scratch/phase3-round5.md` (the static read) and does not
repeat it. **It does not contradict it anywhere.** Everything that note calls
an observation held live; three things it left in "Still unread" are now read
(`[+0x40]` / `[+0x44]`, the frame counts, and the cycle length), and one
sentence of its bash section was an incomplete reading of the column loop,
corrected under "Bash" below with the disassembly that settles it.

Harness: `tools/harness/measure_dig.py` (new). Every address in it is in its
docstring.

---

## SUMMARY

| Thing | Status | Value |
| --- | --- | --- |
| **what a removal writes** | **MEASURED, 38 of 38 cells** | a whole dword 0. `0x21 -> 0x00000000`, `0x5 -> 0x00000000`, `0x1 -> 0x00000000`. **Not** a mask, so `and 0xf77c` is excluded by observation as well as by having no caller. |
| **dig footprint** | **MEASURED, 4 diggers** | 3 cells: columns `trunc(X)-1, trunc(X), trunc(X)+1`, in the row **below** the feet (live grid row `trunc(Y)-1`, Y-up). Then Y steps one row down. Four cycles each, 12 cells each. |
| **dig timing** | **MEASURED** | one cycle **1.2005 s** -- twelve gaps across four diggers, every one in 1.196-1.202 s = **6.00 ticks**. Bite at animation percent **25**, Y step at **30**. |
| **bash footprint** | **MEASURED, 2 bashers, both facings** | the leading column only — `trunc(X) + facing` — **4 rows**, from the feet row upward, cleared at percents 12, 16, 60, 64 in row order +0, +1, +3, +2. One new column per cycle. |
| **bash timing** | **MEASURED** | one cycle **1.440 s** -- gaps 1.438, 1.437, 1.445 against the wall and 1.436 on the staircase = **7.20 ticks**. |
| **mine footprint** | **MEASURED, 1 miner, 4 cycles** | columns `+1` and `+2` ahead, rows feet−1 … feet+3, **minus the one cell the engine skips** (arg 2 with dx 1). Then X +1 column and Y −1 row per cycle. |
| **mine timing** | **MEASURED** | one cycle **1.4997 s** -- gaps 1.496, 1.501, 1.500 and, through a second window on the same miner, 1.499, 1.501, 1.501 = **7.50 ticks**. |
| **the clock** | **READ + MEASURED** | `frames * 0.06 s`. 0.06 is the double at **`0x004B1940`**; the frame count is `[strip]` via `0x0045B8F0`. Read live off the controller: bash **24**, dig **20**, mine **25** frames. 24·0.06 = 1.44, 20·0.06 = 1.20, 25·0.06 = 1.50 — all three measured cycles, to 0.3%. |
| **"interval in ticks" does not survive** | **MEASURED** | 6.00, 7.20, 7.50 ticks. Only dig is a whole tick count. `Rules` now carries milliseconds. |
| **the four unread cell bits** | **READ; permissive half measured** | `0x4` / `0x8` = no bash or mine to the right / left; `0x10` / `0x20` = no dig upright / inverted. `kCellUnreadBits` is now 0. |
| **why a digger "gives up after one tick"** | **EXPLAINED + MEASURED** | it is refused by bit `0x2`, at animation percent 25, which is 0.30 s into the cycle. The assign round's 0.271 s is that number. |
| panel index 5 = basher, 6 = miner | **MEASURED, one level each** | MM_BASH_002 ships `[0,0,0,0,0,10,0,0,0,1]`, MM_MINE `[0,0,0,0,0,0,10,0,0,1]`. |

Re-run. Offline first; it needs no process and it predicts the live result:

```
python tools/harness/measure_dig.py --predict MM_DIG_000  --row 58
python tools/harness/measure_dig.py --predict MM_BASH_002 --row 24
python tools/harness/measure_dig.py --predict MM_MINE
```

Live, one process per level (there is still no route from inside a level back
to the map):

```
# restart the process, then
python tools/harness/measure_dig.py --enter MM_DIG_000 --probe
python tools/harness/measure_dig.py --skill 7 --rows 0 --repeat 2 --seconds 4
python tools/harness/measure_dig.py --skill 7 --refuse --seconds 2
# restart the process, then
python tools/harness/measure_dig.py --enter MM_BASH_002 --skill 5 --any --rows 0 --repeat 2 --seconds 5
# restart the process, then
python tools/harness/measure_dig.py --enter MM_MINE --skill 6 --any --rows 0 --repeat 2 --seconds 5
```

`--enter` is the only thing that writes to the game, and the only write is
`goto_level`'s level-record swap, which is put back (`slot [0][0] restored:
True` on all five entries this session). **No lemming byte was written and no
grid cell was written.** The skill went on through the panel every time, by
`assign_skill.py`'s primitives, and the engine spent a unit of stock on every
click.

---

## The method, and why it was cheap

Phase 2 proved the engine collides against the stored 256×96 `u32` grid and
that the live grid is byte-identical to the `.LVL` bytes. The static note
proved the removal is a store through `0x0040DE60`'s return. Put together,
**the removal is directly observable**: snapshot the grid, click, diff it per
sample. No inference, no instrumentation, nothing written.

`measure_dig.sample` does four `ReadProcessMemory` calls an iteration — the
state pointer, the state block, the lemming's own `0x40` bytes from `+8`, and
a window of grid rows — and prints a row whenever the state, the animation
percent, the integer world Y or **any cell in the window** changes. With a
14-row window it runs at **4,200–4,700 Hz**; with the whole 96-row grid
(98,304 bytes a sample) at **630–680 Hz**. Both are far faster than the
animation, which advances one percent per ~13.8 ms, so no removal can hide
between samples. Every run also diffs the whole grid before against the whole
grid after, and reports any change outside the window; there never was one.

The second half of the method was predicting before clicking. `candidates()`
applies the static read to the live grid — the three cells `0x00415A43` would
probe and whether `0x2` or the direction mask refuses them — and prints
`CLEAR` or `REFUSE` per lemming. The brief's series is then a test of the
read, not a fishing trip.

---

## The offline prediction that made MM_DIG_000 make sense

Observation, from the shipped `.LVL` bytes, no process:

```
MM_DIG_000   cell word     count    dig up  dig inv  bash R  bash L
             0x00000000    15928    clear   clear    clear   clear
             0x00000003     7732    REFUSE  REFUSE   REFUSE  REFUSE
             0x00000021      876    clear   REFUSE   clear   clear
             0x00000082       40    REFUSE  REFUSE   REFUSE  REFUSE
```

**Every solid cell on the dig level is either `0x3` or `0x21`. Not one is a
plain `0x1`.** `0x3` is solid|indestructible and no skill may touch it;
`0x21` is solid|`0x20`, and `0x20` is the bit dig refuses only when
`[lemming+0x44] == -1`, so an upright digger may clear it. The `0x21` cells
are four-row slabs at rows 22–25, 39–42, 55–58 and 71–74, in column spans:

```
 row 55..58: [(29, 66)]            row 71..74: [(6, 89)]
 row 39..42: [(6, 36), (61, 89)]   row 22..25: [(30, 66)]
```

So the level is diggable in those spans and nowhere else, and the prediction
before any click was: a digger in `29..66` at feet row 59 digs; a digger
anywhere else gives up with **no grid change at all**. Both halves were then
observed.

---

## Dig — the series

MM_DIG_000, four diggers, each assigned through the panel with the stock
going down. All four are `dir -1` or `+1`, `vdir +1`, `[+0x360] = -1.0` facing
left and `0.0` facing right (which closes the static note's "the integers in
`lemming+0x40` and `+0x44` are unread": they are ±1 and +1), `[+0x368] = 0.0`.

Digger at (47.370, 75.000), window rows 61..74, 2,680 samples in 4 s:

```
  t 0.269s pct 24  (col 46, row 74)  0x00000021 -> 0x00000000
  t 0.269s pct 24  (col 47, row 74)  0x00000021 -> 0x00000000
  t 0.269s pct 24  (col 48, row 74)  0x00000021 -> 0x00000000
  t 1.471s pct 25  (col 46, row 73)  0x00000021 -> 0x00000000
  t 1.471s pct 25  (col 47, row 73)  0x00000021 -> 0x00000000
  t 1.471s pct 25  (col 48, row 73)  0x00000021 -> 0x00000000
  t 2.673s pct 25  (col 46, row 72)  ...  (three cells)
  t 3.873s pct 24  (col 46, row 71)  ...  (three cells)
```

Three more diggers gave the same shape at (59.234, 75.000), (38.150, 75.000)
and (36.463, 59.000): **3 cells a cycle, four cycles, 12 cells, every one
`0x21 -> 0x00000000`.**

Observation, geometry. The columns are `trunc(X) - 1`, `trunc(X)`,
`trunc(X) + 1`; the row is `trunc(Y) - 1` in the live Y-up grid, i.e. the row
**below** the feet. Entering the dig state snaps X to an integer — 47.370 →
47.000, 79.551 → 79.000, 36.463 → 36.000 — so the three columns are centred
on the lemming's own column with no fractional ambiguity.

Observation, timing, from the state's own fields. `[state+0x2C]` is the
percent and `[state+0x28]` the record cursor:

```
  0.000s dig pct  2 cur 0 y 43.000
  0.278s dig pct 25 cur 1 y 43.000     <- the bite, cursor 0 -> 1
  0.333s dig pct 30 cur 0 y 42.000     <- the Y step, cursor wraps, y -1.000
  1.479s dig pct 25 cur 1 y 42.000
  1.535s dig pct 30 cur 0 y 41.000
  2.675s dig pct 25 cur 1 y 41.000
  2.737s dig pct 30 cur 0 y 40.000
  3.876s dig pct 25 cur 1 y 40.000
  3.938s fall pct 17 cur 49 y 38.900   <- nothing left under it
```

- the bite is at percent **25** and the Y step at **30**, which is the record
  table exactly;
- the Y step is **−1.000**, so `diggerDepth = 1` is measured, not "it could
  not halve";
- the cycle is **1.201, 1.196, 1.201 s** here, **1.202, 1.202, 1.200** on
  the second digger, **1.196, 1.200, 1.202** on the third and
  **1.201, 1.202, 1.201** on the fourth -- twelve gaps, none outside
  1.196-1.202, mean **1.2005 s = 6.00 ticks**;
- the last cycle ends in the **fall**, which is the frame-30 record's own
  route out (`0x0042E913` returns 0, `0x00415FA0` builds the state whose
  vtable store is `mov [eax], 0x004B0A20` at `0x00415FB2`).

The record list, read out of the running process (`[state+0x18]` count at
`+0x00`, pointer array at `+0x0C`):

```
  record list of 0x02983090 vt 0x004b09f0 (dig): 2 entries
    [ 0] frame  25  fn 0x00415a43 dig-bite       arg 0
    [ 1] frame  30  fn 0x00415dd2 dig-step-y     arg 0
  animation strip 4 frames 20 divisor 1.0 rate 0.75
```

### The refusal — and the "digger gives up after one tick" mystery, closed

Predicted `REFUSE`, aimed at deliberately (`--refuse`). Digger at
(79.551, 26.000) facing left, floor row 25 = `0x3`:

```
   0.000s dig  pct  2 cur 0  x 79.000 y 26.000
   0.264s dig  pct 24 cur 0  x 79.000 y 26.000
   0.277s walk pct -1 cur 49 x 78.975 y 26.000
  9325 samples in 2.0s (4662 Hz), 0 cell changes
  stock[7] 7 -> 6  spent=True
  whole-grid diff: no change outside the window
```

**Zero cells changed anywhere in the grid**, the stock was spent, and the
state reverted to `walk` the instant the percent reached 25.

That is the assign round's unexplained "the dig state lives 271 ms". It is
not a tick phenomenon and it is not a missed click: the dig state lives
exactly as long as it takes the animation to reach percent 25, which is
0.25 × 1.2005 s = **0.300 s** from the start of the state, and the assign
round sampled from its first read rather than from the state's birth. The
0.271 s it measured and the 0.277 s here are the same number. The engine also
plays a sound on the refusal (`0x00415B33` loads `[0x004E32C8]`, `0x00406A00`
then `0x0044535B`) before `0x0042F8B9` puts the lemming back to walking, which
is presumably the audible "clang" a player hears.

---

## Bash — the series

MM_BASH_002. Panel index **5** is the basher: the level ships
`[0,0,0,0,0,10,0,0,0,1]` and the engine pre-selects it, the same argument that
identified the digger as 7. (That is one of the five indices
`phase3-assign-skill.md` left unidentified; index 6 falls out of MM_MINE
below. Neither was taken from the classic order.)

Right-facing basher at (190.455, 24.000), flush against the wall at columns
191+, which is plain `0x1`. Whole-grid watch, 3,163 samples in 5 s:

```
  t 0.152s pct 12  (col 191, row 24)  0x00000001 -> 0x00000000
  t 0.206s pct 16  (col 191, row 25)  0x00000001 -> 0x00000000
  t 0.840s pct 60  (col 191, row 27)  0x00000001 -> 0x00000000
  t 0.895s pct 64  (col 191, row 26)  0x00000001 -> 0x00000000
  t 1.590s pct 12  (col 192, row 24)  ...
  t 1.644s pct 16  (col 192, row 25)
  t 2.276s pct 60  (col 192, row 27)
  t 2.340s pct 64  (col 192, row 26)
  t 3.027s pct 12  (col 193, row 24)   ... the same four
  t 4.472s pct 11  (col 194, row 24)   ... cycle five, cut short by the clock
```

Observation. **Four cells a cycle, all in the leading column, at the feet row
and the three rows above it, in the order +0, +1, +3, +2 at percents 12, 16,
60, 64.** One new column per cycle: 191, 192, 193, 194. Relative to the
lemming's own cell the footprint is constant: `d(col) = +1`,
`d(row) = +0, +1, +2, +3`.

Cycle: frame-12 events at 0.152, 1.590, 3.027, 4.472 → **1.438, 1.437,
1.445 s**, i.e. **1.440 s = 7.20 ticks**.

The mirror, facing left, at (156.97, 40.000) — and this one is also the
directional-bit test, below:

```
  (col 152, rows 40,41,43,42)   0x00000005 -> 0x00000000
  (col 151, rows 40,41,43,42)   0x00000005 -> 0x00000000
  (col 150, rows 40,41,43,42)   0x00000005 -> 0x00000000
  (col 149, rows 40,41,43,42)   0x00000005 -> 0x00000000
  (col 148, rows 40,41,43,42)   0x00000005 -> 0x00000000
```

`d(col) = -1` and `d(row) = +0..+3` every time — the same footprint mirrored,
one column per cycle, five cycles.

Record list and animation, live:

```
  record list of 0x0268b0b0 vt 0x004b06ac (bash): 17 entries
    [ 0] frame   4  bash-nudge-x        [ 9] frame  60  bash-bite  arg 3
    [ 1] frame   8  bash-nudge-x        [10] frame  64  bash-bite  arg 2
    [ 2] frame  12  bash-bite  arg 0    [11..15] frames 72,76,80,84,88 nudge
    [ 3] frame  16  bash-bite  arg 1    [16] frame  92  bash-commit
    [4..8] frames 32,36,40,44,48 nudge
  animation strip 0 frames 24 divisor 1.0 rate 0.0
```

which is the static note's constructor table, entry for entry, read out of
memory. The nudges are visible in the X series as steps of exactly
**0.0769 = 1/13** (`0x0040C806` × the 1.0 at `0x004B02D0`, `0x0040C80C` ÷ the
13.0 at `0x004B06D0`): 161.000 → 160.923 → 160.846 → …

### Correction to the static note's bash column loop

`phase3-round5.md` says the bite "walks columns from the current X toward
state `+0x4C`". It is the other way round, and the endpoints matter. From
`0x0040C357`:

```
0040c39d  fild [ecx+0x40]        ; the facing
0040c3a0  fadd [ebp-0x38]        ; + 0x0042F22E's x  (= X + [+0x360])
0040c3ac  call 0x40dec0          ; wrap into [0, 256)
0040c3b1  call 0x49c5ac          ; truncate            -> colX
0040c3da  sete dl / add eax, edx ; +1 when facing left
0040c3f4  ...  rowY + arg * [+0x44], clamped           -> baseRow
0040c423  jne  0x40c595          ; state byte [+0x41] set -> leading cell only
0040c435  je   0x40c595          ; colX == [state+0x4C]   -> leading cell only
0040c4a4  mov [ebp-0x40], [state+0x4c] + step            ; the walk STARTS at
0040c4b8  je   0x40c595          ;   ... and ENDS, exclusive, at colX
0040c4ec  cmp [ebp-0x48], 4      ; four rows per column, baseRow + j*[+0x44]
0040c5e2  call 0x40de60          ; then the one leading cell (colX, baseRow)
```

So the sweep is **the columns strictly between the column the lemming entered
on and the leading column**, four rows each, and the leading column gets one
cell per bite — four over the cycle, which is why the net footprint is the
leading column × 4 rows. The sweep is a catch-up path over cells the basher
has already emptied. It was caught doing something exactly once, and that one
cell is the proof: a basher that had entered at column 156 and reached
leading column 151, inside a tunnel another basher had already cut at rows
40–43, cleared **(152, 44) and nothing else** — column 152 is between 156 and
151, and row 44 is `baseRow + 3` for the frame-16 bite's `arg 1`. Four rows
deep, in an in-between column, predicted and then seen as a single cell.

---

## Mine — the series

MM_MINE. Panel index **6** is the miner: the level ships
`[0,0,0,0,0,0,10,0,0,1]`.

Miner at (193.049, 72.000) facing right, whole-grid watch:

```
  cycle 1  t 0.269s pct 19  (col 194, row 71)   t 0.273s pct 20  (col 195, row 71)
  cycle 2  t 1.714s pct 16  (col 196, row 71)
           t 1.769s pct 20  (col 195, row 70)   (col 196, row 70)
  cycle 3  t 3.215s pct 16  (col 197, row 70)   (col 197, row 71)
           t 3.270s pct 20  (col 196, row 69)   (col 197, row 69)
  cycle 4  t 4.653s pct 12  (col 198, row 71)
           t 4.715s pct 16  (col 198, row 69)   (col 198, row 70)
           t 4.770s pct 20  (col 197, row 68)   (col 198, row 68)
```

all `0x21 -> 0x00000000`, and the lemming went
(193, 72) → (194, 71) → (195, 70) → (196, 69): **one column forward and one
row down per cycle**, which is the frame-68 commit `0x004190F3`.

Every cell fits the static model with nothing left over: for a bite with
argument `a`, the cells are `(trunc(X) + dx*[+0x40], trunc(Y) + a*[+0x44])`
for `dx` in 1, 2, with `(a == 2, dx == 1)` skipped. Worked through: cycle 1
cleared only the `a = -1` pair because rows `+0..+3` were open air above the
surface; cycle 2's `a = 0` bite found `cx+1` already empty and took only
`cx+2`; cycle 3's frame 16 fired both its records (`a = 1` and `a = 0`) and
took one cell each. Cycle 4's frame 12 took `(198, 71)`, which is `cx+2` at
`a = 2` — the argument whose `dx = 1` the engine skips.

Cycle: frame-20 events at 0.273, 1.769, 3.270, 4.770 → **1.496, 1.501,
1.500 s**; a second window on the same miner gave 1.499, 1.501, 1.501.
**1.4997 s = 7.50 ticks.**

```
  record list of 0x027ce770 vt 0x004b0b68 (mine): 17 entries
    [0] frame 12 mine-bite arg 3   [1] frame 12 mine-bite arg 2
    [2] frame 16 mine-bite arg 1   [3] frame 16 mine-bite arg 0
    [4] frame 20 mine-bite arg -1
    [5..15] frames 24,28,...,64 mine-nudge-xy
    [16] frame 68 mine-commit
  animation strip 9 frames 25 divisor 1.0 rate 0.0
```

---

## The clock — "interval in ticks" is the wrong shape

READ. `0x00468B04` is the animation's duration and `0x00468B5D` its current
time:

```
00468b27  call 0x45b8f0          ; the frame count: 0x0045B8F0 returns [this]
00468b32  fmul [0x004b1940]      ; b8 1e 85 eb 51 b8 ae 3f = 0.06
00468b56  fdiv [eax+0x10]        ; the strip's own divisor
00468b69  fild [eax+0x1cc]       ; the CURRENT frame index
00468b6f  fmul [0x004b1940]      ; the same 0.06
00468b78  fadd [ecx+0x1c0]       ; plus the time inside that frame
00468b9c  fdiv [eax+0x10]        ; the same divisor
```

and `0x00468D00` resolves the strip as `[controller+0x1F8 + 0x0C] +
[controller+0x1C8] * 0x98`, with the frame count at `[strip+0x00]` and the
divisor at `[strip+0x10]`. The divisor cancels in the percent the stepper
takes, so

    percent = 100 * (frame + frac/0.06) / frames        and
    one cycle = frames * 0.06 seconds.

MEASURED, read live off the controller at `lemming+0x128`:

| skill | strip | frames | divisor | `+0x1D8` | frames × 0.06 | measured cycle | ticks |
| --- | --- | --- | --- | --- | --- | --- | --- |
| bash | 0 | 24 | 1.0 | 0.0 | 1.440 s | 1.438, 1.437, 1.445 | **7.20** |
| dig | 4 | 20 | 1.0 | 0.75 | 1.200 s | twelve gaps, 1.196-1.202 | **6.00** |
| mine | 9 | 25 | 1.0 | 0.0 | 1.500 s | 1.496, 1.501, 1.500 | **7.50** |

Two things follow, and they are the round's answer to its own question.

**One. There is no interval in ticks.** 7.20 and 7.50 are not tick counts. A
single integer cannot hold either, and rounding bash to 7 makes it 2.8% fast
and to 8 makes it 11% slow — over a ten-cycle bash that is most of a column.
`Rules` therefore carries `basherCycleMs = 24*60`, `diggerCycleMs = 20*60`
and `minerCycleMs = 25*60`, and `World::bitesThisTick` asks whether the
200 ms tick starting now contains the cycle's bite. Sampling the real cycle
that way makes bash act every 7 or 8 ticks and average exactly 7.2 — the gaps
come out 8, 7, 7, 7, 7, which `test_world` now writes out — instead of
rounding the difference away. Dig still acts every 6 ticks, because for dig
alone the tick count is exact.

**Two. `0.75` is not the cycle's divisor.** The static note was right to
refuse to call it a tick interval, and it is not a time scale either: dig's
strip is 20 frames and 20 × 0.06 is the 1.2005 s measured, with no 0.75 in
it. What the rate at `controller+0x1D8` does is still unread. (Bash and mine
read 0.0 there, and `0x0046845C`'s `fadd [controller+0x1D8]` onto the 0.0625
at `0x004B1950` therefore adds nothing for them, which is consistent with
both playing at the plain 0.06 s per frame.) Hypothesis, untested: 0.75 is
dig's *sprite* playback rate — how fast the strip is drawn — and the
simulation clock above ignores it. Do not build on that until somebody reads
`0x00426DB0`.

---

## The four unread cell bits — `kCellUnreadBits` is now 0

READ. `CollisionGrid.h` listed `0x4`, `0x8` and `0x20` as bits with no
test, set or clear anywhere in `.text`, and said of `0x20` that its presence
in the grid was "data, not an instruction". The scan could not have found
them: each bite builds its mask **on the stack** from the lemming's own
direction and then `and`s the cell against the stack slot, so there is no
`and reg, imm` on a cell word to find.

```
dig   00415a86  mov [ebp-0x10], 0x10      ; upright
      00415a90  cmp [ecx+0x44], -1
      00415a96  mov [ebp-0x10], 0x20      ;   inverted
      00415b1b  and eax, 2                ; indestructible, unconditional
      00415b2a  and edx, [ebp-0x10]       ; the directional bit
      00415b2f  je  0x415b7e              ;   both clear -> the store
bash  0040c0dc  stores 4 ; 0040c0ec stores 8 when [+0x40] == -1
      tests at 0040c0f8 / 0040c104 / 0040c110 (the third is 0x800)
mine  004189b0  stores 4 ; 004189c0 stores 8 when [+0x40] == -1
```

so:

| bit | meaning | evidence |
| --- | --- | --- |
| `0x0004` | cannot be bashed or mined **rightward** | read; 2,016 cells of `0x5` on MM_BASH_002, none on the dig or mine levels |
| `0x0008` | cannot be bashed or mined **leftward** | read; no retail level checked carries it |
| `0x0010` | cannot be dug **upright** | read; no retail level checked carries it |
| `0x0020` | cannot be dug **inverted** | read; 876 cells of `0x21` on MM_DIG_000, 2,048 on MM_MINE, 3,024 on MM_FLOAT |

Measured live, the permissive half of two of them:

- **four diggers cleared `0x21` cells while upright** (24 cells), so the
  upright mask does not include `0x20`;
- **a left-facing basher cleared five columns of `0x5`** (20 cells), so the
  leftward mask does not include `0x4`.

Measured live, the blocking half of `0x2`: a digger on a `0x3` floor changed
nothing in the grid and reverted to walking.

Not measured: `0x4` blocking a **rightward** basher, and anything inverted.
The right-facing test needs a lemming on the far side of a `0x5` block —
MM_BASH_002's two blocks are at columns 83–98 and 137–152, their left faces
are off the camera arc this level shows, and `measure_dig`'s `on_screen`
filter correctly refused to aim at a lemming the engine cannot pick. The
inverted tests need anti-grav, which is round 9. **So the direction
dependence is a read, not a measurement, and the note says so.** What is
measured is that the bits do not block the direction the code says they do
not block, on two of the four.

The remaining unknown about all four is the **writer**: no instruction sets
any of them on a cell. They arrive in the `.LVL` bytes, which is consistent
with their being level authoring rather than runtime state.

---

## What the removal writes

MEASURED, and it settles the trap the brief named twice. Every one of the 38
cells cleared this session went to **`0x00000000`**, from `0x21`, `0x5` and
`0x1`. That is `mov dword ptr [ecx], 0` at `0x0040C16D` (bash),
`mov dword ptr [edx], 0` at `0x00415B84` (dig) and `0x00418B29` (mine), and it
is **not** `and 0xf77c`, which would have left `0x20` standing on MM_DIG_000's
cells and `0x4` on MM_BASH_002's. `FUN_0041B683` is therefore excluded by
observation as well as by having no caller in `.text`: it has the wrong effect,
not merely an unknown provenance.

`tools/harness/read_grid.py`'s docstring asserted the `and 0xf77c` reading as
established. It has been corrected, with both the addresses and this
measurement.

---

## Rules — what moved, and why

| field | before | after | why |
| --- | --- | --- | --- |
| `basherInterval` | 4 ticks | **gone** | 7.20 ticks is not an integer |
| `diggerInterval` | 4 ticks | **gone** | replaced by the measured cycle; the count happens to be exactly 6 |
| `minerInterval` | 6 ticks | **gone** | 7.50 ticks is not an integer |
| `basherCycleMs` | — | **1440** | 24 frames × 60 ms, measured 1.440 s |
| `diggerCycleMs` | — | **1200** | 20 frames × 60 ms, measured 1.2005 s |
| `minerCycleMs` | — | **1500** | 25 frames × 60 ms, measured 1.4997 s |
| `basherBitePercent` | — | **12** | the first bite's record frame, read live |
| `diggerBitePercent` | — | **25** | ditto |
| `minerBitePercent` | — | **12** | ditto |
| `basherReach` | 2 | **1** | one column per cycle, both facings |
| `basherHeight` | — | **4** | feet row and the three above |
| `diggerWidth` | — | **3** | was a hard-coded 5 in the stepper |
| `diggerDepth` | 1 | **1**, now measured | the frame-30 record's −1.000 |
| `minerReach` | — | **2** | columns +1 and +2 ahead |

`builderInterval`, `brickLength`, `builderBricks`, `bomberFuse` and
`explosionRadius` were not touched: they are rounds 6 and 7.

The steppers moved with the fields, because the series disagreed with them on
geometry and not only on timing:

- `stepDigger` cleared **5** cells wide; the original clears 3.
- `stepBasher` cleared `basherReach` columns × `lemmingHeight` rows starting
  one row **above** the feet; the original clears one column × 4 rows
  **including** the feet row.
- `stepMiner` cleared a box; the original clears a 2 × 5 block with one cell
  missing, and that missing cell is in the code (`0x00418A1E` against the
  dword 3 at `0x004B10B8`).

### Within-tick simplifications, stated

Three things the original does inside a cycle that a 5 Hz step cannot:

1. **The four bash bites are 12, 16, 60 and 64 percent in** — up to 750 ms
   apart. We clear all four rows on the tick that covers percent 12. Nothing
   in this simulation reads the terrain between them, so the only way to see
   the difference is to render it.
2. **Dig's bite (25) and Y step (30) are 60 ms apart.** We do both on one
   tick.
3. **The first act lands on the tick that covers the bite, not at the bite.**
   Dig's bite is 300 ms into the state and our tick boundary is 400 ms, so our
   first removal is up to one tick late. The *cadence* after that is exact.

---

## Tests

`test_world` gained `testDigFootprintAndCadence`, `testBashFootprintAndCadence`
and `testSkillRules`. **Expectations are written out, not derived from
`Rules`** — the round-4 lesson, and `testStepUpHeight` is the pattern copied.

Verified the way the facilitator verified round 4's: with `basherReach` back
to 2, `basherHeight` to 3, `diggerWidth` to 5 and both cycles back to the
classic `4 * 200` ms, `test_world` fails **19 checks**, and they are the
behavioural rows, not only the field asserts:

```
  FAIL  every cycle took 3 cells (first four: 5, 5, 5, 5)
  FAIL  six ticks between bites (gaps 4, 4, 4)
  FAIL  column x-2 is still there at y+1          (x8, both sides, four rows)
  FAIL  every cycle took 4 cells (first four: 6, 3, 3, 3)
  FAIL  bash gaps 8,7,7,7,7 (were 4,4,4,4,4)
  FAIL  five cycles span 36 ticks (was 20)
  FAIL  three rows up went too
```

Restored, the suite is **16 of 16** and `test_world` is 121 checks,
0 failures.

```powershell
.\scripts\Invoke-Ucrt64.ps1 "make -C src test"
```

---

## Clean negatives, and what was searched for

- **A basher in open air removes nothing and gives up part-way through its
  second cycle.** (181.877, 24.000) facing left on MM_BASH_002: 12 nudges,
  four bites, **zero** cell changes, the frame-92 commit fired (percent 92,
  cursor wrapped to 0), then `walk` at percent 11–12 of the second cycle.
  Observation only — why the second cycle's frame-12 bite ends the state
  while the first one's did not is **not established**. The candidates are
  the `state+0x44 < 2` counter and the `state+0x41` byte in `0x0040C6E8`, and
  the fact that the column walk has something to do on the second cycle
  (`colX != [state+0x4C]`) and nothing on the first. Recorded rather than
  guessed at; it is the natural companion to round 6's builder work.
- **The right-facing `0x5` refusal was not reached.** Named above with the
  reason (camera arc) rather than left as a gap.
- **Nothing outside the watched window ever changed.** Every run diffed the
  whole 96-row grid before and after. If a skill touched the grid anywhere
  else — a second removal, a rendering side effect — this would have caught
  it, and it never fired.
- **No `0x4`, `0x8`, `0x10` or `0x20` was ever *set* on a cell.** The
  whole-grid before/after diffs would have shown it.
- **The animation rate at `+0x1D8` was read, not interpreted.** 0.75 for dig,
  0.0 for bash and mine. It does not divide the cycle. `0x00426DB0` is still
  unread and that is where the next person should look.

## Harness notes

- **`[state+0x2C]` can read above 100.** `0x0041B290` stores the raw percent
  and `0x0041B2B2` reduces it mod the dword 100 at `0x004BF4C8` several
  instructions later, so a sample landing between them sees values like 144
  or 1488. Not corruption; do not "fix" it, and do not treat the percent as a
  clean 0..99 when sampling at kHz.
- **A state object is freed when the skill ends**, so `[[lemming+0x340]]`
  after that is whatever the allocator put there. `sample` prints it as a raw
  vtable rather than guessing.
- **Two lemmings in a skill contaminate a whole-grid diff.** One mine run
  recorded cells belonging to the *previous* miner, still working 5 s later,
  and its `d(col)` offsets were nonsense because they were relative to the
  wrong lemming. The clean runs are the ones where only one lemming had the
  skill. Prefer `--rows 14` (a window around the one lemming) once the
  geometry is known, and `--rows 0` only to prove nothing else moved.
- **`one_run` requires `[screen+0x16D8] == the lemming it chose`**, which is
  stricter than `assign_skill.assign`. With lemmings packed together the
  engine often names a neighbour and the attempt is reported as "none of the
  candidates became hoverable". That cost four runs this session. It is the
  right trade when the point is to aim at a *predicted* lemming; a round that
  only needs any lemming should use `assign_skill.assign`.
- `lrstate.attach` returns `(handle, pid)`. Still true.
- `goto_level.focus` was never called. Entry is `measure_float_splat.enter`
  (`select_level` + `restart_level.ensure_playing`), bracketed by the slot
  [0][0] restore, five times, all five printing
  `slot [0][0] restored: True`.
- **A level ends on you.** One run came back `ui_state 5, not in a level`:
  MM_BASH_002's 300 s clock had run out while the harness was being edited.
  No ESC was pressed. Restart the process; do not try to leave a results
  screen.

---

## Left the tree

| file | change |
| --- | --- |
| `tools/harness/measure_dig.py` | **new.** `--predict` is offline; the live paths are the series above. Every address is in its docstring. |
| `tools/harness/read_grid.py` | docstring only: the `and 0xf77c` removal claim replaced with what the bites actually store, with the three addresses and this session's measurement. |
| `src/sim/World.h` | `Rules`: three interval fields out, three cycle-ms and three bite-percent fields in, `basherReach` 2 → 1, new `basherHeight`, `diggerWidth`, `minerReach`, `diggerDepth` re-labelled measured. The round-5 comment block rewritten with the addresses and the series. `msPerTick()` declared. |
| `src/sim/World.cpp` | `bitesThisTick` added; `stepBasher`, `stepMiner`, `stepDigger` rewritten to the measured footprints and the millisecond cycle. |
| `src/__test__/test_world.cpp` | `testDigFootprintAndCadence`, `testBashFootprintAndCadence`, `testSkillRules`, all with written-out expectations. |
| `src/assets/CollisionGrid.h` | the four directional refusal bits named with their addresses; `kCellUnreadBits` 0x4\|0x8\|0x20 → 0, with why the scan missed them and what is still unread. |

Not touched: `World::spawn`, `findHatchRecords`, `hatchSpawnCell`,
`Entrance::source`, `fallSpeed`, `floatSpeed`, `floatAfter`, `splatDistance`,
`walkSpeed`, `climbSpeed`, `ticksPerSecond`, `maxStepUp`, `lemmingHeight`,
`src/assets/LevelStats.cpp`, `worldToMaskPixel`, `test_corpus`'s numbers,
`test_collisiongrid`, `PLAN.md`, `docs/PHASE3_HANDOFF.md`, and
`docs/scratch/phase3-round5.md`. The five open stepper differences are
untouched; so is the builder's `+1.000`.

## Left the game

**Killed.** No process is running at the end of this session; `Get-Process`
for `*Lemmings*` returns nothing. Five processes were used, one per entry:
MM_DIG_000 (pid 17644), MM_BASH_002 (29408, 42512, 45444) and MM_MINE (7520),
each started by `dump_grids.restart_game` and killed before the next entry.
The last level played was `Compiled\MM_BASH_002.lvl`. No ESC was pressed from
a results screen.

The shim is installed and inert (`dinput.dll` in the game folder, `enable`
left at 0 by `LRInput.release()`). A fresh launch comes up at `ui_state 3`,
the main menu.

## For round 6, and whoever lands PLAN.md

- **The builder shares the clock.** `0x0040FAB6` is the build enter hook and
  `0x004B08DC` the build vtable; the same `0x0041B077` record mechanism runs
  it, so `builderInterval = 8` ticks is almost certainly the wrong shape too
  and round 6 can settle it with one `measure_dig.py --skill 4` run and the
  animation read already in `sample`: read the strip's frame count and the
  record list, and the cycle is `frames * 0.06`.
- **`0x00426DB0` is the one piece of the clock still unread**, and with it
  what dig's 0.75 at `controller+0x1D8` does.
- **The bash catch-up sweep is modelled as nothing.** It only ever touches
  cells the basher has already emptied — except on the one path where it does
  not, which is a basher whose column got ahead of its leading column (a
  basher that fell, or was re-assigned). Worth one deliberate test some day.
- **The right-facing `0x5` refusal and everything inverted are open.** A
  gadget round that produces an inverted lemming gets `0x10` and `0x20` for
  free.
