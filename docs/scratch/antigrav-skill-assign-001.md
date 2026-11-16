# antigrav-skill-assign-001 — needs_review

Playable v1, Playtest 6 V6: ceiling Blockers can now be assigned and remain
supported. A production SDL pointer route reproduces the entry refusal in an
authentic task-entry build and succeeds after the fix. Two following inverted
walkers turn at the ceiling Blocker. A separate simulation script reaches the
installed level's target before timeout and obtains a successful production
completion snapshot. These are clone observations and source tests, not human
playability acceptance or original-fidelity evidence.

Difficulty: difficult; task recommendation Opus for the coordinate/eligibility
trace; this worker uses the explicitly authorized Codex override. No model pins
were changed. Full registered verification is **pending supervisor execution**.

**Level and dependency.** The installed map names node 11, `m06_kf_1`,
“Sometimes up..sometimes down.” Its resolved archive level is `M06_KF_1`.
The [human ledger](../playtest/2026-11-15-playtest-6.md) identifies inability to
assign Blocker upside down as a level blocker. The installed inventory is
`0,0,0,30,0,0,0,0`: Blocker is the only offered skill. Fixing its entry and
continued support is therefore the next dependency, without changing release
or simulation clocks. See the retained [map/terrain measurement](../../build/antigrav-skill-assign-001/worker-evidence/after/installed.txt)
and [18-level gravity/stock census](../../build/antigrav-skill-assign-001/worker-evidence/after/gravity-level-skills.csv).
All eight skill types are offered somewhere among those gravity levels; the
seven absent here are checked with explicitly supplemental stock.

**Authentic baseline.** Before production edits, `World.cpp`, `main.cpp`, and
`CMakeLists.txt` were copied under ignored
`build/antigrav-skill-assign-001/before-source/`. Their hashes match the worker
entry manifest in
`build/agent-runs/20261115T224344-39368-47694f59/before-source-hashes.json`.
The frozen World SHA-256 is
`df8af703d6d383a79a6b31d661d9d324effea5034127228471db74aca3ca89b0`.
The [identity record](../../build/antigrav-skill-assign-001/worker-evidence/before-sha256.json)
and registered `antigrav_skill_identity` check guard that source. The baseline
driver links that frozen World to the same unchanged application/render/input
objects and headers used by the after driver. No header or production app edit
was needed. This is an isolated World comparison, not a claim that every shared
application source was frozen. The backed-up application source remains unchanged.

**Production path and traced cause.** `runApplicationLeg` loads the installed
assets and World. Natural release/walk reaches the lower flipper's support
cells around `(68,88)`, flips gravity, rises, and lands under the platform.
At 9000 ms actor 0 is a Walker at `(73,77)`, gravity -1, supported at `(73,76)`;
its upright support cell `(73,78)` is empty and its current support is not a
flipper. Gravity persists after leaving the flipper.

SDL panel click `(50,148)` selects plate 3 through
`RuntimeSkillSelection::consumePanelClick`. Motion to `(512,351)` hovers the
displayed inverted sprite. `hoveredActor` obtains the shared reflected sprite
boxes from `TubeScene::injectedWalkerScreenBoxes`, constructs
`runtimePickCandidate`, and uses `pickHoveredLemming`. The same helper handles
the subsequent left click and calls `World::assign`.

Picking already succeeds: the old `World::assign` checks support at `y+1` with
gravity +1 for Blocker, so it rejects this ceiling Walker without consuming
stock. The Blocker state's later support check repeats that assumption. The
field stamp and erase also point up in Y-down coordinates regardless of gravity.
Changing only entry would therefore produce a transient Blocker that falls on
its next state update. No picker ranking change belongs to this fix.

`World.cpp` now probes `y+verticalDir` for Blocker entry and continuation, and
stamps/clears `y-k*v` with `v` equal to the gravity sign. For +1 these are exactly
the old expressions. The four-cell field retains its separate bit plane and
does not change terrain material. The existing walker field test supplies the
turn; it needed no change. No other skill eligibility, state entry, stepper,
animation, release rate, timing, save or completion policy was changed.

