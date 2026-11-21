# main-export-001 Public source and GitHub Pages

## Scope and changed files

The November 20 release request covers browser startup under /lemmings-webolution/. Playtest 16 F2 recorded a local webolution.js load failure caused by a file ACL. The user reached the game after that build was replaced. The release addresses a separate startup issue traced in the launcher. It previously loaded the threaded engine immediately and requested hosted archives from the domain root. Promoted replays cover in-level input, outside the changed browser startup and cue evidence paths. [Playtest 16 F2](../playtest/2026-11-18-playtest-16.md)

- scripts/export_main.py copies the allowlisted production files and release/main/ overlay byte for byte into build/main-export/. It relocates 13 linked original-format reader units and 14 headers from decomp-lemmings/ to src/assets/retail/. The top-level web/shell.html is an earlier launcher. CMake uses src/web/webolution.html.
- CMakeLists.txt, cmake/webolution_app.cmake and cmake/webolution_sources.cmake configure the runtime. cmake/webolution_tests.cmake and cmake/webolution_test_app.cmake provide the test and fixture targets when WEBOLUTION_BUILD_TESTS is enabled and src/__test__/ exists.
- src/web/webolution.html checks isolation before loading ./webolution.js. The scoped src/web/isolate.js adds COOP same-origin and COEP require-corp. The page explains the first reload and reports if isolation remains unavailable. web-game-data.mjs looks up hosted archives relative to the page and uses the existing CORS download. The root .gitignore allows the isolation script, and scripts/serve-web.py serves it with the existing local isolation headers.
- release/main/ supplies README.md, .gitignore, the three public presets, the MIT LICENSE and .github/workflows/pages.yml. The workflow pins emsdk 5.0.3 and stages index.html, Wasm and the package scripts for the official Pages actions.

## Review revision

src/audio/CuePlayback.cpp stores the changed strings in CueRoute::evidence. Audio follows CuePlayback::drain ? play ? routedCueName ? playName, which uses the cue and sample name. The revision removes 21 development-history matches across 12 string fragments. It retains the play-site addresses, offsets, DAT_/FUN_ names and provisional labels. The retained reference identity is SHA-256 32675bdb49866e76502713251fdfd2b09078c2481c72ca22ff68ebf4b3737a2f. The evidence is copied from the existing source. [Archived HEAD source](../../build/main-export-001/cue-citations/head-CuePlayback.cpp) ? [Original citation hits](../../build/main-export-001/retained-diagnostics/retained-cue-citations.log)

The scanner evaluates every line. It distinguishes individual browser, renderer and threading terms from history by their token context and still detects adjacent history on the same line. The main ? webo::app::runApplication path retains the approved command-line diagnostics, replay support, frame timing, observers, SDL event injection and ApplicationRunOptions::frameSeconds.

| Source line | Before | After |
| --- | --- | --- |
| 20 | `"docs/scratch/ui-sounds-001.md: assignment 0x00455657 tests the candidate "` | `"assignment 0x00455657 tests the candidate "` |
| 24 | `"old/docs/scratch/phase3-round7.md 'When Ohno plays': helper step 0x00414370 "` | `"helper step 0x00414370 "` |
| 26 | `"docs/scratch/letsgo-text-nuke-ohno-001.md identifies that byte as "` | `"the tested byte is "` |
| 29 | `"docs/scratch/work-sounds-001.md: 0x0045E570..0x0045E582 registers Explode "` | `"0x0045E570..0x0045E582 registers Explode "` |
| 34 | `"old/docs/scratch/phase3-round28-die.md: FallSplat (DAT_004E32E0) is played "` | `"FallSplat (DAT_004E32E0) is played "` |
| 38 | `"docs/scratch/exit-yippee-sound-001.md: 0x0045E540..0x0045E552 loads Yippee "` | `"0x0045E540..0x0045E552 loads Yippee "` |
| 42 | `"docs/scratch/gameplay-sounds-001.md: hatch step 0x00425A95 tests its "` | `"hatch step 0x00425A95 tests its "` |
| 44 | `"opening/release; docs/scratch/letsgo-voice-with-text-001.md: player-requested "` | `"opening/release; player-requested "` |
| 45 | `"clone point is the first drawn Let's Go text frame (playtest 11 A12)"},` | `"clone point is the first drawn Let's Go text frame"},` |
| 47 | `"docs/scratch/gameplay-sounds-001.md: destructible-cell arm 0x0040C0AF "` | `"destructible-cell arm 0x0040C0AF "` |
| 49 | `"docs/scratch/work-sounds-001.md: miner success plays at 0x00418D39, "` | `"miner success plays at 0x00418D39, "` |
| 52 | `"docs/scratch/gameplay-sounds-001.md: 0x0040FF54 tests bricks < 4, "` | `"0x0040FF54 tests bricks < 4, "` |

