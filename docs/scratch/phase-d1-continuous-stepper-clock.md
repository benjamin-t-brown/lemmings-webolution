# D1n: an opt-in continuous sub-tick clock for the fall/walk steppers

Queue id `d1n-world-continuous-clock`. Round 1 of 3. Offline only: build, `ctest`, and a
re-score against the four already-saved live captures (D1a, D1g [D1k-fixed], D1j, D1l). No
live game, no debugger, no stock start. Full worker evidence:
`build/tmp/d1n-world-continuous-clock/{prediction.md,round-note.md,landing_tick_drift_tables.txt}`.

## Question

Does an additive, opt-in continuous sub-tick time budget for the fall/walk steppers
(following D1e's exact shape: default off, old behaviour byte-for-byte when off) close or
reduce the landing-tick-drift and spawn-tick-drift families D1a/D1e/D1l found?

## Answer: landing-tick drift closes substantially on 3 of 4 captures; spawn-tick drift does not close, for a now-understood structural reason

`LevelSetup::continuousStepperClock` (new, default `false`) and `Lemming::stepBudgetSeconds`
(new, default `0.0`) let `stepFaller`/`stepWalker` consume a real-seconds budget instead of
a fixed per-tick count, and hand any unconsumed remainder to the next stepper **within the
same `World::tick()` call** on landing or on D1f's already-existing same-tick turn
continuations. Off, the loops are byte-for-byte the old fixed-count loops — confirmed by
`diff -q` between a flag-off trace and each capture's own already-checked-in
`sim_trace.csv` (D1g, D1j, D1l: no output; D1a's own historical file predates D3a's
unrelated `SimReleaseTrace` rewiring, confirmed not a regression via `git log`).

- **Landing-tick drift** (per-lemming: sim's own first `walker` row minus live's):
  | capture | before (nonzero/max\|drift\|) | after |
  |---|---|---|
  | D1g | 10/10, max 3 | 5/10, max 3 |
  | D1j | 10/10, max 2 | 3/10, max **1** |
  | D1l | 10/10, max 2 | 5/10, max **1** |
  | D1a | 4/10, max 1 | 6/10, max 1 (gets nominally worse — see below) |

  Aggregate totals (same taxonomy as `full_divergence_report.txt`): D1g 653→303, D1j
  537→183, D1l 657→302 — more than halved, led by `x` (the landing-tick-drift-propagation
  family D1l already named). D1a's total nearly triples (162→514), explained, not hidden:
  D1a's own live capture predates D1i's/D1j's already-landed harness-throttle-lag fix, so
  it is the one capture of the four whose own ground truth is independently known to carry
  a timing defect — a more temporally faithful sim naturally diverges more from a
  mistimed reference, and the old model's small drift against D1a was very plausibly
  coincidental cancellation against that same harness bug, not evidence of a better model.
- **Spawn-tick drift**: unchanged, to the exact integer, on all four captures. Structural,
  not a tuning miss: `World::tick()` calls `spawn()` — which inserts the lemming, the only
  thing that makes it "exist" for the comparator — unconditionally once the release
  countdown crosses zero; a reduced first-fall budget changes how far it falls that tick,
  not which tick `spawn()` itself ran on (that tick is already fixed before `spawn()` is
  even called). The round's own prediction named "not at all" as a non-falsifying outcome.

## A bug the round found in its own first draft, and the fix

The first implementation handed a landing faller's leftover budget to the walker
unconditionally. The first offline re-score showed a regression against D1a (162→514,
driven by `x`) even though the same change improved D1g/D1j/D1l. Traced, not reverted
blindly: for an *ordinary* fall with no continuous-release involvement, the loop's own
sub-step index at landing is a deterministic function of level geometry (how far the fall
distance is from a multiple of `fallSpeed`), not of real elapsed time — handing that
number to the walker as leftover real time invents a same-tick walk column nothing in D1m
measured. Added a `hasTrackedPhase` guard (only hand off when the call's own *entering*
budget already carries a real, traced sub-tick offset). Re-scored: identical totals on
every capture to the unguarded version (MM_CLIMB_002's own 150-tick window never exercises
a second mid-walk re-fall), so the guard is currently inert but scopes the mechanism
correctly for a level that does re-fall mid-walk.

## Tests

`testContinuousStepperClock` (new, `test_world.cpp`, 4 checks, unconditional — no
`WEBOLUTION_GAME_DIR` needed). No existing test edited. `ctest`: 27/27 (3 of those —
`simclock`/`pick`/`camerainput` — belong to the concurrently-landed D2a/D3b rounds, file-
disjoint from this one). `test_world.exe` directly: 378/0 with `WEBOLUTION_GAME_DIR` set
(374 + 4 new), 338/0 without.

## Facilitator check (2026-10-21)

Independently reproduced regression safety: ran `SimReleaseTrace` myself with D1g's own
flag-off invocation and diffed against `build/tmp/d1g-full-gate-recheck/sim_trace.csv` —
`diff -q` produced no output (byte-identical), confirming the opt-in is inert when off.

Independently reproduced the aggregate D1g after-score with fresh invocations (not re-using
the worker's saved CSVs): ran `SimReleaseTrace --continuous-stepper-clock` myself with
D1g's own invocation, then ran D1g's own already-landed `diff_trace_all.py` against it —
reproduced **303 divergence(s)** exactly, with 252 `x` and 10 `y` field-level counts exactly
matching the round's claimed after-table.

Independently re-derived the landing-tick-drift table for D1l from scratch (fresh Python,
not importing the round's or any prior round's comparator code — just parsing both CSVs
directly and taking each lemming's first `state == walker` row): reproduced the claimed
10/10→5/10, max 2→max 1 result exactly, cell for cell.

**Mutate-one-alone**: changed the fall-landing leftover computation
(`budget - i * unitSeconds` → `budget - (i + 1) * unitSeconds`, one line, `stepFaller`),
rebuilt. The existing `ctest -R world` did **not** catch this (its 4 new checks don't probe
this exact arithmetic) — expected, and exactly why the offline capture comparator is the
real discriminator here, not the unit suite alone. Regenerated the D1l flag-on trace under
the mutation: 252 of 150-tick×10-lemming rows diverge from the unmutated trace (an X-column
shift from the landing tick onward on every affected lemming), confirming the comparator
discriminates this one-line change behaviourally. Restored `World.cpp` to the
worker-delivered SHA-256 (`d65fcff2...`), rebuilt, full suite **27/27** again.

No Lemmings Revolution process was started by this round or by this check (fully offline,
confirmed both before and after). `git status` showed only the brief's owned files plus
this task's two permitted `docs/tasks.json` fields.

**Landed.** D1 is **not accepted** — landing-tick drift is reduced, not eliminated, on
D1g/D1j/D1l (5/10, 3/10, 5/10 still nonzero, each capped at magnitude 1), spawn-tick drift
is unchanged, and D1a's own result is a known, explained regression against a
harness-defective reference, not a sim defect. The already-parked `facing`-label family
(Phase E) is untouched. Per the brief's round-cap accounting this is round 1 of 3 on the
"continuous clock" implementation question. Next: either a second D1n-family round
targeting the remaining nonzero landing-tick-drift lemmings (if a cheap lead exists) or a
fresh live gate recheck to see whether this partial improvement changes the user's pending
D1-gate-amendment decision (`docs/tasks.json` `standing_run.deferred_questions`) — the
facilitator's call, not decided here.
