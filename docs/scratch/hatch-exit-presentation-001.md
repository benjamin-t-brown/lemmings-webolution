# hatch-exit-presentation-001 — P9/P10/P11 presentation

Status: **needs_review**. Playable v1, roadmap step 7. Difficulty: difficult;
assigned recommendation Sonnet-class, this worker uses the explicit Codex
provider override. Full registered main/recovery verification is **pending
supervisor execution**, followed by independent acceptance review. No original
game, original-byte oracle, debugger, hook, original capture, subagent, detached
work, installation change or commit ran. Existing edits were preserved.

## Failure and the correction to P10

The human ledger is [playtest 1, P9–P11](../playtest/2026-11-12-playtest-1.md).
Hatch models were drawn with closed doors for the entire level, and exit models
remained stationary throughout Pending before Results. These remain demonstrated
task-entry defects, after the accepted actor animation, motion and occlusion work.

**P10 does not reproduce literally on the authentic task-entry source.** The
ledger describes the earlier build `721c64e`, before the accepted per-state actor
animation and motion work. This worker's preserved baseline already releases a
Faller at the stored hatch words, draws `fall`, and visibly descends to the floor.
On `MM_CLIMB_002`, birth is world `(32,42)` / terrain `(32,53)`, with landing at
terrain row 63 / world Y=32. The 20 ms route retains 25 visible Faller frames.
`MM_FLOAT` retains 10 visible Faller frames before landing. Both observations
hold before and after this change. It would be false to label these baseline
images as instant-ground spawns or move the proven original birth coordinates
just to create a new spawn implementation.

P10 is therefore preserved and regression-covered, with the opening doors now
making the emergence from the box readable. No new spawn behavior is claimed.
The acceptance request for a task-entry image showing no falling cannot be met
honestly against this baseline; this explicit correction needs review. The human
ledger is not edited or marked resolved by this worker.

## Production call paths

**Release/drop:** `runApplication` → `runApplicationLeg` →
`sim::loadRetailLevel` → structured hatch words → `hatchSpawnCell` →
`Entrance{HatchWords}` → `RuntimeSimState::advanceElapsed` →
`World::advanceElapsedMs` / `stepRelease` → `World::spawn` →
`State::Faller` at the entrance coordinates → existing `stepFaller` / landing.
After each update, the accepted motion tracker observes the same World;
`LemmingAnimTracker` selects `fall`; `injectedWalkersFromWorld` supplies the
fractional position and clip to `TubeScene` for drawing and picking. None of
those simulation, animation or motion implementations changed here.

**Entry cue:** `EntryStartCue.cpp` draws the localized `Let's Go!` text. Its
clock starts at 3.5 seconds for a map entry or 1.5 for a retry; the text is drawn
after the level/HUD and before the cursor/menu/caption. It neither creates
actors nor gates release. Its large opening text can cover the first birth on
the ordinary frontend route; the second release occurs after it retires.
Those accepted cue clocks and composition rules are unchanged.

**Doors:** `BackdropScene::build` already extracts each inline hatch model from
`entities_425e50`, via its serialized ownership extent. Immediately after load,
`entranceExitPresentation` reads the matching Object 09 auxiliary transform
clips into a separate object owned by this application leg. The frame loop samples
`levelClockMs / releaseArmSeconds`, then calls
`setEntranceExitPresentation`. `applyHatchAnimation` replaces the two door
nodes' local transforms from the retained keys. Existing ancestry composition,
mesh extraction, predicates, rasterization and LegacyUnattributed scene stage
consume the changed matrices. Vertices, materials, hatch body, tube raster,
controller and actor visibility rules are unchanged. This is live geometry,
not a screenshot overlay.

**Balloon:** `RuntimeSimState` continues to feed `LevelEndTransition` after each
admitted simulation slice. The existing transition enters Pending, starts from
timer 7, and publishes its latched results at strict timer < 1. During Pending,
the frame loop samples `7 - timerSeconds`, and `BackdropScene` lifts each exit
root by `5 * departureSeconds` in world Y from its saved initial matrix.
The existing scene path draws the moving basket, ropes and balloon together.
The existing results snapshot/`LegDisposition::Completed` handoff remains in
place; no extra delay, injected win, result flag or end threshold is added.
Pause repeats the same absolute samples; a newly loaded level starts closed
and grounded. Absolute sampling avoids cumulative transform drift. The separate
presentation object also preserves `BackdropScene`'s existing class layout:
historical demonstrations that compile old headers against current support keep
their authentic source. No frozen file was rewritten to accommodate this change.

## Static evidence and provisional choices

