# lemming-pick-nearest-001: nearest cursor-centre selection

Status: **needs_review**. Playable v1, [playtest 6 V7](../playtest/2026-11-15-playtest-6.md),
per the implementation plan and Project lead direction. Difficulty: routine;
recommended Sonnet, executed with the assigned Codex override. Mandatory full
registered verification and independent acceptance review remain with the supervisor.

The demonstrated defect was selection favouring a nearby lemming on one side
instead of the one nearest the cursor dot. The next dependency was the shared
production hover/click ranking, rather than simulation or skill retuning.
The recovered original ranking used state priority and terrain-target distance;
that ranking is intentionally preserved as historical evidence while the clone
uses the requested, **provisional** nearest-screen-centre policy.

## Production path and policy

`runApplication -> runApplicationLeg -> renderFrame` prepares the installed
level, current actor samples, camera and completed scene depth.
`TubeScene::injectedWalkerScreenBoxes -> prepareInjectedWalker` supplies each
visible sprite's placement using the same animation, horizontal mirror,
gravity reflection and scene-depth checks as drawing. `hoveredActor` scans live
World actors in ascending index order and applies the existing stock, active,
known Climber-exclusion and bounded first-level support gates.

Both the framebuffer compositor (which sets `hoveredActor` and `markedActor`)
and the SDL left-button handler call this same `hoveredActor`. The button path
uses the displayed frame's camera, samples and depth before advancing the World,
then calls `World::assign` with the returned actor index. The existing skipped-frame
depth refresh remains intact. Presentation maps pointer events to the same
640x480 game coordinates at every supported window scale.

The new `RuntimePick::pickNearestScreenLemming` ranks the remaining candidates by

```
cx = (placement.left + placement.right) / 2
cy = (placement.top + placement.bottom) / 2
distance2 = (cx - cursorX)^2 + (cy - cursorY)^2
```

It uses the **unrounded displayed placement**, not the feet/origin, truncated
hit-box centre, terrain coordinates, camera target or actor facing. This also
handles the reflected upside-down box. Exact squared-distance ties keep the
first candidate, therefore the **lowest World actor index** in production.
State priority is deliberately absent from this provisional ranking.

Candidate admission is unchanged. The 32x32 cursor box still passes through
the recovered inclusive overlap test: body box overlaps cursor X +/-12 and
Y +/-20, after the original +/-4 adjustments. Existing visibility/occlusion,
skill stock, support and eligibility gates are unchanged. `Pick.cpp`,
`runtimePickCandidate`, `knownPickPriority`, simulation and skill rules are
unchanged; the old native-policy unit tests retain their assertions.

## Original evidence, distinct from clone policy

Only the registered local reference is relevant:
`Lemmings Revolution/Lemmings Revolution.exe`, x86, image base `0x400000`,
SHA-256 `32675bdb49866e76502713251fdfd2b09078c2481c72ca22ff68ebf4b3737a2f`.
No original game, copied original instructions, debugger, hooks, original
capture or runtime oracle ran. No new REA analysis was needed: retained static
instruction evidence answers the bounded ranking question.

Starting evidence: [D2 pick rule](phase-d2-pick-rule.md),
[controller-target identification](first-level-overlap-selection.md),
[visibility/support](first-level-pick-visibility-support.md),
[runtime shared hover](runtime-cursor-hover.md),
[hover feedback](hover-feedback-001.md), and the accepted
[inverted skill integration](antigrav-skill-assign-001.md).
The raw retained instruction reads are
`build/tmp/d2a-pick-rule/reads.txt`; the later controller dataflow packet is
`build/tmp/enacter-pick-reference/`.

* `454CB9..454D18`: inclusive adjusted box overlap.
* `455194..4551AA`: call `45A0E0` with the stock array and candidate; reject
  a zero applicability/stock result before ranking. This is an admission gate,
  not evidence that screen distance determines the winner.
* `4551AB..4551EB`: subtract actor `+8` and the controller's `+58` target vector,
  then compute the three-component dot product.
* `4551EE..455203`: higher state priority wins outright (`jg` at `4551F4`);
  otherwise strict smaller distance wins (`fcomp`, FPU C0 test and skip on
  equality). An exact tie retains list order.

