# Phase 3 round 6, live — when a builder lays a brick, and which cells it writes

2026-10-15. Stock `C:\progs\Lemmings Revolution\Lemmings Revolution.exe`,
SHA-256 `32675bdb49866e76502713251fdfd2b09078c2481c72ca22ff68ebf4b3737a2f`,
image base `0x400000`, driven through `tools/dinput-shim`.

This tests `docs/scratch/phase3-round6.md` (the static read) and does not
repeat it. **It does not contradict that note's answer anywhere**: percent 81,
two cells at offsets +1 and +2 in the facing direction, on the lemming's own
row, `or 1`, 12 bricks, then shrug — all four held live, cell for cell. It
corrects the static note in **one** place, under "The refusal" below: that note
says the percent-90 callback probes the cell at `(truncX + facing, truncY)`,
and the row is the row **after** the unconditional raise, not before; and
there is a **second** probe past the decrement that the note did not reach.

Harness: `tools/harness/measure_build.py` (new). Every address in it is in its
docstring.

---

## SUMMARY

| Thing | Status | Value |
| --- | --- | --- |
| **cells per brick** | **MEASURED, 3 builders × 12 bricks = 72 cells** | **two**, at `(p + facing, row)` and `(p + 2·facing, row)`. `brickLength = 3` is wrong and has moved to **2**. |
| **the row** | **MEASURED** | the builder's **own** row, `trunc(Y)` — not the row below the feet the way the dig bite is. A builder at world Y 23 writes row 23 and then stands at 24. |
| **the column** | **MEASURED, both facings** | `p = trunc(X at the click)` in **both** facings. The enter hook's left-facing `+1` snap and the `-1.0` X bias cancel, exactly as they do in the bash bite (round 5). |
| **the value written** | **MEASURED, 72 of 72 cells** | `0x00000000` → **`0x00000001`**. Plain solid. **Not** `0x403`, so **a brick is destructible** and `Material::Built` is the right material. |
| **the percentage** | **MEASURED, 36 of 36 stamps** | **81**, every time, read off `[state+0x2C]` in the same sample as the grid diff. |
| **the cycle** | **READ + MEASURED** | build is animation **strip 2, 16 frames, divisor 1.0** → `16 × 0.06 / 1.0` = **0.960 s**. 33 gaps across three builders, min 0.956, max 0.969, **mean 0.95982 s = 4.7991 ticks**. |
| **"interval in ticks" does not survive, again** | **MEASURED** | 4.8 ticks. `builderInterval = 8` is **deleted**; `Rules` carries `builderCycleMs = 960`. |
| **the enter seek** | **READ + MEASURED** | the `16.0` at `0x004B08F8` is an enter-time seek of **one sixteenth of a cycle = 6.25%**, so the first brick lands at 75% and not 81%. Predicted 0.718 s after the click, measured 0.715 s. |
| **brick count** | **MEASURED, 3 of 3 builders** | **12**, from `[state+0x3c]`, counted down 12 → 1 in the live series and spent on exactly twelve bricks. |
| **the advance** | **MEASURED, 36 advances** | at percent **90**: world Y **+1.000** (up, Y-up grid) and X **+1·facing**, in that order, in the same callback as the decrement. |
| **exhaustion** | **MEASURED, 2 of 2 builders that got there** | the twelfth percent-90 leaves for **shrug** (`0x004B0C78`), which lasts **0.966 s** and then walks. |
| **the shrug's length** | **READ + MEASURED** | strip **10, 8 frames, divisor 0.5** → `8 × 0.06 / 0.5` = 0.96 s. **This is the first divisor in this project that is not 1.0, and it settles that the divisor belongs in the DURATION** even though it cancels in the percent. |
| **the refusal** | **READ; the first of two exits MEASURED and predicted** | a non-zero step-into cell turns the builder and takes it out of the build state **without spending the brick**. |
| **arrow keys are the engine's own direction filter** | **MEASURED** | holding RIGHT sets `[screen+0x16B1]`, LEFT sets `[screen+0x16B2]`. Pure input; it is how the mirrored facing was reachable on a level with twenty lemmings in one pen. |

Re-run. `--probe` and the strip table send no input:

