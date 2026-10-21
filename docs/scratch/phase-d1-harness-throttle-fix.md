# D1j: the harness throttle fix, and what it did and did not explain

Queue id `d1j-harness-throttle-fix`. Round 1 of 3 on this question, following D1i
(`docs/scratch/phase-d1-release-to-actor.md`). One live, no-input capture of
`MM_CLIMB_002` (150 ticks/30 s), the one authorized stock start.

## The fix

`tools/harness/measure_d1_release_trace.py`'s `capture()` had one throttle:
re-enumerating the actor-list vector (`live = S.lemmings(h)`) was gated on
`t - last_enum > 1.0`, so a newly-released lemming could go unsampled — not just
under-recorded, entirely un-sampled — for up to ~1.02 s past its true appearance
(D1i: there is no such gate in the original).

**Landed fix**: a new `discover_new_actors(h, scanned)` does a targeted tail read of
only the actor-list's newly-appended raw slots (the array is append-only, per D1i's
exe read of `0x00423146`/`0x004233A0`), triggered off the already-unthrottled
`RELEASED` counter read `capture()` performs every pass regardless — zero marginal
reads on no-op passes. `scanned` is tracked as the number of raw array slots examined
(not `len(order)`, the number of *accepted* lemmings — MM_CLIMB_002's actor list holds
two non-lemming-vtable slots ahead of the lemmings, found live and handled in a
same-round refinement, verified offline only since the one live budget was already
spent). A 5 s, non-authoritative full-rescan backstop remains, logged but not trusted,
in case the delta trigger ever misses something (it did not: `backstop_missed` count
0 this round).

**Live result, the fix's own target metric**: `first_seen` now matches the
unthrottled `RELEASED`-poll timestamp to floating-point precision for all 10
lemmings — zero lag, down from up to ~1.02 s. Zero duplicate pointers, exactly 10
distinct lemmings tracked.

## Re-running the unmodified comparator: a mixed result, reported precisely

