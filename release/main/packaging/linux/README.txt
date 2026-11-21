Lemmings Webolution for Linux (x64)
==================================

Extract the archive to a folder. Install system SDL2 2.0.22 or later with
sudo apt install libsdl2-2.0-0 or your distribution's equivalent.

* Copy lemmings.box and patch.box from your Lemmings Revolution installation
  or the CD root (LEMMINGS.BOX and PATCH.BOX) next to webolution. Both
  the 1.0 and 1.1 patch.box are supported.
* You can also place a Lemmings Revolution CD image (.iso or raw .bin) next
  to webolution. On its first start, the game copies the boxes to GameData/
  under its per-user data folder.

Run ./webolution from the extracted folder. Use --game-dir PATH to select
another folder containing both boxes. SavedGame.dat and GameData/ are under
$XDG_DATA_HOME/LemmingsWebolution or ~/.local/share/LemmingsWebolution by default.

Press Enter on New Game and again on Continue?, then click an unlocked
level on the map.
Select a skill on the panel or with 1-8, then click a lemming to assign it.
Left-click a hatch to slow releases or right-click to speed them up.
Right-drag to rotate the view. Space or Ctrl zooms and Backspace flips the view.
P toggles standard pause. Enter (including keypad Enter) or Pause/Break toggles
examine pause. While examining, drag with either mouse button, scroll to zoom,
and point at a switch to see what it controls. P or Esc resumes play. Use O to
open the level menu while paused, or Esc during play. Hold Fast Forward on the
panel to speed up play.

This is an unofficial reimplementation. The MIT license in LICENSE covers
this project's code. The original game and its data belong to their owners.
