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
shell (see Carcer's README for the package list).

```sh
make -C src            # build
make -C src test       # build and run the suite
make -C src info       # summarise the archives in your game installation
make -C src dump       # extract every archive entry to build/assets
make -C src png        # decode every texture to PNG in build/png
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
      `tools/harness/read_lvl_grid.py` reads it from a `.LVL`, `read_grid.py`
      reads it live. **Not yet wired into the simulation** -- `--level` still
      plays on the lattice-derived mask below. See PLAN.md 1.6.
- [~] `.LVL` surface: cylindrical lattice → quads → occupancy mask. This is
      what the engine **draws**; it is not what a lemming stands on, so the
      ring stride and the face rules are rendering concerns now. `--level`
      plays on it for want of the real mask being wired up, and it is far
      sparser than the original: 1.35-23% solid against the engine's 35-63%.
      `lrtool lvlstats` surveys that gap across all 102 levels and 22 of 101
      pass the test that a lemming leaving an entrance lands survivably --
      a measure of our raster, not of the levels. Skin tiles are scenery and
      do not encode solidity.
- [~] Simulation: terrain with cylinder wraparound, lemming state machine, all
      eight skills, deterministic ticks, the level clock. The step rate (5 Hz),
      the walk (one column per step), and the spawn interval (`RELEASERATE.PC`)
      are measured, and so is the climb: one world unit per step, two mask
      rows. The fall, the float, and the umbrella and splat distances are
      read off the binary rather than measured -- 20.0 world units per
      second at `0x004B0A18`, so 8 mask rows per tick and 4 under the
      umbrella, which opens at 20 rows fallen and splats past 60 -- and
      they still owe a live series. The skill constants are classic
      stand-ins. PLAN.md 2.3 has the addresses, and
      `tools/harness/read_steppers.py` re-runs the read.
- [x] Rendering: SDL2 + OpenGL (GL 3.3 core / WebGL2), batched textured quads,
      lemmings drawn from `LEMMINGSLO.SPR`, and both views -- the flat
      unwrapped one and the cylinder projection (`C` toggles), and a named
      level's own skin tiles wrapped onto the tube, both surfaces (`T`
      toggles). With `--level`, collision comes from the surface lattice and
      the skin is drawn behind it as scenery.
- [~] Game shell: SDL mixer plays Letsgo / Chink / Ohno / Explode / Splat /
      Yippee from simulation cues. Menus, level select, saves, FMV still open.
- [ ] Linux, macOS and WebAssembly builds. The emscripten preset exists but has
      never been run -- no activated EMSDK here -- so it is untested, not done.