## Verification

- Two exports produced identical scans and SHA-256 manifests for all 213 LF files. Each exported file matched its allowlisted input and the standalone copy. Outside src/web, the 12 cue fragments are the only tracked source changes from HEAD. The existing web data test matches its archived digest. Scanner probes flag the 21 prior cue matches, adjacent history terms, excluded paths, binary signatures, data extensions, non-LF text and the reference-digest check. [First scan](../../build/main-export-001/cue-citations/export-first.log) ? [Second scan](../../build/main-export-001/cue-citations/export-second.log) ? [SHA-256 manifest](../../build/main-export-001/cue-citations/export-second.sha256) ? [Comparison](../../build/main-export-001/cue-citations/export-comparison.txt) ? [Scanner probes](../../build/main-export-001/cue-citations/scanner-negative.log)
- The checkout CMake listings match the archived pre-split listings byte for byte. Native has 218 targets and 256 tests. Emscripten has 34 targets and one test. Test commands and properties match after omitting registration backtraces from the comparison. [Comparison](../../build/main-export-001/cue-citations/cmake-comparison.txt) ? [Native targets before](../../build/main-export-001/native-before-targets.txt) ? [Native targets after](../../build/main-export-001/cue-citations/native-after-targets.txt) ? [Native tests before](../../build/main-export-001/native-before-ctest-N.txt) ? [Native tests after](../../build/main-export-001/cue-citations/native-after-ctest-N.txt) ? [Web targets before](../../build/main-export-001/web-before-targets.txt) ? [Web targets after](../../build/main-export-001/cue-citations/web-after-targets.txt) ? [Web tests before](../../build/main-export-001/web-before-ctest-N.txt) ? [Web tests after](../../build/main-export-001/cue-citations/web-after-ctest-N.txt)
- Fresh builds from the standalone export passed with MinGW-w64 GCC Release and Emscripten 5.0.3 using cached SDL2. Both reached --help, with the WebAssembly build launched by the registered Node. Both build graphs contain zero test translation units and zero CTest registrations. [Native configure](../../build/main-export-001/cue-citations/standalone-native-configure.log) ? [Native build](../../build/main-export-001/cue-citations/standalone-native-build.log) ? [Native startup](../../build/main-export-001/cue-citations/standalone-native-startup.log) ? [Web configure](../../build/main-export-001/cue-citations/standalone-web-configure.log) ? [Web build](../../build/main-export-001/cue-citations/standalone-web-build.log) ? [Node startup](../../build/main-export-001/cue-citations/standalone-web-startup.log)
- The existing cueplayback test passed after recompiling CuePlayback.cpp. It checks that each evidence string is nonempty and cites an address, along with routing and playback behavior. [Target build](../../build/main-export-001/cue-citations/cue-build.log) ? [CTest](../../build/main-export-001/cue-citations/cue-ctest.log)
- A fresh headless browser loaded the rebuilt package from a loopback origin with no isolation headers. It reloaded once, loaded the service worker, JavaScript and Wasm under /lemmings-webolution/, then reached the relative hosted archive lookup. The local server probe returned 200 and isolation headers for the package and both installed archives. [Browser startup](../../build/main-export-001/cue-citations/pages-startup.log) ? [HTTP trace](../../build/main-export-001/cue-citations/pages-startup-http.json) ? [Node contract](../../build/main-export-001/cue-citations/web-contract.log) ? [Local server](../../build/main-export-001/cue-citations/local-server.log)

