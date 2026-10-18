# Phase 3, round 3b — the umbrella and the splat

2026-10-15. Question: `floatSpeed`, `floatAfter` and `splatDistance` were
binary reads with no live series. Get a series, on a drop long enough to
reach them, or a cited disagreement.

`Rules` **values were not changed.** All three agree with the read. The
comments on `floatSpeed`, `floatAfter`, `splatDistance` and `fallSpeed` in
`src/sim/World.h` were re-labelled from READ to READ AND MEASURED LIVE with
the series behind them; no number moved and `test_world` was not touched.
Suite is **16 of 16** green before and after.

Script: `tools/harness/measure_float_splat.py` (new). Stock exe
`C:\progs\Lemmings Revolution\Lemmings Revolution.exe`, SHA-256
`32675bdb…3737a2f` as `read_steppers.py` checks it.

---

## SUMMARY

| Field | Value | Status after this round |
| --- | --- | --- |
| `fallSpeed` | 4 | MEASURED again on two more levels: −4.000 world units per 200 ms tick, 18 consecutive ticks on MM_H_002 (column 113, Y 88 → 18.000) and 11 on VH13_KF_1. Unchanged. |
| `floatSpeed` | 2 | **MEASURED.** The step halves from −4.000 to −2.010 ± 0.03 a tick, 14 consecutive ticks, on the tick `[lemming+0x4b]` flips. Unchanged. |
| `floatAfter` | 10 | **MEASURED.** The umbrella latch `[state+0x20]` is set on the sub-step where the fall accumulator `[state+0x18]` reaches 10.0, at exactly 10.000 world units fallen. Unchanged. |
| `splatDistance` | 30 | **MEASURED, by probe + observation.** Accumulator held at 29 → lemmings survive a 75-unit drop; held at 31 → they die at the untouched rate. Unprobed on a second level: a 28-unit hatch drop kills nobody. Unchanged. |
| the floater never splats | - | **CONFIRMED.** `0x00416615` / `0x0041661A`. On E10_KF_1's 74-unit drop every lemming that left the actor list was a control (11 of 11, then 17 of 17 on a re-run after the script was patched); of the 15 and then 13 whose `[+0x49]` was written, **none**. |
| faller vtable `0x004B0A20` | — | **CONFIRMED LIVE**, closing Round 2's open negative. Heap objects with that first dword and a `+0x18` that climbs one per world unit of descent. |
| new, unmodelled | — | The *speed* halves ≈ 9.6 world units (0.48 s) **after** the umbrella opens, because `[+0x4b]` is set when the `umbopen` animation ends, not when the distance passes 10. Our stepper halves immediately. No field for it; left for a later round. |

Re-run, in order:

```
python tools/harness/measure_float_splat.py --grid-search
python tools/harness/measure_float_splat.py --enter e10_kf_1 --watch 26 --hz 120 \
       --poke-above 78 --poke-float 82.5 --poke-every 2
python tools/harness/measure_float_splat.py --enter e10_kf_1 --states 28 --hz 150 \
       --poke-float 82.5 --poke-above 78
python tools/harness/measure_float_splat.py --enter e10_kf_1 --states 24 --hz 120 --clamp-acc 29
python tools/harness/measure_float_splat.py --enter e10_kf_1 --states 24 --hz 120 --clamp-acc 31
python tools/harness/measure_float_splat.py --enter VH13_KF_1 --states 22 --hz 120
python tools/harness/measure_float_splat.py --enter MM_H_002  --watch 14  --hz 120
```

The live level table spells E10_KF_1 **lowercase** (`e10_kf_1`); the lookup
in `goto_level.select_level` is case-sensitive. `VH13_KF_1`, `MM_H_002` and
`MM_FLOAT` are upper-case there. `--list` prints the live spellings.

---

## How the drop was found, before the game was started (observation)

Nothing on record had a drop longer than `splatDistance`. The stored
collision grid answers it offline: `read_lvl_grid.find_grid` gives the
256 × 96 cells, `measure_spawn.hatch_words` gives each hatch's
`(count, x, y)`, and the drop for a hatch is `y` minus the highest solid row
below `y` in column `x` (`read_lvl_grid.solid` is the faller's own landing
test from `0x0041A99A`: non-zero and bit 0x80 clear).

