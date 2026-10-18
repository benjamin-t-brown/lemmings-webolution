# Round 14 brief — water as a hazard (Round 9 gadget repeat)

This file is a **self-contained prompt**. Paste everything below the rule into a
fresh session. A facilitator owns `PLAN.md` §1.6, `docs/PHASE3_HANDOFF.md`, and
`.claude/rules/` — do **not** edit those; report proposed ledger text instead.

Written 2026-10-16, after rounds 1–13 closed. Tree green at 16/16. Anti-grav
and trampoline are done; this is the next open gadget.

---

You are picking up Phase 3 Round 14 of Lemmings Webolution at
`C:\progs\lemmings-webolution`, a 1:1 reimplementation of Lemmings Revolution.
**The stock 640x480 executable is the authority.**

## Facilitator ownership

**Must not touch:** `PLAN.md`, `docs/PHASE3_HANDOFF.md`, `.claude/rules/*`.
Park prose in `docs/scratch/phase3-round14-water.md`. Propose a `PLAN.md` §1.6
bullet in your final message.

**You own:** scratch note, sim/asset/harness/tests in scope.

**One agent owns the running game — you.** Restore every live write.

## Read first

| File | What it gives you |
| --- | --- |
| `.claude/rules/match-the-original.md` | evidence standard |
| `.claude/rules/windows-msys2-build.md` | build + PATH traps |
| `docs/PHASE3_HANDOFF.md` | rounds table; anti-grav + trampoline |
| `docs/scratch/phase3-round9-antigrav.md` | `[+0x4c]` from hatch type; zone-bit leads |
| `docs/scratch/phase3-round3b-float-splat.md` | landing path `0x200` / `0x4000` splat exemptions |
| `docs/scratch/phase3-round13-trampoline.md` | prior gadget shape (runtime bit) |
| `PLAN.md` §1.6 | ledger (read-only) |

## Your question — one mechanic

**What does water (as a hazard) do to a lemming, and how does the engine detect it?**

Settle that and stop. Do **not** take acid, teleport, trampoline, or anti-grav.

### Not this round

- **`Exit_water.a3d` is a save exit**, same 2×3 AABB as `Exit.a3d` (round 12).
  Do not re-measure the exit box. If water *hazard* and water *exit* interact,
  say so; do not fold them into one claim without evidence.
- **`Entrance_water.a3d`** (20 levels) may set hatch type / `[+0x4c]` — related
  lead, not automatically the hazard.

### Leads (observations, not conclusions)

- Animation / state string **`"drown"`** at `0x004BF130` (also `"bubbles"`
  nearby); vtable **`0x004B0570`** (`read_steppers.py --states`: enter-side
  `0x004087d0` / `0x00408766` / `0x0040874c` — enter pushes the drown string).
- Related strings (leads only): `"Fluid"`, `"^water_glow"`, `"LemmingsWater"`
  (siblings `"^lava_glow"` / `"^acid_glow"` / `"LemmingsAcid"`).
- Landing-path splat exemptions (round 3b), still open as *which gadget*:
  ```
  0x0041662B   and 0x200    / cmp [eax+0x4c], 1  -> safe landing
  0x00416641   and 0x4000   / cmp [edx+0x4c], 2  -> safe landing
  ```
  Round 9 measured hatch type 1 (`entrance_acid`) → live `[+0x4c] = 1`.
  Which bit / type is water is unread.
- CollisionGrid names `kCellZoneType0 = 0x0200`, `kCellZoneType1 = 0x2000`,
  `kCellZoneType2 = 0x4000` — zone stamps, not yet wired in the sim.
- Level assets: `Entrance_water.a3d` in 20 levels; `Exit_water.a3d` in 14
  (save exit). Re-run your own census; find a level where a lemming actually
  enters drown (actor list `0x004CBB18`, watch for vtable `0x004B0570`).
- Cue inventory also has a `Die` string distinct from `Splat` / `FallSplat` —
  may or may not be this path.

### Shape to expect

Wrong shape is common. Water may be a cell bit, an object AABB, a zone stamp
tied to `[+0x4c]`, a drown animation length, or something else. Do not invent
a classic-Lemmings drown depth.

## Evidence standard

1. Address + bytes, or re-runnable script.
2. Predictions **before** each live run, with falsifier.
3. Actor list `0x004CBB18` only — never billboards. `compare_tick.py` superseded.
4. `.data` citations inside section rawsize `0x7000` only.
5. **One cell is one world unit.** Never multiply by `PIXELS_PER_UNIT`.
6. No classic Lemmings; no halving classic values.
7. Clean negative is a result.
8. Preserve quirks.
9. Read back the loaded path after `--enter` before trusting a dump.

## Settled — do not redo

Every `Rules` field measured/read. Anti-grav `0x8000`. Trampoline `0x1000`
(runtime). Exit AABB 2×3, column `round`. Blocker `0x800`. Do not move
settled fields; stop and report if a finding seems to require it.

## Build and test

```powershell
.\scripts\Invoke-Ucrt64.ps1 "cmake --build --preset dev-debug"
.\scripts\Invoke-Ucrt64.ps1 "ctest --preset dev-debug"
```

No UCRT64 `bin` on PATH for harness Python. Note which binary holds your
test. Mutate each new constant **alone**, behavioural fail, restore (**touch**
source so ninja does not keep a stale `.obj`), suite 16/16.

## Must not touch

`PLAN.md`, `docs/PHASE3_HANDOFF.md`, `.claude/rules/*`,
`test_collisiongrid.cpp`, `test_corpus.cpp` and its expected numbers,
settled `Rules` fields.

## Deliverables

1. `docs/scratch/phase3-round14-water.md`
2. Sim changes for what you settled, MEASURED/READ + addresses
3. Tests that fail when reverted alone; suite 16/16
4. Do **not** update PLAN/handoff/rules — propose the §1.6 bullet
5. Process state + every live write restored

**Report honestly.** Do not round, smooth, or over-claim.
