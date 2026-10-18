# Phase 3 round 14 — water as a hazard (drown)

2026-10-16. Stock `C:\progs\Lemmings Revolution\Lemmings Revolution.exe`,
SHA-256 `32675bdb49866e76502713251fdfd2b09078c2481c72ca22ff68ebf4b3737a2f`,
image base `0x400000`, driven through `tools/dinput-shim`.

Harness: `tools/harness/measure_water.py` (new). Re-run:

```
python tools/harness/measure_water.py --audit
python tools/harness/read_steppers.py --at 0x0041527B --len 0x2C0
python tools/harness/read_steppers.py --at 0x00408700 --len 0x80
python tools/harness/read_steppers.py --states
# restart the process, then
python tools/harness/measure_water.py --enter MM_CLIMB_002 --paint \
        --cols 32,33 --row 31 --watch 14 --hz 80
python tools/harness/measure_water.py --enter MM_W_012 --watch-only 50 --hz 40
python tools/harness/measure_water.py --enter H18_mg_1 --watch-only 50 --hz 40
python tools/harness/measure_water.py --enter M19_MT_1 --watch-only 40 --hz 50
```

`Exit_water.a3d` is a save exit (round 12) — out of scope.

---

## SUMMARY

| Thing | Status | Value |
| --- | --- | --- |
| **detection** | **MEASURED live + READ** | ground cell bit **`0x40`** → drown vtable **`0x004B0570`** (type `0x22`) |
| **probe cell** | **READ** | `(trunc(X), trunc(Y) - [+0x44])` at `0x004152F0` / test `0x00415386` |
| **who tests** | **READ** | spawn side-effect type `0x2f` (vtable `0x004B09C0`) at `0x0042E02C`, parked on `[+0x374]` |
| **what drown does** | **MEASURED + READ** | anim `"drown"`; accumulate dt at `state+8`; **> 2.0 s** → leave actor list |
| **authored 0x40** | **MEASURED offline** | **3 / 102** levels (`H18_MG_1`, `M12_MT_1`, `M19_MT_1`); **0** of 20 `Entrance_water` |
| **Entrance_water** | **MEASURED offline** | correlates with zone **`0x200`** (18/20), not `0x40` |
| **type 0x2e sibling** | **READ; not this round** | when `0x004C89D8` / `0x420a52` matches — leave open |
| **splat arms 0x200/0x4000** | **not this mechanic** | still open (round 3b / 9) |
| **sim** | landed | `Terrain::waterHazard`; `State::Drown`; `Rules::drownMs=2000` |

---

## PREDICTIONS (written before live runs)

### P1 — detection is cell bit `0x40` → drown `0x004B0570`

**Holds.** MM_CLIMB_002 paint `0x1 → 0x41` on (32,31)/(33,31): **5** fall→drown attaches in 14 s; every hatch faller; `under=(32,31) on_paint=True`; `stype=0x22`. Restored both cells to `0x1`.

### P2 — drown lasts ~2.0 s then removes the lemming

**Holds.** Five removals with last sampled `acc` ≈ 2.000; wall-clock attach→leave ≈ 2.00 s each.

### P3 — authored `.LVL` `0x40` is not the water census

**Holds.** `--audit`: Entrance_water ∩ 0x40 = **0**. Live MM_W_012: **0** live `0x40` cells, 528 live `0x200`, **0** drown in 50 s (loaded path `Compiled\MM_W_012.lvl`). Lemmings had `[+0x4c]=1`.

### P4 — H18_mg_1 (authored `0x40`) may drown without Fluid contact

**Inconclusive for natural contact.** Live 424 `0x40` cells present; walkers stayed on floors that never hit those pads in 50 s (**0** drown). Bit identity already settled by P1 paint.

---

## Static reads

