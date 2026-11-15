# lemming-type-sprites-001: species sprites and fluid drowning

Worker status: **needs_review**. Playable v1, playtest 5 **U4/U2**. Difficulty:
difficult; assigned recommendation Sonnet-class, executed with the explicit
Codex override. Full registered main/recovery verification and acceptance remain
the supervisor's responsibility after worker exit. No original execution,
original-byte oracle, debugger, hooks, capture, install tools or subagents ran.

## Failure and next dependency

On **MM_W_012**, Introducing - Water Lemmings, the simulation already carries
sheet type 1 and exempts it from water hazards, but the production scene loaded
only LEMMINGSHI and drew its green-haired ordinary frames for every type. A player
could not identify the water lemmings. On **MM_BUILD_BLOCK_A**, Build and Block,
the ordinary actor correctly entered Drown, played the installed strip, then
held its last frame at the surface until disappearing. Sinking, the brightened
figure and fluid effects were absent. These are the demonstrated U4/U2 defects;
the next dependency is their presentation, not a simulation/timing change.

The [human original-session observations](../playtest/2026-11-14-original-session.md)
and retained [drowning reference](../playtest/reference/original-2026-11-14/s2-drown-zoom.jpg)
show teal hair/pale clothes for water lemmings and about two seconds of surface
flailing, droplets, a pale sinking figure and bubbles. These are previously
authorized human observations, not an original run by this worker or a pixel
fidelity measurement. Retained [fluid-hazard research](fluid-hazard-001.md),
[animation research](actor-anim-001.md) and [actor rendering lessons](../lessons/actor-rendering.md)
provide the connected simulation and rendering evidence.

## Resume, provenance and static authority

Attempt 1 was interrupted by host memory pressure, not rejected gameplay. Its
partial source edits, frozen source and static JSON were reused. Incoming edits
are backed up under `build/lemming-type-sprites-001/resume-entry/`. The authentic
`before-source/src/app/` contains AppEntry.h, LemmingAnim.cpp/.h, TubeScene.cpp/.h
and main.cpp. All six byte-match HEAD source, as recorded in
`build/lemming-type-sprites-001/provenance.json`; HEAD is
`c96bb6998bb056cdd2fc7368b7826c4048f1d604`, following source commit f62b3b8
and two queue-only commits. No resumed modified source was substituted for a
baseline. The added `lemming_type_sprites_identity` CTest pins all six hashes.

Reference: repository-local `Lemmings Revolution/Lemmings Revolution.exe`, x86,
SHA-256 **32675bdb49866e76502713251fdfd2b09078c2481c72ca22ff68ebf4b3737a2f**.
This resume used retained static REA results, including complete instruction
lists, without opening a new analysis session. Their evidence IDs and hashes
are indexed in provenance.json. Pseudocode was checked against those retained
instructions; none of the original instructions was executed.

* **Loader 0x4437F6..0x44389D:** [actor_resource_selection.md](../../decomp-lemmings/docs/evidence/actor_resource_selection.md)
  and its instruction file establish categories 0 ordinary, 1 water, 2 acid,
  3 EvilRat, with Hi/Lo quality suffixes and IDs in **0x4E335C**. The clone's
  existing Hi-quality choice is preserved; no quality/performance work is added.
* **Constructor 0x42DF75..0x42DF89:** reads the constructor type, loads
  `4E335C[type]`, and binds it to actor+0x128 through 0x46836D. **0x42E05F**
  stores that same type at actor+0x4C. Retained `asm-0x42DE51.json`, evidence
  `ev_9b64edaf57979b5f6f0a40deff5b968973ef3152c3a3f72f29ef5fd74dea9b81`,
  corroborates `constructor-pseudo.json`. This is the selection rule, rather
  than an inference from the names/colors of texture files.
* Existing state entry evidence summarized in actor-anim-001 selects action
  names on that bound controller. The same per-state names can therefore use
  the actor's own sheet; the state mappings' earlier provisional limits remain.
* **Fluid entry 0x404261:** instruction **0x40427F** pushes string **0x4BF118**
  (`drown` in retained `drown-strings.json`), followed by controller calls at
  0x404290/0x4042A1. `asm-0x404261.json` is evidence
  `ev_4cf9dde17166ae37a7cd7ea7c98207713c55bdd6be1845d4252e1147a9d81798`.
