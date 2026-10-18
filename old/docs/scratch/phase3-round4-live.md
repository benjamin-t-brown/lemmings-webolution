# Phase 3, round 4 (live) — `maxStepUp` and `lemmingHeight`

2026-10-15. Question: each of the last two provisional geometry fields in
`Rules` is a classic-Lemmings number. Replace it with the constant the
original's own probe uses, or leave it provisional with the search written
down.

Both were replaced. **Both bounds are in hand on both fields, and both came
out as the static note `docs/scratch/phase3-round4.md` predicted** — which is
worth saying plainly, because that note was a hypothesis with no series and
this round could have contradicted it. It did not. It did contradict it on
one secondary point and add two things it missed; those are in "Where this
disagrees with the static note".

Script: `tools/harness/measure_stepup.py` (new). Stock exe
`C:\progs\Lemmings Revolution\Lemmings Revolution.exe`, SHA-256
`32675bdb…3737a2f`, the digest `read_steppers.py` checks. Suite **16 of 16**
green before and after; `test_world` went from 66 checks to 76.

---

## SUMMARY

| Field | Was | Now | Status |
| --- | --- | --- | --- |
| `maxStepUp` | 3 (classic) | **2** | READ AND MEASURED LIVE, both bounds, two levels |
| `lemmingHeight` | 4 (classic) | **3** | READ AND MEASURED LIVE, both of the climb's two probes, 12 of 12 twice |

**`maxStepUp` = 2.** The walk stepper `0x0041BDD4` loads no step height; the
step-up lives in the mask probe `0x0041BFFC` it calls at `0x0041BEE6`, and
that arm is unrolled exactly two deep with the third solid cell a wall
(`0x0041C47D`, `0x0041C48E`, `0x0041C517`, `0x0041C528`, `0x0041C5B1`,
`0x0041C5C1`). Live, with single-column pillars of exactly N solid cells
written into the original's own grid in front of its own walkers: on
**VH13_MT**, 230 adopted rises of exactly 1, **231 of exactly 2 in a single
step (`dy +2.000`)**, 114 turns at the pillar of 3, and nothing ever reached
the pillar of 4. Repeated on **E07_JC_1**: rise 1, rise 2, then 14 lemmings
turned at the 3.

**`lemmingHeight` = 3.** The climb head test `0x004139BF` loads no constant
of its own; its position comes from `0x0042F35F`, which adds `[+0x358]`, and
that double is `3.0` (`mov dword [ecx+0x35c], 0x40080000` at `0x0042E0FC`,
and again at `0x0042F616`). Live, with the climber byte `[lemming+0x48]`
written at birth and a 12-cell pillar carved at column 232 of E07_JC_1:

  * one ceiling cell at (231, 58) → **12 of 12 climbers hit head at feet
    Y = 55.000** (58 − 55 = 3), X jumping 232 → 231;
  * no ceiling, pillar top solid row 62 → **12 of 12 climbers' last climbing
    Y was 61.000**, never 60 and never 62, so the second probe reads Y + 2 =
    the bias minus one vertical unit, exactly as `0x00413B22` writes it.

Re-run, in order:

```
python tools/harness/measure_stepup.py --discriminating
python tools/harness/measure_stepup.py --grid-search
python tools/harness/measure_stepup.py --sites VH13_MT

# one fresh process per level -- there is no route back to the map
python tools/harness/measure_stepup.py --enter VH13_MT  --walk 45 --hz 60 \
       --carve 29:1,33:2,37:3,41:4 --carve-row 59
python tools/harness/measure_stepup.py --enter E07_JC_1 --walk 55 --hz 60 \
       --carve 216:1,224:2,232:3,240:4 --carve-row 51
python tools/harness/measure_stepup.py --enter E07_JC_1 --walk 50 --hz 60 \
       --carve 232:12 --carve-row 51 --cells 231:58 --poke-climb
python tools/harness/measure_stepup.py --enter E07_JC_1 --walk 50 --hz 60 \
       --carve 232:12 --carve-row 51 --poke-climb
```

`VH13_MT` and `E07_JC_1` are upper-case in the live table
(`goto_level.py --list`). The last two runs are one process: the second
re-enters from the results screen, which replays the same level, which is
what is wanted.

