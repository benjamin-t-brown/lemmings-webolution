# D2h: one live floater assignment, 50-tick actor-list trace (D2 part (b), floater)

Queue id `d2h-skill-assign-floater`, round 1 of 3. [Brief](../../build/tmp/d2h-skill-assign-floater-brief.md).
Fourth and last D2(b) skill. Reuses D2f's/D2g's pattern.

## What this round did

One stock start, pid 20376, `MM_FLOAT`. Panel index 1 (floater) selected and assigned
to the first hoverable walker (attempt 2). Both independent landing signals confirmed:
stock index 1 `10 -> 9` (held through the window) and the attribute byte
`[lemming+0x49]` `0 -> 1` (held through the window), while `[lemming+0x48]` (climber)
stayed `0` and the `state` field never changed at the moment of assignment — **this
live-reconfirms the already-known quirk** (`old/docs/scratch/phase3-assign-skill.md`,
Observation 5): the original never makes the parasol a `state` change, only a permanent
attribute flip. Our own `World::assign`'s `Skill::Floater` branch already models this
correctly (`canFloat` is the permanent attribute; `State::Floater` only becomes active
once `fallDistance >= floatAfter`).

`tools/harness/measure_d2b_skill_trace.py` was extended again, purely additively, with
an `attr_name`/`--attr-name` parameter (`"climber"` default, reproducing D2f/D2g's exact
prior behaviour; `"floater"` for this round) — both attribute bytes are recorded
unconditionally regardless of which one the pass/fail check looks at.

The sampled lemming fell twice during the 50-tick window (8 cells each time), both
under `floatAfter` (10), so `State::Floater` itself was never actually entered — a
complete, valid comparison regardless (every tick's X/Y/facing/state is still compared).

## The offline comparison: 11/50, a second landing-tick-drift instance

Result (`diff_assign_trace.py`): **11 of 50 aligned tick-pairs match**, first
divergence at the very first fall's landing:

```
sim tick 11 vs live tick 12
  sim:  (80, 22, 1, 'faller')
  live: (80, 22, 1, 'walker')
```

The live engine reaches `(80, 22)` and is **already** `walker` there; our sim reaches
the same cell one tick earlier but stays `faller` for one more tick before transitioning
— a one-tick-late landing label, same general shape as D2f's climb-to-hoist residual.
The same pattern recurs at the second fall (`sim` ticks 44-45 stay `faller` at the
landing Y while `live`'s corresponding ticks are already `walker`).

**Root-caused (not yet disassembly-confirmed) to `stepFaller`'s per-sub-unit loop**
(`src/sim/World.cpp`): each of a tick's `fallSpeed` (4) sub-units checks
`solidAt(x, y+v)` **before** moving into that cell. When a tick's fall exactly reaches
the ground on its *last* sub-unit, the move happens but the loop has already exhausted
its budget — the re-check that would notice "we are now resting on solid ground" only
happens on the *next* call (next tick)'s first sub-unit. This is architecturally the
same class of bug D2f's repair 1 found and fixed in `stepClimber` (check using a
position the loop has not yet re-examined after the final move of a tick), but in a
sibling stepper, and here **not yet confirmed against the original's bytes**.

**Ruled out, independently confirmed**: this is not the already-known D1n continuous
sub-tick clock gap. `stepFaller`'s continuous-clock opt-in
(`LevelSetup::continuousStepperClock`) is gated on `hasTrackedPhase = continuous &&
l.stepBudgetSeconds != 0.0`; a lemming seeded directly via `World::place()` (not
released through the continuous spawn-budget machinery) always has
`stepBudgetSeconds == 0.0`, so the continuous path collapses to the exact same
sub-unit count as the non-continuous path. A scratch probe with
`setup.continuousStepperClock = true` produced a byte-identical trace to the
non-continuous run (both independently reproduced by the facilitator below).

**Significant connection, flagged not acted on**: this is the *same* fall-to-walker
transition the already-landed D1 investigation's "landing-tick drift" residual is about
(`docs/scratch/phase-d1-full-gate-recheck-fixed-harness.md`,
`docs/scratch/phase-d1-continuous-stepper-clock.md`) — D1l found this drift on **10/10**
lemmings in a population-wide capture; D1n's continuous-clock fix shrank it but did not
close it. This round observed the identical mechanism via a single manually-placed
lemming instead of the population-wide spawn/walk-out trace. Whether a native-bytes-
confirmed reorder fix (D2f's climber precedent) closes this residual **for the general
population-wide case D1's gate cares about**, not just this one floater sample, is an
open question this round does not answer — reported for the facilitator to weigh against
D1's own pending gate-amendment question, not decided here.

## Facilitator check (2026-10-22)

Independently extracted `live_trace.csv`/`assign_result.json` with fresh Python: all 50
rows for `lemming_index 0` match the round's printed table exactly (two 8-cell falls,
landing cells `(80,22)` and `(111,30)`); `floater_before`/`floater_after_quick` read
`0`/`1`; `climber_before`/`climber_after_quick` read `0`/`0`; `state_before` reads
`"walk"` (no state change at the click instant) — the quirk is live-reconfirmed, not
just asserted. `counts_before_click`/`counts_after_quick` read `[0,10,...]`/`[0,9,...]`
— matches.

Independently recompiled `SimAssignProbe.cpp` from source, relinked against the current
`webolution_objects` object files, reran with the round's exact seed (`71 14 1 49`):
output byte-identical to the round's own saved `sim_trace.csv`. Independently reran
`diff_assign_trace.py` against the round's unmodified `live_trace.csv`: reproduces
**11/50 exactly**, same first divergence.

**Independently re-ran the continuous-clock scratch test**: recompiled/relinked the
round's own `SimAssignProbe_contclock_scratch.cpp` (a one-line diff from the primary
probe, `setup.continuousStepperClock = true`), ran with the same seed: output is
byte-identical to the non-continuous trace. The "ruled out" claim holds.

