# Plan history — Phase D

Moved verbatim from `PLAN.md` on 2026-10-24 (user, overlord review 2026-10-24). Only link
paths were rebased. The live milestone table, exit gates, status cells and user decisions
are in [the plan](../../PLAN.md#phase-d--reconnect-the-simulation). New round notes are
appended under the milestone they serve, newest last.

## Phase D — reconnect the simulation

Wire spawn, exits, skill assignment and the measured gadgets into the new renderer, and get a
retail level playable end to end in 3D. Starts as soon as A6 passes; it runs alongside A7
and comes before B and C.

**Milestones and exit gates (user sign-off 2026-10-21, before any D round).** Every gate is
scored against the original's actor list at `0x004CBB18`
(`measure_steppers.py --compare` is the per-tick oracle), never against our own sim. Do not
change a gate after seeing a result; a needed change stops the standing run.

| Milestone | Status | Exit gate |
| --- | --- | --- |
| D1 release and walk-out | **parked in Phase E (2026-10-22, user decision); not accepted** | `MM_CLIMB_002`, no input, from level start: for 150 ticks (30 s), every lemming's cell X/Y, facing and state match the original each tick, and the released count and order match |
| D2 click-to-assign | **(a) accepted**; (b) 3 of 4 (climber/blocker/floater), digger parked | (a) For 20 recorded cursor positions, the in-level pick (AABB against `lemming+0x128`, cursor doubles at `0x4CBFB0`+0x16A0/+0x16A8) picks the same lemming as the original, or none when it picks none. (b) Climber, floater, blocker and digger each assigned once: the 50 ticks after each assignment match the actor list exactly, and the skill counts decrement the same way |
| D3 one retail level end to end | **parked in Phase E (2026-10-22, user decision); not accepted** | `MM_CLIMB_002` playable in `webolution` (3D view, lemmings, click to assign). One scripted solution (timed assignments) gives the same saved count and the same end time in whole seconds as the original, and reaches a level-end state. The A6 and A7 frame gates still pass |

Notes for D1: our sim has a known one-tick divergence on a left-facing turn (`0x0041C67E`,
LESSONS "The simulation"). D1 cannot pass without fixing it, and the blocker-turn
expectation moves with it. The release cadence and the exit boxes are already measured
(`measure_release.py`, `measure_exit.py`).

**D1a first-divergence localization** ([check](../scratch/phase-d1-release-walkout-trace.md)):
a new offline sim-trace tool (`src/tools/SimReleaseTrace.cpp`) and a new live no-input
capture of `MM_CLIMB_002` (one stock start, 30 s / 150 ticks off the actor list) found the
gate's **first** disagreement at tick 1, in the released count, not the previously-known
left-turn tick — our sim's `World` releases its first lemming unconditionally on tick 1
(`sinceSpawn_ = releaseIntervalTicks` at construction), while the original visibly waits
`t = 2.020 s` (~10.1 ticks, neither tick 1 nor a full measured interval) before its first
release, then releases every `2.887 s` (14.433 ticks) thereafter — a non-integer gap no
`int releaseIntervalTicks` can track exactly. The left-turn divergence was never reached.
No behavioral fix was made (out of this round's scope); the original's actual
release-trigger mechanism (what determines the first delay, and whether the gap is
level-specific or derives from `RELEASERATE.PC` through a mapping this project has not
found — `LevelInfo` carries no release-rate index) is unread. D1 is not accepted.

**D1b release-trigger mechanism, read** ([check](../scratch/phase-d1-release-trigger.md)):
a per-hatch real-time countdown (`hatch+0xC0`) drives every release, compared against
`0.0` at `0x00425A95`. It is seeded once to a fixed `1.3 s` (`0x004B0F48`) the moment a
door sub-object (`hatch+0x44`) finishes opening, then every later release *adds back* the
per-level interval at `hatch+0xB8` (a rollover, not a reset) — two different numbers
sharing one field, not one shared threshold with only a different start time. `hatch+0xB8`
is resolved at load (`0x00425DE3`) by clamping a per-hatch rate (`hatch+0xA0`, already
present but unnamed in `findHatchRecords`'s own file-format comment, alongside `+0xA4`
min / `+0xA8` max) into a 99-row table loaded at runtime from `Compiled\ReleaseRate.pc`
(`0x0042609D`) — exactly `src/assets/GameData.h`'s `ReleaseRates`, not yet wired to a
per-level selector. On `MM_CLIMB_002` (rate 20, clamped to `[1,50]`), that row is
`2.8864866275534937 s`, matching D1a's measured mean gap (`2.8867 s`) to `0.0002 s`. The
first release's own `~0.72 s` residual (`1.3 s` seed vs. D1a's measured `2.020 s`) is left
to the door-opening animation's own duration, not yet measured. D1 is still not accepted.

**D1c door-open duration, derived** ([check](../scratch/phase-d1-door-open-duration.md)):
a live, read-only capture of the hatch object (located by vtable `0x004B0F50`) missed the
literal `+0xAC` arming flip (object-location overhead ate the `~0.72 s` window), but
directly measured the countdown's rollover landing in `(2.020, 2.030]` s — confirming
D1a's first release from a second, independent field — and a linear fit of the
pre-rollover countdown (slope `-0.997`, residual `<=0.006 s`) derives `arm_time ~= 0.723 s`;
`arm_time + 1.3 s ~= 2.023 s` matches D1a's `2.020 s` to `~3 ms`. D1's tick-1
release-timing divergence is now closed to a few milliseconds. Open, not blocking: whether
`0.723 s` is a fixed universal delay or itself a per-level animation length, and whether
the hatch's countdown is genuinely continuous (render-frame-driven, consecutive samples
step unevenly at roughly 144 Hz rather than in 200 ms chunks) or only reads that way —
flagged, not settled. D1 is still not accepted; next is a static read of the hatch step's
own call cadence, then an implementation round against this now-measured mechanism.

