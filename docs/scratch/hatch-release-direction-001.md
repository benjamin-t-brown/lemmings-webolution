# hatch-release-direction-001 — stored hatch directions

Status: **needs_review**. Playable v1, human playtest 5 addendum on
**MM_W_012 / Introducing - Water Lemmings**. Difficulty: difficult;
recommendation Opus, explicit Codex override used. Mandatory full registered
main/recovery verification on the final retry and separate acceptance review
are **pending supervisor execution**. The first supervisor run failed five
dependent regression checks, addressed below. This report does not accept the
task or project.

## Failure and production change

The [human report](../playtest/2026-11-14-playtest-5.md) says alternate water
lemmings should leave left, whereas every clone birth faced right. The
task-entry `World::spawn` literally assigned `Facing::Right`. The recovered
compiled-level reader already retained the necessary short and flag, but
`readHatchRecords` omitted them and the retail loader could not pass them on.
This is the next dependency for playing this installed route with the authored
release pattern; no clock or skill change is required.

Production path: `runApplication` → `runApplicationLeg` →
`RuntimeSimState` / `sim::loadRetailLevel` → `loadRetailLevelReport` →
`findLevelObjects` → `readHatchRecords` →
`lemmings::read_compiled_level` / `gadgets.entities_425e50` →
`HatchRecord.direction/alternateDirection` → `Entrance` →
`RuntimeSimState::advanceElapsed` → `World::advanceElapsedMsWithReleaseGate` →
`World::updateSegment` → `stepRelease` → `spawn`. The standalone
`advanceElapsedMs` API forwards to the same gate; the integer `World::tick`
route also uses the same spawn.
Current source anchors: `src/app/main.cpp:803` (load),
`src/assets/LevelObjects.cpp:298` / `:444` (project),
`src/sim/RetailLevel.cpp:284` (pass through),
`src/app/RuntimeHud.cpp:117` (World update),
`src/sim/World.cpp:1302` / `:1183` / `:93` (release/spawn), and
`src/app/main.cpp:1222` (actor presentation).
Existing motion, actor animation, pose selection and `TubeScene` draw the new
facing through the normal application path.

Each World owns its copied entrances. On a successful birth, `spawn` chooses
left only for direction **-1**, otherwise right. It then changes that entrance's
next direction from **1 to -1**, or **any other value to 1**, when alternation
is enabled. Other entrances retain their own phase. The original loaded
direction remains in the caller's LevelSetup; constructing a fresh World from
it resets phases. Runtime `world.setup().entrances` exposes the current next
direction, as the original mutable hatch does.

The complete legacy locator tail also projects these fields. Its historical
acceptance of truncated tails is retained with the existing hand-built default
(fixed right); that fallback is not claimed as original behavior for malformed
levels. All installed records have complete tails, and the structured and
locator projections agree on every installed hatch. Unknown fields stay
unknown; vertical-release behavior is outside this defect.

The new fields occupy existing padding in HatchRecord and Entrance. Existing
field offsets/record sizes and World's layout are preserved for authentic
historical source demonstrations. The new test pins Entrance size 16,
sheetType offset 12 and HatchRecord boxMin offset 32. No frozen source/header
was changed. No changes to release rate, release order, timing, skills,
multi-hatch scheduling, the save codec or scene rendering were made.

## Static authority

Reference: repository-local `Lemmings Revolution/Lemmings Revolution.exe`,
SHA-256 **32675bdb49866e76502713251fdfd2b09078c2481c72ca22ff68ebf4b3737a2f**.
Only the project static REA Ghidra 12.1.4 profile opened it; analysis was closed
without snapshot arguments. No original game, original instructions/oracle,
debugger, hooks, capture, installation tools or original launcher ran.

