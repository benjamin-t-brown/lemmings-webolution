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

Keep the user-owned game files in the ignored `Lemmings Revolution/` folder at
the repository root. The reference EXE is local-only and must not be added to Git.
Run application commands from this repository root; its default game directory
is `Lemmings Revolution/`. Set CMake's `WEBOLUTION_GAME_DIR` to that folder's
absolute path for retail-data tests; tests may skip their
retail portion without it. In a UCRT64 shell, `make -C src` / `make -C src test`
are convenience wrappers. Use `GAME_DIR` for the Makefile's installation path.
See [build rules](.claude/rules/windows-msys2-build.md) for prerequisites/PATH.
Linux/macOS/WebAssembly are not accepted platform targets; a Mac test build
exists with limitations, and the Emscripten preset is not a verified port.

## Current entry point

```powershell
build/cmake/dev-debug/webolution.exe
build/cmake/dev-debug/webolution.exe --game-dir "Lemmings Revolution"
build/cmake/dev-debug/webolution.exe --direct-level
build/cmake/dev-debug/webolution.exe --level MM_CLIMB_002 --sim
build/cmake/dev-debug/webolution.exe --level MM_CLIMB_002 --shot build/tmp/frame.png
build/cmake/dev-debug/webolution.exe --map
build/cmake/dev-debug/webolution.exe --frontend
```

- No options, or only `--game-dir PATH`: open the connected frontend. The
  directory may also come from `WEBOLUTION_GAME_DIR` or the default above.
- Every other option retains its explicit diagnostic route unless `--frontend`
  is supplied. This includes `--level`, `--sim`, `--sim-ticks`, `--shot`, `--map`,
  `--map-flags`, `--frames`, `--window-shot`, `--native` and rendering diagnostics.
  `--direct-level` requests the previous bare-launch SDL diagnostic: retail
  `MM_CLIMB_002`, 3D tube route and camera controls, with injected walker fixtures
  rather than a running simulation. Use `--sim` for simulation.
- `--sim`: optional simulation, actor drawing/picking, counters and skill input.
  Click the first eight plates or press 1–8, then click a lemming. The last two
  commands are inactive. P toggles standard pause. Enter (including keypad Enter)
  or Pause/Break toggles examine pause, a clone addition. In examine pause,
  drag with either mouse button to look around, scroll to zoom, and point at a
  switch to see what it controls. P or Esc resumes play. Standalone unpaused Esc
  closes. Native gameplay and complete input fidelity remain unaccepted.
- Frontend entry: New Game → confirmation → map → simulated retail level.
  Press Return twice, then click the first unlocked node near 206,204. Results
  connect to map/retry.
  Up/Down moves main-menu focus; Return on Play Current Game opens the loaded
  progress map without resetting it. Return on New Game → Yes resets progress.
  Saved progress: a won level writes the original `SavedGame.dat` layout (1504
  bytes: int32 flag, then 5×25 records of int32 A/B/C; C is the completion flag)
  to the clone's own copy at `%LOCALAPPDATA%\LemmingsWebolution\SavedGame.dat`,
  and the next start loads it. New Game → Yes writes the all-zero image there.
  Loss, retry, abandon and exit do not save. A wrong-size or unreadable file is
  reported on stderr, starts empty and is left alone until a later save succeeds.
  The installed game's `SavedGame.dat` is never read, imported or written, and
  the clone refuses any save path under the `--game-dir` tree. This is the
  recovered file layout, not a claim of retail save compatibility; the meaning of
  the flag, A and B is unknown and they are preserved byte-for-byte.
  In connected play, unpaused ESC opens Resume/Try Again/Quit-to-map/Options
  (Options unsupported). O from pause opens the menu; O-close returns to pause,
  while ESC/Resume returns to play. Other main-menu actions are incomplete.
  Explicit `--frontend` selects this route even with diagnostic options or
  `--direct-level`; combining it with `--map`/`--map-flags` remains an error.
  Default entry adds no movie playback or full frontend fidelity.
  Gameplay sound cues: ordinary startup owns one SDL mixer across the frontend and
  level legs (`src/audio/CuePlayback.cpp`; `--no-audio` skips it, a missing device
  or WAV prints a stderr note and the game stays silent). Routed, each with a
  retained static play-site read: `Ohno` (bomber fuse end) and `FallSplat` (long
  fall; the sim's `Splat` is mapped to `FallSplat`, since the retail `Splat` cue is
  never played). Unrouted, no retained play site: `Chink` (assign), `Explode`,
  `Yippee` (exit), `Letsgo`, all menu/frontend sound and music. This is cue
  routing only: no claim of native sample timing, volume, mixing or resampling.
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
[entry point](src/app/main.cpp). The mixer is connected for the two evidenced
gameplay cues above; music, menu sound and a complete menu/map/audio runtime are
not integrated.

## Verification

The [status ledger](docs/STATUS.md) is the current home for accepted scopes,
failures, parks and missing features. Bounded camera/tube/actor/panel passes do
not establish complete rendering or gameplay. Native full-frame and simulation
acceptance remain unfinished. Unit and SDL-dummy tests prove our implementation's
behavior; only a scoped comparison with the original establishes native agreement.

Asset tooling remains available: `make -C src info`, `dump`, `png`, and
`lvlmap LEVEL=MM_FLOAT`; `lrtool` also exposes archive and level inspection.