* **Fluid step 0x404372:** **0x4043BC..0x404400** computes
  `min(1, (actorY - receiver.bottom) / receiver.high) * 255`;
  **0x404403..0x404434** sets selector 2 and equal vertex RGB.
  **0x404655..0x404675** writes `Y -= verticalDir * 20 * dt / 2`.
  `asm-0x404372.json`, evidence
  `ev_a1a7a896a480ee5844853a0ef45f54521982dc5c2ecba85d356c412d3d7ff261`,
  corroborates `fluid-drown-pseudo.json`, evidence
  `ev_bc0f70e00a99ca2e4396ca3b25046db72203789fa1c22a25da621349b10cef9b`.
  Its effect branch creates Bubbles when submerged by 5..10 units, using
  randomized emission. [Retained material research](phase-a4-actor-render-state.md)
  identifies selector 2 as ONE/ONE additive blending; effective original depth
  state remains unknown. The RGB/selector setters are retained separately.

## Installed sprite inventory

Registered read-only measurements are `before/assets.tsv` and `after/assets.tsv`
in the evidence snapshot linked below. They list every clip, temporal count,
direction count, rate and exact referenced texture filename, including Lo assets
for inventory only. Production uses these Hi assets:

| Type | SPRITES asset | Referenced texture family under TEXTURES/SHEETS |
| --- | --- | --- |
| 0 ordinary | LEMMINGSHI.SPR | $LEMSHEETHI*.BMP |
| 1 water | LEMMINGSWATERHI.SPR | $LEMSHEETWATERHI*.BMP |
| 2 acid | LEMMINGSACIDHI.SPR | $LEMSHEETACIDHI*.BMP |
| Water drowning effect | BUBBLES.SPR, `bubbles` | _BUBBLES0.BMP |

`Sprite::animations` indexes time; each entry's `frames` indexes direction.

| State / strip | Ordinary entries | Water entries | Acid entries | Directions / rate |
| --- | ---: | ---: | ---: | --- |
| Walker / walk | 12 | 12 | 12 | 5 / 1 |
| Faller, Bounce / fall | 15 | 15 | 15 | 5 / 1 |
| Climber / climb | 18 | 18 | 18 | 5 / 1 |
| Hoister / flip | 14 | 15 | 15 | 5 / 1 |
| Floater opening / umbopen | 8 | 8 | 8 | 5 / 1 |
| Floater open / float | 15 | 15 | 15 | 5 / 1 |
| Blocker / block | 16 | 16 | 16 | 8 / 1 |
| Builder / build | 16 | 16 | 16 | 5 / 1 |
| Shrugger / shrug | 8 | 8 | 8 | 1 / 0.5 |
| Basher / bash | 24 | 24 | 24 | 5 / 1 |
| Miner / mine | 25 | 25 | 25 | 5 / 1 |
| Digger / dig | 20 | 20 | 20 | 5 / 1 |
| Bomber pre-blast / explode | 17 | 17 | 17 | 5 / 1 |
| Drown / drown | 17 | 17 | 17 | 5 / 1 |
| Teleporting / cartwheel | 16 | 16 | 16 | 8 / 2 |
| Exited, Dead | no exit/dead strip | none | none | Existing omission/handoff unchanged |

Water's serialized order is **drown at 4, dig at 5**, the reverse of ordinary
and acid. Selection uses names, not a common numeric strip index. Trackers also
read their own strip lengths: the regression reaches flip entry **13** for
ordinary and **14** for water/acid. No frame list or simulation skill timing is
retuned. Splatter's held fall pose, Blocker mapping and existing clip loop/hold
choices retain the individual provisional labels in LemmingAnim.

The skill panel keeps its fixed ordinary job icons, as established by
[panel-icon-anim-001](panel-icon-anim-001.md); no evidence establishes a
hover-selected species changing those icons. Actor hover is different:
the retained **0x454ACA..0x454B5C** copy/redraw uses the selected actor's
controller. `drawHoveredWalker` now prepares the same per-type sprite as the
world view. The existing provisional highlight intensity remains unchanged.

