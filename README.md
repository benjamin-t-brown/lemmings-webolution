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
```

- Default: SDL window, retail `MM_CLIMB_002`, 3D tube route and camera controls;
  retained injected walkers are fixtures, **not** a running simulation.
- `--sim`: optional simulation, actor drawing, picking and debug skill input.
  This is not accepted end-to-end retail gameplay or a faithful skill-panel UI.
- `--shot`: optional headless fixture rendering. `--skill-panel`, `--hud-ticks`
  and `--pause-caption` currently compose headless HUD fixtures, not windowed HUD.
- `--flat` is a debug view. The old OpenGL renderer, placeholder frontend,
  music/SFX shell and its old toggles are retired under `old/`, not the current app.

Production wiring: [source inventory](cmake/webolution_sources.cmake),
[entry point](src/app/main.cpp). Audio decoding/mixer and progress helpers exist,
but a complete menu/map/audio runtime is not integrated.

## Verified scope, not completion

- Asset readers, collision grids, tube mesh generation, camera and sprite
  orientation have retained byte/live/offline evidence in [lessons](docs/LESSONS.md).
- A1–A7 pass their bounded/amended gates. B1's menu fixture is conditionally
  accepted; runtime navigation/map remains unfinished.
- C1's two amended panel views pass. C2 counters/caption are partial fixtures;
  cursor and runtime HUD/pause integration remain unfinished.
- C3 full-frame remains **FAIL: 0.0635481771 vs 0.02** (retained baseline).
- Simulation rules/gadgets are implemented, but discrete 5 Hz stepping was later
  retracted. D1/D3 and the D2 digger clock remain user-parked, not accepted.

These are accepted historical results, not a fresh build/live claim from this
README update. Current direction is integrated retail-level progress, C3 → C2 →
B2 → B3; tools support named fidelity decisions, not completion by themselves.

Asset tooling remains available: `make -C src info`, `dump`, `png`, and
`lvlmap LEVEL=MM_FLOAT`; `lrtool` also exposes archive and level inspection.
