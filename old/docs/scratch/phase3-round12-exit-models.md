# Phase 3 round 12 — exit models (water / any / acid)

2026-10-16. Stock `C:\progs\Lemmings Revolution\Lemmings Revolution.exe`,
SHA-256 `32675bdb49866e76502713251fdfd2b09078c2481c72ca22ff68ebf4b3737a2f`,
image base `0x400000`, driven through `tools/dinput-shim`.

Round 11 measured the save AABB on **`Exit.a3d` only** (MM_CLIMB_002,
MM_FLOAT). This round asks whether `Exit_water.a3d`, `Exit_any.a3d`, and
`Exit_acid.a3d` share that box.

Re-run:

```
python tools/harness/measure_exit.py --enter e14_kf_1 --dump
python tools/harness/measure_exit.py --enter E16_KF_1 --dump
python tools/harness/measure_exit.py --enter E23_JC_1 --dump
python tools/harness/measure_exit.py --enter MM_W_012 --dump
# always read loaded path before trusting a dump (VERIFY line)
```

Facilitator owns `PLAN.md`, `docs/PHASE3_HANDOFF.md`, and
`.claude/rules/*` — this note is the agent deliverable.

---

## Census (re-run against `build/lvl_dump/levels/COMPILED`)

102 unique levels (204 files if both `.LVL` and `.lvl` are counted; stems
deduped):

| model | levels | alone (no `Exit.a3d`) |
| --- | ---: | --- |
| `Exit.a3d` | 84 | — |
| `Exit_water.a3d` | 14 | 8 (`E14_KF_1`, `H10_MT_1`, `H16_MT_1`, `MM_ALL_8_003`, `MM_BASH_MINE_WLR1`, `MM_E4_4_SKILLS`, `MM_W_012`, `VH22_JC`) |
| `Exit_any.a3d` | 9 | 9 (all of them) |
| `Exit_acid.a3d` | 1 | 1 (`E23_JC_1`) |

Six levels carry both `Exit.a3d` and `Exit_water.a3d`
(`E19_KF_2`, `M06_KF_1`, `M08_KF_1`, `MM_E1_5_SKILLS`, `SHOW_KF3`,
`VH11_MT`). Dump targets prefer **alone** levels.

Offline `.a3d` translations (Object 09, pad0) used for predictions:

| level | model | ty | ang×256 | floor | round |
| --- | --- | ---: | ---: | ---: | ---: |
| e14_kf_1 | Exit_water.a3d | 25 | 10.989 | 10 | 11 |
| MM_W_012 | Exit_water.a3d | 24 | 133.008 | 133 | 133 |
| E16_KF_1 | Exit_any.a3d | 21 | 210.995 | 210 | 211 |
| E23_JC_1 | Exit_acid.a3d | 79 | 83.004 | 83 | 83 |

Static note: exit ctor `0x004266E0` installs vtable `0x004B0F90`
(`c7 02 90 0f 4b 00` at `0x00426783`). Same imm32 at dtor path
`0x0042687B` and stub `0x004489E2`. No second exit class by vtable imm32
search. Overlap predicate remains `0x00430BA1` (round 11).

Corpus offline: **46** exit placements where `floor(ang×256) ≠ round(ang×256)`
(many of them `Exit.a3d`). Round 11's two dumps both had frac &lt; 0.5, so
floor happened to agree with the live centre.

---

## PREDICTION (written before the live runs)

### P1 — same factory

Every retail `Exit*.a3d` placement becomes one object with vtable
`0x004B0F90`. Falsified if a water/any/acid-only level has zero such
objects while still playable.

### P2 — same box (null hypothesis to break)

Live AABB vs the alone-level `.a3d` pose matches round 11's
`[col±1] × [ty−9 .. ty−6]`, size **2×3**, Z `[0,1]`, for water, any,
and acid.

Falsified if any dumped box differs in size or in those offsets by a
full cell (or more).

### P3 — still a save exit

Entering the dumped AABB attaches save state vtable `0x004B0B28` /
type `0x2a` (and can raise `0x004CBAA4`). Falsified if water/acid
objects exist but never save.

### P4 — loaded path

After `--enter NAME`, `loaded_level()` at `0x004CBBF8` must contain
`NAME`.

---

## Live dumps

