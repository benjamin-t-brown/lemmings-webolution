# Switch hover surface highlight

Playtest 24 P1 in **Designed with love** (`M11_KF_1`, node 33) found that hovering a laser switch drew rectangles. The source projected receiver bounds even when the receiver contributed no pixels. The replacement is a clone addition that brightens the switch and its visible receivers before the HUD and cursor. No rectangle outline remains. [Playtest 24](../playtest/2026-11-20-playtest-24.md)

## Draw path

`runApplicationLeg → materialize` calls `renderFrame` and then `BackdropScene::drawSwitchHover`. Hover uses the accepted visible-mesh pick, pad/laser list, union of lever face lists, and signed `LevelSetup::objectIds` lookup. It shares the saturating RGB addition from `TubeScene::drawHoveredWalker`. The scene rasterizers retain the source RGB and ownership of the last visible fragment while drawing the frame. Accepted foreground writes and actor coverage replace ownership. Switch hover adds the selected fragments once, using neutral texture RGB for textured surfaces and the drawn colour for vertex-coloured surfaces. The rule for blended geometry is provisional and uses the last visible contributor with its source blend weight. Only the presentation image receives the redraw. [Source diff](../../build/switch-hover-highlight-002/revision/production.diff)

| Target | Visible geometry used |
| --- | --- |
| Switch, model gates and doors, laser, teleport, named hazard | `BackdropScene::renderStage` tags placed meshes and billboards by root. Laser effects follow the existing gadget pass and teleport selection uses its base root. |
| Fluid receiver | `BackdropScene::renderFluid` tags accepted fragments of the animated receiver mesh by stream index, including its sides where visible. |
| Terrain or counter gate | `TubeScene::render` tags cell faces using the moving gate vertices and preserves the tag through the opaque tube cache. `renderCounterGates` tags the visible panel and digits. |
| Occupancy sensor | Tagged type 2 has no admitted clone draw path. `BackdropScene::drawSwitchHover` calls the sensor callback supplied by `materialize`. The callback invokes `TubeScene::drawHoveredSensor` to draw a provisional contact ring at the cylinder-volume centre, depth-tested against scenery and actors. The ring was checked by source inspection. |

Registry entries with no admitted visual model and no established sensor volume contribute no visible pixels. On the registered x86 EXE, SHA-256 `32675bdb49866e76502713251fdfd2b09078c2481c72ca22ff68ebf4b3737a2f`, Ghidra 12.1.4 pseudocode and instructions agree that `0x454ACA..0x454B5C` copies the lemming body at actor `+0x128` and sets selector 2. Retained evidence identifies `ONE/ONE` and neutral RGB 255. The original draw's effective depth state remains unknown. The first attempt's REA session closed without a snapshot. [Static reads](../../build/switch-hover-highlight-002/rea-static.json), [retained lemming evidence](hover-lemming-highlight-001.md), [retained sensor evidence](occupancy-sensors-001.md)

## Checks

The first smoke build failed because `test_backdropscene` links `BackdropScene.cpp` without `TubeScene.cpp`. The application now supplies the sensor callback, so `BackdropScene` links independently. The revised `smoke_tests` target linked `test_backdropscene.exe`, and all 95 main smoke-labelled tests passed, including the 102-level sweep. The supervisor's full smoke profile, which includes the recovery suite, is pending. [Failed build](../../build/agent-runs/loop-6e2d848a78e44875925e8faccdd88b0d/0001-verify-0727d5/verification.log), [revised build](../../build/switch-hover-highlight-002/revision/smoke-tests-build.log), [main smoke results](../../build/switch-hover-highlight-002/revision/smoke-tests-ctest.log)