```powershell
# restart the process first; there is no route from inside a level to the map
python tools/harness/measure_build.py --enter MM_BUILD --probe

# a whole twelve-brick staircase, whole-grid watch
python tools/harness/measure_build.py --skill 4 --seconds 16 --rows 0 --min-bricks 12

# the mirror: the RIGHT arrow is held while aiming, so only right-facers pick
python tools/harness/measure_build.py --skill 4 --seconds 16 --rows 0 --min-bricks 6 --facing 1

# the refusal, predicted: at most six bricks will land on air, so it stops
python tools/harness/measure_build.py --skill 4 --seconds 9 --rows 0 --min-bricks 2 --max-bricks 6 --repeat 2
```

`--enter` is the only thing that writes to the game, and the only write is
`goto_level`'s level-record swap, which is put back (`slot [0][0] restored:
True` on all four entries this session). **No lemming byte was written and no
grid cell was written.** The skill went on through the panel every time, by
`assign_skill.py`'s primitives, and the engine spent a unit of stock on every
click.

---

## The method

The same one round 5 and round 8 used, and it was cheap for the same reason:
the engine collides against the stored 256 × 96 `u32` grid, the live grid is
byte-identical to the `.LVL` bytes, and the static read said the brick is a
store through `0x0040DE60`'s return — so **the brick is directly observable**.
Snapshot the grid, click, diff it per sample, print on any change.

`measure_build.sample` does four `ReadProcessMemory` calls an iteration — the
state pointer, the state block (0x40 bytes, so the brick counter at `+0x3C`
comes with the percent at `+0x2C`), the lemming's own `0x40` bytes from `+8`,
and the whole 96-row grid. With the whole grid it runs at **660–680 Hz**,
against an animation that advances one percent per 9.6 ms, so a stamp cannot
hide between samples. Every run also diffed the whole grid before against
after; **nothing outside the watched window ever changed**, on any run.

The prediction came first. `predict_stair` applies the static read — including
the enter hook's X snap — to the live grid and prints the 24 cells it expects;
`clear_prefix` says how many of the twelve bricks would land on air **now**,
which is what makes a short staircase a *predicted* refusal rather than a
surprise.

---

## The brick — three builders, 72 cells

MM_BUILD, panel index **4** (the level ships `[0,0,0,0,10,0,0,0,0,1]` and the
engine pre-selects it). Each builder assigned through the panel with the stock
going down. Three separate processes, because a level ends on you.

**Builder 1**, left-facing, start `(185.147, 23.000)`, `dir -1`, `vdir +1`,
`[+0x360] = -1.0`, `[+0x368] = 0.0`. X snapped to **186.000** on entering the
build state, which is `trunc(185.147) + 1` — the enter hook's left-facing
correction, read at `0x0040FB01` / `0x0040FB0B` and now observed.

```
  brick  1  t  0.694s  count 12  (183,23) 0x0->0x1 @81  (184,23) 0x0->0x1 @81
  brick  2  t  1.653s  count 11  (182,24) 0x0->0x1 @81  (183,24) 0x0->0x1 @81
  brick  3  t  2.611s  count 10  (181,25) (182,25)
  brick  4  t  3.570s  count  9  (180,26) (181,26)
  brick  5  t  4.535s  count  8  (179,27) (180,27)
  brick  6  t  5.493s  count  7  (178,28) (179,28)
  brick  7  t  6.452s  count  6  (177,29) (178,29)
  brick  8  t  7.411s  count  5  (176,30) (177,30)
  brick  9  t  8.370s  count  4  (175,31) (176,31)
  brick 10  t  9.335s  count  3  (174,32) (175,32)
  brick 11  t 10.294s  count  2  (173,33) (174,33)
  brick 12  t 11.252s  count  1  (172,34) (173,34)
  11.334s shrug   x 174.000 y 35.000
  12.301s walk    x 173.975 y 35.000
```

**Builder 2**, right-facing — the mirror — start `(159.845, 23.000)`,
`dir +1`, `[+0x360] = 0.0`. X snapped to **159.000**: plain truncation, no
`+1`, which is the other half of the same read.

```
  brick  1  t  0.685s  count 12  (160,23) 0x0->0x1 @81  (161,23) 0x0->0x1 @81
  brick  2  t  1.644s  count 11  (161,24) (162,24)
  brick  3  t  2.603s  count 10  (162,25) (163,25)
  ...
  brick 12  t 11.244s  count  1  (171,34) (172,34)
  11.328s shrug   x 171.000 y 35.000
  12.293s walk    x 171.030 y 35.000
