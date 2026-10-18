# Round 19 brief — Fire (Round 9 gadget repeat)

This file is a **self-contained prompt**. Paste everything below the rule into a
fresh session. A facilitator owns `PLAN.md` §1.6, `docs/PHASE3_HANDOFF.md`, and
`.claude/rules/` — do **not** edit those; report proposed ledger text instead.

Written 2026-10-16, after rounds 1–18 closed. Tree green at 16/16. Queue after
this round (facilitator will continue): Witch, Lever, ForceF, Axe, Bubble,
Scrape, Rumble, Changeop, Die. **One mechanic this session: Fire only.**

---

You are picking up Phase 3 Round 19 of Lemmings Webolution at
`C:\progs\lemmings-webolution`, a 1:1 reimplementation of Lemmings Revolution.
**The stock 640x480 executable is the authority.**

## Facilitator ownership

**Must not touch:** `PLAN.md`, `docs/PHASE3_HANDOFF.md`, `.claude/rules/*`.
Park prose in `docs/scratch/phase3-round19-fire.md`. Propose a `PLAN.md`
§1.6 bullet in your final message.

**You own:** scratch note, sim/asset/harness/tests in scope.

**One agent owns the running game — you.** Restore every live write.

## Read first

| File | What it gives you |
| --- | --- |
| `.claude/rules/match-the-original.md` | evidence standard |
| `.claude/rules/windows-msys2-build.md` | build + PATH traps |
| `docs/PHASE3_HANDOFF.md` | rounds table; gadget pattern |
| `docs/scratch/phase3-round18-balloon.md` | cue-only clean-negative template |
| `docs/scratch/phase3-round17-speedpad.md` | cue → object → effect pattern |
| `PLAN.md` §1.6 | ledger (read-only) |

## Your question — one mechanic

**What does Fire do to a lemming, and how does the engine detect it?**

Settle that and stop. Do **not** take Witch, Lever, ForceF, or other gadgets.

### Not this round

- Do not remeasure drown / trampoline / SpeedPad / Balloon / exit.

### Leads (observations, not conclusions)

- Cue string **`Fire`** at **`0x004C0D2C`** (after `Balloon`, before `Witch`).
- Sole string imm xref: `push 0x4c0d2c` at **`0x0045E708`** (cue register
  table, same `FUN_0045e51f` block as Balloon/SpeedPad).
- Slot inventory (facilitator): `+0x1c8fc` sites **`0x0045E71C`** (write),
  **`0x0045EB41`** (preload), **`0x0045F186`** (unload). Global candidate
  `0x004C6A08+0x1c8fc = DAT_004E3304` had **0** absolute `.text` refs in a
  quick scan — **re-verify**; Balloon's pattern was the same shape.
- **No `fire.a3d` / flame / burn / torch / lava** model names in COMPILED
  (facilitator census). May still be a differently named object, a cell bit,
  a death state, or another Balloon-style clean negative.
- A clean negative is a valid result if play sites and world gadgets are
  absent — pin it the way round 18 pinned Balloon (`k…InRetail=false` or
  equivalent + mutate-alone + `--audit`).

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

Anti-grav `0x8000`. Trampoline `0x1000`. Drown `0x40` / `drownMs=2000`.
Teleport / SpeedPad object AABB. Balloon cue unused. Exit 2×3. Blocker
`0x800`. Do not move settled fields.

## Build and test

```powershell
.\scripts\Invoke-Ucrt64.ps1 "cmake --build --preset dev-debug"
.\scripts\Invoke-Ucrt64.ps1 "ctest --preset dev-debug"
```

No UCRT64 `bin` on PATH for harness Python. Mutate-alone + touch + suite
16/16.

## Must not touch

`PLAN.md`, `docs/PHASE3_HANDOFF.md`, `.claude/rules/*`,
`test_collisiongrid.cpp`, `test_corpus.cpp` and its expected numbers,
settled `Rules` fields.

## Deliverables

1. `docs/scratch/phase3-round19-fire.md`
2. Sim changes for what you settled (or clean-negative flag)
3. Tests that fail when reverted alone; suite 16/16
4. Do **not** update PLAN/handoff/rules — propose the §1.6 bullet
5. Process state + every live write restored

**Report honestly.** Do not round, smooth, or over-claim.
