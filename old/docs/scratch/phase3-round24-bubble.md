# Phase 3 round 24 — Bubble

2026-10-16. Stock `C:\progs\Lemmings Revolution\Lemmings Revolution.exe`,
SHA-256 `32675bdb49866e76502713251fdfd2b09078c2481c72ca22ff68ebf4b3737a2f`,
image base `0x400000`.

Harness: `tools/harness/measure_bubble.py` (new). Re-run:

```
python tools/harness/measure_bubble.py --audit
```

Facilitator owns `PLAN.md` / handoff / rules — this scratch only.

**Result: no Bubble world gadget.** Cue `Bubble` is a real SFX play (not
Balloon/Fire-shaped). The sole play path is type-`0x23` state step
`FUN_00404372` (vtable `0x004B03C0`). That state is a fluid/zone mismatch
death path on the lemming: enter sets anim `"drown"`, step plays Bubble
once and may spawn particle FX named `Bubbles`/`bubbles`, and fluid
subtype `1` also plays cue `Die` on enter. No `bubble.a3d`. Detection is
not Bubble-specific — attach when walker `FUN_0041bffc` sees
`lem+0x54` (fluid object) with a sheet/fluid-type mismatch.

---

## PREDICTIONS (written before the static audit)

### P1 — slot matches brief

`push 0x4c0c9c` at `0x0045E588`; store to `+0x1c8bc` (global
`0x004C6A08 + 0x1c8bc = DAT_004E32C4`). Three imm32 sites
`0x45e59c` / `0x45e9cb` / `0x45f046` are register / preload / unload.

Falsified if the store after the Bubble push writes a different
displacement.

### P2 — play sites are type-0x23 step, not a Bubble gadget

Abs refs at `0x00404697` / `0x004046C4` are inside `FUN_00404372`
(vtable `0x004B03C0` +4). Path: lookup `FUN_00404cc0` → channel
`FUN_00406a00` → SFX `FUN_0044535b`, gated on height delta `> 5.0` and
one-shot `this+0x1a == 0`. No `bubble.a3d`.

Falsified if those loads are outside that function, or if a Bubble a3d /
exclusive cell bit / object AABB path exists.

### P3 — lemming is affected by the caller state, not by Bubble-as-gadget

Ctor `FUN_004041fd` sets type `0x23`, `+0x11=0` (main state). Sole attach
`call` at `0x0041C3BA` from walker `FUN_0041bffc` when `lem+0x54 != 0`
and sheet (`+0x4c`) / fluid (`+0x128`) mismatch (or fluid subtype `1`).
Enter `FUN_00404261` sets anim `"drown"`; subtype `1` plays `Die`
(`DAT_004E32DC`). Distinct from water-hazard drown type `0x22`
(`0x004B0570`, round 14).

Falsified if play sites never touch a lemming state, or if type `0x23`
is identical to type `0x22`.

### P4 — FX name `Bubbles` is not the SFX cue

Strings `bubbles` / `Bubbles` at `0x004BF120` / `0x004BF128` feed the
particle helper `FUN_00405000`, not cue register `FUN_0041d7d0`.
`SPRITES\BUBBLES.SPR` is that sheet.

Falsified if those strings share the cue global `DAT_004E32C4`.

---

## Static reads

| claim | address / bytes |
| --- | --- |
| cue `Bubble` | `0x004C0C9C` (`Explode\0Bubble\0\0Scrape…`); sole string imm `push` at `0x0045E588` (imm32 at `0x0045E589`) |
| register | `FUN_0045e51f`: `FUN_0041d7d0(..., s_Bubble)` → `*(+0x1c8bc)` store disp `0x0045E59C` |
| cue object base | `0x004C6A08`; Bubble global `DAT_004E32C4` |
| preload / unload | disp `0x0045E9CB` / `0x0045F046` |
| abs play loads | `0x00404697` `mov ecx,[DAT_004E32C4]`; `0x004046C4` `mov edx,[DAT_004E32C4]` — **only** two `.text` imm32 hits |
| play arm | `0x004046A3` `call FUN_00404cc0`; `0x004046CE` `call FUN_00406a00`; `0x004046DC` `call FUN_0044535b`; then `this+0x1a=1` |
| containing step | `FUN_00404372` (`0x00404372`..`0x00404956`); vtable slot `0x004B03C4` |
| vtable | `0x004B03C0`: dtor `0x4049b0` / step `0x404372` / enter `0x404261` / … |
| ctor | `FUN_004041fd`: `mov [eax],0x4b03c0`; `mov [ecx+0x14],0x23` at `0x00404218` (imm `0x0040421B`); `+0x11=0` |
| sole attach | `call FUN_004041fd` at `0x0041C3BA` inside walker `FUN_0041bffc` |
| attach gate | `lem+0x54` fluid ptr; mismatch vs `lem+0x4c` / `fluid+0x128` (or subtype `1`) |
| enter | `FUN_00404261`: push `s_drown_004bf118`; reset `+0x1a`; subtype `0` particle FX; subtype `1` → `DAT_004E32DC` (`Die`) via `FUN_0044535b` |
| drown contrast | type `0x22` / `0x004B0570` / ctor `0x00408700` (round 14 cell bit `0x40`) — different vtable |
| actor list | **no** `0x004CBB18` imm inside `FUN_00404372` (SFX + particles only on this path) |
| FX names | `bubbles` `0x004BF120` / `Bubbles` `0x004BF128` used by `FUN_00405000` — not cue `0x004C0C9C` |

P1–P4 hold.

---

## Offline audit

`--audit` on stock exe + `lemmings.box` / `patch.box`:

| check | result |
| --- | --- |
| `bubble*.a3d` in boxes | **none** |
| `SOUNDS\BUBBLE.WAV` | present (played via cue) |
| `SPRITES\BUBBLES.SPR` + `TEXTURES\SHEETS\_BUBBLES0.BMP` | present (FX sheet for particle path) |
| exclusive Bubble cell bit / a3d placement | N/A — no Bubble gadget object |

---

## Live series

Not required for the Bubble-cue question. Settled by shipped-artifact
reads: the only absolute uses of `DAT_004E32C4` are the two loads inside
type-`0x23` step; attach is the sole ctor call from the walker fluid
branch. No process launched; no live writes. (Whether type `0x23`
fires on a given retail level still depends on `lem+0x54` being set —
out of scope for "what does the Bubble cue do".)

---

## Settled

**What to a lemming:** the cue itself is **SFX only** (and optional
particle FX via the sibling `Bubbles` sheet). There is **no** Bubble
world gadget. The **caller** is main state type **`0x23`** (vtable
`0x004B03C0`), which sets anim `"drown"`, plays Bubble once when the
height delta vs the fluid object exceeds `5.0`, may emit bubble
particles in the `5..10` band, plays `Die` on enter when fluid
`+0x128==1`, and eventually returns `2` from step (leave). Distinct
from water-hazard drown type `0x22`.

**How detected:** not as Bubble. Attach is walker `FUN_0041bffc` when
`lem+0x54` names a fluid/zone object whose subtype mismatches the
lemming sheet (`+0x4c`), or subtype is `1`. Cue play is gated inside
that state's step (height + one-shot `+0x1a`).

Sim: no Bubble gadget (`kBubbleGadgetInRetail = false` in `World.h`).
Type-`0x23` fluid-mismatch death is documented, not newly simulated
this round (water drown `0x22` already covers the `0x40` path).
Harness: `measure_bubble.py --audit`. Test: `testBubbleCueNoGadget`
(mutate-alone: flip the flag to `true`).

Process: none. Live writes: none.