```

**Builder 3**, left-facing, a different part of the level, start
`(233.966, 49.000)`. Bricks `(231,49)(232,49)`, `(230,50)(231,50)`, …,
`(220,60)(221,60)`, then shrug at `(222, 61)`.

Observation, geometry, in live-grid (Y-up) coordinates:

- **two cells**, at `p + facing` and `p + 2·facing`, on **row `trunc(Y)`**;
- `p = trunc(X at the click)` in **both** facings. Builder 1:
  `trunc(185.147) = 185`, cells 184 and 183. Builder 2:
  `trunc(159.845) = 159`, cells 160 and 161. Builder 3:
  `trunc(233.966) = 233`, cells 232 and 231. The snap and the bias cancel.
- the brick is in the lemming's **own** row, and after the advance the
  lemming stands on it: brick *k* occupies `(p_k + facing, row_k)` and the
  lemming's next position is exactly `(p_k + facing, row_k + 1)`.
- so the staircase is 2-wide treads, one column and one row per brick,
  spanning 13 columns and 12 rows for twelve bricks.

Observation, the value: **72 of 72 cells went `0x00000000` → `0x00000001`.**
Bit `0x2` (indestructible) is clear, so **a brick can be dug, bashed and
mined**. The `or edx, 0x403` at `0x004275C1` the brief asked about is **not**
the brick, and the live diff is not the only reason: its function
`0x00427554` has exactly three `E8` callers in all of `.text` —
`0x004271D5`, `0x00427509` and `0x0043B1F8`, that last inside `0x0043B150`,
which walks the object collections at `0x004C8968` / `0x004C8988` gated on
`byte [obj+0x58]` — and the builder's chain
`0x0041011C` → `0x0043B016` → `0x0043AE6C` reaches none of them.
`0x0043AE6C` makes no call to `0x0040DE60` and contains no `or` on a cell; it
calls `0x0040BAA0`, `0x004131A0` and `0x0043BAD0`, which is a mesh update.
**The `0x403` paint belongs to the object pass, not to bricks.**

Observation, the clock, from the state's own fields (`[state+0x2C]` the
percent, `[state+0x28]` the record cursor, `[state+0x3C]` the count):

```
   0.000s build  pct   9 cur 0 bricks 12  x 186.000 y 23.000
   0.692s build  pct  81 cur 1 bricks 12       <- the brick, cursor 0 -> 1
   0.776s build  pct  90 cur 0 bricks 11  x 192.000 y 34.000   (builder 1's
                                                  cycle 1: x -1, y +1)
```

and the record list, read out of the running process on all three builders:

```
  record list of 0x0296d5d8 vt 0x004b08dc (build): 2 entries
    [ 0] frame  81  fn 0x0041011c brick-stamp    arg 0
    [ 1] frame  90  fn 0x0040fbcc build-advance  arg 0
  animation strip 2 frames 16 divisor 1.0 rate 0.0
      => frames * 0.06 / divisor = 0.9600 s = 4.800 ticks