`measure_float_splat.py --grid-search` prints it. **6 of the 144 hatch
records in the corpus drop further than 30**, and four of them are on one
level:

```
  drop  level        count  spawn      lands on row
    76  E10_KF_1        20  ( 94, 82)  6
    75  E10_KF_1        13  (112, 82)  7
    73  E10_KF_1        10  (130, 81)  8
    71  MM_H_002         1  (113, 88)  17
    71  E10_KF_1         7  ( 74, 82)  11
    66  VH01_JC1         5  ( 29, 74)  8
    29  VH13_KF_1       50  ( 70, 58)  29   <- the next one down, under 30
```

**E10_KF_1** is "LEMTRIS", 180 s, 50 lemmings, save 50, and its skill stock
(`build/probe_skills_corpus.py`) is **floater 50, builder 2** — fifty
umbrellas for fifty lemmings that all have to be saved. So the level's own
design predicts a 71-unit drop is fatal without one. That was written down
before the game was launched, and it is what the series then tested.
**VH13_KF_1** is the complement: one hatch, 50 lemmings, a 29-unit drop,
just under the threshold.

Grid predictions for the walk-off series (same script, same grids):
MM_H_002 column 104 is solid at row 61 and column 105's next solid below is
row 17, so a walker going right off column 104 falls 44 units; and
MM_H_002's single long hatch is column 113, floor at row 17.

---

## The method: the actor list, not a billboard

Rounds 1 and 2 tracked a lemming through its drawn billboard quad and lost
the lock mid-fall. Round 3 found the actor vector, and this round reads
positions straight out of it, which removes the tracker entirely:

    0x004CBB18 + 0x00  u32     count
    0x004CBB18 + 0x0C  u32     pointer array
    lemming + 0x00     u32     vtable, 0x004B10E8 (or 0x004B0F6C)
    lemming + 0x08     double  X, unrolled column
    lemming + 0x10     double  Y, world units, up
    lemming + 0x44     int     direction, the `fild` at 0x00416213
    lemming + 0x49     byte    has-floater  (read at 0x00416615, 0x00416128)
    lemming + 0x4b     byte    float animation running (set 0x004162C2,
                               cleared 0x0041602B; the flag 0x00416038 halves on)

Each sample is one `ReadProcessMemory` of one object, addressed by pointer.
**Round 2's tracker gaps do not exist here**, and no step in this note comes
from dividing a gap. Sample rates reached 1198 object reads a second.

Two traps this method has of its own, both now handled in the script:

  * **A pointer is reused.** A lemming freed on a splat hands its address to
    the next one born, and appending across that reads as a 72-unit jump
    *upwards* in the middle of a series. The series key is
    `(pointer, generation)` and a reappearance starts a new series.
  * **Dedup on Y alone leaves holes that look like gaps.** A lemming walking
    a flat ledge holds one Y for seconds. On MM_H_002 that put an
    eleven-second hole in the middle of one series (y 62.000, column 51 →
    column 105) which is a *walk*, not a hold. The key is now `(Y, X)`.

A third point of method, stated plainly because it is not an observation:
**the two probes write memory.** `--poke-float` writes 1 into
`[lemming+0x49]`, the byte the umbrella test and the splat test both read.
`--clamp-acc` holds `[state+0x18]`, the accumulator both compares read.
Neither patches code; both ask the original's own branches what they do with
a given input. Everything labelled "observation" below was read with no
writes at all.

---

## What the binary says (static, `read_steppers.py --at`)

The fall stepper is `0x004160BD`, a method on the fall **state** with the
lemming at `[ebp+8]`, the state at `[ebp-0x38]`, and a pointer to the
seconds remaining at `[ebp+0xc]`. Per outer-loop iteration it takes a
sub-step, moves Y, and only then tests and increments:

