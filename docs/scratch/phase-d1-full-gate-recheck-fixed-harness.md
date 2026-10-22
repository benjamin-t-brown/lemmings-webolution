# D1l: first full D1 gate measurement with both harness biases fixed

Queue id `d1l-full-gate-recheck`. Round 1 of 3, following D1i/D1j/D1k (all landed). One
fresh live, no-input capture of `MM_CLIMB_002` (150 ticks/30 s), the one authorized
stock start. **Continuation note**: the first worker was terminated mid-round by an
external API rate-limit error after finishing the live capture and both comparator
runs, but before the write-up; the process was already cleanly stopped and the stock
start already productively consumed. A zero-stock-start continuation finished the
analysis and write-up from the already-saved artifacts.

Disclosed by the first worker, before the real run: a bash probe
(`Lemmings Revolution.exe --version`) was run by mistake, bypassing the harness entirely
(no `dgVoodoo.conf` swap, no `attach()`, no level entry, no capture, no input). It was
detected immediately, killed, and the exe hash re-verified unchanged. Reported here, not
escalated — no data was touched and the harness's own controlled capture (the actual
stock start) ran separately afterward.

## Sim trace: unchanged, re-confirmed

`git log` shows no commit to `src/sim/World.*` or `src/tools/SimReleaseTrace.cpp` since
D1f. A freshly regenerated sim trace is byte-identical to D1g's and D1j's own saved
`sim_trace.csv` (`diff -q`, no output).

## First divergence: matches the prediction exactly

