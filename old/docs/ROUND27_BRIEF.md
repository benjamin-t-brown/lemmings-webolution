# Round 27 brief — Changeop (Round 9 gadget repeat)

This file is a **self-contained prompt**. Paste everything below the rule into a
fresh session. A facilitator owns `PLAN.md` §1.6, `docs/PHASE3_HANDOFF.md`, and
`.claude/rules/` — do **not** edit those; report proposed ledger text instead.

Written 2026-10-16, after rounds 1–26 closed. Tree green at 16/16. Queue after
this: Die. **One mechanic: Changeop only.**

---

You are picking up Phase 3 Round 27 of Lemmings Webolution at
`C:\progs\lemmings-webolution`, a 1:1 reimplementation of Lemmings Revolution.
**The stock 640x480 executable is the authority.**

## Facilitator ownership

**Must not touch:** `PLAN.md`, `docs/PHASE3_HANDOFF.md`, `.claude/rules/*`.
Park prose in `docs/scratch/phase3-round27-changeop.md`. Propose a `PLAN.md`
§1.6 bullet in your final message.

**You own:** scratch note, sim/asset/harness/tests in scope.

**One agent owns the running game — you.** Restore every live write.

## Read first

| File | What it gives you |
| --- | --- |
| `.claude/rules/match-the-original.md` | evidence standard |
| `docs/scratch/phase3-round26-rumble.md` | object state-commit SFX pattern |
| `docs/scratch/phase3-round22-forcef.md` | unused-cue inventory pattern |
| `PLAN.md` §1.6 | ledger (read-only) |

## Your question — one mechanic

**What does Changeop do, and when is the cue played?**

Settle that and stop. Do **not** take Die.

### Leads (observations, not conclusions)

- Cue string **`Changeop`** at **`0x004C0CB4`**. Register `push 0x4c0cb4` at
  **`0x0045E5D0`**; store to **`+0x1c8c8`** at **`0x0045E5E2`** (disp site
  **`0x0045E5E4`**).
- Global `DAT_004E32D0` (`0x004C6A08+0x1c8c8`). Slot inventory (displacement
  sites only): register **`0x0045E5E4`**, preload **`0x0045EA0D`**, unload
  **`0x0045F082`**.
- Absolute `.text` refs to the global: **`0x0045AD07`** (`mov edx,[DAT_004E32D0]`)
  and **`0x0048EB0C`** (`mov eax,[DAT_004E32D0]`) — play candidates. Prove
  callers and whether a lemming / object / UI path is affected.
- Name suggests operator / skill change — may be SFX only, unused, or a real
  gadget. Census `changeop*.a3d` yourself; no requirement that one exists.

## Evidence standard

Same as prior rounds. Actor list `0x004CBB18`. No `PIXELS_PER_UNIT`.
Mutate-alone + touch + suite 16/16. Windows build via
`scripts/Invoke-Ucrt64.ps1`. Do **not** put UCRT64 `bin` on PATH for
`tools/harness/*.py`.

## Settled — do not redo

Prior gadgets through Rumble (26). Do not move settled skill / bomber
constants unless evidence forces it.

## Deliverables

1. `docs/scratch/phase3-round27-changeop.md`
2. Sim only if new mechanic; else flag (`kChangeopGadgetInRetail=false` or
   equivalent)
3. Tests + suite 16/16
4. Propose §1.6 bullet only
5. Process / restores clean

**Report honestly.**
