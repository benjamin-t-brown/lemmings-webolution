# Lemmings Webolution

An open reimplementation of **Lemmings Revolution** (Psygnosis Leeds / Take-Two, 2000)
in C++23 on SDL2 + OpenGL, building natively on MSYS2/UCRT64, Linux, macOS and
WebAssembly.

The aim is a 1:1 mechanical match with the original, starting at its native 640x480.

**This project ships no game data.** It reads `lemmings.box` and `patch.box` from an
existing Lemmings Revolution installation at runtime, the way OpenRCT2 and devilutionX
do. You need your own copy of the game.

See [PLAN.md](PLAN.md) for the reverse-engineering notes and the phased plan.

## Status

Phase 0 — groundwork. Nothing runs yet.

- [x] `.box` archive format decoded and validated (`tools/proto/box.py`)
- [ ] Repo skeleton / CMake presets
- [ ] Asset layer
- [ ] `.LVL` format
- [ ] Simulation
- [ ] Rendering
