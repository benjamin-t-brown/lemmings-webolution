# D2i: one live digger assignment, 50-tick actor-list trace (D2 part (b), digger)

Queue id `d2i-skill-assign-digger`, round 1 of 3. [Brief](../../build/tmp/d2i-skill-assign-digger-brief.md).
Fourth and last D2(b) skill. Reuses D2f's/D2g's/D2h's pattern.

## What this round did

One stock start, pid 35476, `MM_DIG_000`. Panel index 7 (digger) selected and assigned
to a hoverable walker. Stock index 7 `10 -> 9` held through the window; climber/floater
attribute bytes stayed `0` throughout (correct — digger is a state change, not an
attribute, like blocker). The retry logic happened to land on a lemming standing over
diggable terrain, which dug three full rows before falling and resuming as a walker —
longer and more informative than `old/docs/scratch/phase3-assign-skill.md`'s own
~271 ms instant-refusal observation on this level, which this capture did **not**
reproduce (reported plainly, not forced to match the cited quirk).

## The offline comparison: 16/50, a new residual shape

Result (`diff_assign_trace.py`): **16 of 50 ticks match.** The pattern, read directly
from the full tick-by-tick table: each of the digger's three "bites" (row 20→21,
21→22, 22→23) lands in our sim **one tick later** than live, but **self-heals** each
time (both sides agree again once the bite completes) — the same *shape* as D2f's/D2h's
already-fixed residuals, but at a different transition. The digger→faller handoff then
compounds this into a larger, **non-self-healing** gap during the fall (live is already
several cells further down and already a walker while sim is still mid-fall), which
settles into a **constant one-tick/one-column lag** for the remainder of the captured
walk — the same downstream-propagation shape already seen in D2f's and D2h's residuals
before their fixes.

**Root cause, hypothesized but not yet disassembly-confirmed**: `stepDigger` (and the
same shared helper's other callers, `stepBuilder`/`stepBasher`/`stepMiner`) call
`bitesThisTick(l.frame, msPerTick(), cycleMs, percent)` directly, while the sibling
steppers `stepHoister` and `stepExploder` both use `(l.frame + 1) * msPerTick() <=
cycleMs` — a `+1` `bitesThisTick`'s four callers do not have. This is plausible but
**explicitly not yet confirmed against the original's bytes**, and it is shared
infrastructure touching four steppers, not one isolated function like `stepClimber`/
`stepFaller` — `World.h` already documents that `builder` has its own, separate
"enter hook seeks the animation forward by one sixteenth of a cycle"
(`builderEnterPercent`) compensation that `builderBricks`'s own already-measured count
(12, matching three live builders exactly) depends on; whether `digger` needs the same
general `+1` or something level/stepper-specific is an open, unproven question, not a
settled one.

## Facilitator check (2026-10-22)

Independently recompiled `SimAssignProbe.cpp` from source, relinked against the current
`webolution_objects` object files, reran with the round's exact seed (`47 20 -1 49`):
output byte-identical to the round's own saved `sim_trace.csv`. Independently reran
`diff_assign_trace.py` against the round's unmodified `live_trace.csv`: reproduces
**16/50 exactly**, same first divergence (sim tick 1 / live tick 2, `(47,20,-1,digger)`
vs `(47,21,-1,digger)`). Independently extracted `live_trace.meta.json`: stock
`[...,10,...]`→`[...,9,...]` at index 7, held; climber byte `0`/`0`; `skill_index`/
`skill_name`/`level` read `7`/`"digger"`/`"MM_DIG_000"` — matches exactly. Read
`src/sim/World.cpp`'s `bitesThisTick`, `stepDigger`, `stepHoister` and `stepExploder`
directly: confirms the `+1` asymmetry the round describes (`stepHoister`/`stepExploder`
use `l.frame + 1`; `stepDigger`/`stepBuilder`/`stepBasher`/`stepMiner` pass `l.frame`
unmodified into the shared `bitesThisTick` helper) — a real, present asymmetry in the
code, though whether it is *the* bug (versus, say, a missing enter-hook seek specific to
digger, the way builder already has one) is not established by this reading alone. No
Lemmings/dgVoodoo process after the check; exe hash matches authority. `git status`
shows only `docs/tasks.json`'s permitted fields — no production file touched, correctly
respecting the brief's full must-not-touch list given the shared-helper risk.

**This round is accepted as reported: a genuine 16/50 result with a specific,
well-characterized residual shape and a plausible but unconfirmed hypothesis, correctly
not acted on given the risk to three other steppers' already-measured behaviour.**
D2 part (b) is **not** complete: three of four skills (climber, blocker, floater) are
full matches; digger is not. A disassembly-read repair round is queued next, scoped
narrowly to digger's own native bite-timing (`FUN_00415a43`/`0x00415B33`, already cited
in `World.h`'s Scrape-cue investigation) and a fix attempted only at digger's own call
site if confirmed — not a blanket change to the shared helper or to basher/miner/builder
— with mutate-one-alone re-verification of all four affected steppers' existing tests
before anything lands.
