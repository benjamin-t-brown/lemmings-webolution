# Lemmings Webolution

An unofficial reimplementation of **Lemmings Revolution** (2000) in C++23.
It uses SDL2 and a software 3D renderer. This project is independent of the
owners of Lemmings Revolution and has no affiliation with them.

The game aims for 1 to 1 replication of the original Lemmings Revolution with some small quality of life improvements.  It is close, but not there yet. 

What's implemented:
  - All mechanics implemented: every level is completable
  - Resolution scaling
  - Pause to examine
  - Unlock all levels button
  - Mobile controls /w vertical layout

What's missing:
  - Languages beside English
  - Exact 1 to 1 timings between retail and project
  - .avi cutscenes

This project was primarily implemented with ai development tools.

## Play in your browser

Open the [browser game](https://benjamin-t-brown.github.io/lemmings-webolution/).
The page may reload once to enable WebAssembly threads.

The project does not include original game data.

## Play on Windows or Linux

Download and extract the Windows x64 zip or Linux x64 tarball. On Windows,
run `lemmings_webolution.exe`. On Linux, install system SDL2 2.0.22 or later with
`sudo apt install libsdl2-2.0-0` or your distribution's equivalent, then run
`./lemmings_webolution`. [GitHub Releases](https://github.com/benjamin-t-brown/lemmings-webolution/releases)

Place `lemmings.box` and `patch.box` (any filename case), or a Lemmings
Revolution CD image (`.iso` or raw `.bin`), beside the executable. Use
`--game-dir PATH` to point to the two boxes in another folder.

Webolution writes `SavedGame.dat` and imported `GameData/` under
`%LOCALAPPDATA%\LemmingsWebolution` on Windows and
`$XDG_DATA_HOME/LemmingsWebolution` on Linux. The Linux default is
`~/.local/share/LemmingsWebolution` when `XDG_DATA_HOME` is unset, empty, or relative.

See [BUILDING.md](BUILDING.md) for build-from-source instructions.

## License

The [MIT License](LICENSE) covers this project's source code only. The
original game and its data remain the property of their respective owners.