Confirmed `git diff` on `measure_d2b_skill_trace.py` is purely additive (new
`attr_name`/`--attr-name`, defaults preserve D2f/D2g's prior behaviour exactly, both
attribute bytes always recorded) and `git diff` on `src/sim/World.h`/`World.cpp`/
`test_world.cpp`/`CMakeLists.txt`/`PLAN.md` is empty — no must-not-touch file changed.
No Lemmings/dgVoodoo process after the check; installed exe hash matches authority.

**This round is accepted as reported: a genuine 11/50 result with one specific, bounded,
well-characterized (though not yet disassembly-confirmed) residual, plus a correctly
live-reconfirmed quirk and a correctly ruled-out alternative explanation.** Given this
residual's apparent identity with D1's own already-flagged landing-tick-drift family —
and the size of the prize if a climber-style disassembly-confirmed fix also closes that
family — a disassembly-read repair round is queued next (`match-the-original.md`
already cites a known "fall land" entry point, `0x00416560`, from the trampoline-bounce
investigation, as a starting address). That round's scope explicitly includes
re-checking the fix, if any, against D1's own already-saved captures (D1l/D1j/D1g — no
new live start needed) to see whether it moves D1's gate, without deciding D1's pending
gate-amendment question unilaterally.

## Round 2 (repair 1): fixed, 50/50 on D2h; zero effect on D1 (interrupted once, then completed)

[Repair 1 brief](../../build/tmp/d2h-skill-assign-floater-repair1-brief.md). Offline
only; zero stock starts; the live game was never launched this round.

**First attempt terminated mid-round** on a session rate limit after finishing the
disassembly read (two dumps, `disasm_0x4160BD_full.txt` and
`disasm_0x416433_land_check.txt`) but before writing any code. No `src/**` file was
touched by that attempt. A continuation worker picked up from the saved dumps (verified,
not re-disassembled) and completed the round.