```

which is the static note's constructor table, entry for entry.

### The cycle

| run | span of eleven gaps | per cycle |
| --- | --- | --- |
| builder 1 | 10.557 s | 0.95973 s |
| builder 2 | 10.559 s | 0.95991 s |
| builder 3 | 10.558 s | 0.95982 s |

33 individual gaps, **min 0.956, max 0.969, mean 0.95982 s = 4.7991 ticks**,
against the read `16 × 0.06 / 1.0 = 0.960 s`. 0.02% apart. **4.8 ticks is not
an integer**, so `builderInterval` goes the way `basherInterval` and
`minerInterval` went in round 5, and `World::stepBuilder` samples the real
cycle through `bitesThisTick`: the bricks land on ticks 3, 8, 13, 18, 22, 27,
32, 37, 42, 46, 51, 56 of the state — gaps of 5, 5, 5, 4, 5, 5, 5, 5, 4, 5, 5,
eleven gaps spanning 53 ticks, averaging 4.82. An interval of 5 would span 55
and an interval of 4 would span 44.

### The enter seek — the `16.0` is one frame, and it matters

READ, then checked. The enter hook loads the strip's duration and divides it:

```
0040fb99  e8 66 8f 05 00     call 0x468b04        ; frames * 0.06 / divisor
0040fb9e  dc 35 f8 08 4b 00  fdiv [0x4b08f8]      ; the double 16.0
0040fbb6                     ... to the same [+0x2C] advance method
```

so the enter advances the animation clock by **one sixteenth of a whole
cycle**, which is 6.25% whatever the frame count is (for a 16-frame strip it
is also exactly one frame). MEASURED: the first sample after every click, about
25 ms in, read percent **9–10**, which is 6.25% plus 25 ms of a 9.6 ms-per-
percent clock; and the first brick came **0.692–0.694 s** after that sample,
i.e. **0.715 s** after the click against the **0.718 s** that
`(81 − 6.25)% × 0.960 s` predicts.

That is why `Rules` carries `builderEnterPercent = 6` as well as
`builderBrickPercent = 81`: without it, our bricks four onwards land a tick
late.

---

## The advance, and the refusal

`0x0040FBCC`, the percent-90 record, in the order it runs:

```
0040fbf6  fld  [eax+0x10]            ; Y
0040fc01  add  eax, [ecx+0x44]       ; + the vertical direction, +1 upright
0040fc1b  call 0x40df50              ; clamped
0040fc29  call 0x423275              ; stored back -- UNCONDITIONAL
0040fc35  call 0x42f22e              ; re-read the position: the NEW row
0040fc41  mov  byte [ebp-0x68], 0    ; the object flag
0040fcaf  mov  [ebp-0x14], 2         ; i = 2
0040fcc1  cmp  [ebp-0x14], 4         ;   while i < 4 -> columns p+2, p+3
0040fe07  call 0x427654              ; three object tests, OR'ed into the flag
0040fe28  jne  0x40fe85              ; flag set -> the object branch
0040fe77  call 0x40de60              ; else the cell at (p + facing, NEW row)
0040fe7c  cmp  dword ptr [eax], 0
0040fe7f  je   0x40ff24              ;   empty -> carry on
0040fee3  call 0x42f66b              ; TURN
0040fefb  call 0x42f8b9              ; leave the build state, return 1
0040ff24  ... X += facing ; then 0x0040FF93's decrement
00410028  call 0x42f2c3              ; A SECOND position, different accessor
00410091  call 0x40de60              ; one column ahead of THAT
00410096  cmp  dword ptr [eax], 0
00410099  je   0x4100db              ;   empty -> return 0, keep building
0041009e  call 0x42f66b              ; TURN
004100a6  call 0x42f8b9              ; leave the build state
```

Three things the static note did not have:

1. **The row the first probe uses is the row after the raise.** `0x0042F22E`
   is called at `0x0040FC35`, which is after `0x00423275` has stored the new
   Y, so the probe is `(p + facing, trunc(Y) + vdir)` — the cell the lemming
   is about to step into, not a cell beside the brick it has just laid. (It
   could not be the old row: the brick itself fills `(p + facing, old row)`,
   so no builder would ever lay a second brick.)
2. **The flag is an object test, not terrain.** `[ebp-0x68]` is the OR of
   three `0x00427654` calls against the two collections at `0x004C8968` and
   `0x004C8988` — the same lists `0x0043B150` walks — each given a record
   built from the columns `p + 2` and `p + 3` and the constant `0.5`. On that
   branch the engine moves X by `2·facing`, **lowers Y by 1.0**
   (`fsub [0x004B02D0]` at `0x0040FECF`), turns, and then **still** advances
   and decrements: an object ahead reverses a builder without ending its
   build. What `0x00427654` tests is unread.
3. **There is a second probe, past the decrement**, off a different position
   accessor (`0x0042F2C3`, not `0x0042F22E`), and that one also turns and
   leaves. This is the head-room check our `stepBuilder` always had by
   guesswork.

### Measured: the first exit, predicted before the click

`--min-bricks 2 --max-bricks 6` picks a builder whose staircase the live grid
says will run out of air, so the refusal is predicted rather than stumbled on.

**Left-facing**, start `(161.525, 23.000)`, predicted at most six clear
bricks:

```
  brick 1..6   2 cells each, rows 23..28, columns 159/160 down to 154/155
  brick 7  t  6.452s  count 6   ONE cell: (154,29)   -- (153,29) was already 0x1
  6.535s  build  pct 90  bricks 5   x 155.000 y 30.000
  7.501s  walk   pct 65            x 155.025 y 31.000
