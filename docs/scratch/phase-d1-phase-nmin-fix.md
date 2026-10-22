# D1k: the phase/n_min tick-grid boundary fixed; D1g and D1j now agree

Queue id `d1k-phase-nmin-fix`. Round 1 of 3, following D1i
(`docs/scratch/phase-d1-release-to-actor.md`) and D1j
(`docs/scratch/phase-d1-harness-throttle-fix.md`). Offline only — no live game, no
stock start. Both D1g's and D1j's already-saved `live_raw.jsonl` were reprocessed;
nothing was recaptured.

## The mechanism

`infer_phase` canonicalizes every anchor to `t - TICK_SECONDS * round(t / TICK_SECONDS)`,
which is always inside `(-TICK_SECONDS/2, TICK_SECONDS/2]` by construction, then takes the
median over all anchors pooled across every lemming. `build_table`'s old `n_min` rule
(`while phase + n_min*TICK_SECONDS < -1e-9: n_min += 1`) was a sign test on that bounded
value — negative gives `n_min=1`, non-negative gives `n_min=0` — evaluated with zero
margin at the one point, `phase == 0`, that happens to bisect the band.

D1g's capture: `phase = -0.0199999` → `n_min = 1`. D1j's capture, same level, same
no-input protocol: `phase = +0.0000001` → `n_min = 0`. The round corrected the brief's
own placeholder framing ("sub-millisecond noise"): the actual scale is ~20× larger and
one level deeper. Each individual lemming's own anchors are tight (0.00–0.02 s spread),
but different lemmings carry different, essentially fixed median offsets tiling almost
the whole half-tick band (D1g: -0.08, +0.02, -0.10, -0.02, +0.08, -0.04, +0.04, -0.06,
+0.02, -0.10; D1j: -0.08, 0.00, +0.08, -0.02, +0.06, -0.06, +0.02, -0.08, 0.00, -0.10).
Pooling all lemmings into one scalar median makes the result a function of how many
anchors each lemming happens to contribute during a particular 30 s window — which
differs capture to capture under an identical protocol — not sub-millisecond jitter
around one shared true value.

## The fix

`build_table`'s `n_min` is now `round(-phase / TICK_SECONDS)` (nearest grid point)
instead of the floor/sign test. Because `phase` is always inside
`(-TICK_SECONDS/2, TICK_SECONDS/2]` by `infer_phase`'s own construction, this is
*provably* always `0`, independent of which side of exact zero the pooled median lands
on. Only `build_table`'s `n_min` line changed; `infer_phase`'s own pooled-median
instability is not fixed (out of this round's scope — flagged as a candidate for a
future round if `phase`'s own value, not just `n_min`'s boundary decision, ever needs to
be stable).

## Historical figures changed — flagged explicitly

**D1j's reprocessed output is byte-identical to its original** (phase was already
non-negative, so the fix changes nothing for it).

**D1g's own already-landed figures change.** `n_min` flips 1→0, relabeling every row of
D1g's historical `live_trace.csv` one tick earlier: `new[T] == old[T-1]` for every tick
pair, verified exactly, zero mismatches. D1g's previously-reported "167 total
divergences, first divergence tick 10/`existence`" becomes **653 total, first
divergence tick 11/`released_count`** under the corrected grid. The old, smaller count
(167) is retired as a measurement of a mis-aligned grid, not as a better-fitting one.

## Re-running the unmodified comparators

| | D1g, fixed | D1j, fixed (unchanged) |
|---|---|---|
| first divergence | tick 11, `released_count`, sim=1 live=0 | tick 11, `released_count`, sim=1 live=0 |
| total divergences | 653 | 537 |
| coverage gaps | 26 | 2 |

**The two captures' first-divergence tick and field now agree exactly** — before the
fix they disagreed in both tick (10 vs 11) and field (`existence` vs `released_count`).
This is the thing the fix targets, and it is fixed.

**The total counts still differ (653 vs 537), reported plainly, not rounded toward
"fixed."** This is not a new defect: once the grid-alignment confound is removed, the
remaining count difference is consistent with D1j's own already-landed finding that raw
divergence count does not track spawn-tick drift magnitude — once a lemming's spawn
label is off by even one tick, every later tick of its walk re-shows a one-column
mismatch for the rest of the window, so two genuinely-equivalent 150-tick captures can
still differ in exactly how many ticks each lemming drifts, even with a consistent grid
start. D1j's max-drift finding (4 ticks → 1 tick) is unaffected and not re-derived here.

