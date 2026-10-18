# Phase 3 round 15 — acid as a hazard

2026-10-16. Stock `C:\progs\Lemmings Revolution\Lemmings Revolution.exe`,
SHA-256 `32675bdb49866e76502713251fdfd2b09078c2481c72ca22ff68ebf4b3737a2f`,
image base `0x400000`, driven through `tools/dinput-shim`.

Harness: `tools/harness/measure_acid.py` (new). Re-run:

```
python tools/harness/measure_acid.py --audit
python tools/harness/read_steppers.py --at 0x0041527B --len 0x2C0
python tools/harness/read_steppers.py --at 0x00406C20 --len 0x40
python tools/harness/read_steppers.py --at 0x004071F6 --len 0x70
# restart the process, then
python tools/harness/measure_acid.py --enter H02_JC_1 --watch-only 45 --hz 40
python tools/harness/measure_acid.py --enter H02_JC_1 --paint --bit 0x40 \
        --cols 194,195 --row 67 --watch 16 --hz 80
python tools/harness/measure_acid.py --enter M12_MT_1 --watch-only 40 --hz 40
# on M12 with live 0x40 pads: force a type-0 lemming onto (172,78)
python tools/harness/measure_acid.py --enter MM_CLIMB_002 --paint --bit 0x100 \
        --cols 32,33 --row 31 --watch 12 --hz 80
python tools/harness/measure_acid.py --enter H23_mg_1 --watch-only 18 --hz 40
python tools/harness/measure_acid.py --paint --bit 0x40 \
        --cols 139,140 --row 22 --watch 10 --hz 80
```

`Exit_acid.a3d` is a save exit (round 12) — out of scope.
Water drown `0x40` / `0x004B0570` is settled (round 14).

---

## SUMMARY

| Thing | Status | Value |
| --- | --- | --- |
| **acid hazard** | **MEASURED = water drown** | cell bit **`0x40`** → drown vtable **`0x004B0570`** (type `0x22`), ~2.0 s |
| **type `0x2e` sibling** | **MEASURED unused** | Fluid list `0x004C89D8` empty live; 0 type2e on acid/lava/paint |
| **`0x100` branch** | **MEASURED not death** | paint → `lem[+0x5f]=1`; 0 drown / type2e |
| **`LemmingsAcid` / Entrance_acid** | **MEASURED not hazard** | hatch → `[+0x4c]=1` (sheet); still drowns on `0x40` |
| **`^acid_glow`** | **READ visual** | Fluid subtype 2 at `0x0042A8EA` |
| **sim** | documented | same `Terrain::waterHazard` / `State::Drown` / `drownMs=2000` |

---

## PREDICTIONS (written before live runs)

### P1 — type `0x2e` is the Fluid-match sibling; list may be empty

**Holds.** Live Fluid-list words at `0x004C89D8` were
`[0,0,1,0, 0,0,1,0]` (empty) on H02, M12, H23, MM_CLIMB. Absolute refs to
the list are only the two probe reads. Paint `0x40` on H02 (^acid Fluid):
**50 drown, 0 type2e**. Force type-0 onto M12 authored `0x40`: **1 drown,
0 type2e**.

### P2 — acid Fluid is not a separate cell bit

**Holds.** H02 / H23 (^acid* / Entrance_acid): **0** live `0x40`, **0** live
`0x100`, **0** drown in watch-only. Authored `0x100` levels are not acid
levels.

### P3 — `0x100` is not a lemming death state

**Holds.** MM_CLIMB_002 paint `0x1 → 0x101` on (32,31)/(33,31): faller set
`flag5f 0→1` at under=(32,31); **0 drown, 0 type2e**. Restored to `0x1`.

### P4 — Entrance_acid / LemmingsAcid are not the hazard

**Holds.** H23_mg_1: hatch at x=139 releases `[+0x4c]=1`; watch-only 0 drown.
Paint `0x40` on (139,22)/(140,22): type-1 walkers → drown `stype=0x22`,
**9 drown, 0 type2e**. Restored cells to `0x200`.

### P5 — Fluid glow subtype is visual only for attach

**Holds (static).** `0x0042A892` / `CE` / `EA` push water/lava/acid glow;
attach site at `0x00415419` does not branch on subtype.

---

## Static reads

