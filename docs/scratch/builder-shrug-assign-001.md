# builder-shrug-assign-001 — needs_review

Playable v1, [playtest 9 Y6](../playtest/2026-11-16-playtest-9.md): a spent
builder could not take a new job during its shrug. The next dependency is
interrupting that pause so the player can chain Builder or switch jobs at the
bridge end. Installed routes: **Build and Block, node 14 / MM_BUILD_BLOCK_A**,
and **Seafood Sarnie, node 15 / MM_BLOCK_BUILD_BASH**. Difficulty: routine;
recommended model: Sonnet; assigned Codex override used. **Supervisor smoke
verification and independent review are pending.** Human playtest accepts Y6.

## Production path and change

`runApplicationLeg` -> `hoveredActor` -> `TubeScene::injectedWalkerScreenBoxes`
-> `prepareInjectedWalker` -> displayed shrug sprite box ->
`passesKnownPickExclusions` -> `pickNearestScreenLemming`. Hover uses that hit
for `TubeScene::drawHoveredWalker` and the runtime cursor; the left-button
consumer uses the same hit for `World::assign`. Successful assignment retains
the existing cue, stock decrement and pause-resume path.

The picker already admits an active Shrugger for every otherwise eligible
skill; no shrug-specific visibility/box exclusion was found. The retained
playtest-9 recordings actually contain resolved clicks on state **7 = Shrugger**
(see divergence rows below). Thus the demonstrated assignment failure is in
`World::assign`'s work-state whitelist, where Shrugger was missing. That earlier
omission is explicitly documented in
[round 6](../../old/docs/scratch/phase3-round6-live.md): eligibility was unread
then. No picking, cursor, rendering or ranking change is needed for that path.
The human's reported absence of hover feedback still needs their visual check.

`World::stepBuilder` lays/advances the final brick, enters Shrugger and holds
its position. `World::assign` now includes Shrugger alongside Walker in the
existing work-state list. Blocker/Builder/Basher/Miner/Digger replace the shrug
immediately, resetting `frame=0`; Builder refills bricks and Basher resets its
cycle counter through their existing entry code. Assignment changes neither
`x`, `y` nor facing, so the new job starts at the last bridge endpoint.
Climber/Floater/Bomber already apply as parallel capabilities/fuse during shrug
and keep its progress. Their duplicate refusals, stock checks and Blocker support
check remain in force. No other state's admission changes. Animation tracking
observes the new state on the next composition; motion tracking snaps the
Shrugger-to-job transition to that same position.

## Static evidence and limits

Reference: `C:/progs/lemmings-webolution/Lemmings Revolution/Lemmings Revolution.exe`,
x86, SHA-256 **32675bdb49866e76502713251fdfd2b09078c2481c72ca22ff68ebf4b3737a2f**.
Used the reverse-engineer-anything skill and static REA/Ghidra **12.1.4**;
profile digest `3c06738e45e144d5f073edbd3244c6bff46222f389525fe659bc92dca2ccdece`.
Opened only this reference, then closed analysis without snapshot arguments.

New [constructor packet](../../build/builder-shrug-assign-001/static/shrug-constructor.json):
`41AB42` writes vtable `4B0C78`; `41AB4B` writes **state+14 = 1D**. Pseudocode
and instructions agree. Instruction evidence
`ev_3b941c07e9928fe09ae17af53bef240947522ed4b8f68d72fe12652b3e5fe783`.
The retained [antigravity eligibility research](antigrav-skill-assign-001.md)
provides the dispatcher `45A0E0`, wrappers and their full instruction packets.
Builder `4102B4`, Basher `40C061`, Miner `4187FC` and Digger `415EEA` exclude
categories E/3/0, then use common support eligibility `4183D3`; Blocker
`40E347` also delegates there. The common predicate calls `42E913` for support
and excludes categories **13/21/1E**, not shrug's **1D**. Digger additionally
checks `42F497`. Climber `413C26`, Floater `417EB5`, Bomber `4140A6` retain
their existing capability/fuse checks. Selected retained instructions and the
new constructor instructions are assembled in
[crosscheck.txt](../../build/builder-shrug-assign-001/static/crosscheck.txt).

This supports the inference that a supported shrug can take the usual jobs;
it is not a native runtime comparison. The clone's broader partial native
applicability map, screen picking/occlusion and hybrid timing remain
**provisional**. No additional original restrictions or state support semantics
are invented here. Original timing/fidelity gates, parks and completion criteria
are unchanged. No claim of original hover pixels or full fidelity is made.

## Checks and recorded human routes

