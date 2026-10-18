# Round 13 brief — trampoline (Round 9 gadget repeat)

This file is a **self-contained prompt**. Paste everything below the rule into a
fresh session. It assumes a facilitator owns `PLAN.md` §1.6, `docs/PHASE3_HANDOFF.md`,
and `.claude/rules/` — do **not** edit those; report proposed ledger text instead.

Written 2026-10-16, after rounds 1–12 closed. Tree green at 16/16. Anti-grav
(round 9) is done; this is the next open gadget in the handoff list.

---

You are picking up Phase 3 Round 13 of Lemmings Webolution at
`C:\progs\lemmings-webolution`, a 1:1 reimplementation of Lemmings Revolution.
**The stock 640x480 executable is the authority** — not classic Lemmings, not
plausibility, not a decompiler's guess.

## Facilitator ownership

**Must not touch:** `PLAN.md`, `docs/PHASE3_HANDOFF.md`, `.claude/rules/*`.
Park prose in `docs/scratch/phase3-round13-trampoline.md`. Report a proposed
`PLAN.md` §1.6 bullet in your final message for the facilitator to land after
independent verification.

**You own:** the scratch note, sim/asset code in scope, tests (except the two
named below), and harness under `tools/harness/`.

**One agent owns the running game at a time — that is you.** Restore every live
write; do not leave the process running when you finish.

## Read first, in this order

| File | What it gives you |
| --- | --- |
| `.claude/rules/match-the-original.md` | evidence standard |
| `.claude/rules/windows-msys2-build.md` | build + PATH traps |
| `docs/PHASE3_HANDOFF.md` | rounds table; anti-grav result; how to read a constant |
| `docs/scratch/phase3-round9-antigrav.md` | prior gadget shape (cell bit + object overlap) |
| `docs/scratch/phase3-round5.md` | trampoline vtable `0x004B0D08` / `FUN_0041B683` note |
| `PLAN.md` §1.6 | ledger (read-only) |

## Your question — one mechanic

**What does a trampoline do to a lemming, and how does the engine detect it?**

Settle that and stop. Do not also take water, acid, teleport, or hazards.

### Leads already on the table (observations, not conclusions)

- Animation / state string `"trampoline"` at `0x004BF634`.
- State vtable **`0x004B0D08`** (`read_steppers.py --states`): enter-side
  addresses include `0x0041BBE0`, `0x0041BA81`, `0x0041B838`; slot 3 is
  `FUN_0041B683` (`0x0041B683`) — the four-cell `and 0xf77c` removal that was
  **wrongly** attributed to bash/dig/mine and has **no** direct `e8` caller in
  `.text`. Re-read before assuming what it clears.
- Debug strings in `.data` (check rawsize before citing values from the image):
  `"Bounce hit trampoline"`, `"Bounce hit floor"`, `"Bounce hit head"`,
  `"New bounce pos %2.3f, %2.3f"`, `"Trampoline Block at %d, %d"`,
  `"%d Bounce started at foot position"`.
- `LEMMINGSLO.SPR` carries a `trampoline` strip (duration = frames × 0.06 s /
  divisor — see round 5/6).
- Landing-path zone arms still open and may or may not be this gadget:
  `0x0041662B` `and 0x200` / `[+0x4c]==1`, `0x00416641` `and 0x4000` /
  `[+0x4c]==2`. Anti-grav was `0x8000` in the **walk** probe, not these.
- Level assets: no `.a3d` name contains `tramp` / `bounc`. `pressure_pad.a3d`
  appears in 23 levels — it is a **candidate**, not proven to be the trampoline.
  Re-run your own census; find a playable level that actually enters the
  trampoline state (watch `[lemming+0x340]` / actor list for vtable
  `0x004B0D08`).

### Shape to expect

Wrong *shape* is more common than wrong number here. The bounce may be a
state machine with animation length, a cell bit, an object AABB (like the
exit / anti-grav overlap), or a temporary velocity — do not invent a
"trampolineHeight = N" from classic Lemmings.

## Evidence standard

1. Address + bytes, or a re-runnable script. `python tools/harness/read_steppers.py --at 0x004B0D08 --len 0x40` and `--at 0x0041B838 --len 0x100`.
2. Predictions written **before** each live run, with falsifier.
3. Lemmings from actor list `0x004CBB18` only — never billboard quads.
   `compare_tick.py` is superseded.
4. `.data` citations must fall inside section **rawsize** `0x7000` (file bytes
   only to `0x004C6000`).
5. **One cell is one world unit.** Never multiply by `PIXELS_PER_UNIT`.
6. No classic Lemmings invention; no halving classic values.
7. Clean negative is a result (e.g. pressure_pad is unrelated).
8. Preserve quirks.
9. Always read back the loaded path after `goto_level` / `--enter` before
   trusting a dump.

## Settled — do not redo

Every `Rules` field is measured/read. Anti-grav is `0x8000` + cartwheel.
Exit is object AABB 2×3, column `round(ang×width)`. Blocker is `0x800`
runtime terrain. Do not move settled fields; stop and report if a finding
seems to require it.

## Build and test

```powershell
.\scripts\Invoke-Ucrt64.ps1 "cmake --build --preset dev-debug"
.\scripts\Invoke-Ucrt64.ps1 "ctest --preset dev-debug"
```

Do **not** put UCRT64 `bin` on PATH for harness Python. Note which binary
holds your test (`test_world` / `test_terrain` / `test_levelobjects`).

Mutate each new constant **alone**, require a behavioural failure, restore
(and **touch** the restored source so ninja does not keep a stale `.obj`),
then full suite 16/16.

## Must not touch

`PLAN.md`, `docs/PHASE3_HANDOFF.md`, `.claude/rules/*`,
`src/__test__/test_collisiongrid.cpp`, `src/__test__/test_corpus.cpp` and its
expected numbers, settled `Rules` fields.

In scope: `src/sim/*`, `src/assets/LevelObjects.*` / `Terrain.*` /
`CollisionGrid.*` only as needed for the trampoline finding, other tests,
`tools/harness/`.

## Deliverables

1. `docs/scratch/phase3-round13-trampoline.md`: addresses/bytes, predictions,
   live series, verdict, left open with citations.
2. Sim changes for what you settled, MEASURED/READ comments with addresses.
3. Tests that fail when the new behaviour is reverted alone; suite 16/16.
4. **Do not** update PLAN/handoff/rules — propose the §1.6 bullet in your
   final message.
5. Process state + confirmation every live write was restored.

**Report honestly.** Do not round, smooth, or over-claim.