```
00416038  (accessor)  mov cl,[eax+0x4b] / test / je
0041604a              fld   [0x004b0a18]   ; 20.0
00416050              fdiv  [0x004b03f8]   ; 2.0, only if [+0x4b]
0041607b              fmul  [ebp+0xc]      ; * seconds  -> world units
00416213  fild [eax+0x44] / fmul / fsubr [edx+0x10]      ; Y -= dir * that

0041612F..00416186    if [+0x49] && [state+0x20] && ![+0x4b]:
                      [ebp-0x18] = time left in the running animation
                      (and prints "Time to end of umbrella %2.3f", 0x004BF35C)
004161D6              [ebp-0x24] = 1 when that time is what limited the step
004162B2              push 0x004BF37C ("float") ; set animation
004162C2              mov byte [ecx+0x4b], 1    ; <- the halving flag

004162D6              fld   [eax+0x18]     ; distance fallen
004162D9              fcomp [0x004b03e8]   ; 10.0
004162E4              jne   -> skip        ; i.e. skip while fallen < 10
004162EB              mov cl,[ecx+0x49]    ; has-floater
004162FE              push 0x004BF384 ("umbopen") ; set animation
0041630E              mov byte [edx+0x20], 1      ; one-shot latch
00416315  fld [eax+0x18] / fadd [0x004b02d0] (1.0) / fstp [ecx+0x18]

004165DE  landing:    [ebp-0x10] is the landing cell's flags
004165E2              je  0x4166CB         ; cell 0 -> not landed
004165F3              jne 0x4166CB         ; flags & 0x80 -> not landable
004165FC              fld   [edx+0x18]     ; distance fallen
004165FF              fcomp [0x004b10a8]   ; 30.0
00416607  test ah,0x41 / jne 0x4166A8      ; C0 or C3: fallen <= 30 -> SURVIVE
00416615              mov cl,[eax+0x49]
0041661A              jne 0x4166A8         ; has-floater -> SURVIVE, always
00416620..0041664A    cell 0x200 with [+0x4c]==1, or 0x4000 with ==2 -> SURVIVE
0041664C              push 0x18 / call 0x49C1A8  ; new death state
00416688              call 0x0042E696            ; install it on the lemming
```

Three things fall out of that and are worth repeating because they decide
how the series is read:

1. The animation names are `fall` (`0x004BF354`), `umbopen` (`0x004BF384`)
   and `float` (`0x004BF37C`). The umbrella is a three-stage thing, not two.
2. `0x00416038` halves on `[+0x4b]`, the **animation** flag, and that flag is
   set when `umbopen` ends — not when the distance passes 10.
3. The splat threshold is strict: `test ah, 0x41` takes the survive jump on
   C0 (less) or C3 (equal), so a splat needs fallen **> 30.0**. That matches
   `World.cpp`'s `l.fallDistance > rules_.splatDistance` exactly.

The death state's constructor is `0x0040B3A0`: vtable **0x004B0668**, 0x1A at
+0x14, 0 at +0x11. That vtable is used below as the direct sighting of a
splat.

---

## Observation 1 — the fall is continuous at 20.0 world units a second

Not a 4-unit jump every 200 ms. Sampled at 60–150 Hz, Y moves every frame by
0.12–0.46, and the rate is 20.000 world units a second. E10_KF_1,
lemming `0x02808640`, hatch Y 82, column 74:

    y 81.620 at t=1.758  ->  y 65.120 at t=2.583   =  16.50 / 0.825 s = 20.00 /s

Sampled at the 200 ms tick it is the −4.000 Round 2 measured. Our sim is
integer and tick-based on purpose; the rate is the same and
`fallSpeed = 4` stands. **Do not read this as a reason to put floating point
in the stepper.** The fraction is the engine's own real-time accumulator,
exactly as `ticksPerSecond` already records.

## Observation 2 — the splat series, E10_KF_1 (no floater)

Control lemming `0x028187d0`, hatch Y word 82, never poked, sampled at
120 Hz and printed here at 200 ms:

