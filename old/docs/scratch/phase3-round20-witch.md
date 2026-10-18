# Phase 3 round 20 — Witch

2026-10-16. Stock `C:\progs\Lemmings Revolution\Lemmings Revolution.exe`,
SHA-256 `32675bdb49866e76502713251fdfd2b09078c2481c72ca22ff68ebf4b3737a2f`,
image base `0x400000`.

Harness: `tools/harness/measure_witch.py` (new). Re-run:

```
python tools/harness/measure_witch.py --audit
```

Facilitator owns `PLAN.md` / handoff / rules — this scratch only.

**Result: no lemming gadget.** Cue `Witch` is registered and **is** played
(one Action-style site), but only as a pause-key easter egg on
`compiled\e10_kf_1.lvl`. Nothing attaches a Witch effect to a lemming.
Not a Balloon/Fire-shaped “never played” clean negative — the play site
is real and explained.

---

## PREDICTIONS (written before the static audit)

### P1 — slot matches brief (not sequential +0x1c900)

`push 0x4c0d34` at `0x0045E720` is followed by a store to `+0x1c8a8`
(global `0x004C6A08 + 0x1c8a8 = DAT_004E32B0`). Preload / unload sites
are `0x0045EB57` / `0x0045F19A`.

Falsified if the store after the Witch push writes a different
displacement (e.g. sequential `+0x1c900` after Fire’s `+0x1c8fc`).

### P2 — play site is real and Action-style

`DAT_004E32B0` has an absolute `.text` read at `0x0045D200`
(`mov ecx,[0x4e32b0]`) then `call 0x0044521B` (same helper Action uses).
This is **not** Balloon/Fire-shaped unless that site is a false positive
or dead.

Falsified by: zero absolute refs to `0x004E32B0`, or the call not being
a cue play, or the path never reachable.

### P3 — lemming effect hypothesis

If Witch is a SpeedPad-like gadget, there is a `witch*.a3d` (or another
named object) whose overlap attaches a parallel state / kills / transforms
a lemming, and/or an exclusive cell bit on Witch levels.

Falsified by: no witch a3d; play site not on an object/overlap path;
no actor-list side effect from the cue path.

### P4 — what the play site actually gates (hypothesis)

The sole play load sits in the in-level UI loop (`FUN_0045cff9` case 0)
beside the normal pause path (`s_pause_004c0be8`). Hypothesis: on a
specific level path string it plays Witch instead of entering pause.

Falsified if the strcmp partner is not a level path, or the branch also
attaches a lemming state.

---

## Static reads

| claim | address / bytes |
| --- | --- |
| cue `Witch` | `0x004C0D34` (`…Fire\0\0\0\0Witch\0\0\0Bits…`); sole string imm xref `push 0x4c0d34` at `0x0045E720` (imm32 at `0x0045E721`) |
| register | `FUN_0045e51f`: after Fire store `+0x1c8fc`, `FUN_0041d7d0(..., s_Witch)` → `*(param + 0x1c8a8)` at disp site `0x0045E734` (`a8 c8 01 00`) |
| cue object base | `0x004C6A08`; Witch global `DAT_004E32B0` |
| preload | disp `0x0045EB57` (`a8 c8 01 00`) |
| unload | disp `0x0045F19A` (`a8 c8 01 00`) |
| `+0x1c8a8` inventory | only those three sites |
| play load | **one** absolute `.text` ref: `0x0045D200` `8b 0d b0 32 4e 00` (`mov ecx,[0x4e32b0]`) |
| play call | `0x0045D20C` `e8 0a 80 fe ff` → `FUN_0044521b` (Action also calls this helper; Action cue load contrast `mov eax,[0x4e32fc]` at `0x004556AA`) |
| containing fn | `FUN_0045cff9` (in-level frame / input loop), switch `param_1[0x15cb]` case 0 |
| pause key | `FUN_0048373d` → `FUN_0048348f(0x13)` (binding index `0x13`; same gate as the normal pause arm) |
| easter-egg gate | `__strcmpi(&DAT_004cbbf8, s_compiled_e10_kf_1_lvl_004c0bd0)` at pushes `0x0045D1E8` / `0x0045D1ED`; string `compiled\e10_kf_1.lvl` at `0x004C0BD0` |
| on match | `FUN_0044521b(DAT_004E32B0, '\0')` only — **no** pause UI (`FUN_00498d9a` / `s_pause` / overlay setup is the `else`) |
| on mismatch | normal pause path |
| Balloon / Fire contrast | those globals have **0** absolute play refs; Witch has **1** |
| SpeedPad / Action contrast | gadget / HUD play paths; Witch play is UI-loop + level-name gate, not object AABB |

P1 holds. P2 holds (play site real). P3’s gadget hypothesis is **falsified**.
P4 holds: level-path easter egg; no lemming attach on that arm.

---

## Offline audit

`--audit` on stock exe + `lemmings.box` / `patch.box`:

| check | result |
| --- | --- |
| `witch*.a3d` in boxes | **none** (0 a3d files reference `TEXTURES\MODELS\WITCH\*`) |
| `SOUNDS\WITCH.WAV` | present (played only via the e10 pause easter egg) |
| orphan textures | `TEXTURES\MODELS\WITCH\{BROOM,GHOSTEYE,WITCHFROCK,WITCHGOB,WITCHGOB2}.BMP` — no retail a3d consumer |
| `COMPILED\E10_KF_1.LVL` | present (the strcmp target’s level); easter-egg host only |
| COMPILED `.LVL` substring `Witch` | cue-name table in ~101 levels (same embedded table pattern as Balloon/Fire) — not a gadget placement |
| exclusive cell bit for a Witch gadget | N/A — no candidate gadget object |

---

## Live series

Not required for the lemming question. Settled by shipped-artifact reads:
the only play site is the UI pause branch, gated on the loaded level path
string, with no actor-list / overlap / cell-bit path. No process launched;
no live writes.

---

## Settled

**What to a lemming:** nothing in retail. No object AABB, no cell bit, no
parallel state, no `witch.a3d`.

**How the cue is used:** register / preload / unload with the SFX table;
played once when the pause binding (`FUN_0048373d` / index `0x13`) fires
while `DAT_004cbbf8` equals `compiled\e10_kf_1.lvl`, via
`FUN_0044521b(DAT_004E32B0, 0)`. On that level the normal pause UI arm is
skipped (quirk). Elsewhere, pause behaves normally and Witch is not played.

Sim: no Witch lemming mechanic (`kWitchGadgetInRetail = false` in
`World.h`). Harness: `measure_witch.py --audit`. Test:
`testWitchCueNoLemmingsGadget` (mutate-alone: flip the flag to `true`).

Process: none. Live writes: none.
