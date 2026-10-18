# Round 23 brief — Axe (Round 9 gadget repeat)

This file is a **self-contained prompt**. Paste everything below the rule into a
fresh session. A facilitator owns `PLAN.md` §1.6, `docs/PHASE3_HANDOFF.md`, and
`.claude/rules/` — do **not** edit those; report proposed ledger text instead.

Written 2026-10-16, after rounds 1–22 closed. Tree green at 16/16. Queue after
this: Bubble, Scrape, Rumble, Changeop, Die. **One mechanic: Axe only.**

---

You are picking up Phase 3 Round 23 of Lemmings Webolution at
`C:\progs\lemmings-webolution`, a 1:1 reimplementation of Lemmings Revolution.
**The stock 640x480 executable is the authority.**

## Facilitator ownership

**Must not touch:** `PLAN.md`, `docs/PHASE3_HANDOFF.md`, `.claude/rules/*`.
Park prose in `docs/scratch/phase3-round23-axe.md`. Propose a `PLAN.md`
§1.6 bullet in your final message.

**You own:** scratch note, sim/asset/harness/tests in scope.

**One agent owns the running game — you.** Restore every live write.

## Read first

| File | What it gives you |
| --- | --- |
| `.claude/rules/match-the-original.md` | evidence standard |
| `docs/scratch/phase3-round20-witch.md` | cue with a real play site that is not a lemming gadget |
| `docs/scratch/phase3-round21-lever.md` | object AABB hazard/gadget |
| `PLAN.md` §1.6 | ledger (read-only) |

## Your question — one mechanic

**What does an Axe do to a lemming, and how does the engine detect it?**

Settle that and stop. Do **not** take Bubble, Scrape, Smoke, or other cues.

### Leads (observations, not conclusions)

- Cue string **`Axe`** at **`0x004C0D6C`**. Register differs from the SFX
  table path: `push 0x4c0d6c` at **`0x0045E7C3`**, `mov ecx, 0x58d23c`,
  `call 0x00446040`, store to **`+0x1c914`** at **`0x0045E7D5`**.
- Global `0x004C6A08+0x1c914 = DAT_004E331C` has an absolute ref:
  **`0x00496E40`** `mov eax, [0x4e331c]` → `call 0x00496D5B`. **Not**
  Balloon/Fire-shaped unless you falsify that site (may be UI/FX like
  Witch).
- Slot `+0x1c914` sites include register/preload/unload-like addresses
  `0x45e7d7` / `0x45ebeb` / `0x45eedc` / `0x45eef7` — inventory them.
- **No `axe.a3d`** in COMPILED (facilitator). LVL “axe” hits look like
  embedded FX-name tables next to Smoke/Bubbles — not placements.
- Shape unknown: kill zone, timed blade, UI cursor, FX-only. Prove what
  happens to a lemming (or that nothing does).

## Evidence standard

Same as prior rounds. Actor list `0x004CBB18`. No `PIXELS_PER_UNIT`.
Mutate-alone + touch + suite 16/16.

## Settled — do not redo

Anti-grav / trampoline / drown / teleport / SpeedPad / Lever / Balloon /
Fire / Witch / ForceF / exit / blocker.

## Deliverables

1. `docs/scratch/phase3-round23-axe.md`
2. Sim changes or clean-negative flag
3. Tests + suite 16/16
4. Propose §1.6 bullet only
5. Process / restores clean

**Report honestly.**
