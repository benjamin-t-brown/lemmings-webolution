# test-render-skip-001: render only consumed automation frames

Status: **needs_review**. User-authorized test-speed exception supporting
playable v1, implementation-plan step 8/P18. Difficulty difficult; recommendation
Opus; this run uses the authorized Codex override. Completed supervisor verification
passes **281/281 main and 84/84 recovery tests**; exact logs and full-suite totals
are recorded below. Independent acceptance of this report correction remains pending.

The demonstrated dependency is the accepted [frame profiling report](frame-rate-profile-001.md):
the hatch route spends over 90 percent of its time drawing images its observer
does not inspect. The [human frame-rate addendum](../playtest/2026-11-14-playtest-5.md)
motivates the renderer work; this bounded task accelerates source tests rather
than claiming another player-facing fix or original-fidelity result.

## Production path and contract

`runApplication` -> `runApplicationLeg` -> retail setup/scene construction ->
SDL input -> existing clocks/World updates -> scenery/door/gravity/panel-icon
updates -> camera pose and scene preparation -> optional materialization ->
observer -> RGB upload/copy/present. The full materialization path remains
`renderFrame` -> `scheduleSceneFrame` (primary, legacy, tube, fluid, secondary)
-> actors -> hover/hatches -> HUD, icons, countdown, cue, menu and cursor.
Raster arithmetic, pass order, visibility/depth rules and presentation calls
are retained. No release, skill, simulation or scenery clock is retuned.

The explicit C++ automation option is `app::ScopedFrameConsumption`. No CLI,
frontend action or production environment variable enables it. Without that
scope, or with `ScopedFrameConsumption(false)`, every frame renders as before.
Even an enabled scope requires an owned frame clock (`--frames` or positive
`frameSeconds`). Ordinary interactive wall-clock play continues to render.

Opted-in observers receive simulation and presentation state every iteration.
An uninspected view has an empty image and no raster-derived actors, hover,
cursor-state or hatch-depth results. Before consuming any such fields the
observer calls `app::inspectLevelFrame(view)` (the converted tests use
`test::inspectedImage`). That synchronous request runs the full production
composition once, fills the same view, and makes repeated requests idempotent.
An inspected image is also presented. A final `--window-shot` and C3 raster
diagnostics force rendering. Observer/frame counts and exit conditions do not
change.

The scene's fluid snapshot, lever faces and `TubeScene::syncTerrain` still update
on every iteration. Input, animation trackers, intro/title, door progress,
outcome processing and scenery advance retain every iteration too. If a later
public click needs a discarded frame's depth, input first renders that preceding
pose with the unchanged production scene path, before any simulation advance.
The previous pick-reference and injected actors remain the same operands.
Per-frame picking/occlusion tests were deliberately not converted.

First-draw flame latches are stateful, so skipping is additionally refused until
every sprite node has latched. A permanently hidden/unlatched node conservatively
keeps full rendering enabled. Subsequent sprite temporal updates continue on
every iteration. The application-clock RNG seeding at main/play entry is
unchanged. An automation-only secondary-raster observer records actual sprite
color stores; it never changes pixels, depth, temporal state or RNG.

These are clone automation policies, not reconstructed original behavior.
No new original semantics are inferred. Retained seeding/dispatch evidence in
the accepted scene-branch/profile work remains its authority; the registered
reference identity is SHA-256
`32675bdb49866e76502713251fdfd2b09078c2481c72ca22ff68ebf4b3737a2f`.
No original executable, copied instructions/oracle, debugger, hooks or capture
ran, and no new REA session was needed.

## Audit and conversions

The authoritative pre-task full log is
`build/agent-runs/20261115T065957-22780-c25dfb5b/verification.log`:
280/280 main tests, **1217.67 s** real total; 84/84 recovery, **21.89 s**.
Exactly **60 registrations** exceed 20 s, including one recovery decoder.
Their complete classification and wall times are listed below; machine-readable
source mappings are in `build/test-render-skip-001/classification.json`.

Converted registrations: all 17 `levelsweep_chunk_00..16`, `hatch_timing_after`
(entry mode only), and both `empty_receiver_before/after`. The last two were
below 20 s in that full log but use the same safe event-image contract and give
an installed MM_W_012 fluid/terrain regression. No historical frozen main is
converted. Fast/win/retry/matched hatch routes retain their original rendering.

