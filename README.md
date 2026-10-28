# Lemmings Webolution

A work-in-progress 1:1 reimplementation of **Lemmings Revolution** (2000), in
C++23 with SDL2 presentation and a software 3D rasterizer at native 640x480.
The stock executable is the authority, not classic Lemmings or our own tests.

**No game data is shipped.** Supply your own installation; the asset library
reads `lemmings.box` with the `patch.box` overlay. See [PLAN.md](PLAN.md) for
implemented features, verified scope, signed gates and next deliverables.

## Build on Windows

The authoritative build uses CMake 3.28+, Ninja and MSYS2 UCRT64 GCC/SDL2.
From PowerShell:

```powershell
.\scripts\Invoke-Ucrt64.ps1 "cmake --preset dev-debug"
.\scripts\Invoke-Ucrt64.ps1 "cmake --build --preset dev-debug"
.\scripts\Invoke-Ucrt64.ps1 "ctest --preset dev-debug"
```

Set CMake's `WEBOLUTION_GAME_DIR` for retail-data tests; tests may skip their
retail portion without it. In a UCRT64 shell, `make -C src` / `make -C src test`
are convenience wrappers. Use `GAME_DIR` for the Makefile's installation path.
See [build rules](.claude/rules/windows-msys2-build.md) for prerequisites/PATH.
Linux/macOS/WebAssembly are not accepted platform targets; a Mac test build
exists with limitations, and the Emscripten preset is not a verified port.

## Current entry point

```powershell
build/cmake/dev-debug/webolution.exe --game-dir "C:/progs/Lemmings Revolution"
build/cmake/dev-debug/webolution.exe --level MM_CLIMB_002 --sim
build/cmake/dev-debug/webolution.exe --level MM_CLIMB_002 --shot build/tmp/frame.png
build/cmake/dev-debug/webolution.exe --map
build/cmake/dev-debug/webolution.exe --frontend
```

- Default: SDL window, retail `MM_CLIMB_002`, 3D tube route and camera controls;
  retained injected walkers are fixtures, **not** a running simulation.
- `--sim`: optional simulation, actor drawing, picking, runtime counters and debug skill input.
  P toggles pause; paused ESC resumes. In connected `--frontend` play, unpaused
  ESC opens the [in-level menu](docs/scratch/in-level-menu.md): Resume, Try Again,
  Quit-to-map and a currently unsupported Options row. O from pause opens it;
  O closes it back to pause, while menu ESC/Resume returns to play. Standalone
  diagnostic unpaused ESC still closes.
  The cached skill panel now shows live stock and selection: click the first eight
  plates or press 1–8, then click a lemming. The last two commands are inactive.
  Icon poses remain fixed fixtures; mouse hit regions and count clamping are app
  policy, not verified native behavior. End-to-end gameplay remains unaccepted.
- `--shot`: optional headless fixture rendering. `--skill-panel`, `--hud-ticks`
  and `--pause-caption` remain diagnostic headless flags. Retail cursor artwork
  and shared hover/picking are integrated; native pixel/input acceptance remains
  open. `--skill-panel` retains its headless fixture
  behavior; `--sim` draws the runtime panel automatically.
- `--frames N --window-shot FILE`: save the final window-loop CPU frame. Set
  `SDL_VIDEODRIVER=dummy` to exercise this route without touching the desktop.
- `--flat` is a debug view. The old OpenGL renderer, placeholder frontend,
  music/SFX shell and its old toggles are retired under `old/`, not the current app.
- `--map`: conditional retail background/node display, also usable with `--shot`.
  Optional `--map-flags FILE` takes exactly102signed decimal int32 values in native
  node order; default all-zero flags are a SOURCE fixture, not installed progress.
  This standalone fixture mode has no entry controls or title/hover/cursor/save adapter.