**D1d hatch step clock, read** ([check](../scratch/phase-d1-hatch-step-clock.md)):
`0x00425A95` is reached only through virtual dispatch (vtable `0x004B0F50`+`0x14`), via a
generic per-object step dispatcher (`0x004423D2`) called from exactly three sites inside
`0x0045CFF9` — an unconditional per-pass "leftover" call carrying real wall-clock time, a
queue-drain call, and a paused fallback with a fixed `dt=0.0`. `0x0045CFF9` itself has
exactly one caller, once per rendered frame. **Verdict: the release countdown is
frame-driven, the same continuous-accumulator shape as the project's already-documented
`+0x1CC` animation clock, not the lemming stepper's discrete 200 ms tick** — explaining
D1c's uneven per-sample deltas as the real mechanism. The `dt` getter
(`[hatch+0x44]+0x48`) narrows to firing only once per hatch (the arm transition), not on
the recurring decrement; its concrete target, and whether D1c's `0.723 s` generalizes
beyond `MM_CLIMB_002`, are unresolved by static reading (an independent REA/Ghidra pass
hit the identical wall). D1 is still not accepted; next is an implementation round against
this now-fully-measured shape (continuous real-seconds accumulator, not an integer tick
count), using the general, confidently-known constants (`1.3 s` seed,
`RELEASERATE.PC`-derived interval) and leaving the unresolved `arm_time` generalization as
an explicit open parameter.

**D1e release timing, implemented** ([check](../scratch/phase-d1-release-timing-implementation.md)):
wired additively into `World` (new optional `LevelSetup` fields; every existing
`releaseIntervalTicks`-based setup is byte-for-byte unaffected). D1a's own unmodified
`diff_trace.py`, against D1a's own unmodified live capture and a freshly regenerated sim
trace using the measured constants, now first diverges at **tick 10** (an `existence`
alignment artifact — sim has no row yet where live already shows the lemming existing one
tick early) instead of **tick 1**; spawn-count transition ticks agree at every shared
point, drifting by 0-4 ticks from discretizing the original's continuous accumulator onto
our 200 ms grid. The already-known left-turn divergence (`0x0041C67E`) was never reached
in this comparison and remains the other open D1 blocker. D1 is still not accepted.

**D1f left-turn fix, implemented** ([check](../scratch/phase-d1-left-turn-fix.md)):
`World::stepWalker` now takes its post-turn step within the same tick when the walker
*was* facing Left (not, as this round's own brief mis-stated, when it turns *to* face
Left — corrected and reported by the round itself before any test literal changed).
Offline `compare_tick` reproduces round 10's own cited numbers exactly, fixed and
reverted; one live start matched 40/40 ticks with no regression, though it did not
itself freshly cross the specific left-wall scenario within the printed window (flagged
plainly, substituted with round 10's own already-landed live measurement). One test
literal changed as a direct, justified consequence (`testBlockerTurnsWalkers`'s second
sub-case, `131`→`132`); the first sub-case and `testWallTurn` are unaffected. **Both
named D1 blockers are now fixed.** D1 is still not accepted — a fresh, comprehensive live
re-check of the full gate (150 ticks, no input, every lemming) is the next D1 round.

**D1g full gate re-check, both fixes confirmed, one residual found**
([check](../scratch/phase-d1-full-gate-recheck.md)): a fresh live capture (new stock
start) confirms the tick-10 alignment artifact still appears first, and — checked for the
first time — lemming 0's wall turn at column 64 matches exactly (though that turn
exercises the already-good "right-facing" case, not D1f's new branch). Scanning the full
150-tick window past tick 10 found 167 more disagreements: 161 are D1e's already-known
spawn-tick drift propagating forward, and 6 are a new, smaller, self-healing single-tick
`facing`-only mismatch at every "walker was facing Left, turns Right" event — exactly
D1f's own target scenario, though `x` matches exactly before and after and an independent
full scan found zero reappearance of the stall D1f fixed. Whether this is a genuine sim
gap or an artifact of the harness's single global-phase alignment (D1a's own open
limitation: individual lemmings have stable but different phases) is unresolved. D1 is
still not accepted; the next D1 round is this one narrow, fully offline question — no new
live capture needed.

**D1h per-lemming-phase check, mixed result** ([check](../scratch/phase-d1-full-gate-recheck.md#d1h-follow-up-the-per-lemming-phase-hypothesis-mixed-result-2026-10-21)):
re-aligning each flagged lemming with its own measured phase (not the harness's one
pooled global phase) flips 3 of the 6 residuals to match exactly; the other 3 each have a
distinct, fully-characterized reason they don't (zero residual, a 0.01 s near-miss bounded
by raw-sample spacing, a wrong-signed residual at the phase method's own wraparound
boundary). **Correction surfaced in the same round, confirmed independently**: only 3 of
the 6 flagged ticks are the D1f-fixed "turns Right" scenario; the other 3 are the
already-good "turns Left" case, misattributed in D1g's own text (now corrected there too).
Every one of the 6 now has a complete, understood explanation, and an independent
full-trace scan found zero reappearance of the stall D1f fixed. **Both named D1 blockers
remain fixed and verified; the only open item is this one small, well-understood,
self-healing single-tick `facing`-label residual**, which traces to the live-capture
harness's alignment precision (not the simulation), not conclusively a sim defect. See
Phase E for the parking decision.

**D1 direction (user, 2026-10-21) and facilitator observation:** the D1 gate stays as
written. The facing-label question is deferred until the spawn drift is fixed. The drift is
not 200 ms discretization. In both saved captures the `released` counter steps every
2.88–2.90 s, while lemmings enter the actor list only on a ~1.02 s cycle
(`2.04 + k * 1.02` s within one 0.02 s sample in D1g; ~1.013 s in D1a). `World` spawns on
the counter. This is an observation from the saved meta files; the mechanism is D1i's
question.

**D1i actor-list delay explained: harness artifact, not an exe mechanism**
([check](../scratch/phase-d1-release-to-actor.md)): the ~1.02 s cycle above does not
exist in the game. The Lemming constructor (`0x0042DE51`) publishes the new pointer into
the `0x004CBB18` actor list strictly before it increments the `RELEASED` counter at
`0x004CBA78`, a few instructions later inside the same synchronous call — no clock read, no
animation-frame check, on the path (`0x00425A95`→`0x0042DE51`→`0x004230A2`→`0x004233A0`,
addresses and bytes read and independently re-read). The ~1.02 s period is
`tools/harness/measure_d1_release_trace.py`'s own `t - last_enum > 1.0` re-enumeration
throttle landing on its `1/hz = 0.02 s` sampling grid; an offline model replaying that
throttle against each capture's own real per-pass timestamps reproduces all twenty saved
`first_seen_seconds` values exactly (independently reproduced from scratch, plus a
discriminating no-throttle control that fails as expected). `released_count` in the D1
gate means the counter, which the exe read shows is the same quantity as the actor-list
size at any instant in the original — they diverge only in this project's own measurement.
**New, unactioned finding**: `measure_d1_release_trace.py::build_table()` builds its
per-tick `released`/`existence` columns from this same throttled `first_seen`, so D1e/D1g's
161-tick spawn-drift residual may be partly or wholly this harness bias rather than a sim
timing defect — open for D1j, not yet checked.

**D1j harness throttle fixed; mixed, precisely-characterized result**
([check](../scratch/phase-d1-harness-throttle-fix.md)): a landed fix (targeted tail
discovery off the already-unthrottled `RELEASED` read, not a full re-scan) eliminates the
throttle's detection lag outright — `first_seen` now matches the counter to
floating-point precision for all 10 lemmings in a fresh capture, independently confirmed.
Re-running D1a's/D1g's own unmodified comparators against this fix is a **mixed** result,
not a clean "fixed": the raw divergence count did **not** shrink (167 → 537 naturally,
275 even after correcting for a second, independent harness bug found as a byproduct —
below), but the metric the fix actually targets, each lemming's own spawn-tick drift
magnitude, **did** collapse, from up to 4 ticks (9/10 lemmings) down to at most 1 tick
(8/10 lemmings, consistent with the structural floor of discretizing a continuous release
onto a 200 ms tick, not with the throttle explaining nothing). The raw count does not track
drift magnitude once forward-propagation is accounted for (one tick of drift at spawn
produces one `x`-mismatch row per remaining tick). The already-parked 6-entry `facing`
family (D1g/D1h) is unchanged, as predicted. **D1 is still not accepted.**

A second, previously unknown harness bug was found as a byproduct, not fixed this round:
`build_table()`'s `n_min` (how many tick-grid points precede the capture's own `t=0`) is a
step function of the inferred per-capture `phase` landing on one side of zero or the other,
so two otherwise-equivalent captures' tick labels can differ by a full 200 ms — confirmed
by relabeling this round's own capture one tick earlier and reproducing D1g's exact
tick-10 first-divergence shape. This is queued next as D1k: fix the phase/`n_min` boundary,
then re-run a clean, uncorrected full-gate comparison. No new live capture is needed —
both D1g's and D1j's raw samples are already saved.