Registered UCRT64 GCC/CMake/Ninja build passed:
[build.log](../../build/builder-shrug-assign-001/build.log), then the unit-only
rebuild [unit-build.log](../../build/builder-shrug-assign-001/unit-build.log).
New **legacy** `shrug_assign` passes in **0.04 s**:
[focused.log](../../build/builder-shrug-assign-001/focused.log),
[JUnit](../../build/builder-shrug-assign-001/focused.xml). It guards all eight
skills early/late in shrug and both facings, hover eligibility, unchanged
position, entry state/resources, parallel duplicates, and rejection of work
skills in the other eleven excluded states. It is a synthetic contract unit,
not a new play route. No smoke assertions/timeouts or existing tests changed.

Ran `build/agent-native/main/webolution.exe --game-dir "Lemmings Revolution"
--replay <file>` for **all 17 promoted recordings**. Exact filenames/results
and individual logs: [replay-results.tsv](../../build/builder-shrug-assign-001/replay-results.tsv);
invocation [script](../../build/builder-shrug-assign-001/run_replays.py).

| Level | Result |
| --- | --- |
| M06_KF_1 | PASS, 93/50 saved, won=1, completed=1 |
| MM_BASH_002 | PASS, 10/10, won=1, completed=1 |
| MM_BASH_MINE_WLR1 | PASS, 20/10, won=1, completed=1 |
| MM_BLOCK_001_A | PASS, 24/15, won=1, completed=1 |
| MM_BLOCK_BOMB_A | PASS, 12/10, won=1, completed=1 |
| MM_BOMB | PASS, 17/10, won=1, completed=1 |
| MM_BUILD | PASS, 20/10, won=1, completed=1 |
| MM_CLIMB_002 | PASS, 10/1, won=1, completed=1 |
| MM_DIG_000 | PASS, 19/10, won=1, completed=1 |
| MM_FLOAT | PASS, 10/1, won=1, completed=1 |
| MM_MINE | PASS, 20/10, won=1, completed=1 |
| MM_W_012 | PASS, 16/10, won=1, completed=1 |
| MM_BASH_DIG_001_A | PASS, 20/10, won=1, completed=1 |
| MM_BUILD_BLOCK_A | PASS, 15/10, won=1, completed=1 |
| MM_FLOAT_CLIMB2 | PASS, 10/6, won=1, completed=1 |
| MM_BLOCK_BUILD_BASH | DIVERGED at step 2850: actor 3 assignment acceptance changed |
| MM_E4_4_SKILLS | DIVERGED at step 2075: actor 2 assignment acceptance changed |

Both divergences directly encode Y6's intended behavior change:

- Seafood Sarnie's row `a 2850 3 4 0 198 63 7 0 0 0` records a **refused
  Builder** on shrugging actor 3 at **(198,63)**; it now succeeds.
- A Long and Lonely Road's row `a 2075 2 5 0 169 74 7 0 0 0` records a
  **refused Basher** on shrugging actor 2 at **(169,74)**; it now succeeds.

The replay reader checks position/state before acceptance, so each reached the
recorded shrug position/state before diverging. The prior task's
[results](../../build/pad-pipe-presentation-001/replay-results.tsv) retain their
earlier PASS results (21/20 and 28/20). This run stops at the changed click and
does **not** establish their later outcomes. Recordings and replay validation
are untouched; no legacy check was removed to hide the divergence.

Changed: `src/sim/World.cpp`, `src/__test__/test_shrug_assign.cpp`, one legacy
target in `CMakeLists.txt`, and this report. Pre-task source/CMake backups are
under [entry/](../../build/builder-shrug-assign-001/entry/); the
[task-only patch](../../build/builder-shrug-assign-001/task-only.patch) excludes
incoming edits. All pre-existing changes were preserved. Supervisor PID 11176
holds RepositoryLease; no protected controls were edited. `Get-Process` found
no clone/build/test runner at entry; CIM command-line access was denied, so the
supported process-list fallback was used. No such runner remained after checks.
Runs isolate LocalAppData under `build/builder-shrug-assign-001/appdata`;
replay and unit paths open no SaveStore. Normal and installed saves were unused.
No original/oracle execution, debugger, hooks, capture, installation, subagents,
background work, commits or pushes occurred. The mandatory smoke profile was
not duplicated and remains pending supervisor execution.

Handoff: shrug reassignment is implemented; the legacy unit and 15 replays pass, with two explained intended replay divergences.
Next: supervisor smoke and independent review, then the user's hover/reassignment playtest for acceptance.

What to check: **Build and Block (node 14)** or **Seafood Sarnie (node 15)** —
select Builder, hover the lemming while it shrugs after its twelfth brick, and
click: expect the cursor/body highlight and a fresh bridge starting at that
endpoint without waiting or walking first. Repeat with Basher: it should start
from the same feet position immediately. Falling/dead lemmings should retain
their existing work-skill refusal.
