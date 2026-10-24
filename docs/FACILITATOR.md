# Facilitator contract

An explicit `/facilitator` invocation resumes project facilitation without needing
another task description. Apply the [invocation and queue-selection rules](ORCHESTRATION.md#facilitator-invocation);
reading these instructions is the startup step, not the completed task.

For project facilitation, read in order: [lessons](LESSONS.md), [plan](../PLAN.md),
[queue](tasks.json), [orchestration](ORCHESTRATION.md), [model mapping](ROLES.md),
[guardrails](AGENT_SAFETY.md), and the full canonical
[evidence rules](../.claude/rules/match-the-original.md).
Read build/REA instructions when the task uses those tools. The original, Windows builds and
Windows-only harness scripts run on `mouv`: from either clone, read
[running the original on mouv](agents/MOUV_SSH.md) before any such step and pass it to every
worker that touches Windows. Its session-0 and shared-tree rules are mandatory.

Then read [the overlord directive](DIRECTIVE.md). If it is `Status: active`, it is the
user's authorization for the next standing run when the queue's run is stopped or absent;
follow its scope and stop point.

Time every pass (user, overlord review 2026-10-23). Record the wall-clock start when the
invocation begins and the end when you stop, and finish the final report with the total
elapsed time, plus the time each round took from dispatch to landing. Use real clock
readings (`date`), not estimates.

If `standing_run` is set in the queue and `stopped` is null, follow
[the standing run](ORCHESTRATION.md#standing-run): read its `handoff`, take ownership as that
section describes, and loop round after round and milestone after milestone. Stop only on its
stop conditions; everything else is yours to handle.

Orchestration is the authority for decomposition, ownership, dispatch, repair and recovery.
If delegation is unavailable, report the limitation and the bounded task plan; do not
perform worker research or implementation yourself. Historical checkpoints and failure
examples are in [FACILITATOR_HISTORY.md](FACILITATOR_HISTORY.md), not required startup reading.

## Prepare the brief

Use [the template](briefs/TEMPLATE.md). Give the worker exact relevant lesson headings,
evidence links and the plan section, rather than requiring the entire history. Search the
lessons for related findings before dispatch. Distinguish established evidence from optional
leads and hypotheses. State the question and acceptance criteria without prescribing an
unproven answer; the worker records its prediction and falsifier before a live run.

Before choosing the question, apply the
[milestone, gate and round-cap rules](ORCHESTRATION.md#milestones-gates-and-round-caps):
name the milestone gate the round serves, and count earlier rounds on the same question.

Keep ownership explicit, including the narrow queue-field exception in orchestration.
Run `python scripts/check_agent_framework.py --brief <path>` before dispatching a new brief.
If a legacy brief lacks required fields, complete it before reusing it; do not rewrite an
in-flight brief behind its worker. Keep original-game evidence gates independent of this lint.

## The verification loop — this is the job

Run all of it, every round, before a number reaches `PLAN.md`, `docs/` or the rules.

**1. Independently reproduce the offline headline.** Use an existing independent checker
or write a bounded separate check. Simply rerunning the worker's script is not independent
verification; writing a fresh checker is unnecessary if an independent one covers the claim. A fresh twenty-line
scan over `build/lvl_dump/levels/COMPILED` (102 `.LVL` files) confirmed one round's 18-of-18
audit in two minutes, and later caught another round having measured one exit model out of
four.

**2. Re-read every cited address against the executable.**

```
python tools/harness/read_steppers.py --at 0x00430BA1 --len 0x90    # bytes at an address
rea inspect "C:\progs\Lemmings Revolution\Lemmings Revolution.exe" --provider ghidra
```

Read the actual bytes. A decompiler line is not a measurement and an agent's paraphrase of
an instruction is not either. This caught a claim that a third `add [+0x44]` did not exist
(it does, at `0x0041C81B`) and a `.data` byte asserted as `0xFF` that the loader zero-fills.

**3. Mutate one thing alone and require a *behavioural* failure.** Not bundled — agents
revert everything at once, which proves only that *something* was tested. Back up the file,
change one field, rebuild, run, restore, diff the backup to prove you restored it.

Behavioural means a position series, a state transition, a tick count, a terrain change, a
saved count. A test whose expectation derives from the field under test passes at any value.

Two catches that paid for the whole practice: `hoistCycleMs` 840 survived the agent's bundled
revert but flipping it alone changed nothing (at a 200 ms tick, 800–999 are
indistinguishable, so it is a read, not a test-backed measurement); and widening
`exitTriggerFromPose` back to its old guess left **all tests green**, meaning that round's
headline number was unprotected.

**Watch which binary holds the test.** `test_world`, `test_terrain` and `test_levelobjects`
are separate executables. A good test once looked non-discriminating because the wrong one
was run.

**4. Rebuild and run the whole suite.**

```powershell
.\scripts\Invoke-Ucrt64.ps1 "cmake --build --preset dev-debug"
.\scripts\Invoke-Ucrt64.ps1 "ctest --preset dev-debug"
```

**5. Check the live side was left clean.** Game process still running, and in what state?
Live grid writes restored? Anything written to a lemming, or a save target raised? Rounds
have correctly restored trench cells, carved walls and level-table slots — you confirm it.
`Get-Process | Where-Object { $_.ProcessName -like '*Lemming*' }`.

**6. Check documentation integrity before adding to it.** Are earlier retractions still
present? Did the new bullet land as its own bullet?

**7. Land it yourself**, with a **"Facilitator check"** paragraph recording what you
re-verified and what you narrowed. Narrowing is the normal outcome, not an accusation.

### For the visual rounds ahead, step 3 changes

Phase A is rendering, where a "behavioural failure" is a *pixel* difference. The substitute
is: perturb the measured camera or sprite-view value, re-render, and show the screenshot diff
against the original gets worse. The first visual round must **state its "close enough"
threshold explicitly** — an image diff through a D3D11 wrapper is never pixel-exact, and a
threshold discovered later is a threshold fitted to the result.
