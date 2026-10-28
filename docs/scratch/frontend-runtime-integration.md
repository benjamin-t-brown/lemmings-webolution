# Frontend runtime integration: New Game to a retail level

**Historical implementation snapshot.** Later [main-menu row1 evidence](frontend-return-negative-prediction-2026-10-28.md)
supersedes the blanket nonzero-Return claim below. [Confirmation focus observation](frontend-confirmation-focus-2026-10-28.md)
now establishes the bounded DOWN/UP/DOWN and Return-on-No route; the initial
fixed-Yes adapter and unverified-keyboard statements below describe this earlier
stage, not the latest native findings. See [STATUS](../STATUS.md) for current scope.

User approved the next frontend integration step after runtime-panel delivery.
This implementation is opt-in with **`webolution --frontend`**. Ordinary direct
level, `--map` fixture, and headless commands retain their prior entry routes.
No original-game launch, attach, desktop input or installed-save write occurred.

## What now works

- The production menu uses the existing qualified main-menu renderer and UK
  strings. New Game opens the actual **Continue? / Yes / No** intermediate screen.
- Return at the initial menu and Yes choice traverses the retained sequence
  **Menu3 → Confirm4 → Map5**. Clicking No reconstructs the menu. Other menu
  actions and keyboard traversal remain unsupported; the main-menu Return-at-
  nonzero-highlight quirk remains in the model and its tests.
- Map clicks use the indexed `LEVEL MASK.BMP` plane, not sprite boxes or RGB.
  `(206,204)` resolves node0; only effective states2/3 are eligible. Its native
  mapping `(group0,slot0)` resolves through `LEVELS.MAP` to `MM_CLIMB_002`.
- The selected ID is passed into the existing level-loader/render/simulation
  path in `main.cpp`, with simulation enabled and a fresh tick0 world. This does
  not repair or reopen parked Phase D timing. Unpaused in-level ESC still exits;
  level-to-map is not implemented.
- The frontend SDL window is released before the level window is created.
  `--frames N --window-shot FILE` captures the frontend CPU frame if its frame
  cap is reached there, or the level's final CPU frame after selection. Selection
  deliberately does not write an intermediate screenshot over that output.

New Game uses **fresh in-memory progress**, never native `SavedGame.dat`. Native
Yes resets/saves progress; we do not emulate that by modifying the installation.
`--map-flags` remains a separate fixture option and cannot accompany `--frontend`.
SDL Return/left-click and half-open choice-panel hit regions are an explicit
SOURCE adapter, not a new physical-input measurement. The screen's initial Yes
focus is fixed; clicking No activates its recovered semantic route directly.

## Newly recovered confirmation screen (READ/DERIVED)

The former generic `Confirm` model label is now identified as new-game
confirmation, vtable **0x4B3F38**, constructor **0x49BB30**, init **0x49BB6B**.
Static packet: `build/tmp/enacter-frontend-confirm/`, including full REA evidence,
28 raw ranges, recipe, and explicit unknowns. Own static session was closed.

- Factory selector0x24 uses `[0x4C08F0] → 0x4C0904`,
  `textures\small_back.bmp`; surface construction specifies640×480.
- `0x49BB74` sets layout base200; table `0x4C4514` holds UK string indices21/22
  (**Yes/No**). Shared row builder adds100, centres at320, steps font height29,
  and creates/focuses the choice panels. Choice tops are300/329.
- `0x49BBD0..0x49BC0A` positions a separate UK[129] **Continue?** heading at
  `(320,250)`, RGB255/0/0, flags0x840. It is not given a panel. Shared glyph-table
  widths yield left272 and width97. The existing alpha glyph blitter is reused.
- Semantic handler **0x49BC59** branches Yes→selector0x23 (map), No→selector7
  (rebuilt menu). Native Yes also calls the progress reset/save functions.
  RETURN-at-No, physical key traversal and exact shared chrome remain unverified.

Main-agent verification independently parsed raw-backed PE section ranges,
authenticated the stock SHA256, checked all28 retained ranges and the key
resource/table/position/colour operands. Script/result:
`build/tmp/enacter-frontend/verify_native.py` / `native-verification.json`.
No decompiler output was copied into production. This is not a new native
screenshot comparison: background composition, shared panels/blending, cursor
and footer/chrome still need full-frame validation.

## The retained C→G input-context storage gap

The specific missing storage link in [B3k](phase-b3k-native-binding-partial.md)
is now read, with its runtime qualifications retained:

1. Startup **0x4507B0** puts `0x4C6A08` in ECX and **0x4507B5** calls
   `0x45F1AD` on the normal initialization arm; **0x4507AA** can skip that arm.
2. **0x45F1C8** saves this to local-0x20. With a non-null allocation,
   **0x45F31A** calls the already-read constructor `0x482FBA` with the allocated C.
3. The return passes through locals-0x24/-0x18; **0x45F338..0x45F33E** stores it
   at `this+0x55B8`. **0x4C6A08+0x55B8 = 0x4CBFC0**, exactly G, not a new BSS
   value read from the file. Allocation failure stores null.
4. The previously read startup load **0x450873** passes G to `0x486A5E`;
   **0x486A64** stores it in shared context global `0x5D59DC`.

Replay: `python tools/harness/read_frontend_input_context.py --self-test`.
The checker authenticates the exe, rejects non-raw-backed ranges, checks the
instruction chain/address arithmetic, and rejects two isolated in-memory byte
mutations. It never executes or changes the stock executable. Full init-body
disassembly and narrower windows are retained in `build/tmp/enacter-frontend/`.
An initial locator at0x45F2E0 started inside an instruction and was discarded;
the complete read uses prologue0x45F1AD. No claim relies on that locator.

