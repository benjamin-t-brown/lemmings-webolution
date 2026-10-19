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

## Read first

| File | What it gives you |
| --- | --- |
| `docs/LESSONS.md` | **every fact worth carrying**, each with the address or script behind it |
| `PLAN.md` | the goal, the phases, the model policy |
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

**Models are split deliberately** (`PLAN.md`). **User-authorized correction for Codex:**
facilitator on **Sol** (the Opus role), task agents on **Luna** (the Sonnet role), dispatched
with an explicit **`model: gpt-6-luna`**. This supersedes the original Opus/Sonnet names;
verification still never delegates. A round is bounded work against a written
brief, where a mid-tier model is fast and cheap enough to run many; the judgement — whether a
claim is supported, whether it over-generalises, whether a document has gone stale — is where
the stronger model earns its cost. **Verification never delegates.**

Every brief contains: the authority statement and a read-first table; **what is already
settled, listed explicitly** so nothing is rediscovered; the round's question with every lead
you already hold (addresses and bytes, including anything you verified yourself); the
evidence standard, with the prediction written **before** the run and its falsifier; the
mutate-one-thing-alone test requirement and the which-binary warning; build commands and
environment traps; a must-not-touch list; deliverables including the scratch note and the
state of the game process; and "report honestly — do not round, smooth or over-claim".

**If an agent dies mid-round** (it happened three times on rate limits): died early with
nothing read, re-dispatch verbatim. Died late with edits on disk, do not re-dispatch — build,
run the suite, and finish the missing verification yourself.

## Reset baseline and current round

The historical reset baseline was branch `decompile`, commit `562a099`, clean tree,
**14/14 tests**. Do not treat that historical commit or cleanliness as a current status report.

The simulation is **done and correct** — 31 rounds, every `Rules` field a read plus a live
series, all nine skills, every gadget including eleven clean negatives. It is kept.

The reset of 2026-10-18 retired the *visible* half: the flat-first renderer and the
placeholder shell are in `old/src/`. **There is no runnable executable** until the new
3D-first entry point exists, and asset-side test coverage for tile naming and state→sprite
mapping regressed with three moved tests. Both debts are recorded in `PLAN.md` Phase A.

**A1 has been dispatched and remains open.** Read the
[verified subset and retained failures](scratch/phase-a1-camera.md) before writing another
brief. Pose/lens and bounded control fixtures have been checked; the failed left-key view
and depth/clipping still require work. Do not redispatch settled operand reads or claim
the entire round passed because the simulation suite is green.

The subsequent [active draw-path check](scratch/phase-a1-cylinder-draw.md) corroborates
one inner-bank submission at the retained left camera. The subsequent
[cell-fixture replay](scratch/phase-a1-cell-oracle.md) retains a failed geometry candidate
and unprotected isolated selection mutations.
The [emission checkpoint](scratch/phase-a1-cell-emission.md) now checks the selected
quad's ordered coordinates and UVs. Its next question is the original depth/cache
producer under the existing pixel criterion; do not redispatch the bounded route,
cell capture or emission trace. Foreground acquisition is automated for probes
as recorded in the rules. Explicitly delete hardware breakpoints while the original
is stopped before detaching, and verify cleanup rather than trusting a detach message.

The subsequent [projection/cache replay](scratch/phase-a1-projection-cache.md)
records independent source-derived agreement under a diagnostic arithmetic model,
rejected agent claims, and the review-capacity blocker. Its operands remain READ,
not pixel-accepted. Continue with the missing candidate render and isolated pixel
mutations; do not redispatch the located matrix copy stage or run the shelved
access-watch prototypes. The latest REA session's closure is unconfirmed.

Remaining A1 evidence must still come from the executable and the original frame, using
REA where needed. The accepted operands replace the camera guesses in `old/src/gfx`;
they do not establish clipping or full-scene fidelity. After A1, continue with A2 the
tube, A3 sprite view selection (the largest single "the sprites look wrong" fix — `.SPR`
stores 12 views for `walk`, 8 for `block`, 7 for `fall`, and the old renderer hardcoded view
0), then A4 the entry point with revolve as a first-class control.

## Two standing cautions

**A stale 385 MB worktree is registered.** `git worktree list` shows
`.claude/worktrees/plan-continue` at `c9e9b1d` on branch `worktree-plan-continue`, left by an
earlier agent. It is gitignored. Removing it would reclaim the space, but it was left alone
because that branch may hold work nobody has reviewed. Do not commit it.

**Preserve quirks.** German menus, RETURN only when the menu highlight is exactly 0, a short
ESC that fails to leave pause, the drawn crosshair disagreeing with the tested cursor. All
real, all in the rules file, and an agent's instinct is to smooth them out. Do not let it.
