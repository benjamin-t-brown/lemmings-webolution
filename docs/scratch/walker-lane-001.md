# walker-lane-001: facing-dependent radial walker lanes

Status: **needs_review**. Playable v1, roadmap step 4 / playtest 2 Q1 in
[the human ledger](../playtest/2026-11-13-playtest-2.md). Difficulty: difficult;
recommended model: Sonnet-class (the facing rule was found), using the assigned
Codex provider override. Full registered main/recovery verification remains
**pending supervisor execution**, then independent acceptance review.

## Demonstrated failure and production change

The human reported that original walkers keep to their relative right and slide
across when they turn; the clone drew both directions on one radial line. This is
the next placement dependency after accepted P7 smooth X/Y presentation. No
simulation position, collision, release rate, skill rule or timing was changed.

Actual installed route: `runApplication --level MM_CLIMB_002 --sim --native`,
natural hatch release, fall to the first walkway and walk right into the first
obstruction at simulation cell **(64,63)**, turn left, pause during the radial
crossing, hover the displayed sprite, resume and continue left. No injected
actor, terrain, state or completion. Tests use a declared 20-ms frame interval,
normal speed, SDL dummy video/audio, the real production compositor and present
path. This is scripted clone observation, not a human playtest, a win, an
original runtime comparison or a measured real-time FPS claim.

Before: `runApplicationLeg` observes `LemmingMotionTracker` before/after each
existing World update. `injectedWalkersFromWorld` combines the fractional X/Y
with `terrainSampleFromLemming`'s constant Z **0.75** for every facing.
`prepareInjectedWalker` -> `actorPoseFromNativeSample` ->
`interpolateCellMeshPosition` blends inner/outer mesh vertices using that Z,
then projects/draws the sprite.

After: the same read-only motion tracker also stores radial `laneZ`, separately
from the X/Y path. `injectedWalkersFromWorld(..., motion)` copies `drawn.z` into
the existing `plus8Z` sample. All remaining placement code is unchanged. The
lane targets are **0.75 for right, 0.25 for left**. In the recovered coordinate
system, increasing X is right-facing along the cylinder and Z increases outward;
these targets place each facing on the relative right side of the walkway.

Picking uses `TubeScene::injectedWalkerScreenBoxes`, which calls the **same
`prepareInjectedWalker`** as drawing, including pose, projection and occlusion.
`main.cpp::hoveredActor` uses those boxes and the same `plus8X/Y/Z` for candidate
distance. No separate screen-space shift or stale pick rectangle was added.
The existing no-motion adapter overload keeps its diagnostic/fixture Z=0.75.

## Retained evidence and targeted static cross-check

Reference: repository-local `Lemmings Revolution/Lemmings Revolution.exe`,
SHA-256 **32675bdb49866e76502713251fdfd2b09078c2481c72ca22ff68ebf4b3737a2f**.
No original executable, copied instructions, oracle, debugger, hook or capture
was run. REA opened this path with provider `ghidra` and was closed without
snapshot arguments. Ghidra/Java analysis is not target execution.

Started with [actor rendering](../lessons/actor-rendering.md),
[camera/tube](../lessons/camera-tube-sprites.md),
[accepted motion implementation](actor-motion-001.md), and recovered
`decomp-lemmings/src/actor_walking.cpp`. The decisive retained sources are
[walking surface/depth](../../decomp-lemmings/docs/evidence/actor_walking_surface.md)
and [motion trace](../../decomp-lemmings/docs/evidence/motion_trace.md).
Their historical native-oracle results were read, not rerun.

New complete returned evidence is retained in
`build/walker-lane-001/static-evidence.json`, with reference identity, provider,
profile, evidence IDs, pseudocode, instructions and limitations:

| Static fact | Instruction cross-check / evidence |
| --- | --- |
| `0x42F93C` sets `4C5B30=0.25`, `4C5B28=0.75`, `4C5B20=0.75` | `FLD [4B10C8]`, subtract/add `[4B10D0]`, stores at `42F94B`, `42F95D`, `42F96F`; `ev_523e7854cb9f9b57892dc1489199f81033084fda61b3ad8741d0ec6351ff5c79` |
| With byte `actor+5E` enabled, direction `actor+40 == -1` selects `4C5B30`; other direction selects `4C5B20` | `42F985` reads flag; `42F9A5` compares direction; `ev_02c58da3c91f595080e86461fc9839d5188b49b9cb2c29f882ac0d21eda7636e` |
| Depth `actor+18` approaches its target by 0.02, with crossing clamp | Same instruction record: `FADD` / `FSUB [4B10E0]`, stores/comparisons; returned constant bytes `7b14ae47e17a943f` = binary64 `0x3F947AE147AE147B`; `ev_b3b81bf144dca9b5ff090c2d83e6a12cc6114891b7f2f3226a5cbdf22de6c2bb` |
| It is gradual per walking update, not an instantaneous turn assignment | `0x41BDD4` calls `0x42F977` at `0x41BF53`, after its remaining-time loop; `ev_506590b44316fdcd779629fb0a97592c01c4cec70f51127c15420ade408f1863` |

Ghidra pseudocode `ev_a53a8d7c7eb2ac6de5e363777fa76fceef833c0f7318f1ea87b1bb82e1dbdb71`
agrees with those instruction branches for ordinary finite depths. Reading the
uninitialized-on-disk global bytes gave zeros, **not** usable lane values; the
initializer and retained constant evidence establish the targets. The older
`0x42EDCD` helper that pulls to 0.5 belongs to another behavior and is not the
walking lane helper. The raw global read and initial bounded lookups remain in
the tool history; the relevant returned records are archived above.

The target values and clamped linear approach are evidenced. **Duration in
seconds is not established**: 0.02 is per helper invocation, without an elapsed
argument. A complete 0.5-depth crossing needs roughly 25 such calls; their runtime
cadence is not inferred from clone ticks or SPR animation speed.

## Explicit provisional presentation policy

The clone approaches by **one radial blend unit per level second**, hence a
full crossing takes **500 ms**. The tracker integrates the previous confirmed
walking facing up to the next observation, then retargets; the observed turn
frame itself keeps its previous lane. It clamps, never overshoots and can
reverse midway through another turn. This is linear sliding, not an invented
original acceleration curve. Repeated observations and pause consume zero time.
Existing World level time supplies stall caps and every fast-forward pass.
Neither frame rate nor extra observer calls change the speed.

Newly observed walkers start on their facing's lane. Other newly observed states
retain the previous clone default of 0.75; non-walking states hold their last
lane, avoiding a depth pop on a skill/state change. The original flag `+5E` and
other states' distinct depth rules are not reconstructed here; applying the
walking rule to all clone walkers and retaining depth in other states are
bounded provisional choices. X/Y discontinuity, gravity and teleport handling
from P7 are untouched. An authorized per-frame original actor+18/facing/flag and
clock trace through turning would settle duration and gating. No such run is
authorized or required for this task.

## Clone measurements and images

Final CTest evidence directories:
`build/agent-native/main/walker-lane-{baseline,production,units}/`.
Each application side records **503 frames**, turns at frame **446 / 8940 level
ms**, and includes **320 visible right-facing** and **57 visible left-facing**
walker frames. Baseline asserts Z=0.75 in both directions on all those frames.
Production asserts bounded depth steps, intermediate crossing depths, arrival
at 0.25 and simultaneous left/right walkers on opposite lanes.

At turn frame 446 the sample is `(64,32,0.75)` in file coordinates. Frame 447
is Z=0.73; frame 454 is Z=0.59. Five paused frames hold **Z=0.59 / 9100 level
ms**; the four subsequent hover queries select index 0. Crossing resumes and
arrives at **Z=0.25 at frame 476 / 9440 level ms**: exactly 500 level ms after
the turn, with the pause excluded. There are **29 intermediate-depth frames**
including paused frames. CSV origins are the production projected origins,
not sprite-art extents fitted after rendering.

Retained full clone PNG sequences: `turn-0.png` through `turn-29.png`,
`paused.png`, `settled-left.png` on both sides. Reviewed baseline settled frame
and production sequence entries 0, 12, 17, 24, 29 and settled: the leading
left-facing actor slides toward the back/inner edge while other right-facing
walkers use the front/outer edge. Early/late crossing images plus the CSV
separate radial movement from simultaneous X/Y movement and changing artwork.

Examples: [before settled](../../build/agent-native/main/walker-lane-baseline/settled-left.png),
[turn start](../../build/agent-native/main/walker-lane-production/turn-0.png),
[paused crossing](../../build/agent-native/main/walker-lane-production/paused.png),
[near end](../../build/agent-native/main/walker-lane-production/turn-29.png),
[after settled](../../build/agent-native/main/walker-lane-production/settled-left.png).

