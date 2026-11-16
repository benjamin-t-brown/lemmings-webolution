# window-scale-001: integer window scaling

Status: **needs_review**. Playable v1, Playtest 6 V9
([human ledger](../playtest/2026-11-15-playtest-6.md)). Difficulty: routine;
recommended model: Sonnet; this worker used the explicit Codex provider override.
Full registered main/recovery verification is **pending supervisor execution**,
then independent acceptance review. No queue or shared controls were edited.

## Failure, dependency and production path

The user found the ordinary game window too small. The accepted
[shared-window integration](frontend-window-quit-001.md), independently accepted
2026-11-13 (queue review and
`build/agent-runs/20261113T165533-32228-13cdd598/verification.log`), supplies the
single presentation lifetime needed to enlarge frontend and gameplay together.
This task implements V9 only; simulation, raster size, timing and skill rules
are unchanged.

Before, `runApplication` owned `SdlWindow`, whose `ensure` hard-coded 640x480 and
a software SDL renderer. `runFrontendWindow` and the level leg uploaded their
640x480 RGBA/RGB textures and copied them to that window. Both SDL event loops
sent window coordinates directly to gameplay/frontend consumers. Enlarging only
the window would therefore break map clicks, panel selection and actor picking.

After:

1. `parseArgs` accepts **`--window-scale 1|2|3`**, strictly rejecting missing or
   other values before a window or progress store is created. The option alone
   preserves ordinary frontend routing. `windowScale` defaults ordinary
   interactive frontend/level windows to 2; `--frames` (including an explicit
   zero), `--shot`, diagnostic `--map`, and observer-driven automation keep 1
   unless the caller explicitly chooses a scale.
2. `runApplication` constructs the shared `SdlWindow` with that scale. Standalone
   level/map windows use the same adapter. `ensure` requests 640x480, 1280x960 or
   1920x1440 client dimensions; it reuses the same window across frontend/level
   legs. It prefers an accelerated final blit with software fallback, without a
   vsync request. Real Windows runs selected `direct3d11`, flags0xa, vsync0;
   dummy CTest runs used the fallback.
3. Frontend and level textures remain 640x480, explicitly nearest-neighbor.
   `SDL_RenderCopy` fills the fixed-size window; no logical-size SDL event filter
   is installed. The software raster, camera viewport and CPU images retain
   their old dimensions. Headless diagnostics retain their existing resolution,
   including the existing distinction between default and `--native` shots.
4. Each frontend/level SDL event passes through `SdlWindow::gameCoordinates`
   exactly once. Absolute coordinates become floor(window coordinate / scale),
   including negative coordinates. The level menu, map, panel, hover, cursor,
   assignment and drag-onto command paths all receive these same game pixels.
   Relative camera deltas divide by scale in floating point, preserving small
   deltas; keyboard, simulation and camera timing are unchanged.
5. PNG/framebuffer saves happen before presentation scaling.
   `scripts/Play-Clone.ps1` already passes only `--game-dir`, so it inherits 2x
   without a script edit. Its release binary was rebuilt (see below).

## Observed before and after

The production driver is `src/__test__/test_windowscale.cpp`. It queues public
SDL events inside the source-built clone and reads the actual SDL presenter
output before presentation. Link wrappers observe only this clone process;
they are not installed hooks, original code or original-game captures.
Frontend observations occur after present, so their corresponding presenter
image is retained by the wrapper before the swap.

The installed route is ordinary menu -> New Game confirmation -> map node0 at
(206,204) -> **MM_CLIMB_002** -> natural hatch release -> a supported Walker ->
pause -> Climber plate -> hover -> successful Climber assignment. The owned
0.2-second test clock advances the World through the production loop; there is
no World mutation or injected actor. A second installed **MM_BASH_002** route
clicks the Basher plate to prove an actual selection change from index0 to5;
the first tutorial only stocks Climber, already selected by default.

