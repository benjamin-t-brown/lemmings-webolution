# Switch hover highlighting

Playtest 22 M3 requested a clone addition for levers, pressure pads, and laser
switches. Previously the level frame picked actors and hatches and drew no
feedback over these switches. Designed with love (`M11_KF_1`, node 33) has three
laser switches, all using the pressure-pad class. Seafood Sarnie
(`MM_BLOCK_BUILD_BASH`, node 15) has the drain switch.
[Playtest 22](../playtest/2026-11-19-playtest-22.md)

## Link and draw path

- **Static evidence.** Ghidra 12.1.4 opened the registered x86
  `Lemmings Revolution/Lemmings Revolution.exe`, SHA-256
  `32675bdb49866e76502713251fdfd2b09078c2481c72ca22ff68ebf4b3737a2f`.
  Pseudocode and instructions agree on `0x433AC0 → 0x433B2E → 0x444BA1`.
  The signed test at `0x433B4F–0x433B61` gates the base-id lookup.
  At `0x43199D–0x4319A7` a pad passes its `+0xBC` list to the same dispatcher.
  The Ghidra session closed without a snapshot.
  [Static results](../../build/switch-hover-highlight-001/rea-static.json) · [Lever links](lever-links-research-001.md) · [Pressure pad](pressure-pad-drain-001.md)
- **Clone source.** `runApplicationLeg → materialize → BackdropScene::drawSwitchHover`
  uses the logical SDL cursor, the hatch picker's projected visible mesh bounds, and
  the existing lever and pad presentation bindings. The switch gets a gold outline.
  Cyan outlines follow the pad's list or the union of both lever face lists.
  The lists come from `World::setup()` and resolve through the existing
  `linkLevelGates` construction-order `objectIds` table. The table now also
  carries model identity and a placement anchor for other receivers.
  The draw runs on the composed image before the HUD and cursor. Simulation,
  input recording, replay dispatch, and skill assignment stay on their existing paths.
- **Presentation choices.** Receiver outlines are projected locators. Fluid
  regions and terrain gates use `TubeScene::cylinderScreenBounds`. The fluid
  region's raised footprint locates an empty receiver. Other targets use scene
  model bounds or their stored cylinder position when available. A locator may
  show through terrain and clips at the viewport edges. Switch picking uses the
  same half-open visible rectangle as hatch hover.

## Checks

| Installed view | Resolved targets | Desktop clone frames |
| --- | --- | --- |
| Designed with love, switch 0 at yaw 163 | Fluid 12, 11, 10 | [Hover](../../build/switch-hover-highlight-001/M11_KF_1-yaw-163-hover-paused.png) · [Away](../../build/switch-hover-highlight-001/M11_KF_1-yaw-163-away-after-hover.png) |
| Designed with love, switch 1 at yaw 245 | Terrain gate 5 | [Hover](../../build/switch-hover-highlight-001/M11_KF_1-yaw-245-hover-paused.png) · [Away](../../build/switch-hover-highlight-001/M11_KF_1-yaw-245-away-after-hover.png) |
| Designed with love, switch 2 at yaw 83 | Terrain gates 4 and 3 | [Hover](../../build/switch-hover-highlight-001/M11_KF_1-yaw-83-hover-paused.png) · [Away](../../build/switch-hover-highlight-001/M11_KF_1-yaw-83-away-after-hover.png) |
| Seafood Sarnie, switch 0 at yaw 46 | Fluid 8, 7, 6, 5, 4 | [Hover](../../build/switch-hover-highlight-001/MM_BLOCK_BUILD_BASH-yaw-46-hover-paused.png) · [Away](../../build/switch-hover-highlight-001/MM_BLOCK_BUILD_BASH-yaw-46-away-after-hover.png) |
| `MM_W_012`, lever 2 at yaw 184 | Fluid 5 and 6, initially empty | [Hover](../../build/switch-hover-highlight-001/MM_W_012-yaw-184-hover-paused.png) · [Away](../../build/switch-hover-highlight-001/MM_W_012-yaw-184-away-after-hover.png) |

The desktop SDL probe exercised pointer departure and re-entry, pause, resume,
and the existing fast-forward hold. Holding fast-forward requires the pointer
on its plate, so the outlines cleared on the plate and returned on the switch.
Wasm headless frames reproduced the cyan pixels at the same coordinates in all
five views. The browser interaction remains unverified because headless Chrome
exited during graphics initialization.
[Desktop probe](../../build/switch-hover-highlight-001/frame-probe-final.log) · [Wasm probe](../../build/switch-hover-highlight-001/wasm-frame-probe.log) · [Pixel audit](../../build/switch-hover-highlight-001/frame-pixel-audit.txt) · [Browser startup](../../build/switch-hover-highlight-001/browser-engine.log)

The UCRT64 and Emscripten builds passed. The shared Wasm preset already had a
mismatched bundle. The relink updated its Wasm before Windows denied the loader
write. Use the matched task-local bundle for browser review. All 28 promoted
recordings passed on desktop and Wasm with the same final steps and save counts.
The probes and replay runs used private save locations. The supervisor smoke
gate is pending.
[Desktop build](../../build/switch-hover-highlight-001/native-final-build.log) · [Wasm build](../../build/switch-hover-highlight-001/wasm-private-build.log) · [Shared preset write denial](../../build/switch-hover-highlight-001/wasm-cached-build.log) · [Desktop replay results](../../build/switch-hover-highlight-001/native-replay-results.tsv) · [Wasm replay results](../../build/switch-hover-highlight-001/wasm-replay-results.tsv)

Changed `src/app/{BackdropScene.cpp,BackdropScene.h,HoverFeedback.h,RuntimeHud.cpp,TubeScene.cpp,TubeScene.h,main.cpp}`,
`src/sim/{RetailLevel.cpp,World.h}`, and this report.
[Task diff](../../build/switch-hover-highlight-001/production.diff)

Handoff — `needs_review`.  
Next action — Supervisor smoke gate and independent review.

What to check — On **Designed with love**, hover each of the three laser switches.
Expect a gold switch outline and three, one, and two cyan receiver outlines at
yaws 163, 245, and 83 respectively. On **Seafood Sarnie**, hover the lower
walkway's drain switch and expect five cyan outlines. Pause and move the pointer
off and back onto each switch. The outlines should disappear and return with
the pointer.
