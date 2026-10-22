# D3d: one scripted solution for MM_CLIMB_002, with the original's saved count and end time

Queue id `d3d-solution-discovery`, round 1 of 3. [Brief](../../build/tmp/d3d-solution-discovery-brief.md).
Establishes the oracle data D3's own gate comparison needs.

## The scripted solution

**Assign panel skill 0 (climber) to every lemming, in arrival order, as each becomes
hoverable** — ten calls to the already-proven `assign_skill.assign(h, inp, skill=0)`
(`old/docs/scratch/phase3-assign-skill.md` already proved this exact mechanism on this
exact level). "Which lemming" is not a single chosen pointer — it is "every lemming
that comes out of the hatch," identified by arrival order, since lemming pointers are
not stable between runs. All 10 assignments landed on the first or a quick retry
(1-3 attempts each, when the just-released lemming was briefly off-camera); final stock
was `[0]*10` (climber exhausted).

## Live result: a clean win

One stock start (pid 6712, 1 of the 2 authorized). `ui_state` (`0x004CC134`) stayed `0`
(in-level) through the whole capture and transitioned to **`4`** the instant every
lemming resolved (`alive=0`, `released=0`) — an unambiguous level-end state reached with
no input beyond the 10 clicks.

| Field | Address | Value | Source (already-landed module) |
| --- | --- | --- | --- |
| saved counter | `0x004CBAA4` (`SAVED_WORD`) | **10** | `measure_exit.py` |
| save target | `0x004CBAB0` (`SAVE_TARGET`) | 1 | `measure_float_splat.py` |
| time limit | `0x004CBAA8` (`TIME_LIMIT`) | 300 | `measure_float_splat.py` |
| level clock at end | `0x004CBAAC` (`CLOCK`) | 92 | `measure_steppers.py` |
| **end time** | `time_limit − end_clock` | **208 whole seconds** | computed, both inputs live |

**Saved 10 of 10, far exceeding the save target of 1 — a win.** `CLOCK`'s "one whole
real second per decrement" behavior is not re-derived here; it was already cross-checked
in `build/tmp/d1g-full-gate-recheck/live_trace.meta.json` (a 30-unit drop over a measured
30.0014 s window). Saved-count progression, read directly from the raw samples: 7 already
saved by the time the post-assignment watch began (several climbs/exits happened during
the ~95 s the assignment loop itself took), then 8 at t=21.64s (clock 166), 9 at t=62.52s
(clock 125), 10 at t=88.77s (clock 98), `ui_state` leaving 0 about 6.4s later (clock 92).

## Honestly left open (not resolved this round)

- **`ui_state` 4 vs. 5**: an unrelated earlier round recorded `ui_state 5` at a results
  screen under a deliberately-raised `SAVE_TARGET`. This round's own natural all-saved
  finish stopped polling at the first `ui_state != 0` sample, which read `4`. Whether `4`
  is a transitional screen that would advance to `5`, or a genuinely different end state,
  is not established.
- Whether fewer than 10 climbers (down to the `saveTarget`-minimum of 1) would also win —
  this round deliberately matched the level's own apparent design ("give everyone the one
  skill it ships"), not the cheapest solution, per the brief's "one solution" framing.
- The ~6-second gap between the last save and the `ui_state` transition.

## Facilitator check (2026-10-22)

Independently confirmed all four cited addresses (`0x004CBAA4`, `0x004CBAB0`,
`0x004CBAA8`, `0x004CBAAC`) are already defined exactly as cited in `measure_exit.py`,
`measure_float_splat.py` and `measure_steppers.py` — no new address was invented this
round. Independently re-parsed `watch_samples.json` with fresh Python (476 samples): the
saved-count transitions (7→8 at t=21.64/clock 166, 8→9 at t=62.52/clock 125, 9→10 at
t=88.77/clock 98) and the `ui_state` transition (0→4 at t=95.18/clock 92) reproduce the
round's own table exactly, cell for cell. Independently recomputed `end_time_seconds =
300 − 92 = 208` from `solution.json`'s own raw fields. Independently confirmed 10
distinct lemming pointers in `distinct_lemmings_assigned` (no double-assignment).
Confirmed the new harness file imports every helper (`assign_skill`, `measure_spawn`,
`measure_steppers`, `measure_exit`, `measure_float_splat`, `measure_d1_release_trace`)
by reference and edits none of them. No Lemmings/dgVoodoo process after the check;
installed exe hash matches authority. `git status` shows only the new harness file and
`docs/tasks.json`'s permitted fields — no `src/**` or other `docs/**` file touched.
`stock_starts_consumed` corrected 0 to 1.

**Accepted as reported.** This is the oracle a later D3 round compares our own `World`'s
matching scripted run against: **10 climber assignments in arrival order → saved 10,
end time 208 whole seconds**, with every field's exact address. The next D3 round wires
this same scripted sequence through our own `sim::World`/app and compares.