The sweep still inspects the same settled and terminal frames, computes their
original statistics/thumbnails and writes both PNGs. Its former intermediate
`lastImage` copies were overwritten before any consumer: it now requests that
image only on the unchanged terminal/quit criterion or frame cap. It still
visits all 102 installed map nodes, with the same frame caps, watchdogs, outcome
reporting and parent isolation. No natural-play or winnability claim follows.

Hatch entry uses installed nodes 0 and 2 (`MM_CLIMB_002`, `MM_BOMB`), retaining
all original periodic, door, cue and release images, pause/resume and release
assertions. The receiver variants use MM_W_012's original 300-frame camera route,
retaining first/deep/floor and matched-frame PNGs. All stores remain private
under each test's output directory; installed and normal clone saves are not
used as stores or imported/reset.

All existing test assertions, sample counts, timeouts, dependencies and
`RUN_SERIAL` settings remain unchanged. The original 280 main registrations
remain; one additional installed-route regression, `frame_consumption`, tests
the automation contract. Nothing changes the verifier/controller/queue/tool
registry, shared instructions, model pins or completion/fidelity criteria.

## Retained equivalence evidence

All paths below are relative to the repository root.

`build/test-render-skip-001/entry-source/` and `entry-sha256.json` preserve
task-entry sources before changes, alongside entry executable copies,
`entry-status.txt` and `pre-existing.patch`. The initially dirty files were
the hook log and supervisor-owned queue/status; they remain outside this task.
The coordinating supervisor owns the live repository lease. No manual protected
control edit was made and no competing build/clone runner was present at entry.

`instrumented-entry-main.cpp` is a **separate** copy of that authentic loop with
only the sprite-store observation added. The authentic source was not replaced.
Optional `render_skip_reference_*` targets compile that loop against the same
final test drivers and unchanged production scene/simulation objects. They are
excluded from ALL and normal verification. Main builds use the registered
UCRT64 GCC/CMake/Ninja paths, C++23, `-O2 -g` with assertions and at most eight jobs.

`tools/measure_frame_consumption.py` synchronously runs the same driver with
`WEBOLUTION_TEST_RENDER_ALL=1` (test helper disables skipping) and with that
override absent (explicit helper scope enables it). It never substitutes a
different simulation, frame count or action route. Each output contains five
byte-compared record streams: gameplay (all actor fields, stock, terrain version,
lever/receiver state), releases, queued actions, consumed frame state, and
outcomes. Original driver traces/PNGs remain beside these additional records.
Raster-derived fields are deliberately absent on unconsumed frames and are
compared through inspected full images instead.

The initial three-way control is
`build/test-render-skip-001/paired-001/comparison.json`: frozen entry, current
full-render and current demand-render, for sweep chunk 00, hatch entry and both
receiver variants. All state records are byte-identical; inspected images match
outside actual sprite-store masks. Complete commands, exit codes and wall times
are retained in that JSON and each mode's `process.log`. Installed and normal
save bytes/presence are unchanged.

| Same-driver route | Current full render | Demand render |
| --- | ---: | ---: |
| sweep chunk 00, six installed levels | 18.68 s | 2.68 s |
| hatch entry, MM_CLIMB_002 + MM_BOMB | 49.23 s | 4.00 s |
| MM_W_012 receiver before | 16.99 s | 0.87 s |
| MM_W_012 receiver after | 17.35 s | 0.96 s |

The frozen-loop times were respectively 18.71, 48.90, 16.97 and 16.65 s.
Those three-way checks comprise 50 record-file comparisons and 98 inspected
images **per comparison direction**. The 20 receiver PNGs are exact in both
directions. The hatch/sweep variations are confined to actual flame writes;
no entire level, rectangle or generic tolerance is excluded. The comparison
decodes the repository PNG writer's pixels and requires zero differing pixels
outside the union of that image's two recorded sprite-store masks. The original
clock seed itself is neither fixed nor overridden.

The remaining sixteen chunks are retained in
`build/test-render-skip-001/paired-sweep-001/comparison.json`. Together the two
paired manifests cover all **20 converted registrations**, all **102 installed
levels**, **530 byte-identical record files** and **290 inspected PNGs**. Of
those PNGs, 123 are exact; the other 167 differ only inside actual sprite stores,
with **zero unexpected pixels**. `paired-summary.json` records the aggregate:
683.49 s drawing every frame versus 52.21 s on demand (13.1x over these serial
same-driver process runs). This is not a main-suite total or a parallel-suite
speedup measurement. Both save snapshots remain unchanged.