D1a's `diff_trace.py` and D1g's `diff_trace_all.py` were re-run unedited against a
freshly regenerated sim trace (confirmed byte-identical to D1g's own) and against this
round's new capture.

| | before (D1g, throttled) | after, as produced | after, phase-corrected |
|---|---|---|---|
| total divergences | 167 | 537 | 275 |
| first divergence | tick 10, `existence` | tick 11, `released_count` | — |

**Taken literally, the residual count got worse, not better.** Investigation found a
second, previously unknown, unrelated harness fragility (below) dominates the raw
"after" comparison; even correcting for it, the raw count is still larger (275 vs 167).

**But the metric the throttle fix was actually about — each lemming's own spawn-tick
drift magnitude, not the raw divergence count — did collapse**, independently
recomputed from the sim trace and both live traces (`load_sim`/`load_live`, each
lemming's own first row tick):

| | before (D1g) | after (phase-corrected) |
|---|---|---|
| drift magnitudes | 0 to 4 ticks (9/10 nonzero) | 0 or 1 tick only (8/10 nonzero, 2/10 exact) |
| sum \|drift\| | 18 | 8 |

Max drift collapsed from 4 ticks to 1 — consistent with hitting the structural ≤1-tick
floor of comparing a continuous release instant against a discrete 200 ms sim tick
(the same family as D1e's already-documented discretization), not with the throttle
explaining nothing. The raw divergence *count* does not track drift *magnitude*: once
any lemming's spawn is off by even one tick, every later tick of its walk shows a
one-column `x` mismatch for the rest of the window — the count scales with ticks
remaining after a nonzero drift, not with the drift's size. So "the residual collapses"
(true, by drift magnitude) and "the divergence count collapses" (false, by raw count)
are different claims; only the first is supported here.

The separate, already-documented 6-entry `facing`-only family (D1g/D1h, Phase E)
reproduced exactly, unchanged, as predicted — it is not produced by this throttle.

## A second, unrelated harness fragility, found as a byproduct (not fixed this round)

`measure_d1_release_trace.py`'s `build_table()` decides the tick grid's `n_min` (how
many grid points sit before the capture's own `t=0`) from the inferred `phase` via
`while phase + n_min*TICK_SECONDS < -1e-9: n_min += 1` — a step function. D1g's
capture inferred `phase = -0.02` (giving `n_min = 1`); this round's capture inferred
`phase ~= +1e-7` (giving `n_min = 0`) — both phases are close to zero in the circular
sense `infer_phase`'s own median fit targets, but landing on opposite sides of zero
shifts every tick's nominal label by a full 200 ms. A diagnostic reconstruction (not a
code change, not a second live run) relabeling this round's own raw samples one tick
earlier reproduces D1g's exact tick-10/`existence` first-divergence shape and is a
clean, verified one-tick relabeling of the natural result (every row checked matches
exactly). This is a pre-existing fragility in phase/`n_min` inference, not introduced
by the throttle fix and not in this round's scope (round-cap: this is D1j's own
question, answered; the phase/`n_min` fragility is a new, separate question).

## What this does and does not settle

- **Settled**: the throttle bug was real, is fixed, and is independently confirmed to
  zero detection lag. The per-lemming spawn-tick drift magnitude it explained collapsed
  to the structural ≤1-tick quantization floor.
- **Not settled by this round, and not the throttle fix's fault**: the raw divergence
  count D1g used to size "161" did not shrink, because that metric is not proportional
  to drift magnitude once propagation is accounted for, and because a second,
  independent harness bug (phase/`n_min` boundary sensitivity) currently makes an
  honest before/after count comparison unreliable without a manual correction.
- **Open, queued next (D1k)**: fix the phase/`n_min` boundary fragility so future
  captures' tick-grid labeling does not depend on which side of zero a near-zero
  circular phase estimate happens to land, then re-run a clean, uncorrected D1 full-gate
  comparison to see what (if anything) remains once both known harness biases are
  removed.

## Facilitator check

2026-10-21: independently re-derived the delivered `live_trace.meta.json`'s
`first_seen`/`released_events` match (diff `0.0` for all 10 lemmings) and confirmed
exactly 10 distinct pointers with zero duplicates from `live_raw.jsonl` directly, not
from the worker's own summary. Re-ran `offline_discovery_test.py` myself: both parts
pass, confirming the delta-discovery mechanism and its same-round refinement (tracking
raw-scanned-slot count, not accepted-lemming count) examine every raw slot exactly
once with zero re-reads. Confirmed `sim_trace.csv` byte-identical to D1g's own via
`diff`. Independently re-ran D1a's own unmodified `diff_trace.py` against the new
capture: reproduced "tick 11, `released_count`, sim=1 live=0" exactly. Independently
re-ran D1g's own unmodified `diff_trace_all.py` three times (against D1g's old
capture, this round's new capture, and the delivered phase-shifted diagnostic CSV):
reproduced the declared totals 167 / 537 / 275 exactly via each report's own printed
summary line (an initial naive line-count by this facilitator against the full report
text overcounted, having not excluded the reports' own "coverage gaps" and "zoom
window" sections — corrected by reading each report's own stated total instead).

One correction to the round's own narrative: the round-note describes the phase-shift
diagnostic as "`phase` shifted by `+0.2`", but `build_table`'s own `n_min` formula
needs a *negative*-going phase change to raise `n_min`, not a positive one (confirmed
by calling the landed `build_table()` directly with `phase + 0.2`, which left `n_min`
at `0`, unchanged). The delivered artifact itself is not affected by this — independently
regenerating the natural and shifted CSVs from this round's own `live_raw.jsonl` and
diffing row-by-row confirms the delivered `live_trace_shift_test.csv` is exactly the
natural capture's own grid relabeled one tick earlier (every one of 749 checked rows
matches `natural[T+1] == shifted[T]` exactly) — a sound, honest diagnostic, just
described with an inverted sign in the prose. Noted here, not escalated; it does not
change any reported number.

Independently recomputed the per-lemming spawn-tick-drift table directly from
`sim_trace.csv`, D1g's `live_trace.csv`, and this round's `live_trace_shift_test.csv`
with fresh code (not importing the worker's own helpers): magnitudes match exactly —
before 0/1/1/2/2/4/4/0/1/2, after 1/0/1/1/1/1/1/1/0/1 (opposite sign convention,
identical magnitudes) — confirming the "max drift 4 to 1" headline independently.

Confirmed via PowerShell `Get-Process`/`Get-ScheduledTask` that no Lemmings Revolution
process or `LemWeb-*` scheduled task is running; exe SHA-256 matches the authority
(`32675bdb...`). `git status --short` shows only this task's owned
`tools/harness/measure_d1_release_trace.py` plus `docs/tasks.json`'s permitted fields —
no `src/**` or other `docs/**` file touched. Fresh `ctest --preset dev-debug`: 24/24
(no source changed this round; run for due diligence since this round's sim trace
depended on the existing `SimReleaseTrace.exe` build).