## Connected implementation and drowning corrections

Species production path:
`readHatchRecords -> RetailLevel::Entrance::sheetType -> World::spawn ->
Lemming::sheetType -> main.cpp::LemmingAnimTracker::advance(per-type sheets) ->
injectedWalkersFromWorld -> TubeScene::drawInjectedWalkers ->
prepareInjectedWalker -> drawActorSprite`.
Screen boxes, picking and hover use the same sheet/clip/direction preparation.
Each scene loads all three named sheets and the bubble sheet. Missing/invalid
types are not silently drawn as ordinary; missing sheets are scene diagnostics.

For fluid drowning, the tracker keeps presentation-only seconds in the current
Drown state, advanced with the already accepted level animation budget. Pause
passes zero; fast-forward uses its existing budget. The adapter finds the live
receiver at the simulation contact point and moves only the **draw sample**
downward at the statically traced 10 units/s. It computes neutral brightness
from height above its bottom. The actual `World::Lemming` remains at its existing
contact position; `stepDrown`, its duration, counters and skill/hazard rules are
untouched. Static-cell drowning with no fluid receiver retains its old path.

The before/after strips establish these corrections:

1. **No wrong clip, frame order or rate was demonstrated.** Both use the
   installed 17-entry `drown` strip and the existing 0.06-second controller
   threshold; it reaches entry 16 then holds. SPR frame offsets, view selection
   and anchoring are preserved. The failure was holding that pose at the
   surface, not evidence for reversing/reordering/retiming the strip.
2. **Missing descent:** the production draw Y changes while simulation Y does
   not. On Build and Block the first actor stays at `(176,67)` in the World;
   its original-coordinate draw sample descends from **27 to 9** over the
   retained drowning samples. Receiver bottom is 4, mean surface 28.5.
3. **Missing bright/fading figure:** only fluid drowning uses neutral
   height-dependent RGB and additive composition, replacing ordinary position
   lighting/opaque replacement for that effect. It becomes pale against water
   and fades as it descends.
4. **Missing droplets/bubbles:** the installed eight-entry, one-direction
   BUBBLES strip supplies two small early droplets and subsequent rising
   bubbles. This deterministic two-particle approximation, its scales,
   trajectories and 0.6-second split are explicitly **provisional**, rather
   than a reconstruction of the original random emitter or body fragmentation.
5. **Submerged visibility:** main retains opaque-scene depth around the fluid
   pass and includes later secondary depth writes. Only the additive drowning
   figure/effects use that depth, so water can show the figure while opaque
   scenery still masks it. They also omit the ordinary 4-unit supporting-wall
   contact allowance, retaining the traced normalized bias. This is a bounded
   **provisional U2 policy**, not a change to ordinary actor depth, the fluid's
   accepted Less/write-on policy, clipping gates or original device-state claims.

The clamp at the receiver bottom and retention of the clone's existing
two-second death timer are provisional adapters: the original type-0x23 state
can remove the actor on reaching the bottom. No timing/lifetime contract was
rewritten to match that different original state.

## Clone images and regression evidence

Final retained snapshot:
[`build/lemming-type-sprites-001/worker-resume-20261115T0424/`](../../build/lemming-type-sprites-001/worker-resume-20261115T0424/).
The supervisor can regenerate root `before/after/`; the worker snapshot and its
manifest preserve this review's evidence. All images are source-built clone
framebuffers or read-only installed-frame strips, not original capture.

