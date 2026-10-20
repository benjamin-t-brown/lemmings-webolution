# The facilitator role

This hands over the **facilitator role** itself, not a research round. Read it before you
dispatch anything.

Reset handoff recorded on 2026-10-18 at commit `562a099`, with a clean tree and 14 tests
green. For current round status, read `PLAN.md` and its linked facilitator checks, then
check the tree. The predecessor's longer Phase 3 version is at `old/docs/FACILITATOR_HANDOFF.md`; its
mechanics still hold but its paths and status are retired.

## What the job is

You dispatch one research round at a time to a subagent, **verify its headline claims
independently before anything is promoted**, own the documentation, and decide what goes
next. You do not usually do the research yourself.

The project's results are sound because of the verification step, not because the agents are
reliable. Across 31 rounds, several had a claim narrowed or retracted on re-check — including
claims made by the facilitator.

## Task sizing: make the cheap tier succeed

Decomposition is part of the job, not a preliminary. Break each request into the smallest
round a **targeted** worker can finish and you can verify quickly. A good brief has one
question, one falsifier, one ownership area, a bounded list of addresses/commands, and a
candidate note that fits comfortably in the worker's context.

Split the work further when the brief:

- spans more than one subsystem or file area;
- needs more than a handful of live runs, or unbounded search;
- has no single acceptance check, or the check is a judgement call rather than a
  measurement;
- says “and also”.

Use the **hard** tier only when the task genuinely cannot be split — ambiguous cross-cutting
reverse engineering, a targeted attempt that failed for a reason other than size, or a change
whose failure mode needs broad judgement. Write the escalation reason in the brief and the
queue entry. Prefer splitting over escalating; a small failed round usually means the brief
was too big, not that the worker needed a stronger model.

This is also the recovery plan: if a worker dies mid-round, a small task leaves little work
lost, and the queue shows exactly where to resume.

## Read first

| File | What it gives you |
| --- | --- |
| `docs/LESSONS.md` | **every fact worth carrying**, each with the address or script behind it |
| `PLAN.md` | the goal and the phases |
| `docs/tasks.json` | in-flight / blocked / ready-for-verify |
| `docs/ORCHESTRATION.md` | orchestrator–worker loop for every harness |
| `docs/ROLES.md` | model names for *this* harness |
| `docs/briefs/TEMPLATE.md` | brief skeleton |
| `docs/REA_SETUP.md` | the verified Ghidra-backed decompiler setup, and two traps |
| `.claude/rules/*` | the evidence standard you enforce, plus all 31 rounds' findings |

`old/` holds the retired plan, four handoffs and 42 round notes. Nothing was deleted. Code
comments in `src/sim/World.h` cite notes as `docs/scratch/phase3-*.md`; read that as
`old/docs/scratch/`.

## What you own

**You own** `PLAN.md`, everything in `docs/`, and `.claude/rules/**`. No subagent edits
them — put that in every brief's must-not-touch list and land results yourself.

Not territorial: agents read those files first, so an error there propagates into every later
round, and an agent appending its own result tends to over-generalise it. Two real cases: a
round wrote its finding as applying to `Exit*.a3d` when it had measured one of four models,
and a round inserted its bullet into the **middle** of an existing bullet, orphaning the tail
so it read as current and revived three claims a later round had closed.

**Correction to the original scratch-note exception:** the user's stricter promotion gate
also covers `docs/scratch/`. Agents keep unverified candidate notes in `build/tmp/`; the
facilitator lands accepted material in `docs/scratch/` only after verification. Agents own
the code, the tests, and new scripts under `tools/harness/`.

## The verification loop — this is the job

Run all of it, every round, before a number reaches `PLAN.md`, `docs/` or the rules.

**1. Re-run the offline headline with your own script, not theirs.** A fresh twenty-line
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

## The failure catalogue

Agent-side, by how often it actually happened:

1. **Over-generalising a correct measurement.** One model measured, all models claimed. Ask
   "how many distinct cases exist, and how many did they sample?" — usually answerable
   offline in minutes.
2. **Claiming the tests pin a result when they pin part of it.**
3. **A plausible series from the wrong object.** The worst case here: a billboard-quad probe
   was fitted to "a lemming" and produced clean, plausible numbers for **nine rounds** before
   anyone noticed the quad was not a lemming. Two measurements had to be retracted. Read the
   actor list at `0x004CBB18`.
4. **Reading past a section's raw data.** `.data` vsize `0x1185B8`, rawsize `0x7000` — file
   bytes end at `0x004C6000`.
5. **Near-tautological tests**, caught twice.
6. **Wrong shape, not wrong number** — see `docs/LESSONS.md`.
7. **Silent environment failures.** `goto_level` once reported entering one level while the
   loaded path stayed another, so the dump was of the wrong level. Insist briefs say "read
   back the loaded path before trusting a dump".

Facilitator-side — the predecessor's own errors, so you can expect them:

- **Wrote a lessons file from memory instead of from the tree.** It was wrong within twenty
  minutes: rounds 12–31 had run in other sessions and settled every gadget, including eleven
  clean negatives, while the fresh doc listed them as open work. **Read the tree and
  `.claude/rules/` before writing any status claim.**
- **Recommended a round that was already done.** Proposed a "Phase 2 close-out" from a stale
  plan list; all three items were done or residue. **Verify work is still open before
  dispatching it.**
- **Put a wrong formula in a brief** (`floor(worldH − y)`; the engine truncates). The agent
  caught it. Briefs are not authoritative; the exe is.
- **Attributed an observation to the wrong cause.** Supplied a correct grid read with a wrong
  explanation attached. Supply observations; let the agent find the cause.
- **Left stale text in owned files.** The plan said `climbSpeed` is 2 — the retired
  factor-of-2 doubling — long after it was measured as 1. Sweep for contradictions with the
  code periodically.

## Dispatch mechanics

**One round, one question.** One agent owns the running game at a time; say so in the brief.

**Read `docs/tasks.json` before dispatching.** If `active` is `blocked` or
`ready-for-verify`, finish or unblock that first — do not start a second round that needs
the live game. Update the queue yourself when you write the brief (`queued`), when the
worker starts (`running`), when it returns (`ready-for-verify` if they have not), when
approval/capacity stops you (`blocked` + reason), and when you land (`landed` / clear
`active`). Schema and permissions: `docs/ORCHESTRATION.md`.

**Models are split deliberately** — names only in `docs/ROLES.md`. Dispatch the targeted
worker by default; escalate to the hard worker only for a recorded reason. An explore agent
is for map-only work. A round is bounded work against a written brief; judgement (support,
over-generalisation, stale docs) stays on the strong tier. **Verification never delegates.**
A cheap explore or lint pass is not verification.

Every brief contains: the authority statement and a read-first table; **what is already
settled, listed explicitly** so nothing is rediscovered; the round's question with every lead
you already hold (addresses and bytes, including anything you verified yourself); **why the
work fits the targeted tier, or the recorded reason for escalating to the hard tier**; the
single acceptance check you will re-run; the evidence standard, with the prediction written
**before** the run and its falsifier; the mutate-one-thing-alone test requirement and the
which-binary warning; build commands and environment traps; a must-not-touch list;
deliverables including the scratch note and the state of the game process; and "report
honestly — do not round, smooth or over-claim".
Point the brief at `docs/tasks.json` `active.id` so the worker knows which queue slot it is.

**If an agent dies mid-round** (it happened three times on rate limits): died early with
nothing read, re-dispatch verbatim. Died late with edits on disk, do not re-dispatch — build,
run the suite, and finish the missing verification yourself. Set `blocked` or
`ready-for-verify` on the queue so the next harness session sees it.

## Reset baseline and current round

The historical reset baseline was branch `decompile`, commit `562a099`, clean tree,
**14/14 tests**. Do not treat that historical commit or cleanliness as a current status report.

The simulation is **done and correct** — 31 rounds, every `Rules` field a read plus a live
series, all nine skills, every gadget including eleven clean negatives. It is kept.

The reset of 2026-10-18 retired the *visible* half: the flat-first renderer and the
placeholder shell are in `old/src/`. At that historical checkpoint there was no runnable
executable, and asset-side test coverage for tile naming and state→sprite mapping regressed
with three moved tests. A4a restored the headless executable and asset-side tests; the
current A4d/A4e state is recorded below and in `PLAN.md` and `docs/tasks.json`.

**A1 is complete for its tested scope.** The early
[camera checkpoint](scratch/phase-a1-camera.md),
[cell replay](scratch/phase-a1-cell-oracle.md) and
[projection/cache replay](scratch/phase-a1-projection-cache.md) retain real earlier
failures and corrections. The [true live-arena pass](scratch/phase-a1-live-arena.md),
[depth check](scratch/phase-a1-depth-clipping.md),
[control mapping](scratch/phase-a1-control-mapping.md), and final
[zoom/elevation check](scratch/phase-a1-zoom-elevation.md) supersede the old open-A1
handoff. Do not redispatch settled operands, the cell capture, or the shelved
access-watch prototypes. The zoom writer and wrapper-internal Z bits remain
explicit residual unknowns, not grounds to reopen all camera research.

**A2's declared fixed-ROI fixture and scenery inventory are complete.** Read the
[A2 facilitator check](scratch/phase-a2-tube.md) before A3/A4. It corrects the
brief's unflipped tile rows, implements both OUT banks and center filters, and
passes nine retained views plus an exposed inner-wall region. Isolated route
selection and draw-order mutations survive the old ROI; those choices are READ
evidence, not pixel-discriminated parameters. The wider changed-pixel diagnostic
still fails (`0.022030839 > 0.02`), as do cavity and upper-rim regions. Keep these
failures and the omitted-scene inventory for A4; do not call the full scene accepted.

