# Release for Windows and Linux

Use GitHub **Actions → Publish desktop release → Run workflow**. Select
**main**, enter a new semantic version such as **0.2.0** without the
`v` prefix, and check **prerelease** if appropriate. GitHub Actions builds
that commit for both platforms and tags it `v0.2.0` after checking both packages.

You can also push a new tag on a commit from public `main`. For example,
from that commit run `git tag v0.2.0` and `git push origin v0.2.0`.
Versions with a prerelease suffix such as `v0.2.0-rc.1` are marked as
prereleases automatically.

Each release contains generated notes and both archives with their `.sha256` files.

- `lemmings-webolution-<tag>-windows-x64.zip` bundles the executable and its DLLs.
- `lemmings-webolution-<tag>-linux-x64.tar.gz` is built on Ubuntu 24.04 and links
  against system SDL2 (2.0.22 or later). Install SDL2 with
  `sudo apt install libsdl2-2.0-0` or your distribution's equivalent.

Both archives include a `README.txt` and `LICENSE`. Players put the two game boxes
or a CD image (`.iso` or raw `.bin`) beside the executable. Linux writes imported
boxes to `GameData/` and progress to `SavedGame.dat` under
`$XDG_DATA_HOME/LemmingsWebolution` or `~/.local/share/LemmingsWebolution` by default.
The Windows data folder is `%LOCALAPPDATA%\LemmingsWebolution`.

The browser game published to GitHub Pages and the desktop build checks use the
same version on `main`. The first numbered build is `1.0.1`, and each subsequent
commit on the first-parent history of `main` increments the patch number. Reruns
keep the same version; a push with several commits can skip patch numbers.

**Check desktop builds** also runs for pull requests. Desktop packages are
published separately using the version entered above or the pushed tag.