| Case | Before | After / meaning |
| --- | --- | --- |
| Natural water walking/falling on MM_W_012 | [strip](../../build/lemming-type-sprites-001/worker-resume-20261115T0424/before/MM_W_012/state-strip.png) | [strip](../../build/lemming-type-sprites-001/worker-resume-20261115T0424/after/MM_W_012/state-strip.png), teal hair/pale body replaces green/blue |
| Publicly assigned water Blocker | [strip](../../build/lemming-type-sprites-001/worker-resume-20261115T0424/before/water-skill/state-strip.png) | [strip](../../build/lemming-type-sprites-001/worker-resume-20261115T0424/after/water-skill/state-strip.png), walk/fall/block columns; [hover](../../build/lemming-type-sprites-001/worker-resume-20261115T0424/after/water-skill/hover.png) |
| Natural acid on installed M20_MT_1 | [strip](../../build/lemming-type-sprites-001/worker-resume-20261115T0424/before/M20_MT_1/state-strip.png) | [strip](../../build/lemming-type-sprites-001/worker-resume-20261115T0424/after/M20_MT_1/state-strip.png), orange hair/green body; no skill column on this unassigned route |
| Natural Build and Block drowning | [20 samples](../../build/lemming-type-sprites-001/worker-resume-20261115T0424/before/MM_BUILD_BLOCK_A/drown-strip.png) | [20 samples](../../build/lemming-type-sprites-001/worker-resume-20261115T0424/after/MM_BUILD_BLOCK_A/drown-strip.png), bright surface phase then sinking/fade/effects |
| Placed species/state controls on MM_W_012 | [all ordinary](../../build/lemming-type-sprites-001/worker-resume-20261115T0424/before/MM_W_012-type-states.png) | [three types](../../build/lemming-type-sprites-001/worker-resume-20261115T0424/after/MM_W_012-type-states.png) |
| Same controls on M20_MT_1 | [before](../../build/lemming-type-sprites-001/worker-resume-20261115T0424/before/M20_MT_1-type-states.png) | [after](../../build/lemming-type-sprites-001/worker-resume-20261115T0424/after/M20_MT_1-type-states.png) |

State-control sheets have ordinary/water/acid columns. Rows: walk, fall, climb,
flip, umbopen, block, build, shrug, bash, mine, dig, drown, explode, cartwheel.
There are **84 visible placed samples** (14 states x three types x two real
levels): ordinary images match the frozen renderer in all **28** comparisons;
all **56** special-type images change, and each row has three distinct pixel
hashes. These placed controls supplement natural routes. MM_W_012 has only a
sheet-1 hatch, so its ordinary control is explicitly placed, not a fictitious
natural ordinary release. M20_MT_1 has sheet-2 and sheet-0 hatches.

The public application driver never writes World or progress. It uses
`runApplication --level ... --sim --native`, 100-ms integration-test frames and
private FileSaveSink paths. For the water skill it pauses, selects Blocker with
the public key, aims until production hover confirms actor 0, clicks, resumes,
and verifies the visible Blocker. Separate `walker.png`, `faller.png`,
`blocker.png`, `hover.png` and full drowning frames retain context.

`frames.csv` records actual state/type/clip/temporal entry, World coordinates,
draw Y and screen box. `simulation.txt` compares every actor's position, state,
type and frame plus the clock/dead/saved counts, across all five natural/public
input runs. They match the frozen run, including the first loss on Build and
Block. Its 20-column drowning strip samples frames **122..141**, 100 ms apart,
including the first removed frame; the World still removes actor 0 at 14.2 s.
`assets.tsv` and installed `*-walk.png`, `*-drown.png`, `BUBBLES.SPR-bubbles.png`
retain independent frame inventory/ordering evidence.

The ordinary-only **MM_CLIMB_002** application compares all **80 whole frames**
pixel-identically. Fixed installed-scene ordinary controls cover every listed
state on the two mixed-type test levels. Whole-frame equality is not asserted
on Build and Block: its flame sprite phases are deliberately wall-time seeded
in production. An initial new test incorrectly assumed deterministic flames;
`resume-images-2.log` retains that failure. A pixel diagnostic localized flame
scenery differences even in a pre-drowning walking frame; those development
frames were subsequently regenerated. No existing project assertion, timeout
or fidelity gate was altered to mask that variation.

## Checks, preservation and limits

Development history remains under the task build root. The first resumed build
succeeded. `resume-before-ctest.log` exposed the new acid demonstration's wrong
camera; the revised camera admits natural fall/walk samples, and placed acid
controls use a naturally reached falling coordinate. `resume-images-3.log`
passed its three task tests before the last rim/effect/hoist refinements.
`resume-regressions-4.log` then passed 12/12 in 276.55 s. Those runs retain
development history rather than superseding the final content checks below.

