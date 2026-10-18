# Round 26 brief — Rumble (Round 9 gadget repeat)

This file is a **self-contained prompt**. Paste everything below the rule into a
fresh session. A facilitator owns `PLAN.md` §1.6, `docs/PHASE3_HANDOFF.md`, and
`.claude/rules/` — do **not** edit those; report proposed ledger text instead.

Written 2026-10-16, after rounds 1–25 closed. Tree green at 16/16. Queue after
this: Changeop, Die. **One mechanic: Rumble only.**

---

You are picking up Phase 3 Round 26 of Lemmings Webolution at
`C:\progs\lemmings-webolution`, a 1:1 reimplementation of Lemmings Revolution.
**The stock 640x480 executable is the authority.**

## Facilitator ownership

**Must not touch:** `PLAN.md`, `docs/PHASE3_HANDOFF.md`, `.claude/rules/*`.
Park prose in `docs/scratch/phase3-round26-rumble.md`. Propose a `PLAN.md`
§1.6 bullet in your final message.

**You own:** scratch note, sim/asset/harness/tests in scope.

**One agent owns the running game — you.** Restore every live write.

## Read first

| File | What it gives you |
| --- | --- |
| `.claude/rules/match-the-original.md` | evidence standard |
| `docs/scratch/phase3-round25-scrape.md` | skill-SFX pattern |
| `docs/scratch/phase3-round7*.md` | bomber / explode cues |
| `PLAN.md` §1.6 | ledger (read-only) |

## Your question — one mechanic

**What does Rumble do, and when is the cue played?**

Settle that and stop. Do **not** take Changeop or Die.

### Leads (observations, not conclusions)

- Cue string **`Rumble`** at **`0x004C0CFC`**. Register `push 0x4c0cfc` at
  **`0x0045E690`**; store to **`+0x1c8e8`** at **`0x0045E6A2`**.
- Global `DAT_004E32F0` (`0x004C6A08+0x1c8e8`) has abs ref **`0x00423823`**
  (play candidate). Extra `+0x1c8e8` sites `0x45ead3` / `0x45f122` /
  `0x46dfc3` / `0x489453` — inventory (preload/unload vs other uses).
- May be camera shake / explosion aftermath / UI — prove callers and whether
  a lemming is affected. No requirement for `rumble.a3d`.

## Evidence standard

Same as prior rounds. Actor list `0x004CBB18`. No `PIXELS_PER_UNIT`.
Mutate-alone + touch + suite 16/16.

## Settled — do not redo

Prior gadgets through Scrape (25). Do not move settled bomber constants
unless evidence forces it.

## Deliverables

1. `docs/scratch/phase3-round26-rumble.md`
2. Sim only if new mechanic; else flag
3. Tests + suite 16/16
4. Propose §1.6 bullet only
5. Process / restores clean

**Report honestly.**