**Disassembly, confirmed with bytes**: the native fall stepper's outer per-sub-unit loop
(`0x004160bd`) writes each sub-unit's move back to `lemming+0x10` at `0x0041627f`
**strictly before** calling the land/floater/splat check (`0x0041632b: call 0x416433`),
which re-reads the lemming's position fresh at its own entry (`0x00416459: call
0x42f22e`) — so that check necessarily observes the already-moved Y. The trampoline-bit
test inside it sits at exactly `0x00416560`, the same address `match-the-original.md`
already cited from the unrelated bounce investigation. This is move-then-check, the same
architecture as climber's already-fixed residual, but nested inside a loop that can run
up to `fallSpeed` (4) sub-units per tick rather than once.

**Fix**: `src/sim/World.cpp`'s `stepFaller` now moves (`l.y += v; ++l.fallDistance;`)
unconditionally at the top of each sub-unit, before the water/solid/trampoline/splat
checks — matching the native ordering. A safety invariant was checked, not assumed,
before relying on an unconditional move: `stepWalker`'s own Faller-entry condition
guarantees a fall's gap-to-ground is never less than 3 at its first `stepFaller` call,
and a call that does not land leaves a gap of at least 2 going into the next call — so
gap-to-ground is never 1 at the start of any call, meaning the unconditional move can
never walk a lemming through solid ground before its own post-move check gets a chance
to catch it. **Named consequence**: the existing D1n continuous-clock `leftover` formula
changed from `budget - i*unitSeconds` to `budget - (i+1)*unitSeconds`, since the sub-unit
that lands is now counted as consumed before the check runs.

**New test** (`test_world.cpp`): `testFallLandsSameTickAsWalkHandoff`, same shape as
D2f's `testClimbTopsOutSameTickAsHoistHandoff`. No existing test literal needed changing.

**D2h's own residual: closed.** Re-run against round 1's unmodified `live_trace.csv`:
**50 of 50 ticks now match** (was 11/50).

**D1's landing-tick drift: unaffected — a clean negative, not softened.** `SimReleaseTrace.exe`
rebuilt against the fix and rerun with D1n's own exact invocations for D1a and for
D1g/D1j/D1l's shared argument shape: all four regenerated traces are **byte-for-byte
identical** to D1n's own saved pre-fix baselines. Every named D1 residual family
(landing-tick drift, spawn-tick drift, the facing label) is numerically unchanged. The
disassembly explains why: this fix's trigger condition is a fall whose gap-to-ground at
a tick's start is exactly `fallSpeed + 1` (5, for `fallSpeed=4`) — and that specific gap
does not occur for any of the ten lemmings' actual drop heights on `MM_CLIMB_002` in any
of the four saved captures. The two mechanisms (D1's population-wide landing-tick drift,
and this floater residual) share a *family* (same-tick landing handoff timing) but are
not the *same* occurrence of it — this round's own `prediction.md` named both outcomes
("partially" vs. "not unaffected") as falsifiers in advance, and the data falsified
"partially" in favor of "not unaffected."

### Facilitator check (round 2)

Independently re-ran `read_steppers.py` against both cited address ranges
(`--at 0x004160BD --len 0x2b0`, `--at 0x00416433 --len 0x2c3`) directly against the
installed exe: both raw dumps byte-identical to the saved files (produced by the first,
interrupted attempt, before the rate limit — confirming they were accurate before the
continuation worker ever read them). Walked the actual instruction bytes myself: `0041627f:
fstp qword ptr [ecx+0x10]` (the Y write-back) precedes `0041632b: call 0x416433` in
program order within the same loop body; `00416459: call 0x42f22e` is the land-check
function's own first substantive call, confirming it reads position at its own entry
(necessarily after the caller's write-back); `00416560: and edx, 0x1000` matches
`match-the-original.md`'s existing citation exactly.

Independent mutate-one-alone: restored `World.cpp` to the pre-fix content (`git show
HEAD`, SHA-256 `99c288b8...`, the same hash D2f's own commit left behind), rebuilt
`test_world.exe` alone: **345 checks, 2 failures** — the exact two predicted failures,
reproduced exactly. Restored to the worker-delivered SHA-256 (`37b75c74...`), full
rebuild, fresh `ctest --preset dev-debug`: **28/28**.

Independently recompiled and relinked the D2h probe from scratch against the fixed
object files, reran with round 1's exact seed: byte-identical to the round's own saved
trace; independently reran `diff_assign_trace.py` against round 1's unmodified
`live_trace.csv`: **50/50 exactly**, confirming the headline.

Independently rebuilt `SimReleaseTrace.exe` and regenerated **two** of the four D1
traces myself from scratch (D1a's own argument shape, and D1g's/D1j's/D1l's shared
shape): both match the round's own saved post-fix traces exactly, and both are
**byte-identical to D1n's own saved pre-fix baselines** — independently confirming the
"zero effect" finding for the two distinct trace-generation paths this project uses (the
other two captures, D1j and D1l, are documented, pre-existing byte-identical copies of
D1g's own trace, not separately generated, so verifying D1g's generation path covers
them too).

No Lemmings/dgVoodoo process ran this round or during this check (zero stock starts, as
authorized). `git status` shows exactly the two expected production files
(`src/sim/World.cpp`, `src/__test__/test_world.cpp`) plus `docs/tasks.json`'s permitted
fields. **Floater is accepted as a full 50/50 match.** The D1-relevant negative finding
is recorded in `PLAN.md`'s Phase E and in `standing_run.deferred_questions`' evidence
list as a new, independently-confirmed data point — it does not resolve D1's pending
gate-amendment question, which remains the user's to decide. D2 part (b) now has three
of four skills fully matching (climber, blocker, floater); digger is the last untested
skill.