```

Cycle 8 stamped into `(153, …)` and `(152, …)`, both already the wall, so the
grid diff shows nothing — **the stamp is an `or`, so a brick laid into standing
terrain writes nothing at all**. Then percent 90 raised Y to 31, probed
`(153, 31)`, and that cell reads `0x00000001` — read back out of the live grid
afterwards, together with the whole of column 153 from row 23 to row 32. The
builder turned and walked, and **the counter stayed at 5**: the eighth brick
was never spent, which is the `0x0040FF93` decrement sitting past the branch.

### Observation, not explained: the second exit fired and the first did not

**Right-facing**, start `(185.182, 23.000)`, predicted at most six:

```
  brick 1  t 0.692s  count 12  (186,23) (187,23)
  brick 2  t 1.651s  count 11  (187,24) (188,24)
  brick 3  t 2.611s  count 10  ONE cell: (188,25)   -- (189,25) was already 0x1
  2.687s  build  pct 89  bricks 10  x 187.000 y 25.000  dir +1
  2.701s  walk   pct 33            x 187.970 y 25.000  dir -1
```

The first probe's cell would be `(188, 26)`, and that cell reads
**`0x00000000`** — read back afterwards, along with column 189 solid from row
23 to row 32 and column 188 empty above row 25. So this builder was **not**
stopped by the first probe. The second probe, one column further on at
`0x0042F2C3`'s biased position, would be in column 189, which is the wall; the
facing did reverse; and the count's last observed value was 10. That is
consistent with the second exit, and it is the honest limit of what this
session established: **the row `0x0042F2C3` biases to was not isolated live,
and the observed Y of 25 rather than the 26 the unconditional raise wrote is
not accounted for.** Recorded rather than guessed at. The candidates are
`0x0042F2C3`'s own bias and whatever `0x0042F8B9`'s exit state does to Y.

Two deliberate attempts to isolate the refusal on a lemming standing *against*
a wall (`--blocked`) failed for an aiming reason, not a reading one: the only
such lemming on MM_BUILD is the one pressed against the pen's left wall at
column 154 for a single tick, and `0x00454D6C`'s nearest-survivor pick named a
neighbour every time.

---

## Exhaustion — the shrug, and the divisor

MEASURED, both builders that reached twelve bricks:

```
  11.328s shrug   (builder 2)      12.293s walk      -> 0.965 s
  11.334s shrug   (builder 1 / 3)  12.300s walk      -> 0.966 s
```

READ, the shrug's step is slot `+4` of vtable `0x004B0C78`, which is
`0x0041AB96`, and the whole function is animation: it advances the controller
(`call [eax+0x2c]` at `0x0041ABB3`), accumulates the slice at `state+0x08`,
asks the controller whether the strip has finished (`call [eax+0x44]` at
`0x0041ABE3`) and on a yes calls `0x0042F8B9` at `0x0041AC06` and returns 1.
**No `0x0040DE60` anywhere in it, and no `0x0042E913`** — so the shrug has no
ground check of the kind the block step has, and `World::stepShrugger` does
not invent one.

And the length is a read: live, the shrug's controller holds **strip 10, 8
frames, divisor 0.5**.

> **This corrects the shape of round 5's clock formula, without contradicting
> any of its numbers.** Round 5 said one cycle is `frames × 0.06` and noted
> that the strip's divisor at `[strip+0x10]` cancels in the percent. It does
> cancel in the percent — but `0x00468B04` is `frames × 0.06 / divisor`
> (`fdiv [eax+0x10]` at `0x00468B56`), so the divisor stays in the
> **duration**. Bash, dig and mine all have divisor 1.0, which is why this
> never showed. The shrug has 0.5, and `8 × 0.06 / 0.5` = 0.96 s is the
> 0.966 s measured; `8 × 0.06` would be 0.48 s and is half of it. The whole
> live strip table is in `--probe`: strips 10 and 14 are the two whose divisor
> is not 1.0.

The shrugger stands on the brick it just laid: after the twelfth advance the
lemming is at `(p + facing, row + 1)`, and `(p + facing, row)` is brick
twelve's near cell.

---

## The level's animation strip table, live

Read off the controller at `lemming+0x128` (`[[+0x1F8]+0x0C] + [+0x1C8]·0x98`,
frame count at `[strip+0]`, divisor at `[strip+0x10]`), so a cycle is a read
rather than a fit:

```
 [0] 24f/1   [1] 16f/1   [2] 16f/1   [3] 18f/1   [4] 20f/1
 [5] 17f/1   [6] 17f/1   [7] 15f/1   [8] 14f/1   [9] 25f/1
