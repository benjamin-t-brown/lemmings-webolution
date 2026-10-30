# Lemmings Webolution

A work-in-progress 1:1 reimplementation of **Lemmings Revolution** (2000), in
C++23 with SDL2 presentation and a software 3D rasterizer at native 640x480.
The stock executable is the authority, not classic Lemmings or our own tests.

**No game data is shipped.** Supply your own installation; the asset library
reads `lemmings.box` with the `patch.box` overlay. Work on `decompile`.

- [Completion plan](PLAN.md): goal, sequence and next work package.
- [Current status](docs/STATUS.md): implemented versus native-verified scope.
- [Documentation map](docs/README.md) and [startup lessons](docs/LESSONS.md).

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
- `--sim`: optional simulation, actor drawing/picking, counters and skill input.
  Click the first eight plates or press 1–8, then click a lemming. The last two
  commands are inactive. P toggles pause; paused ESC resumes. Standalone unpaused
  ESC closes. Native gameplay and complete input fidelity remain unaccepted.
- `--frontend`: opt-in New Game → confirmation → map → simulated retail level.
  Press Return twice, then click the first unlocked node near 206,204. Results
  connect to map/retry; completion is in memory, not installed-save progress.
  Up/Down moves main-menu focus; Return on Play Current Game opens the current
  session's map without resetting progress. It does **not** load the retail save.
  In connected play, unpaused ESC opens Resume/Try Again/Quit-to-map/Options
  (Options unsupported). O from pause opens the menu; O-close returns to pause,
  while ESC/Resume returns to play. Other main-menu actions are incomplete.
- `--shot`: optional headless fixture rendering. `--skill-panel`, `--hud-ticks`
  and `--pause-caption` are diagnostic flags; `--sim` draws the runtime panel.
- `--frames N --window-shot FILE`: save the final window-loop CPU frame. Set
  `SDL_VIDEODRIVER=dummy` to exercise this route without touching the desktop.
- `--flat` is a debug view. The old OpenGL renderer, placeholder frontend,
  music/SFX shell and its old toggles are retired under `old/`, not the current app.
- `--map`: conditional retail background/node display, also usable with `--shot`.
  `--map-flags FILE` takes exactly 102 signed decimal int32 values in native node
  order. Default zero flags are a SOURCE fixture; standalone map has no entry controls.

Production wiring: [source inventory](cmake/webolution_sources.cmake),
[entry point](src/app/main.cpp). Audio decoding/mixer and progress helpers exist,
but a complete menu/map/audio runtime is not integrated.

## Verification

The [status ledger](docs/STATUS.md) is the current home for accepted scopes,
failures, parks and missing features. Bounded camera/tube/actor/panel passes do
not establish complete rendering or gameplay. Native full-frame and simulation
acceptance remain unfinished. Unit and SDL-dummy tests prove our implementation's
behavior; only a scoped comparison with the original establishes native agreement.

Asset tooling remains available: `make -C src info`, `dump`, `png`, and
`lvlmap LEVEL=MM_FLOAT`; `lrtool` also exposes archive and level inspection.