| claim | address | bytes / observation |
| --- | --- | --- |
| cell `0x40` test | `0x00415386` | `83 e0 40` |
| Fluid list loop | `0x004153D5` | `mov ecx, 0x4c89d8` (only absolute refs) |
| match → type `0x2e` | `0x00415425` / `0x0041545B` | `push 0x2e`; `call 0x406c20` |
| else drown | `0x00415496` / `0x004154D8` | `push 0x22`; `call 0x408700` |
| type `0x2e` vtable | `0x004B04D0` | step `0x004071F6`, enter `0x00406C6C` (no anim site) |
| cell `0x100` arm | `0x00415552` | `and edx, 0x100` → ctor `0x00415710`; `mov [edx+0x5f],1` at `0x00415627` |
| glow subtype | `0x0042A8B2` / `CE` / `EA` | water / lava / acid glow |
| `LemmingsAcid` sheet | `0x004C0030` | table `0x004BFFC8`, index use `0x0044384C` |

---

## Offline audit

```
levels 102
Entrance_acid.a3d: 6 -> E23_JC_1 H17_MG_1 H23_MG_1 H24_KF_1 M20_MT_1 VH12_KF1
^acid*: 9 -> H02_JC_1 H23_MG_1 H24_KF_1 M08_MG_1 M20_MT_1 MM_ALL_8_001
             MM_E1_5_SKILLS MM_TELE_003 VH12_KF1
any cell & 0x40: 3 -> H18_MG_1 M12_MT_1 M19_MT_1
any cell & 0x100: 3 -> H08_MT_1 H22_MT_1 VH06_MT
Entrance_acid & 0x40: []
^acid* & 0x40: []
^acid* & 0x100: []
```

---

## Live series

### H02_JC_1 — watch-only then paint `0x40`

Loaded `Compiled\H02_JC_1.lvl`. live `0x40`/`0x100`: **0**. Fluid list empty.
Watch 45 s: 0 drown. Paint (194,67)/(195,67) `0x3 → 0x43`: **50 drown,
0 type2e**, 50 removals at acc≈2.0. Restored to `0x3`.

### M12_MT_1 — authored `0x40` + lava

Loaded `Compiled\M12_MT_1.lvl`. live `0x40`: 10 cells at cols 172–173 rows
78–82. Watch-only: walkers elsewhere, 0 drown. Force type-0 to (172.5, 79)
under=(172,78): **walk→drown** within 12 ms, leave at acc=2.001; **0 type2e**.
XY restored; pad still live.

### MM_CLIMB_002 — paint `0x100`

Loaded `Compiled\MM_CLIMB_002.lvl`. Paint (32,31)/(33,31) `0x1 → 0x101`:
`FLAG5f 0→1` on fall through under=(32,31); land walk with 5f=1; **0 drown,
0 type2e**. Restored to `0x1`.

### H23_mg_1 — Entrance_acid + paint

Loaded `Compiled\H23_mg_1.lvl`. Hatch x=139 → `[+0x4c]=1`; other hatch
`[+0x4c]=2`. live `0x40`: 0. Watch-only: 0 drown. Paint (139,22)/(140,22)
`0x200 → 0x240`: type-1 → drown, **9 drown, 0 type2e**. Restored to `0x200`.

---

## Verdict

**Acid as a hazard is the same drown path as water:** ground cell bit
**`0x40`** → attach drown (`0x004B0570` / type `0x22`) → `"drown"` ~**2.0 s**
→ leave actor list. It is **not** a separate cell bit, **not** type `0x2e`
(Fluid-list match arm is dead in retail), **not** `0x100` (sets `[+0x5f]`
only), **not** `Entrance_acid` / `LemmingsAcid` (hatch type → `[+0x4c]`
sheet), and **not** `^acid_glow` (Fluid subtype 2 visual).

Clean negative for a distinct acid mechanic; positive identity with round-14
water drown. Sim: document on `waterHazard` / `Drown` / `drownMs`; test
`testAcidHazardSharesDrown`. Mutate-alone: `drownMs 2000→200` fails duration.
Suite **16/16**.

Binary under test: `build/cmake/dev-debug/test_world.exe`.

---

## Left open

- What (if anything) ever populates `0x004C89D8` so type `0x2e` can fire.
- Runtime painter of `0x40` (setter `0x0042081B` still has no E8 caller).
- Whether `0x100` / `[+0x5f]` has a later gameplay effect beyond the probe.