The revised Emscripten build used the frozen workspace cache and the SDK's existing SDL2 sources. The desktop SDL probe ran `materialize` through pause, pointer departure and return, and a seven-pass fast-forward hold. That hold occupies the pointer on the HUD. In paused comparisons, receiver pixels brightened on hover and returned to their previous values on departure, excluding the cursor and the HUD timer glyph from the scene comparison. [Wasm build](../../build/switch-hover-highlight-002/revision/wasm-build.log), [SDL and desktop frame probe](../../build/switch-hover-highlight-002/revision/native-frame-probe.log)

The retained scene probe run on the revised desktop and Node builds matched every hover and away pixel. Every away frame matched the unmarked scene. Every changed pixel was a visible selected fragment and increased RGB by its recorded additive sample. Browser interaction still needs the human playtest. [Node frame probe](../../build/switch-hover-highlight-002/revision/wasm-frame-probe.log), [pixel audit](../../build/switch-hover-highlight-002/revision/frame-and-replay-parity.txt)

| Installed view | Brightened pixels | Frame pairs |
| --- | --- | --- |
| Designed with love, yaw 163 | 346 switch, 0 from fluid IDs 12, 11, 10 | [Hover](../../build/switch-hover-highlight-002/revision/native-frames/M11_KF_1-yaw-163-hover.png), [away](../../build/switch-hover-highlight-002/revision/native-frames/M11_KF_1-yaw-163-away.png) |
| Designed with love, yaw 245 | 361 switch, 24 gate ID 5 | [Hover](../../build/switch-hover-highlight-002/revision/native-frames/M11_KF_1-yaw-245-hover.png), [away](../../build/switch-hover-highlight-002/revision/native-frames/M11_KF_1-yaw-245-away.png) |
| Designed with love, yaw 83 | 260 switch, 0 from gate IDs 4 and 3 | [Hover](../../build/switch-hover-highlight-002/revision/native-frames/M11_KF_1-yaw-83-hover.png), [away](../../build/switch-hover-highlight-002/revision/native-frames/M11_KF_1-yaw-83-away.png) |
| Seafood Sarnie, yaw 46 | 345 switch, 519 fluid across IDs 8, 7 and 6. IDs 5 and 4 contributed 0. | [Hover](../../build/switch-hover-highlight-002/revision/native-frames/MM_BLOCK_BUILD_BASH-yaw-46-hover.png), [away](../../build/switch-hover-highlight-002/revision/native-frames/MM_BLOCK_BUILD_BASH-yaw-46-away.png) |
| `MM_W_012`, yaw 184 | 191 lever, 0 from the two initially empty receivers | [Hover](../../build/switch-hover-highlight-002/revision/native-frames/MM_W_012-yaw-184-hover.png), [away](../../build/switch-hover-highlight-002/revision/native-frames/MM_W_012-yaw-184-away.png) |

All 28 promoted recordings reproduced with `--game-dir "Lemmings Revolution" --replay <file>` on the revised desktop and Node builds, with matching final steps and save counts. The per-recording tables give each command log path. Desktop runs used private LocalAppData and Node used its private MEMFS save root. [Desktop replay results](../../build/switch-hover-highlight-002/revision/native-replay-results.tsv), [Node replay results](../../build/switch-hover-highlight-002/revision/wasm-replay-results.tsv)

This revision changed `src/app/BackdropScene.cpp`, `src/app/BackdropScene.h`, `src/app/main.cpp`, and this report. The first attempt also changed `src/app/{HoverFeedback.h,Raster.cpp,Raster.h,SceneRaster.cpp,SceneRaster.h,TubeScene.cpp,TubeScene.h}` and `src/assets/{CellMesh.cpp,CellMesh.h}`.

Handoff **needs_review**.  
Next action is the supervisor smoke gate, which is pending, followed by independent review.

**What to check.** In **Designed with love**, hover the laser switch controlling gate 5 with that gate in view (yaw 245). In **Seafood Sarnie**, hover the drain switch on the lower walkway (yaw 46). The switch and the exposed gate or water pixels should brighten. Move the pointer away while paused and expect the scene to return to its regular colours.
