# Pending level-end transition and latched results (2026-10-28)

Offline follow-up to [results/map return](level-results-map-return.md). No stock
process/desktop/registry/save access. Static evidence supports the transition;
SOURCE tests verify its integration, not native timing or movie acceptance.

## Native count and producer closure

The new packet is `build/tmp/enacter-end-transition/findings.md` with aligned
raw assembly, producer chains, evidence IDs and `verify.py`. Main independently
reran its35 static/contract checks. The retained portable checker
`tools/harness/read_level_results.py` now checks24 raw anchors and3 doubles.

445436 counts entries in the retained update/interaction list4C88D0 whose
actor+4C is **not3**. +4C is the sheet type: table4BFFC8 names type3 `EvilRat`.
It is not a dead-state marker. Hatch425A95 creates the actor via42DE51,
whose base constructor registers that same pointer in the authoritative
registry4CBB18, then successful update inserts it into4C88D0. Loaded actors
and rats have separately traced producer paths. This does not supersede the
requirement to use4CBB18 for live actor reads.

Actor update false removes list membership before the end gate; completed save
animation increments saved and returns removal status in that update. Pending
death/save animations still count. **Do not use `activeCount()`**, which excludes
SOURCE Splatter before its animation finishes.

Controller+5098 is selected-type **unreleased hatch population**, refreshed by
444ABC from per-type remaining counts. Hatch load adds its remaining word;
release decrements it. Nuke has a separate override. For ordinary supported
SOURCE populations, the adapter uses alive non-rat actors plus
`setup.totalLemmings - spawned()`. Selected-type subsets, nuke, rat populations
and general preplaced-actor accounting remain outside this correspondence.
Synthetic tests isolate the rat/death membership predicates without claiming
complete native preplaced-actor loading.

## Implemented transition

`LevelEndTransition` is a small explicit state machine, porting444BC2/445498:

1. Ordinary load/retry starts Running, timer**7.0**, win=false, pending=false.
2. After each admitted simulation slice, timeout takes precedence; otherwise
   retained non-rat count plus signed selected unreleased must equal **exactly0**.
   Reaching the rescue target alone does not trigger an end.
3. A trigger latches `saved >= needed`, enters Pending, and returns **without
   subtracting that call's dt** or reseeding the timer.
4. Pending subtracts simulation dt. Strict **<3.0** controls effect bookkeeping;
   strict **<1.0** publishes Results, clears effect and resets the stored timer7.
   Exactly3/1 retain the preceding boundary behavior.
5. Results snapshot uses **latched success** and **current final-call counts**.
   The adapter consumes it once, then stops simulation; retry creates fresh
   transition/clock state.

This is not a configurable six-second wall-clock delay. SOURCE runs the
transition after its normal0.2s World ticks; native uses event-sliced simulation
milliseconds with normal frame caps and speed-update repetitions. Floating-point
rounding, dt partition, pause and speed affect the strict crossing. Existing
simulation timing discrepancies are not closed by retaining the native timer.

Native simulation, level clock and hatches continue during Pending. The runtime
now does the same; pause freezes both World and countdown, discarding accumulated
paused time as before. `World::tick(bool continueHatchesAfterTimeout=false)` keeps
the legacy diagnostic release behavior by default. Connected play explicitly
opts into native425A95's lack of a timeout/pending hatch guard, so pending hatches
can release rather than being incorrectly frozen when the clock reaches0.

**Preserved quirk:** late rescues may make final saved>=needed, but a previously
latched timeout loss stays a loss. `FrontendSession` no longer rejects such a
snapshot as inconsistent, and main no longer derives success from final
`world.won()`. Results can therefore truthfully display a failed title with
counts that meet the target; completion flags still follow the latched outcome.

No invented visual fade was added. Effect draw helper490ED6 is empty;490EE1's
normalization has no renderer/state side effect. AVI playback is a separate
outer-dispatch stage after mode3/4, still unimplemented. Bookkeeping an effect
flag is not evidence for painting a black overlay.

## Checks and sensitivity

- `levelend`: literal contract cases, including large trigger dt, target-only
  non-trigger, exact signed sum versus negative sum, timeout priority, exact3/1,
  current counts/needed with latched win/loss, immutable snapshot and fresh reset.
- `runtimehud`: pending continues simulation past timeout, pause freezes it,
  result publication freezes further ticks; an actual SOURCE actor rescues after
  timeout and produces saved1/needed1/**loss**; pending hatches release while the
  default diagnostic World does not; live sheet3 is excluded and a pending
  Splatter remains counted despite `activeCount()==0`.
- `results`: accepts late-rescue loss without unlocking progress.
- `levelreturn`: actual SDL dummy application observes Pending for both natural
  rescue and timeout runs before Results, then preserves its map-return checks.

Initial test-only mistakes were caught and corrected: a synthetic Faller was
placed without its required floor gap; a helper stopped at exactly1 while
expecting Results (and then dereferenced an empty optional). The corrected
helper crosses below1 and guards snapshot access; no production threshold was
weakened to satisfy it.

Isolated mutations fail: decrement on the trigger call; use <=1; recompute win
from final counts; suppress post-timeout hatch releases; count active rather
than retained dying actors. Hash restoration uses the packet's
`pre-mutation.sha256`. Baseline tests are recorded separately from the final
full-suite run.

Restored full build/CTest: **66/66 PASS**,178.04seconds; `levelend`0.03s,
`runtimehud`0.14s, actual-app `levelreturn`60.48s. Log is
`build/tmp/enacter-end-transition/full-suite.log`. No new native timing or image
acceptance is implied by these SOURCE passes.

Open: exact native event-sliced timing and animation/removal timing, all
population-selection/nuke cases, same-host-frame accelerated dispatch, physical
input during Pending, movies, best-score/save persistence and retry fades.