`paired-source/sha256.json` preserves the exact sources used for those paired
runs. The final source additionally preserves the original input behavior before
frame zero (only an actually discarded frame invalidates picking depth), avoids
duplicate scene preparation on drawn frames, and keeps preparation/RGB conversion
in their original profiling categories. `final-source/` and
`final-source-sha256.json` retain that boundary. The final registered runs and
their record comparisons are listed below; no other production source changed.

`final-registered-comparison.json` now also compares the **final compiled source**
outputs from all 20 registered converted checks with their retained full-render
references: all 530 state records and six original `frames.csv`/`releases.txt`/
`app.csv` files are byte-identical, and all 290 inspected images match outside
actual sprite stores. `final-registered-evidence/` preserves those registered
outputs independently of later supervisor reruns. `final-binaries/` and
`final-executables-sha256.json` preserve the six final executables, including the
ordinary CLI and the contract regression. This is final-source clone evidence,
not an original comparison.

Example inspected images:
[full hatch release](../../build/test-render-skip-001/paired-001/full/hatch_timing_after/MM_CLIMB_002/first-release.png),
[demand hatch release](../../build/test-render-skip-001/paired-001/skip/hatch_timing_after/MM_CLIMB_002/first-release.png),
[demand sweep terminal](../../build/test-render-skip-001/paired-001/skip/levelsweep_chunk_00/chunk-000/node-000/frame-final.png).
Both hatch images were visually inspected: the same first lemming, open hatch,
HUD and scene are present. These are source-clone CPU exports.

The installed-route `frame_consumption` regression runs default, explicitly
disabled and enabled policies over the same 120-frame MM_CLIMB_002 route. It
checks every state record, selected images, repeated inspection, a public skill
assignment/resume following an intentionally discarded paused frame, final
`--window-shot`, frame count and scope restoration. The first development run
passed in 12.50 s (`frame-consumption-first.log`); final-content validation is
listed with the scoped checks below. Early instrumentation development found a
MinGW inline TLS-vector initializer link conflict; function-local thread-local
storage fixed it. No test assertion or timeout was relaxed.

The final registered `frame_consumption` check passes in **15.15 s** in the
four-job scoped run. Its default and explicitly disabled policies render every
frame; the enabled policy discards more than 100 frames, preserves all 120 state
records and every requested image, and refreshes picking correctly after the
discarded paused frame. The final screenshot matches exactly.

`assertion-call-counts.json` supplements the source diff: the three converted
drivers retain their respective 8, 73 and 33 `require`/`check` call sites.
The existing CMake registrations/properties have no edits; the only root CMake
change includes the new regression/optional-control module after app sharing.

<!-- TASK_TIMES_BEGIN -->
## Every registration over 20 seconds

N = no rendering consumed; I = specific inspected images; F = every-frame pixels or raster-derived state. All F registrations retain full rendering.

