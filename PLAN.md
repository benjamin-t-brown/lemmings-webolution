# Lemmings Webolution — Plan

Reset 2026-10-18. The previous plan and all 31 research rounds are preserved under `old/`.

**Read `docs/LESSONS.md` first.** It carries every fact worth keeping from the old plan, so
nothing below needs to re-explain the formats, the measured constants or the traps.

## The goal, restated

The old goal was *the simulation is correct*. It was reached, across **31 research rounds** —
every behavioural constant in `src/sim/World.h` is a read of the shipped bytes plus a live
series, all nine classic skills step, and every gadget in the game is settled and
implemented — and it produced something that does not look like Lemmings Revolution. It booted into a flat 2D debug view,
drew every lemming from sprite view 0 regardless of viewing angle, and showed a placeholder
menu in our own font.

**The new goal is visible and tactile fidelity: a screenshot of ours should be hard to tell
from a screenshot of the original, and it should play in 3D on the rotating tube.**

That is a falsifiable goal, which is why it is phrased that way.

## Method change: read the code, diff the pixels

Phase 3 could measure behaviour by watching memory while the game ran. Rendering mostly
cannot — you cannot measure a projection matrix by watching a lemming walk. So:

- **Static reading is now primary.** [REA](https://github.com/morluto/rea) is installed and
  verified against the stock exe; see `docs/REA_SETUP.md`. Use it to read the render setup,
  the sprite view selection, the frontend state machine and the HUD layout.
- **The oracle becomes the screenshot.** `tools/harness/screengrab.py` captures the original
  through a desktop BitBlt. The new comparison is *our frame against its frame at the same
  level and camera*. An image diff will never be pixel-exact through a D3D11 wrapper, so the
  first round that uses it must **set the "close enough" threshold explicitly** rather than
  discover it later.
- **The old standard still holds.** Cite the address and the bytes. A decompiler line is not
  a measurement. Where a thing is READ and not MEASURED, label it so.

## Where the tree stands after the reset

Kept, correct, and still building — **14 tests green**:

- `src/assets/**` — the container, LRZ, `.SPR`, `.WAV`, `.LVL` chunk walk, the stored
  collision grid, placed objects, hatch records.
- `src/sim/**` — `Terrain` and `World`: every constant measured, all nine classic skills,
  blocker fields, the 7x7 blast stencil, and every gadget — anti-grav, trampoline bounce,
  water/acid drown, teleport, SpeedPad, lever, splat-exemption zones and inverted gravity.
- `src/audio/Mixer` — opens an SDL device and plays retail cues. Kept but currently unwired.
- `src/shell/Progress` — unlock and progression logic, no SDL.
- `tools/harness/**` — 40+ re-runnable live-read scripts.

Moved to `old/`, to be rebuilt rather than retrofitted:

- `old/src/gfx/**` — the flat-first renderer. Its camera constants (`radius 60`,
  `distance 150`, `fovY 0.9`) are **guesses**, which is exactly what Phase A replaces by
  reading the original.
- `old/src/main.cpp`, `old/src/shell/Shell.*` — the placeholder entry point and menu.
- `old/src/__test__/test_cylinder.cpp`, `test_cylinderview.cpp`, `test_sprite.cpp` — each
  depended on `src/gfx`.

**Two debts this created, both deliberate and both to be paid in Phase A:**

1. **There is no runnable executable** until Phase A4. `src/main.cpp` is gone and the
   `webolution` CMake target with it.
2. **Asset-side test coverage regressed.** The three moved tests also covered
   `assets/Cylinder` tile naming (`gfx::parseTilePosition`, INN/OUT, row/col) and
   `assets/Sprite` state mapping (`gfx::spriteNameForState`). Re-establish that coverage once
   the new renderer decides where tile parsing and state→sprite mapping live. The old tests
   are in `old/src/__test__/` to copy from.

## Phase A — the 3D view

The whole phase is "make it look and move like the original". One round per item.

- **A1. Camera and projection.** Read the original's field of view, eye distance from the
  axis, eye height, near/far planes, and how it maps mouse and keys to rotation. Deliverable:
  a camera whose frame matches a captured original frame at a known level, with the
  threshold stated.
  **Complete for the tested scope:** the
  [verified camera checkpoint](docs/scratch/phase-a1-camera.md) retains the operand reads,
  standalone pixel fixture, and individual mutation/restore proofs. Initial, mouse, right and
  left static regions now pass the fixed criterion with the live mesh arrays. The
  entry-point debt remains A4.
  The [active draw-path check](docs/scratch/phase-a1-cylinder-draw.md) now corroborates a
  bounded inner-bank submission. The [cell-fixture check](docs/scratch/phase-a1-cell-oracle.md)
  subsequently reproduced the retained failure; its isolated selection mutations
  were unprotected. The [device-emission check](docs/scratch/phase-a1-cell-emission.md)
  now corroborates the selected quad's ordered coordinates and UVs, while its depth
  representation differs from the candidate's. The subsequent
  [projection/cache replay](docs/scratch/phase-a1-projection-cache.md) reproduces the
  retained matrices and selected depth fields under a diagnostic arithmetic model;
  its new operands have not cleared isolated pixel mutations. The subsequent
  [sourced candidate](docs/scratch/phase-a1-sourced-candidate.md) rendered the captured
  radius-60 shell and per-pass depth: it fails `initial` (0.0249) and `left` (0.0304), and
  its isolated bias mutations are pixel-invisible. It also corrects the mesh-quad decode to
  file offset 16, which means the earlier "passing" cell candidate was the smooth cylinder
  alone. The subsequent [diffuse/decode check](docs/scratch/phase-a1-diffuse-decode.md)
  settles the decode from the object headers (the mesh4 windows are one allocation off) and
  applies the exact per-quad diffuse (`0x46D460`, chop-rounded). The best candidate still
  failed before the live capture. The [live-arena check](docs/scratch/phase-a1-live-arena.md)
  then read the true mesh0 arrays and passed all four views: initial `0.015574`, mouse
  `0.016047`, right `0.014810`, left `0.015663`. The fixed pixel gate is met. The
  [depth/clipping check](docs/scratch/phase-a1-depth-clipping.md) then found no numeric
  near/far planes in the executable, no Z/clip render-state writes, and a normalized 0..1
  viewport; the device is the shipped wrapper and depth ordering is effective in the frames.
  The [control mapping](docs/scratch/phase-a1-control-mapping.md) reproduces 473 live points
  with one 2.9e-4 timing outlier and restores the reference camera block byte-for-byte. The
  [zoom/elevation check](docs/scratch/phase-a1-zoom-elevation.md) then passed canonical,
  zoom 1 and both elevation clamps (0.0116–0.0159); the lens is invariant and the eye-vector
  scaling is verified. **A1 is complete for the tested scope.** Residual unknowns: the
  gameplay `control+0x10` writer is unlocated (behaviour measured), and the wrapper's
  internal Z bits are unreadable. A2 and A3 checkpoints follow below; next is A4. The
  entry-point debt remains A4.
- **A2. The tube.** INN/OUT texture addressing, draw order, inner surface before outer so
  the far side shows through gaps, and whatever scenery the original draws that we skip.
  **Complete for the declared fixed-ROI fixture and scenery inventory:** the
  [A2 facilitator check](docs/scratch/phase-a2-tube.md) corrects the texture-row
  flip (192/192 captured cells; the old formula disagrees on 128), implements the
  source-derived OUT bank selection and center filters, and retains INN-before-OUT
  as READ evidence. All nine retained views and an exposed INN region pass; 54/54
  isolated default restores match byte-for-byte. The upper gold band survives live
  suppression of the whole tube walker, establishing a separate scene contribution.
  **No whole-scene acceptance:** the wider changed-pixel diagnostic still fails
  at `0.022030839 > 0.02`; cavity and upper-rim regions fail too. A4 must carry
  those failures and the omitted scenery/object inventory into integration. The
  entry-point and asset-test debts remain A4. The A3 checkpoint follows.
- **A3. Sprite view selection. Complete for the tested selection scope:** the
  [A3 facilitator check](docs/scratch/phase-a3-sprite-view.md) corrects the old SPR
  axis claim: walk's 12 and fall's 7 LO entries are temporal; their directional
  lists have five images. Block/cartwheel have eight directions; shrug has one.
  The score/index/mirror helpers match 608 native branch cases and 42 live
  actor-bound predictions, including controlled eight-entry graphics and camera
  sector crossings. Both shared-stock-raster candidate crops and restores are
  exact; isolated index and mirror mutations change pixels. The stricter >0.02
  sensitivity diagnostic still fails. Replacement rasterization, sprite placement
  and animation integration remain A4; this does not accept a full scene.
- **A4. Entry point and controls.** A new `src/main.cpp` and renderer, **3D by default**
  with the flat unwrapped view as a debug flag — the inverse of the old arrangement. Revolve
  must be a first-class, discoverable control. Pay debt 1 and debt 2 here.
  **A4a checkpoint:** the [first production frame](docs/scratch/phase-a4-renderer-core.md)
  now exists as a headless `webolution` target; 17/17 tests pass and the retired asset-side
  coverage is restored. It loads a retail level and draws the tube from stored data at the
  verified A1 camera, but **0/9 retained views pass** (best `0.037812306`). The stored
  surface pool matches the live bank, yet the per-cell runtime mesh is not the stored
  `PolyList`/`SurfaceMesh` data (8/450 exact index sets, 0/450 full index+UV). Next is the
  producer of the per-cell quads at `0x4ECB00`; the SDL window/event loop and revolve
  controls follow. See the A2/A3 retained failures.
  **A4b checkpoint:** the [producer check](docs/scratch/phase-a4-cell-mesh-source.md)
  pins the level-load chain (`0x43A41A -> 0x461344 -> 0x463070(0x4ECB10)`;
  `0x43A44E -> 0x4608BB`; `0x460A41 -> 0x462264 -> 0x4615E9`;
  `0x46227E -> 0x463410`; `0x4622ED -> 0x464F9C`) and confirms the topology input
  is the stored 256x96 collision grid via `0x40DE60`. The exact scan in
  `FUN_004615E9` and the part table `FUN_00461E1D` remain undecoded, so no production
  rule landed and the A4a `0/9` result stands. Next: decode those two functions and
  validate an offline generator against the retained arena.
  **A4c checkpoint:** the [cell-mesh rule](docs/scratch/phase-a4-cell-mesh-rule.md) is
  decoded and reproduces the retained arena exactly from stored data (`389/389` mesh0,
  `387/387` mesh4 by indices, UVs, colour and order). `LevelIndex::grid0` is the
  `(x, y, layer) -> pool index` table, not a face list; `FUN_004615E9` is the gap pass.
  `src/assets/CellMesh` and `test_cellmesh` land the rule; 18/18 tests pass. Nine retained
  views still **0/9** (best `initial` `0.037620585`, a small improvement). The remaining
  gap is the draw/texture binding: route mesh0/mesh4 through A2's INN/OUT banks, side and
  center filters, and decide where the constant wall remains. The door path is
  implemented but unexercised; only `MM_CLIMB_002` is validated.
  **A4d checkpoint:** the [route check](docs/scratch/phase-a4-cellmesh-route.md) submits
  the decoded cell mesh through the `0x43A4EC` walker's real order — INN, OUT right
  mesh4, OUT left mesh0, OUT centre — and replaces the constant wall. The `HI` tile
  variant is required (plain 0/9, `HI` 9/9). All nine retained views now pass the
  fixed gate (`initial` 0.015584142, worst `left` 0.015679210); the production frame
  matches A2's passing fixture at 2.6e-5 full-frame. Swapping mesh0/mesh4 or disabling
  INN drops to 6/9. The A2 wider/cavity/rim and A3 sensitivity failures remain; the
  centre filter is not discriminated by this ROI. Next: SDL window/controls, then
  simulation + A3 actor rendering, then the omitted scene.
  **A4e checkpoint:** the [window/control check](docs/scratch/phase-a4-window-controls.md)
  adds the 640x480 SDL window, discoverable grab/revolve/zoom/flip controls and a
  fixed 144 Hz input accumulator. Normal launch defaults to the accepted tube route;
  `--shot` remains headless. The facilitator independently passes 19/19 tests,
  54 retained-stock input comparisons, native window/SPACE/close smoke, and all
  nine unchanged A4d scores/PNG hashes. Fourteen isolated controller/timing/event
  regressions are discriminated after strengthening an initially ineffective
  zoom-exit test; the old default wall independently fails the pixel gate.
  Byte reads narrow the zoom setup to entry only and add its 0.1 ratio deadzone.
  No new stock live measurement or full-scene acceptance. Next bounded proposal:
  one actor billboard placement/raster fixture using A3 selection, before whole
  simulation integration; ownership/dependencies/acceptance are in the linked note.
  **A4f checkpoint:** the [actor billboard check](docs/scratch/phase-a4-actor-billboard.md)
  matches native corners within one float ULP for two mirrored fixtures; seven isolated
  placement/UV/alpha/scale mutations fail the focused test; 20/20 tests and nine
  unchanged tube views. Pixels fail the absent-actor gate: the live actor's vertex RGB
  is 152, not the constructor's 255, and its writer is unfound. Next: find that writer
  (static), then rerun the pixel gate.
  **A4g checkpoint:** the [tint-writer check](docs/scratch/phase-a4-actor-tint-writer.md)
  finds the uniform-RGB setter `0x406260` and a fluid-only fade, but nothing yet that
  gives walk/block 152. A4h: the `0x4682a6` link copies the actor into prototypes
  (clean negative). Static leads are exhausted. A4i/A4j GDB hardware watches named no
  writer and were shown to miss a write (see the linked note).
  **A4k–A4q checkpoint (2026-10-19): actor tint solved, and the A4f pixel gate passes.**
  Frida bisects (A4k–A4o) found the writer: `0x403820`, on the lemming's update, copies
  `palette[map[y·128 + (x & 0xFF)/2]]` into all four vertex records. A4p showed the map
  (12288 B) and palette (256 × 3 f64) are stored LVL data after the collision grid. A4q
  (`src/assets/LightMap.{h,cpp}`, MODULATE in `drawActorSprite`) passes both fixtures:
  0.00323 / 0.00362 against absent-actor 0.00619 / 0.00685.
  **A4r checkpoint (static):** the [render-state check](docs/scratch/phase-a4-actor-render-state.md)
  reads RGB `MODULATE(TEXTURE, DIFFUSE)` on all state-writing material paths in
  `0x47261F`, conditional alpha handling and the blend-selector table at `0x414D80`.
  Device3 names are inferred from paired vtable layout and call shape; caches skip
  assignments. The actor's material flags and blend selector are still unresolved,
  so actor-specific alpha/blend behavior and effective device state remain open.
  No production behavior change; independent operand/SDK check and 20/20 CTest pass.
  Next bounded question: establish those actor flags and selector. Other open items:
  an observed colour change across cells, the `_strcmp` guard at `0x42E622`, the
  sprite-pixel residual, and wiring actors into the app renderer.

## Phase B — the frontend

The original's own screens: backgrounds, buttons, fonts, `.LTX` localised text (the menus are
German), the screen state machine, level select and the map. Preserve the quirks —
**RETURN only when the menu highlight is exactly 0**, and a short ESC that fails to leave
pause. Frontend screen ids are already known (`0x004C416C`: 3 menu, 4 confirm, 5 map).