| t (s) | y | Δy | Δt |
| --- | --- | --- | --- |
| 1.750 | 81.740 | — | — |
| 1.950 | 77.860 | −3.880 | 200 ms |
| 2.150 | 73.800 | −4.060 | 200 ms |
| 2.350 | 69.800 | −4.000 | 200 ms |
| 2.550 | 65.800 | −4.000 | 200 ms |
| 2.750 | 61.920 | −3.880 | 200 ms |
| 2.950 | 57.880 | −4.040 | 200 ms |
| 3.150 | 53.860 | −4.020 | 200 ms |
| 3.350 | 49.840 | −4.020 | 200 ms |
| 3.550 | 45.800 | −4.040 | 200 ms |
| 3.750 | 41.780 | −4.020 | 200 ms |
| 3.950 | 37.880 | −3.900 | 200 ms |
| 4.150 | 33.860 | −4.020 | 200 ms |
| 4.350 | 29.820 | −4.040 | 200 ms |
| 4.550 | 25.800 | −4.020 | 200 ms |
| 4.750 | 21.760 | −4.040 | 200 ms |
| 4.950 | 17.880 | −3.880 | 200 ms |
| 5.150 | 13.860 | −4.020 | 200 ms |
| 5.350 | 9.820 | −4.040 | 200 ms |
| (last raw sample) | **8.000** | | |
| 6.250 | — | **left the actor list** | |

Where the fall started: the hatch Y word, 82 — not the first sample. Round 3
established `World::spawn` writes that word, and `--watch` prints the words
out of the `.LVL` beside the series so the two can be compared. Where it
ended: y = 8.000, the top of the solid row 7 the grid gives for column 112.
**Distance fallen 74.0, and the lemming was gone 0.9 s later.**

Eleven lemmings did exactly that in one 26-second window; every one of them
vanished from the list within a second of touching its floor, at y 7.000,
8.000, 9.000 or 12.000 — the four floors the grid predicts for columns 94,
112, 130 and 74. The level then **ended early**, at clock 128 of 180, with
no lemmings left.

Direct sighting, not an inference from the list: with the window scanned for
vtable `0x004B0668`, the state the splat branch allocates, four of them
appeared at t = 3.496, 3.601, 3.705 and 3.750 — the moment the first batch
of lemmings reached the floor. (Only four, because the death states are
allocated outside the window this script scans; it is a sighting, not a
census.)

## Observation 3 — the splat series on a second level, MM_H_002

MM_H_002, hatch `(1, 113, 88)`, column 113 held constant for the whole fall,
lemming `0x0265f560`. Eighteen consecutive ticks:

    88 (hatch word) -> 87.880, 84.000, 79.960, 75.960, 71.920, 67.900,
    64.020, 60.000, 55.980, 51.960, 47.900, 44.020, 40.020, 35.960,
    31.940, 28.020, 24.000, 19.980, 18.000 (last raw sample)

Δy per 200 ms tick: −3.880, −4.040, −4.000, −4.040, −4.020, −3.880, −4.020,
−4.020, −4.020, −4.060, −3.880, −4.000, −4.060, −4.020, −3.920, −4.020,
−4.020. **Distance fallen 70.0**, landing on row 17 as the grid predicts for
column 113, and the lemming left the actor list at t = 5.425, 0.1 s later.

The same level also gives a **walk-off** of the same kind, which is a
different mechanism reaching the same branch. Lemming `0x02639470` was
released from the 44-count hatch at column 19, landed on row 77 after a
10-unit drop, walked down steps to y = 62.000, walked along that ledge from
column 51 to column 105 over eleven seconds, and went off the edge:

    62.000 -> 57.951, 53.771, 49.751, 45.731, 41.831, 37.791, 33.791,
    29.771, 25.831, 21.791, 18.000

Eleven ticks of −4.0, **44 world units fallen** (62 → 18), gone from the list
at t = 28.784. The grid predicted this edge before the run: column 104 solid
at row 61, column 105's next solid below at row 17, so 44.

## Observation 4 — the float series, E10_KF_1

Lemming `0x02808640`, hatch Y 82, column 74, `[+0x49]` written to 1 at
birth (y 81.740, 0.3 units into the fall), 120 Hz, printed at 200 ms:

| t (s) | y | Δy | +49 | +4b |
| --- | --- | --- | --- | --- |
| 1.750 | 81.740 | — | 0 | 0 |
| 1.950 | 77.860 | −3.880 | 1 | 0 |
| 2.150 | 73.800 | −4.060 | 1 | 0 |
| 2.350 | 69.800 | −4.000 | 1 | 0 |
| 2.550 | 65.800 | −4.000 | 1 | 0 |
| 2.750 | 62.160 | −3.640 | 1 | **1** |
| 2.950 | 60.140 | **−2.020** | 1 | 1 |
| 3.150 | 58.130 | −2.010 | 1 | 1 |
| 3.350 | 56.120 | −2.010 | 1 | 1 |
| 3.550 | 54.100 | −2.020 | 1 | 1 |
| 3.750 | 52.090 | −2.010 | 1 | 1 |
| 3.950 | 50.140 | −1.950 | 1 | 1 |
| 4.150 | 48.130 | −2.010 | 1 | 1 |
| 4.350 | 46.110 | −2.020 | 1 | 1 |
| 4.550 | 44.100 | −2.010 | 1 | 1 |
| 4.750 | 42.080 | −2.020 | 1 | 1 |
| 4.950 | 40.140 | −1.940 | 1 | 1 |
| 5.150 | 38.130 | −2.010 | 1 | 1 |
| 5.350 | 36.110 | −2.020 | 1 | 1 |

and on to y 13.760 at t = 7.583 — 61.68 → 13.76 is 47.92 units in 4.791 s =
**10.00 world units a second**, half of 20.00, measured over 24 ticks. The
step before the change is −4.000 and the step after is −2.000, and the tick
it changes on is the tick `[+0x4b]` goes 0 → 1. **`floatSpeed = 2` holds.**

Then the lemming **landed at y = 9.000 and walked away** — the series
continues through walking (Y stepping ±1.000 and ±2.000 per 200 ms at
columns 79 … 154), and `[+0x4b]` goes back to 0 when it later starts a
second fall, while `[+0x49]` stays 1. That is `0x0041602B` resetting the
animation flag on each new fall, and it is exactly our split between the
persistent `canFloat` and the per-fall `floating`.

## Observation 5 — `floatAfter` read off the latch

The number that matters is on the fall **state**, which the lemming object
does not point at: `--dump-lemming` shows the lemming is about 0x80 bytes,
everything past +0x80 zero, and +0x78 an embedded animator (vtable
`0x004B14A8`). Round 2's heap scan for `0x004B0A20` failed; it works, but
only while something is actually falling — with no lemming airborne the only
hits are the two image xrefs `0x00415FB4` / `0x00415FF4`, which are
instructions. `--find-state` and `--states` do the scan and then re-scan a
window round the hits, because the blocks are pooled and reused fall to fall.

E10_KF_1, state `0x02798350`, lemming `0x027987b0` poked at birth, hatch Y
82, 150 Hz:

| `[state+0x18]` | `[state+0x20]` | y | fallen | `[+0x4b]` |
| --- | --- | --- | --- | --- |
| 1 | 0 | 81.996 | 0.004 | 0 |
| 2 | 0 | 80.916 | 1.084 | 0 |
| 3 | 0 | 79.936 | 2.064 | 0 |
| 4 | 0 | 78.936 | 3.064 | 0 |
| 5 | 0 | 77.976 | 4.024 | 0 |
| 6 | 0 | 76.836 | 5.164 | 0 |
| 7 | 0 | 75.896 | 6.104 | 0 |
| 8 | 0 | 74.916 | 7.084 | 0 |
| 9 | 0 | 73.936 | 8.064 | 0 |
| **11** | **1** | **71.996** | **10.004** | 0 |
| 12 | 1 | 70.896 | 11.104 | 0 |
| … | | | | |
| 20 | 1 | 62.976 | 19.024 | 0 |
| 20 | 1 | 62.398 | 19.602 | **1** |
| 21 | 1 | 61.988 | 20.012 | 1 |

Read it as follows. The accumulator is `floor(distance fallen) + 1` — at
fallen 0.004 it is 1, at 10.004 it is 11 — because the loop takes a
zero-length first sub-step on a lemming born exactly on an integer Y, and
`0x00416315` adds 1.0 on every boundary. `0x004162D6` reads the accumulator
*before* that increment, so the 10.0 compare fires on the boundary where the
pre-increment value is 10, which is the boundary at **10.000 world units
fallen** — y 71.996 from a hatch Y of 82.0, the row the accumulator then
leaves reading 11. **`floatAfter = 10` holds, as a distance into the fall.**

A second lemming on the same run, from the Y-81 hatch, latched at
accumulator 11, y 70.885: 81 − 70.885 = 10.115. Same answer on a different
hatch.

The latch requires the floater. On VH13_KF_1 and on E10_KF_1's controls the
accumulator ran past 10 to 28 and to 74 with `[state+0x20]` **still 0**,
because `0x004162EB` reads `[lemming+0x49]` first.