**D1k phase/n_min boundary fixed; D1g and D1j now agree on first divergence**
([check](../scratch/phase-d1-phase-nmin-fix.md)): `n_min` (which tick-grid point is
"tick 1") was a sign test on a per-capture `phase` value that is always within half a
tick of exact zero by construction, with zero margin at the test's own boundary.
Replacing it with a nearest-grid-point choice (`round(-phase/TICK_SECONDS)`, provably
always `0` under the current `phase` definition, independently confirmed over 200,000
sampled values) makes D1g's and D1j's captures agree exactly on first divergence (tick
11, `released_count`, sim=1 live=0) where they previously disagreed in both tick and
field. **D1g's own historical figures change as a direct, flagged consequence**: its
167-total/tick-10 result becomes 653-total/tick-11 under the corrected grid — the old
number measured a mis-aligned grid, not a better one. The raw-count gap between the two
captures (653 vs 537) persists even with aligned grids, consistent with D1j's
already-landed finding that count tracks propagation length, not drift magnitude; this
is not re-litigated. **D1 is still not accepted.** Both known harness biases (D1i/D1j's
throttle lag, D1k's grid alignment) are now fixed; the next D1 round is a fresh, clean,
full live gate recheck against the fully-fixed harness.

**D1l: first full gate measurement with both harness biases fixed — does not pass, every
residual named**
([check](../scratch/phase-d1-full-gate-recheck-fixed-harness.md)): a fresh live
capture reproduces the predicted tick 11/`released_count` first divergence and the
≤1-tick spawn-drift floor exactly (3/10 lemmings, magnitude 1). **New finding**: the
faller→walker landing tick drifts −1 or −2 ticks for **all 10 lemmings**, including the 6
with zero spawn drift — a second, previously-unisolated discretization-floor mechanism
(a continuous landing event quantized onto the 200 ms tick), not the same quantity as
spawn-tick drift though structurally analogous. Of 657 total divergences in the 150-tick
window, every one is attributed: 14 to the already-parked `facing`/turn-rounding family
(Phase E), 643 to forward propagation from spawn-tick drift (3/10 lemmings) or
landing-tick drift (6/10 lemmings) — **0 unexplained**. Read against the gate's literal
text, every named field (X, Y, facing, state) and the released-count clause shows at
least one disagreement; order does not. **D1 does not pass, and every residual is now a
named, bounded, deterministic discretization-floor or already-parked quantity — none is
an open, untraced defect.**

This is recorded as a pending question for the user (`docs/tasks.json`'s
`standing_run.pending_questions`), not decided here: whether to amend D1's gate to
exclude this now-fully-characterized discretization-floor class of residual (the same
shape of amendment already granted to A7 for HUD elements), to invest in closing it
outright (which would mean re-deriving the sim's spawn/landing timing at sub-tick
precision or changing the tick architecture — not a bounded round), or to leave D1
parked. The standing run stops here per
[its historical gate-change stop condition](../../old/docs/agent-framework/ORCHESTRATION.md#standing-run).

**D1 direction after D1l (user, 2026-10-21, CEO review): gate held as written, not amended.**
Review observation, a hypothesis and not a measurement: D1l's own raw 50 Hz samples look
continuous, not stepped. X changes on 7224 of 7464 consecutive 20 ms samples. The walk-out
fits `32 + 5.0·(t − land)` within −0.022..+0.115 over 2684 samples. The fall at 20.0 u/s from
Y 42 lands at release + 0.50 s for 10/10 lemmings. On its spawn tick our `World` has already
fallen one full 4-unit step. Re-run with `python build/tmp/ceo-2026-10-21/continuous_fit.py`.
If the original integrates real `dt` per frame, as D1d already found for the hatch, then
"discretization floor" is the wrong name. The residual is a model mismatch, and the fix is
the sim's clock, not the gate. Neither the "5 Hz, measured three ways" lesson nor any D1
round has tested a fixed step against a continuous one: the speeds are 5.0 and 20.0 per
second, and a fixed 0.2 s step turns them into 1 and 4. **D1m** reads the lemming step's
`dt` source and replays the saved captures with a zero-parameter continuous model. If the
answer is continuous, D1n changes `World`'s clock. If it is stepped, the gate question goes
back to the user with D1m's evidence.

**D1m: continuous, on both a structural read and a behavioural replay**
([check](../scratch/phase-d1-stepper-clock.md)): the walk stepper (`0x0041BDD4`) takes
its time budget as a mutable, re-checked, byref double that it can spend across more than
one lattice column per call and writes the decremented remainder back through the same
pointer — a shape with no reason to exist under a fixed 0.2 s step, independent of any
live measurement. A zero-fitted-parameter continuous replay (release, fall 20.0 u/s, land,
walk 5.0 u/s, no wall-turn logic) against all three saved captures (D1l, D1j, D1g)
collapses the `y`-field mismatch count 76–100% (29→4, 28→0, 10→1) — the landing-tick-drift
floor D1l isolated as its own mechanism nearly disappears once fall/land timing is modelled
continuously — while the full-150-tick total is honestly worse (the model was deliberately
given no wall-turn logic, as scoped). The re-examined "5 Hz, measured three ways" lesson
does not survive: two of the three rest on the retracted billboard-quad method and the
third measured the hatch, not the lemming (full correction in
[LESSONS.md](../LESSONS.md#the-simulation--measured-and-worth-keeping)). Open, disclosed
gap: the exact caller chain from the frame dispatcher to the Lemming's own stepper was not
traced within budget; the verdict rests on the stepper's own loop shape, not a completed
chain. **D1 is still not accepted.** Per the pre-authorized branch, the next round is
**D1n**: a bounded `World`/`SimReleaseTrace` clock change, re-scored offline against all
four saved captures, before any live recheck — not a re-ask of the discretization-floor
gate question, since a confirmed model mismatch calls for fixing the model.

**D1n: implemented, partial result — landing-tick drift reduced, not closed; spawn-tick
drift unchanged, now structurally explained** ([check](../scratch/phase-d1-continuous-stepper-clock.md)):
an additive, opt-in continuous sub-tick clock (`LevelSetup::continuousStepperClock`,
default `false`, byte-for-byte old behaviour when off — confirmed by `diff -q` against each
capture's own checked-in `sim_trace.csv`) lets a landing faller hand its unconsumed
real-seconds budget to the walker it becomes within the same tick. The landing-tick-drift
family D1l isolated **shrinks substantially on D1g/D1j/D1l**: roughly half of the
previously-drifting lemmings now land on the exact tick, the rest from magnitude up to 3 to
at most 1 (aggregate totals more than halve: 653→303, 537→183, 657→302). **D1a gets
nominally worse** (162→514) — explained, not papered over: D1a's own live capture predates
the already-landed D1i/D1j harness-throttle-lag fix, so it is the one capture whose own
ground truth is independently known to be mistimed; a more temporally faithful sim
naturally diverges more from a mistimed reference. Spawn-tick drift is **unchanged on every
capture**, for a now-understood structural reason: `spawn()` fixes which tick a lemming
exists on before the budget it receives is even computed, so no fall-budget reduction can
move it. A real bug in the round's own first draft (an unguarded landing handoff that
regressed D1a before the `hasTrackedPhase` guard was added) is disclosed in the check, not
hidden. 27/27 tests pass (4 new, 0 edited); the facilitator's own mutate-one-alone (a
one-line leftover-budget off-by-one) was not caught by the unit suite but was caught by the
offline capture comparator (252 of 1500 rows diverge), confirming that comparator — not the
unit suite alone — is the real discriminator for this class of change. **D1 is still not
accepted.** Whether this partial, honestly-measured improvement changes the calculus on the
user's pending D1-gate-amendment decision (`docs/tasks.json`
`standing_run.deferred_questions`) is for a future round or the user to weigh — not decided
here.

**Addendum from D2h (2026-10-22), cross-referenced not re-litigated**: a disassembly-
confirmed move-then-check fix to `stepFaller` (found while investigating D2 part (b)'s
floater skill, [check](../scratch/phase-d2h-skill-assign-floater.md)) was re-checked
against D1a's/D1g's/D1j's/D1l's own saved captures and produced byte-for-byte identical
traces to D1n's pre-fix baselines on all four — no measurable effect on the
landing-tick-drift family this section's own gate-amendment question is about. The two
mechanisms share a family (same-tick landing-handoff timing) but are not the same
occurrence of it: the fix's trigger condition (gap-to-ground exactly `fallSpeed + 1` at
a tick's start) does not occur for any of D1's own captured lemmings' drop heights. This
rules out one specific, plausible-looking candidate explanation; it does not change
D1n's own partial-improvement result or the pending decision below.

**D1o: one fresh live recheck, then parked (user decision, 2026-10-22, CEO review)**
([check](../scratch/phase-d1-continuous-live-recheck.md)): a new no-input live capture of
`MM_CLIMB_002` (150 ticks), scored with the continuous-stepper-clock flag both off and on.
Fails the gate literally under both settings (657 and 305 total divergences). The flag-on
improvement reproduces on fresh data (not an artifact of D1n's three saved captures), with
two new, source-traced qualifications: this capture's landing-tick-drift improvement (8/10
still nonzero) is weaker than any of D1n's three, which is why `y` does not collapse at all
here even though `x` and `facing` do; and the `x`-closure is binary per lemming, tied to
whether a lemming's own landing drift happens to equal its spawn drift, not a uniform
shrink. Per the user's decision, **D1 is now parked in Phase E** at this result — measured,
not tuned, and not re-opened by this round. Work proceeds to D3, then Phase B/C.

### D2. Click-to-assign

**D2a: the pick rule read and ported, round 1** ([check](../scratch/phase-d2-pick-rule.md)):
`src/app/Pick.{h,cpp}` ports the original's in-level cursor-to-lemming pick
(`0x00454D33`/`0x00454CB9`/`0x00455F20`/`0x0045A0E0` chain, `0x00454D6C`'s full
accept/skip/tie-break body). **Falsifies the brief's own hypothesis**: the tie-break is
**priority (desc, from a per-state category table) then squared distance (asc, strict)**,
not list order — list order only breaks an exact tie on both keys, keeping the earlier
node. All four box edges are inclusive (`<=`/`>=` with the cursor's own ±4px adjustment
already folded in), confirmed by re-disassembling `0x00454CB9` directly, not by trusting
the cited old note's prose. Two skip gates not in the brief's "already settled" list were
found and excluded from the ported function with reasons: a facing-filter UI pair
(`screen+0x16b1`/`+0x16b2`) and the skill-stock validity gate (`0x0045A0E0`) — both depend
on runtime UI/skill state, not lemming-vs-cursor geometry, so D2(a)'s own geometry gate
does not model them; the brief explicitly named the skill-stock gate as something to
report, not omit silently. Two upstream widget-hit-test loops (lists `0x4C88F8`/`0x4C8908`)
can suppress the pick before the lemming loop even runs if a UI panel is hit first —
flagged for the D2(a) live capture to avoid. Two genuine gaps, reported not guessed: the
priority-category table's contents (`0x004C05D0`) and what `screen+0x58` holds (the
distance reference point) are both runtime facts this round's static reads cannot settle.

**D2b: 20 live cursor positions, round 2** ([check](../scratch/phase-d2-pick-capture.md)):
pid 39764 reached `Compiled\MM_CLIMB_002.lvl` after the highlight scan found one
candidate. Twenty live sprite-box positions give 19 agreements with
`[screen+0x16D8]` and 1 disagreement. Sample 15 overlaps two lemmings, both
state vtable `0x004b0d28`, both passed to the port with priority 0 and distance
0; the port keeps index 2 and the original stores index 6. Eleven positions
pick none and nine name a lemming. An earlier start, pid 47392, never left the
main menu and took no rows. D2 part (a) does not pass. The priority table and
`screen+0x58` remain the open tie keys. Part (b) was not run.

**D2c: tie-key operands, round 1** ([check](../scratch/phase-d2-tie-keys.md)):
the priority load at `0x004550CC` indexes `0x004C05D0` with `[state+0x14]`,
and `state` is `[lemming+0x340]`, not the vtable. The distance reference is
the three doubles at `screen+0x58`, `+0x60`, and `+0x68`, subtracted from
`lemming+8`. The walker constructor writes index `0x12` beside vtable
`0x004B0D28`, and that file dword is 0. Equal vtables are not forced to share
a priority. Sample 15's winner is still open, because the live field and the
positions were not saved. D2 part (a) does not pass.

**D2d: one live overlap with the real tie keys, round 1**
([check](../scratch/phase-d2-tie-live.md)): one cursor on `MM_CLIMB_002`
overlaps two walkers. Both have `[state+0x14] = 0x12` and priority 0, live and
in the file. The reference at `screen+0x58` is `(32.0, 48.0, 0.5)`. Squared
distance selects index 3 (`698.64`) over index 2 (`1070.03`), and index 3 is
the lemming at `[screen+0x16D8]`. D2 part (a) still does not pass: this is one
overlap, not the 20 positions, and it reads only table index `0x12`.

**D2e: 20-position rescore attempt, clean negative — the stock exe crashed before the
scan, round 1 of 3** ([check](../scratch/phase-d2-rescore-crash.md)): a new
`measure_d2_rescore.py` feeds every eligible candidate's real live priority and real
squared distance (never a forced 0) into the landed `pickHoveredLemming` through a
driver built from an unedited HEAD snapshot of `Pick.{h,cpp}`. The one authorized live
start (pid 23616) reached the main menu, then died with an access violation
(`0xC0000005`) while the harness was acquiring foreground for the first menu `Down` —
before any key was sent, before the level was entered. No retry, per the brief's
"no second Popen." No 20-position data was collected; D2 part (a) remains exactly where
D2d left it. Restoration confirmed (process gone, all three install hashes unchanged,
independently re-verified).

**D2e repair 1: second crash, earlier in the sequence, round 2 of 3**
([check](../scratch/phase-d2-rescore-crash.md)): same unmodified script, same offline
self-tests reproduced exactly. The one authorized live start crashed again with the same
`0xC0000005` exit code, this time *before* the harness ever attached to the process
(during the fixed boot wait), earlier than round 1's crash (which reached the main menu
and died during foreground-acquisition polling). No data collected; restoration again
clean. A facilitator-only, read-only event-log/WER pass found no crash report for either
specific incident, a long pre-existing history of this exe crashing on this machine in
unrelated prior sessions, and the host cycling logon/sandbox-restart events roughly
every 40-70 seconds through both crash windows -- an unproven environmental correlation,
not a diagnosed cause. Per the round cap, the worker correctly did not self-dispatch a
third attempt. D2 part (a) still does not pass.