[10]  8f/0.5 [11] 15f/1  [12]  8f/1  [13] 12f/1  [14] 16f/2
```

Identified by live observation, not by the alphabetical strip list:
**0 bash, 2 build, 4 dig, 7 fall, 9 mine, 10 shrug** (0, 4 and 9 from round 5;
7 off a falling lemming; 2 and 10 this round). The others are unidentified and
have **not** been filled in from the sheet's name order — that order would make
`build` index 2 and `dig` index 5, and dig is measured at 4, so the table is
not the alphabetical list.

---

## The direction filter is input, not a write

MEASURED, and it is what made the mirrored facing reachable. `assign_skill`'s
header already records that `0x00454D6C`'s hover pass tests the arrow-key
direction filters `[screen+0x16B1]` / `[screen+0x16B2]` against
`[lemming+0x40]`. Holding the key through the shim sets them:

```
  bytes [screen+0x16B0..3] idle    63000000
  RIGHT down ->                    63010000      ; +0x16B1 = 1
  LEFT  down ->                    63000100      ; +0x16B2 = 1
  UP / DOWN  ->                    unchanged
  both release ->                  63000000
```

so while RIGHT is held **only right-facing lemmings are pickable at all**, and
`[screen+0x16D8]` names one of them or nothing. `measure_build` holds the
matching arrow while it aims and clicks, and releases it before the sample so
nothing holds a key during the series.

**Why this was needed.** MM_BUILD puts twenty lemmings in one twenty-column pen
at row 23. Their stored screen rectangles (`lemming+0x128`) overlap, and
`0x00454D6C` names whichever survives its own nearest-by-world-distance pass:
in 24 aimed attempts across two processes, with six clean right-facing
candidates on screen, it named a left-facing neighbour **every time**. Round 5
recorded the same failure mode and lost four runs to it. The arrow key is the
engine's own answer, costs no write, and turned four failed runs into one that
worked first time.

---

## Clean negatives, and what was searched for

- **Nothing outside the watched window ever changed.** Every run diffed the
  whole 256 × 96 grid before and after; the "changes OUTSIDE the window" line
  printed zero every time. A brick touches two cells and nothing else.
- **No cell bit other than `0x1` was ever set.** Not `0x2`, not `0x400`, not
  `0x403`, not `0x800`. 72 of 72 writes were `0x00000000` → `0x00000001`.
- **`brickLength = 3` is excluded by the loop bound as well as by the diff.**
  `0x004101AB` is `cmp dword ptr [ebp-0x10], 3` against a counter that starts
  at 1, so the body runs for 1 and 2. There is no third iteration to find.
- **The builder is not on a tick interval and there is no `builderInterval`
  anywhere.** `0x004B08F8`'s `16.0` is the enter seek (above), the step's other
  constants are the `2.0 × 0.06` / `13.0 × 0.06` window at `0x004C55C0` /
  `0x004C55B0` and the `7.0 × 0.06` at `0x004C55B8`, and the only two things
  that fire per cycle are the two records at frames 81 and 90.
- **One level.** All three builders were on MM_BUILD, in three separate
  processes, at two different parts of the level and in both facings. MM_BUILD
  is the only level in reach that stocks the builder alone, the same position
  round 5 was in for bash (MM_BASH_002) and mine (MM_MINE). Stated rather than
  left as a gap.

---

## Harness notes

- **A level ends on you, twice in this session.** Two runs came back
  `ui_state 5, not in a level`: MM_BUILD's 300 s clock. No ESC was pressed.
  Restart the process; do not try to leave a results screen.
- **`find_menu_index` failed once in four entries** and correctly refused to
  press RETURN. Retried, and the second attempt entered. One in six still
  looks right.
- **`[state+0x2C]` can read above 100** — still true, still
  `0x0041B290` / `0x0041B2B2`. And **`[state+0x3C]` after the state is freed is
  whatever the allocator left**: the series prints `bricks 145` and
  `bricks 41387232` after the shrug, which is not the counter.
- **Aim at a candidate, then take whoever the engine names.** Insisting on the
  chosen lemming is right when the point is to test a per-lemming prediction;
  this round's geometry is read off the lemming's own live position afterwards,
  so any walker of the right facing will do, and accepting the engine's choice
  turned the hit rate around.
- `lrstate.attach` returns `(handle, pid)`. Still true.
- `goto_level.focus` was never called. Entry is `measure_float_splat.enter`
  (`select_level` + `restart_level.ensure_playing`), bracketed by the slot
  `[0][0]` restore, four times, all four printing `slot [0][0] restored: True`.

---

## Rules — what moved, and why

| field | before | after | why |
| --- | --- | --- | --- |
| `builderInterval` | 8 ticks | **gone** | 4.8 ticks is not an integer |
| `brickLength` | 3 | **2** | the loop runs i = 1, 2; 72 live cells in pairs |
| `builderBricks` | 12 (classic) | **12, measured** | `[state+0x3c]` counted 12 → 1, three builders |
| `builderCycleMs` | — | **960** | strip 2, 16 frames × 60 ms, measured 0.95982 s |
| `builderBrickPercent` | — | **81** | the first record's frame, 36 of 36 stamps |
| `builderEnterPercent` | — | **6** | the `16.0` at `0x004B08F8`: one sixteenth of a cycle |
| `shrugCycleMs` | — | **960** | strip 10, 8 frames × 60 ms ÷ 0.5, measured 0.966 s |

`State::Shrugger` is new, with `World::stepShrugger`. `bomberFuse` and
`explosionRadius` were not touched: they are round 7.

The stepper moved with the fields, because the series disagreed with it on
geometry as well as timing:

- `stepBuilder` filled **three** cells **including the lemming's own column**
  (`bx = dir > 0 ? l.x : l.x - brickLength + 1`, width 3); the original fills
  two, at offsets +1 and +2, and never the lemming's own cell.
- it moved X by `dir * brickLength / 2`, which happened to be one column for
  length 3 — right answer, wrong reason. It is one column because
  `0x0040FF2A` adds `[+0x40]`.
- it acted on `frame % 8`; the cycle is 4.8 ticks.
- its stop condition was a head-room check at `lemmingHeight`, with no
  citation. The measured first exit — the step-into cell, brick unspent — is
  now there; the head-room check is kept **below** it, explicitly labelled as
  the unmeasured second exit at `0x00410028`–`0x00410099`, because without
  some such test a builder walks its staircase into a ceiling.

### Within-tick simplifications, stated

1. **The brick (81) and the step up and forward (90) are 86 ms apart.** We do
   both on the tick that covers the brick — round 5's convention for bash's
   four bites.
2. **The first act lands on the tick that covers the brick, not at the brick.**
   The brick is 718 ms into the state and our tick boundaries are at 600 and
   800 ms, so our first brick is up to 82 ms late. The cadence after that is
   exact to the 200 ms grid.
3. **The shrug is 960 ms and our ticks are 200 ms**, so our shrug is five
   ticks = 1000 ms against the measured 966 ms.

---

## Tests

`test_world` gained `testBuilderFootprintAndCadence` and
`testBuilderRefusesAtAWall`, and `testSkillRules` gained the six builder
fields. **Expectations are written out, not derived from `Rules`** — the
round-4 lesson, with `testStepUpHeight` as the pattern.

Verified the way the facilitator verified rounds 4, 5 and 8: with
`brickLength` back to 3, `builderCycleMs` back to `8 * 200`,
`builderEnterPercent` back to 0 and the old `stepBuilder` body restored
(`frame % 8`, `fillBox(bx, l.y, brickLength, 1)`, `x += dir * brickLength / 2`,
head-room stop, straight to `Walker`), `test_world` fails **20 checks**, and
they are the behavioural rows and not only the field asserts:

```
  FAIL  twelve bricks were laid (10 seen)
  FAIL  every brick is 2 cells (first four: 3, 3, 3, 3)
  FAIL  24 cells of terrain in all (was 30)
  FAIL  bricks on ticks 3,8,13,18,22,27,32,37,42,46,51,56 (were 0,8,16,24,32,40,48,56,64,72)
  FAIL  brick 11 filled the column 11 ahead          (and 11/12, 12/12, 12/13)
  FAIL  the cell the builder stood in is still air
  FAIL  the builder advanced twelve columns (x 157 -> 0)
  FAIL  and rose twelve rows (y 79 -> 0)
  FAIL  the shrugger is standing on the brick it just laid
  FAIL  the shrug starts on the twelfth brick's tick (was -1)
  FAIL  the shrug lasts five ticks (was 0)
  FAIL  and it walks again on tick 61 (was -1)
  FAIL  bricks wrote 2, 2, 1, 1 cells (were 3,3,2,2)
  FAIL  three bricks were spent and the fourth was not (left 8)
  FAIL  brickLength is 2 cells (was classic 3)
  FAIL  build cycle is 16 animation frames of 60 ms
  FAIL  the 16.0 enter seek is a sixteenth of a cycle
