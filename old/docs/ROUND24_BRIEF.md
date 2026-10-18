# Round 24 brief — Bubble (Round 9 gadget repeat)

This file is a **self-contained prompt**. Paste everything below the rule into a
fresh session. A facilitator owns `PLAN.md` §1.6, `docs/PHASE3_HANDOFF.md`, and
`.claude/rules/` — do **not** edit those; report proposed ledger text instead.

Written 2026-10-16, after rounds 1–23 closed. Tree green at 16/16. Queue after
this: Scrape, Rumble, Changeop, Die. **One mechanic: Bubble only.**

---

You are picking up Phase 3 Round 24 of Lemmings Webolution at
`C:\progs\lemmings-webolution`, a 1:1 reimplementation of Lemmings Revolution.
**The stock 640x480 executable is the authority.**

## Facilitator ownership

**Must not touch:** `PLAN.md`, `docs/PHASE3_HANDOFF.md`, `.claude/rules/*`.
Park prose in `docs/scratch/phase3-round24-bubble.md`. Propose a `PLAN.md`
§1.6 bullet in your final message.

**You own:** scratch note, sim/asset/harness/tests in scope.

**One agent owns the running game — you.** Restore every live write.

## Read first

| File | What it gives you |
| --- | --- |
| `.claude/rules/match-the-original.md` | evidence standard |
| `docs/scratch/phase3-round17-speedpad.md` | cue play on activate |
| `docs/scratch/phase3-round23-axe.md` | FX-name table vs gadget |
| `PLAN.md` §1.6 | ledger (read-only) |

## Your question — one mechanic

**What does Bubble do to a lemming, and how is it detected?**

Settle that and stop. Do **not** take Scrape, Rumble, Changeop, or Die.

### Leads (observations, not conclusions)

- Cue string **`Bubble`** at **`0x004C0C9C`**. Register `push 0x4c0c9c` at
  **`0x0045E588`**; store to **`+0x1c8bc`** at **`0x0045E59A`**.
- Global `DAT_004E32C4` (`0x004C6A08+0x1c8bc`) has absolute refs at
  **`0x00404697`** and **`0x004046C4`** — play-shaped (lookup then
  `call 0x00406A00` / similar). **Not** Balloon/Fire-shaped. Re-verify and
  walk callers.
- **No `bubble.a3d`** in COMPILED (facilitator). May be FX/SFX on drown,
  hatch, or another event — prove the caller and whether a lemming state
  changes.
- Neighbour cues in the table: Scrape (`0x004C0CA4`), Ohno — out of scope.

## Evidence standard

Same as prior rounds. Actor list `0x004CBB18`. No `PIXELS_PER_UNIT`.
Mutate-alone + touch + suite 16/16.

## Settled — do not redo

Prior gadgets through Axe (round 23). Do not move settled fields.

## Deliverables

1. `docs/scratch/phase3-round24-bubble.md`
2. Sim changes or clean-negative-for-lemmings flag
3. Tests + suite 16/16
4. Propose §1.6 bullet only
5. Process / restores clean

**Report honestly.**