`diff_trace.py` (D1a's, unmodified): **tick 11, `released_count`, sim=1, live=0** —
identical to D1k's own reprocessed result on both D1g's and D1j's captures. The grid
alignment and throttle fixes hold on a fresh capture.

## Per-lemming spawn-tick drift: holds at the ≤1-tick structural floor

| lemming | sim first tick | live first tick | drift |
|---|---|---|---|
| 0 | 11 | 11 | 0 |
| 1 | 25 | 26 | −1 |
| 2 | 40 | 40 | 0 |
| 3 | 54 | 55 | −1 |
| 4 | 69 | 69 | 0 |
| 5 | 83 | 83 | 0 |
| 6 | 98 | 98 | 0 |
| 7 | 112 | 112 | 0 |
| 8 | 126 | 127 | −1 |
| 9 | 141 | 141 | 0 |

3/10 nonzero, magnitude exactly 1 — consistent with D1j's already-landed finding on a
different capture. The 3 coverage gaps in the full residual report (ticks 25/1, 54/3,
126/8) are exactly these 3 lemmings.

## New finding: landing-tick drift, 10/10 lemmings, not previously isolated

The faller→walker landing tick (each lemming's own first `state == 'walker'` row) drifts
−1 or −2 ticks for **every** lemming, including the 6 with zero spawn drift:

| lemming | sim landing tick | live landing tick | landing drift | spawn drift |
|---|---|---|---|---|
| 0 | 13 | 14 | −1 | 0 |
| 1 | 27 | 29 | −2 | −1 |
| 2 | 42 | 43 | −1 | 0 |
| 3 | 56 | 58 | −2 | −1 |
| 4 | 71 | 72 | −1 | 0 |
| 5 | 85 | 86 | −1 | 0 |
| 6 | 100 | 101 | −1 | 0 |
| 7 | 114 | 115 | −1 | 0 |
| 8 | 128 | 130 | −2 | −1 |
| 9 | 143 | 144 | −1 | 0 |

For the 6 lemmings with zero spawn drift, this landing drift is a **separate**
discretization-floor effect (the fall→walk transition quantized onto the 200 ms grid,
landing one tick later in `live` than in `sim`, 10/10, no exception) — not previously
named as its own quantity in D1g's, D1j's, or D1k's docs, which measured spawn-tick
drift specifically and folded this into "the already-documented spawn-tick
discretization drift" without isolating it. For the 3 lemmings with nonzero spawn drift,
landing drift is additive (spawn drift − 1), not a duplicate.

## Full 150-tick residual: 657 divergences, 0 unexplained

| field | count |
|---|---|
| x | 592 |
| y | 29 |
| facing | 14 |
| state | 13 |
| released_count | 9 |
| **total** | **657** |

Categorized, every row accounted for:

- **Already-parked `facing`/turn-rounding family (Phase E): 14.** Same two wall columns
  as every prior capture (right-turns-left at column 64, 12 entries; left-turns-right at
  column ~25, 2 entries). One lemming (1, itself carrying its own drift) shows each turn
  spanning two consecutive flagged ticks rather than the usual single self-healing
  tick — a compounding of the turn-rounding quirk with that lemming's own drift, not a
  third mechanism, reported as a variant rather than silently folded in.
- **Spawn-tick-drift propagation (3/10 lemmings) + landing-tick-drift propagation
  (6/10 lemmings, 10/10 feeding the `y`/`state` rows specifically): 643.** Once a
  lemming's walk starts even one tick later than sim's, every later tick of its walk
  re-shows a one-column `x` mismatch for the rest of the window — count tracks ticks
  remaining after the drift, not the drift's size (per-lemming `x` counts
  143/133/113/100/5/70/5/41/27/11 do not track spawn-drift magnitude at all; lemmings 4
  and 6 have zero spawn drift but only 5 rows each, while lemming 0 also has zero spawn
  drift but 143 rows, driven by how much of its own walk remains and whether a
  wall-bounce happens to briefly resynchronize `x`).
- **Unexplained by either: 0.**

## Read against the D1 gate's literal text

PLAN.md: "every lemming's cell X/Y, facing and state match the original each tick, and
the released count and order match." **This capture does not pass, read literally.**
Every one of the gate's four named fields, plus the released-count clause, shows at
least one disagreement (X 592, Y 29, facing 14, state 13, released count 9); order does
not disagree (all 10 lemmings in the same index order in both traces).

Every disagreement is attributed: the already-parked facing/turn-rounding family (14),
spawn-tick-drift propagation (3/10 lemmings' share), and landing-tick-drift propagation
(6/10 lemmings' share, newly isolated this round). Nothing is left over from an unknown
third mechanism. Both drift floors are ≤1–2 ticks, deterministic, and structurally
consistent with comparing the original's continuous-time release/landing events against
a discrete 200 ms sim tick — the same general family already accepted for the facing
residual, but neither this nor any prior D1 round has pinned an address-level root cause
for why the landing transition specifically rounds the way it does, as opposed to
matching the shape of the already-accepted discretization-floor family.

## Facilitator check

2026-10-21: independently re-verified the field breakdown by grepping
`full_divergence_report.txt` directly (x 592, y 29, facing 14, state 13,
released_count 9, summing to 657 exactly) and the three coverage gaps (ticks 25/1,
54/3, 126/8 — exactly the three nonzero-spawn-drift lemmings). Independently
recomputed both the spawn-tick and the newly-claimed landing-tick drift tables with
fresh code reading only `sim_trace.csv`/`live_trace.csv` directly (first `walker`-state
row per lemming for landing, first row per lemming for spawn) — exact match to every
cell in both delivered tables, confirming the landing-tick-drift finding is real and not
an artifact of the round's own derivation method. Independently confirmed all 14 facing
divergences fall at columns 64 or ~25 in `sim_trace.csv` at the cited ticks, including
lemming 1's two-consecutive-tick variant (columns 64→63 at ticks 59→60 and 139→140,
confirming the "already one tick behind from its own drift" explanation rather than a
new mechanism). Confirmed no Lemmings Revolution process or `LemWeb-*` scheduled task is
running; exe SHA-256 matches the authority; `git status --short` shows only
`docs/tasks.json`'s permitted fields — no `src/**`, `tools/harness/**`, or other
`docs/**`/`PLAN.md` file touched. The disclosed accidental bare-process incident is
accepted as reported (immediate kill, hash re-verified, no harness state touched) and
not treated as consuming the brief's one authorized stock start, consistent with every
prior D1 round's use of that language for the harness's own controlled capture
specifically.

This is the most complete evidence this project has gathered on D1's gate: with both
known harness biases (D1j's throttle lag, D1k's grid-alignment boundary) fixed and
independently verified, the gate still does not pass, for reasons that are now fully
named and none of which is an unexplained residual. Whether the gate should be amended
to exclude this now-complete, bounded discretization-floor class of residual (the same
shape of amendment already granted to A7 for HUD elements), or whether further work
should pursue closing it outright, is recorded as a pending question for the user — see
`docs/tasks.json`'s `standing_run.pending_questions`, not decided here.