**Retained pointer behavior and images.** The driver uses
`runApplication --level m06_kf_1 --sim --native --no-audio --frames 230`, the
owned 0.1-second clock, ordinary SDL panel/motion/click events, and the
read-only `LevelFrameView` callback. It never mutates the application World.
Pause holds the natural ceiling state for hover/click; successful assignment
resumes through the existing application policy.

| Observation | Authentic before | After |
| --- | --- | --- |
| Before-click frame | [full](../../build/antigrav-skill-assign-001/worker-evidence/before/before-click.png) | [full](../../build/antigrav-skill-assign-001/worker-evidence/after/before-click.png), byte-identical pixels |
| Click result | [Walker detail](../../build/antigrav-skill-assign-001/worker-evidence/before/after-click-detail.png), stock 30, still paused | [Blocker detail](../../build/antigrav-skill-assign-001/worker-evidence/after/after-click-detail.png), stock 29, resumed |
| Subsequent ceiling Walker | No assigned Blocker | [before turn, 13.5 s](../../build/antigrav-skill-assign-001/worker-evidence/after/turn-sequence-13500.png), [turn, 13.7 s](../../build/antigrav-skill-assign-001/worker-evidence/after/turn-1.png), [departing left, 14.2 s](../../build/antigrav-skill-assign-001/worker-evidence/after/turn-sequence-14200.png) |

I inspected the paired click details and full turn-sequence frames. The selected
actor is visibly inverted below the platform; its Blocker pose extends its arms.
The approaching walker overlaps that pose near the turn, so the CSV is the
precise facing/identity evidence, alongside the actual composed frames. Detail
PNGs are 6x nearest-neighbour crops of retained full application frames.

The [after state record](../../build/antigrav-skill-assign-001/worker-evidence/after/pointer-route.csv)
records actor 0 still at `(73,77,-1)`, Blocker, at 9100 ms. Actor 2 changes from
right-facing approach to left-facing Walker at that field at 13700 ms; actor 4
turns there at 18500 ms. The driver asserts the Blocker remains supported through
the entire remaining run. The [before record](../../build/antigrav-skill-assign-001/worker-evidence/before/pointer-route.csv)
uses the same hover and click coordinates and observes refusal. Both drivers
finish with zero assertion failures. [Comparison and assignment index](../../build/antigrav-skill-assign-001/worker-evidence/comparison.json).

**Original static evidence, separately from clone observations.** Only the
registered repository reference was opened:
`Lemmings Revolution/Lemmings Revolution.exe`, x86, image base `0x400000`, SHA-256
`32675bdb49866e76502713251fdfd2b09078c2481c72ca22ff68ebf4b3737a2f`.
REA used Ghidra 12.1.4, profile digest
`3c06738e45e144d5f073edbd3244c6bff46222f389525fe659bc92dca2ccdece`.
The analysis was closed without snapshot arguments. The full returned records,
including provider limitations, are retained in
[static-pseudocode.json](../../build/antigrav-skill-assign-001/static-pseudocode.json)
and [static-instructions.json](../../build/antigrav-skill-assign-001/static-instructions.json).
The latter cross-checks the pseudocode against decoded instructions.

* `45A0E0` dispatches the eight skill eligibility checks. There is no general
  inverted-gravity ban. Skill/state/parallel-capability exclusions still apply;
  this is not a statement that every skill is assignable in every state.
* Blocker eligibility `40E347` delegates to `4183D3`, which requires support via
  `42E913` and excludes fall/bounce/other categories. `42E989` loads actor `+44`
  and `42E98C` subtracts it from native Y before the support lookup. Instruction
  evidence: `ev_757721cbed9f79ce26a3aaf9244fe31edc53073d3c4c916cf4f45dca6cc02eed`.
* `40E2D7` multiplies the stamp row offset by actor `+44`; `40E19A` does the same
  for erase. Evidence:
  `ev_b63f2004c02b776b6eccd34cfa2f7f045fb95ba9fdf504cdf1f55bd03cd06a71`
  and `ev_989c8c4fcc6f1370a8fc226642ce9b94a4161a26df6ac33e5dac53019b059dad`.
  These agree with the retained [Blocker research](../../old/docs/scratch/phase3-round8-blocker.md).
