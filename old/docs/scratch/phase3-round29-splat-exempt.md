# Phase 3 round 29 — splat exemption zones `0x200` / `0x4000`

2026-10-16. Stock `C:\progs\Lemmings Revolution\Lemmings Revolution.exe`,
SHA-256 `32675bdb49866e76502713251fdfd2b09078c2481c72ca22ff68ebf4b3737a2f`,
image base `0x400000`, driven through `tools/dinput-shim`.

Harness: `tools/harness/measure_splat_exempt.py`. Re-run:

```
python tools/harness/measure_splat_exempt.py --audit
python tools/harness/read_steppers.py --at 0x00416615 --len 0x60
python tools/harness/read_steppers.py --at 0x00428D4F --len 0x60
# restart process, then (type-1 + 0x200, force splat arm via raise-acc):
python tools/harness/measure_splat_exempt.py --enter MM_W_012 --natural \
       --bit 0x200 --clamp-acc 31 --raise-acc --watch 25 --hz 200
# type-2 + paint 0x4000: see tools/harness/_tmp_r29_type2_paint.py pattern
# (force raise-acc on H23_mg_1 type-2 hatch floor after OR-ing 0x4000)
```

Live spellings: `MM_W_012`, `H23_mg_1` (group 2 slot 22).

---

## SUMMARY

| Thing | Status | Value |
| --- | --- | --- |
| **who stamps** | **MEASURED offline + READ** | **Fluid** (`FUN_00428a47`, string `0x004BF7C0`). Object `+0x128` 0/1/2 → bits `0x200` / `0x2000` / `0x4000` at `0x00428D63` / `0x00428D84` / `0x00428DA5` (plus sibling sites in the same Fluid path). Glow `^water_glow` / `^lava_glow` / `^acid_glow`. |
| **census** | **MEASURED offline, 102/102** | Every level with zone bits embeds `Fluid` (63/63 of `0x200`, 11/11 of `0x2000`, 9/9 of `0x4000`). Zero zone levels without Fluid. |
| **`[+0x4c]`** | **MEASURED** | Hatch `+0xB0` → spawn `mov [edx+0x4c], eax` at **`0x0042E05F`**. Sheet table **`0x004BFFC8`**: 0 `Lemmings`, 1 `LemmingsWater`, 2 `LemmingsAcid`. Hatch model `entrance_acid` → type **1** (20); model `entrance` with type word **2** → LemmingsAcid (7). |
| **landing arms** | **READ + MEASURED live** | `FUN_00416433`: after floater, survive when `(cell&0x200 && [+0x4c]==1)` or `(cell&0x4000 && [+0x4c]==2)`. **`0x2000` has no splat arm.** |
| **live type-1** | **MEASURED** | MM_W_012: raise `[state+0x18]` to 31 on fall (`+0x340` vtable `0x004B0A20`), natural type 1, land on live `0x200` → **fall→walk** (7/7 on paint). |
| **live control** | **MEASURED** | Same raise; type-1 onto solid **without** `0x200` (col 182 row 68, word `0x1`) → **splat**. Type-2 onto `0x3` at (106,67) without `0x4000` → **splat**. |
| **live type-2** | **MEASURED** | H23_mg_1: paint `0x4000` onto type-2 hatch floor, raise-acc 31 → **4** fall→walk on `0x4000`, **0** splats on the painted zone. |
| **not drown** | settled round 14 | `0x40` / Entrance_water ∩ `0x200` correlation is a different mechanic. |
| **sim** | landed | `Lemming::sheetType`, `Terrain::zoneWater` / `zoneAcid` / `splatExempt`, faller land gate; hatch type → entrance → spawn. |

---

## Static reads

| claim | address | bytes / observation |
| --- | --- | --- |
| floater exempt | `0x00416615` | `mov cl,[eax+0x49]` / `jne` survive |
| water zone arm | `0x00416623` | `and edx, 0x200` ; `cmp [eax+0x4c], 1` ; `je` survive |
| acid zone arm | `0x00416639` | `and ecx, 0x4000` ; `cmp [edx+0x4c], 2` ; `je` survive |
| splat alloc | `0x0041664C` | death state ctor `0x0040B3A0` |
| decompiler | `FUN_00416433` | splat iff fallen>30 && !floater && !(0x200&&type1) && !(0x4000&&type2) |
| hatch → sheet | `0x0042E05F` | `mov [edx+0x4c], eax` (param from hatch type) |
| sheet names | `0x004BFFC8` | Lemmings / LemmingsWater / LemmingsAcid / EvilRat |
| Fluid stamp type0 | `0x00428D4F` | `cmp [ecx+0x128],0` / `push 0x200` / `call 0x4311d0` |
| Fluid stamp type1 | `0x00428D70` | `cmp …,1` / `push 0x2000` |
| Fluid stamp type2 | `0x00428D91` | `cmp …,2` / `push 0x4000` |
| `"Fluid"` | `0x004BF7C0` | xref `0x00428E09` in `FUN_00428a47` |

---

## Offline audit

```
levels 102
hatch type counts: {0: 117, 1: 20, 2: 7}
hatch model -> types:
  'entrance': {0: 117, 2: 7}
  'entrance_acid': {1: 20}
Fluid: 77
zone 0x200: 63  Fluid&z0: 63  z0-Fluid: 0
zone 0x2000: 11 Fluid&z1: 11 z1-Fluid: 0
zone 0x4000: 9 Fluid&z2: 9 z2-Fluid: 0
```

Naming trap (observation): hatch **model** string `entrance_acid` sets type **1** = **LemmingsWater** sheet. Type **2** = LemmingsAcid comes from model `entrance` with type word 2 on levels that also place `Entrance_acid.a3d`. Round 9/15's shorthand "entrance_acid → LemmingsAcid" confused the a3d name with the sheet table.

---

## Live series

### MM_W_012 — type 1 + `0x200` + raise-acc (positive)

Loaded `Compiled\MM_W_012.lvl`. Natural live `0x200`: 528 cells. Hatch type 1.
Raised fall `[state+0x18]` to 31.0 via lemming `+0x340` (vtable `0x004B0A20`)
every sample while airborne (43 writes).

| event | n |
| --- | --- |
| LAND-WALK on `0x200` with `+0x4c=1` | **7** |
| deaths on zone paint | **0** |

### Control — type 1 without zone

After walking off the water, same raise on a fall onto `(182,68)` word
`0x00000001` (no `0x200`): **splat**. Type-1 alone does not exempt.

### H23_mg_1 — type 2 without / with `0x4000`

- Natural type-2 hatch floor `(106,67)` word `0x3`, raise-acc → **splat**.
- Paint `\| 0x4000` onto cols 104–109 rows 65–69; raise-acc → **4** LAND-WALK
  on `0x4000` with `+0x4c=2`, **0** splats on the painted zone. Later walk-offs
  onto clear `0x3` still splat (restored 30 cells).

---

## Sim

- `Lemming::sheetType` / `Entrance::sheetType` from `HatchRecord.type`.
- `Terrain::zoneWater` (`0x200`), `zoneAcid` (`0x4000`), `splatExempt`.
- `World::stepFaller` skips splat when `splatExempt(foot, sheetType)`.
- Test: `testSplatZoneExempt` (mutate-alone: remove the guard).

Suite **16/16** after land.

---

## Left open

- Whether lava `0x2000` has any non-splat gameplay use (not this round).
- Runtime Fluid AABB footprint vs authored grid (live `0x200` count can be
  lower than the file; stamp sites are settled).

## Process

Game left at a fresh main menu after the session (relaunch + no level held).
Every paint path restored cells before exit when the process stayed alive.