The live suppression probe establishes that the upper gold band persists without
the tube walker; its exact background model/resource remains unassigned. Every
temporary byte and camera write was restored, the owned process stopped, and the
executable/save/wrapper hashes match the starting snapshot. No debugger or hardware
breakpoints were used. This round's actual REA session was closed. The build and
all 14 CTest tests pass. Existing unrelated uncommitted harness/configuration and
A1-note work was preserved; do not infer a clean committed checkpoint from this.

**A3 is complete for the tested selection scope.** Read the
[A3 facilitator check](scratch/phase-a3-sprite-view.md): the old “walk 12 / fall 7
views” assertion reversed the SPR axes. Those are LO temporal counts; walk/fall
have five directions per temporal entry, block/cartwheel eight, shrug one. The
new score/index/mirror helper passes 608 raw native instruction cases and 42 live
actor-bound predictions, with controlled eight-entry graphics and camera sector
crossings. Both stock-raster candidate crops and restores are exact; isolated
index and mirror changes are pixel-visible. The stricter worker sensitivity
`>0.02` diagnostic remains failed. Production sprite rasterization is not accepted.
The original's stored basis and phase inputs must be wired faithfully in A4;
platform sin/cos is not an exact x87 boundary-rounding claim.

The facilitator independently verified eleven isolated source regressions and
exact restores, then rebuilt and passed all 15 CTest tests. Final worker and
facilitator memory writes were restored while suspended, both owned processes
stopped, and executable/save/wrapper hashes match baseline. Earlier failed worker
attempts and invalid derived scores are retained and narrowed in the A3 note.
The worker's actual REA session is closed; no debugger was used in this round.

**Current checkpoint, independently verified 2026-10-19: A4e complete for its bounded
scope.** The user confirmed that the prediction-only predecessor worker was interrupted.
One replacement worker completed the same slot; the facilitator independently verified
and landed the [window/control check](scratch/phase-a4-window-controls.md).
`src/app/main.cpp` now opens the 640x480 SDL window, defaults to the accepted `route`,
and retains headless `--shot`. The fixed 144 Hz input accumulator consumes queued deltas
and key edges once; the controller covers the accepted revolve, zoom and flip mapping.
The title and `--help` describe the core controls. Tile-position and sprite-state tests
are restored. A4 remains open for actor/simulation presentation and the omitted scene.

The facilitator passes 19/19 CTest, 54 retained-stock input comparisons, a native
window/SPACE/WM_CLOSE smoke, and nine independent A4d rerenders with unchanged scores and
PNG hashes. The native initial frame is exactly the explicit route headless frame.
Fourteen isolated controller/timing/event regressions are discriminated; the zoom-exit
one initially survived until the worker strengthened its test. Each source mutation was
restored exactly and the focused test passed. The old default wall independently fails
the pixel gate. No stock process was started; the stock hash is unchanged and no owned
process remains. See the linked note for READ corrections and verification artifacts.

**A4f landed 2026-10-19 as geometry-verified, pixel-negative.** Read the
[A4f check](scratch/phase-a4-actor-billboard.md). Corners match native within one ULP;
seven isolated mutations fail; 20/20 tests; nine tube views unchanged. Pixels fail the
absent-actor gate because the live actor vertex RGB is 152 while `0x440A10` initialises
255; a comparison-only 152/255 scale passes, a U flip does not. Do not hard-code 152.
`ctrl+0x1B0` UV reordering and the unmirrored branch remain READ-only.

**A4g landed narrowed** ([check](scratch/phase-a4-actor-tint-writer.md)): the setter
`0x406260` and a fluid-only fade were found; walk/block 152 is still unexplained. The worker
wrongly said the level file was missing, and the facilitator corrected it.
**A4h landed as a clean negative.** `0x4682a6` copies the actor's controller *into* eight
prototypes; it never writes the actor. Static leads around `0x406260` are exhausted. **A4i/A4j
(GDB hardware watches) named no writer.** A4j showed the watch missing a polled write, so
A4i's "set once" is retracted to inconclusive.
**A4k–A4q solved the tint** (see the tint-writer note). It is a stored per-level grey light
map applied by `0x403820`, and the A4f gate now passes with `LightMap`. Lessons for briefs:
- Frida via REA works where GDB did not.
- Hooks derived from the *first* backtrace can be on the wrong stack (level-load vs walker).
- Worker mutation restores that preserve the old mtime leave ninja objects stale. Rebuild
  after touching the restored files before trusting a CTest result.
The queue is clear.

Foreground acquisition is automated as recorded in the rules. If future probes
use hardware breakpoints, delete them while the original is stopped and verify
cleanup rather than trusting a detach message.

## Two standing cautions

**A stale 385 MB worktree is registered.** `git worktree list` shows
`.claude/worktrees/plan-continue` at `c9e9b1d` on branch `worktree-plan-continue`, left by an
earlier agent. It is gitignored. Removing it would reclaim the space, but it was left alone
because that branch may hold work nobody has reviewed. Do not commit it.

**Preserve quirks.** German menus, RETURN only when the menu highlight is exactly 0, a short
ESC that fails to leave pause, the drawn crosshair disagreeing with the tested cursor. All
real, all in the rules file, and an agent's instinct is to smooth them out. Do not let it.
