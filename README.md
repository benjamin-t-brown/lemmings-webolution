# Lemmings Webolution

An unofficial reimplementation of **Lemmings Revolution** (2000) in C++23.
It uses SDL2 and a software 3D renderer. This project is independent of the
owners of Lemmings Revolution and has no affiliation with them.

This project was created with a vast amount of help from AI development tools.

The game aims for 1 to 1 replication of the original Lemmings Revolution with some small quality of life improvements.  It is close, but not there yet. 

What's implemented:
  - All mechanics implemented: every level is completable
  - Resolution scaling
  - Pause to examine
  - Unlock all levels button

What's missing:
  - Languages beside English
  - Exact 1 to 1 timings between retail and project
  - .avi cutscenes

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

### Touch controls

In the browser, holding a touch-first device upright automatically switches
levels to portrait mode. The job plates move to two rows at the bottom, with
**Fast Fwd**, **Nuke**, **2 Finger Rotate** and the level stats at the top. Turn the device
sideways for the original layout. The title screen and in-level menu also support
portrait mode. The level map uses the wider layout.

- **Tap** a skill plate, then tap a lemming to assign it. Tapping the scene
  also moves the targeting cursor to that spot. Tapping the **Menu** button at
  the top opens and pauses the level menu; tap a menu row to choose it.
- **One-finger drag** on the scene moves the targeting cursor. The cursor sits
  about one fingertip above your finger so it stays visible; release assigns
  the selected skill at that aimed position, then the cursor settles at the
  spot where you lifted your finger.
- Hold **2 Finger Rotate** with one finger, then drag with another to rotate the
  camera. You can lift and reposition the dragging finger while keeping the
  first finger down. You can also plant the first finger on an empty part of the scene.
  Touch rotation starts in the opposite direction to mouse drag; the
  **Reverse camera drag** option flips both together.
- Use the single **+ / −** plate near the bottom of the scene to toggle zoom in and out.
  It zooms toward the current targeting cursor, and the label shows the next
  action. Pinch zoom was removed.
- **Hold** the fast-forward plate to speed up time. A second finger can tap a
  lemming without stopping fast-forward.
- Each entrance has **−** and **+** text buttons on either side for release
  rate changes. Tapping changes one step; holding repeats like holding the
  mouse button. After a tap, the hatch briefly shows `RATE n`, then returns to
  the normal remaining count. You can also **hold** the hatch and slide **up**
  to raise its rate or **down** to lower it.
- **Double-tap** the Nuke plate to use it.

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
