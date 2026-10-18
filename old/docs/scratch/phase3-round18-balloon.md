# Phase 3 round 18 — Balloon

2026-10-16. Stock `C:\progs\Lemmings Revolution\Lemmings Revolution.exe`,
SHA-256 `32675bdb49866e76502713251fdfd2b09078c2481c72ca22ff68ebf4b3737a2f`,
image base `0x400000`.

Harness: `tools/harness/measure_balloon.py` (new). Re-run:

```
python tools/harness/measure_balloon.py --audit
```

Facilitator owns `PLAN.md` / handoff / rules — this scratch only.

**Result: clean negative.** The cue string is registered and the WAV asset
exists; nothing in the retail exe plays the cue, and there is no world
gadget that attaches a Balloon effect to a lemming.

---

## PREDICTIONS (written before the static audit)

### P1 — slot correction

Brief lead said Balloon stores at `+0x1c8f4`. Decompile of register
`FUN_0045e51f` predicts **Action** → `+0x1c8f4` and **Balloon** →
`+0x1c8f8` (global `0x004C6A08 + 0x1c8f8 = DAT_004E3300`). The three
imm32 sites named in the brief (`0x45e6ec` / `0x45eb15` / `0x45f15e`)
are Action's register / preload / unload, not Balloon's.

Falsified if store after `push 0x4c0d24` writes `+0x1c8f4`.

### P2 — play sites

If Balloon is a SpeedPad-like gadget, `DAT_004E3300` (or `+0x1c8f8`
outside register/preload/unload) is read by a play path into
`FUN_0044535b`, and/or a `balloon*.a3d` object exists in COMPILED.

Falsified by: zero absolute refs to `0x004E3300`, only three `+0x1c8f8`
displacements (all inside cue register/preload/unload), and no
`balloon.a3d` in the boxes.

### P3 — exit meshes are not this gadget

`Models\exit\balloon_*` / `exit_acid` / `exit_water` / `exit_any` paths
are exit decorations (round 11/12). They do not identify a Balloon
gadget.

---

## Static reads

| claim | address / bytes |
| --- | --- |
| cue `Balloon` | `0x004C0D24` (`Balloon\0Fire\0…`); sole string imm xref `push 0x4c0d24` at `0x0045E6F0` |
| register | `FUN_0045e51f`: `FUN_0041d7d0(..., s_Balloon_004c0d24)` → `*(param_1 + 0x1c8f8)` at store whose disp sits at `0x0045E704` |
| cue object base | `0x004C6A08` (same block SpeedPad uses); Balloon global `DAT_004E3300` |
| preload | `FUN_00406440(..., *(+0x1c8f8))` site disp `0x0045EB2B` (batch with siblings when `+0x1c9b1==0`) |
| unload | `FUN_0045ed81` → `FUN_004065e0(..., *(+0x1c8f8))` site disp `0x0045F172` |
| Action contrast | brief's three sites are Action `+0x1c8f4`; Action **does** play via `MOV EAX,[0x004E32FC]` at `0x004556AA` |
| SpeedPad contrast | play via `DAT_004E32F8` inside `FUN_00432632` (`0x004326D1` / `0x004326FE`) |
| Balloon play | **zero** `.text` imm32 to `0x004E3300`; **zero** of 23 `FUN_0044535b` call sites load Balloon |
| `+0x1c8f8` inventory | only `0x45e704` (write), `0x45eb2b` (preload), `0x45f172` (unload) |

---

## Offline audit

`--audit` on stock exe + `lemmings.box` / `patch.box`:

| check | result |
| --- | --- |
| `balloon*.a3d` in boxes | **none** |
| `SOUNDS\BALLOON.WAV` | present (asset only; no play site) |
| exit decoration paths | `Models\exit\balloon_default_corded`, `exit_acid\balloon_acid`, `exit_water\balloon_water`, `exit_any\balloon_any_*` + textures |
| COMPILED `.LVL` substring `balloon` | exit mesh paths and/or embedded cue-name table (`…SpeedPad…Action…Balloon…Fire…Witch…`) — not a gadget placement |
| exclusive cell bit for a Balloon gadget | N/A — no candidate gadget levels |

P1–P3 hold. P2's gadget/play hypothesis is **falsified** (clean negative).

---

## Live series

Not required. The question is settled by shipped-artifact reads: there is
no play site and no gadget object to enter. No process launched; no live
writes.

---

## Settled

**What:** nothing to a lemming in retail. Cue `Balloon` is registered,
preloaded, and unloaded with the other HUD/SFX cues; it is never played.

**How detected:** not detected — no object AABB, no cell bit, no parallel
state. Exit `balloon_*` meshes are decorations only.

Sim: no Balloon mechanic (`kBalloonGadgetInRetail = false` in `World.h`).
Harness: `measure_balloon.py --audit`. Test: `testBalloonCueUnused`
(mutate-alone: flip the flag to `true`).

Process: none. Live writes: none.
