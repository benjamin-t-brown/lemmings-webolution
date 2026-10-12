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
```

Point the tooling at your installation with `GAME_DIR`:

```sh
make -C src info GAME_DIR="C:/Games/Lemmings Revolution"
```

Tests that read real game data skip themselves when `WEBOLUTION_GAME_DIR` is
unset, so the suite is green on a machine without the game.

## Status

Phase 1 — asset layer, in progress.

- [x] `.box` archive format decoded and validated
- [x] Repo skeleton, CMake presets, test suite
- [x] `Box` / `AssetLibrary` reader with patch overlay (`src/assets/`)
- [x] `lrtool` CLI: `info`, `list`, `extract`, `dump`
- [x] Input shim so the original can be driven by script (`tools/dinput-shim/`)
- [ ] LRZ texture decoder — next
- [ ] `.ALF`, `.LTX`, `.SPR`, `LEVELMAP.DAT`, `LEVELS.MAP`
- [ ] `.LVL` format
- [ ] Simulation
- [ ] Rendering