Reference: `C:/progs/lemmings-webolution/Lemmings Revolution/Lemmings Revolution.exe`,
SHA-256 `32675bdb49866e76502713251fdfd2b09078c2481c72ca22ff68ebf4b3737a2f`.
The registered static REA Ghidra 12.1.4 profile opened this target and closed
without snapshot arguments. [Raw pseudocode and instruction evidence](../../build/hatch-exit-presentation-001/static/)
is retained; pseudocode is provider output, not original source. No executable
instructions from the original were executed.

Retained evidence consulted: [spawn words](phase-a6-plus8-spawn-init.md),
[release trigger](phase-d1-release-trigger.md),
[derived door-arm measurement](phase-d1-door-open-duration.md),
[scene ownership](scene-integration-001.md),
[retained row scaling at 0x4381F0](phase-c3bk-scale-bias.md),
[end transition](level-end-transition.md),
[entry cue](level-entry-start-cue.md),
[actor animation](actor-anim-001.md), and [motion](actor-motion-001.md).

New static reads, distinguished from clone observations:

- `0x425A95`, particularly `0x425BF7..0x425C30`, sign-extends stored hatch
  X/Y words into the actor constructor. `0x42DE51` consumes those arguments.
  Pseudocode was checked against full instruction reads. Evidence IDs include
  `ev_529e52e7994f42f2209413a10a0a48aac5cbb0bec1015ee42e9da22e3e9464c8`
  and `ev_0127c213588ee61c76fed2b78a45986552693a321f6650bdba2004233e8b74e8`.
- Hatch reader `0x425E50` selects animation 0 with model vtable slots +0x34 and
  +0x3C. Model `0x451479` selects the animation and samples its initial time;
  `0x451121` advances its clock; `0x432383` forwards the update. Full instructions
  for the selection/forwarding paths are retained.
- `0x4764C4` samples transform tracks, replaces node rotation/translation and
  applies scale. `0x477130` selects the first key whose end time is >= time and
  blends toward the following key using the previous key's end as the interval
  start. `0x4817DA` builds the row-vector rotation from quaternion order
  `(w,x,y,z)`. The latter two were checked against complete instructions;
  IDs `ev_0a8766c67fe56bf646d7021117d3c727ca52af78b6e344438381fa2b304b2636`
  and `ev_7ae4dade7dae868ac9e9f21657d16016c377fd3b82a0c3cc7ade84dfc51623bc`.
- Exit update `0x42688D`, instructions `0x426A0C..0x426A18`, computes
  `Y += dt * 5`. Its departure predicate uses remaining populations per exit
  type (or the type-4 aggregate), and it also drifts/rotates. The complete
  instructions and pseudocode are retained. These establish the vertical
  speed, not the clone's global trigger or full motion fidelity.

Read-only installed-asset census: all **102 levels, 144 hatches and 111 exits**.
All 144 hatches have two 70-key door tracks at preorder node indices 2/3,
duration `2.799999952316284`, with linear quaternion keys. Regular hatches use
clip `entrance`; 20 water-model hatches use `entrance_acid`. All stored birth
Y words are two world units below the model floor. This is measured data, not
a rule used to reconstruct new birth positions. Census and representative keys:
[asset evidence](../../build/agent-native/main/hatch-exit-assets/).

Explicit **provisional playable-v1 policies**, also labelled in code:

1. Fit the serialized full door clip to the existing `releaseArmSeconds`
   interval. The accepted clone interval is approximately 0.723 seconds;
   the retained original estimate does not resolve its discrepancy with the
   2.8-second stored clip. This ensures doors finish before the first release
   without changing release time/count/cadence. It does not resolve D1 timing.
2. Use normalized, shortest-arc linear quaternion interpolation for these
   linear keys. The native routine has additional interpolation behavior;
   no general animation engine or spherical-key support is claimed.
3. Trigger all exit roots from global Pending, including timeout/loss, and
   apply only the traced vertical speed. Per-exit/type exhaustion, idle
   bobbing, lateral/radial drift, rotation and height-based native removal
   remain unported. The visible departure occurs during the existing pending
   interval; complete disappearance outside every possible camera is not
   claimed.

An authorized original observation of a complete opening at matched elapsed
times would settle the clip speed/phase and interpolation. A matched exit
sequence through per-type exhaustion and timeout would settle departure trigger,
drift, rotation and removal. No original run was requested or attempted here.
Full timing, frame/depth fidelity and all historical parks/gates remain unchanged.

## Retained before/after clone evidence

These are source-built clone observations, not human play or original comparisons.
The authentic task-entry source is preserved in
[before-source](../../build/hatch-exit-presentation-001/before-source/), with
[entry hashes](../../build/hatch-exit-presentation-001/before-sha256.json).
The two executables compile the frozen versus current `main.cpp` and
`BackdropScene.cpp`; unchanged common support is shared. The frozen-source CTest
checks those exact hashes. No modified source was relabelled as a baseline.

