# Phase 3, round 3 — spawn from the hatch words

2026-10-14. Question: does a released lemming's first position come from the
hatch int16 pair, and does `World::spawn` writing those words agree with the
first live sample on `MM_CLIMB_002`, `MM_FLOAT` and `MM_DIG_000`?

**Answer: yes, on all three, and the words are now parsed structurally out of
every one of the 102 retail levels.** `Rules` was not touched. No `test_world`
expectation moved; three checks were added.

Stock exe `C:\progs\Lemmings Revolution\Lemmings Revolution.exe`, SHA-256
`32675bdb49866e76502713251fdfd2b09078c2481c72ca22ff68ebf4b3737a2f`, base
`0x400000`. Re-run any disassembly below with
`python tools/harness/read_steppers.py --at <addr> --len <n>`.

---

## SUMMARY

| number | value |
| --- | --- |
| Retail levels with at least one hatch record | **102 of 102** |
| Hatch records found | **144** (1 per level ×69, 2 ×27, 3 ×3, 4 ×3) |
| Records where `y` word is **not** entrance world Y − 2 | **0 of 144** |
| Records where `x` word is **not** `boxMin.x + 4` | **0 of 144** |
| Max \|unrolled entrance column − `x` word\| | **0.0193** (the unroll's float error) |
| Model name on every record | `entrance` ×124, `entrance_acid` ×20, nothing else |
| Type word `+0xB0` | 0 ×117, 1 ×20, 2 ×7 |
| Live words vs file words, 3 levels | **identical** (32,42) (65,85) (84,79) |
| Live birth `+0x08` vs `x` word, 3 levels | **exact**: 32.0, 65.0, 84.0 |
| Live birth `+0x10` below `y` word | 0.0013 / 0.121 / 0.083 (integrator already running) |
| Live birth `+0x18` | 0.5 on all three |
| Our sim's landing row from the word vs the live landing | **equal on all three** (rows 63 / 14 / 20) |
| Side observation: Σ`count` word vs `LEVELMAP.DAT` totalLemmings | **equal on 102 of 102** |

Re-run commands:

```
python tools/harness/measure_spawn.py --hatch-words MM_CLIMB_002 MM_FLOAT MM_DIG_000
python tools/harness/measure_spawn.py --hatch-words            # all 102 + the summary
# live, one level per process (there is no safe route back to the map):
python tools/harness/goto_level.py MM_CLIMB_002                # focus() stubbed, see below
python tools/harness/measure_spawn.py --watch
# sim side:
make -C src ; WEBOLUTION_GAME_DIR=... build/cmake/dev-debug/test_world.exe
```

---

## Observation — the structural locator

The brief asked for the words to be found by the record's structure, not by a
remembered offset. The read order is the anchor.

### The stream primitives

Each is a thin wrapper on the stream read `0x0047DAC8` with a literal size:

| address | bytes read | what |
| --- | --- | --- |
| `0x00402540` | `push 1` | u8 |
| `0x00423360` | `push 2` | u16 |
| `0x00402560` | `push 4` | u32 |
| `0x00402580` | `push 8` | f64 |
| `0x00480BB0` | three × `0x00402580` into `[this]`, `[this+8]`, `[this+0x10]` | Vec3 |

### The hatch loader's read order

`0x00425E50` (hatch vtable slot 6) calls the base loader `0x0043130E` and then
reads its own fields. `0x0043130E` calls `0x00432283` (which calls the actor
base reader `0x00423291`, three u8, and — when the second of those is set —
the model constructor `0x00465924`) and then:

```
00431328  ecx = this+0x48 ; call 0x480bb0     ; Vec3 -> hatch+0x48
0043133a  ecx = this+0x60 ; call 0x480bb0     ; Vec3 -> hatch+0x60
00431348  ecx = this+0x78 ; ...               ; one f64
```

then, back in `0x00425E50`:

```
00425ef1  call 0x402560 ; mov [ecx+0xb0], eax ; u32  type
00425f02  call 0x423360 ; mov [edx+0x98], ax  ; u16  release count
00425f14  call 0x423360 ; mov [ecx+0x9a], ax  ; u16  spawn X
00425f26  call 0x423360 ; mov [edx+0x9c], ax  ; u16  spawn Y
00425f38  call 0x402560 ; mov [ecx+0xa0], eax ; u32
00425f49  call 0x402560 ; mov [edx+0xa4], eax ; u32
00425f5a  call 0x402560 ; mov [ecx+0xa8], eax ; u32
00425f6b  call 0x402540                       ; u8
```

So the bytes just before the triple are a run of **seven f64 then one u32**.
That run is what the locator looks for, and over the corpus it is always the
same shape — an axis-aligned 8 × 5 box in unrolled space with z 0 then z 1:

```
Vec3 A = (x0,     y0,     0)
Vec3 B = (x0 + 8, y0 + 5, 1)
f64    = 0.0
u32    = type, 0..2
u16 u16 u16 = count, x, y
```

Every component of the run is checked (integral, the two deltas exactly 8 and
5, z exactly 0 and 1, type ≤ 8, count 1..200, x < 256, y < 256). Nothing is
carried from one level to the next. Both implementations are the same test:

- `tools/harness/measure_spawn.py` → `hatch_words(data)`, flag `--hatch-words`
- `src/assets/LevelObjects.cpp` → `findHatchRecords`, field `LevelObjects::hatches`

### It recovers the validation anchors

The three ad-hoc offsets from `docs/scratch/phase2-spawn.md` come back exactly,
and the locator finds them without being told where to look:

| level | words@ | count, x, y |
| --- | --- | --- |
| MM_CLIMB_002 | `0x0A0C5A` | 10, 32, 42 |
| MM_FLOAT | `0x13C197` | 10, 65, 85 |
| MM_DIG_000 | `0x130AE3` | 20, 84, 79 |

### What the record looks like in the file

`MM_CLIMB_002`, the whole tail, from the model's second name record:

```
0x0a0c01  08 00 00 00 "entrance"      ; name record
          00 00 00 00 01              ; 5 bytes
0x0a0c12  08 00 00 00 "entrance"      ; name record
0x0a0c1e  28.0  44.0  0.0             ; Vec3 -> hatch+0x48
0x0a0c36  36.0  49.0  1.0             ; Vec3 -> hatch+0x60
0x0a0c4e  0.0                         ; f64  -> hatch+0x78
0x0a0c56  00 00 00 00                 ; u32  -> hatch+0xB0, type 0
0x0a0c5a  0a 00  20 00  2a 00         ; u16  -> +0x98 +0x9A +0x9C = 10, 32, 42
0x0a0c60  14 00 00 00  01 00 00 00  32 00 00 00   ; -> +0xA0/A4/A8 = 20, 1, 50
0x0a0c6c  00                          ; u8
```

The two name records are the model's, read inside `0x00465924`; they are
printed beside each hit so a match can be eyeballed, and they are **not** part
of the match. Across 144 records they are only ever `entrance` (124) or
`entrance_acid` (20).

---

## Observation — the corpus survey (deliverable 2)

`python tools/harness/measure_spawn.py --hatch-words`

- **102 of 102 levels have at least one hatch record.** 144 records in total:
  69 levels have one, 27 have two, 3 have three (`H06_MT_1`, `VH06_MT`,
  `VH12_KF1`), 3 have four (`E10_KF_1`, `H03_KF_1`, `MM_H_001`).
- **144 of 144 have `y` == entrance world Y − 2. There are no exceptions.**
- 144 of 144 have `x` == `boxMin.x + 4`, the centre column of the 8-wide box.
- Every record pairs 1:1 with a parsed `Entrance*.a3d` placement; the largest
  disagreement between the `x` word and the unrolled column is 0.0193, which
  is the `atan2` unroll's own error, not a different number.

Two notes on the pairing, because an earlier run of this survey reported one
false exception:

1. Pairing hatch records to entrance placements by **column alone** is wrong.
   `VH04_JC1` stacks two entrances on column 40 (world Y 79 and 25), so both
   records matched the same placement and the second looked like `79 − 23 = 56`.
   The script now pairs by nearest `(column, world Y)` — geometric, so it
   cannot assume the 2 it is testing for.
2. The fact that it holds 144 times is **still not a rule the code may use.**
   There is no subtract instruction anywhere: `0x00425F2E` stores the word
   straight from the stream, and the only other write of `+0x9A`/`+0x9C` is
   the hatch constructor `0x004258C0` zeroing them. The 2 is the difference
   between two independently stored fields. `findHatchRecords` reads the
   stored word; nothing reconstructs it.

### Side observation, not wired

Σ of each level's `count` words (`+0x98`) equals that level's
`LEVELMAP.DAT` `totalLemmings` on **102 of 102**. E.g. `MM_H_001` is
5+5+5+5 = 20, `VH21_JC` is 1+49 = 50. The summary line of `--hatch-words`
re-checks it. Round 3 is position only, so nothing in the sim uses it; it
belongs to whoever does per-hatch release scheduling.

---

## Observation — the live check (deliverable 4)

One level per process; the process was restarted between levels. Entry was
`goto_level`'s own path with `focus()` stubbed to a no-op, because
`goto_level.focus()` crashes the process through dgVoodoo on this machine
after a cold attach (`docs/scratch/phase3-round2.md`). The shim still polled
at 76–96 Hz. One of the four entry attempts failed to take the node click
(the known one-in-six) and was retried after a restart.

### MM_CLIMB_002, pid 34180, `Compiled\MM_CLIMB_002.lvl`

```
hatch 0x026a4010  doubles +08 32.0  +10 44.0  +18 0.0
  words +98 count 4  +9A 32  +9C 42  type +B0 0
birth  0x026d64c0 vt 0x004b10e8  +08 32.0  +10 41.99870300292969  +18 0.5
```

`+9A`/`+9C` are the file words (32, 42) exactly. `+0x08` is the `x` word
exactly. `+0x10` is **0.0013** below the `y` word — the tightest sample of the
three rounds, and it pins the pre-integration value at 42.0 rather than at
some stored fraction (phase 2 caught this birth 0.061 late). `+0x18` is 0.5.

### MM_FLOAT, pid 46016, `Compiled\MM_FLOAT.lvl`

```
hatch 0x0273d180  doubles +08 65.0  +10 87.0  +18 0.0
  words +98 count 6  +9A 65  +9C 85  type +B0 0
birth  0x02773c50 vt 0x004b10e8  +08 65.0  +10 84.87919616699219  +18 0.5
...
  t= 0.199s  +08 65.00521087646484  +10 81.0  +18 0.5199999809265137   <- landed
```

### MM_DIG_000, pid 36656, `Compiled\MM_DIG_000.lvl`

```
hatch 0x028763c0  doubles +08 84.0  +10 81.0  +18 0.0
  words +98 count 17  +9A 84  +9C 79  type +B0 0
birth  0x028a4c50 vt 0x004b10e8  +08 84.0  +10 78.91696166992188  +18 0.4999997913837433
...
  t= 0.208s  +08 83.93923950195312  +10 75.0  +18 0.45999976992607117  <- landed
```

**Both sides agree on all three levels.** The live hatch words are the words
the file-side locator read; the live lemming's `+0x08` is the `x` word to the
bit; `+0x10` is below the `y` word by the amount the 20 units/s integrator
covers in the few ms before the pointer is published, which is not one value
(0.0013, 0.121, 0.083) and is therefore not a stored offset.

---

## Observation — which cell the words are, and a one-cell correction

The round brief gave the flip as `floor((worldH - y) * pixelsPerUnit)` with
`pixelsPerUnit = 1`, i.e. `worldToMaskPixel`. That is **one cell off for an
integer y**, and the spawn words are always integers:

- `Terrain::fromCollisionGrid` maps file row `r` to terrain row `height-1-r`.
- `worldToMaskPixel` maps world Y to `floor(worldH - y)`.
- For non-integer y the two agree (`y = 32.190` → 63 = 95 − 32). For
  `y = 42` exactly they give 53 and 54.

The engine settles it. Its float→int helper is `0x0049C5AC`:

```
0049c5b3  d9 7d fe   fnstcw [ebp-2]
0049c5bb  80 cc 0c   or     ah, 0xc        ; rounding control 11b = chop
0049c5c2  d9 6d fc   fldcw  [ebp-4]
0049c5c5  df 7d f4   fistp  qword [ebp-0xc]
```

`or ah, 0xc` sets the x87 rounding field to truncate-toward-zero, so a world Y
becomes a file row by **truncation**: a `y` word of 42 *is* file row 42, and
the terrain row is `height - 1 - 42 = 53`.

The birth sample agrees: the live `+0x10` of a fresh lemming is the word
itself (0.0013 off 42 on MM_CLIMB_002), so the release starts at world Y 42,
not 41.

**The landing does not discriminate**, and the first version of this note
cited it as though it did. From either start row -- terrain 53 or 54 -- the
fall rests on the same floor, so the landing is equally consistent with
`height - 1 - y` and with `floor(96 - y)`. It corroborates that the *column*
and the floor beneath it are right; it says nothing about which of the two
start rows the engine used. The chop read and the birth sample are the
evidence. (Facilitator re-check, 2026-10-14: the grid side below was re-read
from the three `.LVL` files independently and matches.)

At each level's spawn column, the `y` word's own file row is empty and the
highest solid row below it is:

| level | col | `y` word | row solid? | highest solid below | rest world Y | live landing `+0x10` |
| --- | --- | --- | --- | --- | --- | --- |
| MM_CLIMB_002 | 32 | 42 | no | 31 | **32** | **32.0** |
| MM_FLOAT | 65 | 85 | no | 80 | **81** | **81.0** |
| MM_DIG_000 | 84 | 79 | no | 74 | **75** | **75.0** |

A spawn at `floor(96 - y)` would start one row lower on all three -- which
the birth sample rules out, not the landing. So
`hatchSpawnCell` uses `height - 1 - y`, with the citation in its header
comment. `worldToMaskPixel` was **not** changed: it is now only the fallback
path for levels without a hatch record (there are none in retail) and for the
lattice tooling, and changing it would move `LevelStats` inputs, which this
round must not touch. The one-cell difference between the two is recorded here
rather than smoothed over; whoever owns the entrance-translation path next
should decide whether `worldToMaskPixel`'s half-open interval is right for its
own callers.

Our sim, stepped from the words on the stored grid, comes to rest on exactly
the row each live birth landed on — rows 63, 14, 20 (`= 95 - 32/81/75`). That
is the differential result, printed by `test_world`:

```
spawn from the hatch words
  MM_CLIMB_002: words x 32 y 42 -> cell (32, 53), first lemming x 32
    lands at row 63 (live world Y 32 is row 63)
  MM_FLOAT: words x 65 y 85 -> cell (65, 10), first lemming x 65
    lands at row 14 (live world Y 81 is row 14)
  MM_DIG_000: words x 84 y 79 -> cell (84, 16), first lemming x 84
    lands at row 20 (live world Y 75 is row 20)
```

---

## What changed in the code

- `src/assets/LevelObjects.h/.cpp` — `HatchRecord`, `findHatchRecords`,
  `hatchSpawnCell`, and `LevelObjects::hatches` filled by `findLevelObjects`.
  The read order and the three measured triples are in the header comment.
- `src/sim/World.h` — `Entrance` gains `Source { EntranceModel, HatchWords }`
  so the code says which read filled a slot. No `Rules` field touched.
- `src/sim/World.cpp` — `World::spawn`'s comment now describes what it does
  rather than what it fails to do. The retail load supplies the words; the
  `Entrance*.a3d` translation remains the fallback for hand-built levels.
- `src/main.cpp` — `makeSurfaceLevel` prefers `objects->hatches` and keeps the
  translation only when there is no hatch record.
- `tools/harness/measure_spawn.py` — `--hatch-words`, `hatch_words()`,
  `spawn_cell()`, `level_names()`, `level_bytes()`, `level_totals()`.
- `src/__test__/test_world.cpp` — new `testRetailSpawn`. Three added checks
  per level; **no existing expectation moved.**

### Not concluded

- **Facing.** The original writes `+0x40 = 1` at `0x0042DFDD` and our
  `Facing::Right` is also 1, so the numbers match, but which column direction
  that 1 is was **not** isolated. The first post-landing step went toward
  increasing column on MM_CLIMB_002 and MM_FLOAT and toward *decreasing*
  column on MM_DIG_000 (`+0x08` 83.939 at landing), and a wall can turn a
  walker on its first step. Left as `Facing::Right`, labelled unverified.
- **The constructor's 0.5.** Still the Z of the original's position triple
  (`push 0x3FE00000` / `push 0` at `0x0042DE6F`). The sim has no Z and does
  not store it. It was 0.4999997913837433 on MM_DIG_000's birth, so it is
  carried through arithmetic, not a flag.
- **`y` == entranceY − 2 as a formula.** 144 records is not an instruction.
  Nothing in the code computes it.
- **The `+0xA0`/`+0xA4`/`+0xA8` dwords.** Parsed into nothing. They look like
  release-rate style triples (`(20,1,50)`, `(25,1,50)`, `(5,1,99)`,
  `(99,99,99)`) and are not this round's question.
- **The type word `+0xB0`** (0/1/2) indexes the release counter array at
  `0x004CBA78` (`0x0042DFF1`). Not wired.

---

## Harness and build notes

- `.\scripts\Invoke-Ucrt64.ps1` produced **no output and exit 0** for every
  command this session — `cmd /c` from the PowerShell tool returns nothing
  here. The build was run from the Bash tool with
  `PATH=/c/progs/msys2/ucrt64/bin:/c/progs/msys2/usr/bin:$PATH` instead, which
  worked. Harness scripts were run with the default Python, no UCRT64 on PATH,
  so `capstone` stayed available.
- **`test_collisiongrid` fails, and it failed before this round.** "collision
  grid -> Terrain: every cell agrees (10046 do not)". It is the uncommitted
  Y-flip in `Terrain::fromCollisionGrid` against a test that still compares
  cell-for-cell without the flip. Verified pre-existing by stashing only this
  round's files and re-running: the failure is identical. Not touched — the
  handoff says a `test_collisiongrid` failure is a parser bug, and this one is
  a stale expectation that belongs to whoever landed the flip.
  The other 15 tests pass; `test_world` is 91 checks, 0 failures.

## Left the game

pid **36656**, loaded path **`Compiled\MM_DIG_000.lvl`**. `ui_state` was **0**
during the measurement (released counter reached 20 of 20) and the level then
ended by itself while the write-up was being made, so the process was last
seen at `ui_state` **5**, the post-level results screen. **Nothing was
pressed to get there and nothing should be pressed now** — ESC from a
post-level screen can quit the process, and the next round should restart
rather than try to navigate out. The shim
was released after each entry; nothing is held enabled. Three earlier
processes (34180 MM_CLIMB_002, 46016 MM_FLOAT, and one that failed to enter)
were killed with `taskkill` as the deliberate way to change level — there is
no safe route from inside a level back to the map. Slot [0][0] of the level
record table was restored by `goto_level` after every entry (it printed
`slot [0][0] restored: True` each time), so the game's own table is unaltered.