## Phase C — the in-level HUD

The skill panel art and counts, and the counters the engine names itself: `^saved`, `^out`,
`^time`, the icon sets `GreyIcons` / `BlackIcons` / `AdditiveIcons`, and the cursor set
(`_curLem`, `_curNoLem`, `_curEnt`, `_curExit`, and the left/right variants). Panel indices
0, 1, 3, 4 and 7 are known; 2, 5, 6, 8, 9 are not.

## Phase D — reconnect the simulation

Wire spawn, exits, skill assignment and the measured gadgets into the new renderer, and get a
retail level playable end to end in 3D.

## Phase E — the remaining debts

Short, and explicitly parked rather than cancelled. **The gadget backlog is gone** — rounds
13–31 closed every one, including eleven clean negatives where a cue turned out not to be a
gadget at all. See the gadget table in `docs/LESSONS.md` before assuming anything here is
open.

- The left-facing turn tick (`0x0041C67E`), entangled with the blocker-turn expectation, so
  the two must be fixed together.
- 14 of the 19 call sites of the object-overlap predicate `0x00430BA1` are still
  unidentified. Anti-grav, exit, teleport, SpeedPad and lever account for five.
- Panel indices 2, 5, 6, 8 and 9 (jump table at `0x0045A4DD`). Needed for Phase C.
- Platform targets: Linux, macOS, WebAssembly. The old GLES3/WebGL2 claim was a code
  property that was never actually run.
- Intro, music and FMV. The `.avi` codecs are still unidentified.

## How we work

**One round, one question.** A round that half-answers three things is worth less than one
that closes a single thing. One agent owns the running game at a time.

**Orchestrator–worker across every harness.** The facilitator plans and verifies; a round
agent runs one brief; an optional explore agent maps “where is X?”. Shared queue:
`docs/tasks.json`. Full loop and permissions: `docs/ORCHESTRATION.md`. Model names per
client: `docs/ROLES.md` only — do not pin a vendor string here.

Keep unverified candidate prose in `build/tmp/` so no original-game number reaches `docs/`
before the facilitator completes verification; the facilitator lands accepted scratch notes.
**Verification is not optional and does not delegate.**

The facilitator owns `PLAN.md`, `docs/**` and `.claude/rules/**`. Task agents own their
candidate scratch note outside `docs/`, the code and the tests.

Every brief states: the already-settled facts (so nothing is rediscovered), the round's
question with every lead already in hand, the evidence standard, the mutate-one-thing-alone
test requirement, the environment traps, a must-not-touch list, and "report honestly — do
not round, smooth or over-claim".
