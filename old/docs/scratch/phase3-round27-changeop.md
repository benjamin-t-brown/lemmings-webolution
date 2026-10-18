# Phase 3 round 27 — Changeop

2026-10-16. Stock `C:\progs\Lemmings Revolution\Lemmings Revolution.exe`,
SHA-256 `32675bdb49866e76502713251fdfd2b09078c2481c72ca22ff68ebf4b3737a2f`,
image base `0x400000`.

Harness: `tools/harness/measure_changeop.py` (new). Re-run:

```
python tools/harness/measure_changeop.py --audit
```

Facilitator owns `PLAN.md` / handoff / rules — this scratch only.

**Result: no Changeop world gadget.** Cue `Changeop` is real UI SFX
(not Balloon/ForceF-shaped). Two absolute loads of `DAT_004E32D0` play
it: skill-panel select `FUN_0045ac5c` (via `FUN_0044521b`) and options
SFX-volume slider `FUN_0048e8d5` (vtable `0x004B24D4`, via
`FUN_00404cc0` → `FUN_00497f39`). Neither path touches the actor list.
No `changeop.a3d`. `SOUNDS\CHANGEOP.WAV` (+ orphan `OLD-CHANGEOP.WAV`)
exist.

---

## PREDICTIONS (written before the static audit)

### P1 — slot matches brief

`push 0x4c0cb4` at `0x0045E5D0`; store to `+0x1c8c8` (global
`0x004C6A08 + 0x1c8c8 = DAT_004E32D0`). Displacement sites only
register / preload / unload (`0x45e5e4` / `0x45ea0d` / `0x45f082`).

Falsified if the store after the Changeop push writes a different
displacement, or extra real `+0x1c8c8` sites exist.

### P2 — both abs refs are play paths

`0x0045AD07` and `0x0048EB0C` load `DAT_004E32D0` into play helpers
(`FUN_0044521b` / `FUN_00404cc0`→`FUN_00497f39`), not register/preload.

Falsified if either load is not a play arm.

### P3 — UI / skill-panel, not a lemming gadget

Name "Changeop" = change operator. Play sites sit in skill-panel select
and an options slider; no `changeop.a3d`; no actor-list imm in either
play function.

Falsified if an a3d placement, exclusive cell bit, or lemming-attach
path is Changeop-named.

---

## Static reads

| claim | address / bytes |
| --- | --- |
| cue `Changeop` | `0x004C0CB4` (`…Ohno\0\0\0\0Changeop\0\0\0\0Splat…`); sole string imm `push` at `0x0045E5D0` (imm32 at `0x0045E5D1`) |
| register | `FUN_0045e51f`: `FUN_0041d7d0(..., s_Changeop)` → `*(+0x1c8c8)` store disp `0x0045E5E4` (`89 82 c8 c8 01 00`) |
| cue object base | `0x004C6A08`; Changeop global `DAT_004E32D0` |
| preload / unload | disp `0x0045EA0D` (`8b 91 …`) / `0x0045F082` (`8b 82 …`) |
| `+0x1c8c8` inventory | **only** those three sites |
| abs play loads | **exactly two**: `0x0045AD07` (`mov edx,[DAT_004E32D0]`) and `0x0048EB0C` (`mov eax,[DAT_004E32D0]`) |
| play A — skill panel | insn `0x0045AD05` inside `FUN_0045ac5c`; after `this+0x28 = param_1` and skill-count `[this+index*4] ≠ 0`, gate `cmp dword [0x004C6A08], 2` / `jnz` skip; then `push 0` / push cue / `mov ecx,0x4c6a08` / `call FUN_0044521b` at `0x0045AD11` |
| play A callers | `FUN_00455aff` (panel click → `FUN_0045ac5c(&DAT_004c91d8, index)` when index ≠ 8); `FUN_00456f5c` case 3 (replay message); also `0x45a5fb` / `0x45ac0e` / `0x45ae10` |
| play A object | skill-panel UI `DAT_004C91D8` (`+0x28` = selected index); **not** a lemming |
| play B — options slider | insn `0x0048EB0B` inside `FUN_0048e8d5`; vtable slot `0x004B24D4`; when event target equals `this+0x44`, set float via `FUN_004989f9(&DAT_0058cb88, …)` then `FUN_00404cc0(&DAT_0058cb90, Changeop)` → `FUN_00497f39` |
| play B sibling | `this+0x40` adjusts via `FUN_00498993` **without** Changeop (music-shaped contrast) |
| actor list | **no** `0x004CBB18` imm in `FUN_0045ac5c` or `FUN_0048e8d5` |
| gate note | sole `.text` `cmp dword [0x004C6A08], imm8` is the skill-panel gate (`== 2`). No absolute `mov [0x004C6A08], 2` found; meaning of value `2` is **not** settled beyond the compare bytes. Slider path has no such gate. |

P1–P3 hold.

---

## Offline audit

`--audit` on stock exe + `lemmings.box` / `patch.box`:

| check | result |
| --- | --- |
| `changeop*.a3d` in boxes / COMPILED `.LVL` | **none** |
| `SOUNDS\CHANGEOP.WAV` | present (played via the cue) |
| `SOUNDS\OLD-CHANGEOP.WAV` | present; orphan sibling name (no `Old-Changeop` cue string) |
| exclusive Changeop cell bit / a3d placement | N/A — no Changeop gadget object |

---

## Live series

Not required for the Changeop-cue question. Both play paths and
register/preload/unload inventory are static. No process launched;
no live writes.

---

## Settled

**What to a lemming:** nothing. There is **no** Changeop world gadget /
no lemming attach. The cue is UI SFX.

**When played:**

1. Skill-panel select `FUN_0045ac5c` when the chosen skill count is
   nonzero and `dword[0x004C6A08] == 2` → `FUN_0044521b(Changeop, 0)`.
2. Options control `FUN_0048e8d5` when adjusting the `this+0x44` slider
   (SFX volume preview) → channel lookup → `FUN_00497f39`.

Sim: no Changeop gadget (`kChangeopGadgetInRetail = false` in `World.h`).
Harness: `measure_changeop.py --audit`.
Test: `testChangeopCueNoGadget` (mutate-alone: flip the flag to `true`).

Process: none. Live writes: none.

Mutate-alone `kChangeopGadgetInRetail=true` → FAIL; restored + touch →
suite **16/16**. `--audit` AUDIT PASS.
