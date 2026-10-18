# Round 20 brief — Witch (Round 9 gadget repeat)

This file is a **self-contained prompt**. Paste everything below the rule into a
fresh session. A facilitator owns `PLAN.md` §1.6, `docs/PHASE3_HANDOFF.md`, and
`.claude/rules/` — do **not** edit those; report proposed ledger text instead.

Written 2026-10-16, after rounds 1–19 closed. Tree green at 16/16. Queue after
this: Lever, ForceF, Axe, Bubble, Scrape, Rumble, Changeop, Die. **One
mechanic: Witch only.**

---

You are picking up Phase 3 Round 20 of Lemmings Webolution at
`C:\progs\lemmings-webolution`, a 1:1 reimplementation of Lemmings Revolution.
**The stock 640x480 executable is the authority.**

## Facilitator ownership

**Must not touch:** `PLAN.md`, `docs/PHASE3_HANDOFF.md`, `.claude/rules/*`.
Park prose in `docs/scratch/phase3-round20-witch.md`. Propose a `PLAN.md`
§1.6 bullet in your final message.

**You own:** scratch note, sim/asset/harness/tests in scope.

**One agent owns the running game — you.** Restore every live write.

## Read first

| File | What it gives you |
| --- | --- |
| `.claude/rules/match-the-original.md` | evidence standard |
| `.claude/rules/windows-msys2-build.md` | build + PATH traps |
| `docs/PHASE3_HANDOFF.md` | rounds table |
| `docs/scratch/phase3-round18-balloon.md` / `phase3-round19-fire.md` | cue clean-negative pattern |
| `docs/scratch/phase3-round17-speedpad.md` | cue play on activate |
| `PLAN.md` §1.6 | ledger (read-only) |

## Your question — one mechanic

**What does a Witch do to a lemming, and how does the engine detect it?**

Settle that and stop. Do **not** take Lever, ForceF, Axe, or other gadgets.

### Not this round

- Do not reopen Balloon/Fire clean negatives or SpeedPad/teleport/drown.

### Leads (observations, not conclusions)

- Cue string **`Witch`** at **`0x004C0D34`**. Register `push 0x4c0d34` at
  **`0x0045E720`**; store to **`+0x1c8a8`** at **`0x0045E734`** (not the
  sequential `+0x1c900` — confirm yourself). Preload/unload inventory:
  **`0x0045EB57`**, **`0x0045F19A`**.
- Global `0x004C6A08+0x1c8a8 = DAT_004E32B0` has at least one absolute
  `.text` ref: **`0x0045D200`** `mov ecx, [0x4e32b0]` then
  `call 0x0044521B` (same play helper Action uses). **This is not a
  Balloon/Fire-shaped clean negative** unless you falsify that site.
- No `witch*.a3d` in COMPILED (facilitator census). Detection may be an
  object with another name, a UI/menu cue, a skill, or a world gadget —
  prove which, and what it does to a lemming (or that it never touches one).
- Remaining queue after you: Lever, ForceF, Axe, Bubble, Scrape, Rumble,
  Changeop, Die — out of scope this round.

## Evidence standard

1. Address + bytes, or re-runnable script.
2. Predictions **before** each live run, with falsifier.
3. Actor list `0x004CBB18` only — never billboards.
4. `.data` citations inside section rawsize `0x7000` only.
5. **One cell is one world unit.** Never multiply by `PIXELS_PER_UNIT`.
6. No classic Lemmings; no halving classic values.
7. Clean negative is a result (but the play ref above must be explained).
8. Preserve quirks.
9. Read back the loaded path after `--enter` before trusting a dump.

## Settled — do not redo

Anti-grav / trampoline / drown / teleport / SpeedPad / Balloon / Fire /
exit / blocker. Do not move settled fields.

## Build and test

```powershell
.\scripts\Invoke-Ucrt64.ps1 "cmake --build --preset dev-debug"
.\scripts\Invoke-Ucrt64.ps1 "ctest --preset dev-debug"
```

Mutate-alone + touch + suite 16/16. No UCRT64 `bin` on PATH for harness
Python.

## Must not touch

`PLAN.md`, `docs/PHASE3_HANDOFF.md`, `.claude/rules/*`,
`test_collisiongrid.cpp`, `test_corpus.cpp` expected numbers, settled
`Rules` fields.

## Deliverables

1. `docs/scratch/phase3-round20-witch.md`
2. Sim changes for what you settled
3. Tests that fail when reverted alone; suite 16/16
4. Propose §1.6 bullet only — do not edit facilitator docs
5. Process / restores clean

**Report honestly.**