| Check | Authentic before | Final after |
| --- | --- | --- |
| Ordinary startup window | 640x480 | 1280x960 |
| Explicit 1x / 2x / 3x | option unsupported; all 640x480 | 640x480 / 1280x960 / 1920x1440 |
| Shared creations per menu -> map -> level route | 1 | 1 |
| Map hover/click game coordinates | (206,204), selects MM_CLIMB_002 | same at all scales |
| Climber panel coordinates | (50,25) | same at all scales |
| Walker hover | (318,287), actor0 | same at all scales |
| Climber assignment | actor0, consumes one Climber | same at all scales |
| Basher plate selection | supplemental after check | (50,225), index0 ->5 at every scale |
| CPU images | 640x480 | 640x480; 24/24 PNG comparisons byte-identical |
| Presenter RGB versus exact integer enlargement of CPU image | 0 differing values | 0 differing values, software and Direct3D11 |

Input events use the **last physical pixel** of each logical pixel cell, e.g.
map (413,409) at 2x, rather than testing only exact multiples of the scale.
Additional arithmetic checks exhaust the horizontal pixel cells and boundaries
for scales1-3, including negative coordinates and unchanged relative deltas.
The driver asserts ordinary unbounded frontend and direct-level defaults, invalid
scales, and the automation default. Bounded screenshot output at 2x and headless
output with scale3 are byte-identical to the unscaled 640x480 frontend shot.

Evidence, relative to repository root:

- Authentic before executable: `build/window-scale-001/before-driver-corrected.exe`;
  log `before-corrected.log`; images/audit/summary under
  `build/window-scale-001/before-1794780272211035900/`.
  Built and executed before production edits. Entry copies and source hashes are
  retained under `before-source/` and `provenance.json`; all three copied production
  files still match their entry hashes. `HEAD-main.cpp` retains the requested git-show.
- Final registered dummy run:
  `build/agent-native/main/window-scale-test/after-1794780830792874000/`.
  `audit.csv` records scale, client/output dimensions, hover target, Basher
  selection and zero presenter RGB differences; `summary.txt` records failures0
  and unchanged installed/normal saves.
- Final Windows driver run:
  `build/window-scale-001/desktop-final-1794780878853453200/`, with
  `build/window-scale-001/desktop-final.log`. Its same assertions all pass on
  actual Direct3D11 output, at all three scales.
- [Before 1x level](../../build/window-scale-001/before-1794780272211035900/scale-1-hover-window.png),
  [final 2x level](../../build/window-scale-001/desktop-final-1794780878853453200/scale-2-hover-window.png),
  [final 2x map](../../build/window-scale-001/desktop-final-1794780878853453200/scale-2-map-window.png),
  [2x assigned Climber](../../build/window-scale-001/desktop-final-1794780878853453200/scale-2-assigned-window.png),
  [2x Basher plate](../../build/window-scale-001/desktop-final-1794780878853453200/scale-2-basher-panel-window.png).
  These are full client presenter captures, excluding OS title bars. The 1280x960
  map/level images were visually inspected; exhaustive RGB comparison also proves
  full coverage without filtering blur or cropping.
- `build/window-scale-001/image-comparison.json` records all24 comparisons of
  final dummy/desktop CPU PNGs, across three scales and four scenes, against the
  authentic pre-edit CPU PNGs. All match exactly. `summarize.py` reproduces this
  comparison, performance statistics and `provenance.json` using only retained data.

Correction to the first development driver: it initially compared a frontend
CPU frame with the next presented page and attempted Climber hover on a Faller,
which the existing picker correctly excludes. That failed run and log remain
under `before-1794780210876882200/` and `before.log`. The driver was corrected
while production sources were still unchanged; the corrected baseline above
passes. Those initial failures were not production scaling defects.

## Frame time on MM_CLIMB_002

Final Windows measurement: registered GCC16.1, -O2/-g, assertions enabled,
SDL Windows + Direct3D11 without vsync. Each run uses the production loop,
`--sim-ticks 20 --frames 240 --window-scale 1|2`, an owned 1/60-second simulation
clock and a private save sink. No observer, pixel readback or PNG capture runs
inside measured frames. Existing `FrameProfile` measures complete frames; CSV
writes are outside the measured interval. Discard the first30 frames, retaining
210/run. Six sequential runs were ordered 1,2,2,1,1,2; three runs/630 measured
frames per scale.

| Scale | Run mean frame times (ms) | Combined mean | Approx. fps |
| --- | --- | ---: | ---: |
| 1x | 44.4319, 44.5216, 44.5439 | **44.4991 ms** | 22.4724 |
| 2x | 44.4467, 44.5143, 44.6455 | **44.5355 ms** | 22.4540 |