**D2e repair 2: third attempt reaches the level; 20/20 agree; D2 part (a) accepted**
([check](../scratch/phase-d2-rescore-crash.md)): same unmodified script, round 3 of 3
(the question's last pre-authorized round). No third crash -- the one authorized start
attached immediately, entered `MM_CLIMB_002` on the first try, and filled all 20 rows
with zero discards: 10 original-none, 4 two-overlap rows, **20/20 agreement** between
the landed `pickHoveredLemming` (fed live priority and live squared distance) and the
original's own stored pick. The facilitator independently reproduced this from scratch
with a standalone checker that imports nothing from the project's own harness or ported
code, empirically re-deriving the exact overlap formula (the cursor box inset 4px on
every edge, then an inclusive AABB test) before it matched all 20 rows exactly -- not
merely re-running the worker's own driver. This also caught and narrowed an error in the
round's own report: a nonzero live priority dword (`2`, at `state+0x14 = 19`, matching
the file table exactly) **was** read this round, on three candidates that simply never
overlapped a sampled cursor box -- the first nonzero live priority value seen in the
entire D2 investigation, though the priority-beats-distance tie-break branch remains
untested live. **D2 part (a) is accepted.** D2 part (b) is untouched and remains open.

**D2f: one live climber assignment, 45/50 ticks match, one bounded residual found**
([check](../scratch/phase-d2-skill-assign-capture.md)): the first round of D2 part
(b), climber only. Reused already-proven tooling throughout (`assign_skill.py`'s panel
click, `measure_d1_release_trace.py`'s sampling) for a live climber assignment on
`MM_CLIMB_002`, then compared the assigned lemming's 50-tick post-assignment trace
against a fresh `sim::World` seeded at the live-measured cell via the already-existing
`World::place()`. **45 of 50 ticks match exactly** (the full walk-approach and the full
14-tick climb run, X/Y/facing/state all exact), independently reproduced by the
facilitator from a from-scratch recompile and relink, byte-identical output. The one
divergence: our `World`'s climb stepper stays in `State::Climber` one tick longer than
the original at the topout cell before handing off to `State::Hoister` — both reach the
same cell at the same tick, but only the state label lags by one tick, which then
propagates as a constant one-tick shift for the remaining captured rows (hoist duration
and post-hoist walk speed are both still exactly right). `climbSpeed`/`climberTopOut`
are independently reconfirmed live, unaffected. D2 part (b) is not yet settled (one of
four skills attempted; this skill has an open residual). Queued next: a disassembly
read of the native climb/hoist handoff ordering, offline-only (no new live capture
needed — replays round 1's own saved trace), to confirm and fix the one-tick lag.

**D2f repair 1: the one-tick lag read with bytes and fixed; climber now 50/50**
([check](../scratch/phase-d2-skill-assign-capture.md)): offline only, zero stock
starts. Disassembly of `0x004137CA` (the climb stepper's outer loop) and `0x004139BF`
(the check function it calls) shows the native stepper moves the lemming's Y
(`0x0041388B`, the write-back) **before** calling the check function, which reads that
already-moved position at entry (`0x4139DD`→`0x42F35F`) for both the ceiling-hit and
topout probes, and installs the Hoist state (`0x417C80`/`0x42E696`) inside that same
call — move, check, and transition all happen in one call, same tick. `World.cpp`'s
`stepClimber` is fixed to match: move first, then check. A new unit test
(`testClimbTopsOutSameTickAsHoistHandoff`) fails against the old ordering and passes
after; one existing test's literal (`lastClimbingY`, `41`→`42`) changed as a named,
explained consequence, not a silent edit. Re-checked against round 1's own unmodified
live capture, offline: **50 of 50 ticks now match exactly** (was 45/50).
**Correction, beyond this round's own question**: this also settles the ceiling-hit
check's timing as post-move, correcting `old/docs/scratch/phase3-round10-steppers.md`'s
"tests the ceiling first" characterization — that note did not have this call-graph
split in view. Climber is now a full match; D2 part (b) overall remains open (floater,
blocker, digger untested).

**D2g: one live blocker assignment, 50/50 ticks match on the first attempt**
([check](../scratch/phase-d2g-skill-assign-blocker.md)): second of four D2(b) skills,
reusing D2f's exact pattern. On `MM_BLOCK_001_A`, panel index 3, one live assignment
holds the sampled lemming frozen at `(137, 14, 1, blocker)` for all 50 ticks, matching
an offline `sim::World` seeded via `World::place()` + `assign(0, Skill::Blocker)`
exactly, no repair needed. `tools/harness/measure_d2b_skill_trace.py` gained
`--skill`/`--skill-name` options (purely additive; climber's prior default behaviour is
unchanged). Blocker is a full match; floater and digger remain untested.

**D2h: one live floater assignment, 11/50 ticks match -- a second landing-tick-drift
instance, flagged to D1** ([check](../scratch/phase-d2h-skill-assign-floater.md)):
fourth D2(b) skill. Live-reconfirms the already-known parasol quirk (the original never
makes floater a `state` change, only a permanent attribute flip at `[lemming+0x49]`;
our sim already models this the same way). The sampled lemming's two falls (8 cells
each) never reached `floatAfter`, so `State::Floater` itself was not exercised, but
every tick's X/Y/facing/state was still compared. **11 of 50 ticks match**; the
residual is `stepFaller` labelling `faller` for one tick after actually reaching the
landing cell, the same shape of bug D2f's repair 1 found and fixed in `stepClimber` --
but here in a sibling stepper, not yet confirmed against the original's bytes.
Independently ruled out (confirmed twice, including by the facilitator): this is not
the already-known D1n continuous-clock gap, since a directly-placed lemming never
carries a tracked sub-tick phase.

**Flagged, not decided: this residual looks like the same mechanism as D1's own
"landing-tick drift"** ([D1l](../scratch/phase-d1-full-gate-recheck-fixed-harness.md),
[D1n](../scratch/phase-d1-continuous-stepper-clock.md)) -- the fall-to-walker
transition D1l found drifting on 10/10 lemmings population-wide, which D1n's
continuous-clock fix shrank but did not close. If a native-bytes-confirmed reorder (the
same technique that fully closed climber's residual) also closes this one, it may bear
on D1's own pending gate-amendment question (`standing_run.deferred_questions`) --
reported here as a lead, not acted on; D1's gate stays exactly where the user left it.
A disassembly-read repair round is queued, re-checking any fix against D1's own already-
saved captures (no new live start) as well as this round's floater capture.

**D2h repair 1: fixed with a byte-confirmed read; floater now 50/50; zero effect on D1,
a clean negative** ([check](../scratch/phase-d2h-skill-assign-floater.md)): offline
only, zero stock starts (one attempt was interrupted by a session rate limit mid-round,
after the disassembly read but before any code change; a continuation worker completed
it from the already-saved reads, nothing redone). Disassembly of `0x004160bd` (the fall
stepper's outer per-sub-unit loop) and `0x00416433` (the land/floater/splat check it
calls) shows the native moves each sub-unit's Y **before** calling the check, which
reads the already-moved position — the same move-then-check architecture as climber's
fix, nested inside a loop that runs up to `fallSpeed` (4) times per tick.
`World.cpp`'s `stepFaller` is fixed to match (move unconditionally, every sub-unit,
before the landing checks), with a checked invariant (not an assumption) showing the
unconditional move can never walk a lemming through solid ground. Re-checked against
D2h's own unmodified live capture: **50 of 50 ticks now match** (was 11/50).
**Also re-checked, as scoped, against D1's own already-saved population captures
(D1a/D1g/D1j/D1l, zero new live starts): all four regenerated traces are byte-for-byte
identical to their pre-fix baselines** — this fix has no measurable effect on D1's
landing-tick-drift family. The disassembly explains why: this fix's trigger condition
(a fall's gap-to-ground exactly `fallSpeed + 1` at a tick's start) does not occur for
any of the ten lemmings' actual drop heights in any of the four captures. This is a new,
independently-confirmed data point for D1's own pending gate-amendment question (see
Phase E below) — it does not resolve that question. Climber, blocker and floater are now
full matches; digger remains the last untested D2(b) skill.

**D2i: one live digger assignment, 16/50 ticks match -- a new residual shape, not yet
fixed** ([check](../scratch/phase-d2i-skill-assign-digger.md)): fourth and last D2(b)
skill. The assigned lemming dug three rows before falling and walking -- longer and
more informative than the cited ~271ms instant-refusal quirk, which this capture did
not reproduce. Each dig "bite" lands one tick late in our sim, self-healing each time
(the same shape D2f's/D2h's fixes already addressed, at a different transition); the
digger-to-faller handoff then compounds into a larger gap during the fall, settling
into a constant one-tick/one-column lag for the rest of the walk. **Hypothesized, not
yet disassembly-confirmed**: `stepDigger` (and `stepBuilder`/`stepBasher`/`stepMiner`,
sharing the same `bitesThisTick` helper) lack the `+1` compensation
`stepHoister`/`stepExploder` already apply for the same postfix `++l.frame` placement --
a real asymmetry in the code, confirmed by direct read, but not proven to be *the*
cause (builder already has its own separate enter-hook timing compensation that digger
may or may not need equivalently). Correctly not fixed this round, given the risk to
three other already-measured steppers sharing the same helper. D2 part (b) is not
complete: climber/blocker/floater are full matches, digger is not. A disassembly round
is queued, scoped to digger's own call site only.

**D2i repair 1: the `+1` hypothesis corrected, not confirmed; no fix applied; digger
parked** ([check](../scratch/phase-d2i-skill-assign-digger.md)): offline only, zero
stock starts. Disassembly of `0x0041B077` (the shared bash/dig/mine stepper, confirmed
byte-identical across all three vtables) shows it is **not** a discrete tick counter at
all: it is a continuous, real-valued percent-of-cycle clock (`lemming+0x128`'s own
`0x00468B04`/`0x00468B5D` accessors) that fires at most one scheduled record per call
and hands the unspent time budget back by reference; the generic per-lemming dispatch
loop (`0x0042E2BF`) re-invokes the same stepper with that leftover until exhausted --
the real mechanism behind two records firing in one tick, not a tick-granularity
coincidence. The controller's own clock does not reset at state entry (confirmed: no
caller found from digger's enter hook to the one zeroing function located in a full
`.text` scan), corroborated by round 1's own saved live data (first bite at ~130-140ms,
well under a from-zero calculation's ~300ms). **Round 1's `+1` hypothesis is corrected,
not confirmed** -- there is no discrete counter to add one to. No fix was applied;
`World.h`/`World.cpp`/`test_world.cpp` are untouched. Closing this properly needs a
persistent cross-state percent clock plus a generic same-tick leftover-budget
re-invocation dispatch -- a materially larger change than one bounded round. **Digger
is parked in Phase E** (see below) rather than spending a third round on an attempt
already known to need more than a bounded fix. D2 part (b) is accepted as 3-of-4: climber,
blocker and floater are full matches; digger is the one open, parked residual.