Each run restarted the game process (`dump_grids.restart_game`) because
`--enter` while already playing leaves the previous loaded path
(round 11's MM_DIG_000 failure). `measure_exit.py` now refuses to dump
when the path does not contain the requested name.

| # | level | model | loaded path | exit objs | live AABB | vs P2 |
| --- | --- | --- | --- | ---: | --- | --- |
| 1 | e14_kf_1 | Exit_water.a3d | `Compiled\e14_kf_1.lvl` | 1 | X[10..12] Y[16..19] Z[0..1] size 2×3 | Y = ty−9..ty−6; X centres on **round(col)=11**, not floor 10 |
| 2 | E16_KF_1 | Exit_any.a3d | `Compiled\E16_KF_1.lvl` | 1 | X[210..212] Y[12..15] Z[0..1] size 2×3 | Y match; X centres on **round(col)=211**, not floor 210 |
| 3 | E23_JC_1 | Exit_acid.a3d | `Compiled\E23_JC_1.lvl` | 1 | X[82..84] Y[70..73] Z[0..1] size 2×3 | exact match (floor=round=83) |
| 4 | MM_W_012 | Exit_water.a3d | `Compiled\MM_W_012.lvl` | 1 | X[132..134] Y[15..18] Z[0..1] size 2×3 | exact match (floor=round=133) |

All four: one `0x004B0F90` object, inclusive 2×3, Z slab `[0,1]`, Y
offsets identical to round 11. **P1 holds. P2 holds for size and Y;
X centre is `round(ang×width)`, not `floor`.** That is a correction to
the round-11 column formula, not a per-model extent — confirmed on two
models where floor≠round (water, any) and two where they agree (water,
acid).

Live pose Y still drifts (anim); integer `ty` from the placement remains
the Y anchor.

### Hold-probes (P3)

| level | model | result | restores |
| --- | --- | --- | --- |
| MM_W_012 | Exit_water.a3d | **HIT** save-state `0x004B0B28` type `0x2a` (42) at AABB centre; lemming Z≈0.78 | lemming XY restored; save target `0x004CBAB0` 80→**10** |
| E23_JC_1 | Exit_acid.a3d | AABB dumped; save attach **not** completed this session (later re-enter failed on menu highlight≠0; an earlier in-level attempt saw 27 actors but the VT filter matched none before restore) | first attempt restored target to **48**; no lemming write landed |

Water is a save exit. Acid shares the exit object class and the same
AABB; save attach for acid is **left open** (not a clean negative — the
probe did not finish).

---

## Suite / sim

### Code

`exitTriggerFromPose` now uses `std::lround(ang * maskWidth)` then wraps,
instead of `floor`. Extents stay `width=2`, `height=3`,
`x = col - 1`, Y from `ty−9 .. ty−6`. No per-model table — dumps say one
box for all four models. Comments in `LevelObjects.h` updated.

### Task 2 — pin test

`testExitTriggerFromPose` in `src/__test__/test_world.cpp`, binary
**`test_world`**:

- MM_CLIMB_002 pose → Terrain box `(220, 61, 2×3)`
- e14_kf_1 water pose → `(10, 76, 2×3)` (fails if column uses floor)
- centre of that box saves on the next tick via `World::place` + `tick`
- one cell outside each of the four edges does **not** save

### Mutate-alone

Temporarily set `e.x = col - 6`, `e.width = 12` only (Y untouched).
`test_world.exe`:

```
FAIL  MM_CLIMB_002 box (got 215,61,12x3)
FAIL  e14_kf_1 water box (got 5,76,12x3)
273 checks, 2 failures
```

Restored measured `col - 1` / `width = 2`. Full suite after restore:
**16/16**.

---

## Verdict

**Same 2×3 box for every retail exit model** (`Exit.a3d`,
`Exit_water.a3d`, `Exit_any.a3d`, `Exit_acid.a3d`). Not per-model. Not
"water/acid are not exits" for the cases probed (water saves; acid is
the same object class with the same AABB).

Column centre is **`round(unroll×width)`**, not `floor`. Round 11's
`Exit.a3d` dumps remain valid; their fractional parts were &lt; 0.5.

---

## Left open

- Natural walk-into-exit with `0x004CBAA4` rising (task 3; not done —
  budget spent on dumps + pin test).
- Acid save-state hold-probe (same class/AABB measured; attach not
  confirmed live this session).
- Exact writer chain from `.a3d` → `object+0x48..+0x70` (still open from
  round 11; extents match dumps without it).
- Whether exit `team` at `+0x98` gates which lemming types save (exit
  step reads team-indexed words at `0x004CBA78` / `0x004CBA8C` before
  the overlap walk) — not required for the extent result.
- The other 17 call sites of `0x00430BA1`.

---

## Process / restores

Final state: **no Lemmings Revolution process** (stopped after the last
probe). Live writes this round:

| write | restored to |
| --- | --- |
| `0x004CBAB0` save target (raised to 50/80 while probing) | **10** (MM_W_012), **48** (E23 first attempt) |
| lemming +0x08/+0x10 teleports | original XY written back when a teleport landed |
| level-table slot [0][0] | always restored by `enter` / `select_level` |
| collision grid | never written |

No grid-cell paints this round.
