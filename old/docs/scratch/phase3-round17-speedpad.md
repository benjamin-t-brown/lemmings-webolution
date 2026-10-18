# Phase 3 round 17 — SpeedPad

2026-10-16. Stock `C:\progs\Lemmings Revolution\Lemmings Revolution.exe`,
SHA-256 `32675bdb49866e76502713251fdfd2b09078c2481c72ca22ff68ebf4b3737a2f`,
image base `0x400000`, driven through `tools/dinput-shim`.

Harness: `tools/harness/measure_speedpad.py` (new). Re-run:

```
python tools/harness/measure_speedpad.py --audit
# restart the process, then
python tools/harness/measure_speedpad.py --enter VH21_JC --dump-objects --watch 45 --hz 80
# forced attach proof (restores writes):
python tools/harness/_tmp_speedpad_proof3.py
```

Facilitator owns `PLAN.md` / handoff / rules — this scratch only.

---

## PREDICTIONS (written before live runs)

### P1 — asset census

`speedup_two_way.a3d` appears in **9** retail COMPILED levels (16
placements). No cell bit is exclusive to those 9. `pressure_pad.a3d`
co-occurs on some but not all (speed-only: H10_MT_1, SHOW_MT3, VH21_JC).

Falsified by `--audit` mismatch.

### P2 — behaviour

Overlapping a SpeedPad attaches the FASTER parallel state (vtable
`0x004B0A38`, type `0x10`). Enter (`0x0041688C`) sets lemming `+0x50 = 2`
and a 5.0 s countdown at state `+0x18`. While `+0x50 == 2`, the actor
tick `0x0042E2BF` runs the main state **twice** per frame → ~2× walk
rate. Detection is object AABB (`0x00430BA1`), not a cell bit.

Falsified by: 45 s watch on H10_MT_1 with hatch traffic and **no**
`+0x50 == 2` / no FASTER on the parallel list / no walk-rate jump near a
`speedup_two_way` pose.

### P3 — which object

Live object whose pose matches `speedup_two_way.a3d` owns the attach.
Candidates: vtables `0x004B1148` (AABB message pad), `0x004B1178`,
`0x004B12B4`, `0x004B1288` (lever — **out of scope** as the mechanic).
Cue string `SpeedPad` at `0x004C0D10` is registry-only unless a play site
appears.

---

## Static reads

| claim | address / bytes |
| --- | --- |
| cue `SpeedPad` | `0x004C0D10`; xref `0x0045E6C0` (cue table load) → `DAT_004E32F8` |
| cue play on activate | `DAT_004E32F8` xrefs `0x004326D1` / `0x004326FE` inside `FUN_00432632` |
| object vtable | `0x004B1178` → step `0x0043260D` → `FUN_004339fc`; overlap `0x0043259F` → `FUN_00432742`; activate `0x004325E3` → `FUN_00432632(this,1)` |
| overlap | `FUN_00432742`: list `0x004C88D0` + AABB `0x00430BA1`; facing filter `lem+0x40` vs `obj+0xbc` (0=any); vdir via `obj+0xa0` (0→upright, else inverted) |
| activate | `FUN_00432632`: latch `+0xb8=1`; for each hit in `+0xd8` call `FUN_00433B2E(this+0xc0)`; play cue |
| embed message | live `+0xc4..`: `(−2, 6, 1)` |
| attach factory | `FUN_00433B2E`: `+4==-2 && +8==6 && +0xc==1` → `FUN_00416730` (FASTER) |
| FASTER ctor | `FUN_00416730` → vtable `0x004B0A38`, type `0x10`, `+0x11=1` (parallel list) |
| FASTER enter | `0x0041688C`: `+0x50=2`, timer double `5.0` (`0x40140000` hi at state `+0x18`) |
| FASTER tick | `0x0041677C`: countdown; on expiry `+0x50=1` |
| 2× loop | `0x0042E2BF`: `if (+0x50==2) local_14=2` |
| SLOWER (sibling, not this round) | type `0xf`, vtable `0x004B0C90`; enter sets `+0x50=3` |

---

## Offline audit

`--audit` on `lemmings.box` + `patch.box` (102 COMPILED levels):

| check | result |
| --- | --- |
| `speedup_two_way.a3d` levels | **9** / **16** placements |
| `pressure_pad.a3d` levels | 23; co-occur 6; speed-only H10_MT_1, SHOW_MT3, VH21_JC |
| bits exclusive to all speedup levels | **[]** (empty) |

9 levels: `H08_MT_1` `H10_MT_1` `H15_MT_1` `H17_MG_1` `M21_KF_1`
`SHOW_MT3` `VH06_MT` `VH16_MT` `VH21_JC`.

P1 holds. Cell-bit-alone detection is **falsified** (clean negative).
`pressure_pad.a3d` is **not** required for SpeedPad (speed-only levels exist;
trampoline round 13 already cleared it for bounce).

---

## Live series

### H10_MT_1

Entered; pad pose Y=29 (file) / live object present but **culled** — timer
poke did not decrement (not stepped). Natural watch: no FASTER. Not a
behavioural negative for the mechanic; the pad was not in the stepped set.

### VH21_JC

`VERIFY loaded_level='Compiled\\VH21_JC.lvl'`. Two live objects, vtable
`0x004B1178`:

| ptr | pose (X,Y,Z) | AABB X | AABB Y | msg at +0xc4 |
| --- | --- | --- | --- | --- |
| `0x027b91f0` | (223, 63, 0) | [222..224] | [63..69] | (−2, 6, 1) |
| `0x027d3090` | (18, 63, 0) | [17..19] | [63..69] | (−2, 6, 1) |

Timer poke decrements (pads stepped). `face_filt=+0xbc=0`, `a0=0`
(upright, any facing). Period `+0x98=0` (latch clears only on leave).

### Forced walker-on-pad (`_tmp_speedpad_proof3.py`)

Prediction: placing a listed walker on the natural pad AABB attaches
FASTER (`0x004B0A38`) with timer 5.0 and sets `+0x50=2`; column rate under
direct `+0x50=2` poke is ~2× `+0x50=1`. Falsified by no FASTER node / no
mode flip / ratio ≈1.

Result:

- HIT frame=1 `mode=2` `hasF=True` sides `[(0x280b070, 0x4b0a38, 5.0)]`
- timer 5.0 → 4.444 over ~0.5 s
- poke `mode2 col/s=0.028` vs `mode1 col/s=0.013` (~2.15×)

P2 and P3 hold. Object is `0x004B1178` / `speedup_two_way.a3d`. Cue string
plays on activate via `DAT_004E32F8` (not registry-only).

---

## Settled

**What:** temporary ~2× walk for 5.0 s (FASTER parallel state).
**How detected:** object AABB on `speedup_two_way.a3d` (vtable
`0x004B1178`), list `0x004C88D0` + `0x00430BA1`, facing/vdir filters;
embed message `(−2,6,1)` → FASTER. Not a cell bit. Not `pressure_pad.a3d`.

Sim: `SpeedPad` + `Rules::fasterMs=5000`; walker double-step while
`Lemming::fasterMs > 0`. Harness: `measure_speedpad.py`. Test:
`testSpeedPadFaster` (mutate-alone: delete `speedPadAt` arm or zero
`fasterMs`).

All live writes restored in the proof scripts. Process was already gone
after the suite (no Lemmings/Revolution PID); nothing left to kill.
