# Current reconstruction roadmap

Updated 2026-11-15 by the project lead: smoke gate and playtest-driven confirmation. Current
milestone: **playable v1**. Full-project completion criteria in completion.json
are unchanged and are not satisfied by this milestone.

## Playable v1
Goal: a person can play every one of the 102 installed levels from the ordinary
frontend, see what the lemmings and the level are doing, win, and keep progress,
without the original executable.

The first playtest ([ledger](../docs/playtest/2026-11-12-playtest-1.md)) showed
that a scripted win is not this bar: scripts read simulation state, while a
player saw one-frame lemmings, invisible terrain edits and an unusable Basher.
Scripted routes remain regression tests. Playability is confirmed by short human
playtests on an optimized build.

Set aside for this milestone, not resolved: matching the original's timing (D1/D3,
release/digger cadence, base clock), full-frame pixel fidelity (C2/C3), original
depth state and Armageddon timing. Their ledger entries in docs/STATUS.md stay
FAIL/OPEN/PARKED and remain required for full completion. Fixes for playtest
defects may touch the same code (skills, animation timing, occlusion); they aim at
correct visible behavior, use retained static evidence where it exists, and label
anything provisional. They are not fidelity passes.

## Work order
Defects come from the playtest ledgers under docs/playtest/; the queue (agent/RUN_STATUS.md)
holds the current ones. Standing order:
1. Finish release-rate-control-001 under the smoke gate.
2. **replay-recorder-001**: record a played level's inputs and replay them headless, so each
   level the user wins becomes a regression check without anyone writing a test.
3. Remaining playtest-6 defects (death particles, yippee cue), then each new ledger batch.
4. **test-legacy-prune-001**: delete before/baseline/identity replays and their CMake.

After each batch the lead rebuilds the playtest binary and gives the user a what-to-check
list. Progress is the level table, [docs/playtest/levels.md](../docs/playtest/levels.md):
one row per installed level, won by a person or not.

## Acceptance for playable v1
- Every level is marked won by a person in the level table (or, where the user accepts it,
  by a recorded replay), or has an explicit user-accepted exclusion.
- Every skill and gadget type has visible, readable behavior.
- The smoke gate passes on the final content: no crash, hang, failed load or blank view on
  any installed level, and the recovery suite passes.
- No open playtest item marked blocker.
- One `--profile all` run at sign-off; legacy tests that fail on intended changes are
  removed with reasons, the rest pass.
- Reports keep human observation, scripted clone observation, source inference and original
  comparison distinct, and restate no historical gate as passed.

## Verification
Per-task gate: `python tools/agent_control/verify.py --profile smoke`. First run 2026-11-15:
95 main smoke tests in 60 s and 84 recovery tests in 19 s, 1 min 40 s wall with an
incremental build. Tiers and rules are in AGENTS.md "Verification tiers" and
cmake/test_tiers.cmake.

Why it changed: between 2026-11-12 and 2026-11-15 the main suite grew from 148 tests and
5.5 minutes to 330 tests and 19 minutes, 39% of it re-running frozen pre-task code, while
the user's playtests twice overruled work the tests and reviews had accepted. Commits in
that period added about 13,000 lines of tests and CMake against 3,400 of production code.

Do not run the clone against the normal save while verification runs; use
scripts/Play-Clone.ps1.

History, full suite on 2026-11-12, commit 721c64e (build/test-parallel-benchmark-001/):

| Configuration | Main suite wall time | Result |
| --- | ---: | --- |
| Debug, 1 test job | 2583 s | 147/148; the failure was a concurrent human playtest writing the normal save |
| Debug, 4 test jobs | 1183 s and 1212 s | 148/148 twice; `levelreturn` 321 s and 351 s of a 360 s timeout |
| -O2 with assertions, 4 test jobs | 329 s | 148/148; `levelreturn` 66 s |

The full suite keeps `-O2 -g` with assertions and four test jobs.

## Execution
The project lead (interactive Fable session) decides sequence, queue, controller
and model changes and commits accepted work; see AGENTS.md. Queue workers and
reviewers stay on Claude Sonnet 5.5/high. Moving review and research to Opus is
intended but not yet implemented in the controller. Original-game execution is
approved for interactive sessions only; queue workers stay static-only and
oracle-001 stays blocked for the queue.