| Registration | Before full log (s) | After scoped run (s) | Consumes | Decision |
| --- | ---: | ---: | :---: | --- |
| `actor_motion_baseline` | 84.30 | 82.82 | F | keep full render |
| `actor_motion_production` | 40.78 | 50.61 | F | keep full render |
| `antigrav_after` | 26.74 | 33.12 | F | keep full render |
| `antigrav_before` | 39.53 | 39.94 | F | keep full render |
| `audioapp` | 26.09 | 26.46 | N | keep wall clock |
| `basherfix_probe_gap1` | 36.73 | 45.23 | F | keep full render |
| `basherfix_probe_gap3` | 34.86 | 42.70 | F | keep full render |
| `climber_occlusion_after` | 61.13 | 79.06 | F | keep full render |
| `climber_occlusion_app_before` | 20.32 | 20.09 | F | keep full render |
| `climber_occlusion_before` | 76.25 | 75.13 | F | keep full render |
| `empty_receiver_gadget_before` | 120.81 | 150.76 | F | keep full render |
| `empty_receiver_hatch_before` | 151.45 | 146.59 | F | keep full render |
| `fluid_hazard_after` | 43.49 | 43.88 | F | keep full render |
| `fluid_hazard_before` | 43.57 | 42.24 | F | keep full render |
| `fluid_hazard_gadget_before` | 46.75 | 46.78 | F | keep full render |
| `fluid_hazard_gadget_replay` | 33.66 | 34.35 | F | keep full render |
| `fluidlever_play_ordinary-camera` | 92.26 | 92.76 | I | keep wall clock |
| `fluidlever_play_stable-camera` | 92.14 | 92.67 | I | keep wall clock |
| `gravity_gadgets_after` | 27.41 | 33.88 | F | keep full render |
| `gravity_gadgets_before` | 28.83 | 32.25 | F | keep full render |
| `hatch_exit_app_before` | 31.06 | 31.66 | F | keep full render |
| `hatch_exit_end_after` | 25.70 | 31.92 | F | keep full render |
| `hatch_exit_end_before` | 47.98 | 47.17 | F | keep full render |
| `hatch_timing_after` | 44.69 | 4.43 | I | demand |
| `hatch_timing_before` | 70.71 | 68.30 | I | keep frozen loop |
| `hatch_timing_routes_after` | 93.01 | 92.40 | F | keep full render |
| `hatch_timing_routes_before` | 154.43 | 143.59 | F | keep full render |
| `hatch_timing_win_after` | 25.55 | 31.54 | F | keep full render |
| `hatch_timing_win_before` | 31.52 | 39.57 | F | keep full render |
| `installed_texture_payloads` | 21.88 | 20.46 | N | no level loop |
| `lemming_type_sprites_after` | 28.01 | 31.44 | F | keep full render |
| `lemming_type_sprites_before` | 40.01 | 39.47 | F | keep full render |
| `levelreturn` | 33.33 | 42.03 | F | keep full render |
| `levelsweep_chunk_01` | 28.39 | 2.66 | I | demand |
| `levelsweep_chunk_02` | 21.36 | 2.80 | I | demand |
| `levelsweep_chunk_03` | 26.30 | 2.86 | I | demand |
| `levelsweep_chunk_04` | 46.68 | 2.81 | I | demand |
| `levelsweep_chunk_05` | 66.08 | 2.89 | I | demand |
| `levelsweep_chunk_06` | 71.18 | 2.92 | I | demand |
| `levelsweep_chunk_07` | 37.23 | 2.86 | I | demand |
| `levelsweep_chunk_08` | 49.77 | 2.94 | I | demand |
| `levelsweep_chunk_09` | 42.67 | 2.86 | I | demand |
| `levelsweep_chunk_10` | 21.29 | 2.82 | I | demand |
| `levelsweep_chunk_11` | 42.35 | 2.90 | I | demand |
| `levelsweep_chunk_12` | 30.23 | 2.86 | I | demand |
| `levelsweep_chunk_13` | 25.86 | 2.91 | I | demand |
| `levelsweep_chunk_14` | 35.79 | 2.99 | I | demand |
| `levelsweep_chunk_15` | 45.91 | 2.96 | I | demand |
| `levelsweep_chunk_16` | 79.63 | 2.98 | I | demand |
| `lever_render_app_after` | 32.06 | 36.47 | F | keep full render |
| `lever_render_app_before` | 47.92 | 49.00 | F | keep full render |
| `playablebaseline_play_1` | 20.26 | 20.28 | F | keep full render |
| `playableexpansion_play_1` | 20.29 | 20.27 | F | keep full render |
| `playableexpansion_play_2` | 90.82 | 90.81 | F | keep full render |
| `playableexpansion_play_3` | 92.01 | 90.88 | F | keep full render |
| `playableexpansion_play_4` | 46.76 | 46.76 | F | keep full render |
| `targetedplay_play_1` | 259.93 | 260.18 | F | keep full render |
| `terrainrender_bomb` | 20.75 | 20.32 | F | keep full render |
| `walker_lane_baseline` | 37.89 | 37.55 | F | keep full render |
| `walker_lane_production` | 22.55 | 27.17 | F | keep full render |

The before column is the latest complete registered log cited above. The after column uses actual final-source checks from `partial-slow-tests.log`, `final-remaining-tests.log` and `final-recovery-slow.log`, with the original four-job/serial properties. Different selections and ordering change contention: these rows do not establish the main-suite after total. Source mappings and full consumption reasons are retained in `classification-with-times.json`.

Converted registrations below the threshold are still measured:

- `levelsweep_chunk_00`: 19.38 s before, 2.72 s after; specific inspected images, demand rendering.
- `empty_receiver_before`: 19.10 s before, 0.93 s after; specific inspected images, demand rendering.
- `empty_receiver_after`: 16.18 s before, 1.04 s after; specific inspected images, demand rendering.

<!-- TASK_TIMES_END -->

## Checks and limitations

Final worker checks: the complete main source build passes (`final-build.log`).
The 38 retained completed checks plus the dependency-correct remaining **39/39**
checks pass: **77 distinct main registrations**, including every slow main case,
all 20 conversions, the installed-route contract and required dependency stages.
`final-remaining-tests.log` / `.xml` retain that 39-check run's **592.00 s**
real total; this is expressly not the main-suite total. The separately scoped
recovery `installed_texture_payloads` check passes in **20.46 s**
(`final-recovery-slow.log` / `.xml`). All 60 slow registrations now have actual
before/after times in the table. Source hashes still match `final-source-sha256.json`;
authentic entry backups remain unchanged; task sources are LF. Python syntax and
task-only `git diff --check` pass. Existing registration properties and assertion
counts remain as audited. Full main/recovery verification was pending at that
worker handoff; the completed supervisor result is recorded below.

The initial timing-only CTest selection omitted intermediate `DEPENDS` stages.
It was stopped through the managed execution session; an OS `taskkill` attempt
was denied by the sandbox and was not retried. A process check confirmed the
source-test tree had stopped. `partial-slow-tests.log` retains 38 completed
passing checks; interrupted assessment stages are not counted as passes.
`selection-correction.json` records the correction, and
`remaining-with-dependencies.txt` recursively closes every remaining dependency
while retaining completed checks. This changes only this worker's measurement
selection, not registered scheduling, assertions, timeouts or protected controls.
Private partial run directories and all frozen baseline evidence are preserved.

### Completed supervisor verification (2026-11-15 correction)

The [pre-task full verification log](../../build/agent-runs/20261115T065957-22780-c25dfb5b/verification.log)
and [completed supervisor verification log](../../build/agent-runs/loop-bea97485bf464c098da1a44b276433ba/0001-verify-679451/verification.log)
record these full registered results:

| Suite | Before result | Before real total | After result | After real total |
| --- | --- | ---: | --- | ---: |
| Main | 280/280 passed | 1217.67 s | 281/281 passed | 1017.66 s |
| Recovery | 84/84 passed | 21.89 s | 84/84 passed | 18.41 s |

The main-suite total decreased by **200.01 s (16.4%)** in these full runs.
The additional main registration is the `frame_consumption` contract regression;
all existing registrations remain. These are full-suite CTest real totals,
distinct from the per-registration scoped timings and serial paired-driver
measurements above. Scheduling and contention still limit speedup attribution.

The [independent review](../../build/agent-runs/loop-bea97485bf464c098da1a44b276433ba/0001-review-1ad214/decision.json)
accepted the implementation/equivalence evidence for the first four criteria and
requested this documentation correction for the fifth. This section supersedes
the worker handoff's pending-verification statement while preserving its scoped
measurements and limitations. This revision changes only this report; no build
or test was rerun, and the completed supervisor run is cited by its exact path.

No original-fidelity or human-playability claim is made. Existing timing,
depth/clipping and other parks, historical failures and completion criteria remain
unchanged. The conservative unlatched-sprite fallback can retain rendering even
for an otherwise safe observer. Tests using the ordinary wall clock retain full
rendering because changing wall cost would change their elapsed-time records.

Task files: `src/app/{main.cpp,FrameConsumption.h}`;
`src/__test__/{test_levelsweep.cpp,test_hatch_timing.cpp,test_empty_receiver.cpp,
FrameConsumptionAudit.h,test_frame_consumption.cpp}`; `CMakeLists.txt`;
`cmake/frame_consumption.cmake`; `tools/measure_frame_consumption.py`; this report.
No subagents, concurrent source writers, original run, install, commit, push, reset, clean
or stash was used.

Handoff: demand rendering and equivalence evidence are ready for acceptance review; completed supervisor verification passed 281/281 main and 84/84 recovery tests, with main-suite totals 1217.67 s before and 1017.66 s after.
Next action: independently review this report correction for test-render-skip-001; leave task needs_review until accepted. The supervisor owns any required subsequent verification.
