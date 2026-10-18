# Phase 3 round 23 — Axe

2026-10-16. Stock `C:\progs\Lemmings Revolution\Lemmings Revolution.exe`,
SHA-256 `32675bdb49866e76502713251fdfd2b09078c2481c72ca22ff68ebf4b3737a2f`,
image base `0x400000`.

Harness: `tools/harness/measure_axe.py` (new). Re-run:

```
python tools/harness/measure_axe.py --audit
```

Facilitator owns `PLAN.md` / handoff / rules — this scratch only.

**Result: no lemming gadget.** `Axe` is a **HUD sprite-sheet** name
(`SPRITES\AXE.SPR` → `TEXTURES\SHEETS\$AXE0.BMP`), registered via the
sprite-sheet registrar (not the SFX table). The sole absolute ref to its
global is a **UI sprite bind**, not an SFX play and not a lemming
overlap / cell-bit / parallel-state path. Not Balloon/Fire-shaped
(those have zero abs refs); not Witch-shaped (Witch is an SFX easter
egg). Shape: UI sheet like Bits / GreyIcons / BlackIcons / AdditiveIcons
/ Smoke.

---

## PREDICTIONS (written before the static audit)

### P1 — slot and registrar match the brief

`push 0x4c0d6c` at `0x0045E7C3`, `mov ecx, 0x58d23c`, `call 0x00446040`,
store to `+0x1c914` at `0x0045E7D5` (global `0x004C6A08 + 0x1c914 =
DAT_004E331C`). Slot imm sites include the brief’s four addresses
`0x45e7d7` / `0x45ebeb` / `0x45eedc` / `0x45eef7` (register + preload +
two unload-family ops — sprite sheets use a longer teardown than SFX).

Falsified if the store after the Axe push writes a different
displacement, or if registration goes through the usual SFX helper
`FUN_0041d7d0`.

### P2 — the abs ref at `0x00496E40` is real and not SFX-shaped

`mov eax, [0x4e331c]` at `0x00496E40` then `call 0x00496D5B` is a live
use. Hypothesis: it binds the Axe **sprite sheet** into in-level UI, not
`FUN_0044535b` / `FUN_0044521b` cue play.

Falsified if that call is an SFX play helper, or if the path attaches a
lemming state / kills via actor list `0x004CBB18`.

### P3 — lemming effect hypothesis

If Axe is a SpeedPad/Lever-like world gadget, there is an `axe*.a3d`
(or exclusive cell bit) whose overlap kills or transforms a lemming.

Falsified by: no axe a3d; LVL “axe” hits only in embedded name tables
next to Smoke/Bubbles; play/bind site not on an object/overlap path.

### P4 — string neighborhood is sprite sheets, not SFX cues

Neighbor strings `Bits` / `GreyIcons` / `BlackIcons` / `AdditiveIcons` /
`Axe` / `Smoke` / `_curNoLem` / `_curLem` predict a cursor / HUD sheet
batch. `SPRITES\AXE.SPR` exists; `SOUNDS\AXE.WAV` does not.

Falsified if Axe registers via the SFX path or a WAV is required.

---

## Static reads

| claim | address / bytes |
| --- | --- |
| name `Axe` | `0x004C0D6C` (`…AdditiveIcons\0\0\0Axe\0Smoke\0…`); sole string imm xref `push 0x4c0d6c` at `0x0045E7C3` (imm32 at `0x0045E7C4`) |
| register | **not** `FUN_0041d7d0`: `push 0x4c0d6c` / `mov ecx, 0x58d23c` / `call 0x00446040` → `mov [ecx+0x1c914], eax` at `0x0045E7D5` (disp imm at `0x0045E7D7`) |
| cue/object base | `0x004C6A08`; Axe global `DAT_004E331C` |
| sibling batch (same registrar) | Bits `+0x1c?` via `0x4c0d3c`; GreyIcons `0x4c0d44` → `+0x1c908`; BlackIcons `0x4c0d50` → `+0x1c90c`; AdditiveIcons `0x4c0d5c` → `+0x1c910`; Axe → `+0x1c914`; Smoke `0x4c0d70` → `+0x1c904` |
| `+0x1c914` inventory | `0x0045E7D7` write; `0x0045EBEB` preload (`push` → `call 0x0042FF90`); `0x0045EEDC` / `0x0045EEF7` unload-family (`0x00446010` / `0x00430050`) — **four** sites |
| abs play/bind load | **one** `.text` imm32 to `0x004E331C`: at `0x00496E41` inside `FUN_00496e39` (`0x00496E40` `a1 1c 33 4e 00` / `mov eax,[DAT_004E331C]`) |
| bind call | `0x00496E49` `e8 0d ff ff ff` → `FUN_00496d5b` (sprite bind; **not** `FUN_0044535b` / `FUN_0044521b`) |
| bind caller | sole `call FUN_00496e39` at `0x00419506` from `FUN_004194f0` (`this+0xb0`); that from level-UI init `FUN_004188f6` at `0x00418E5B` |
| SFX contrast | **0** of `FUN_0044535b` / `FUN_0044521b` call sites load `DAT_004E331C` |
| Balloon/Fire contrast | those globals have **0** abs refs; Axe has **1** (UI bind) |
| Witch contrast | Witch abs ref is SFX helper `FUN_0044521b` on pause; Axe is sprite bind `FUN_00496d5b` |

P1 holds. P2 holds (bind site real, not SFX). P3’s gadget hypothesis is
**falsified**. P4 holds.

---

## Offline audit

`--audit` on stock exe + `lemmings.box` / `patch.box`:

| check | result |
| --- | --- |
| `axe*.a3d` in boxes | **none** |
| `SPRITES\AXE.SPR` | present (embeds `Sheets\$Axe0`; dims in header include 80×48) |
| `TEXTURES\SHEETS\$AXE0.BMP` | present |
| `SOUNDS\AXE.WAV` | **absent** |
| COMPILED `.LVL` substring `Axe` | embedded name tables next to `AdditiveIcons` / `Smoke` / `Bubbles` (~100 levels) — not a gadget placement |
| exclusive cell bit for an Axe gadget | N/A — no candidate gadget object |

---

## Live series

Not required for the lemming question. Settled by shipped-artifact reads:
the only absolute use of `DAT_004E331C` is the UI sprite bind path above;
no actor-list / overlap / cell-bit path. No process launched; no live
writes.

---

## Settled

**What to a lemming:** nothing in retail. No object AABB, no cell bit, no
parallel state, no `axe.a3d`, no axe WAV.

**How the name is used:** sprite-sheet register / preload / unload via
`FUN_00446040` (and sibling teardown); bound once into in-level UI by
`FUN_00496e39` reading `DAT_004E331C`. Same family as Bits / GreyIcons /
BlackIcons / AdditiveIcons / Smoke.

Sim: no Axe lemming mechanic (`kAxeGadgetInRetail = false` in
`World.h`). Harness: `measure_axe.py --audit`. Test:
`testAxeSheetNoLemmingsGadget` (mutate-alone: flip the flag to `true`).

Process: none. Live writes: none.