The new registered `pick_nearest_reference` rechecks the exact local EXE hash
and the 23-byte skill-gate and 21-byte priority/distance instruction spans
against those retained reads. These are read-only data measurements, not
execution of the instructions. The requested nearest-screen-centre policy
therefore **differs from the traced original** and is labelled provisional in
the production code. Native general applicability remains only partially mapped
in the existing clone. This task preserves those limits.

An authorized original observation holding a crowded view fixed, moving the
cursor across the two body centres, and recording selected actor, selected
skill, state priority, actor positions and controller target would establish
the visible original behaviour across those gates. None was performed or is
required to adopt the user's requested clone policy. Historical fidelity gates,
parks and full-project completion criteria are unchanged.

## Clone evidence and provenance

The authentic task-entry `src/`, `cmake/` and `CMakeLists.txt` were copied to
[before-source](../../build/lemming-pick-nearest-001/before-source/) before any
task edits. [Manifest](../../build/lemming-pick-nearest-001/before-manifest.json),
incoming status/diff and a separate HEAD main are retained beside it. The
baseline uses the actual incoming main, including accepted uncommitted work,
not HEAD or resumed modified source. Its SHA-256 is
`66f2070a6388d55be8e6abfe9032f47e9ed5174a23219e926162c904399bf2a0`, also recorded in
the supervisor's `build/agent-runs/20261115T232323-39368-dab7e8e7/before-source-hashes.json`.
`pick_nearest_identity` checks it. Before and after link the same unchanged
scene/simulation support; only the before main retains the former call path.

The registered paired drivers use public SDL pause, panel selection, pointer
motion and same-pointer click through `runApplication`. `--sim-ticks` prepares
natural actor states from installed level data; no actors, terrain or stocks
are fabricated. Each of seven scenarios starts a fresh source-built application.
These prepared clone routes are regression evidence, not ordinary frontend
playthroughs, level wins, human playability acceptance or original comparisons.

| Installed route | Pointer | Before -> after selected actor | Coverage |
| --- | --- | --- | --- |
| `MM_CLIMB_002`, tick 100, left of both candidate centres | 408,284 | 5 -> 3 | Two candidates, both walking directions; nearer squared distance 251.753 vs former 254.722. |
| Same, right of both centres | 422,292 | 5 -> 3 | Cursor almost at actor 3's centre: 0.0665 vs former 240.267. |
| Same, between centres | 421,292 | 5 -> 3 | 0.6104 vs former 214.648. |
| `MM_CLIMB_002`, tick 98, overlapping opposite-facing sprite boxes | 362,298 | 0 -> 2 | Actual installed-level sprite overlap, with deterministic nearest choice. |
| `m06_kf_1`, tick 45, naturally inverted Walker after gravity flipper | Left 513,351; right/centre 514,351 | 0 -> 0 | All three clicks assign Blocker to the highlighted upside-down actor; stock 30 -> 29. |

On every before and after route, the highlighted actor alone receives the job,
exactly one stock is spent, and successful assignment resumes from pause. The
new comparison independently recomputes nearest distance from retained displayed
geometry, confirms pre-click geometry/state/eligibility and pointers match
between sides, and requires real policy disagreements to select a strictly
closer actor. Both left- and right-facing selections are exercised. The inverted
controls have one eligible actor; they establish reflected-centre input and
assignment, not a crowded upside-down original comparison.

Paired `hover.png`, `assigned.png`, raw RGBA, per-actor `selection.csv` and
`summary.txt` are retained for every route in the immutable worker archive:
[before](../../build/lemming-pick-nearest-001/worker-evidence/before/),
[after](../../build/lemming-pick-nearest-001/worker-evidence/after/), and
[independent comparison](../../build/lemming-pick-nearest-001/worker-evidence/comparison.json).
Representative images:
[right-side before](../../build/lemming-pick-nearest-001/worker-evidence/before/upright-right/hover.png),
[right-side after](../../build/lemming-pick-nearest-001/worker-evidence/after/upright-right/hover.png),
[overlap after](../../build/lemming-pick-nearest-001/worker-evidence/after/upright-overlap/hover.png),
[inverted hover](../../build/lemming-pick-nearest-001/worker-evidence/after/inverted-between/hover.png),
[inverted assigned](../../build/lemming-pick-nearest-001/worker-evidence/after/inverted-between/assigned.png).
The right-side crowd comparison
changes 146 pixels, all inside the two selected sprite boxes; inverted control
hover images are byte-identical. The worker visually inspected the paired
right-side hover images, the overlapping-pair image, and inverted hover and
assigned images. The cursor covers actor 3 while the before image brightens
actor 5 off to its left; after, the actor under the dot brightens. The inverted
assignment displays the Blocker stock decrement and the ceiling actor's job.

