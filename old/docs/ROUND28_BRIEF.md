# Round 28 brief — Die (Round 9 gadget repeat)

This file is a **self-contained prompt**. Paste everything below the rule into a
fresh session. A facilitator owns `PLAN.md` §1.6, `docs/PHASE3_HANDOFF.md`, and
`.claude/rules/` — do **not** edit those; report proposed ledger text instead.

Written 2026-10-16, after rounds 1–27 closed. Tree green at 16/16. Queue after
this: **empty** (last Round-9 cue repeat). **One mechanic: Die only.**

---

You are picking up Phase 3 Round 28 of Lemmings Webolution at
`C:\progs\lemmings-webolution`, a 1:1 reimplementation of Lemmings Revolution.
**The stock 640x480 executable is the authority.**

## Facilitator ownership

**Must not touch:** `PLAN.md`, `docs/PHASE3_HANDOFF.md`, `.claude/rules/*`.
Park prose in `docs/scratch/phase3-round28-die.md`. Propose a `PLAN.md`
§1.6 bullet in your final message.

**You own:** scratch note, sim/asset/harness/tests in scope.

**One agent owns the running game — you.** Restore every live write.

## Read first

| File | What it gives you |
| --- | --- |
| `.claude/rules/match-the-original.md` | evidence standard |
| `docs/scratch/phase3-round24-bubble.md` | fluid type `0x23` plays `Die` on enter |
| `docs/scratch/phase3-round3b-float-splat.md` / splat notes | `Splat` / `FallSplat` contrast |
| `PLAN.md` §1.6 | ledger (read-only) |

## Your question — one mechanic

**What does Die do (vs Splat / FallSplat), and when is the cue played?**

Settle that and stop. This is the last Round-9 cue repeat.

### Leads (observations, not conclusions)

- Cue string **`Die`** at **`0x004C0CD0`**. Register `push 0x4c0cd0` at
  **`0x0045E618`**; store to **`+0x1c8d4`** at **`0x0045E62A`** (disp site
  **`0x0045E62C`**).
- Global `DAT_004E32DC` (`0x004C6A08+0x1c8d4`). Slot inventory (displacement
  sites only): register **`0x0045E62C`**, preload **`0x0045EA4F`**, unload
  **`0x0045F0BE`**.
- Absolute `.text` ref (facilitator inventory): **`0x004042CF`** — Round 24
  already saw fluid subtype `1` enter play `Die` via `FUN_0044535b`. Confirm
  whether that is the **sole** play load, and inventory any other callers.
- Distinct from `Splat` (`0x004C0CC0`) and `FallSplat` (`0x004C0CD4`) — pin how
  Die differs (SFX-only on an already-measured death, a separate death path,
  or unused extras). No requirement for `die.a3d`.

## Evidence standard

Same as prior rounds. Actor list `0x004CBB18`. No `PIXELS_PER_UNIT`.
Mutate-alone + touch + suite 16/16. Windows build via
`scripts/Invoke-Ucrt64.ps1`. Do **not** put UCRT64 `bin` on PATH for
`tools/harness/*.py`.

## Settled — do not redo

Prior gadgets through Changeop (27). Do not re-open Bubble/water/acid death
mechanics unless Die evidence forces a correction — then stop and report.

## Deliverables

1. `docs/scratch/phase3-round28-die.md`
2. Sim only if new mechanic; else flag (`kDieGadgetInRetail=false` or
   equivalent)
3. Tests + suite 16/16
4. Propose §1.6 bullet only
5. Process / restores clean

**Report honestly.**