This closes the **static storage link**, not proof of successful API/device
initialization, absence of later pointer replacement, loaded shim identity, or
physical left-button equivalence. B3k's wildcard/first-available-button and
shim-injection caveats remain; no launcher-safety component was changed.

## Offline checks and limits

All commands use the documented UCRT64 wrapper. SDL tests explicitly use the
dummy video driver and push events only into their own process; no desktop
focus/input or stock start is involved.

Restored full build/CTest: **54/54 PASS**, 77.87s, including the retail-backed
frontend and dummy-window tests (`build/tmp/enacter-frontend/final-tests.log`).

- `mapscene`: four prior SOURCE image hashes unchanged; exhaustive pixel picking
  across four flag fixtures, locked-node rejection, node0 mapping and the
  **221 mask pixels / 68 bounding-box holes** in its17×17 box.
- `frontendsession`: production menu composition; confirmation backdrop,
  exact selected/unselected panel pixels and heading position/tint; Yes/No,
  screen counters, background rejection, selected retail ID, real loader and
  World spawning from that setup. This last check is own-sim wiring only.
- `frontendwindow`: actual production SDL runner driven by synthetic events:
  Return sequence, repeat suppression, map entry, New Game/No clicks, focus
  loss/gain, close and frame cap. It returns the expected retail ID and Level5.
- `frontend_menu`: real `webolution --frontend` headless and dummy-window
  screenshots equal `MenuRenderDump` byte-for-byte. Rendering primitives are
  shared; this is integration agreement, not a new independent native oracle.
- Fresh C1 zoom1/elev_down renders are byte-identical to the preceding runtime-
  panel check (0.0160863988750699 / 0.016121214378294987, both passing). Outputs
  and `c1-regression.log` use this pass's separate scratch directory; prior
  screenshots/evidence were not overwritten.
- Five isolated source mutations were compiled and rejected: unflipped mask
  rows; accepting locked nodes; returning the wrong level ID; black instead of
  red confirmation heading; No incorrectly routing to the map. All four mutated
  source files were restored byte-exactly against pre-mutation SHA256 records.
  Logs: `build/tmp/enacter-frontend/mutation-*.log`.

The initial pass exercised the SDL runner and selected-ID loader separately.
The follow-up below closes that own-app test gap, but not native fidelity.
Native B2/B3 gates, map title/hover/cursor, other menu actions, save adaptation
and return-to-map remain unaccepted. No change to existing fidelity gates or parks.

## Follow-up: actual application handoff

`AppEntry.h` exposes the existing application body as `runApplication`; the CLI
is a thin call to it. The optional frontend observer is empty in normal runs.
`test_frontendentry` compiles that same production path and queues events only
into its own SDL dummy-driver process. It refuses to run with a desktop driver.

The test traverses Menu3 → Confirm4 → Map5, clicks node0, then continues through
the **actual application branch into level loading, the simulation clock, the
level window loop, and actor/panel/HUD composition**. Its final 30-frame image
equals the direct headless `MM_CLIMB_002` one-tick composition byte-for-byte. A
tick0 control differs, so the clock assertion is not an identical-image no-op.
This is still our simulation's clock, not validation of native timing.

The caller deliberately requests `MM_DIG_000` initially, so the test cannot pass
merely by leaving the app at its usual default level. One entry starts without
`--sim`, checking that frontend entry enables gameplay; a second supplies20
diagnostic starting ticks, checking that New Game resets them before advancing.
Three isolated production mutations—ignoring the selected level, not enabling
simulation, and keeping the diagnostic ticks—each fail this end-to-end test.
`main.cpp` was restored against its pre-mutation SHA256.

Restored full build/CTest: **55/55 PASS**, 95.33s; the end-to-end test took15.84s
(`build/tmp/enacter-frontend-entry/final-tests.log`).

Logs: `build/tmp/enacter-frontend-entry/`. Images are retained in fresh exclusive
`build/cmake/dev-debug/frontend-entry-test/run-*` directories on every execution;
tests do not overwrite previous outputs. No original-game process, installed
save, launcher/controller, physical input or visible desktop window is involved.

A bounded evidence search found no qualified retained **native** map/first-entry
frame pair with recorded unlock state and valid640×480 geometry. B3i/B3j images
are SOURCE renders, and the B2m geometry failure is not a replacement reference.
Further visual acceptance needs a new, safely arranged native comparison, not
more agreement between our own rendering paths.

**Follow-up:** the [2026-10-28 native comparison](frontend-native-comparison-2026-10-28.md)
now supplies menu/confirmation/one fresh-map-state images and observed first-level
entry from one owned process. All three frontend frame errors are below0.02;
missing map footer/cursor and the unmatched starting level view remain explicit.
Save/config were restored byte-exactly. This supersedes the missing-reference
statement above only for the newly captured states, not the remaining gates.

The subsequent map-footer correction adds the actual Back to Main Menu row and
its statically recovered semantic route (click/Return are SOURCE adapters).
The same native fresh-map frame error improves from0.0165464 to0.0082174.
This map→menu route is distinct from the still-unimplemented level→map route.

The [first-level opening camera move](level-entry-camera-intro.md) is now wired
on natural frontend entry into `MM_CLIMB_002`; direct-level fixtures remain
static. The end-to-end test's expected frame now uses the independent static-
pose equivalent at29/144seconds, not the old settled view. Native intro-clock
matching and general selected-hatch camera metadata parsing remain open.
Latest restored suite after that change: **56/56 PASS**,98.28seconds; C1 fixture
images remain byte-identical.
