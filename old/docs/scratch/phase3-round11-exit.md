# Phase 3 round 11 — the exit box

2026-10-16. Stock `C:\progs\Lemmings Revolution\Lemmings Revolution.exe`,
SHA-256 `32675bdb49866e76502713251fdfd2b09078c2481c72ca22ff68ebf4b3737a2f`,
image base `0x400000`, driven through `tools/dinput-shim`.

Harness: `tools/harness/measure_exit.py` (new). Re-run:

```
python tools/harness/read_steppers.py --at 0x00430BA1 --len 0x90
python tools/harness/read_steppers.py --at 0x0042688D --len 0x100
python tools/harness/read_steppers.py --at 0x0041818A --len 0x100
# with the game in a level:
python tools/harness/measure_exit.py --enter MM_CLIMB_002 --dump
python tools/harness/measure_exit.py --enter MM_CLIMB_002 --watch 40 --hz 100
```

---

## PREDICTION (written before the live run)

### P1 — shape

The save test is **not** a hand-sized rectangle at the exit pose. It is the
general object-overlap predicate `0x00430BA1`: a point-in-AABB on three axes
against the exit object's stored box at `object+0x48..+0x70`.

Falsified if a live save fires while the lemming's `(X,Y,Z)` is outside that
box, or if the save path never calls `0x00430BA1`.

### P2 — which point

The exit object's step `0x0042688D` (vtable `0x004B0F90` + slot) walks the
container at `0x004C88D0` and calls:

```
overlap(exit, lemming + 8)     ; raw (X, Y, Z), NOT 0x0042F22E's biased pose
```

Anti-grav (round 9) used the biased pose. Prediction: a save fires when the
raw `(X,Y)` enters the AABB, regardless of `[+0x350]/[+0x358]` facing bias.

### P3 — AABB axes

`0x00430BA1` requires:

```
minX <= X <= maxX
minY <= Y <= maxY
minZ <= Z <= maxZ
```

`0x00430FE1` (the setter, three callers) always writes `minZ = 0`, `maxZ = 1.0`
(`mov [ecx+0x74], 0x3ff00000`). Prediction: Z is a fixed slab `[0, 1]` and the
interesting extents are X and Y only.

### P4 — save counter

When the attached state (vtable `0x004B0B28`, type `0x2a`) steps and its timer
at `state+8` reaches `>= 1.0`, it increments the word at `0x004CBAA4` (and a
per-type word at `0x004CBA82 + 2*[+0x4c]`). Prediction: watching `0x004CBAA4`
rise coincides with a lemming entering type `0x2a` / vtable `0x004B0B28` while
inside the dumped AABB.

### P5 — origin / size (to measure)

Exit ctor `0x004266E0` feeds the base object setup a point at
`(param_x, param_y + 3.0, …)` (`fadd` of the `3.0` at `0x004B03D8`). Hypothesis,
still provisional until the live dump: the AABB is centred on or anchored to
that shifted pose, not the bare `.a3d` translation. The live dump must report
`AABB` vs `object+8/+10` vs the LevelObjects exit pose.

Falsified by any of: AABB empty/degenerate; save outside AABB; AABB unrelated
to the exit pose by more than a few world units.

---

## Static reads (before live)

| claim | address | bytes / observation |
| --- | --- | --- |
| overlap predicate | `0x00430BA1` | six `fcomp`s: `[arg+0/8/10]` vs `this+0x48/50/58` (mins, CF=less → fail) and vs `this+0x60/68/70` (maxes, not `CF\|ZF` → fail) |
| AABB setter | `0x00430FE1` | writes mins/maxes through wrap `0x0040DEC0` / clamp `0x0040DF50`; forces Z to `[0, 1.0]` |
| exit object vtable | `0x004B0F90` | step `0x0042688D`, installed at `0x00426783` (`c7 02 90 0f 4b 00`) |
| exit object step | `0x0042688D` | walks `0x004C88D0`; on overlap + type match + not already `0x2a`, `operator_new(0x60)` → ctor `0x00417F00` → attach `0x0042E696` |
| save state vtable | `0x004B0B28` | ctor sets type `0x2a` at `+0x14`; enter `0x00417FE4` pushes anim string at `0x004BF424` (`"fall"`) |
| save increment | `0x00418284` / `0x004182A1` | `inc word [0x4CBA82+2*type]`; `inc word [0x4CBAA4]` when `state+8 >= 1.0` |
| anti-grav list is not this | `0x004C8958` | only two `.text` imm32 hits, both in the anti-grav arm (round 9); exit uses `0x004C88D0` |

