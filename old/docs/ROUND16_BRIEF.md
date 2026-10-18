# Round 16 brief — teleport (Round 9 gadget repeat)

This file is a **self-contained prompt**. Paste everything below the rule into a
fresh session. A facilitator owns `PLAN.md` §1.6, `docs/PHASE3_HANDOFF.md`, and
`.claude/rules/` — do **not** edit those; report proposed ledger text instead.

Written 2026-10-16, after rounds 1–15 closed. Tree green at 16/16. Anti-grav,
trampoline, and water/acid drown are done; this is the next open gadget.

---

You are picking up Phase 3 Round 16 of Lemmings Webolution at
`C:\progs\lemmings-webolution`, a 1:1 reimplementation of Lemmings Revolution.
**The stock 640x480 executable is the authority.**

## Facilitator ownership

**Must not touch:** `PLAN.md`, `docs/PHASE3_HANDOFF.md`, `.claude/rules/*`.
Park prose in `docs/scratch/phase3-round16-teleport.md`. Propose a `PLAN.md`
§1.6 bullet in your final message.

**You own:** scratch note, sim/asset/harness/tests in scope.

**One agent owns the running game — you.** Restore every live write.

## Read first

| File | What it gives you |
| --- | --- |
| `.claude/rules/match-the-original.md` | evidence standard |
| `.claude/rules/windows-msys2-build.md` | build + PATH traps |
| `docs/PHASE3_HANDOFF.md` | rounds table; gadget pattern |
| `docs/scratch/phase3-round9-antigrav.md` | object list + `0x00430BA1` overlap |
| `docs/scratch/phase3-round11-exit.md` | same overlap used for exit; 19 call sites |
| `PLAN.md` §1.6 | ledger (read-only) |

## Your question — one mechanic

**What does a teleport do to a lemming, and how does the engine detect it?**

Settle that and stop. Do **not** take SpeedPad, Balloon, Fire, or other gadgets.

### Not this round

- Do not remeasure exit / anti-grav / drown / trampoline.
- A harness teleport-and-hold for probing is fine; that is not the game's
  teleport gadget.

### Leads (observations, not conclusions)

- Level assets: **`teleport.a3d`** and **`teleport_fx.a3d`** each in **18**
  retail levels (same set). Named level **`MM_TELE_003`**. Re-run your own
  census against `build/lvl_dump/levels/COMPILED`.
- No `"teleport"` / `"portal"` / `"warp"` ASCII string in the stock exe image
  (search returned 0). Detection is unlikely to be a named anim strip alone —
  look for object vtables, overlap, or cell bits.
- General object-overlap **`0x00430BA1`** has **19** `.text` call sites.
  Known: anti-grav `0x0041C1E3`; exit among them. The rest are unread and are
  the inventory for "which object uses AABB overlap":
  ```
  0x41c1e3 0x41c9f6 0x41ca6a 0x41ca9a 0x41cabe 0x41cf40 0x41cfb1
  0x41d442 0x41d45b 0x426b5b 0x42c7e9 0x42c82d 0x430c7b 0x431ad3
  0x4327c3 0x435467 0x4354cb 0x436da9 0x436ddc
  ```
- Anti-grav needed **both** a cell bit and an object from list `0x004C8958`.
  Teleport may be object-only (pair of pads), cell+object, or something else.
- Cue leftovers in `.data` include `Changeop` (`0x004C0CB4`) and `SpeedPad`
  (`0x004C0D10`) — may or may not be this round; do not chase them unless the
  teleport path lands there.
- `teleport_fx.a3d` may be the visual only (round 14/15 glow pattern) — prove
  which model owns the behaviour.

### Shape to expect

Wrong shape is common. Expect a pair of pads, an AABB, a state that relocates
`(X,Y)`, facing preservation quirks, or fx then snap. Do not invent classic
Lemmings teleporter rules. A clean negative ("fx only, behaviour is elsewhere")
is a result.

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

Anti-grav `0x8000`. Trampoline `0x1000`. Water/acid drown `0x40` /
`drownMs=2000`. Exit AABB 2×3, column `round`. Blocker `0x800`. Do not move
settled fields; stop and report if a finding seems to require it.

## Build and test

```powershell
.\scripts\Invoke-Ucrt64.ps1 "cmake --build --preset dev-debug"
.\scripts\Invoke-Ucrt64.ps1 "ctest --preset dev-debug"
```

No UCRT64 `bin` on PATH for harness Python. Mutate each new constant
**alone**, behavioural fail, restore (**touch** source), suite 16/16.

## Must not touch

`PLAN.md`, `docs/PHASE3_HANDOFF.md`, `.claude/rules/*`,
`test_collisiongrid.cpp`, `test_corpus.cpp` and its expected numbers,
settled `Rules` fields.

## Deliverables

1. `docs/scratch/phase3-round16-teleport.md`
2. Sim changes for what you settled, MEASURED/READ + addresses
3. Tests that fail when reverted alone; suite 16/16
4. Do **not** update PLAN/handoff/rules — propose the §1.6 bullet
5. Process state + every live write restored

**Report honestly.** Do not round, smooth, or over-claim.
