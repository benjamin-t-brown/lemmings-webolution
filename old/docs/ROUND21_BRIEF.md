# Round 21 brief — Lever (Round 9 gadget repeat)

This file is a **self-contained prompt**. Paste everything below the rule into a
fresh session. A facilitator owns `PLAN.md` §1.6, `docs/PHASE3_HANDOFF.md`, and
`.claude/rules/` — do **not** edit those; report proposed ledger text instead.

Written 2026-10-16, after rounds 1–20 closed. Tree green at 16/16. Queue after
this: ForceF, Axe, Bubble, Scrape, Rumble, Changeop, Die. **One mechanic:
Lever only.**

---

You are picking up Phase 3 Round 21 of Lemmings Webolution at
`C:\progs\lemmings-webolution`, a 1:1 reimplementation of Lemmings Revolution.
**The stock 640x480 executable is the authority.**

## Facilitator ownership

**Must not touch:** `PLAN.md`, `docs/PHASE3_HANDOFF.md`, `.claude/rules/*`.
Park prose in `docs/scratch/phase3-round21-lever.md`. Propose a `PLAN.md`
§1.6 bullet in your final message.

**You own:** scratch note, sim/asset/harness/tests in scope.

**One agent owns the running game — you.** Restore every live write.

## Read first

| File | What it gives you |
| --- | --- |
| `.claude/rules/match-the-original.md` | evidence standard |
| `.claude/rules/windows-msys2-build.md` | build + PATH traps |
| `docs/PHASE3_HANDOFF.md` | rounds table; object AABB pattern |
| `docs/scratch/phase3-round16-teleport.md` / `phase3-round17-speedpad.md` | object gadgets |
| `docs/scratch/phase3-round9-antigrav.md` | object lists |
| `PLAN.md` §1.6 | ledger (read-only) |

## Your question — one mechanic

**What does a Lever do (to the world / to a lemming), and how does the
engine detect interaction?**

Settle that and stop. Do **not** take ForceF, Axe, Bubble, or other gadgets.

### Not this round

- Do not reopen teleport / SpeedPad / Witch easter egg / drown / trampoline.

### Leads (observations, not conclusions)

- Cue string **`Lever`** at **`0x004C0CE0`**. Register `push 0x4c0ce0` at
  **`0x0045E648`**; store to **`+0x1c8dc`** at **`0x0045E65A`**.
- Level assets: **`lever.a3d`** in **55** COMPILED levels (**130**
  placements — facilitator census). Strong world-object lead (unlike
  Balloon/Fire/Witch).
- Detection may be object AABB (`0x00430BA1`), click/pick, walker overlap,
  or a linked door/bridge. Shape unknown: toggle terrain, open a gate,
  flip gravity, play cue only, etc. Do not invent classic Lemmings switch
  rules.
- Round 16 noted a lever-ish vtable candidate among object inventory —
  re-verify; do not assume.

## Evidence standard

1. Address + bytes, or re-runnable script.
2. Predictions **before** each live run, with falsifier.
3. Actor list `0x004CBB18` only — never billboards.
4. `.data` citations inside section rawsize `0x7000` only.
5. **One cell is one world unit.** Never multiply by `PIXELS_PER_UNIT`.
6. No classic Lemmings; no halving classic values.
7. Clean negative is a result.
8. Preserve quirks.
9. Read back the loaded path after `--enter` before trusting a dump.

## Settled — do not redo

Anti-grav / trampoline / drown / teleport / SpeedPad / Balloon / Fire /
Witch / exit / blocker. Do not move settled fields.

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

1. `docs/scratch/phase3-round21-lever.md`
2. Sim changes for what you settled
3. Tests that fail when reverted alone; suite 16/16
4. Propose §1.6 bullet only
5. Process / restores clean

**Report honestly.**