Both complete `simulation.txt` traces are byte-identical, SHA-256
**7e2e66a6a373e7abab98760be1f64fe0d790d8aa24ec86a5cb1656f610a560e8**.
They record all public lemming fields (including facing, state, timing budgets,
capabilities and teleport exclusions), stock, terrain version, clock, ticks,
explicit updates, spawned/saved/dead and outcome. The bounded route remains in
progress; this is regression evidence, not proof that the level is playable.

## Checks, provenance and preservation

Registered UCRT64 GCC 16.1, CMake/Ninja, `-O2 -g` with assertions, 12 build jobs.
Focused build logs: `build/walker-lane-001/build-1.log`, `build-2.log`.
Initial Q1 checks: `tests-1.log`, **4/4 passed in 53.59 s**. That run's evidence
was retained separately under `build/walker-lane-001/walker-lane-*` before the
additional simultaneous-lane and rapid-reversal assertions were added.
The final scoped regression log is `build/walker-lane-001/tests-final.log`:
**12/12 passed in 154.44 seconds**, including all four Q1 cases and the adjacent
unchanged motion, pose, pick, click-to-assign and shared-timing checks.
Full main/recovery verification is pending the supervisor; no full profile was
run, restarted or claimed complete by this worker.

New CTest coverage registers both production and authentic baseline application
demonstrations, source identity and lane units. Units cover both initial facings,
wall turns, exact bounded approach/retargeting, pause mid-crossing, repeated
observations, a narrow corridor reversing during a crossing and the existing
seven-pass fast-forward budget. They compare simulation against an unobserved
control World. Existing P7 motion, pose, runtime pick, click-to-assign and shared
timing tests provide the adjacent regression checks. Synthetic fixtures are
supplemental to the installed route above.

Before editing, source and CMake files were copied to ignored
`build/walker-lane-001/before-source/`. `before-hashes.json`,
`baseline-source-check.json` and `pre-existing.patch` retain provenance:
**all 277 copied source files match** the supervisor's entry manifest
`build/agent-runs/20261113T233225-17836-92532a4d/before-source-hashes.json`.
The baseline executable compiles those actual pre-task application, renderer,
tracker and simulation sources; unchanged recovered asset-decoder libraries are
shared. It is not a new flag recreating the old behavior. Its CTest identity
check pins the entire before-source manifest and checks every copied file.
Clean checkouts without the optional baseline still run production and units;
on this checkout the paired comparison is mandatory.

Final copies are archived at `build/walker-lane-001/evidence/{baseline,production,units}/`,
with trace hashes, transition measurements and image names in `evidence-index.json`.
`worker-tested-sources.json`, `task-only.patch`, `HEAD-main.cpp` and
`preservation-check.json` identify the tested changes and preserved incoming source.
`git diff --check` passes; task text is LF. Git still warns about CRLF in the
pre-existing protected queue file, which this worker did not edit or normalize.

Every new application invocation supplies a private temporary clone save store
under its own evidence directory, with default saves disabled. Complete installed
and normal clone save existence/bytes are asserted unchanged on both sides.
The supervisor's existing RepositoryLease owns this assigned worker. No manual
protected-control edit or second writer was introduced. Incoming map-name edits
and all other pre-existing changes were preserved. No queue, controller, verifier,
tool registry, permissions, shared instruction, model pin or completion-criteria
edit; no commit, push, reset, clean, stash, subagent or background worker.

Changed task files: `src/app/LemmingMotion.{h,cpp}`, `src/app/TubeScene.cpp`,
`src/__test__/test_walker_lane.cpp`, `cmake/walker_lane.cmake`,
`cmake/walker_lane_identity.cmake`, additive `CMakeLists.txt` registration and
this report. Existing assertions/timeouts were not changed.

Limitations: one installed natural turn route plus source fixtures; no all-level
or human playability claim. Original lane speed, flag/state coverage, exact
within-update turn timestamp, D1/D3 timing and C2/C3/full-frame fidelity remain
unresolved. All historical failures and parks stand.

Handoff: Q1 radial lanes and continuous turn sliding are implemented; installed clone images, identical simulation and drawn-position hover evidence are retained; needs_review.
Next action: supervisor full registered main/recovery verification, independent acceptance review, then a short optimized human Q1 playtest.