Retained research read first:
[structured hatches](native-009-structured-hatch-records.md),
[spawn coordinates](phase-a6-plus8-spawn-init.md),
[release-to-actor](phase-d1-release-to-actor.md),
[recovered emitter](../../decomp-lemmings/docs/evidence/emitter_update.md),
[hatch presentation](hatch-exit-presentation-001.md),
[accepted startup work](hatch-timing-001.md) and
[dry receiver correction](empty-receiver-fall-001.md).
Historical oracle results in those reports were not rerun and are not new
evidence for this task.

Fresh [pseudocode and full instruction reads](../../build/hatch-release-direction-001/static/)
cross-check the existing recovery:

- Reader **0x425E50**: **0x425F85..0x425F8D** reads/stores the direction word
  at hatch **+0xB4**. **0x425F97..0x425FA8** reads a byte, normalizes any
  nonzero value to 1 and stores hatch **+0xB6**. This is stored per hatch,
  not a list of per-lemming faces or the model's rotation.
- Actor constructor **0x42DE51**, instruction **0x42DFDD**, initializes
  horizontal direction **+0x40 = 1**.
- Hatch release **0x425A95**, **0x425C6E..0x425C7D**, sign-extends +0xB4,
  compares exactly to **-1**, and calls **0x42F66B** only on equality.
- **0x42F675..0x42F67E** reads actor +0x40, multiplies by -1 and writes it
  back. Remaining instructions maintain the original direction-dependent
  terrain offsets; the clone's existing facing consumers remain in use.
- **0x425C82..0x425CAC** compares +0xB6 exactly to 1, then maps +0xB4
  **1 → -1; anything else → 1**. This happens after choosing this birth's
  face and before **0x425CCC**, the actor's first update. The normalized
  serialized flag explains why a bool is appropriate at the asset boundary.

Pseudocode is provider output, not original source. The bit/width/ordering
claims above were checked against instructions; no runtime observation of the
original is claimed. The fresh release pseudocode's Evidence ID is
`ev_cb5060d61687d55b5dcc52a522a3fb4632e98808e7dda30886a946fb2b80bc41`.
The individual JSON records retain the other Evidence IDs, profile digest,
instruction extents and provider limitations.

## Installed data and actual clone births

[Source-built census](../../build/hatch-release-direction-001/assets/hatches.csv):
**102 effective installed levels / 144 hatches**. Of these, 110 are fixed
right, 22 fixed left, and 12 alternate starting right. This is read-only asset
measurement, not original execution. The test runs the real retail loader and
World for up to six births on every level, respecting levels with fewer than
six lemmings. Supplemental fixtures cover noncanonical shorts, default fixed
right, independent two-hatch phases and a fresh World's phase reset.

The same owned-process application driver runs the hash-checked authentic
task-entry World/retail-loader/hatch-projector sources and the current sources,
with common unchanged presentation support and current compatible headers.
Frozen source identities are checked by `hatch_direction_identity`; originals
are under [before-source](../../build/hatch-release-direction-001/before-source/),
with broader incoming backups/manifest under `incoming/`. This compares the
release implementation, not a historical full application build.

| Installed route | Stored direction / alternation | Before births 1–6 | After births 1–6 |
| --- | --- | --- | --- |
| MM_W_012 | +1 / true | R R R R R R | **R L R L R L** |
| MM_CLIMB_002 | +1 / false | R R R R R R | **R R R R R R** |
| MM_BASH_002 | -1 / false | R R R R R R | **L L L L L L** |

All three use the unchanged 20 ms owned frame partition. Before/after birth
clocks are exactly **2040, 4940, 7820, 10700, 13600, 16480 ms**. Tick/update
counts, newborn coordinates, state and species match exactly on all 18 pairs;
only the expected facing changes. These are clone timings, not original
release-cadence evidence. Every birth was observed as a Faller.

