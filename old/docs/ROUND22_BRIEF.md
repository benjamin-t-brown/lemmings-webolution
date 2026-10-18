# Round 22 brief — ForceF (Round 9 gadget repeat)

This file is a **self-contained prompt**. Paste everything below the rule into a
fresh session. A facilitator owns `PLAN.md` §1.6, `docs/PHASE3_HANDOFF.md`, and
`.claude/rules/` — do **not** edit those; report proposed ledger text instead.

Written 2026-10-16, after rounds 1–21 closed. Tree green at 16/16. Queue after
this: Axe, Bubble, Scrape, Rumble, Changeop, Die. **One mechanic: ForceF only.**

---

You are picking up Phase 3 Round 22 of Lemmings Webolution at
`C:\progs\lemmings-webolution`, a 1:1 reimplementation of Lemmings Revolution.
**The stock 640x480 executable is the authority.**

## Facilitator ownership

**Must not touch:** `PLAN.md`, `docs/PHASE3_HANDOFF.md`, `.claude/rules/*`.
Park prose in `docs/scratch/phase3-round22-forcef.md`. Propose a `PLAN.md`
§1.6 bullet in your final message.

**You own:** scratch note, sim/asset/harness/tests in scope.

**One agent owns the running game — you.** Restore every live write.

## Read first

| File | What it gives you |
| --- | --- |
| `.claude/rules/match-the-original.md` | evidence standard |
| `docs/scratch/phase3-round18-balloon.md` / `phase3-round19-fire.md` | cue clean-negative pattern |
| `docs/scratch/phase3-round21-lever.md` | real object gadget contrast |
| `PLAN.md` §1.6 | ledger (read-only) |

## Your question — one mechanic

**What does ForceF do to a lemming (or the world), and how is it detected?**

Settle that and stop. Do **not** take Axe, Bubble, Scrape, or other gadgets.

### Leads (observations, not conclusions)

- Cue string **`ForceF`** at **`0x004C0CC8`**. Register `push 0x4c0cc8` at
  **`0x0045E600`**; store to **`+0x1c8d0`** at **`0x0045E612`**.
- Slot inventory (facilitator): `+0x1c8d0` sites **`0x0045E614`**,
  **`0x0045EA39`**, **`0x0045F0AA`** (register / preload / unload shape).
- Global `0x004C6A08+0x1c8d0 = DAT_004E32D8` had **0** absolute `.text`
  refs in a facilitator scan — **re-verify**. May be Balloon/Fire-shaped.
- **No** `force*` / `fan*` / `blow*` `.a3d` in COMPILED (facilitator).
  Prove play sites or clean-negative; do not invent a fan force.

## Evidence standard

Same as prior rounds. Actor list `0x004CBB18`. No `PIXELS_PER_UNIT`.
Mutate-alone + touch + suite 16/16.

## Settled — do not redo

Anti-grav / trampoline / drown / teleport / SpeedPad / Lever / Balloon /
Fire / Witch / exit / blocker.

## Deliverables

1. `docs/scratch/phase3-round22-forcef.md`
2. Sim / clean-negative flag as warranted
3. Tests + suite 16/16
4. Propose §1.6 bullet only
5. Process / restores clean

**Report honestly.**
