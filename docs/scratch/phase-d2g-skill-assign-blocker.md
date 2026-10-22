# D2g: one live blocker assignment, 50-tick actor-list trace (D2 part (b), blocker)

Queue id `d2g-skill-assign-blocker`, round 1 of 3. [Brief](../../build/tmp/d2g-skill-assign-blocker-brief.md).
Second of four D2(b) skills; reuses D2f's exact pattern (`assign_skill.py`'s click,
`measure_d1_release_trace.py`'s sampling, `World::place()` + a round-local probe linked
against the already-built object files).

## What this round did

One stock start, pid 36740, `MM_BLOCK_001_A`. Panel index 3 (blocker) selected, click
landed on attempt 3 (stock index 3 `10 -> 9`, held through the capture window). The
climber attribute byte stayed `0` throughout — correctly, since blocker is a state
change, not an attribute, unlike climber/floater.

`tools/harness/measure_d2b_skill_trace.py` was extended, purely additively, with
`--skill`/`--skill-name` arguments (it was climber-only/hardcoded before); the climber
default (`skill=0`, `skill_name="climber"`) is unchanged, so D2f's own prior usage and
artifacts remain valid.

**Live result: all 50 ticks frozen at `(137, 14, 1, blocker)`** — X, Y, facing and state
identical from tick 1 through tick 50. This matches `old/docs/scratch/phase3-assign-
skill.md`'s own Observation 4 ("world X frozen... while the rest of the level walks").

## The offline comparison

`SimAssignProbe.cpp`, adapted from D2f's/D2g's own template (only the level, seed, and
`Skill::Blocker` changed): seeds a fresh `sim::World` with one `Walker` at `(137, 14,
facing 1)` via `World::place()`, calls `assign(0, Skill::Blocker)`, ticks 49 times.
`World::assign`'s `Skill::Blocker` branch (`src/sim/World.cpp`, confirmed by direct
read) requires solid ground beneath (`solidAt(l.x, l.y+1)`) before setting
`State::Blocker` and stamping the terrain-only blocker field (`stampBlockerField`, which
writes only to `setup_.terrain`, never to a `Lemming` field — so it cannot leak into the
compared X/Y/facing/state). The per-tick `State::Blocker` case only re-checks the
ground; nothing else mutates while it holds solid.

Result (`diff_assign_trace.py`): **50 of 50 aligned tick-pairs match exactly.**

## Facilitator check (2026-10-22)

Independently extracted `live_trace.csv`/`live_trace.meta.json` with fresh Python: all
50 rows for `lemming_index 0` read exactly `(137, 14, 1, blocker)`, no exceptions;
`counts_before_click`/`counts_after_quick`/`counts_after_capture` are
`[0,0,0,10,...]`/`[0,0,0,9,...]`/`[0,0,0,9,...]` (index 3 held at 9 through the window);
`skill_index`/`skill_name`/`level` read `3`/`"blocker"`/`"MM_BLOCK_001_A"` — matches the
round's claims exactly. Spot-read `src/sim/World.cpp`'s `assign()` `Skill::Blocker`
branch and the per-tick `State::Blocker` case directly: both match the round's
description (ground check on assign, ground-only re-check per tick, field stamp writes
only to `setup_.terrain`). Confirmed `git diff` on `measure_d2b_skill_trace.py` is
purely additive (new `--skill`/`--skill-name` arguments, defaults preserve climber's
prior behaviour exactly) and `git diff` on `src/sim/World.h`/`World.cpp`/
`test_world.cpp`/`CMakeLists.txt`/`PLAN.md` is empty — no must-not-touch file changed.

Independently recompiled `SimAssignProbe.cpp` from source (fresh object file), relinked
against the current `webolution_objects` object files (same 21-file list D2f/D2g's own
link commands use), and reran with the round's exact seed (`137 14 1 49`): output is
byte-identical to the round's own saved `sim_trace.csv`. Independently reran
`diff_assign_trace.py` against this freshly-produced trace and the round's unmodified
`live_trace.csv`: reproduces **50/50 exactly**, no divergence.

No Lemmings/dgVoodoo process after the check; installed exe and `dgVoodoo.conf` hashes
match the authority and the round's own before/after values. `stock_starts_consumed`
corrected 0 to 1 (pid 36740 did launch).

**Blocker is accepted as a full 50/50 match, on the first attempt — no repair needed.**
D2 part (b) overall remains open: floater and digger are still untested.