### And the part our stepper gets wrong

`[+0x4b]` — the flag the *speed* branches on — was set at fallen 19.602, not
at 10.004. The gap is the `umbopen` animation: `0x0041612F`…`0x00416186`
clamps the step to the time left in it and `0x004162C2` sets the flag when
it ends. Measured two ways:

  * assigned at birth: `[+0x4b]` at fallen 19.62–19.70 over seven lemmings
    (and 19.68 from the Y-81 hatch), so 9.6–9.7 units after the umbrella
    opened at 10;
  * assigned mid-fall with the accumulator already past 10, so `umbopen`
    starts at the write: Δy from the first sample showing `[+0x49]` = 1 to
    the first showing `[+0x4b]` = 1 was 9.99, 9.99, 10.27 and 10.34 units —
    the excess over 9.6 being the distance to the next sub-step boundary,
    which is in [0, 1) by construction.

So `umbopen` lasts about **0.48 s**, ≈ 9.6 world units at 20/s, and the
original keeps falling at 4 a tick for those two and a half ticks after the
umbrella opens. `World::stepFaller` halves immediately. This is a real
difference and it is **not** a disagreement about `floatAfter`: 10 is where
the umbrella opens, which is what the field says. Encoding the delay needs a
field this round was told not to invent, so it is recorded here and nothing
was changed.

## Observation 6 — a floater never splats (`0x0041661A`)

E10_KF_1, one 26-second window, every lemming poked at birth or left alone,
alternately (`--poke-every 2`). Both groups fall the same 73–76 units down
the same columns in the same seconds.

| run | group | n | left the actor list |
| --- | --- | --- | --- |
| first | control, `[+0x49]` = 0 | 11 landed in-window | **11** |
| first | `[+0x49]` = 1 | 15 | **0** |
| re-run | control, `[+0x49]` = 0 | 13 | **17 departures, all of them** |
| re-run | `[+0x49]` = 1 | 13 | **0** |

Every pointer that left the list was on the control list; not one poked
lemming did, in either run. (17 departures against 13 controls in the re-run
because a freed pointer is handed on and departs again under the next
generation; the direction is not in doubt.) The poked ones landed at the same y 9.000 / 12.000 their
controls died on, and walked. `0x00416615` reads that byte and `0x0041661A`
jumps to the survive path when it is set, with no second look at the
distance — and 73 units is more than twice `splatDistance`. **The binary's
prediction holds.**

## Observation 7 — `splatDistance` pinned to 30 by holding the accumulator

A probe, labelled as one. `--clamp-acc V` writes V into `[state+0x18]` of
every live faller whenever it reads above V, so the landing branch sees V or
V+1 (the next sub-step adds 1.0 between the last write and the landing
test). That is why the probe is run at 29 and 31 rather than at 30.

E10_KF_1, 24-second windows, no floater anywhere, nothing else changed:

| probe | deaths in the first 12 s | deaths in 24 s | still listed at the end |
| --- | --- | --- | --- |
| nothing written (baseline) | 11 in the first 9 s | 22 | 4 |
| `--clamp-acc 29` (branch sees 29 or 30) | **1** | 11 | **20** |
| `--clamp-acc 31` (branch sees 31 or 32) | **13** | 27 | 4 |

With the accumulator held at 29, lemmings landing from a 75-unit drop stay
in the list and walk; the eleven that eventually went were all after
t = 16.5 s, by which time survivors had walked on and taken second falls
that the window did not always clamp. With it held at 31 they die from
t = 3.6 s at the untouched rate. Combined with `test ah, 0x41` surviving on
`<= 30.0`: the threshold is **30**, i.e. splat iff fallen > 30.

Unprobed confirmation on a second level and a second geometry —
**VH13_KF_1**, hatch `(50, 70, 58)`, one hatch, no writes of any kind:

    y 57.978 (acc 1) ... 31.978 (acc 27) -> 30.000 (acc 28)
    57.978 -> 31.978 is 26.000 units in 1.300 s = 20.00 /s

`0 lemmings left the actor list` over 22 s, 10 still listed, `[state+0x20]`
0 throughout (no floater). A **28-unit** fall onto row 29 kills nobody. With
E10_KF_1's 74 and MM_H_002's 70 and 44 all fatal, the live bracket on its
own is (28, 44]; the probe closes it on 30.