---

## Static, verified byte by byte (observation)

Everything below was disassembled this session with
`read_steppers.py --at <addr> --len <n>`, not copied from the earlier note.

### The walk stepper has no step height

`read_steppers.py --states` puts the walk at vtable `0x004B0D28`, stepper
`0x0041BDD4`, enter hook `0x0041BDBA`. Through its `ret 8` at `0x0041BF5E`
its only `.rdata` doubles are `2.0` (`0x004B03F8`, taken only when
`[lemming+0x50] == 3`), `0.0` (`0x004B0338`, the loop test) and `5.0`
(`0x004B10B0`, the speed). The handoff's standing instruction is right:
`maxStepUp` is not a double in the walk stepper.

What the stepper does per sub-step, read off the bytes:

```
0041be70  dd05b0104b00  fld   [0x004b10b0]   ; 5.0 world units a second
0041be76  dc4de8        fmul  [ebp-0x18]     ; * the slice in seconds
0041be7c  da4840        fimul [eax+0x40]      ; * facing
0041be7f  dc4208        fadd  [edx+8]         ; + X
0041be8d  call 0x40dec0                       ; wrap the column
0041be95  dd5908        fstp  [ecx+8]          ; -> X    (X moves FIRST)
0041bebc  8b45f8 ...    test  [ebp-8]          ; only on a consumed slice:
0041bece  call 0x49c5ac                       ;   truncate X
0041bedc  dd5a08        fstp  [edx+8]          ;   -> X, now an integer
0041bee6  e811010000    call  0x41bffc         ;   the mask probe
```

`[ebp-8]` is set at `0x0041BE4D` when the animation limited the slice, so the
probe runs **once per animation frame**, not once per rendered frame. X is an
integer at the instant the probe runs and is immediately moved off it again,
which is why a live sample almost never reads an integer X.

### The probe's step-up arm is two deep

`0x0041BFFC` has exactly one direct caller, `0x0041BEE6`. Through its `ret 4`
at `0x0041C872`:

```
0041c47a  8b55f0        mov edx, [ebp-0x10]    ; cell(X, Y)
0041c47d  83e201        and edx, 1
0041c482  0f8467020000  je  0x41c6ef           ; clear -> the step-DOWN arm
0041c48e  034844        add ecx, [eax+0x44]    ; row += vertical direction
...                                             ; blocker bit 0x800 turns first
0041c514  8b4da0        mov ecx, [ebp-0x60]    ; cell(X, Y+1)
0041c517  83e101        and ecx, 1
0041c51c  0f848c010000  je  0x41c6ae           ; clear -> ADOPT  (rise 1)
0041c528  034244        add eax, [edx+0x44]    ; row += direction again
...
0041c5b1  83e001        and eax, 1             ; cell(X, Y+2)
0041c5b6  0f84f2000000  je  0x41c6ae           ; clear -> ADOPT  (rise 2)
0041c5bc  8b4d08        mov ecx, [ebp+8]
0041c5bf  33d2          xor edx, edx
0041c5c1  8a5148        mov dl, [ecx+0x48]     ; the climber byte
0041c5c4  85d2          test edx, edx
0041c5c6  0f84aa000000  je  0x41c676           ; not a climber -> turn
0041c5cc  6a20          push 0x20              ; else new 0x20-byte state
0041c5ef  e85c71ffff    call 0x413750          ;  -> vtable 0x004B0960, climb
```

`0x0041C6AE` is the adopt path (`fild [ebp-0x1c]` → `0x0042EE74`, which
subtracts `[+0x368]` and stores Y). `0x0041C676` is the refuse path
(`0x0042F66B`, the turn, then return 0).

**There is no third `add`.** Over the whole function, `add ..., [reg+0x44]`
appears at `0x0041C48E`, `0x0041C528` and `0x0041C81B` only, and the last of
those is on the step-down arm; `sub ..., [reg+0x44]` appears at `0x0041C6FF`
and `0x0041C741` only. `[+0x44]` is the integer `1` written at spawn
(`0x0042DFE7`) and `0x0042F5E3` only negates it. So the greatest rise this
probe can write is 2 world units, and a stack of three is a wall.

