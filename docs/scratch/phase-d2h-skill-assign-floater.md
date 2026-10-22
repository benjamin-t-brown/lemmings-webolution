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
