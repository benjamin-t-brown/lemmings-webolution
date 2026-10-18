# Phase 3 round 22 — ForceF

2026-10-16. Stock `C:\progs\Lemmings Revolution\Lemmings Revolution.exe`,
SHA-256 `32675bdb49866e76502713251fdfd2b09078c2481c72ca22ff68ebf4b3737a2f`,
image base `0x400000`.

Harness: `tools/harness/measure_forcef.py` (new). Re-run:

```
python tools/harness/measure_forcef.py --audit
```

Facilitator owns `PLAN.md` / handoff / rules — this scratch only.

**Result: clean negative.** The cue string is registered and WAV assets
exist; nothing in the retail exe plays the cue, and there is no world
gadget that attaches a ForceF effect to a lemming.

---

## PREDICTIONS (written before the static audit)

### P1 — slot matches brief

`push 0x4c0cc8` at `0x0045E600` is followed by a store to `+0x1c8d0`
(global `0x004C6A08 + 0x1c8d0 = DAT_004E32D8`). The three imm32 sites
`0x45e614` / `0x45ea39` / `0x45f0aa` are register / preload / unload.

Falsified if the store after the ForceF push writes a different
displacement.

### P2 — play sites

If ForceF is a SpeedPad/Lever-like gadget, `DAT_004E32D8` (or `+0x1c8d0`
outside register/preload/unload) is read by a play path into
`FUN_0044535b` / `FUN_0044521b`, and/or a `force*` / `fan*` / `blow*`
`.a3d` object exists in COMPILED.

Falsified by: zero absolute refs to `0x004E32D8`, only three `+0x1c8d0`
displacements (all inside cue register/preload/unload), and no
force/fan/blow a3d in the boxes.

### P3 — orphan WAVs are not this gadget

`SOUNDS\FORCEF.WAV` and `SOUNDS\FORCEFLD.WAV` are assets. They do not
identify a ForceF lemming gadget unless a play site or object path ties
them to a lemming effect. (Hypothesis: FORCEFLD is a leftover sibling
name with no cue string in the retail exe.)

Falsified if a `.text` play load of `DAT_004E32D8` exists, or an a3d /
exclusive cell-bit gadget is found.

---

## Static reads

| claim | address / bytes |
| --- | --- |
| cue `ForceF` | `0x004C0CC8` (`Splat\0\0\0ForceF\0\0Die\0FallSplat…`); sole string imm xref `push 0x4c0cc8` at `0x0045E600` (imm32 at `0x0045E601`) |
| register | `FUN_0045e51f`: after Splat store `+0x1c8cc`, `FUN_0041d7d0(..., s_ForceF_004c0cc8)` → `*(param + 0x1c8d0)` at store opcode `0x0045E612` / disp site `0x0045E614` (`d0 c8 01 00`) |
| cue object base | `0x004C6A08`; ForceF global `DAT_004E32D8` |
| preload | disp `0x0045EA39` (`d0 c8 01 00`) |
| unload | disp `0x0045F0AA` (`d0 c8 01 00`) |
| sibling contrast | Die (`+0x1c8d4` / `DAT_004E32DC`) has absolute play ref(s); SpeedPad / Action / Lever globals play |
| ForceF play | **zero** `.text` imm32 to `0x004E32D8`; **zero** imm32 anywhere in the PE file; **zero** of 23 `FUN_0044535b` and 6 `FUN_0044521b` callers load ForceF |
| `+0x1c8d0` inventory | only `0x45e614` (write), `0x45ea39` (preload), `0x45f0aa` (unload) |

P1 holds. P2's gadget/play hypothesis is **falsified** (clean negative).
P3 holds for the audit scope: no play site ties the WAVs to a lemming.

Facilitator lead of **0** abs refs to `DAT_004E32D8` **re-verified**
(empty `.text` scan and empty whole-PE imm32 scan).

---

## Offline audit

`--audit` on stock exe + `lemmings.box` / `patch.box`:

| check | result |
| --- | --- |
| `force` / `fan` / `blow` / `wind` `.a3d` in boxes | **none** (windmill BG textures only; `FANFARESTART.WAV` unrelated) |
| `SOUNDS\FORCEF.WAV` | present (asset only; no exe string path beyond cue name) |
| `SOUNDS\FORCEFLD.WAV` | present; **no** `ForceFld` / `FORCEFLD` string in retail exe |
| COMPILED `.LVL` substring `ForceF` | embedded cue-name table only (`…Splat…ForceF…Die…` beside SpeedPad/Balloon/…) — not an a3d placement |
| exclusive cell bit for a ForceF gadget | N/A — no candidate gadget object / play site |

---

## Live series

Not required. Settled by shipped-artifact reads: no play site and no
gadget object to enter. No process launched; no live writes.

---

## Settled

**What:** nothing to a lemming in retail. Cue `ForceF` is registered,
preloaded, and unloaded with the other HUD/SFX cues; it is never played.

**How detected:** not detected — no object AABB, no cell bit, no parallel
state. Orphan `FORCEF.WAV` / `FORCEFLD.WAV` are assets without retail
play/placement refs.

Sim: no ForceF mechanic (`kForceFGadgetInRetail = false` in `World.h`).
Harness: `measure_forcef.py --audit`. Test: `testForceFCueUnused`
(mutate-alone: flip the flag to `true`).

Process: none. Live writes: none.