### D3. One retail level end to end

**D3a: production retail-level loader, round 1** ([check](../scratch/phase-d3-retail-level-setup.md)):
`webo::sim::loadRetailLevel`/`loadRetailLevelReport` (`src/sim/RetailLevel.{h,cpp}`) build
a `LevelSetup` for any retail level from the archive alone — terrain, entrances, release
timing, exits/teleports/speed-pads/levers, totals and skills, each from an already-tested
reader. **All 102 retail levels load**, matching a prediction written before coding. D1b's
open "no retail rate falls outside its clamp" is now checked and holds (0 of 144 hatch
records across all 102 levels). `SimReleaseTrace` rebuilt on the new loader is
byte-identical to its pre-change output on D1g's own invocation (sha256
`7653aedb4108d06aa0ad8026a70df645fe60273638db358638f2368ee7dde664`, both before and after).
Named loss, not hidden: 33 of 102 levels have more than one hatch, but `LevelSetup` carries
one shared release-timing triple — every hatch's own position still becomes an `Entrance`,
and every hatch's own rate/min/max is still read and reported, but only the first hatch's
cadence drives the shared accumulator. `releaseArmSeconds` (`0.723061695938398`) stays the
MM_CLIMB_002-only DERIVED constant for every level, per the brief's own instruction, not
re-derived elsewhere. Deliberately no `old/src`-style guessed fallback for a hatch-less or
exit-less level — none exists in the retail corpus, so this path is untested because it
never fires, not because it was masked.