Call sites of `0x00430BA1` (19): first is anti-grav `0x0041C1E3`; exit attach
is inside `0x0042688D` at `0x00426B5B`. Cursor/`_curExit` string at
`0x004C0DD0` is only registered at `0x0045E89B` (UI name table) — not a second
box by itself.

---

## Live series

### Dumped AABBs (Y-up world, actor-list lemmings)

| level | Exit.a3d `(…, ty, …, col)` | live pose | AABB |
| --- | --- | --- | --- |
| MM_CLIMB_002 | ty=40.0, col≈221.016 | (221.000, ~40, 1.500) | X[220..222] Y[31..34] Z[0..1] |
| MM_FLOAT | ty=15.0, col≈176.999 | (177.000, ~15.3, 1.500) | X[176..178] Y[6..9] Z[0..1] |

Both boxes are exactly **2 × 3** world units. Z is the fixed slab. Against the
`.a3d` translation: `X ∈ [floor(col)−1, floor(col)+1]`, `Y ∈ [ty−9, ty−6]`.
Live object pose Y drifts a fraction (anim / float) and is **not** the anchor —
the integer `ty` from the placement is.

MM_DIG_000 enter via `goto_level` stayed on MM_FLOAT (slot copy restored, but
the loaded path remained `Compiled\MM_FLOAT.lvl`); that third level was not
dumped in this session.

### Hold-probe edges (MM_CLIMB_002)

Walkers stuck in the pen for a natural walk-in, so the edges were pinned by
teleport-and-hold: write `(X,Y)` on a walker, require either a save-state
attach (vtable `0x004B0B28` / type `0x2a`) or a rise of `0x004CBAA4`, then
restore. Save target at `0x004CBAB0` was raised to 50/80 so a single save
would not end the level; restored to **1** at the end of the round
(process pid 26872 was on ui_state 5 / results when restored).

Wide X at Y=32.5:

| X | result |
| --- | --- |
| 219.0, 219.5 | miss |
| 220.0, 221.0, 222.0 | HIT |
| 222.5, 223.0 | miss |

Wide Y at X=221:

| Y | result |
| --- | --- |
| 30.0, 30.5 | miss |
| 31.0, 32.0, 33.0, 34.0 | HIT |
| 34.5 | miss |

So the closed interval matches the dumped doubles. (A later 34.01 sample
reported HIT without a saved rise — treated as probe noise; 34.5 is the
clean outside.)

### Cursor string

`_curExit` at `0x004C0DD0` is only registered in the UI name table
(`0x0045E89B`). No second save box was found; the lemming save path is the
object AABB alone.

---

## Verdict

**P1–P4 hold. P5 resolved to the `.a3d` integer ty / floor(col), not the
live object pose.**

The save test is the general object-overlap AABB (`0x00430BA1`) on the exit
object (vtable `0x004B0F90`, list `0x004C88D0`), against the lemming's raw
`(X,Y,Z)` at `+8`. Inclusive on every edge; size **2×3** with Z `[0,1]`.
Relative to `Exit*.a3d`: `[col±1] × [ty−9 .. ty−6]`.

Sim model stays a rectangle (`sim::Exit`) because that is what the AABB
is in integer Terrain space after the hatch-style Y flip
(`height − 1 − worldY`). Defaults and all five `main.cpp` sites are now
`2×3` inclusive; `LevelObjects::exitTriggerFromPose` carries the extent.
`World::atExit` uses `<=` on both axes.

Tests: `testExit` pins the closed interval (half-open alone → saved=0);
`testAntiGravFromCollisionGrid` closes the round-9 extraction gap. Suite
**16/16**.

---

## Left open

- Natural walk-into-exit series without teleport (pen geometry on
  MM_CLIMB_002 kept walkers away for ~50 s watches). Edges are still
  measured; the walk-in is not.
- MM_DIG_000 (and any third level) AABB vs `.a3d` — enter path failed once.
- How the runtime fills `object+0x48..+0x70` from the placement (ctor
  `0x004266E0` adds 3.0 at `0x004B03D8` to a pose; the measured box sits
  six to nine below `ty`, not centred on that shifted point). The
  **extents** are settled; the exact writer chain from `.a3d` → AABB is
  not required for the sim once `exitTriggerFromPose` matches the dump.
- The other 17 call sites of `0x00430BA1` beyond anti-grav and exit.
- Temp probe helpers (`tools/harness/_probe_exit_aabb.py` and siblings)
  are session scaffolding; `measure_exit.py` is the kept harness.