| claim | address | bytes / observation |
| --- | --- | --- |
| drown vtable | `0x004B0570` | slots `0x4087d0` / `0x408766` / `0x40874c` / … |
| drown ctor | `0x00408700` | type `0x22` at `+0x14` |
| enter `"drown"` | `0x0040874c` | `FUN_0042fd25(..., 0x004BF130)` |
| drown step >2.0 | `0x00408766` | accumulate `state+8`; `> 2.0` → return 2 |
| fluid probe ctor | `0x004151F0` | vtable `0x004B09C0`, type `0x2f` |
| spawn attaches 0x2f | `0x0042E02C` | then fall `0x00415FA0` at `0x0042E09D` |
| probe cell 0x40 | `0x00415386` | `83 e0 40` |
| ground row | `0x004152F0` | `sub eax, [ecx+0x44]` |
| drown attach site | `0x004154D8` | `call 0x408700` (only xref) |
| type 0x2e sibling | `0x0041545B` | when `0x004C89D8` / `0x420a52` matches |
| `"Fluid"` | `0x004BF7C0` | xref `0x00428E09` in `0x00428A47` |
| `"^water_glow"` | `0x004BF7C8` | xref `0x0042A8B2` |
| `"LemmingsWater"` | `0x004C0020` | near `"LemmingsAcid"` |

Landing-path splat arms `0x200`/`[+0x4c]==1` and `0x4000`/`[+0x4c]==2` at
`0x0041662B` / `0x00416641` are **not** this mechanic.

---

## Offline audit

```
levels 102
Entrance_water.a3d: 20
Entrance_acid.a3d: 6
any cell & 0x40: 3 -> ['H18_MG_1', 'M12_MT_1', 'M19_MT_1']
Entrance_water & 0x40: 0
Entrance_water & 0x200: 18 / 20
Entrance_water & 0x2000: 2 / 20
Entrance_water & 0x4000: 4 / 20
```

H18 / M12 / M19 also embed `Fluid` / `water` strings in the `.LVL` blob.
MM_W_012 has `Entrance_water` + `Fluid` + 3600 authored `0x200` cells and
**zero** authored `0x40`.

---

## Live series

### MM_CLIMB_002 — paint `0x40` (P1/P2)

Loaded path `Compiled\MM_CLIMB_002.lvl`. Painted (32,31)/(33,31)
`0x00000001 → 0x00000041`. First cycle:

```
t 0.218  fall->drown  x=32.000 y=32.920 under=(32,31) on_paint=True stype=0x22
t 2.224  LEFT actor list after drown (last acc=2.000)
```

Five drown attaches, zero type2e, five removals. Both cells restored to `0x1`.

### MM_W_012 — watch-only

Loaded path `Compiled\MM_W_012.lvl`. live `0x40`: **0**. live `0x200`: 528.
`[+0x4c]=1` on released lemmings. **0** drown in 50 s (splats from long drops).

### H18_mg_1 — watch-only

Loaded path `Compiled\H18_mg_1.lvl`. live `0x40`: **424**. Pads at cols
169..176 × rows 42..94. Hatch walkers on other floors; **0** drown in 50 s.

### M19_MT_1 — watch-only

Loaded path `Compiled\M19_MT_1.lvl`. live `0x40`: **16** at (106..107, 34..41).
**0** drown in 40 s (pads not walked).

---

## Verdict

Water-as-hazard is cell bit **`0x40`** on the ground under the feet. Every
lemming carries a type-`0x2f` side-effect that tests it and attaches **drown**
(`0x004B0570`), which plays `"drown"` for **~2.0 s** then removes the lemming.
It is **not** `Exit_water` (save exit). It is **not** zone `0x200` /
`Entrance_water` alone — those can exist with zero live `0x40` and zero drown
(MM_W_012). Authored `0x40` appears in three retail grids; paint alone is
enough to drown without a Fluid object match (type `0x2e` path unused).

Sim: `Terrain::waterHazard` from `kCellFlag0x40`, `State::Drown`,
`Rules::drownMs = 2000`. Tests in `test_world` / `test_terrain`. Mutate-alone:
`drownMs 2000→200` fails duration; removing both attach sites fails enter.
Suite **16/16**.

Binary under test: `build/cmake/dev-debug/test_world.exe` (and `test_terrain.exe`
for the grid extract).

---

## Left open

- What paints `0x40` at runtime on levels that lack it in the file (setter
  `0x0042081B` still has no found E8 caller; Fluid / LemmingsWater are leads).
- Type `0x2e` sibling when `0x004C89D8` / `0x420a52` matches — may be acid
  or a particle path; not measured this round.
- Zone `0x200` / `0x4000` splat exemptions with `[+0x4c]` (separate gadget).
- Natural drown on H18/M19 pads (pathing, not bit identity).
- Interaction if a water-hazard cell and an Exit_water AABB overlap.
