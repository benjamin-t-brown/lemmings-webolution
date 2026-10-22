# D2f: one live climber assignment, 50-tick actor-list trace (D2 part (b), climber)

**Climber: 50/50 ticks now match exactly** after repair 1's climb-to-hoist fix. See
"Round 2 (repair 1)" below for the fix and its verification; the rest of this file is
round 1's original capture and its 45/50 result.

Queue id `d2f-skill-assign-capture`, round 1 of 3. [Brief](../../build/tmp/d2f-skill-assign-capture-brief.md).

## What this round did

Reused already-proven tooling end to end, nothing new live: `tools/harness/assign_skill.py`
(pre-reset, 38/38 historical click-to-assign hit rate) for the panel click, and
`tools/harness/measure_d1_release_trace.py`'s own `capture()` for the post-click sampling.
The new file, `tools/harness/measure_d2b_skill_trace.py`, composes those two primitives
and nothing else — no new memory write, no new click mechanism.

One stock start (pid 5708) on `MM_CLIMB_002`: entered with no input, waited for the
hatch, selected panel index 0 (climber), and clicked the first hoverable lemming found
(second attempt — the first found no hoverable lemming yet, matching the pre-reset
round's own "Observation 2" about lemmings still in the hatch-release grace). The
assignment landed on the first click actually sent: `[lemming+0x48]` (the climber
attribute) flipped `0 -> 1` and the stock count at `0x004C91D8` index 0 went `10 -> 9` in
the same instant, independently re-confirmed a third time after a full 14-second
(70-tick) post-click sample held both values unchanged.

The assigned lemming's own 50-tick trace, read directly off `live_trace.csv`:

```
tick  x   y  facing  state
   1  36  63    1    walker
  ...                         (walks +1 column/tick, 27 more ticks)
  28  63  63    1    walker
  29  64  63    1    climber   <- hits the column-64 wall, starts climbing
  ...                         (rises exactly 1 cell/tick, 14 ticks)
  42  64  50    1    climber
  43  64  49    1    hoister   <- climb -> hoist (topout) transition
  44-46   64  49    1    hoister
  47  64  47    1    walker    <- resumes walking, 2 cells above the wall top
  48  65  47    1    walker
  49  66  47    1    walker
  50  67  47    1    walker
```

This independently reconfirms (not re-derives — the values are already `MEASURED` in
`World.h`'s `Rules`) `climbSpeed = 1` cell/tick (14 consecutive 1-cell climb steps, no
exceptions) and `climberTopOut = 2` cells (the walker resumes exactly 2 cells above the
wall-entry row).

## The offline comparison

A fresh `webo::sim::World` was seeded with one `Lemming` already a `Walker` at the
live-measured cell (`x=36, y=63, facing=Right`) via the already-existing
`World::place()` — whose own doc comment says this is exactly its purpose ("a live
comparison starts from a measured cell instead of that drop") — then
`World::assign(0, Skill::Climber)` was called once and the sim ticked forward 49 times.
This sidesteps D1's own still-unaccepted release-schedule question entirely (it is not
replayed from level start) rather than solving it.

The probe (`SimAssignProbe.cpp`, round-local, **not** a CMake target, **not** referenced
by `CMakeLists.txt`) was compiled and linked directly against the already-built
`webolution_objects` object files, using the exact object list and flags already recorded
in `build/cmake/dev-debug/build.ninja` for `SimReleaseTrace.exe`'s own link line. No file
under `src/`, `CMakeLists.txt`, or any must-not-touch harness file was modified.

Result (`diff_assign_trace.py`, sim tick T vs live tick T+1 by construction — the sim's
own tick-0 marker IS live's already-aligned tick-1 row, so no inferred phase-matching was
needed):

```
checked 50 aligned tick-pairs (shift=1), 45 exact matches
FIRST DIVERGENCE: sim tick 42 vs live tick 43
  sim:  (64, 49, 1, 'climber')
  live: (64, 49, 1, 'hoister')
```

**45 of 50 compared tick-pairs match exactly** — the full 28-tick walk approach and the
full 14-tick climb run, X/Y/facing/state all exact every tick. The one divergence is a
**single, bounded, one-tick-early state label** at the exact same cell: both reach
`(64, 49)` at the corresponding tick, but live is already `hoister` there while our sim
is still `climber` for one more tick. The lag then propagates forward as a constant
one-tick shift for the remaining 7 captured rows (hoist duration and post-hoist walk
speed are both unaffected and still exactly right) — not a growing or compounding error.

Stated as a hypothesis, not yet confirmed against the original's bytes: our `World`'s
climb stepper (`stepClimber`) evaluates the topout-transition check using the
*pre-this-tick* Y position before moving, so it can only detect "we have reached the
topout row" on the tick *after* Y has already arrived there — one tick later than the
original, which (on this one sample) hands off to hoist the same tick Y reaches the
topout row.

## Facilitator check (2026-10-22)

**Confirmed `World::place()` already existed before this round** (git diff on
`src/sim/World.h`/`World.cpp` is empty; the method and its exact doc comment, cited
verbatim in the round's own note, are present in the file at the lines claimed) — this
round did not add new API surface to a must-not-touch file.

**Independently recompiled the probe from source**, not reusing the worker's `.exe`:
rebuilt `SimAssignProbe.cpp` to a fresh object file with the same flags, relinked against
the current `webolution_objects` object files (the same ones a fresh, no-op
`cmake --build --preset dev-debug` confirmed were already up to date), and reran with the
worker's exact seed arguments (`36 63 1 49`). Output is **byte-identical** to the
worker's own saved `sim_trace.csv` (`diff`, no output). Independently reran
`diff_assign_trace.py` against this freshly-produced trace and the worker's unmodified
`live_trace.csv`: reproduces the exact same result (45/50, first divergence at sim
tick 42 / live tick 43, identical values).

**Independently extracted the raw data myself**, not trusting the round's printed table:
read `live_trace.csv` directly with fresh Python, pulled every row for `lemming_index 0`
(50 rows, ticks 1-50, no gaps) — matches the round's printed table exactly, cell for
cell. Read `live_trace.meta.json` directly: `assigned_lemming_index_in_order` is `0`
(confirming which lemming is the assigned one), `counts_before_click`/`counts_after_quick`/
`counts_after_capture` are `[10,...]`/`[9,...]`/`[9,...]` (stock held at 9 through the
full window), `climber_byte_before`/`_after_quick`/`_after_capture` are `0`/`1`/`1` (held
through the full window) — both claimed independent re-confirmations are real, not
asserted.

Independently re-verified process/install state: no `Lemmings`/`dgVoodoo` process after
the round's work; installed exe and `dgVoodoo.conf` hashes (`32675bdb...`,
`d639e220...`) match the authority and the round's own before/after values. `git status`
shows only `docs/tasks.json`'s permitted fields and one new untracked file
(`tools/harness/measure_d2b_skill_trace.py`) — no file under `src/**`, `CMakeLists.txt`,
or any other harness/doc file was touched. Spot-read `measure_d2b_skill_trace.py`'s
`click_and_capture`: it sends the click via `inp.button(0, True)`/`wait_frames(4)`/
`button(0, False)` (a real input-shim click) and calls `assign_skill.select_skill`/
`nearest_hoverable` and `measure_d1_release_trace.capture` by reference — no
reimplemented click mechanism, no new memory write, consistent with the brief's "reuse,
do not reinvent" instruction.

`ctest` was correctly not re-run by the worker (nothing under `src/`/`CMakeLists.txt`
changed); no rebuild was needed for this check either, confirmed by `ninja: no work to
do` immediately before compiling the verification probe.

**This round is accepted as reported: a genuine, reproducible 45/50-tick match with one
specific, bounded, well-characterized residual.** D2 part (b) is not yet settled (only
one of four skills attempted, and even this one skill has an open residual) — the
climb-to-hoist one-tick lag is queued as repair 1 (round 2 of 3) rather than chased
further here, per the brief's own scope.

## Round 2 (repair 1): the one-tick lag read with bytes and fixed, offline only

[Repair 1 brief](../../build/tmp/d2f-skill-assign-capture-repair1-brief.md). Zero stock
starts; the live game was never launched this round.

**Disassembly, with addresses and bytes (`tools/harness/read_steppers.py`):**
`0x004137CA` (the climb stepper's outer per-tick budget loop) writes the lemming's new
Y back to `lemming+0x10` at `0x0041388B` (`fstp qword ptr [ecx+0x10]`), **strictly
before** calling the check function at `0x004138E8` (`call 0x4139bf`). That check
function (`0x004139BF`) reads the lemming's position via the biased-position accessor
(`0x42f35f`, called at `0x004139E4`) as the very first thing it does — reading
`lemming->Y` *after* the outer loop's write-back, since the call itself only happens
after that write-back. Both the ceiling-hit ("Climber hit head") probe
(`0x00413A4E`-`0x00413A5B`) and the topout probe (`0x00413B43`-`0x00413B58`) operate on
that already-moved position, and on topout the Hoist state is constructed
(`call 0x417c80` at `0x00413BA8`) and installed (`call 0x42e696` at `0x00413BD0`)
**inside this same call**, before it returns. There is no second, later call that
performs the handoff — the original moves, checks the new position, and transitions
state all within one call, same tick.

**Correction found beyond round 1's narrower question**: this also settles the ceiling-
hit ("ceiling" / head-bump) check's timing, which `old/docs/scratch/phase3-round10-
steppers.md` had characterized as testing "first" (implying before the move, i.e. the
original check-then-move shape our own pre-fix code used). The actual call graph —
`0x4137ca`'s move, then `0x4139bf`'s two checks sharing one post-move position read — was
not in view when that note was written. Both checks are post-move; neither is
pre-move. `old/` is preserved historical note-taking, not edited here; this file records
the correction as current project knowledge.

**Fix**: `src/sim/World.cpp`'s `stepClimber` now applies the Y move first, then runs
both the wall-collision (ceiling) check and the topout check against the new position —
matching the native ordering exactly, not just the topout half of it.

**New test** (`test_world.cpp`): `testClimbTopsOutSameTickAsHoistHandoff` — places a
climber at a known wall, ticks until it reaches `Hoister`, and asserts no tick is ever
`Climber`-labelled at that same Y. **Side effect, named and explained, not silently
absorbed**: the existing `testClimbWallColumnTopOutAndHoist`'s `lastClimbingY` literal
changed from `41` to `42`, because under the corrected ordering the tick that reaches
Y=41 is already `Hoister`-labelled — there is no tick where the lemming is
`Climber`-labelled at Y=41 any more. The test's comment explains why, citing this
round's own disassembly notes.

### Facilitator check (round 2)

**Independently re-read both disassembled address ranges myself**, running
`read_steppers.py --at 0x004137CA --len 0x200` and
`--at 0x004139BF --len 0x270` directly against the installed exe (not trusting the
worker's saved dumps): both raw dumps are **byte-identical** to the worker's saved
files. Walked the actual instruction bytes myself and confirmed, independent of the
worker's prose: `0041388b: fstp qword ptr [ecx+0x10]` (the Y write-back) precedes
`004138e8: call 0x4139bf` in program order within the same function; `004139dd`/`004139e4`
(`lea eax,[ebp-0x24]; push eax; call 0x42f35f`) is the first substantive call inside
`0x4139bf`, confirming the position read happens at entry to the post-move check; and
`00413ba8: call 0x417c80` / `004113bd0: call 0x42e696` are both inside `0x4139bf`,
confirming the Hoist state is constructed and installed in the same call that does the
topout probe. Every claim in the round's disassembly notes is independently confirmed at
the byte level, not merely re-read from its own summary.

**Independent mutate-one-alone**: restored `World.cpp` to the pre-fix content (`git show
HEAD:src/sim/World.cpp`, SHA-256 `d65fcff2...` — the same hash D1n's own facilitator
check recorded, confirming this is genuinely the unmodified prior baseline), rebuilt
`test_world.exe` alone, and ran it: **342 checks, 2 failures** — the exact two failures
the round predicted (`lastClimbingY` and the new test's "no Climber-labelled tick at the
hoist Y" assertion), reproduced exactly. Restored `World.cpp` to the worker-delivered
SHA-256 (`99c288b8...`), full rebuild, fresh `ctest --preset dev-debug`: **28/28**.

**Independently recompiled and relinked the comparison probe from scratch** (not reusing
the worker's `.exe`): fresh `SimAssignProbe.cpp` → object → link against the current
`webolution_objects` object files, ran with round 1's exact seed (`36 63 1 49`). Output
is byte-identical to the worker's own saved `sim_trace.csv`. Independently reran
`diff_assign_trace.py` against round 1's **unmodified** `live_trace.csv`:
**50 of 50 aligned tick-pairs match exactly**, confirming the headline.

No Lemmings/dgVoodoo process ran this round or during this check (zero stock starts,
as authorized). `git status` shows exactly the three expected files
(`src/sim/World.cpp`, `src/__test__/test_world.cpp`, `docs/tasks.json`'s permitted
fields) — no other file touched. **Climber is accepted as a full 50/50 match.** D2 part
(b) overall remains open: floater, blocker and digger are still untested.
