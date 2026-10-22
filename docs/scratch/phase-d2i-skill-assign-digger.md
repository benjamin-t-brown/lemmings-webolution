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

## Round 2 (repair 1): the `+1` hypothesis corrected, not confirmed — no fix applied

[Repair 1 brief](../../build/tmp/d2i-skill-assign-digger-repair1-brief.md). Offline
only; zero stock starts; the live game was never launched this round.

**The real native mechanism, read with bytes**: digger's state vtable (`0x004B09F0`)
step slot is `0x0041B077` — byte-identical to basher's and mine's own step slots
(independently reconfirming `World.h`'s existing "all three skills share slot-1 stepper
0x0041B077" comment against the vtable itself). `0x0041B077` is **not** a discrete
`frame`/`frame+1` tick counter at all: it is a continuous, real-valued
**percent-of-cycle** clock, read from the lemming's own `+0x128` animation-controller
object (`0x00468B04` for the strip's whole-cycle duration, `0x00468B5D` for elapsed
time within it — both already partially named in `World.h`'s own round-5/6 comment).
Confirmed via four still-live debug-print format strings the function calls through
("Staggered action current frame %d", etc., read directly from `.rdata`) and the
integer constant `100` it divides by (`fild dword ptr [0x4bf4c8]`, confirmed by direct
byte read, not the disassembler's string-guess). It fires **at most one** scheduled
record per call, then hands the **unspent remainder of the caller's own time budget
back by reference**.

**The generic per-lemming dispatch loop** (`0x0042E2BF` — the same function `World.h`
already cites for its "`+0x50==2`, run main state twice" check) re-invokes the *same*
step function with that leftover time, for every state, until it returns "no change"
or the leftover drops to ~zero. This is the real mechanism that lets digger's
percent-25 ("the bite") and percent-30 ("the Y step") records — both already
`MEASURED` in `World.h` — fire within the same tick: not a tick-granularity
coincidence, but a same-tick, cross-call re-invocation this function's own leftover-dt
protocol drives.

**The animation clock does not reset at state entry**: digger's enter hook
(`0x004159B4`) installs the "dig" strip via `0x0042FD25`→`0x0046836D`, which stores the
new strip pointer and resource handle but never touches the controller's own elapsed-
time fields (`+0x1CC`/`+0x1C0`). A full `.text` scan found exactly one function that
zeroes `+0x1CC` (`0x00468BC1`), with no caller reachable from digger's own entry path
found this round. **Practical consequence, corroborated by round 1's own already-saved
`live_raw.jsonl`** (read-only, not recaptured): the first dig-bite lands at **~130–140
ms** after the click — independently reconfirmed by the facilitator below — well under
the ~300 ms a from-zero "25% of a freshly-reset 1200 ms cycle" calculation would
predict, consistent with a clock that was already partway through a cycle (inherited
from the walk state in progress) rather than one that restarts at 0% on assignment.

**Round 1's `+1` hypothesis is corrected, not confirmed.** There is no discrete tick
counter in the native path to add 1 to; the comparison is a continuously-recomputed
percent, and the clock it derives from is not reset at state entry. A narrow
`bitesThisTick(l.frame + 1, ...)` patch would be cosmetically similar to
`stepHoister`'s/`stepExploder`'s own shape but is not what these bytes show, and was
correctly **not applied** — `World.h`, `World.cpp` and `test_world.cpp` are all
untouched. Closing this residual properly would need a persistent, cross-state,
real-valued percent clock plus a generic same-tick leftover-budget re-invocation
dispatch — a materially larger architectural change than one bounded round, not
attempted here.

### Facilitator check (round 2)

Independently re-ran `read_steppers.py --states` and confirmed the three vtables'
step-slot byte-identity (`0041b077` for bash/dig/mine) directly, not from the round's
own prose. Independently re-dumped `0x0041B077` (both halves, `--len 0x200` from
`0x0041B077` and `--len 0x182` from `0x0041B1EB`), `0x0042E2BF` (`--len 0x280`), and
`0x004159B4` (`--len 0x30`) directly against the installed exe: all byte-identical to
the round's own saved dump files (confirmed after normalizing line endings, which
differed only cosmetically between my redirect and the round's saved files). Since the
round's own quoted instruction excerpts are drawn verbatim from these now-independently-
confirmed byte dumps, its disassembly claims carry through. Independently recomputed
the first-bite timing from `live_raw.jsonl` and `assign_result.json`/`live_trace.meta.json`
directly (fresh Python, not the round's own analysis): the assigned lemming's Y first
changes at capture-relative `t=0.140 s` — consistent with the round's cited "~130-140 ms"
window. `git status` shows only `docs/tasks.json`'s permitted fields — confirmed no
production file was touched, correctly respecting the brief's scope given the risk to
three other already-measured steppers.

**This round is accepted as reported: a genuine, well-supported hypothesis correction
and clean negative, not a failure to try.** D2 part (b) is **not complete**: climber,
blocker and floater are full matches; digger is not, and the fix it needs is out of
scope for a bounded round. Digger is parked in `PLAN.md` Phase E with this evidence and
a resume brief, rather than spending the question's third round on an attempt already
known to need more than a bounded fix.