* Retained [inverted presentation](antigrav-presentation-001.md),
  [gravity integration](gravity-gadgets-001.md), and
  [inverted climb/dig evidence](../../old/docs/scratch/phase3-round30-inverted.md)
  explain the existing feet-coordinate convention and its limits.

**Every skill's assignment result.** These results concern a supported inverted
Walker. The [skill audit](../../build/antigrav-skill-assign-001/worker-evidence/after/skills.csv)
recreates the natural real-level ceiling route in a fresh World for each skill.
For the seven zero-stock jobs it adds exactly one supplemental use; it does not
claim those jobs are offered by this level. `passesKnownPickExclusions` also
accepts that natural Walker for each job. Only Blocker had a source refusal.

| Skill | This level | Inverted assignment result | Original evidence / limit |
| --- | --- | --- | --- |
| Climber | 0, supplemental stock | Accepted; `canClimb=true` | `45A0E0 -> 413C26` checks capability; retained inverted climb/hoist observations supplement entry. |
| Floater | 0, supplemental stock | Accepted; `canFloat=true` | `45A0E0 -> 417EB5` checks capability, no gravity exclusion. Full inverted floating presentation not assessed here. |
| Bomber | 0, supplemental stock | Accepted; fuse 5000 ms | `45A0E0 -> 4140A6` checks existing fuse state, no gravity exclusion. Inverted explosion geometry not assessed here. |
| Blocker | 30, real panel/hover/click | Before refused; now accepted and persistent; two inverted walkers turned | `40E347 -> 4183D3 -> 42E913`; signed field loops above. |
| Builder | 0, supplemental stock | Accepted; Builder state | `45A0E0 -> 4102B4 -> 4183D3`, no gravity exclusion. Full inverted building behavior not assessed. |
| Basher | 0, supplemental stock | Accepted; Basher state | `45A0E0 -> 40C061 -> 4183D3`, no gravity exclusion. Full inverted bashing behavior not assessed. |
| Miner | 0, supplemental stock | Accepted; Miner state | `45A0E0 -> 4187FC -> 4183D3`, no gravity exclusion. Full inverted mining behavior not assessed. |
| Digger | 0, supplemental stock | Accepted; Digger state | `45A0E0 -> 415EEA`; support/zone restrictions remain. Retained inverted dig/refusal evidence supplements entry. |

The extra Digger helper pseudocode is retained separately in
[static-digger-extra.json](../../build/antigrav-skill-assign-001/static-digger-extra.json):
`42F497` tests support flags `0x4200`, not a blanket gravity exclusion. That
extra helper was not independently re-read as instructions in this task; no new
zone behavior was implemented from it. Existing partial native applicability
mapping, including non-Walker cases, remains partial. Builder/Basher/Miner still
contain upright-coordinate expressions in their existing steppers; assignment
success here does not certify complete inverted skill behavior. Broad stepper
corrections are not silently included in this assignment fix.

**Scripted win, distinct from the pointer demonstration.** The separate route
uses the real loaded World and `RuntimeSimState::advanceElapsed(..., true)` with
20 ms input, including the production timeout latch and completion snapshot.
It reads simulation state and calls `World::assign` at these seven naturally
reached Walker positions. It does not place actors, change terrain, alter stock,
accelerate release, reset a save, or fabricate a result. It is not a full
frontend/pointer playthrough.

| Time ms | Actor | Feet X,Y | Gravity |
| ---: | ---: | --- | ---: |
| 10640 | 1 | 96,27 | +1 |
| 13140 | 0 | 91,63 | -1 |
| 22220 | 3 | 65,39 | +1 |
| 24180 | 2 | 64,46 | -1 |
| 32660 | 5 | 91,56 | +1 |
| 34620 | 4 | 91,33 | -1 |
| 43540 | 7 | 64,70 | +1 |

