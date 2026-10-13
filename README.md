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

The game runs: `make -C src run`. The terrain is a built-in test level, because
`.LVL` is still being reverse engineered -- but the lemmings are drawn with the
original's own artwork, and the level's parameters come from the game's data:

```sh
make -C src run                                   # the test level
build/cmake/dev-debug/webolution --level MM_BLOCK_001_A   # real parameters
build/cmake/dev-debug/webolution --cylinder               # wrapped on the tube
build/cmake/dev-debug/webolution --level MM_CLIMB_002 --cylinder  # with its artwork
build/cmake/dev-debug/webolution --native                 # the original's 640x480
make -C src lvlmap LEVEL=MM_FLOAT                 # unroll a level's geometry
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
- [~] `.LVL` surface: located and parsed. The two `u32` arrays index its
      vertices on a cylindrical lattice (`key+1` = inner/outer, `key+16` =
      next angle, `key+Y` = next ring), which is enough to rebuild quads.
      What remains is rasterising them into the walkable mask, and deciding
      whether the skin tiles also contribute. See PLAN.md 1.6.
- [~] Simulation: terrain with cylinder wraparound, lemming state machine, all
      eight skills, deterministic ticks, the level clock. **Rule constants are
      provisional** until validated against the original via the oracle -- the
      exception being the spawn interval, which comes from `RELEASERATE.PC`.
- [x] Rendering: SDL2 + OpenGL (GL 3.3 core / WebGL2), batched textured quads,
      lemmings drawn from `LEMMINGSLO.SPR`, and both views -- the flat
      unwrapped one and the cylinder projection (`C` toggles), and a named
      level's own skin tiles wrapped onto the tube, both surfaces (`T`
      toggles). The skin is scenery: what is solid is still the simulation's
      own terrain mask, drawn over it, because the mapping between artwork and
      collision is part of the unsolved surface topology.
- [~] Game shell: SDL mixer plays Letsgo / Chink / Ohno / Explode / Splat /
      Yippee from simulation cues. Menus, level select, saves, FMV still open.
- [ ] Linux, macOS and WebAssembly builds. The emscripten preset exists but has
      never been run -- no activated EMSDK here -- so it is untested, not done.