```

Restored, the suite is **16 of 16** and `test_world` is 206 checks,
0 failures.

```powershell
.\scripts\Invoke-Ucrt64.ps1 "make -C src test"
```

---

## Left the tree

| file | change |
| --- | --- |
| `tools/harness/measure_build.py` | **new.** `--probe` / `--strips` read only; `--skill 4` is the series above; `--min-bricks` / `--max-bricks` / `--blocked` / `--facing` are the predicted cases. Every address is in its docstring. It imports round 5's `measure_dig` and teaches its maps about the build vtable at runtime rather than editing that file. |
| `src/sim/World.h` | `Rules`: `builderInterval` out; `brickLength` 3 → 2; `builderCycleMs`, `builderBrickPercent`, `builderEnterPercent`, `shrugCycleMs` in; `builderBricks` re-labelled measured. New `State::Shrugger` and `stepShrugger`. The round-6 comment block carries the addresses and the series. |
| `src/sim/World.cpp` | `stepBuilder` rewritten to the measured footprint, the millisecond cycle and the measured refusal; `stepShrugger` added; `stateName` and the step switch extended. |
| `src/__test__/test_world.cpp` | `testBuilderFootprintAndCadence`, `testBuilderRefusesAtAWall`, six rows in `testSkillRules`, all with written-out expectations. |
| `src/gfx/LemmingSprites.h` | `State::Shrugger` → the `shrug` strip. |

Not touched: `fallSpeed`, `floatSpeed`, `floatAfter`, `splatDistance`,
`walkSpeed`, `climbSpeed`, `ticksPerSecond`, `maxStepUp`, `lemmingHeight`,
`blockerFieldHeight`, round 5's cycle and footprint fields, `World::spawn`,
`findHatchRecords`, `Terrain::blockerField` and the walk probe's field test,
`src/assets/LevelStats.cpp`, `worldToMaskPixel`, `test_corpus`'s numbers,
`test_collisiongrid`, `PLAN.md`, `docs/PHASE3_HANDOFF.md`, and
`docs/scratch/phase3-round6.md`. The four open stepper differences (step-down
before falling, climber top-out, hoist hold, delayed float halving) are
untouched. Rounds 7 and 9 were not started.

One thing deliberately left alone: **a shrugger is not in `World::assign`'s
interruptible-state list**, so a click on one is refused. Whether
`0x0045A0E0` accepts a shrug was not read, and adding it would be a guess
either way.

## Left the game

**Killed.** No process is running at the end of this session; `Get-Process` for
`*Lemming*` returns nothing. Three processes were used, all on MM_BUILD
(pids 39724, 3300/25456 after restarts), each started by
`dump_grids.restart_game` and killed before the next entry. Four entries, four
`slot [0][0] restored: True`. No ESC was pressed from a results screen; the two
runs that found `ui_state 5` were answered with a restart.

The shim is installed and inert (`dinput.dll` in the game folder, `enable`
left at 0 by `LRInput.release()`). A fresh launch comes up at `ui_state 3`,
the main menu.

## For round 7, and whoever lands PLAN.md

- **The divisor belongs in the duration.** `frames × 0.06 / [strip+0x10]`.
  Round 5's three skills all had 1.0; strips 10 and 14 do not. Any round that
  times an animation should read the divisor as well as the frame count.
- **`0x00427654` and the object collections at `0x004C8968` / `0x004C8988`
  are the next unread thing on the builder's path**, and they are also where
  `0x00427554`'s `or 0x403` paint lives (`0x0043B150`). A builder that walks
  into one of those objects turns round and **keeps building**, which is a
  behaviour nothing in our sim has. That is round 9's territory, and round 9
  now has a concrete entry point.
- **The second build exit at `0x00410028` is read but not isolated.** It needs
  a level where a builder's head, not its feet, meets terrain — and
  `0x0042F2C3`'s bias read off a live lemming would settle it in one sample.
- **The arrow-key direction filter works.** Any later round that needs a
  particular facing out of a crowd should hold the arrow rather than aim
  harder.
- `0x00426DB0` and dig's `0.75` at `controller+0x1D8` are still unread, as
  round 5 left them. The build strip's rate reads 0.0.
