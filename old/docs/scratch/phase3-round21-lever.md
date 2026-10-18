# Phase 3 round 21 — Lever

2026-10-16. Stock `C:\progs\Lemmings Revolution\Lemmings Revolution.exe`,
SHA-256 `32675bdb49866e76502713251fdfd2b09078c2481c72ca22ff68ebf4b3737a2f`,
image base `0x400000`, driven through `tools/dinput-shim`.

Harness: `tools/harness/measure_lever.py` (new). Re-run:

```
python tools/harness/measure_lever.py --audit
# restart the process, then
python tools/harness/measure_lever.py --enter E16_KF_1 --dump-objects --proof
```

Facilitator owns `PLAN.md` / handoff / rules — this scratch only.

---

## PREDICTIONS (written before live runs)

### P1 — asset census

`lever.a3d` appears in **55** retail COMPILED levels (**130** placements).
No cell bit is exclusive to those 55. Poses pass the world-pose tube check
(real world objects, not FX-only).

Falsified by `--audit` mismatch.

### P2 — object identity

Live objects whose poses match `lever.a3d` use vtable **`0x004B1288`**
(ctor install at `0x00436CE6`). Cue `Lever` at `0x004C0CE0` registers to
`DAT_004E32E4` (`+0x1c8dc`) and is **played** on activate (`0x00436E29`).

Falsified by: zero `0x004B1288` objects on a lever level, or cue global
never loaded at the activate site.

### P3 — detection + effect

Detection is object AABB (`0x00430BA1`) over list `0x004C88D0` inside
`0x00436D30`, with a **facing filter** against toggle state `+0xb8`:
`+0xb8==1` requires facing `−1`; otherwise facing `+1`. On hit, activate
`0x00436E14` plays the Lever cue, calls `0x00436E6A(facing)` which sets
`+0xb8` to that facing, picks message list `+0xcc` (face +1) or `+0xbc`
(face −1), and fans out via `0x00433AC0` → `FUN_00433B2E`. **No** lemming
parallel state / type attach on this path.

Falsified by: forced AABB place with correct facing and **no** `+0xb8`
flip; or a new parallel state on the actor at pull time.

---

## Static reads

| claim | address / bytes |
| --- | --- |
| cue `Lever` | `0x004C0CE0`; `push 0x4c0ce0` @ `0x0045E648`; store `+0x1c8dc` @ `0x0045E65A` |
| cue global | `0x004C6A08+0x1c8dc` = `DAT_004E32E4` |
| cue play | sole abs load @ `0x00436E29` → `call 0x0044535b` (via `0x00406a00`) |
| vtable | `0x004B1288`; ctor `mov [eax],0x4b1288` @ `0x00436CE6` |
| overlap | `0x00436D30`: list `0x004C88D0` + AABB `0x00430BA1`; latch hit at `+0xa4` |
| activate | `0x00436E14` → cue + `0x00436E6A` |
| toggle | `0x00436E6A`: face+1 → `+0xb8=1`, list `+0xcc`; face−1 → `+0xb8=-1`, list `+0xbc`; anim `"lever"` @ `0x004BF940`.. |
| messages | `0x00433AC0` walks list (elem stride `0x18` via `0x0042CAA0`) → `FUN_00433B2E` |
| shared step | vt+0x14 = `0x004339FC` (timer gate; activate only when `+0xa8 <= 0`) |

---

## Offline audit

`--audit` on `lemmings.box` + `patch.box` (102 COMPILED levels):

| check | result |
| --- | --- |
| `lever.a3d` levels | **55** |
| placements with pose | **130** |
| bits exclusive to all lever levels | **[]** (empty) |

P1 holds. Cell-bit-alone detection is **falsified** (clean negative).

---

## Live series (E16_KF_1)

Entered via `measure_float_splat.enter` / goto_level record swap.
`VERIFY loaded_level='Compiled\\E16_KF_1.lvl'`.
`slot [0][0] restored: True` immediately after enter.

### Live lever object (vtable `0x004B1288`)

| ptr | pose (X,Y,Z) | AABB X | AABB Y | +0xb8 |
| --- | --- | --- | --- | --- |
| `0x027a3060` | (21, 13, 0.5) | [20..22] | [13..19] | 1 |

AABB shape: **col ±1 × [ty .. ty+6]** closed (2×6 world units) — same
closed box as SpeedPad round 17. Timer poke `0.8 → 0.600` (object stepped).

### Message lists (container `+0xc` = array, stride `0x18`)

Face−1 list (`+0xbc`, dispatched when pull sets `+0xb8=-1`):

| i | id | type | params |
| --- | --- | --- | --- |
| 0..3 | 6,5,4,3 | **5** | (0,0,0) |

Face+1 list (`+0xcc`, when pull sets `+0xb8=1`):

| i | id | type | params |
| --- | --- | --- | --- |
| 0..3 | 6,5,4,3 | **4** | (0,0,0) |

`FUN_00433B2E`: type 4 → target vt+4; type 5 → target vt+8. Linked door /
gate objects are **not** this round's sim target; the dispatch path is
READ + listed live.

### Forced pull (`--proof`)

Prediction: pin walker in AABB with face −1, timer 0 → `+0xb8` 1→−1; no
FASTER / new parallel vt. Falsified by no flip.

Result:

- `HIT frame=1 +0xb8 1->-1`
- `par before ['0x4b09c0'] after ['0x4b09c0'] faster=False`
  (`0x4b09c0` is the ubiquitous side-effect type from round 14, not Lever)
- First attempt without timer-zero / re-pin: overlap set `hit` but activate
  skipped (`FUN_004339fc` cooldown) and walker walked out of the 2-col box
  — **not** a behavioural negative; method bug, fixed before the holding run
- Lever blob + lemming pose prefix restored `True`; post-restore `+0xb8=1`

P2 and P3 hold.

---

## Settled

**What (lemming):** nothing — no parallel state, no type attach, no speed
change.

**What (world):** facing-gated toggle of lever `+0xb8`, Lever cue + `"lever"`
anim, then message fanout to linked object ids (type 4 one way / type 5 the
other) via `FUN_00433B2E`.

**How detected:** object AABB on `lever.a3d` (vtable `0x004B1288`), list
`0x004C88D0` + `0x00430BA1`, facing vs `+0xb8`. Not a cell bit.

Sim: `Lever` + `leverTriggerFromPose`; walker pull flips `Lever::state`.
Harness: `measure_lever.py`. Test: `testLeverToggle` (mutate-alone: `#if 0`
the `leverAt()` arm in `World::step`).

---

## Process / restores

- Live writes: level-table slot `[0][0]` (goto_level) restored `True`; proof
  restored full lever `0x100` blob and lemming `0x50` pose prefix.
- Process `38816` (`Lemmings Revolution.exe`) killed at closeout.
  No Lemmings process left.