Supervisor smoke is pending for this revision. The preceding revision passed 95 main smoke tests and 84 recovery tests. Live archive.org transfer, browser save persistence and GitHub Pages deployment remain unverified. [Previous smoke run](../../build/agent-runs/20261121T032836-36712-f7fa87e9/verification.log)

## Included paths

<details>
<summary>213 export files and inclusion reasons</summary>


- `.clang-format` — C++ formatting rules.
- `.gitattributes` — LF line endings.
- `.github/workflows/pages.yml` — Build and publish the browser game to GitHub Pages.
- `.gitignore` — Ignore game data and local build products.
- `CMakeLists.txt` — Configure and link the game.
- `CMakePresets.json` — Provide native debug, native release and Emscripten release builds.
- `LICENSE` — MIT license for the exported source.
- `README.md` — Player setup and build instructions.
- `cmake/webolution_app.cmake` — Compile the application object library.
- `cmake/webolution_sources.cmake` — Select the production translation units.
- `src/app/ActorAnim.cpp` — Game entry, SDL presentation, input, or rendering.
- `src/app/ActorAnim.h` — Game entry, SDL presentation, input, or rendering.
- `src/app/ActorDepth.h` — Game entry, SDL presentation, input, or rendering.
- `src/app/ActorOcclusion.h` — Game entry, SDL presentation, input, or rendering.
- `src/app/ActorPose.cpp` — Game entry, SDL presentation, input, or rendering.
- `src/app/ActorPose.h` — Game entry, SDL presentation, input, or rendering.
- `src/app/ActorSprite.cpp` — Game entry, SDL presentation, input, or rendering.
- `src/app/ActorSprite.h` — Game entry, SDL presentation, input, or rendering.
- `src/app/ActorTerrain.cpp` — Game entry, SDL presentation, input, or rendering.
- `src/app/ActorTerrain.h` — Game entry, SDL presentation, input, or rendering.
- `src/app/ActorVisibility.cpp` — Game entry, SDL presentation, input, or rendering.
- `src/app/ActorVisibility.h` — Game entry, SDL presentation, input, or rendering.
- `src/app/AppEntry.h` — Game entry, SDL presentation, input, or rendering.
- `src/app/BackdropScene.cpp` — Game entry, SDL presentation, input, or rendering.
- `src/app/BackdropScene.h` — Game entry, SDL presentation, input, or rendering.
- `src/app/BuilderBrick.h` — Game entry, SDL presentation, input, or rendering.
- `src/app/Camera.cpp` — Game entry, SDL presentation, input, or rendering.
- `src/app/Camera.h` — Game entry, SDL presentation, input, or rendering.
- `src/app/CameraInput.cpp` — Game entry, SDL presentation, input, or rendering.
- `src/app/CameraInput.h` — Game entry, SDL presentation, input, or rendering.
- `src/app/CameraIntro.cpp` — Game entry, SDL presentation, input, or rendering.
- `src/app/CameraIntro.h` — Game entry, SDL presentation, input, or rendering.
- `src/app/CrtRand.cpp` — Game entry, SDL presentation, input, or rendering.
- `src/app/CrtRand.h` — Game entry, SDL presentation, input, or rendering.
- `src/app/CursorPolicy.cpp` — Game entry, SDL presentation, input, or rendering.
- `src/app/CursorPolicy.h` — Game entry, SDL presentation, input, or rendering.
- `src/app/DeathParticles.h` — Game entry, SDL presentation, input, or rendering.
- `src/app/EntryStartCue.cpp` — Game entry, SDL presentation, input, or rendering.
- `src/app/EntryStartCue.h` — Game entry, SDL presentation, input, or rendering.
- `src/app/ExitBalloon.h` — Game entry, SDL presentation, input, or rendering.
- `src/app/FrameConsumption.h` — Game entry, SDL presentation, input, or rendering.
- `src/app/FrameProfile.h` — Game entry, SDL presentation, input, or rendering.
- `src/app/FrontendSession.cpp` — Game entry, SDL presentation, input, or rendering.
- `src/app/FrontendSession.h` — Game entry, SDL presentation, input, or rendering.
- `src/app/FrontendWindow.cpp` — Game entry, SDL presentation, input, or rendering.
- `src/app/FrontendWindow.h` — Game entry, SDL presentation, input, or rendering.
- `src/app/HatchAnimation.cpp` — Game entry, SDL presentation, input, or rendering.
- `src/app/HatchAnimation.h` — Game entry, SDL presentation, input, or rendering.
- `src/app/HoverFeedback.h` — Game entry, SDL presentation, input, or rendering.
- `src/app/LemmingAnim.cpp` — Game entry, SDL presentation, input, or rendering.
- `src/app/LemmingAnim.h` — Game entry, SDL presentation, input, or rendering.
- `src/app/LemmingCountdown.cpp` — Game entry, SDL presentation, input, or rendering.
- `src/app/LemmingMotion.cpp` — Game entry, SDL presentation, input, or rendering.
- `src/app/LemmingMotion.h` — Game entry, SDL presentation, input, or rendering.
- `src/app/LevelEndTransition.cpp` — Game entry, SDL presentation, input, or rendering.
- `src/app/LevelEndTransition.h` — Game entry, SDL presentation, input, or rendering.
- `src/app/LevelMenu.cpp` — Game entry, SDL presentation, input, or rendering.
- `src/app/LevelMenu.h` — Game entry, SDL presentation, input, or rendering.
- `src/app/LevelOutcome.h` — Game entry, SDL presentation, input, or rendering.
- `src/app/LevelStartSequence.h` — Game entry, SDL presentation, input, or rendering.
- `src/app/MapScene.cpp` — Game entry, SDL presentation, input, or rendering.
- `src/app/MapScene.h` — Game entry, SDL presentation, input, or rendering.
- `src/app/Pick.cpp` — Game entry, SDL presentation, input, or rendering.
- `src/app/Pick.h` — Game entry, SDL presentation, input, or rendering.
- `src/app/Raster.cpp` — Game entry, SDL presentation, input, or rendering.
- `src/app/Raster.h` — Game entry, SDL presentation, input, or rendering.
- `src/app/RasterRows.h` — Game entry, SDL presentation, input, or rendering.
- `src/app/Replay.hpp` — Game entry, SDL presentation, input, or rendering.
- `src/app/ResultsScene.cpp` — Game entry, SDL presentation, input, or rendering.
- `src/app/ResultsScene.h` — Game entry, SDL presentation, input, or rendering.
- `src/app/RuntimeCursor.cpp` — Game entry, SDL presentation, input, or rendering.
- `src/app/RuntimeCursor.h` — Game entry, SDL presentation, input, or rendering.
- `src/app/RuntimeHud.cpp` — Game entry, SDL presentation, input, or rendering.
- `src/app/RuntimeHud.h` — Game entry, SDL presentation, input, or rendering.
- `src/app/RuntimePick.cpp` — Game entry, SDL presentation, input, or rendering.
- `src/app/RuntimePick.h` — Game entry, SDL presentation, input, or rendering.
- `src/app/RuntimeSkillPanel.cpp` — Game entry, SDL presentation, input, or rendering.
- `src/app/RuntimeSkillPanel.h` — Game entry, SDL presentation, input, or rendering.
- `src/app/SceneBillboardSource.cpp` — Game entry, SDL presentation, input, or rendering.
- `src/app/SceneBillboardSource.h` — Game entry, SDL presentation, input, or rendering.
- `src/app/SceneClip.cpp` — Game entry, SDL presentation, input, or rendering.
- `src/app/SceneClip.h` — Game entry, SDL presentation, input, or rendering.
- `src/app/SceneDepth.cpp` — Game entry, SDL presentation, input, or rendering.
- `src/app/SceneDepth.h` — Game entry, SDL presentation, input, or rendering.
- `src/app/SceneDiffuse.cpp` — Game entry, SDL presentation, input, or rendering.
- `src/app/SceneDiffuse.h` — Game entry, SDL presentation, input, or rendering.
- `src/app/SceneDispatch.cpp` — Game entry, SDL presentation, input, or rendering.
- `src/app/SceneDispatch.h` — Game entry, SDL presentation, input, or rendering.
- `src/app/SceneFrame.h` — Game entry, SDL presentation, input, or rendering.
- `src/app/SceneMaterialFlags.h` — Game entry, SDL presentation, input, or rendering.
- `src/app/SceneMeshSource.cpp` — Game entry, SDL presentation, input, or rendering.
- `src/app/SceneMeshSource.h` — Game entry, SDL presentation, input, or rendering.
- `src/app/SceneRaster.cpp` — Game entry, SDL presentation, input, or rendering.
- `src/app/SceneRaster.h` — Game entry, SDL presentation, input, or rendering.
- `src/app/SceneVertexPack.cpp` — Game entry, SDL presentation, input, or rendering.
- `src/app/SceneVertexPack.h` — Game entry, SDL presentation, input, or rendering.
- `src/app/SdlShutdown.hpp` — Game entry, SDL presentation, input, or rendering.
- `src/app/SdlWindow.hpp` — Game entry, SDL presentation, input, or rendering.
- `src/app/SimClock.cpp` — Game entry, SDL presentation, input, or rendering.
- `src/app/SimClock.h` — Game entry, SDL presentation, input, or rendering.
- `src/app/SkillPanel.cpp` — Game entry, SDL presentation, input, or rendering.
- `src/app/SkillPanel.h` — Game entry, SDL presentation, input, or rendering.
- `src/app/SoundMenu.cpp` — Game entry, SDL presentation, input, or rendering.
- `src/app/SoundMenu.h` — Game entry, SDL presentation, input, or rendering.
- `src/app/TubeScene.cpp` — Game entry, SDL presentation, input, or rendering.
- `src/app/TubeScene.h` — Game entry, SDL presentation, input, or rendering.
- `src/app/WindowScale.h` — Game entry, SDL presentation, input, or rendering.
- `src/app/main.cpp` — Game entry, SDL presentation, input, or rendering.
- `src/assets/Bitmap.cpp` — Original data decoding or asset lookup.
- `src/assets/Bitmap.h` — Original data decoding or asset lookup.
- `src/assets/Box.cpp` — Original data decoding or asset lookup.
- `src/assets/Box.h` — Original data decoding or asset lookup.
- `src/assets/CellMesh.cpp` — Original data decoding or asset lookup.
- `src/assets/CellMesh.h` — Original data decoding or asset lookup.
- `src/assets/CollisionGrid.cpp` — Original data decoding or asset lookup.
- `src/assets/CollisionGrid.h` — Original data decoding or asset lookup.
- `src/assets/Cylinder.cpp` — Original data decoding or asset lookup.
- `src/assets/Cylinder.h` — Original data decoding or asset lookup.
- `src/assets/GameData.cpp` — Original data decoding or asset lookup.
- `src/assets/GameData.h` — Original data decoding or asset lookup.
- `src/assets/LevelIndex.cpp` — Original data decoding or asset lookup.
- `src/assets/LevelIndex.h` — Original data decoding or asset lookup.
- `src/assets/LevelObjects.cpp` — Original data decoding or asset lookup.
- `src/assets/LevelObjects.h` — Original data decoding or asset lookup.
- `src/assets/LevelStats.cpp` — Original data decoding or asset lookup.
- `src/assets/LevelStats.h` — Original data decoding or asset lookup.
- `src/assets/LightMap.cpp` — Original data decoding or asset lookup.
- `src/assets/LightMap.h` — Original data decoding or asset lookup.
- `src/assets/Lrz.cpp` — Original data decoding or asset lookup.
- `src/assets/Lrz.h` — Original data decoding or asset lookup.
- `src/assets/Png.cpp` — Original data decoding or asset lookup.
- `src/assets/Png.h` — Original data decoding or asset lookup.
- `src/assets/PolyList.cpp` — Original data decoding or asset lookup.
- `src/assets/PolyList.h` — Original data decoding or asset lookup.
- `src/assets/Rar20.cpp` — Original data decoding or asset lookup.
- `src/assets/Rar20.h` — Original data decoding or asset lookup.
- `src/assets/SceneNode.cpp` — Original data decoding or asset lookup.
- `src/assets/SceneNode.h` — Original data decoding or asset lookup.
- `src/assets/Sprite.cpp` — Original data decoding or asset lookup.
- `src/assets/Sprite.h` — Original data decoding or asset lookup.
- `src/assets/SurfaceMesh.cpp` — Original data decoding or asset lookup.
- `src/assets/SurfaceMesh.h` — Original data decoding or asset lookup.
- `src/assets/TextureCache.h` — Original data decoding or asset lookup.
- `src/assets/Wav.cpp` — Original data decoding or asset lookup.
- `src/assets/Wav.h` — Original data decoding or asset lookup.
- `src/assets/codepages.inc` — Original data decoding or asset lookup.
- `src/assets/retail/include/lemmings/animation_playback.hpp` — Header used by the production file readers.
- `src/assets/retail/include/lemmings/box_archive.hpp` — Header used by the production file readers.
- `src/assets/retail/include/lemmings/compiled_level.hpp` — Header used by the production file readers.
- `src/assets/retail/include/lemmings/compiled_level_header.hpp` — Header used by the production file readers.
- `src/assets/retail/include/lemmings/level_entity_prefix.hpp` — Header used by the production file readers.
- `src/assets/retail/include/lemmings/level_gadget_records.hpp` — Header used by the production file readers.
- `src/assets/retail/include/lemmings/level_model.hpp` — Header used by the production file readers.
- `src/assets/retail/include/lemmings/level_name_tables.hpp` — Header used by the production file readers.
- `src/assets/retail/include/lemmings/level_stream.hpp` — Header used by the production file readers.
- `src/assets/retail/include/lemmings/object_base_prefix.hpp` — Header used by the production file readers.
- `src/assets/retail/include/lemmings/sprite_resource.hpp` — Header used by the production file readers.
- `src/assets/retail/include/lemmings/texture_resource.hpp` — Header used by the production file readers.
- `src/assets/retail/include/lemmings/world_coordinates.hpp` — Header used by the production file readers.
- `src/assets/retail/include/lemmings/world_geometry_prefix.hpp` — Header used by the production file readers.
- `src/assets/retail/src/box_archive.cpp` — Compiled production reader for the original file formats.
- `src/assets/retail/src/compiled_level.cpp` — Compiled production reader for the original file formats.
- `src/assets/retail/src/compiled_level_header.cpp` — Compiled production reader for the original file formats.
- `src/assets/retail/src/level_entity_prefix.cpp` — Compiled production reader for the original file formats.
- `src/assets/retail/src/level_gadget_records.cpp` — Compiled production reader for the original file formats.
- `src/assets/retail/src/level_model.cpp` — Compiled production reader for the original file formats.
- `src/assets/retail/src/level_name_tables.cpp` — Compiled production reader for the original file formats.
- `src/assets/retail/src/level_stream.cpp` — Compiled production reader for the original file formats.
- `src/assets/retail/src/object_base_prefix.cpp` — Compiled production reader for the original file formats.
- `src/assets/retail/src/sprite_resource.cpp` — Compiled production reader for the original file formats.
- `src/assets/retail/src/texture_resource.cpp` — Compiled production reader for the original file formats.
- `src/assets/retail/src/world_coordinates.cpp` — Compiled production reader for the original file formats.
- `src/assets/retail/src/world_geometry_prefix.cpp` — Compiled production reader for the original file formats.
- `src/audio/CuePlayback.cpp` — Game sound playback and mixing.
- `src/audio/CuePlayback.h` — Game sound playback and mixing.
- `src/audio/Mixer.cpp` — Game sound playback and mixing.
- `src/audio/Mixer.h` — Game sound playback and mixing.
- `src/audio/OverlapMix.h` — Game sound playback and mixing.
- `src/audio/SpatialSound.h` — Game sound playback and mixing.
- `src/gfx/SpriteState.h` — Renderer-independent sprite and tile geometry.
- `src/gfx/SpriteView.cpp` — Renderer-independent sprite and tile geometry.
- `src/gfx/SpriteView.h` — Renderer-independent sprite and tile geometry.
- `src/gfx/TilePosition.h` — Renderer-independent sprite and tile geometry.
- `src/save/SaveStore.cpp` — Progress and settings storage.
- `src/save/SaveStore.h` — Progress and settings storage.
- `src/save/SavedGame.cpp` — Progress and settings storage.
- `src/save/SavedGame.h` — Progress and settings storage.
- `src/save/SoundSettings.cpp` — Progress and settings storage.
- `src/save/SoundSettings.h` — Progress and settings storage.
- `src/shell/Frontend.cpp` — Frontend menus, HUD, and level progress.
- `src/shell/Frontend.h` — Frontend menus, HUD, and level progress.
- `src/shell/HudText.cpp` — Frontend menus, HUD, and level progress.
- `src/shell/HudText.h` — Frontend menus, HUD, and level progress.
- `src/shell/MenuRender.cpp` — Frontend menus, HUD, and level progress.
- `src/shell/MenuRender.h` — Frontend menus, HUD, and level progress.
- `src/shell/PauseCaption.cpp` — Frontend menus, HUD, and level progress.
- `src/shell/PauseCaption.h` — Frontend menus, HUD, and level progress.
- `src/shell/Progress.cpp` — Frontend menus, HUD, and level progress.
- `src/shell/Progress.h` — Frontend menus, HUD, and level progress.
- `src/sim/RetailLevel.cpp` — Gameplay simulation and level setup.
- `src/sim/RetailLevel.h` — Gameplay simulation and level setup.
- `src/sim/Terrain.cpp` — Gameplay simulation and level setup.
- `src/sim/Terrain.h` — Gameplay simulation and level setup.
- `src/sim/World.cpp` — Gameplay simulation and level setup.
- `src/sim/World.h` — Gameplay simulation and level setup.
- `src/web/Browser.cpp` — Browser startup, game-data setup, or WebAssembly support.
- `src/web/Browser.h` — Browser startup, game-data setup, or WebAssembly support.
- `src/web/WebMixer.cpp` — Browser startup, game-data setup, or WebAssembly support.
- `src/web/isolate.js` — Browser startup, game-data setup, or WebAssembly support.
- `src/web/node-mount.js` — Browser startup, game-data setup, or WebAssembly support.
- `src/web/web-audio.js` — Browser startup, game-data setup, or WebAssembly support.
- `src/web/web-game-data.mjs` — Browser startup, game-data setup, or WebAssembly support.
- `src/web/webolution.html` — Browser startup, game-data setup, or WebAssembly support.

</details>

Handoff `needs_review`. The supervisor runs smoke before review.\
The lead can create `main` from `build/main-export/` after review.

**What to check** Open the Pages subpath in a fresh browser profile and import game data. Expect one reload, then select **Just Climb** (`MM_CLIMB_002`) from the map and confirm the level renders and accepts a Climber assignment.
