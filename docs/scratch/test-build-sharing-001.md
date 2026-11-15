# test-build-sharing-001: shared application compilation

**Implementation complete; needs_review.** This is the lead-authorized build workflow
task supporting playable v1, under AGENTS.md's Project lead direction. Difficulty:
difficult; advisory recommendation: Sonnet-class; this worker uses the explicit
Codex override. Mandatory full registered main/recovery verification is **pending
supervisor execution after worker exit**, followed by independent acceptance review.

The source-traced failure is repeated compilation, measured in the actual registered
native configuration: **1,867 compile steps for 367 distinct source files**. This is
the bounded dependency described in [the accepted design](test-build-sharing-design.md),
not a new gameplay defect or a claim that any playtest item is resolved. No production
source, test source/assertion, simulation rule, timing, save codec, queue, controller,
verifier, tool registry or shared instruction was edited. No original executable,
copied original code, oracle, debugger, hook or capture was run. No REA lookup was
needed to change this build graph.

**Implementation and call path.** `cmake/webolution_app.cmake` builds the 39 non-main
application sources (`WEBOLUTION_GFX_SOURCES` minus main, plus the four scene files)
once in `webolution_app`. `webolution_app_nomain` builds main.cpp once with
`WEBOLUTION_APP_NO_MAIN=1`. `webolution` retains its own ordinary main.cpp. The 21
full application drivers now list only their test source and link the shared objects;
their existing private definitions stay on the driver.