Difference: **+0.0364 ms (+0.0817%)**, within observed run variation (standard
deviation of run means: 0.0593 /0.1011 ms). No measurable scale-dependent frame
rate regression was resolved by this measurement. This is a bounded clone
measurement on this host, not a general frame-rate or original-timing claim.

Exact final logs are `build/window-scale-001/performance-{1..6}-bench{1|2}.log`;
`performance-summary.json` lists each corresponding run directory, raw
`frames.csv`, frame counts, means, medians and presentation costs. The final run
directories are:

- `bench1-1794780901979475700`, `bench2-1794780912953244800`,
  `bench2-1794780923901898100`, `bench1-1794780934861213100`,
  `bench1-1794780945825442600`, `bench2-1794780956792587600`, all under
  `build/window-scale-001/`.

Preliminary software-only Windows samples were 44.0987 ms at1x and44.7381 ms at2x
(`bench1-1794780432748173400`, `bench2-1794780464982197100`). Their presentation
cost rose from0.2412 to0.8031 ms. That observed extra blit cost motivated the
accelerated final presenter, with the same backend selection at every scale.
Other preliminary samples remain retained and are excluded from the final
six-run summary. Software fallback is functional and pixel-exact, but a host
without acceleration can retain that small extra enlargement cost.

## Checks, preservation and limits

- `build/window-scale-001/final-build.log` and `panel-driver-build.log`: scoped
  registered UCRT64 builds succeeded. No compiler/library from another MSYS2
  installation was used.
- `build/window-scale-001/targeted-final.log`: **9/9** CTests passed:
  runtimewindow, frontendwindow, frontendentry, windowscale,
  windowscale_frames_1x, windowscale_frames_2x, frontendwindowquit,
  levelinteraction and frontend_menu. The three new tests register the public
  input/presenter and frame-profile demonstrations for supervisor replay.
- `build/window-scale-001/framebuffer-regression.log`: existing
  frame_profile_MM_CLIMB_002 passed unchanged; all150 production frames were
  measured and its final PNG retained the registered SHA-256
  `61933b158580b1c869dd545500c7592b02c26cc36bd67ae2d93ec79cec4413ba`.
- `build/window-scale-001/release-build.log`: rebuilt the optimized
  `build/cmake/gcc-release/webolution.exe` used by Play-Clone.ps1. The script
  itself was not invoked, so no normal progress was copied/imported.
- Each driver run injects a unique private `FileSaveSink` below its evidence
  directory. Full existence/byte comparisons assert installed and normal user
  saves are untouched. No test assertion, existing timeout or protected control
  was changed. Pre-existing edits to hooks.log, RUN_STATUS.md and tasks.json
  remain outside this task. No commits, agents, original execution, debugger,
  installed hooks or original capture were used.

Static context was reused from the accepted shared-window report and
[A4 window controls](phase-a4-window-controls.md). The registered reference hash
was rechecked read-only:
`32675bdb49866e76502713251fdfd2b09078c2481c72ca22ff68ebf4b3737a2f`.
Retained A4 instructions at0x454010..0x4540E4 describe native cursor clamping;
they do not establish native integer window scaling or filtering. No new REA
session was needed. **Provisional clone policy**, labelled in code: 2x default,
1/2/3 selection, presentation filtering and pointer conversion. Original
observation of client/render dimensions and pointer hit behavior under resizing
would settle the corresponding original behavior; it is not required or
authorized for this task. Other DPI configurations/backends and perceived
responsiveness still need human use. This route demonstrates input/scaling,
not a human win, original fidelity or full-project completion. All historical
fidelity gates and timing/other parks remain unchanged.

Changed files: `src/app/{SdlWindow.hpp,FrontendWindow.cpp,main.cpp}`,
`src/__test__/test_windowscale.cpp`, `CMakeLists.txt`, and this report.

Handoff: window-scale-001 is implemented with retained before/after images, exact pointer/pixel checks and measured 1x/2x frame times; needs_review.
Next action: supervisor full registered main/recovery verification, then independent acceptance review and human use of the default 2x play window.
