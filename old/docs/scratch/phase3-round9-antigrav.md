# Phase 3 round 9 — anti-grav (cell bit `0x8000`)

2026-10-16. Stock `C:\progs\Lemmings Revolution\Lemmings Revolution.exe`,
SHA-256 `32675bdb49866e76502713251fdfd2b09078c2481c72ca22ff68ebf4b3737a2f`,
image base `0x400000`, driven through `tools/dinput-shim`.

Harness: `tools/harness/measure_antigrav.py` (new). Re-run:

```
python tools/harness/measure_antigrav.py --audit
python tools/harness/read_steppers.py --at 0x0041C104 --len 0x220
python tools/harness/read_steppers.py --at 0x0042F5E3 --len 0x90
# restart the process, then
python tools/harness/measure_antigrav.py --enter m06_kf_1 --watch-only 50 --hz 100
```

Live spelling is lower-case: `m06_kf_1` (group 4 slot 4).

---

## SUMMARY

| Thing | Status | Value |
| --- | --- | --- |
| **which bit** | **MEASURED, offline, 102/102** | **`0x8000`**. Exactly the 18 levels with `anti_grav.a3d`; no mismatches. |
| **which cell** | **READ** | ground under the feet: `(trunc(X), trunc(Y) - [+0x44])` at `0x0041C104` / tested `0x0041C154`. |
| **what the bit does** | **READ + MEASURED** | walk probe attaches cartwheel (`0x004B0CA8`, type `0x2b`); enter calls `0x0042F5E3` which negates `[+0x44]`; then attaches fall `0x004B0A20`. |
| **live flip** | **MEASURED, 27 flips, m06_kf_1** | every walker whose ground cell was pad `(68,7)` flipped `+1 → -1` at `x = 68.000`, state `walk → fall`, strip **14**, then fell with world Y increasing (8 → 15). |
| **bit alone is not enough** | **MEASURED, clean negative** | OR-ing `0x8000` onto MM_CLIMB_002's pen floor (no `anti_grav.a3d` object) produced **0** flips; the probe also requires a matching object from the list at `0x004C8958` (`0x0041C1E3` / `0x00430BA1`). |
| **`[+0x4c]` from hatch type** | **MEASURED, side finding** | hatch word type 1 (`entrance_acid`) → live `[+0x4c] = 1` on every released lemming of m06_kf_1 / H10_MT_1. Not this round's mechanic. |
| **sim** | landed | `Terrain::antiGrav` from `0x8000`; `Lemming::verticalDir`; walker flips and falls along it. |

---

## PREDICTION (written before the live anti-grav run)

### P1 — bit identity

Across all 102 retail `.LVL` grids, cells carrying `0x8000` are exactly the
18 levels that embed `anti_grav.a3d`. Falsified by any mismatch.

**Holds.** `--audit`: 18 / 18, match True.

### P2 — which cell the walk probe tests

```
0041c104  call 0x40de60              ; cell (trunc(X), trunc(Y - vdir))
0041c10b  mov [ebp-0x18], ecx
0041c154  and edx, 0x8000
```

Falsified if a live walker flips while that ground cell is clear of `0x8000`.

**Holds.** Every flip had `under=(68, 7)` with `(68,7)` in the live `0x8000` set.

### P3 — gravity flip

Cartwheel enter calls `0x0042F5E3` (`[+0x44] := -[+0x44]`). Prediction: a
walker stepping onto a real pad flips `+1 → -1` and falls with world Y
increasing.

**Holds.** 27 flips on m06_kf_1; first:

```
t  5.062s  VDIR +1 -> -1  x=68.000 y=8.040 walk->fall strip 14
           under=(68, 7) on_paint=True
t  5.419s  fall->walk     x=68.065 y=15.000 vdir=-1
```

### P4 — bit without object

Prediction after reading the object-list gate at `0x0041C192`..`0x0041C21C`:
painting `0x8000` onto MM_CLIMB_002 (no anti-grav object) flips nothing.