Fixed-camera entry routes use the real production application, installed
`MM_CLIMB_002` and `MM_FLOAT`, a 20 ms owned frame clock, and four paused frames.
The ordinary map route uses public SDL New Game/confirmation/map-node input,
assigns Climbers through actual displayed actors, reaches a natural 10/10 win,
pauses three pending frames, and reaches the ordinary Results screen. Its owned
clock is 200 ms; it is a scripted regression route, not proof of human playability.

| Evidence | Before | After |
| --- | --- | --- |
| Closed initial hatch | [frame](../../build/hatch-exit-presentation-001/before/MM_CLIMB_002/frame-0.png) | [frame](../../build/hatch-exit-presentation-001/after/MM_CLIMB_002/frame-0.png) |
| Door motion, before any birth | [static](../../build/hatch-exit-presentation-001/before/MM_CLIMB_002/frame-20.png) | [swinging](../../build/hatch-exit-presentation-001/after/MM_CLIMB_002/frame-20.png) |
| Doors open before release | [still closed](../../build/hatch-exit-presentation-001/before/MM_CLIMB_002/frame-45.png) | [open](../../build/hatch-exit-presentation-001/after/MM_CLIMB_002/frame-45.png) |
| Natural drop begins | [already falling](../../build/hatch-exit-presentation-001/before/MM_CLIMB_002/frame-105.png) | [falling through open hatch](../../build/hatch-exit-presentation-001/after/MM_CLIMB_002/frame-105.png) |
| Drop continues | [frame](../../build/hatch-exit-presentation-001/before/MM_CLIMB_002/frame-129.png) | [frame](../../build/hatch-exit-presentation-001/after/MM_CLIMB_002/frame-129.png) |
| Exit early Pending | [stationary](../../build/hatch-exit-presentation-001/end-before/pending-1.png) | [departure starts](../../build/hatch-exit-presentation-001/end-after/pending-1.png) |
| Exit late Pending | [still stationary](../../build/hatch-exit-presentation-001/end-before/pending-30.png) | [balloon has risen](../../build/hatch-exit-presentation-001/end-after/pending-30.png) |
| Results after departure | [win](../../build/hatch-exit-presentation-001/end-before/results.png) | [same win](../../build/hatch-exit-presentation-001/end-after/results.png) |

Full image sequences, frame/state traces and summaries are retained in those
directories. Ordinary frontend entry and first/second release images are also
retained under `end-before/` and `end-after/` as `entry-*` and `drop-*`.
Supplemental [isolated model renders](../../build/hatch-exit-presentation-001/models/)
demonstrate transform-to-pixel behavior; they do not replace the application runs.

The balloon region changes **0 pixels before / 4,244 pixels after** between
pending samples 1 and 30; HUD/time/cursor-independent region `(45,130,55,200)`.
Both routes retain 33 Pending frames including three paused frames. Before/after
simulation traces compare every frame, including stock, actors, births, counts,
positions, terrain version and end phase/timer. The Climber route still wins
10/10. No existing scripted route assertion was changed; their complete
classification check remains part of supervisor verification.

## Checks, preservation and handoff

Development used the registered UCRT64 GCC 16.1/CMake/CTest paths, existing
`-O2 -g` assertion-enabled build and 12 compile jobs; task tests ran serially.
No full verifier profile was duplicated by this worker. Seven task CTests cover
authentic-source identity, paired installed entry, paired natural win/results,
all installed animation assets and actual model/pixel behavior.

[Development 3](../../build/hatch-exit-presentation-001/development-3.log):
**7/7 passed**, 104.23 seconds. Earlier development failures are retained:
the test driver initially omitted SDL Return's key symbol, and the first census
recognized only `entrance`, not water models' `entrance_acid`. Both were corrected;
no existing assertion or timeout was weakened. The focused rerun after
formatting, explicit unsupported-spherical-key rejection and extra frontend
image retention passed **5/5** in 63.77 seconds:
[development 4](../../build/hatch-exit-presentation-001/development-4-final.log).
After moving animation data out of `BackdropScene` to preserve historical
header compatibility, all seven task checks plus the affected unchanged
`fastforward_toggle_baseline` passed **8/8** in 106.00 seconds:
[development 5](../../build/hatch-exit-presentation-001/development-5-layout.log).
The final census additionally exercises the application's production binding
factory for every installed hatch/exit; **1/1 passed**, 0.45 seconds:
[development 6](../../build/hatch-exit-presentation-001/development-6-bindings.log).

