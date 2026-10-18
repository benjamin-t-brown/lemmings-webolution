# Phase 3 round 28 — Die

2026-10-16. Stock `C:\progs\Lemmings Revolution\Lemmings Revolution.exe`,
SHA-256 `32675bdb49866e76502713251fdfd2b09078c2481c72ca22ff68ebf4b3737a2f`,
image base `0x400000`.

Harness: `tools/harness/measure_die.py` (new). Re-run:

```
python tools/harness/measure_die.py --audit
```

Facilitator owns `PLAN.md` / handoff / rules — this scratch only.

**Result: no Die world gadget.** Cue `Die` is a real SFX play (not
Balloon/ForceF-shaped). The sole absolute load of `DAT_004E32DC` is inside
type-`0x23` enter `FUN_00404261` (vtable `0x004B03C0` +8) when fluid
object `+0x128 == 1` → channel `FUN_00406a00` → `FUN_0044535b`. That is
SFX on the already-measured fluid-mismatch death path (Round 24); Die
does not attach a new state. Distinct from fall-splat cue `FallSplat`
(type `0x1a` enter) and from registered-but-unplayed cue `Splat`. No
`die.a3d`.

---

## PREDICTIONS (written before the static audit)

### P1 — slot matches brief

`push 0x4c0cd0` at `0x0045E618`; store to `+0x1c8d4` (global
`0x004C6A08 + 0x1c8d4 = DAT_004E32DC`). Displacement sites only
register / preload / unload (`0x45e62c` / `0x45ea4f` / `0x45f0be`).

Falsified if the store after the Die push writes a different
displacement, or extra real `+0x1c8d4` sites exist.

### P2 — sole abs play is type-0x23 enter (fluid subtype 1)

Facilitator lead `0x004042CF` is the imm32 of
`mov ecx,[DAT_004E32DC]` inside enter `FUN_00404261`. Gate
`fluid+0x128 == 1` then `FUN_00406a00` → `FUN_0044535b`. No other
`.text` imm32 hits to `DAT_004E32DC`.

Falsified if extra absolute loads exist, or the load is outside that
enter / not on the subtype-`1` arm.

### P3 — SFX on measured death, not a Die gadget / not fall splat

No `die.a3d`. Cue does not mutate lemming state beyond the type-`0x23`
path Round 24 already settled. Contrast: `FallSplat` (`DAT_004E32E0`)
plays from splat-state enter `FUN_0040b3ec` (vtable `0x004B0668`, type
`0x1a`); cue `Splat` (`DAT_004E32D4` / `+0x1c8cc`) has **0** absolute
play refs (register/preload/unload only).

Falsified if a Die a3d / exclusive cell bit / Die-named attach path
exists, or if Die shares the splat-state enter.

---

## Static reads

| claim | address / bytes |
| --- | --- |
| cue `Die` | `0x004C0CD0` (`…ForceF\0\0Die\0FallSplat…`); sole string imm `push` at `0x0045E618` (imm32 at `0x0045E619`) |
| register | `FUN_0045e51f`: `FUN_0041d7d0(..., s_Die)` → `*(+0x1c8d4)` store disp `0x0045E62C` (`89 81 d4 c8 01 00` at opcode `0x0045E62A`) |
| cue object base | `0x004C6A08`; Die global `DAT_004E32DC` |
| preload / unload | disp `0x0045EA4F` / `0x0045F0BE` |
| `+0x1c8d4` inventory | **only** those three sites |
| abs play loads | **exactly one**: imm32 `0x004042CF` (`mov ecx,[DAT_004E32DC]` insn `0x004042CD`) |
| containing enter | `FUN_00404261` (`0x00404261`..`0x00404371`); vtable `0x004B03C0` +8 = enter |
| vtable | `0x004B03C0`: dtor `0x4049b0` / step `0x404372` / enter `0x404261` |
| enter gate | `iVar4 = *(lem+0x54)+0x128`; `== 0` particle arm; `== 1` → Die SFX |
| play arm | `push 0` / `mov ecx,[DAT_004E32DC]` / `call FUN_00406a00` (`0x004042D7`) / `call FUN_0044535b` (`0x004042E5`) |
| ctor / attach | unchanged from Round 24: ctor `FUN_004041fd` type `0x23`; sole attach `0x0041C3BA` |
| actor list | **no** `0x004CBB18` imm inside `FUN_00404261` |
| `FallSplat` contrast | string `0x004C0CD4` → `+0x1c8d8` / `DAT_004E32E0`; sole abs play imm `0x0040B4B6` inside splat enter `FUN_0040b3ec` (vtable `0x004B0668` +8; ctor `FUN_0040b3a0` type `0x1a`; fall-land attach `FUN_00416433`) |
| `Splat` contrast | string `0x004C0CC0` → `+0x1c8cc` / `DAT_004E32D4`; **0** absolute `.text` refs; slot only register/preload/unload — cue registered, not played |

P1–P3 hold.

---

## Offline audit

`--audit` on stock exe + `lemmings.box` / `patch.box`:

| check | result |
| --- | --- |
| `die*.a3d` in boxes / COMPILED `.LVL` | **none** |
| `SOUNDS\DIE.WAV` | present (played via the cue) |
| `SOUNDS\SPLAT.WAV` / `SOUNDS\FALLSPLAT.WAV` | present (sibling cues; FallSplat is the fall-death play) |
| exclusive Die cell bit / a3d placement | N/A — no Die gadget object |

---

## Live series

Not required for the Die-cue question. Sole absolute play load, enter
gate, and Splat/FallSplat contrast are static. No process launched; no
live writes.

---

## Settled

**What to a lemming:** the cue itself is **SFX only**. There is **no** Die
world gadget / no Die attach. The **caller** is main state type **`0x23`**
enter (already Round 24): anim `"drown"` always; when fluid
`+0x128 == 1`, play `Die` once via `FUN_0044535b`. Death/leave remains
that state's step — Die does not open a parallel death path.

**vs Splat / FallSplat:**

| cue | global | when played |
| --- | --- | --- |
| `Die` | `DAT_004E32DC` | type-`0x23` enter, fluid subtype `1` |
| `FallSplat` | `DAT_004E32E0` | type-`0x1a` splat enter after long fall (`FUN_00416433` → ctor `FUN_0040b3a0`) |
| `Splat` | `DAT_004E32D4` | **never** (register/preload/unload only) |

**When played:** exactly once on enter of type `0x23` if the fluid
object named by `lem+0x54` has `+0x128 == 1`.

Sim: no Die gadget (`kDieGadgetInRetail = false` in `World.h`).
Harness: `measure_die.py --audit`.
Test: `testDieCueNoGadget` (mutate-alone: flip the flag to `true`).

Process: none. Live writes: none.

Mutate-alone `kDieGadgetInRetail=true` → FAIL; restored + touch →
suite **16/16**. `--audit` AUDIT PASS.