The step-down arm, recorded so it is not folded in: `[ebp-0x18]` nonzero
(the cell under the feet) leaves Y alone; one empty cell with something under
it writes Y one unit down (`0x0041C6FF`, then `0x0041C809` adds the direction
back and stores through `0x0042EE74`, unless `[+0x4A]` is set); two empty
cells take the fall transition at `0x0041C77D` (`push 0x28`).

**The two cell tests are not the same test.** The step-up arm masks with 1
(`and edx, 1`); the step-down arm compares the whole dword with zero
(`cmp dword [eax], 0` at `0x0041C732` and `0x0041C774`); and the faller's
landing probe at `0x0041A99A` is a third (nonzero with bit `0x80` clear,
which is what `read_lvl_grid.solid` implements). `measure_stepup.step_solid`
and `any_solid` keep the first two apart.

### The climb's two probes

Climb stepper `0x004137CA`, slot 1 of vtable `0x004B0960`. It moves Y by
`5.0 * slice * [+0x44]` (`0x0041386C`, the same 5.0 as the walk — so the
climb is one world unit per 200 ms tick, which is `climbSpeed`), does **not**
truncate Y, prints `0x004BF2DC`, then at `0x004138E8` calls the head test —
the test's only direct caller.

`0x004139BF` through `0x00413C23` loads no `.rdata` double and has no
immediate 3 or 4. It gets a position from `0x0042F35F`:

```
0042f394  dd8150030000  fld   [ecx+0x350]
0042f39a  dc4008        fadd  [eax+8]          ; X + [+0x350]
0042f3a6  dd8058030000  fld   [eax+0x358]
0042f3ac  dc4210        fadd  [edx+0x10]       ; Y + [+0x358]
```

and then takes two one-cell samples off it:

```
004139f3  db4140        fild  [ecx+0x40]       ; facing
004139f6  dc6ddc        fsubr [ebp-0x24]       ; (X + [+0x350]) - facing
00413a09  call 0x49c5ac                        ; -> column
00413a11  dd45e4        fld   [ebp-0x1c]       ; Y + [+0x358]
00413a14  call 0x49c5ac                        ; -> row
00413a53  call 0x40de60                        ; cell(column, row)
00413a58  83380 0       cmp   dword [eax], 0
00413a5b  7513          jne   0x413a70         ; nonzero -> "Climber hit head"
                                               ;   (0x004BF2FC); 0x00413A79
                                               ;   writes X = X - facing, then
                                               ;   0x0042F66B turns
00413b0c  dd45dc        fld   [ebp-0x24]       ; X + [+0x350], NO facing term
00413b1a  call 0x49c5ac
00413b22  2b4244        sub   eax, [edx+0x44]  ; row = (Y + [+0x358]) - 1
00413b50  call 0x40de60
00413b55  83380 0       cmp   dword [eax], 0
00413b58  0f859f000000  jne   0x413bfd         ; nonzero -> keep climbing
00413b5e  6810f34b00    push  0x004bf310       ; zero -> "Climber topping out"
00413b8e  6a18          push  0x18             ;   new state via 0x00417C80
```

`[+0x358]` is the double `3.0`, IEEE `00 00 00 00 00 00 08 40`, written as a
pair of dwords in two places and nowhere else on this path:

```
0042e0f2  c78158030000 00000000   mov [ecx+0x358], 0           ; constructor
0042e0fc  c7815c030000 00000840   mov [ecx+0x35c], 0x40080000  ; = 3.0
0042f60c  c78258030000 00000000   mov [edx+0x358], 0           ; after a flip,
0042f616  c7825c030000 00000840   mov [edx+0x35c], 0x40080000  ;   if [+0x44]==1
```