[Paired trace summary](../../build/hatch-exit-presentation-001/paired-summary.json)
retains the byte-identical simulation hashes: 546 frontend/game frames and
180 frames on each fixed-camera entry route. On the 200 ms Climber route,
release clocks in milliseconds are identically `2200, 5000, 8000, 10800,
13800, 16600, 19600, 22400, 25200, 28200` before and after. The 20 ms routes
both first release at 2040 ms; these are different owned frame partitions,
not a claimed timing improvement or original match.
[Unchanged-path hashes](../../build/hatch-exit-presentation-001/unchanged-paths.json)
confirm World, retail setup, RuntimeSim, entry cue, end transition and actor
animation/motion sources match task entry. The
[task-only patch](../../build/hatch-exit-presentation-001/task-only.patch)
separates this task's edits to pre-existing files from incoming work.

Normal and installed save existence/content are checked unchanged by the entry
and end drivers. Connected tests use private `FileSaveSink` paths inside the
task evidence directory. No installed or normal user save was imported, reset
or written. No queue, verifier, controller, model pin, shared instruction,
completion criterion or historical status ledger was edited.

Changed production: `src/app/HatchAnimation.cpp/.h`, `BackdropScene.cpp/.h`,
`main.cpp`; source registration in `cmake/webolution_sources.cmake`.
Support/evidence: `src/__test__/test_hatch_exit.cpp`, the new CMake registrations,
`cmake/hatch_exit_identity.cmake`, and this report. Incoming unrelated edits remain.

## Retry after the supervisor's first verification

The supervisor run
[`20261113T215726-34648-c1fb6993/verification.log`](../../build/agent-runs/20261113T215726-34648-c1fb6993/verification.log)
completed **207/209 main tests**, including all seven hatch/exit checks. It failed
`runtimewindow` and `frontendentry`; recovery verification did not run after that
main-suite failure. This supersedes any implication that the initial development
checks established a passing full suite. Final full verification remains pending.

Both failed assertions compared an animated entry frame with the old static,
closed-hatch diagnostic image. At frame 30 the application has admitted 208 ms,
so its hatch doors have moved; `--sim-ticks 1 --shot` still intentionally renders
the stored model pose. This is an expected image change for P9, not a camera,
HUD, release or simulation regression. The failure and original fixtures remain
in the supervisor log and the existing test output directories.

The retry adds a small **test-only independent headless compositor**,
`src/__test__/HatchEntryFixture.h`, shared by `frontendentry` and a command-line
fixture helper used by `runtimewindow`. It builds the installed first-level
scene, samples the door transforms at an explicit elapsed time, and composes the
ordinary scene/panel/HUD/cursor primitives without entering the application loop.
Before accepting that reference, both tests require its zero-time image to be
byte-identical to the existing direct diagnostic fixture at the same camera and
HUD tick. They then sample the doors at 208 ms for frame 30 (6 ms for frame 1).
The frontend test also retains the original closed reference as `closed-*.png`.

**All existing full-frame equality assertions, positive controls, route/reset
checks and timeouts remain. No pixel region is masked and no difference tolerance
is added.** The only corrected expectation is the formerly frozen hatch pose.
The helper refuses post-release inputs, avoiding invented actor animation clocks.
No production source, simulation rule or timing changed in this retry; no frozen
source was updated. The supervisor's existing RepositoryLease owns the worker;
no manual protected-control edit was made. The process check found no live clone,
compiler or test process before development.

Registered UCRT64 targeted build succeeded. The focused rerun of both actual
failures passed **2/2**, 9.78 seconds:
[`retry-2/fixture-tests.log`](../../build/hatch-exit-presentation-001/retry-2/fixture-tests.log).
The final build log is
[`retry-2/build-fixtures.log`](../../build/hatch-exit-presentation-001/retry-2/build-fixtures.log).
The first helper compile exposed a missing direct `Box.h` include; it was fixed
before this successful build. No full verifier profile was duplicated. Installed
and normal saves are not opened by the helper; the existing direct-level CLI
checks and null-store frontend test do not use a persistent store.

Visual inspection of the retained closed/open frame-45 pair, birth frame-105
pair, descending frame 129, late-Pending balloon pair and Results image confirms
the report's scoped observations: open hanging doors, an already-working fall
through the hatch, a raised balloon and the same 10/10 result. This is clone image
inspection, not human playtesting or an original comparison.

Additional changed files in this retry: `src/__test__/{runtime_window.cmake,
test_frontendentry.cpp,HatchEntryFixture.h,test_hatch_entry_fixture.cpp}` and the
corresponding target/fixture argument in `CMakeLists.txt`. Earlier task changes
are listed above. Backups, final hashes and the retry-only patch are under
[`retry-2/`](../../build/hatch-exit-presentation-001/retry-2/).

Handoff: P9/P11 implemented; P10 preserved and tested, with its task-entry baseline correction explicit. Status **needs_review**.
Next: supervisor full main/recovery verification and independent review; human confirmation remains outstanding.