**Facilitator check on D2a and D3a (2026-10-21):** independently re-read the cited
addresses against the installed exe — `0x00454CB9` (the whole box-overlap-test function,
byte-for-byte match) and `0x004551E4`-`0x00455203` (the priority/distance tie-break,
byte-for-byte match). Independently confirmed the new hatch tail-word offsets
(`src/assets/LevelObjects.cpp`'s `kTailAt = kTripleAt + 6`, i.e. hatch+0x9A+6 = +0xA0,
+0xA4, +0xA8) match D1b's already-landed offsets exactly. Ran one isolated
mutate-one-alone test per round, each rebuilt and re-tested in the full tree (both rounds
landed into the same working copy): swapping `RetailLevel.cpp`'s entrance `e.x`/`e.y`
assignment is caught by `test_retaillevel`'s MM_CLIMB_002 check (`FAIL  entrance at
hatchSpawnCell (32, 53) (got (53, 32))`); loosening `Pick.cpp`'s left-edge comparison from
`<=` to `<` is caught by `test_pick`'s exact-boundary-hit case (`FAIL: left edge:
cursor.right-4 == lemming.left is still a hit`). Both files restored to their
worker-delivered SHA-256 and a fresh full rebuild plus `ctest --preset dev-debug` passes
**26/26** (both `retaillevel` and `pick` included), confirming the two concurrently
dispatched rounds' shared `CMakeLists.txt`/`cmake/webolution_sources.cmake` edits merged
without conflict, exactly as their dispatch note anticipated. No Lemmings Revolution
process ran for either round. D3b (app sim-loop integration, depends on D3a) and D1n
(World clock change) do not share an owned file and may now run concurrently.

**D3b: `sim::World` ticking inside `webolution`, round 1** ([check](../scratch/phase-d3-app-sim-loop.md)):
`webolution --sim`/`--sim-ticks <N>` (off by default) builds a `World` via D3a's
`loadRetailLevel` and draws its live lemmings through the existing
`drawInjectedWalkers`/`terrainSampleFromLemming` path (`TubeScene::injectedWalkersFromWorld`,
one entry per `alive()` lemming). A new `SimClock` (`src/app/SimClock.*`) drives the
windowed loop's ticking off real time, independent of the 144 Hz camera clock, capped at 10
ticks (2.0 s) per frame with any backlog carried, not dropped. Every pre-existing command
line renders byte-identically (A6 and A7 PNG hashes unchanged); `--sim --sim-ticks N` at
N = 0, 15, 75 matches an independent `SimReleaseTrace` run on the same level exactly. Named
limitation, reported as its own prediction anticipated: `drawInjectedWalkers` only has a
`"walk"` sprite action, so non-`Walker` alive states (10 of 181 sampled live-lemming-ticks
were `Faller`) reuse that same strip at a fixed phase — no sprite invented, no animation
clock wired to `World::tick()`. Ran concurrently with D1n (file-disjoint); its own checks
were relative (before/after the same build), not against a frozen `World.cpp`. 27/27 tests
pass. D3c (click-to-assign in the app) depends on this round and D2a and may now be queued.

**D3c: click-to-assign wired offline, round 1** ([check](../scratch/phase-d3-click-to-assign.md)):
`TubeScene::injectedWalkerScreenBoxes` exposes the sprite box `drawInjectedWalkers`
already computes, and `test_clicktoassign` selects lemming 0 from that box and
selects nothing from the same box shifted +10000 in x. Tick 25 on `MM_CLIMB_002`
has two alive lemmings. The windowed `--sim` path calls `World::assign` on a
left click, with keys 1–8 as a debug skill stand-in, not the original panel.
That window path was not executed. A6 and A7 PNG hashes are unchanged and
`--sim-ticks` 15 and 75 stderr are unchanged. 28/28 tests pass. This does not
grade D2's or D3's gate. The live 20-cursor comparison remains D2b.

**D3d: one scripted solution for `MM_CLIMB_002`, with the original's own saved count
and end time** ([check](../scratch/phase-d3-solution-discovery.md)): oracle-gathering,
no `src/**` change. The solution: assign climber (panel 0) to every lemming in arrival
order as each becomes hoverable, reusing the exact click mechanism D2(b)'s rounds
already proved. One live run (1 of 2 authorized stock starts) reaches a clean win:
**saved 10 of 10** (`SAVED_WORD` `0x004CBAA4`) against a save target of 1
(`0x004CBAB0`), **end time 208 whole seconds** (`time_limit` 300 at `0x004CBAA8` minus
the level clock's 92 at `0x004CBAAC` the instant `ui_state` left in-level play).
Honestly left open: whether `ui_state 4` (this run's own end state) and `ui_state 5`
(an unrelated earlier round's results-screen reading, under an artificially-raised
save target) are the same state reached at different moments or two different states;
whether fewer than 10 climbers would also win (this round matched the level's own
apparent "give everyone the one skill it ships" design, not a minimal solution, per
the brief's "one solution" framing). This is the oracle the next D3 round's own
`sim::World` run is compared against.

**D3e: our own `World` matches the saved count exactly; the end time does not, by
roughly half** ([check](../scratch/phase-d3-sim-solution-run.md)): entirely offline,
zero stock starts. A round-local probe loads `MM_CLIMB_002` through the unmodified
production loader and assigns climber to every lemming on its very first active tick
(matching `World::assign`'s own `Skill::Climber` branch, which has no `State::Walker`
requirement, confirmed by direct read). Result: **saved 10 of 10, matching the
original exactly** — but **finishes at tick 514 (102-103 s), roughly half the
original's 208 s.** Every one of the ten lemmings takes exactly 373 ticks from spawn
to save with zero variance, because instant assignment means every lemming takes the
shortest possible identical path (no lemming-vs-lemming collision exists in this
engine). **Named, falsifiable candidate cause, not yet confirmed**: D3d's own live
capture shows the original's assignment process was not instantaneous (~95 s of
real/game time to land all ten clicks) — this probe's instant-at-spawn rule cannot
reproduce that delay by construction. D1's own already-landed per-event residuals are
explicitly ruled out as far too small to explain a gap this size. D3's gate does not
pass: the saved-count half is met, the end-time half is not.

**D3e repair 1: a structured click-pacing model gets close, but no integer value hits
208 s exactly** ([check](../scratch/phase-d3-sim-solution-run.md)): offline only,
zero stock starts. Swept two assignment-delay models in a parametrized probe:
`uniform` (independent per-lemming delay) and `serial` (one globally-paced clicking
process). Both are step functions, not linear (a 1-tick delay change can shift the
finish time by 112 ticks — an extra walk-turn-walk lap). Neither model's integer sweep
lands on exactly 1040 ticks (208 s): `uniform` jumps 994→1106 between `D=513`/`514`;
`serial` jumps 1011→1106 between `CI=71`/`72`, with `CI=77-78` giving 1074 (+6.8 s).
**`uniform`'s closest value is implausible** (would require the very first lemming to
wait 100+ seconds, contradicted by the oracle's own log). **`serial`'s closest value
(14.2-15.6 s/click) is far more plausible** — structurally consistent with one person
serializing ten clicks, and numerically the same order of magnitude as an
independently-computed 111 s total-loop-duration figure (`clock_at_entry 298` minus
the first watch sample's `clock 187`, read directly from D3d's own saved log — a
correction to D3d's own round-note, whose cited "~0,3,6,9,12,15s" estimate turns out
not to be literally present in the raw log it summarized). D1's own per-event
residuals remain ruled out. D3's gate, as literally written ("the same end time in
whole seconds"), does not pass on this close-but-not-exact result; recorded as a
pending question for the user below, not decided unilaterally.

**D3f: recorded real assignment times, replayed exactly; then parked (user decision,
2026-10-22, CEO review)** ([check](../scratch/phase-d3-timed-solution.md)): per the user's
decision not to amend the gate and not to run a third fitted click-model round, this round
recorded each assignment's real landing time in one live run and replayed `World` with
exactly those times — no fitting or sweeping. Unanticipated, directly-measured finding: two
live runs of the identical scripted solution ended 75 real seconds apart (208 s vs 133 s),
confirming by direct measurement, not a model sweep, that the gate's own premise (a
scripted solution has *a* reproducible end time) does not hold for the original itself. On
the self-consistent comparison this round can actually run — this run's own recorded times
fed back through `World`, compared to that same run's own result — saved count matches
exactly (10/10) and `World::finished()` matches the run's own last save to the whole second
exactly (125 s vs 125 s), well inside D1's own already-parked few-tick budget. Against the
gate's own literal `end_time_seconds` field (133 s, the `ui_state` transition, not the last
save), the residual is -8 s: -6.3 s of it is an already-named, now twice-independently-
measured last-save-to-`ui_state`-transition delay `World` does not model (6.345 s here vs
D3d's 6.4 s, on two absolute timescales 75 s apart); the remainder (-1.7 s whole-second /
-0.53 s continuous) is left unattributed beyond D1's budget, not assumed into either
bucket. **D3 does not pass literally and is now parked in Phase E** at this result — the
same posture D1 reached after D1o, for the same reason (the question's authorized rounds
are spent and the user's decision forecloses both a gate amendment and a further round).