The already-parked `facing`-only family (D1g/D1h, Phase E) is present in both fixed
reports, including the cited example (tick 146, lemming 7, sim=-1, live=1) — but the two
reports do **not** have an identical facing-divergence count (15 in D1g-fixed vs 13 in
D1j-fixed; D1g-fixed has two extra entries at lemming 3, ticks 89 and 129, that
D1j-fixed does not). The round-note's "reproduces identically" line is accurate only for
the one cited example, not for the family's full count — corrected here (see
"Facilitator check").

## What this settles and what remains open

- **Settled**: the `n_min` boundary fragility is fixed, deterministically, and
  independently confirmed to make D1g's and D1j's first-divergence tick/field agree
  where they previously disagreed.
- **Settled, flagged**: this changes D1g's own historical total/first-divergence
  figures (167/tick 10 → 653/tick 11). The change is correct, not a regression — the old
  figures measured a mis-aligned grid.
- **Not fixed, not this round's job**: `infer_phase`'s pooled-median instability itself
  (the ~20 ms per-capture `phase` value still varies capture to capture; only which grid
  point gets called tick 1 is now stable).
- **Open for the next round**: with both known harness biases now fixed (D1i/D1j's
  throttle lag, D1k's grid alignment), the next D1 round is a fresh, clean, full live
  gate recheck — one new no-input capture of `MM_CLIMB_002`, compared against the sim
  trace with the now-fully-fixed harness — to see what residual, if any, remains against
  the D1 gate as literally written, now that the ≤1-tick structural floor D1j found is
  understood rather than conflated with harness bias.

## Facilitator check

2026-10-21: independently re-derived `phase`/`n_min` for both captures from scratch —
reading `infer_phase`'s actual residual formula directly from the file (confirmed
`t - TICK_SECONDS*round(t/TICK_SECONDS)`, always bounded to `(-0.1, 0.1]`) and calling
the now-patched `find_anchors`/`infer_phase`/`build_table` against each capture's own
`live_raw.jsonl` with fresh driver code (not the worker's `reprocess.py`): got
`phase=-0.019999900041149488, n_min=0` for D1g and the worker's own cited values for
D1j, exactly matching the delivered `live_trace.meta.json` files. Independently verified
the fixed formula (`round(-phase/TICK_SECONDS)`) is `0` over 200,000 random `phase`
samples spanning the full `(-0.1, 0.1)` range, confirming the "provably always 0" claim
rather than trusting the two cited data points alone. Confirmed D1j's reprocessed output
is byte-identical to its original capture (`diff`, empty). Confirmed D1g's relabeling
claim directly against the delivered files (not re-deriving the shift myself first):
`new[T] == old[T-1]` holds for all 734 non-blank (tick, lemming) rows checked, zero
mismatches (an initial check in the wrong direction, `new[T]` vs `old[T+1]`, produced
715/723 mismatches before the arithmetic was corrected — the round's own stated
direction, `old[T-1]`, is the right one and is what the independent recheck confirms).
Independently re-ran D1a's own unmodified `diff_trace.py` against both delivered
reprocessed captures: reproduced "tick 11, `released_count`, sim=1, live=0" exactly for
both. Independently re-ran D1g's own unmodified `diff_trace_all.py` against both:
reproduced 653 (D1g-fixed) and 537 (D1j-fixed, unchanged) exactly via each report's own
printed total. Found one imprecision in the round's own narrative: its claim that the
`facing` family "reproduces identically in both fixed reports" holds only for the one
cited row (tick 146, lemming 7) — the two reports' facing-divergence counts differ (15
vs 13; D1g-fixed carries two extra entries, lemming 3 at ticks 89 and 129, that
D1j-fixed does not). Noted here, not escalated — it does not affect any of the round's
load-bearing claims (the grid-alignment fix, the first-divergence agreement, or the
total counts), and the already-parked facing family remains Phase E's concern either
way. No Lemmings Revolution process running (PowerShell `Get-Process`); `git status
--short` shows only the owned `tools/harness/measure_d1_release_trace.py` plus
`docs/tasks.json`'s permitted fields — no `src/**` or other `docs/**`/`PLAN.md` file
touched, and no live game process was started this round (confirmed: `reprocess.py` and
this facilitator's own independent driver both only read saved `.jsonl` files).
