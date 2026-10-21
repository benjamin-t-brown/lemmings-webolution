# Phase D1f: the known left-turn tick divergence, fixed

Queue id `d1f-left-turn-fix`. Round 6 of D1, independent of D1a-D1e's
release-timing work. Landed 2026-10-21.

## The fix

`World::stepWalker` now captures `wasLeft = (l.facing == Facing::Left)`
before either `turn(l)` call inside it. On both turn sites (a blocker-field
hit, and the generic wall `rise > maxStepUp` arm), if `wasLeft` was true, the
function calls `stepWalker(l)` once more in the same tick — matching the
original's measured mechanism (`old/docs/scratch/phase3-round10-steppers.md`
Part 6, `0x0041C67E`): the turn arm returns `0`, so the original's slice
loop keeps going in the same tick and a walker that *was* facing Left steps
once more, immediately, in its new (Right) direction. A walker that *was*
facing Right already consumed the tick moving onto the wall column
(`settleOnProbeColumn()`) and must not get the extra step — "right-facing
turns already match" is preserved exactly.

**A correction to this round's own brief**, found and reported before any
test literal was touched: the brief's framing ("a right-facing walker
turning to face left") was backwards. Tracing the code against round 10's
own worked numbers (wall at column 23, live `24,25,26,27` vs ours
`24,24,25,26`) shows the broken transition is the *reverse*: a walker that
*was* facing Left, turning to face Right. The round built the fix against
the measured numbers, not the brief's gloss, and reported the disagreement
explicitly.

One test literal changed as a direct, justified consequence:
`testBlockerTurnsWalkers`'s **second** sub-case (approaching a blocker from
the right, facing left) — `turnedAt == 131` → `132`, with an inline comment.
The **first** sub-case (`130`) and `testWallTurn` are confirmed unaffected.

## Validation

- **Offline `compare_tick.exe`** (re-added to `CMakeLists.txt`; the source
  needed no changes) reproduces round 10's own cited numbers exactly: fixed,
  `24,25,26,27`; reverted (mutate-one-alone), `24,24,25,26`. A right-facing
  turn at column 64 reproduces round 10's own table unchanged either way
  (`64,63,62,61,60`) — the already-correct case stays correct.
- **One live start**: `measure_steppers.compare(h, "walker", ...)`
  (unmodified) matched **40/40 ticks** on `MM_CLIMB_002`, including a
  right-facing turn at tick 32, with no divergence anywhere. **Honestly
  flagged gap**: that lemming did not reach the column-23 wall within the
  printed window, so this round's own fresh live data does not itself cross
  a left-facing turn — the left-turn claim rests on round 10's own
  already-landed live measurement plus this round's offline reproduction of
  those exact numbers, not a fresh live crossing. A narrow follow-up (same
  script, a longer window or a narrower `--band`) would close this with one
  more start, if ever judged necessary.

## Facilitator check (2026-10-21)

- **Rebuilt after touching every changed file**: clean, all targets
  including the newly re-added `compare_tick`. Fresh `ctest`: **24/24**.
- **Own, independent mutate-one-alone** (separate from the round's own
  backup/restore): disabled both `wasLeft` branches with `false &&`,
  rebuilt — `ctest -R world` failed exactly as predicted
  (`turned in cell 132 (got 131)`); ran `compare_tick.exe ... walker 30 63
  left 10` myself and got the broken `24,24,25,26,27` sequence. Restored the
  source; `sha256sum` matched the round's own recorded backup hash
  (`9523211f...3362e1f10`) exactly. Rebuilt; `compare_tick` reproduced the
  fixed `24,25,26,27,28` sequence; `ctest`: **24/24** again.
- **Independently re-ran the right-facing sanity check**
  (`walker 60 63 right 8`): reproduces `64,63,62,61,60` exactly.
- **Independently re-read `run_compare.log`**: confirms
  `SUMMARY walker MATCH 40 ticks on MM_CLIMB_002` exactly as reported.
- **Checked whether the fix moves D1e's open residual**: regenerated the
  `SimReleaseTrace` output with both fixes now present and re-ran D1a's own
  `diff_trace.py` against D1a's own `live_trace.csv` — unchanged, still
  `tick 10, lemming 0, field 'existence'`. The two fixes are independent;
  this one does not touch the release-timing alignment artifact (expected —
  no lemming reaches a wall that early in the 150-tick window).
- **Live-game state, independently checked**: no `Lemmings Revolution`
  process, installed exe hash matches the authority. `git status --short`
  shows exactly the four files the brief's allowlist names.

Accepted as landed. Both named D1 blockers (release timing, D1a-D1e; the
left-turn divergence, this round) are now fixed and independently verified.
The honestly-flagged live-crossing gap above is not treated as blocking —
the evidence (historical live measurement + this round's exact offline and
partial-live reproduction + two independent mutate-one-alone confirmations)
is strong; a fresh full-gate live re-check is the natural next D1 round
regardless.
