# Lemmings Webolution

An open reimplementation of **Lemmings Revolution** (Psygnosis Leeds / Take-Two, 2000)
in C++23 on SDL2 + OpenGL, building natively on MSYS2/UCRT64, Linux, macOS and
WebAssembly.

The aim is a 1:1 mechanical match with the original, starting at its native 640x480.

**This project ships no game data.** It reads `lemmings.box` and `patch.box` from an
existing Lemmings Revolution installation at runtime, the way OpenRCT2 and devilutionX
do. You need your own copy of the game.

See [PLAN.md](PLAN.md) for the reverse-engineering notes and the phased plan.

## Build

Needs CMake 3.28+, Ninja and a C++23 compiler. On Windows use an MSYS2 UCRT64
shell. Agents in PowerShell should wrap commands with
`scripts/Invoke-Ucrt64.ps1` (same pattern as Carcer) so they get that
environment instead of guessing at MSVC or bare `g++`.

UCRT64 packages:

```sh
pacman -S \
  mingw-w64-ucrt-x86_64-gcc \
  mingw-w64-ucrt-x86_64-cmake \
  mingw-w64-ucrt-x86_64-ninja \
  mingw-w64-ucrt-x86_64-make \
  mingw-w64-ucrt-x86_64-pkgconf \
  mingw-w64-ucrt-x86_64-SDL2
```

```sh
make -C src            # build
make -C src test       # build and run the suite
make -C src info       # summarise the archives in your game installation
make -C src dump       # extract every archive entry to build/assets
make -C src png        # decode every texture to PNG in build/png
```

From PowerShell:

```powershell
.\scripts\Invoke-Ucrt64.ps1 "make -C src test"
```

Point the tooling at your installation with `GAME_DIR`:

```sh
make -C src info GAME_DIR="C:/Games/Lemmings Revolution"
```

Tests that read real game data skip themselves when `WEBOLUTION_GAME_DIR` is
unset, so the suite is green on a machine without the game.

## Status

The game runs: `make -C src run`. With `--level` the terrain is rebuilt from
the level's surface lattice; without it a hand-built test level is used.
Lemmings use the original's artwork when an installation is reachable:

```sh
make -C src run                                   # the test level
build/cmake/dev-debug/webolution --level MM_CLIMB_002 --cylinder
build/cmake/dev-debug/webolution --native
make -C src lvlmap LEVEL=MM_FLOAT                 # unroll surface vertices
# lrtool lvlmask <game-dir> <level> <out.png>     # rasterised collision mask
# lrtool lvlstats <game-dir> [--csv]              # corpus terrain survey
```

- [x] `.box` archive format decoded and validated
- [x] Repo skeleton, CMake presets, test suite
- [x] `Box` / `AssetLibrary` reader with patch overlay (`src/assets/`)
- [x] `lrtool` CLI: `info`, `list`, `extract`, `dump`
- [x] Input shim so the original can be driven by script (`tools/dinput-shim/`)
- [x] LRZ texture decoder, verified byte-identical to the original
- [x] `.ALF` alpha planes, the `^ + $ _ ~` name sigils, BMP -> RGBA, PNG export
- [x] `.LTX` strings (8 languages, 3 codepages), `LEVELMAP.DAT`, `LEVELS.MAP`, `RELEASERATE.PC`
- [x] `.WAV` sounds: all 73 retail files decode (17 rates, 8/16-bit, mono/stereo) with load-time resampling
- [x] `.SPR` sprite sheets: 7,354 frames, animation strips render correctly
- [x] `.LVL` header: title, seven header words, and the asset manifest, parsing
      exactly across all 102 levels
- [x] `.LVL` cylinder: the tube is identical in every level -- 7 rings, 32
      angular steps of 11.25 degrees, radii 50 and 60 -- which pins the
      coordinate system the simulation wraps in. `lrtool lvlmap` draws it.
- [~] `.LVL` collision: the terrain mask is **stored in the level file** --
      256x96 `u32` flag cells, length-prefixed, read verbatim by the engine
      into the object it collides against. Verified byte-identical to the
      running original's own grid on five independently entered levels, and
      present exactly once in each of the 102 files.
      Anchored in the chunk order, not by signature: it is the third array
      after the `Cylinder` chunk, which lands in all 102 levels. `--level`
      **plays on it** -- the simulation now runs at the engine's own one cell
      per world unit, and `Rules` carries the world-unit constants read off
      the binary. All five live dumps reproduce byte for byte from the file
      alone. See PLAN.md 1.6.
- [~] `.LVL` surface: one counted array of 49,858 surface indices after the
      Cylinder preamble, drawn as quads. It is what the engine **draws**; the
      stored collision grid is what a lemming stands on. `--level` plays on
      the grid. The earlier "22 of 101 releasable" figure was a `header[3]`
      window into that index, not the levels. Three `MM_CLIMB_002` constants
      in the face builder are marked as render-only fits. Skin tiles are
      scenery and do not encode solidity.
- [~] Simulation: terrain with cylinder wraparound, lemming state machine, all
      eight skills, deterministic ticks, the level clock. The step rate (5 Hz),
      the walk (one column per step), and the spawn interval (`RELEASERATE.PC`)
      are measured, and so is the climb: one world unit per step, one cell.
      The fall, the float, and the umbrella and splat distances are
      read off the binary rather than measured -- 20.0 world units per
      second at `0x004B0A18`, so 4 cells per tick and 2 under the
      umbrella, which opens at 10 cells fallen and splats past 30 -- and
      they still owe a live series. A released lemming is created from two
      hatch int16s plus the constant 0.5 (`0x0042DE51`), not from the
      entrance object's translation. The sim still drops from that
      translation. The skill constants are classic
      stand-ins. PLAN.md 2.3 has the addresses, and
      `tools/harness/read_steppers.py` re-runs the read.
- [x] Rendering: SDL2 + OpenGL (GL 3.3 core / WebGL2), batched textured quads,
      lemmings drawn from `LEMMINGSLO.SPR`, and both views -- the flat
      unwrapped one and the cylinder projection (`C` toggles), and a named
      level's own skin tiles wrapped onto the tube, both surfaces (`T`
      toggles). With `--level`, collision comes from the stored grid and
      the skin is drawn behind it as scenery.
- [x] Game shell: LTX menus (main / confirm / options / map / results),
      progress unlocks in `webolution_progress.dat`, in-level HUD, SFX +
      looping `MUSIC\LEMMING01..13`. FMV still open. `--level` bypasses the
      shell; default language is UK (English) when present.
- [ ] Linux, macOS and WebAssembly builds. The emscripten preset exists but has
      never been run -- no activated EMSDK here -- so it is untested, not done.
