# Phase D1g: fresh, comprehensive live re-check with both fixes in place

Queue id `d1g-full-gate-recheck`. Round 7 of D1, following D1a and the
landed fixes D1e (release timing) and D1f (left-turn). Landed 2026-10-21.

## What was checked

One fresh live capture (new stock start, no input, 150 ticks) of
`MM_CLIMB_002`, diffed against a freshly regenerated sim trace with both
fixes in effect, using an extension of D1a's own unmodified `diff_trace.py`
that reports every divergence rather than stopping at the first.

**(a) The tick-10 "existence" artifact still appears first** — identical in
shape to D1e's/D1f's own offline re-checks.

**(b) Looking past it**, lemming 0's wall turn at column 64 (ticks 39–59)
matches exactly, zero divergence. That turn is `facing 1 → -1` — the
already-good "right-facing" case (per round 10 Part 6), not the branch D1f
added; it does not itself exercise the new fix.

## A new, smaller divergence found past tick 10

167 tick/lemming/field disagreements beyond the tick-10 row, all resolving
into two already-understood families — **no reappearance of the stall D1f
fixed** (confirmed: an independent full scan of both traces, every lemming,
all 150 ticks, finds zero duplicate-`x` walker-state stalls in either):

1. **161 entries**: the already-documented D1e spawn-tick discretization
   drift (0–4 ticks per lemming) propagating forward into `released_count`,
   the post-spawn `y`/`state` rows, and — for the two lemmings with the
   largest drift — a sustained one-column `x` offset through their entire
   subsequent walk. Not a new mechanism.
2. **6 entries**: a recurring, single-tick, self-healing `facing`-only
   mismatch. **Correction (2026-10-21, from D1h's independent check,
   confirmed by the facilitator against `sim_trace.csv`): only 3 of the 6
   are the "walker was facing Left, turns to face Right" event D1f's fix
   targets** (column 24: lemming 0 @ tick 85, lemming 2 @ 114, lemming 4 @
   143). **The other 3 are the opposite, already-good "Right turns Left"
   case at wall column 64** (lemming 1 @ ticks 59 and 139, lemming 3 @ 88) —
   not the D1f branch at all. This paragraph originally said all 6 were one
   scenario; that was wrong, and is corrected here rather than only in D1h's
   own note. `x` matches exactly immediately before and after each one
   regardless of type. Raw (unaligned)
   actor-list samples for one instance show the true continuous turnaround
   lands at `t ≈ 16.93 s`, between two raw samples (`t=16.92` facing −1,
   `t=16.94` facing +1) — closer to the harness's own nominal "tick 85"
   (`16.98 s`) than "tick 84" (`16.78 s`) by raw distance, but our sim's own
   tick-grid placement of the same (correctly computed) event lands one
   tick earlier. Character: self-healing, `x` unaffected, same direction
   every time — the same "continuous-process-vs-200ms-grid" family as the
   tick-10 row, not conclusively ruled a defect vs. a harness alignment
   limitation.

## Facilitator check (2026-10-21)

- Fresh rebuild, `ctest`: **24/24**.
- **Reproduced the first-divergence check** against the fresh pair with
  D1a's own unmodified `diff_trace.py`: identical output (tick 10, lemming
  0, `existence`).
- **Independently re-derived the full divergence list** with fresh code
  reading only the raw CSVs (not the round's own `diff_trace_all.py`): the
  field-level counts match exactly (`x` 129, `y` 8, `state` 2,
  `released_count` 21) and, most specifically, **all six `facing`
  divergences match exactly** — same ticks, same lemmings, same
  `sim=-1/live=1` values (`(59,1)`, `(85,0)`, `(88,3)`, `(114,2)`,
  `(139,1)`, `(143,4)`). (My own quick script over-counts "existence" by not
  separating true mismatches from alignment coverage gaps the way D1a's
  reused logic does — a limitation of my independent script, not a
  disagreement with the round's more careful, already-vetted methodology.)
- **Independently re-scanned both traces for the D1f stall signature**
  (duplicate `x` across consecutive walker-state ticks, every lemming, all
  150 ticks): **zero** in either trace — confirms the fix generalizes and
  has not regressed.
- **Independently re-read the raw samples** around lemming 0's tick-85
  turn: `t=16.9200 x=24.0254 facing=-1`, `t=16.9400 x=24.0796 facing=1` —
  exact match. **Independently recomputed the nominal tick times** from the
  capture's own saved phase/`n_min`: tick 84 → `16.7800 s`, tick 85 →
  `16.9800 s` — exact match.
- Live-game state and `git status` independently confirmed clean (no
  process, exe hash matches authority).

Accepted as landed. **D1's gate does not yet pass** — both major defects
(release timing, left-turn stall) are fixed and hold across a fresh,
comprehensive check, but one smaller, well-characterized residual remains:
a single-tick `facing`-label rounding at turn instants, self-healing, with
zero effect on `x`. Whether this is a genuine sim gap or an artifact of the
harness's known single global-phase alignment (D1a's own open limitation:
individual lemmings showed stable but *different* phases) is the natural
next, narrow, fully offline question — no new live capture is needed; the
raw data is already saved.

## D1h follow-up: the per-lemming-phase hypothesis, mixed result (2026-10-21)

Computing each flagged lemming's own phase (the same `infer_phase` function,
restricted to that lemming's own anchors — the per-lemming phases D1a's
"Open limitation" already observed: lemming 0 at `-0.08`, lemming 1 at
`+0.02`, lemming 2 at `-0.10`, lemming 3 at `-0.02`, lemming 4 at `+0.08`,
against the global pooled `-0.02`) and re-aligning with it flips **3 of the
6** flagged ticks to match `live` exactly (lemming 0 @ 85, lemming 1 @ 59
and 139). The other 3 do not flip, each for a distinct, fully-characterized
reason: lemming 3's own phase coincides with the global value (no
correction available), lemming 2's residual has the right sign but falls
`0.01 s` short of the needed shift (bounded by the `0.02 s` raw-sample
spacing), and lemming 4's residual has the wrong sign and sits at the
phase-inference method's own wraparound boundary (`±0.10 s`, half a tick).

**Facilitator check**: independently recomputed every lemming's own phase
with the unedited `find_anchors`/`infer_phase` functions, restricted to
each lemming's own raw samples — exact match to all five values (`-0.08,
+0.02, -0.10, -0.02, +0.08`) and their anchor counts (`136, 123, 109, 94,
79`). Independently confirmed the event-type correction above directly from
`sim_trace.csv`: ticks `59/lem1`, `88/lem3`, `139/lem1` are all at `x=64`
with facing `1→-1` (the already-good case); ticks `85/lem0`, `114/lem2`,
`143/lem4` are all at `x=24` with facing lagging to `-1→1` one tick late
(the D1f-fixed case) — matching exactly. Hand-verified the tick-85/lemming-0
flip arithmetic from the raw samples.

**Conclusion**: the per-lemming-phase hypothesis is directly supported (every
flagged lemming has a stable, nonzero, individually-explicable residual
against the pooled global phase) but does not, on this data, resolve all 6
— it resolves half, and the other half's non-resolution is itself explained
by the same residual arithmetic (too small, zero, or wrong-signed), not an
unrelated mechanism. This residual stays in the "harness alignment
precision limit, not conclusively a sim defect" family: every one of its 6
instances now has a complete, understood explanation, and none shows the
stall D1f fixed recurring. Parked in `PLAN.md` Phase E — closing it fully
would need sub-20 ms live sampling or a per-lemming-aware harness, which is
a tooling investment, not a simulation fix, and needs its own gate
justification before another round is spent on it.