Retained [before](../../build/hatch-release-direction-001/before/) and
[after](../../build/hatch-release-direction-001/after/) directories each contain
six actual production PNGs and `births.csv` per level. Representative inspected
images: [water #2 before](../../build/hatch-release-direction-001/before/MM_W_012/birth-2.png),
[water #2 after](../../build/hatch-release-direction-001/after/MM_W_012/birth-2.png),
[fixed-left #1](../../build/hatch-release-direction-001/after/MM_BASH_002/birth-1.png)
and [fixed-right #1](../../build/hatch-release-direction-001/after/MM_CLIMB_002/birth-1.png).
The hatch and newborn are visible in the selected hatch-facing cameras.
[Birth comparison](../../build/hatch-release-direction-001/birth-comparison.svg)
embeds literal image crops with direction labels;
[machine-readable comparison](../../build/hatch-release-direction-001/birth-comparison.json)
records every row and image digest. The comparison script asserts all expected
patterns and unchanged non-facing birth fields. Its actual hatch-crop RGB
comparison changes **140 pixels** on water births **2/4/6**, zero on water
births **1/3/5**, zero on every fixed-right birth, and nonzero on all six
fixed-left births (147–360 pixels as earlier actors take different paths).
This verifies visible sprite changes as well as simulation-facing values;
it is not an original full-frame comparison.

## Regression corrections and development history

Existing assertions/timeouts, frozen identities and outcome checks are retained
except the following directly affected expectations/input assumptions:

1. `test_hatch_timing.cpp`: MM_BLOCK_001_A also stores alternating births.
   The former actor 1 goes left and never reaches the old second blocker
   site. The corrected winning route turns actor 1 at **(110,14)** first,
   then uses actors **0,2,4,6** at the same four physical historical sites.
   It still asserts **WIN 23/30**, genuinely earned gadget access and exact
   before/after startup-relative state/release comparison. The losing route
   uses the right-born even actors at its historical sites and still asserts
   zero saved. The historical `EMPTY_RECEIVER_BASELINE` variant keeps its
   original placements and original two-save gadget assertion.
2. The same driver's gadget route now uses actors **4,6,18** at the original
   three physical sites. It still asserts LOSS, 0 saved, full 20 births and
   unchanged earned progress. Right-born 0/2 splat at **(222,91), fall 50**;
   all ten odd/left-born actors splat at **(40,91), fall 51**. Exact identities
   and geometry replace the obsolete two-actor dry-floor set. Fast-forward
   can complete a splat between observer frames, so the check also recognizes
   Dead with the same retained floor/fall-distance operands; it does not infer
   arbitrary dead actors as receiver losses.
3. `test_fluid_hazard.cpp`: the old full-population supported-water equality
   assumed all-right release on both Worlds. The historical World stays
   frozen; the control now compares actor 0's exact supported-water motion
   without population saved/dead totals, which legitimately change when odd
   births head left. All complete population traces and dry/wet outcome
   assertions are still emitted/executed; the comparison script is unchanged.
4. `test_fluid_hazard_gadget_replay.cpp`: historical actor 11 now starts left.
   The public two-blocker replay uses right-born actors **12/14**, preserving
   its physical edge windows, blocker confirmation, no-lever loss and complete
   private-progress preservation checks.
5. `test_gravity_gadgets.cpp`: MM_BASH_002's old assertion that all nongravity
   control pixels/actor states were identical encodes fixed-right births.
   Its retained before/current traces now pin right/left birth facings,
   respectively. MM_CLIMB_002 remains the exact state/pixel control, and the
   gravity route's pre-defect equality and roof-loss checks are unchanged.

Development 1: seven new route/identity checks passed; the census incorrectly
asked two installed levels with fewer than six actors for six births. The
check now asks `min(6,totalLemmings)`; all task acceptance routes still require
six. Its failed log and original camera images are retained under
`development-1.log` / `development-1-artifacts/`.
Development 2 passed the eight new checks plus World/retail/asset units but
exposed the old fixed-ID hatch route assumptions. Development 3 passed 20/21
relevant checks (basher, terrain, animation, fluids, dry receivers and new
directions); the remaining full-population water control was corrected as
described above. No full verifier profile was duplicated.

The ignored `probe.cpp` / `probe-*.txt` are supplementary source simulation
experiments that explain the revised placements and left/right loss paths.
They do not establish public-input playability or original fidelity. Actual
frontend regression evidence and final focused check results follow below.

## Completed checks on the final implementation

Registered UCRT64 incremental build with **at most eight jobs**:
[final-build.log](../../build/hatch-release-direction-001/final-build.log), exit 0.
The existing main configuration remains `-O2 -g`, assertions enabled.
[final-focused.log](../../build/hatch-release-direction-001/final-focused.log)
passes **16/16 in 275.98 seconds**: nine direction identity/asset/application/
comparison checks, World/retail/level-object units, both current-simulation
startup route variants and both gravity demonstrations. The real-data direction
test itself reports **1658 checks, zero failures**.

The comparison script subsequently gained the crop RGB assertions described
above; its separately registered final run is retained as
[final-comparison.log](../../build/hatch-release-direction-001/final-comparison.log).
No production or compiled test source changed after the successful focused run.

Additional completed checks in
[development-3-console.log](../../build/hatch-release-direction-001/development-3-console.log)
include Basher, Bomber/Builder/Digger terrain presentation, Bomber/Builder/Basher
animation, fluid rendering and both empty-receiver demonstrations. The full
21-test selection passed 20 checks and failed the then-obsolete full-population
water control. In
[development-4.log](../../build/hatch-release-direction-001/development-4.log),
both fluid-hazard variants, the corrected exact water control, public gadget
replay and authentic historical hatch replay pass. That development selection
still fails its two current hatch variants' transient-splat-only observer and
the old fixed-right gravity comparison; those corrections are covered by the
16/16 final run. Failed development logs remain historical, not full passes.

Final connected startup comparisons:

| Route | Before startup / after startup | Confirmed clicks | Outcome on each |
| --- | --- | --- | --- |
| MM_BLOCK_001_A, winning sites | 2200 / 4200 ms first birth | 5 | WIN, 23/30 |
| MM_W_012, earned gadget | 2200 / 6000 ms first birth | 3 | LOSS, 0/20; 12 exact dry-floor losses |
| MM_BLOCK_001_A, losing sites | 2200 / 4200 ms first birth | 4 | LOSS, 0/30 |

All **80 birth offsets** agree exactly. Every compared actor/stock/gadget state
agrees before the current route's absolute timeout: **538 / 485 / 538 rows**,
respectively. The baseline has two additional late samples per route because
its earlier start reaches the absolute deadline later relative to birth; whole
trace files are deliberately not represented as byte-identical. The registered
driver compares the current pre-timeout prefix, retaining both complete traces.
[Condensed comparison](../../build/hatch-release-direction-001/frontend-regression-comparison.json)
records those bounds, matching offsets and matching outcomes.
Full route traces, clicks, births, PNGs and summaries remain under
[routes-before](../../build/hatch-timing-001/routes-before/) and
[routes-after](../../build/hatch-timing-001/routes-after/).
The inspected final [winning Results image](../../build/hatch-timing-001/routes-after/winning-placements/results.png)
shows 23 saved against 15 required.

The authentic historical all-right/dry-lid hatch replay still passes with its
original four-click 23/30 win and two-save gadget loss
(`empty_receiver_hatch_before`, development 4, **135.35 seconds**).
The final current public two-blocker gadget replay confirms actors **12/14** at
**(214,41)/(165,41)**, attachment ticks **338/416**, LOSS with **0 saved / 14
dead**, and byte-preserved private progress. Its
[summary](../../build/fluid-hazard-001/gadget-replay/summary.txt) and
[inspected loss frame](../../build/fluid-hazard-001/gadget-replay/dry-pit-loss.png)
retain this current-direction result; it passes in **35.83 seconds**.

`git diff --check` passes on task-modified tracked files. All 14 initial task source/
support/report files use LF. Incoming `src/app/main.cpp` and
`cmake/webolution_sources.cmake` hashes remain unchanged. Task-only patch and
final source hashes are retained under the task build directory. These paragraphs
describe the initial worker invocation; no full verifier profile ran in it.

## Retry after supervisor verification failure

The supervisor's exact
[verification log](../../build/agent-runs/20261115T175318-37880-81129669/verification.log)
records **292/297 main checks passed, five failed**. Recovery verification was
not reached. That failure remains historical evidence. The resumed worker
preserved the implemented facing rule, authentic frozen sources and all prior
logs; its incoming backups and subsequent logs are under
[retry-2](../../build/hatch-release-direction-001/retry-2/).

Two further dependent expectations needed correction:

6. Both paired climber snapshot/application drivers used natural actor 0 on
   **MM_FLOAT_CLIMB2**, whose first hatch stores **-1 / fixed**, at (235,87).
   The old all-right birth reached the outer wall at (243,9); the corrected
   left-born actor walks inward and dies without climbing. A source simulation
   probe confirms this dependency rather than manufacturing a new release or
   changing that level. The second installed route is now **E22_KF_1**, which
   has actual Climber/Floater stock and naturally reaches a right-facing climb
   at **8400 ms, (136,39)**. The paired old/current renderers still compare
   identical simulation states, 48 views of each route in both tube-only and
   complete scenery, all per-pixel visibility/depth checks, and **288 unchanged
   non-climber frames**. View 4 still requires the fully covered sprite to have
   zero pixels/no pick box after the correction. On E22_KF_1 the explicit fully
   exposed snapshot uses **view 42** (315 degrees) rather than view 40, where
   neighbouring terrain legitimately covers part of the actor. This preserves
   the entire-raw-sprite/pickability assertion: full-scene view 4 changes
   **102 -> 0 actor pixels**; view 42 remains **112 -> 112**. The public route
   retains its exposed view-40 hover check, where the visible portion is
   pickable, its covered view-4 check, actual stock spending, pause, full turn,
   exact world/camera/stock equality and exact no-climber PNG comparisons.
   Both E22_KF_1 application variants record **137 frames**, **74 no-climber
   frames**, **48 rotations**, and one covered/exposed hover check each.
   Earlier accepted MM_FLOAT_CLIMB2 images remain untouched under
   `build/climber-occlusion-001/evidence/`; new route output is the ordinary
   `before/after/climber2[-full-scene]` and `app-before/app-after/E22_KF_1`.
7. `frame_profile_MM_W_012` pinned a performance-era image containing the
   defective all-right births. Its expected PNG SHA256 changes from
   **3633704696154420f7a8edf02eb20fe109c6e32669867e20a0f8a1feca8c4b40** to
   **03c498d568daedf2eb74d90e321b3233178dee70e6f51bddd237bccaa57df38d**.
   This is independently justified by new registered
   `hatch_direction_profile_before/after` demonstrations: the authentic task-entry
   release/loader sources and current sources run the same `--direct-level
   --level MM_W_012 --native --sim-ticks 50 --frames 150 --no-audio` application
   inputs with private save stores. The before build reproduces the **exact old
   hash**; the after build reproduces the **exact new hash**. Their literal
   [before](../../build/hatch-release-direction-001/profile-before/frame.png) /
   [after](../../build/hatch-release-direction-001/profile-after/frame.png) images
   differ on **59 RGB pixels**, bounded by **(511,53)..(515,71)** at the actor;
   all other **307141 pixels match**. The
   [comparison](../../build/hatch-release-direction-001/retry-2/profile-comparison.json)
   retains hashes, counts and bounds. The fixed-right MM_CLIMB_002 golden and
   both performance demonstrations' 150-frame/count/time-limit checks remain
   unchanged. Messages now call these expected regression frames, avoiding the
   inaccurate claim that the water frame still matches its pre-fix image.

No production implementation changed during this retry. The supplementary
`climb-probe.cpp` and `climb-candidates-scenery.txt` explain the replacement
route. A first candidate, H08_MT_1, completed public actions but had ambient
random sprite differences and a partially exposed view-4 snapshot; it was
rejected, preserving exact pixel assertions. `focused-1.log` and `failed-H08/`
retain those failures. `focused-2.log` records the E22_KF_1 public passes and
the two snapshot failures from still using the old view-40 fully-exposed
assumption; the final source selects the geometrically exposed view 42.

Full registered main/recovery verification remains pending supervisor execution
on this final content. The resumed worker did not duplicate a full suite.

The final registered focused selection passes **18/18 in 140.73 seconds**:
[final-focused.log](../../build/hatch-release-direction-001/retry-2/final-focused.log).
It includes all five formerly failing checks, both authentic-source/new profile
frames, all six actual six-birth routes, both source identities, the full
real-data census/boundaries (**1658 checks, zero failures**) and the birth/image
comparison. Incremental builds use the registered GCC/CMake configuration and
**at most eight jobs**; `build-2.log`, `build-3.log` and `final-build.log` retain
the final changed-driver builds. Existing assertion counts and timeouts remain
unchanged except the explicitly documented camera/level/golden expectations;
two additional profile-frame regression tests are registered.

Selected new climber states, rotation traces, summaries and full-scene
covered/exposed images are retained in
[retry-2/climber-evidence](../../build/hatch-release-direction-001/retry-2/climber-evidence/).
`retry-2/final-source/` and `final-source-hashes.json` preserve the final task
files separately from the authentic task-entry sources. All **19 task files**
use LF and scoped `git diff --check` passes. The five production files are
byte-identical to the initial worker's checked implementation; the retry changes
only dependent tests, their registration/support and this report. The existing
supervisor lease remains PID 37880. Shared controls and unrelated incoming
edits were preserved.

## Preservation, limits and handoff

The supervisor owns RepositoryLease **PID 37880** throughout this worker.
Initial CIM process enumeration was denied; Get-Process and the lock record
identified the existing runner, with no clone/compiler/CTest live before work.
No second lease/runner, subagent, concurrent writer, detached work, commit,
push, reset, clean or stash was used. Existing unrelated edits, original hook
log, protected controls, model pins and shared instructions were preserved.
All task source/report changes use LF; runtime text artifacts may use CRLF.

The demonstrations use private `FileSaveSink` paths under this task's build
directory, never the installed or normal clone save. Both real saves are
snapshotted and checked unchanged. No real save was imported or reset.
Existing frontend regression routes continue using their own private saves.

Task-changed production files: `src/assets/LevelObjects.h/.cpp`,
`src/sim/World.h/.cpp`, `src/sim/RetailLevel.cpp`.
Supporting changes: `CMakeLists.txt`, `cmake/hatch_direction_identity.cmake`,
`src/__test__/test_hatch_direction.cpp`, `test_hatch_timing.cpp`,
`test_fluid_hazard.cpp`, `test_fluid_hazard_gadget_replay.cpp`,
`test_gravity_gadgets.cpp`, `test_climber_occlusion.cpp`,
`test_climber_occlusion_app.cpp`, `cmake/hatch_direction_profile.cmake`,
`cmake/frame_rate_profile.cmake`, `cmake/frame_rate_profile_run.cmake`,
`tools/compare_hatch_direction.py` and this report.

There is no provisional facing rule for supported installed data: the static
trace settles its initial direction, per-hatch ownership and post-birth toggle.
A visible original MM_W_012 recording of the first six births would provide
independent runtime confirmation of **R L R L R L**, but none was executed or
claimed here. Multi-hatch release counts/scheduling, native cadence, other
timing parks, fidelity gates and completion criteria remain unchanged.
Scripted wins are regression evidence, not a human playability pass.

Handoff: stored hatch directions implemented; status **needs_review**; full registered verification pending supervisor execution.
Next: supervisor main/recovery verification and independent acceptance review, then human confirmation of MM_W_012 emergence.
