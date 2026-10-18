# Round 31 brief — unread cell-flag writers (`0x100` / `0x400`)

This file is a **self-contained prompt**. Paste everything below the rule into a
fresh session. A facilitator owns `PLAN.md` §1.6, `docs/PHASE3_HANDOFF.md`, and
`.claude/rules/` — do **not** edit those; report proposed ledger text instead.

Written 2026-10-16, after Round 30. Tree green at 16/16. Queue after this:
**empty** (last Phase 3 residue). **One mechanic family: cell-flag writers
only.**

---

You are picking up Phase 3 Round 31 of Lemmings Webolution at
`C:\progs\lemmings-webolution`, a 1:1 reimplementation of Lemmings Revolution.
**The stock 640x480 executable is the authority.**

## Facilitator ownership

**Must not touch:** `PLAN.md`, `docs/PHASE3_HANDOFF.md`, `.claude/rules/*`.
Park prose in `docs/scratch/phase3-round31-cell-writers.md`. Propose a
`PLAN.md` §1.6 bullet in your final message.

**You own:** scratch note, sim/asset/harness/tests in scope.

**One agent owns the running game — you.** Restore every live write.

## Read first

| File | What it gives you |
| --- | --- |
| `.claude/rules/match-the-original.md` | evidence standard |
| `docs/scratch/phase3-round15-acid.md` | `0x100` reader → `lem[+0x5f]`; not death |
| `src/assets/CollisionGrid.h` | `kCellFlag0x100` / `kCellFlag0x400` comments |
| `PLAN.md` §1.6 / Phase 2 residue notes | writer status |

## Your question — one mechanic family

**What paints and clears cell bits `0x100` and `0x400`, and does that change
the Round 15 / CollisionGrid reader story?**

Settle that and stop. This closes the Phase 3 residue list.

### Leads (observations, not conclusions)

- **`0x100`:** reader `and edx, 0x100` at **`0x00415552`** → may set
  `lem[+0x5f]=1` at `0x00415627`. Live paint on MM_CLIMB_002 flips the flag,
  zero drown (round 15). Authored in 3/102 grids (H08_MT_1, H22_MT_1,
  VH06_MT). **No `or`/`and`-imm setter found yet** — find the writer path
  (or prove authored-only / stamped offline).
- **`0x400`:** paint `or edx, 0x403` at **`0x004275C1`**; erase
  `and edx, 0xfbfc` at **`0x00427641`**. Listed callers in CollisionGrid.h
  (`0x004271D5`, `0x00427509`, `0x0043B1F8` paint; erase siblings). Readers
  `0x00461704` / `0x00461843` inside scan `0x004615E9`. Finish the caller
  chain and name the mechanic (or prove dead / UI-only / authored).
- Do not reopen water/acid drown (`0x40`) or zone splat exemption (29)
  unless evidence forces a correction — then stop and report.

## Evidence standard

Same as prior rounds. Actor list `0x004CBB18`. No `PIXELS_PER_UNIT`.
Mutate-alone + touch + suite 16/16. Windows via `scripts/Invoke-Ucrt64.ps1`.
No UCRT64 on harness PATH.

## Settled — do not redo

Round 15: `0x100` is not acid death. Round 14 drown. Zone stamps (29).
Cue rounds.

## Deliverables

1. `docs/scratch/phase3-round31-cell-writers.md`
2. Harness audit as needed
3. Update CollisionGrid comments in-tree (agent may edit that header)
4. Sim only if a missing mechanic falls out
5. Tests + suite 16/16
6. Propose §1.6 bullet only
7. Process / restores clean

**Report honestly.** Include (a) verdict, (b) evidence, (c) proposed §1.6
bullet, (d) suite, (e) files changed.
