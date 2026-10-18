# Round 25 brief — Scrape (Round 9 gadget repeat)

This file is a **self-contained prompt**. Paste everything below the rule into a
fresh session. A facilitator owns `PLAN.md` §1.6, `docs/PHASE3_HANDOFF.md`, and
`.claude/rules/` — do **not** edit those; report proposed ledger text instead.

Written 2026-10-16, after rounds 1–24 closed. Tree green at 16/16. Queue after
this: Rumble, Changeop, Die. **One mechanic: Scrape only.**

---

You are picking up Phase 3 Round 25 of Lemmings Webolution at
`C:\progs\lemmings-webolution`, a 1:1 reimplementation of Lemmings Revolution.
**The stock 640x480 executable is the authority.**

## Facilitator ownership

**Must not touch:** `PLAN.md`, `docs/PHASE3_HANDOFF.md`, `.claude/rules/*`.
Park prose in `docs/scratch/phase3-round25-scrape.md`. Propose a `PLAN.md`
§1.6 bullet in your final message.

**You own:** scratch note, sim/asset/harness/tests in scope.

**One agent owns the running game — you.** Restore every live write.

## Read first

| File | What it gives you |
| --- | --- |
| `.claude/rules/match-the-original.md` | evidence standard |
| `docs/scratch/phase3-round24-bubble.md` | SFX-on-state pattern |
| `docs/scratch/phase3-round5*.md` / bash dig mine notes | terrain scrape candidates |
| `PLAN.md` §1.6 | ledger (read-only) |

## Your question — one mechanic

**What does Scrape do (to a lemming / terrain), and when is the cue played?**

Settle that and stop. Do **not** take Rumble, Changeop, or Die.

### Leads (observations, not conclusions)

- Cue string **`Scrape`** at **`0x004C0CA4`**. Register `push 0x4c0ca4` at
  **`0x0045E5A0`**; store to **`+0x1c8c0`** at **`0x0045E5B2`**.
- Global `DAT_004E32C8` (`0x004C6A08+0x1c8c0`) has abs refs at
  **`0x0040C139`** and **`0x00415B33`** — play-shaped. Re-verify callers;
  one sits near dig/bash terrain work (`0x00415B*`).
- May be SFX for basher/digger/miner removal (already measured skills) rather
  than a new gadget — if so, pin that and stop (no new sim unless a gap).
- No requirement for a `scrape.a3d`; census yourself.

## Evidence standard

Same as prior rounds. Actor list `0x004CBB18`. No `PIXELS_PER_UNIT`.
Mutate-alone + touch + suite 16/16.

## Settled — do not redo

Prior gadgets through Bubble (24). Do not change settled bash/dig/mine
constants unless evidence forces it — stop and report if so.

## Deliverables

1. `docs/scratch/phase3-round25-scrape.md`
2. Sim only if a new mechanic; else clean flag / comment
3. Tests + suite 16/16
4. Propose §1.6 bullet only
5. Process / restores clean

**Report honestly.**