The [winning trace](../../build/antigrav-skill-assign-001/worker-evidence/after/winning-route.csv)
reaches 50 saved at **173800 ms**, before the installed 180-second timeout.
The [completion summary](../../build/antigrav-skill-assign-001/worker-evidence/after/winning-summary.txt)
records `success_snapshot=1` at 186020 ms, 54 saved, zero dead, 78 spawned, seven
Blockers, and the unchanged 50-of-100 target. Remaining Blockers prevent early
population completion; the ordinary timeout path publishes success. The existing
multi-hatch/release limits remain unchanged; this result is not a release-fidelity
claim.

Correction to exploratory development: the first routes used poor turns or
incorrect underside coordinates and failed. A later longer route reached 50 at
184200 ms, after timeout; the initial target-only assertion incorrectly called
that a win. [Its retained record](../../build/antigrav-skill-assign-001/late-route/winning-summary.txt)
is **not valid win evidence**. The final route shortens the turns, explicitly
asserts the target is reached before timeout, and requires the actual successful
completion snapshot. Earlier failed development logs remain under the task build
directory; the final cited run supersedes their conclusions without changing any
production clock or existing test assertion.

**Checks and preservation.** Registered CTest drivers measure the installed map,
gravity-level inventories, natural assignment route and all skill entries. The
after driver additionally checks both gravity signs' field direction, unchanged
support material, full cleanup on reassignment, cleanup when a neighbouring
Digger removes support, and stock preservation when support is absent.

The paired upright `MM_CLIMB_002` controls apply each of eight skills, then retain
100 ticks of actor/stock/field/material-hash records per skill.
`antigrav_skill_normal_unchanged` compares all 800 records byte for byte. Existing
normal-gravity tests and their assertions were left untouched.

Final focused GCC `-O2 -g` build and CTest results are retained as
[final-build.log](../../build/antigrav-skill-assign-001/worker-evidence/final-build.log)
and [final-ctest.log](../../build/antigrav-skill-assign-001/worker-evidence/final-ctest.log):
**8/8 passed**, 25.03 seconds. Tests: `world`, `runtimepick`, `clicktoassign`,
`runtimeskillpanel`, `antigrav_skill_identity`, `antigrav_skill_before`,
`antigrav_skill_after`, `antigrav_skill_normal_unchanged`. Only focused builds/tests
were run; main/recovery full verification is pending the supervisor.

The production demonstration injects a private `FileSaveSink` beneath its output
directory. Installed and normal clone saves are compared for bytes and presence
before/after; both remain unchanged, as does the direct route's private save.
[Before](../../build/antigrav-skill-assign-001/worker-evidence/before/summary.txt)
and [after](../../build/antigrav-skill-assign-001/worker-evidence/after/summary.txt)
records confirm this. The archive retains 60 evidence/source/log files with
[SHA-256 hashes](../../build/antigrav-skill-assign-001/worker-evidence/sha256.json).
The original game, copied original instructions, hooks, debugger and original
capture were never run. No tools were installed and no background job remains.

**Unknowns and changed files.** Exact conversion from native raw Y and `+368`
bias to the clone's feet grid remains provisional. The signed footprint is
static-traced; reflecting the accepted upright footprint about clone feet is the
bounded player-correct adapter, labelled in code. A separately authorized original
observation of upright/inverted Blockers recording raw Y, `+368`, support samples
and stamped rows would settle that conversion. No such observation was made.
Other skills' full inverted execution and native non-Walker eligibility remain
outside the demonstrated assignment coverage. Fidelity gates and user parks
remain unchanged; no full-project completion is claimed.

Files changed by this task: `src/sim/World.cpp`,
`src/__test__/test_antigrav_skill_assign.cpp`, `CMakeLists.txt` (one include),
`cmake/antigrav_skill_assign.cmake`, `cmake/antigrav_skill_identity.cmake`, and this
report. Pre-existing app/window, queue/status, hook-log, window-scale report/test
edits were preserved. The supervisor's active lease was observed; no protected
control or shared instruction was edited by this worker.

Handoff: antigrav-skill-assign-001 is needs_review; ceiling assignment/turns and a before-deadline scripted success are demonstrated; all eight focused checks pass.
Next action: supervisor runs mandatory full registered verification, then independent acceptance review; human playtest and original fidelity remain separate.