**Holds.** 0 flips; walker landed on painted `(32,31)` and walked.

---

## Static reads

| claim | address | bytes / observation |
| --- | --- | --- |
| `0x8000` test | `0x0041C154` | `81 e2 00 80 00 00` |
| type `0x2b` gate | `0x0041C162` | `6a 2b` ; `call 0x42ecc9` |
| object-list search | `0x004C8958` | loop at `0x0041C192`; match `0x00430BA1` |
| cartwheel ctor | `0x0041AD90` | vtable `0x004B0CA8`, type `0x2b` |
| gravity flip | `0x0042F5E3` | `6b c9 ff` ; `89 4a 44` |
| fall after | `0x00415FA0` | vtable `0x004B0A20`, type `0x13` |
| fall enter keeps cartwheel anim | `0x0041600B` | `6a 2b` ; `jne` skips `"fall"` |

Landing-path splat exemptions (`0x200` / `[+0x4c]==1`, `0x4000` /
`[+0x4c]==2` at `0x0041662B` / `0x00416641`) are **not** this mechanic.

---

## Live series

### Clean negative — MM_CLIMB_002 paint

Painted columns 28..48 at file row 31 with `0x8000` (words `0x1 → 0x8001`).
One walker landed on `(32,31)` (`on_paint=True`) and stayed `vdir=+1` for the
whole watch. All 21 cells restored; process later exited so post-restore
reads were unreadable, but each restore write used the logged old word.

### m06_kf_1 — natural pads

Live `0x8000` cells: `(24,88)`, `(24,89)`, `(25,88)`, `(25,89)`, `(68,7)`,
`(69,7)`. Hatch at column 43 feeds the upper floor (world Y ≈ 8) past pad
`(68,7)`.

| t (s) | event |
| --- | --- |
| 5.062 | first flip: x=68.000 y=8.040, `+1→-1`, walk→fall, strip 14 |
| 5.419 | land inverted: y=15.000, vdir=-1, walk |
| … | 26 further `+1→-1` flips at the same pad, same shape |
| (recycle) | some pointers show `-1→+1` at hatch respawn — new birth, not a pad |

Strip 14 during the transition is the round-10 unnamed strip
(`16` frames, divisor `2` → `0.480 s`). Hypothesis, not settled: that strip
is `cartwheel`.

---

## Verdict

Anti-grav is cell bit **`0x8000`**, tested on the ground under the feet, and
its effect is to **negate `[lemming+0x44]`** through the cartwheel state's
enter hook, then fall under the new gravity. Settled for the upright→inverted
transition on a real pad. The sim carries `Terrain::antiGrav` and
`Lemming::verticalDir` and flips a walker that stands on a pad.

Test `testAntiGravFlipsGravity` fails when the flip is disabled alone
(`verticalDir became -1 within 20 ticks`), and the full suite is 16/16 green.

---

## Left open (with citations)

- **Object overlap predicate** at `0x00430BA1`: which bounds of the anti-grav
  placement must contain the lemming. The bit alone is insufficient
  (MM_CLIMB_002 paint).
- **Cartwheel strip index / spinner duration** (`state+0x18` from
  `rate*3/2` at `0x0041AE65`). Strip 14 is a candidate only.
- **Zone bits `0x200` / `0x2000` / `0x4000`** and splat exemptions — separate
  gadgets; `[+0x4c]` now known to come from the hatch type word, but which
  placed object stamps those zone bits is unread.
- **Inverted step-up / climb / dig**: this round only generalises fall and
  the walker's ground / step-down along `verticalDir`.
- **Re-flip on the same pad** after `0x2b` has left: not measured; the
  original's gate is `0x0042ECC9(0x2b)`.

## Process / writes

Final game process: not running (exited after the m06 watch). The only live
grid writes this round were the MM_CLIMB_002 paint; every cell was restored
from the logged old word before the handle was closed.
