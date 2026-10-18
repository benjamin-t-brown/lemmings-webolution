# Phase 3 round 19 — Fire

2026-10-16. Stock `C:\progs\Lemmings Revolution\Lemmings Revolution.exe`,
SHA-256 `32675bdb49866e76502713251fdfd2b09078c2481c72ca22ff68ebf4b3737a2f`,
image base `0x400000`.

Harness: `tools/harness/measure_fire.py` (new). Re-run:

```
python tools/harness/measure_fire.py --audit
```

Facilitator owns `PLAN.md` / handoff / rules — this scratch only.

**Result: clean negative.** The cue string is registered and WAV/SPR assets
exist; nothing in the retail exe plays the cue, and there is no world
gadget that attaches a Fire effect to a lemming.

---

## PREDICTIONS (written before the static audit)

### P1 — slot matches brief

`push 0x4c0d2c` at `0x0045E708` is followed by a store to `+0x1c8fc`
(global `0x004C6A08 + 0x1c8fc = DAT_004E3304`). The three imm32 sites
`0x45e71c` / `0x45eb41` / `0x45f186` are register / preload / unload.

Falsified if the store after the Fire push writes a different displacement.

### P2 — play sites

If Fire is a SpeedPad-like gadget, `DAT_004E3304` (or `+0x1c8fc` outside
register/preload/unload) is read by a play path into `FUN_0044535b`,
and/or a `fire*.a3d` / flame / burn object exists in COMPILED.

Falsified by: zero absolute refs to `0x004E3304`, only three `+0x1c8fc`
displacements (all inside cue register/preload/unload), and no fire/flame
/burn a3d in the boxes.

### P3 — decorative assets are not this gadget

`SOUNDS\FIRE.WAV`, `SPRITES\FIRE.SPR`, mine `_FireBraize*` textures, and
`^LAVA_GLOW` are assets or decorations. They do not identify a Fire
lemming gadget unless a play site or object path ties them to a lemming
effect.

Falsified if a `.text` play load of `DAT_004E3304` exists, or an a3d /
exclusive cell-bit gadget is found.

---

## Static reads

| claim | address / bytes |
| --- | --- |
| cue `Fire` | `0x004C0D2C` (`Balloon\0Fire\0\0\0\0Witch…`); sole string imm xref `push 0x4c0d2c` at `0x0045E708` (imm32 at `0x0045E709`) |
| register | `FUN_0045e51f`: after Balloon store `+0x1c8f8`, `FUN_0041d7d0(..., s_Fire_004c0d2c)` → `*(param + 0x1c8fc)` at disp site `0x0045E71C` (`fc c8 01 00`) |
| cue object base | `0x004C6A08`; Fire global `DAT_004E3304` |
| preload | disp `0x0045EB41` (`fc c8 01 00`) |
| unload | disp `0x0045F186` (`fc c8 01 00`) |
| Balloon contrast | sibling cue; also zero absolute global refs (round 18) |
| SpeedPad / Action contrast | play via `DAT_004E32F8` / `DAT_004E32FC` |
| Fire play | **zero** `.text` imm32 to `0x004E3304` (Ghidra xrefs empty; byte scan empty); **zero** of 23 `FUN_0044535b` call sites load Fire |
| `+0x1c8fc` inventory | only `0x45e71c` (write), `0x45eb41` (preload), `0x45f186` (unload) |

P1 holds. P2's gadget/play hypothesis is **falsified** (clean negative).
P3 holds for the audit scope: no play site ties the assets to a lemming.

---

## Offline audit

`--audit` on stock exe + `lemmings.box` / `patch.box`:

| check | result |
| --- | --- |
| `fire` / `flame` / `burn` / `torch` / `lava` `.a3d` in boxes | **none** |
| `SOUNDS\FIRE.WAV` | present (asset only; no exe string path, no LVL path ref) |
| `SPRITES\FIRE.SPR` | present (asset only; no exe string path, no LVL path ref) |
| decorative textures | `_FireBraize01/02` under mine BG; `^LAVA_GLOW` — not a gadget object |
| COMPILED `.LVL` substring `Fire` / `fire` | (1) cue-name table `…Balloon…Fire…Witch…`; (2) `_FireBraize*` texture paths; (3) length-prefixed lowercase `fire` beside `Smoke`/`Bubbles` in an embedded FX-name list — **not** the SFX cue at `0x004C0D2C`, and not an a3d placement |
| exclusive cell bit for a Fire gadget | N/A — no candidate gadget object / play site |

---

## Live series

Not required. Settled by shipped-artifact reads: no play site and no
gadget object to enter. No process launched; no live writes.

---

## Settled

**What:** nothing to a lemming in retail. Cue `Fire` is registered,
preloaded, and unloaded with the other HUD/SFX cues; it is never played.

**How detected:** not detected — no object AABB, no cell bit, no parallel
state. Mine `_FireBraize*` / lava glow textures are decorations. Orphan
`FIRE.WAV` / `FIRE.SPR` are assets without retail play/placement refs.

Sim: no Fire mechanic (`kFireGadgetInRetail = false` in `World.h`).
Harness: `measure_fire.py --audit`. Test: `testFireCueUnused`
(mutate-alone: flip the flag to `true`).

Process: none. Live writes: none.