Every driver injects its own `FileSaveSink` under the test output directory.
Installed, normal clone and private save byte/existence snapshots remain
unchanged. The normal and installed saves are never opened for writing.

## Existing expectations, checks and limits

`test_leveloverlap.cpp` formerly predicted terrain-target distance. That
expectation encoded V7 and now independently measures screen-centre distance;
its non-first selection, hover, assignment, pause/resume, stock and same-cursor
retry assertions remain. `test_hover_feedback.cpp` now uses the same new
independent screen oracle. Its historical frozen pre-feedback comparison
chooses a multiple-candidate point where the two ranking policies agree, so
its paired state/stock and precise feedback pixel checks remain meaningful.
The new dedicated task routes require policy disagreements separately. No
assertion, timeout, comparison tolerance or historical criterion was weakened.

The new unit cases cover co-located overlapping boxes, exact ties, stable order,
fractional centres, full 2D distance, unchanged inclusive radius edges,
ineligible and outside-radius candidates, empty input and no hit. The installed
pair route supplements those synthetic cases.

Initial development logs are retained. `ctest-1.log` exposed an overconstrained
test fixture requiring overlapping bodies at tick 100, rather than shared
cursor candidacy; `ctest-2.log` also showed a later gravity pre-roll had no
supported inverted crowd. The final fixtures use tick 98 for actual upright
overlap and tick 45 for the known natural ceiling route. No production timing,
release or skill rule was changed to manufacture those states. `ctest-3.log`
passes 26 focused checks; `ctest-4.log` passes the expanded geometry/pixel
comparison and reference checks. These logs do not establish mandatory full
main/recovery verification.

Final focused development checks: **35/35 pass, 114.06 seconds** with registered
MinGW-w64 GCC, `-O2 -g` and assertions, SDL dummy video/audio, eight build jobs
and one test job. [Build log](../../build/lemming-pick-nearest-001/worker-evidence/build-5.log),
[CTest log](../../build/lemming-pick-nearest-001/worker-evidence/final-ctest.log),
[JUnit](../../build/lemming-pick-nearest-001/worker-evidence/final-ctest.xml).
This includes all 18 new task checks; existing `pick`, `runtimepick`,
`clicktoassign`, `leveloverlap`, `levelvisibility`, `levelcursor`,
`runtimeskillpanel`, all eight hover-feedback checks, `actor_motion_production`
and `frame_consumption`. No full suite was duplicated. Mandatory full
main/recovery verification remains pending supervisor execution.

The archive has a [SHA-256 manifest](../../build/lemming-pick-nearest-001/worker-evidence/sha256.json).
[worker-only.patch](../../build/lemming-pick-nearest-001/worker-only.patch)
isolates this task from the incoming working edits.
All changed text is LF; `git diff --check` passes for the task paths. A final
comparison of every backed-up incoming source file confirms that only the six
intended existing source/CMake files differ; incoming World and window work is
byte-identical to task entry.

Changed production files: `src/app/main.cpp`, `src/app/RuntimePick.{h,cpp}`.
Test/evidence support: `src/__test__/test_pick_nearest.cpp`,
`src/__test__/test_leveloverlap.cpp`, `src/__test__/test_hover_feedback.cpp`,
`cmake/pick_nearest{,_identity,_reference}.cmake`,
`tools/compare_pick_nearest.py`, one CMake include and this report. Incoming
window, World, hook-log, prior task, queue and generated-status edits were
preserved. The live supervisor lease belongs to PID 39368; this worker made
no protected-control or shared-instruction edit. CIM enumeration was denied;
ordinary `Get-Process` supplied the process check. No subagent, detached work,
tool installation, commit or push was started.

Unknowns: general native skill applicability and exact original visible
selection across all states remain incomplete; nearest-centre is the explicit
clone policy. Body centre means the full displayed sprite rectangle, rather
than the centroid of its nontransparent or unoccluded pixels. No broader
eligibility, depth, animation, simulation or original-fidelity work is claimed.

Handoff: lemming-pick-nearest-001 is **needs_review**; shared nearest-centre selection, paired clone evidence and 35/35 focused checks are ready.
Next action: supervisor mandatory full registered verification, then independent acceptance review and human playtest confirmation.