- `--frontend`: opt-in New Game → “Continue?” → map → simulated retail level.
  Press Return twice, then click an unlocked node (the first is near 206,204).
  New Game and Yes/No also accept clicks; the map's Back to Main Menu footer
  accepts click or Return. Other menu actions and keyboard choice
  traversal are not implemented. Progress starts empty in memory; the installed
  save is never reset. A [native comparison](docs/scratch/frontend-native-comparison-2026-10-28.md)
  verifies the connected entry and menu/confirmation/one fresh-map frame; physical
  input equivalence and full frontend acceptance remain open. Direct `--level`/`--map`
  and default entry behavior are preserved.
  First-level frontend entry includes the original's statically recovered
  three-second opening camera move; matched native animation timing remains unverified.
  Skill selection requires available stock. A successful actor assignment resumes
  a paused level; rejected clicks leave it paused. Overlapping walkers now use
  recovered distance ordering and skip already-assigned Climbers; full native
  eligibility remains incomplete ([overlap checks](docs/scratch/first-level-overlap-selection.md)).
  Actors culled by the tube's angular visibility rule are omitted from drawing
  and picking; first-level upright Walkers also require support for Climber
  ([scope and tests](docs/scratch/first-level-pick-visibility-support.md)).
  [Cursor feedback](docs/scratch/runtime-cursor-hover.md) uses the same picker:
  retail target/no-target artwork, available-skill panel feedback and right-drag
  Grab. This is an explicitly bounded SDL adapter, not full native input fidelity.
  Connected `--frontend` play now shows [win/loss results](docs/scratch/level-results-map-return.md).
  Click **Continue** to return to the map, or **Try Again** after a loss. Earned
  completion stays in memory; New Game resets it without changing the installed
  save. Loss-results [Quit returns to the map](docs/scratch/loss-results-quit.md);
  Retry skips the first-level opening camera move. Results keyboard focus, movies
  and native end timing remain incomplete.
  The [“Let's Go!” entry cue](docs/scratch/level-entry-start-cue.md) now shrinks into
  place and pauses with gameplay; its clock starts earlier on Retry.
  The [pending finish delay](docs/scratch/level-end-transition.md) continues simulation
  and preserves the original's latched outcome, including late-rescue timeout losses.

Production wiring: [source inventory](cmake/webolution_sources.cmake),
[entry point](src/app/main.cpp). Audio decoding/mixer and progress helpers exist,
but a complete menu/map/audio runtime is not integrated.

## Verified scope, not completion

- Asset readers, collision grids, tube mesh generation, camera and sprite
  orientation have retained byte/live/offline evidence in [lessons](docs/LESSONS.md).
- A1–A7 pass their bounded/amended gates. B1's menu fixture is conditionally
  accepted; [New Game navigation is connected](docs/scratch/frontend-runtime-integration.md),
  but full frontend acceptance remains unfinished. The optional map base layer is
  [source-tuple verified](docs/scratch/phase-b3j-app-base-map.md), not B3 acceptance.
- C1's two amended panel views pass; the [runtime panel](docs/scratch/runtime-skill-panel-integration.md)
  is connected with live stock/selection, not native animation/input acceptance.
  C2 runtime counters/pause are connected and
  tested in our SDL window; native full-HUD/hover fidelity remains unaccepted.
- C3 full-frame remains **FAIL: 0.0635481898 vs 0.02** ([C3bv source render](docs/scratch/phase-c3bv-production-pack.md)).
- Simulation rules/gadgets are implemented, but discrete 5 Hz stepping was later
  retracted. D1/D3 and the D2 digger clock remain user-parked, not accepted.

See the linked checks for dated results/scopes; editing this README alone does
not establish fresh build/live evidence. Current direction is C3 → C2 →
B2 → B3; tools support named fidelity decisions, not completion by themselves.

Asset tooling remains available: `make -C src info`, `dump`, `png`, and
`lvlmap LEVEL=MM_FLOAT`; `lrtool` also exposes archive and level inspection.