Final production/test code: [final-focused-build.log](../../build/lemming-type-sprites-001/worker-resume-20261115T0424/final-focused-build.log)
built the selected targets; [final-focused-ctest.log](../../build/lemming-type-sprites-001/worker-resume-20261115T0424/final-focused-ctest.log)
passed **15/15 in 282.57 s**, serially. Coverage includes animation clock,
state selection/pause, actor-depth reference/identity/before/after, all five
climber cases, and the three new task checks. The final P17 header-compatibility
registration was checked separately: [build](../../build/lemming-type-sprites-001/worker-resume-20261115T0424/final-compatibility-build.log)
and [CTest](../../build/lemming-type-sprites-001/worker-resume-20261115T0424/final-compatibility-ctest.log)
passed **2/2 in 3.14 s** on unchanged final production code. No full verifier
profile was run or restarted by this worker. **Full registered main/recovery
verification remains pending supervisor execution.**

I reviewed the final natural water before/after walk/fall/Blocker strips, the
production water hover image, acid walk/fall strips, all 14 rows of both
species-control sheets, the installed walk/drown/bubble strips, the human
original-session drowning sheet and the final clone drowning before/after
strips. Water is teal/pale, acid orange/green, and the ordinary column is
unchanged. The final drowning strip shows bright initial contact, small
droplets, a moving/fading figure and bubble highlights; it does not reproduce
the reference's full fragmented particle cloud. That limitation remains
provisional, not an original-comparison pass.

The final snapshot's `source-hashes.json` pins all ten task files and all six
baseline source files; `manifest.json` pins the retained images, CSVs, source
copies, logs and static records. LF and task-scoped diff checks pass. No task
production or test code changed after these final checks.

The new CMake module also gives the earlier climber before-renderer its own
header in driver/main and uses its authentic retained main.cpp (identical to
this task's pre-change main). This prevents old/new TubeScene and SceneOptions
layouts from mixing. It changes no frozen source, old assertion or timeout.
The before/after climber and actor-depth regressions are included in development
checks because the public renderer header changed.

P17's `test_fastforward_toggle_baseline` retains a whole older include tree but
links current renderer/animation support. A two-header overlay makes TubeScene
and LemmingAnim match that shared support, preserving its frozen input loop,
HUD/panel sources and their headers. Its baseline/production checks are run
separately as a compatibility check; no frozen file or expectation is modified.

Each new route compares the installed and normal clone save's existence and
entire bytes before/after and uses its own private path under build/. The
`saves-proof.txt` outputs retain the result. No save was imported/reset. No
queue, controller, supervisor, verifier, registry, permissions, shared
instructions or completion criteria were edited. Pre-existing hook-log,
queue and generated-status edits were preserved. The parent supervisor owns
RepositoryLease; this worker made no manual protected-control edit. No commits,
pushes, resets, cleans, stashes or concurrent writers were used. Builds use the
registered MinGW-w64/UCRT64 paths, -O2 with assertions and **four** compile jobs.

Remaining limits: original randomized splash/bubble emission, exact underwater
blend/depth/device state, final body breakup and original fluid-state removal
timing are not reconstructed. The visible deterministic effect is provisional.
The static-cell drowning path, inverted/draining-during-drowning interactions
and natural skill/exit routes for every species are not newly established.
An authorized, matched-camera original sequence with fixed-time actor positions,
fluid bounds, controller state and effect samples would settle those details;
this task does not authorize that execution or broader timing/clipping repairs.
Scripted images/tests are regression evidence, not human playability acceptance
or full original fidelity. All historical gates and parks remain unchanged.

Changed task files: `src/app/LemmingAnim.cpp/.h`, `src/app/TubeScene.cpp/.h`,
`src/app/main.cpp`, `CMakeLists.txt`, `cmake/lemming_type_sprites.cmake`,
`cmake/lemming_type_sprites_identity.cmake`,
`src/__test__/test_lemming_type_sprites.cpp`, and this report.

Handoff: per-type sprites and the bounded drowning presentation are ready for needs_review; original particle/depth/lifetime fidelity remains provisional.
Next: supervisor runs full registered main/recovery verification on final sources, then independent acceptance review and a human U4/U2 recheck.
