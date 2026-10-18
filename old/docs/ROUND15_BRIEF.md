# Round 15 brief — acid as a hazard (Round 9 gadget repeat)

This file is a **self-contained prompt**. Paste everything below the rule into a
fresh session. A facilitator owns `PLAN.md` §1.6, `docs/PHASE3_HANDOFF.md`, and
`.claude/rules/` — do **not** edit those; report proposed ledger text instead.

Written 2026-10-16, after rounds 1–14 closed. Tree green at 16/16. Anti-grav,
trampoline, and water-hazard are done; this is the next open gadget.

---

You are picking up Phase 3 Round 15 of Lemmings Webolution at
`C:\progs\lemmings-webolution`, a 1:1 reimplementation of Lemmings Revolution.
**The stock 640x480 executable is the authority.**

## Facilitator ownership

**Must not touch:** `PLAN.md`, `docs/PHASE3_HANDOFF.md`, `.claude/rules/*`.
Park prose in `docs/scratch/phase3-round15-acid.md`. Propose a `PLAN.md` §1.6
bullet in your final message.

**You own:** scratch note, sim/asset/harness/tests in scope.

**One agent owns the running game — you.** Restore every live write.

## Read first

| File | What it gives you |
| --- | --- |
| `.claude/rules/match-the-original.md` | evidence standard |
| `.claude/rules/windows-msys2-build.md` | build + PATH traps |
| `docs/PHASE3_HANDOFF.md` | rounds table; water = `0x40` drown |
| `docs/scratch/phase3-round14-water.md` | **sibling type `0x2e` left open as possible acid** |
| `docs/scratch/phase3-round9-antigrav.md` | hatch type 1 (`entrance_acid`) → `[+0x4c]=1` |
| `PLAN.md` §1.6 | ledger (read-only) |

## Your question — one mechanic

**What does acid (as a hazard) do to a lemming, and how does the engine detect it?**

Settle that and stop. Do **not** take water, teleport, trampoline, or anti-grav.

### Not this round

- **`Exit_acid.a3d` is a save exit** (round 12, one level `E23_JC_1`). Same
  2×3 AABB as other exits. Do not re-measure it.
- **Water hazard is settled:** cell bit `0x40` → drown `0x004B0570` ~2.0 s
  (round 14). Do not relitigate unless acid *is* that path under another name
  — if so, prove it with a series, do not assume.

### Leads (observations, not conclusions)

- Round 14's fluid probe (`0x2f` / vtable `0x004B09C0`) has a **sibling**:
  when cell `0x40` is clear, `0x00415552` `and edx, 0x100` builds another
  object (`0x00415710`, vtable `0x004B09D8`). Separately, when the object
  loop at `0x004C89D8` / `0x00420A52` **matches**, attach goes to type
  **`0x2e`** at `0x0041545B` instead of drown `0x22`. Round 14 explicitly
  left `0x2e` open as a candidate for acid — start there.
- Strings: `"LemmingsAcid"` near `"LemmingsWater"`; `"^acid_glow"` /
  `"^lava_glow"` near `"^water_glow"` / `"Fluid"`.
- **`Entrance_acid.a3d`** in 6 levels (`E23_JC_1`, `H17_MG_1`, `H23_MG_1`,
  `H24_KF_1`, `M20_MT_1`, `VH12_KF1`). Round 9: hatch type 1 → live
  `[+0x4c] = 1`. That may gate splat exemptions (`0x200` / type 1), not
  the hazard itself — measure which.
- No separate `"acid"` animation in `read_steppers.py --states` (only
  `"drown"`). Acid may reuse drown, use another string (`Die`, bubbles),
  or a different death path. Do not invent a classic melt.
- Cell bit `0x100` (`kCellFlag0x100`) is still unnamed except as the
  clear-`0x40` branch above — may or may not be acid.

### Shape to expect

Wrong shape is common. Acid may be an object AABB (Fluid match), a cell bit,
a lemming type gate, or reuse of drown under another attach. A clean negative
("acid uses the same drown path as water") is a publishable result if evidenced.

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

Anti-grav `0x8000`. Trampoline `0x1000`. Water drown `0x40` / `drownMs=2000`.
Exit AABB 2×3. Blocker `0x800`. Do not move settled fields; stop and report
if a finding seems to require it.

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

1. `docs/scratch/phase3-round15-acid.md`
2. Sim changes for what you settled, MEASURED/READ + addresses
3. Tests that fail when reverted alone; suite 16/16
4. Do **not** update PLAN/handoff/rules — propose the §1.6 bullet
5. Process state + every live write restored

**Report honestly.** Do not round, smooth, or over-claim.