and `[+0x360]` / `[+0x368]` (the walk probe's own offsets) are `0.0` upright
(`0x0042E109`, `0x0042E120`). `0x0042F5E3`'s other arm moves the `3.0` onto
`[+0x368]` when `[+0x44] != 1`.

So: head room is **3 above the feet**, top-out is **2 above the feet**, and
`3.0` is the only constant in sight. The climber's own column is the wall
column; the head sample is the open column behind it (`− facing`) and the
top-out sample is the wall column itself.

### The X bias — new this round

`0x0042F66B`, the turn the walk probe calls, writes more than the facing:

```
0042f675  8b4840        mov  ecx, [eax+0x40]
0042f678  6bc9ff        imul ecx, ecx, -1
0042f67e  894a40        mov  [edx+0x40], ecx
0042f684  837840ff      cmp  dword [eax+0x40], -1
0042f688  7530          jne  0x42f6ba
0042f68d  c78150030000 00000000   mov [ecx+0x350], 0
0042f697  c78154030000 0000f0bf   mov [ecx+0x354], 0xbff00000   ; [+0x350] = -1.0
0042f6a4  c78260030000 00000000   mov [edx+0x360], 0
0042f6ae  c78264030000 0000f0bf   mov [edx+0x364], 0xbff00000   ; [+0x360] = -1.0
0042f6bd  c78050030000 00000000   mov [eax+0x350], 0            ; facing +1:
0042f6c7  c78054030000 00000000   mov [eax+0x354], 0            ;   both 0.0
```

`[+0x360]` is the X bias the walk probe's position helper `0x0042F22E` adds,
so the probe column is `trunc(X) + 0` facing right and `trunc(X) − 1` facing
left, with X already moved by the facing. **Confirmed live** (below). The
static note recorded `[+0x350]`/`[+0x360]` as `0.0` from the constructor and
missed that the turn rewrites them.

---

## The offline prediction, made before the game was started

`measure_stepup.py --discriminating` walks the exe's own arm out of every
survivable hatch landing in the corpus and asks where a walker could reach a
stack of **exactly 3**, because a stack of 4 or more is a wall whether the
limit is 2 or 3 and says nothing. Four of 101 levels have one reachable:

```
  ticks  level      hatches  states   nearest (col,feet,dir,probe)   stacks seen
     32  VH13_MT          2     258   ( 56, 57,+1, 57)   {1: 58, 2: 36, 3: 5, 12: 4}
     58  E07_JC_1         1     278   (154, 48,+1,155)   {1: 27, 2: 12, 3: 5, 4: 1}
     63  VH01_JC1         1     191   (206,  8,+1,207)   {1: 9, 2: 48, 3: 2, 4: 1, 12: 2}
    169  VH21_JC          1     258   ( 58, 73,+1, 59)   {1: 12, 2: 6, 3: 2, 11: 1, 12: 1}
```

and `--grid-search` found exactly one hatch whose deterministic trajectory
reaches one on a survivable drop:

> **E07_JC_1**, hatch (25, 210, 56), lands row 50 (feet 51). A right-walking
> lemming reaches column 154 with feet 48 after ~201 ticks (≈40 s), having
> adopted a rise of 2 at column 135. At column 155 the ledge rises from row
> 47 to row 50 — a stack of exactly 3. `maxStepUp = 2` → it turns there;
> `maxStepUp = 3` → it rises to feet 51 and carries on right.

**That prediction was wrong about the level, and the reason is a limitation
of the offline walk, not of the hypothesis.** The live run (`e07_run1.txt`)
showed every lemming walking right from column 210 at a flat y = 51.000 and
leaving the actor list at about column 5 — the exit is in the way, 50 columns
along, and `walk_sim` knows nothing about exits. Recorded as a negative:
**the corpus's own geometry does not put a reachable exactly-3 rise in front
of a walker on any level this round could drive to.**

So the series was taken a different way.

---

## The method: pillars written into the original's own grid

A probe, labelled as one, in the same shape as round 3b's `[+0x49]` write. It
patches no code. It writes bit `0x1` — the bit the step-up arm tests — into
named cells of the live collision grid and asks the original's own arm what
it does with a stack of exactly N. Addresses from `read_grid.py`, which took
them off `0x0040DE60`: world object `0x004C6A10`, row count at `+0x418`,
data pointer at `+0x41C+0x0C`, 256-wide row-major `u32` cells, cell address
`base + ((row << 8) + (col & 0xff)) * 4`.

A **single-column** pillar resets itself, which is what makes a sequence of
them usable in one run: standing on a pillar of 1 the next column is a
one-cell step down, and on a pillar of 2 or 3 it is a fall of 2 or 3 world
units, well inside the measured `splatDistance` 30. And the pillars are
placed in increasing order along the walking direction, so if the 3 turns
them nothing ever reaches the 4 — which is itself a result.

Every run restores the cells it wrote (`restored N grid cells` in each log)
and brackets `select_level` with the slot [0][0] restore. The grid is reloaded
from the `.LVL` on every level load in any case.

### Series 1 — VH13_MT, the second level, untouched floor

Prediction written down first: floor row 58 for columns 26–43, walkers at
feet 59 out of the 50-count hatch at column 26. Pillars of 1/2/3/4 at columns
29/33/37/41 above feet row 59. `maxStepUp = 2` → rise 1 at 29, rise 2 at 33,
turn at 37, nothing reaches 41.

```
carved [(29, 1), (33, 2), (37, 3), (41, 4)] at feet row 59: 10 cells written
  col 29: cells from row 59 = ['0x1', '0x0']
  col 33: cells from row 59 = ['0x1', '0x1', '0x0']
  col 37: cells from row 59 = ['0x1', '0x1', '0x1', '0x0']
  col 41: cells from row 59 = ['0x1', '0x1', '0x1', '0x1', '0x0']

60909 object reads in 45.0 s = 1354 /s
restored 10 grid cells

rises adopted while walking (dy on a column step): {1: 230, 2: 231}
solid cells in the probe column at a turn:          {3: 114, 12: 94}
largest rise surmounted = 2   -> maxStepUp >= 2
smallest wall refused   = 3 solid cells   -> maxStepUp <= 2

  col  29 pillar 1:  rose 230 [1]   turned 0
  col  33 pillar 2:  rose 231 [2]   turned 0
  col  37 pillar 3:  rose 0 []      turned 114
  col  41 pillar 4:  rose 0 []      turned 0
```

Every one of the 461 rise events was at a carved pillar, the 94 turns at a
12-stack are the level's own wall at column 14 (the control), and **no
lemming ever reached column 41 in 45 s**. Raw samples:

```
  t= 10.267  y 60.000  x  29.083  facing +1  dy +1.000   ROSE 1
  t= 11.067  y 61.000  x  33.073  facing +1  dy +2.000   ROSE 2
  t= 11.967  y 59.000  x  36.947  facing -1  dy +0.000   TURN +1->-1
                                   feet 59 probe candidates [(37, 3), (38, 0)]
  t= 12.567  y 61.000  x  33.922  facing -1  dy +2.000   ROSE 2
  t= 13.467  y 60.000  x  29.942  facing -1  dy +1.000   ROSE 1
  t= 16.467  y 59.000  x  15.063  facing +1  dy +0.000   TURN -1->+1
                                   feet 59 probe candidates [(13, 12), (14, 12)]
```

Note `dy +2.000` in a **single** sample at 60 Hz: the arm adopts `feet + 2`
in one probe, it does not take two one-cell steps.

### Series 2 — E07_JC_1, the same shape, a different level

Prediction written down first: floor row 50, walkers at feet row 51 marching
right from column 210; pillars of 1/2/3/4 at 216/224/232/240 above row 51.

```
  col 216 pillar 1:  rose (y 51 -> 52 at x 216.019)   turned 0
  col 224 pillar 2:  rose (y 51 -> 53 at x 224.044)   turned 0
  col 232 pillar 3:  rose 0                           turned 14
  col 240 pillar 4:  rose 0                           turned 0
```

```
  t= 14.083  y 52.000  x 216.019  facing +1  dy +1.000
  t= 15.683  y 53.000  x 224.044  facing +1  dy +2.000
  t= 17.367  y 51.000  x 231.945  facing +1  dy +0.000
  t= 17.383  y 51.000  x 231.950  facing -1  dy +0.000  TURN +1->-1
```

Fourteen lemmings turned at the pillar of 3; none reached the 4. The
pre-turn sample at `x 231.945` facing `+1` and the post-turn sample at
`x 231.950` facing `−1` bracket the probe instant at `X = 232.000`: X is
truncated to 232, the probe reads column 232, the arm refuses, the facing
flips and X starts back down. **The lemming never leaves the wall column
after a refusal** — X was written before the probe ran (`0x0041BE95`) and the
refuse path does not put it back.

That run also gave rises of 1 and 2 on the level's **untouched** terrain,
after the turned lemmings walked back left:

```
  t= 30.283  y 52.000  x 167.954  facing -1  dy +1.000   ROSE 1
  t= 30.483  y 53.000  x 166.944  facing -1  dy +1.000   ROSE 1
  t= 30.683  y 54.000  x 165.969  facing -1  dy +1.000   ROSE 1
  t= 33.433  y 51.000  x 152.969  facing -1  dy +2.000   ROSE 2
```

and those four samples are the live confirmation of the **X bias**. The
stored grid's feet rows are: column 167 → 52, 166 → 53, 165 → 54, 152 → 51
(from 49). A lemming whose X truncated to **168** adopted column **167**'s
row; at 167 it adopted 166's; at 166 it adopted 165's. The probe column
facing left is `trunc(X) − 1`, not `trunc(X)`. Facing right it is `trunc(X)`
— the rise at `x 216.019` is column 216's pillar. Same pattern on VH13_MT
(`x 33.922` facing left adopted column 33's pillar).

---

## `lemmingHeight`: the climb, with the climber byte written

The parasol panel click is still not landed (round 3b's open negative), so
the climber came the same way round 3b's floaters did: a write of
`[lemming+0x48] = 1`, the byte `0x0041C5C1` reads, so the wall arm builds the
climb state at `0x00413750` instead of turning. A probe, labelled as one. It
worked on the first attempt and `measure_dropoff.lemming_slots` was never
needed, so **the `is_lemming` box was not touched and remains the blocker it
was**.

### Observation — the head test samples 3 above the feet

Prediction written down first: a 12-cell pillar at column 232 above feet row
51, and one ceiling cell at column 231 row 58 (the open column behind the
wall, which is where `0x004139BF` looks). The head test probes
cell(231, Y + bias). bias 3.0 → hit head at feet **Y = 55**; bias 4.0 →
Y = 54; bias 2.0 → Y = 56.

One climber in full, sampled at 60 Hz:

```
  t=  6.633  y 51.000  x 231.915  facing +1  climb 1   (walking)
  t=  6.650  y 51.020  x 232.000  facing +1  climb 1   dy +0.020  <- probe at X=232
  t=  6.850  y 51.990  x 232.000                        (boundary, Y = 52)
  t=  7.050  y 53.000  x 232.000                        (boundary, Y = 53)
  t=  7.250  y 54.005  x 232.000                        (boundary, Y = 54)
  t=  7.433  y 54.905  x 232.000  facing +1  climb 1
  t=  7.450  y 54.938  x 231.000  facing -1  climb 1   dx -1.000  TURN +1->-1
  t=  7.467  y 54.678  x 231.000  facing -1            dy -0.260  (falling, 20/s)
```

X stays exactly 232.000 for the whole climb. Y rises continuously at 4.96
world units a second (51.02 → 54.905 in 0.783 s), which is the 5.0 at
`0x004B10B0` and one world unit per 200 ms tick — `climbSpeed` again, not
remeasured, just not contradicted. The animation boundaries are visible at
t ≈ 6.85, 7.05, 7.25, 7.45 with Y at 52, 53, 54, **55**, and the head test
fires on a boundary. The sample at 7.450 reads 54.938 because the fall had
already started by the time it was taken; the boundary itself is Y = 55.000,
one boundary after 54 and 0.2 s after it.

`58 − 55 = 3`. All twelve climbers in the window:

```
  t=  7.450  y 54.938  x 231.000   t= 25.183  y 54.857  x 231.000
  t= 11.000  y 54.865  x 231.000   t= 28.733  y 54.784  x 231.000
  t= 14.550  y 54.814  x 231.000   t= 32.267  y 54.973  x 231.000
  t= 18.100  y 54.742  x 231.000   t= 35.817  y 54.920  x 231.000
  t= 21.633  y 54.929  x 231.000   t= 39.367  y 54.828  x 231.000
                                   t= 42.917  y 54.755  x 231.000
                                   t= 46.450  y 54.943  x 231.000
```

Twelve of twelve, spread 54.742–54.973 — all inside one fall-frame of
55.000, and none of them anywhere near 54.000, which is where a bias of 4
would have stopped them a whole tick earlier. The `dx -1.000` on every one of
them is `0x00413A79` writing `X = X − facing`, and the facing flip after it
is `0x0042F66B`.

### Observation — the top-out test samples 2 above the feet

Same pillar, no ceiling cell. Pillar top solid row 62. Prediction written
down first: the top-out probe is cell(232, Y + 2), so it fires at **Y = 61**;
at Y + 3 it would fire at Y = 60, at Y + 1 at Y = 62.

```
  t= 40.551  y 60.958  x 232.000  facing +1  climb 1
  t= 40.567  y 61.000  x 232.000  facing +1  climb 1   dy +0.042
    (0.834 s with Y and X both held -- the hoist)
  t= 41.401  y 63.000  x 232.028  facing +1  climb 1   dy +2.000
  t= 41.584  y 63.000  x 232.933  facing +1            (walking on top)
  t= 41.601  y 62.867  x 233.000  facing +1  dy -0.133 (off the 1-wide pillar)
```

Twelve of twelve: `y 61.000 x 232.000` appears for all twelve climbers,
`y 62.000 x 232.000` and `y 60.000` as a last climbing Y appear for none, and
all twelve then show the `dy +2.000` to `y 63.000`. `0 TURN` events in the
whole run, so no climber hit a head when there was no ceiling — the control
holds.

So the two probes are one cell apart and the single constant behind both is
`3.0`. `lemmingHeight = 3`.

---

## Where this disagrees with the static note

`docs/scratch/phase3-round4.md` is right on both headline numbers and on
every address it cites that this round re-read. Three corrections and
additions:

1. **It missed the X bias.** It recorded `[+0x350]` and `[+0x360]` as `0.0`
   "at construction" and concluded "the probe row is the lemming's own `Y`"
   and the column its own X. The constructor values are right, but
   `0x0042F66B` rewrites both to `−1.0` whenever the facing becomes `−1`
   (`0x0042F697`, `0x0042F6AE`) and back to `0.0` when it becomes `+1`
   (`0x0042F6BD`). Measured live on two levels. The asymmetry is real and is
   the kind of thing the rules say to preserve, not smooth out.
2. **It read the step-down arm's direction backwards in one sentence.** It
   says "One empty cell with something under it writes `Y` one unit down" and
   cites the `add` at `0x0041C81B`. The `add` is right and so is the
   conclusion, but the mechanism is that `[ebp-0x1c]` has already been
   decremented twice by `0x0041C6FF` and `0x0041C741`, and `0x0041C81B` adds
   one unit back before the store. Worth stating because the same `add` on the
   same field is *not* a third step-up.
3. **It treated the head test as the whole of the climb's geometry.** There
   are two probes, and `0x00413B22` makes the second one a cell lower. Our
   `stepClimber` uses one field for both; see "Left open" below.

It was also right to keep both numbers out of `Rules` until a series existed.
They are in now.

---

## Harness notes

  * New: `tools/harness/measure_stepup.py`. Offline modes
    (`--discriminating`, `--grid-search`, `--sites`) need no process. Live
    modes attach, enter through `goto_level.select_level` +
    `restart_level.ensure_playing` bracketed by the slot [0][0] restore, and
    never call `goto_level.focus` — `goto_level.py` is **unmodified**.
  * `lrstate.attach` returns `(handle, pid)`, **not** `(pid, handle)`. Getting
    that backwards costs a whole run: `select_level` then reads memory with a
    pid as a handle and reports "0 entries in the live level table", which
    reads like a level-table problem and is not one.
  * `walk_sim` / `--grid-search` / `--discriminating` do not model exits,
    hazards, or gadgets. They are candidate finders. E07_JC_1's walkers
    reached its exit 50 columns along and never got near the 201-tick
    prediction.
  * The `--carve` / `--cells` restore list had a bug on the first climb run
    (`saved = carve(...)` clobbered the ceiling entry appended before it), so
    cell (231, 58) was left written in that process. Harmless — the grid is
    reloaded from the `.LVL` on every level load, and the top-out run that
    followed saw no head hits, which is the proof — and it is fixed
    (`saved += carve(...)`).
  * `find_menu_index` found its candidate first try on all four entries this
    session. No blind RETURN. No ESC was pressed at all.
  * Changing level still means restarting the process; re-entering the *same*
    level from the results screen works and was used for the two climb runs.
  * Four live runs, ~160 s of play, sample rates 262–1354 object reads/s. The
    low rate is the one with 25 live lemmings; the per-object read is the same.

## Rules

Two values moved, each with its addresses and its series in the field comment
in `src/sim/World.h`:

  * `maxStepUp` 3 → **2**
  * `lemmingHeight` 4 → **3**

The "PROVISIONAL from here down" banner and the halved-guess note above those
fields were amended to say these two are no longer on that list.

`test_world` **gained** a check rather than having one relaxed:
`testStepUpHeight` builds pillars of 1, 2, 3 and 4 on a flat floor and
asserts the four outcomes the original gave — rise to 78, rise to 77, turn,
turn — with the numbers written out rather than derived from
`Rules::maxStepUp`, because deriving them makes the test pass at any value of
the field (it did, on the first draft; the fix is in the comment). It fails
with `maxStepUp = 3`, verified by flipping the field and re-running. Suite
**16 of 16**, `test_world` 76 checks, 0 failures.

## Left open, with citations — not this round's to change

  * **The climber's top-out is one cell lower than ours.** `0x00413B22`
    samples `Y + 2` where the ceiling arm samples `Y + 3`, and twelve of
    twelve climbers topped out at Y = 61 over a top solid row of 62.
    `World::stepClimber` uses `lemmingHeight` for both, so with the field now
    3 the ceiling check matches and the top-out is one cell high.
  * **The hoist is about four ticks and lifts 2.** Live: Y and X held for
    0.834 s at (61.000, 232.000), then feet at 63.000 with X still on the wall
    column. `World::stepHoister` waits 2 ticks, moves X by the facing, and
    lifts `lemmingHeight`.
  * **The walker's step *down* is one cell, then a fall.** `0x0041C6FF` /
    `0x0041C741` / `0x0041C77D`. `World::stepWalker` has no step-down arm: it
    moves at constant Y and enters `Faller` whenever the next cell is
    unsupported.
  * **The probe column is asymmetric in the facing** (`[+0x360]` = `0.0`
    right, `−1.0` left). `World::stepWalker` probes `l.x + dir` either way.
  * **The walk probe turns on the blocker bit `0x800`** before either cell
    test: `and eax, 0x800` at `0x0041C430`, `and edx, 0x800` at `0x0041C4C9`
    with `cmp [eax+0x4c], 3` at `0x0041C4D6`, and `and ecx, 0x800` at
    `0x0041C563` with `cmp [edx+0x4c], 3` at `0x0041C570` -- each falling
    through to `0x0042F66B` unless `[+0x4c]` is 3. That is round 8's, named
    here only because it sits inside the function this round read.
  * `0x0042F1C6`, the fourth position helper, was seen in the `[+0x350..0x368]`
    store scan and not read. Nothing this round needed it.

## Left the game

Killed and relaunched at the end of the session, so it is sitting at a fresh
**main menu**: pid **47992**, `ui_state` **3**, frontend screen id
`0x004C416C` = **3**, nothing loaded (`loaded ''`), no level entered. Level
table reports **102 entries** with **MM_CLIMB_002 at group 0 slot 0**, its own
record, so all four `--enter` calls this session put their swap back
(each printed `slot [0][0] restored: True`). Every carved grid cell was
restored in-process as well, and in any case the grid is reloaded from the
`.LVL` on each level load. The last level actually played was
`Compiled\E07_JC_1.lvl`.

The process this round was handed, **pid 4144**, was **already gone** when the
session started — no `Lemmings Revolution` process existed. It was launched
fresh as **pid 18276** (E07_JC_1, the plain walk and the four-pillar carve),
then restarted for each level change: **25208** (VH13_MT), **22944**
(E07_JC_1, both climb runs), and **47992** as left. No process died on its
own this session.
