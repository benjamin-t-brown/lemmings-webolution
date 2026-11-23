# Level name above the level menu

**needs_review.** Mobile request V5 (2026-11-23) adds the current level name above the
in-level menu. In **Just Climb** (`MM_CLIMB_002`, node 0), the previous
`runApplicationLeg → openMenu → LevelMenu::draw` path composed the five action rows
without a heading. Direct simulated play had no level menu. [Request ledger](../playtest/2026-11-23-mobile-request.md)

`MapScene::build` and `LevelMenu::load` now share the case-insensitive LEVELMAP.DAT
lookup. The menu uses the resolved archive ID on map entry, Retry and direct play
(`--level ID --sim`). It falls back to the ID for a missing or empty display title.
Direct play shares the existing menu session for input, sound options, Retry and
Quit to map, and retains its diagnostic entry clock and recorder mode.
[Level startup](../../src/app/main.cpp) · [Metadata lookup](../../src/assets/GameData.cpp)

The provisional heading uses the installed `^FONT.BMP` in black with the map hover's
red (220, 50, 50) shadow offset two pixels down and right. Each line is centred
within the 640-pixel frame. A title over 604 glyph pixels wraps at a space, or at
a glyph boundary within an overlong word. Each extra line moves drawing and row
hit testing down 31 pixels. A title exceeding eight lines ends with an ellipsis
to keep all five rows on screen. Focus, pointer actions and the touch Menu plate
use their own existing rectangles and row indices. [Layout and input](../../src/app/LevelMenu.cpp)

## Evidence and checks

- Static REA used the registered x86 `Lemmings Revolution.exe`, SHA-256
  `32675bdb49866e76502713251fdfd2b09078c2481c72ca22ff68ebf4b3737a2f`.
  Ghidra pseudocode at `0x48F51B` shows the five-row builder. The instructions at
  `0x48F522–0x48F541` and xref at `0x48F532` corroborate its setup and table
  `0x4C4228`. The builder delegates to the row widgets and shared initialization.
  Map hover instructions at
  `0x49AF2D–0x49B02A` give the shadow offset and colours. The retained original
  images establish black over red. REA was closed without a snapshot.
  [Static evidence](../../build/level-menu-title-001/rea.json) · [Map hover evidence](map-hover-text-001.md)
- Source-built SDL frames show **Just Climb** opened with Escape after map entry
  and with O after Try Again. All 102 installed titles were measured. Node 40,
  `E17_KF_1`, **Build 'em up ....then bring em down**, is both longest at 35
  characters and widest at 356 glyph pixels. It fits on one line at the ordinary
  row positions. The direct probe opened it via the touch Menu plate, tapped the
  heading, opened Options, retried and quit to the map.
  [Map frame](../../build/level-menu-title-001/frames/just-climb-from-map.png) ·
  [Retry frame](../../build/level-menu-title-001/frames/just-climb-after-retry.png) ·
  [Longest title frame](../../build/level-menu-title-001/frames/longest-title-direct.png) ·
  [Probe build](../../build/level-menu-title-001/probe-build.log) ·
  [Probe and title census](../../build/level-menu-title-001/probe.log)
- Registered UCRT64 built `smoke_tests`. All 10 focused smoke tests passed,
  including the extended `levelmenu` test for the heading, focus, misses,
  wrapping, shifted hits and touch plate. The old rows-only image expectation
  now includes the requested heading while checking the same row pixels.
  [Build](../../build/level-menu-title-001/final-smoke-build.log) ·
  [Focused smoke](../../build/level-menu-title-001/final-focused-smoke.log)
- All 32 promoted recordings reproduced with the clone CLI, including 31 wins
  and the recorded map exit on The Lone Ranger at step 12893. Checks used private
  save and temporary paths. Probe source and frames are retained, and its
  executable and object were removed. [Per-recording results](../../build/level-menu-title-001/promoted-replays.tsv) ·
  [CLI log](../../build/level-menu-title-001/promoted-replays.log)

Changed code covers `LevelMenu`, `MapScene`, `GameData`, the application entry
and frontend session paths, and their level menu tests and test build entry.

**Handoff** `level-menu-title-001` is ready for review. The supervisor's full smoke gate is pending.\
**Next** Independent review and human confirmation on a phone.

**What to check** Open the menu in **Just Climb**, select Try Again and reopen it.
Expect **Just Climb** centred above Resume both times. In **Build 'em up ....then
bring em down**, open the menu with the touch plate and tap the heading. Expect
the full name to fit above the rows and the menu to stay open.
