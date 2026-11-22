# Lemmings Webolution

An unofficial reimplementation of **Lemmings Revolution** (2000) in C++23.
It uses SDL2 and a software 3D renderer. This project is independent of the
owners of Lemmings Revolution and has no affiliation with them.

## Play in your browser

Open the [browser game](https://benjamin-t-brown.github.io/lemmings-webolution/).
The page may reload once to enable WebAssembly threads.

The project ships no original game data. On the setup screen, choose your
installation folder, select its `lemmings.box` and `patch.box` files, or open
a Lemmings Revolution CD image. You can also download the image through the
setup screen or get it from the [Internet Archive item](https://archive.org/details/lemmings-revolution).
The browser stores imported data and saved progress on this device.

Select **Start game**, then start a new game and choose an unlocked level.
Select a skill on the panel or press **1–8**, then click a lemming to assign it.
Press **Enter** (including keypad Enter) or **Pause/Break** to toggle examine
pause. While paused, drag with either mouse button, scroll to zoom, and point
at a switch to see what it controls. **P** or **Esc** resumes play.

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