Legacy partial targets select exactly their requested objects using
`$<FILTER:$<TARGET_OBJECTS:webolution_app>,...>`; they acquire no current replacement
for a frozen source. Six support targets (the design's five plus `audio_demo_app`)
are archives of selected existing objects, with their prior public requirements.
They perform no application compilation of their own. Panel and antigravity support
interfaces reuse the fluid support archive. Explicit driver/frozen objects provide
the existing overrides. The footer documents the link pattern for future full drivers.

The final sharing pass runs after the task modules have supplied frozen include
paths. It hashes retained headers against current counterparts and conservatively
keeps a target's sources when any differ or lack a counterpart. Current/frozen header
changes trigger CMake regeneration. The exact decisions and both header hashes are
retained in [the target ledger](../../build/test-build-sharing-001/after/webolution-app-sharing.txt)
and [the header ledger](../../build/test-build-sharing-001/after/webolution-app-sharing-headers.tsv).

The unchanged runtime route is test driver -> `runApplication` (main.cpp:1300) ->
`runApplicationLeg` -> `runFrontendWindow` / retail level loader -> the production
`renderFrame` stages (BackdropScene, TubeScene and scene dispatch) -> results/map/menu.
The application main at main.cpp:1366 calls that same public entry. Changing the
objects' owner does not introduce a replacement runtime path.

**Macro audit.** [audit.py](../../build/test-build-sharing-001/audit.py) checked all 52
command-line definition names from the baseline against production source/header
text outside src/__test__. [Results](../../build/test-build-sharing-001/macro-audit.json):
no test-private macro is read there. `WEBOLUTION_HAVE_SDL2` controls the existing SDL
paths; `WEBOLUTION_APP_NO_MAIN` is read only at the main.cpp entry guard. Existing
source-specific `readSceneNode` / `dispatchSecondaryChildren` renames belong to frozen
translation units and remain there. The existing audiodemo source includes main.cpp
with its own loader substitution, and pausecaption includes HudText/MenuRender inside
its driver; these test translation units remain unchanged, and are not claimed as
separate shared compile_commands entries.

**Measured results.** Both fresh Ninja directories use the registered UCRT64 GCC
16.1.0, C++23, Debug `-O2 -g`, assertions enabled, installed repository assets and
`cmake --build ... --parallel 8`. A fresh directory supplies each full clean build;
no previous build or frozen source was deleted. The no-change build follows it;
then the script touches only src/app/TubeScene.h's timestamp, builds, and restores
that timestamp. These are single sequential observations with uncontrolled filesystem
cache, not repeated cold-cache measurements or a general performance guarantee.

| Measurement | Before | After |
| --- | ---: | ---: |
| compile_commands.json compile steps | 1,867 | 519 |
| Distinct source paths | 367 | 367 |
| Repeated steps beyond distinct sources | 1,500 | 152 |
| Full clean build, seconds | 500.071 | 205.865 |
| No-change build, seconds | 3.564 | 0.284 |
| TubeScene.h rebuild, seconds | 145.997 | 50.055 |
| Actual C++ compilations in header rebuild log | 118 | 43 |

The graph removes **1,348 steps (72.2%)**, exceeding the required half. The measured
clean build is 2.43 times faster; the header rebuild is 2.92 times faster. The
application executable and every registered native target linked successfully.

Reproduction: run the retained foreground
[measure.py](../../build/test-build-sharing-001/measure.py) with `before` on the
task-entry CMake files, or `after` on the final files, using the registered Python.
Do not reuse populated benchmark directories as clean-build evidence. Initial CMake
files, including the pre-existing untracked antigravity module, are backed up in
`build/test-build-sharing-001/entry/`; `before/entry.patch` and `before/git-status.txt`
preserve entry state. `edit.py` retains the mechanical transformation.
The full [before](../../build/test-build-sharing-001/before/compile_commands.json)
and [after](../../build/test-build-sharing-001/after/compile_commands.json) databases,
census.json, timings.json, configure.log, clean-build.log, no-change-build.log and
header-rebuild.log are retained in those two evidence directories.

**Exceptions kept private.** These twelve targets retain their directly listed
application sources because the conservative header audit found differences. Their
existing support subset still reuses current objects compiled against current headers.

| Target | Differing retained header(s); full paths/hashes are in the ledgers |
| --- | --- |
| test_savedemo_before | FrontendSession.h |
| test_maphover_before | FrontendSession.h, MapScene.h |
| test_cameraentry_before | AppEntry.h, LevelObjects.h |
| test_actor_motion_baseline | Retained app headers including ActorPose, ActorSprite, AppEntry, BackdropScene, FrontendSession, LemmingAnim, MapScene, Raster, RuntimeHud, RuntimeSkillPanel, SkillPanel, TubeScene |
| test_hover_feedback_before | ActorSprite, AppEntry, BackdropScene, RuntimeSkillPanel, SkillPanel, TubeScene |
| test_hatch_timing_before | AppEntry, BackdropScene, RuntimeHud |
| test_hatch_exit_before | Retained app headers including ActorPose, ActorSprite, AppEntry, BackdropScene, FrontendSession, LemmingAnim, LemmingMotion, MapScene, Raster, RuntimeHud, RuntimeSkillPanel, SkillPanel, TubeScene |
| test_fluid_hazard_before, test_fluid_hazard_gadget_before | World.h |
| test_lever_render_before | BackdropScene.h |
| test_panel_icon_before | RuntimeSkillPanel.h, SkillPanel.h |
| test_antigrav_before | ActorSprite.h, TubeScene.h |

Every explicit frozen .cpp remains a separate compilation, including the all-frozen
`test_frontendwindowquit_before` and `test_walker_lane_baseline` application layers.
`lsc_before_base` and `native023_before_base` also keep their existing base source sets.
Other frozen targets preserve their swapped files and reuse eligible current support;
`test_levelselection_before` can share its current remainder because its retained
files introduce no differing frozen headers. [details.json](../../build/test-build-sharing-001/details.json)
lists all 26 targets compiling retained .cpp files. All 367 compiled-source hashes,
723 retained-header hashes and the task-entry src manifest match after the work.

**Behavior and checks.** The identical focused CTest selection passed **50/50 before**
and **50/50 after**. Exact logs:
[before/focused-ctest.log](../../build/test-build-sharing-001/before/focused-ctest.log),
[after/focused-ctest.log](../../build/test-build-sharing-001/after/focused-ctest.log).
It includes numerical/rendering units, frozen identity checks, map hover, and both
current/frozen window-Quit drivers. The additional variant selection passed **23/23**
in **247.43 seconds**, checking archive
overrides across scene integration, camera entry, fast-forward, hatch/exit, actor
occlusion/depth, fluid, panel and antigravity; its result is retained in
[after/variant-ctest.log](../../build/test-build-sharing-001/after/variant-ctest.log).
This selection supplements, and does not replace, supervisor verification.

The ordinary frontend route goes through New Game -> map node 0 -> installed
**MM_CLIMB_002** -> natural timed loss -> results Quit -> map -> menu -> application
Quit, using the existing private FileSaveSink and owned frame clock. Before and
after both report one window creation/destruction, effective mouse/keyboard Quit,
30 level frames, zero failures, and untouched installed/normal saves. All **51 PNGs**
from the current/frozen window and map drivers match byte for byte between builds.
I visually inspected the paired current
[before settled frame](../../build/test-build-sharing-001/before-build/frontend-window-quit-test/run-1794703668213538400/level-settled.png)
and [after settled frame](../../build/test-build-sharing-001/after-build/frontend-window-quit-test/run-1794703986997732200/level-settled.png).
This is clone regression evidence, not an original comparison or a level win.
`before/frame-hashes.json` and `after/frame-hashes.json` index the full image set.

Both `ctest -N` listings contain **260 tests in the same order**. After normalizing
only the binary directory, the listings and each test's command plus **every JSON
property** match: names, count, timeout, RUN_SERIAL, DEPENDS, environment and all
other properties are unchanged. Both raw listings/inventories are retained under
before/ and after/; [ctest-N-comparison.txt](../../build/test-build-sharing-001/ctest-N-comparison.txt)
and [comparison.json](../../build/test-build-sharing-001/comparison.json) retain the
comparison. `inspect.py` records the exact focused selection and comparison logic.
Normal clone and installed save presence/bytes match their initial manifest.

Task changes: CMakeLists.txt; new cmake/webolution_app.cmake;
cmake/fluid_kinds.cmake; cmake/panel_icon_anim.cmake; the pre-existing
cmake/antigrav_presentation.cmake; this report. Pre-existing source, control,
installation-log and report edits remain preserved. No commit or push was made.
Remaining unknowns: full registered main/recovery results on this final graph,
unmeasured platforms/configurations, and repeatability of these single timing samples.
Original timing/fidelity gates, parks and project completion criteria are unchanged.

Handoff: shared application compilation is implemented; 1,867 -> 519 steps, unchanged test inventory, sources, frozen headers, saves and 51 clone frames; needs_review.
Next action: supervisor runs mandatory registered main/recovery verification, then independent acceptance review; do not dispatch another task from this worker.
