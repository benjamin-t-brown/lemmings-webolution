# Phase 3 round 25 — Scrape

2026-10-16. Stock `C:\progs\Lemmings Revolution\Lemmings Revolution.exe`,
SHA-256 `32675bdb49866e76502713251fdfd2b09078c2481c72ca22ff68ebf4b3737a2f`,
image base `0x400000`.

Harness: `tools/harness/measure_scrape.py` (new). Re-run:

```
python tools/harness/measure_scrape.py --audit
```

Facilitator owns `PLAN.md` / handoff / rules — this scratch only.

**Result: no Scrape world gadget.** Cue `Scrape` is real SFX (not
Balloon/ForceF-shaped). The only absolute loads of `DAT_004E32C8` are
two refusal arms already measured in round 5: dig-bite
`FUN_00415a43` and bash cell helper `FUN_0040c0af` (called from bash
bite `FUN_0040c357`). Mine refuse plays sibling cue `MineMetal`
(`DAT_004E32EC`), not Scrape. No `scrape.a3d`. Cue does not mutate
terrain or attach a new lemming state — callers already refuse the
cell / return to walk.

---

## PREDICTIONS (written before the static audit)

### P1 — slot matches brief

`push 0x4c0ca4` at `0x0045E5A0`; store to `+0x1c8c0` (global
`0x004C6A08 + 0x1c8c0 = DAT_004E32C8`). Three imm32 sites for the
slot are register / preload / unload.

Falsified if the store after the Scrape push writes a different
displacement.

### P2 — only two play loads; dig + bash refuse

Abs refs at `0x0040C139` / `0x00415B33` are the only `.text` imm32
hits to `DAT_004E32C8`. First is inside bash helper `FUN_0040c0af`;
second inside dig-bite `FUN_00415a43`. Both sit on steel / direction-
bit / blocker refuse arms (round 5), not on successful cell clears
(those play `Mine` / `DAT_004E32E8`).

Falsified if additional absolute refs exist, or if either load is on
a success / gadget path.

### P3 — mine refuse is MineMetal, not Scrape

Mine bite `FUN_004188f6` loads `DAT_004E32EC` (`MineMetal`) on its
steel refuse arm. Scrape is not a shared three-skill cue.

Falsified if mine loads `DAT_004E32C8`.

### P4 — no scrape.a3d / no exclusive gadget

No `scrape*.a3d` in `lemmings.box` / `patch.box`. `SOUNDS\SCRAPE.WAV`
exists (played via the cue). Actor list `0x004CBB18` has no imm in
either play function body.

Falsified if an a3d placement or exclusive cell-bit / object AABB path
is Scrape-named.

---

## Static reads

| claim | address / bytes |
| --- | --- |
| cue `Scrape` | `0x004C0CA4` (`Bubble\0\0Scrape\0\0Ohno…`); sole string imm `push` at `0x0045E5A0` (imm32 at `0x0045E5A1`) |
| register | `FUN_0045e51f`: `FUN_0041d7d0(..., s_Scrape)` → `*(+0x1c8c0)` store disp `0x0045E5B4` |
| cue object base | `0x004C6A08`; Scrape global `DAT_004E32C8` |
| preload / unload | disp `0x0045E9E1` / `0x0045F05A` |
| abs play loads | `0x0040C139` `mov eax,[DAT_004E32C8]`; `0x00415B33` same — **only** two `.text` imm32 hits (imm at +1) |
| dig refuse arm | `FUN_00415a43` (dig-bite record fn): on bit `0x2` / dir steel `0x10`/`0x20` → channel `FUN_00406a00` → SFX `FUN_0044535b(Scrape)` → walk `FUN_0042f8b9` |
| dig success contrast | same fn plays `DAT_004E32E8` (`Mine`) after cell zero at `0x00415B84` |
| bash refuse arm | `FUN_0040c0af`: on bit `0x2` / dir `0x4`/`0x8` / `0x800` → `FUN_00445274(..., Scrape)` return 1 |
| bash caller | bite `FUN_0040c357` probes via `FUN_0040de60` then calls `FUN_0040c0af` (round 5) |
| bash success contrast | clear path plays `DAT_004E32E8` (`Mine`) at `0x0040C317` |
| mine refuse contrast | `FUN_004188f6` loads `DAT_004E32EC` (`MineMetal`) — **not** Scrape |
| actor list | **no** `0x004CBB18` imm inside `FUN_00415a43` or `FUN_0040c0af` |

P1–P4 hold.

---

## Offline audit

`--audit` on stock exe + `lemmings.box` / `patch.box`:

| check | result |
| --- | --- |
| `scrape*.a3d` in boxes | **none** |
| `SOUNDS\SCRAPE.WAV` | present (played via cue) |
| exclusive Scrape cell bit / a3d placement | N/A — no Scrape gadget object |

---

## Live series

Not required for the Scrape-cue question. Round 5 already measured dig
refuse live (zero cells changed, stock spent, walk restore) and noted
the clang at `0x00415B33` / `DAT_004E32C8`. This round pins that global
as cue `Scrape` and the second play site as bash helper refuse. No
process launched; no live writes.

---

## Settled

**What to a lemming / terrain:** the cue itself is **SFX only**. There
is **no** Scrape world gadget. Dig/bash refuse logic (already modelled)
skips the cell zero and may return the lemming to walk; Scrape is the
audible clang on those arms. Mine refuse uses `MineMetal` instead.

**When played:** (1) dig-bite `FUN_00415a43` when the probed cell has
indestructible bit `0x2` or direction steel `0x10`/`0x20`; (2) bash
cell helper `FUN_0040c0af` when the cell has `0x2`, direction steel
`0x4`/`0x8`, or blocker `0x800`.

Sim: no Scrape gadget (`kScrapeGadgetInRetail = false` in `World.h`).
No new dig/bash/mine constants. Harness: `measure_scrape.py --audit`.
Test: `testScrapeCueNoGadget` (mutate-alone: flip the flag to `true`).

Process: none. Live writes: none.
