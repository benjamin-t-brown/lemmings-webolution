# Round 30 brief — inverted step-up / climb / dig

This file is a **self-contained prompt**. Paste everything below the rule into a
fresh session. A facilitator owns `PLAN.md` §1.6, `docs/PHASE3_HANDOFF.md`, and
`.claude/rules/` — do **not** edit those; report proposed ledger text instead.

Written 2026-10-16, after Round 29. Tree green at 16/16. Queue after this:
cell-flag writers (31). **One mechanic family: inverted steppers only.**

---

You are picking up Phase 3 Round 30 of Lemmings Webolution at
`C:\progs\lemmings-webolution`, a 1:1 reimplementation of Lemmings Revolution.
**The stock 640x480 executable is the authority.**

## Facilitator ownership

**Must not touch:** `PLAN.md`, `docs/PHASE3_HANDOFF.md`, `.claude/rules/*`.
Park prose in `docs/scratch/phase3-round30-inverted.md`. Propose a `PLAN.md`
§1.6 bullet in your final message.

**You own:** scratch note, sim/asset/harness/tests in scope.

**One agent owns the running game — you.** Restore every live write.

## Read first

| File | What it gives you |
| --- | --- |
| `.claude/rules/match-the-original.md` | evidence standard |
| `docs/scratch/phase3-round5-live.md` | dig/bash directional refuse (upright measured) |
| `docs/scratch/phase3-round9-antigrav.md` | `verticalDir` / `[+0x44]` flip |
| `docs/scratch/phase3-round10-steppers.md` | upright walker/climber series |
| `src/assets/CollisionGrid.h` | `0x10`/`0x20` dig; `0x4`/`0x8` bash |
| `src/sim/World.cpp` | existing `verticalDir` threading |

## Your question — one mechanic family

**After anti-grav (`verticalDir = -1` / `[+0x44]==-1`), do step-up, climb,
and dig match the upright series with gravity flipped? Do directional refuse
bits `0x10`/`0x20` (and bash `0x4`/`0x8` if reachable) behave as the
stack-mask reads claim?**

Settle that and stop. Do **not** take cell-flag writers.

### Leads (observations, not conclusions)

- Dig refuse: `mov [ebp-0x10], 0x10` at **`0x00415A86`**, then
  `cmp [ecx+0x44], -1` → `mov [ebp-0x10], 0x20` at **`0x00415A96`**, tested
  `and edx, [ebp-0x10]` at **`0x00415B2A`**. Upright dig of `0x20` cells was
  measured (round 5); **inverted dig of `0x10` / refuse of `0x20` was not**.
- Bash: mask `0x4` / `0x8` when `[+0x40]==-1` at `0x0040C0DC` / `0x0040C0EC`.
  Right-facing half unread live — only if an inverted or far-side run is
  natural; do not invent a level if unreachable.
- Anti-grav flip measured (round 9) on `m06_kf_1`. Actor list `0x004CBB18`.
- Sim already uses `verticalDir` in walk/climb/anti-grav — measure gaps and
  fix only what diverges.

## Evidence standard

Same as prior rounds. No `PIXELS_PER_UNIT`. Mutate-alone + touch + suite
16/16. Windows via `scripts/Invoke-Ucrt64.ps1`. No UCRT64 on harness PATH.

## Settled — do not redo

Upright stepper series (round 10). Anti-grav flip (round 9). Splash cue
rounds. Do not change upright constants unless evidence forces a correction.

## Deliverables

1. `docs/scratch/phase3-round30-inverted.md`
2. Live series + harness as needed
3. Sim/tests promoting read→measured for inverted halves
4. Suite 16/16
5. Propose §1.6 bullet only
6. Process / restores clean

**Report honestly.** Include (a) verdict, (b) evidence, (c) proposed §1.6
bullet, (d) suite, (e) files changed.