---

## Clean negatives

  * **A genuine panel assignment of the parasol was not landed.** The panel
    index static `0x004C9200` was re-measured on MM_FLOAT and
    `measure_dropoff`'s mapping is right — clicking the strip at x ≈ 38–54
    gives index k at y ≈ 27 + 40k (0 at 28, 1 at 67–69, 2 at 108, 3 at
    146–148, 4 at 187, 5 at 227, 6 at 268, 7 at 309, 9 at 388), and
    `panel_buttons` reports 28 drawn button rectangles in three strips, not
    ten. But `--find-parasol` walked all 28 and assigned nothing, because
    `measure_dropoff.lemming_slots` found no lemming-sized sprite in its
    screen box (cx 180–580, cy 220–420) on MM_FLOAT, so every lemming click
    was a no-op before it was sent. So this round's floaters are all
    `[+0x49]` writes. The byte is the one both branches read
    (`0x00416128`, `0x00416615`), and the write reproduces the documented
    three-stage `fall` → `umbopen` → `float` sequence exactly, but "a panel
    click sets that byte" is still **not shown live**. Whoever needs it
    should widen `is_lemming`'s box first, on a level where lemmings walk.
  * **The splat state was sighted, not counted.** `0x004B0668` objects were
    seen appearing as E10_KF_1's lemmings landed, but they are allocated
    outside the window `--states` scans, so four sightings against 22
    deaths is a coverage figure, not a rate.
  * **No "saved" or "dead" counter was found.** The statics that move during
    play are `0x004CBA78` (released, counts up), `0x004CBA8C` (left to
    release, counts down), `0x004CBAA0` (packed, moves), `0x004CBAAC` (the
    clock). Death is read here from the actor list plus the splat vtable.
    Finding the counter would make the splat series a one-liner.
  * `goto_level.focus()` was never called, and `goto_level.py` is
    **unmodified**. Entry goes through `select_level` + `restart_level
    .ensure_playing`, bracketed by the slot [0][0] restore, in
    `measure_float_splat.enter`. `ensure_playing` is used rather than
    `goto_level.enter` because it knows how to leave the results screen
    (ui_state 5), where `find_menu_index` has no menu to find and correctly
    refuses to press RETURN.
  * **There is still no way back to the map from inside a level.** RETURN
    from the results screen replays the *same* level whatever slot [0][0]
    says, so changing level means killing and relaunching the process. This
    session did that five times.

## Harness notes

  * `find_menu_index` found its candidate first try on every entry this
    session (6 of 6). No blind RETURN was pressed.
  * Entry is faster from a freshly launched process (ui_state 3, screen 3)
    than from a results screen.
  * E10_KF_1 ends at about clock 128 of 180 on its own, because all 50
    lemmings splat. Enter and measure in one process invocation
    (`--enter … --states …`); a separate `--enter` then `--watch` wastes the
    level.
  * `--states` waits for a lemming above y 40 before scanning, because in
    the first second of a level nothing is falling and the scan finds no
    state object at all.
  * The game process died once between runs while sitting on the results
    screen with no input sent (pid 4156). Relaunch; nothing was lost.

## Rules

No value changed. `floatSpeed` 2, `floatAfter` 10, `splatDistance` 30 and
`fallSpeed` 4 all agree with the series. `src/sim/World.h` comments
re-labelled READ → READ AND MEASURED LIVE with the series and the addresses,
plus the `umbopen` delay recorded against `floatSpeed` as an unmodelled
difference. `test_world` untouched; suite 16 of 16.

## Left the game

Killed and relaunched at the end of the session, so it is sitting at a
fresh **main menu**: pid **4144**, `ui_state` **3**, frontend screen id
`0x004C416C` = **3**, nothing loaded, no level entered, shim live and the
game polling it at **144 Hz** with `enable = 0` (inert). Level table slot
[0][0] holds **MM_CLIMB_002**, its own record — checked, `level_table`
reports 102 entries with `MM_CLIMB_002` at group 0 slot 0, so every
`--enter` in this session put its swap back. The last level actually played
was `Compiled\MM_H_002.lvl`.
