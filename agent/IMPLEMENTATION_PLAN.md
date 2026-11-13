# Current reconstruction roadmap

Updated 2026-11-12 by the project lead after the first human playtest. Current
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
From the playtest ledger; item numbers are its P-numbers.
1. **basher-fix-001** (queued): Basher usable at a wall on MM_BASH_002 (P1).
2. **level-sweep-001** (queued): all-level smoke sweep for crashes, hangs and
   blank renders. Its ranked list feeds later ordering; it does not preempt 3-5.
3. **terrain-render-001** (queued): terrain removed or added by skills is visible
   (P2, P3), for Bomber, Builder, Basher, Miner and Digger.
4. Per-state lemming animation and the bomber countdown (P4-P6), then smooth
   movement between simulation steps (P7).
5. Lemmings hidden behind the cylinder (P8).
6. Small fixes: one window across frontend and level (P12), Quit (P13),
   fast-forward off (P17, after checking the running original).
7. Hatch doors, drop-in spawn, balloon departure (P9-P11).
8. Map level names (P14), remaining sound cues and music (P15, P16), renderer
   speed (P18).
After each of 3, 4 and 6 the user playtests ten to fifteen minutes.

## Acceptance for playable v1
- Every level has been won by a person or, where the user accepts it, by a
  scripted public-input route, from the ordinary frontend with persistence; or
  has an explicit user-accepted exclusion.
- Every skill and gadget type has visible, readable behavior: state animation,
  terrain change, gadget effect.
- No known crash, hang or soft-lock on any installed level.
- No open playtest item marked blocker.
- Full registered main/recovery verification passes on the final content.
- Reports keep human observation, scripted clone observation, source inference
  and original comparison distinct, and restate no historical gate as passed.

Progress today: scripted wins exist on 5 levels (map nodes 0-3 and 8); by the bar
above none is yet confirmed playable. One playtest, 18 open items.

## Verification
Measured 2026-11-12 on commit 721c64e (build/test-parallel-benchmark-001/):

| Configuration | Main suite wall time | Result |
| --- | ---: | --- |
| Debug, 1 test job | 2583 s | 147/148; the failure was a concurrent human playtest writing the normal save |
| Debug, 4 test jobs | 1183 s and 1212 s | 148/148 twice; `levelreturn` 321 s and 351 s of a 360 s timeout |
| -O2 with assertions, 4 test jobs | 329 s | 148/148; `levelreturn` 66 s |

Deterministic outputs were byte-identical across the debug runs. The verifier now
builds the main suite with `-O2 -g` (assertions kept, `NDEBUG` not defined) and runs
four test jobs; `--test-jobs 1` remains the fallback. The recovery suite keeps its
plain Debug build. Tests, assertions and timeouts are unchanged. Do not run the
clone against the normal save while verification is running: several play tests
assert that the normal save is untouched.

Process work stops here. Preflight and verification reuse are dropped as goals; a
six-minute suite does not need them.

Known waste, not yet addressed: playableexpansion steps 2 and 3 (Blocker, Builder)
end in a watchdog timeout in every run and pass as driver-integrity tests.

## Execution
The project lead (interactive Fable session) decides sequence, queue, controller
and model changes and commits accepted work; see AGENTS.md. Queue workers and
reviewers stay on Claude Sonnet 5.5/high. Moving review and research to Opus is
intended but not yet implemented in the controller. Original-game execution is
approved for interactive sessions only; queue workers stay static-only and
oracle-001 stays blocked for the queue.
