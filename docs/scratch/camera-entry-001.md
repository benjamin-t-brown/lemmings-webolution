# camera-entry-001: MM_BASH_002 entry from the serialized selected hatch

2026-10-31 (resumed run). Status: **needs_review**; separate read-only
acceptance is required. The queue was not edited. No original executable,
copied original instructions, oracle, hook, debugger, capture, launcher or
installation write was used. The installed save was never read or written
(tests use a temporary clone save only).

## Runtime change

Milestone step: select an installed level -> enter play with its selected-hatch
camera orientation. Concrete route: ordinary startup -> persistent
`FrontendSession` -> map node for `MM_BASH_002` -> `runApplicationLeg` ->
`COMPILED\MM_BASH_002.LVL` -> `CameraController` / `CameraIntro` -> production
scene rendering.

* **Before (observed, frozen build):** the frontend-selected level rendered from
  canonical `CameraParams::yawUnits = 32` and `CameraIntro` was armed only for
  `MM_CLIMB_002`, so there was no opening move.
* **After (observed, current build):** the same route seeds yaw229 from the level's
  serialized selected hatch, resets zoom to 0, arms the existing intro (subject to
  the session's intro preference), keeps yaw229 on Retry without replaying the
  intro, and re-arms the intro on map re-entry.

## Static provenance (reference EXE SHA-256 `32675bdb...37a2f`, read as bytes only)

Retained chain, not re-derived: `0x004432C1` -> hatch list `0x004487F0`; selector
store `0x00443BDC` into G+0x5568; hatch loader `0x00425E50` -> base reader
`0x00423291` (base triple into hatch+8); entry initializer `0x00444D1E` (default
yaw128 at `0x00444FA2/0x00444FA9`, replaced by selected hatch+8 when the signed
selector is below the list count; controller vectors `0x00452FB7/0x00452FD3`; zoom
reset `0x004450EB`); successful entry `0x0045C9AB` copies the intro preference and
zeroes its clock. The historical model callback and native post-load behaviour are
**not** newly verified; nothing here contradicts a retained instruction.

Installed `MM_BASH_002.LVL` (709,367 bytes, SHA-256
`65455693fa887f4474c66c2795526cb3b2bef2616449a5e1dc55debb9d7e5cd2`), reproduced by
the registered `levelobjects` test: selector word at LVL `0xA347B` is 0, hatch base
triple at `0x9F473` is (229, 75, 0), `read_compiled_level` yields one hatch and
yaw229. `MM_CLIMB_002` (699,117 bytes) reproduces selector 0 / yaw32. A corpus
census (reported, not asserted) reads all 102 installed levels with 0 rejections,
102 selectors in range, 0 defaulted to yaw128, and **6 nonzero selectors**.

Original ABI versus portable structures: `SelectedHatchCamera` is a portable
projection (selector, list count, yaw). It is not the original hatch object or
controller layout.

## Changed files

| File | Change |
| --- | --- |
| `src/assets/LevelObjects.h/.cpp` | `SelectedHatchCamera` + `readSelectedHatchCamera`: unchanged `read_compiled_level(..., require_eof=false)`; `bit_cast<int32_t>(gadgets.value_5568)`; in-range selector -> `bit_cast<double>(entities_425e50[i].double_bits[0])` with no arithmetic; nonnegative out-of-range or empty list -> 128.0; negative selector and nonfinite selected yaw -> portable-safeguard errors; only the reader's `invalid_argument` is translated to a `selected-hatch camera: ...` expected error. Caller storage stays borrowed for the whole parse. Existing hatch/spawn projections untouched. |
| `src/app/main.cpp` | For `args.frontend && args.camera.empty()`, after the loaded terrain-height initialization and before the first render and `CameraController` construction: read the metadata (a failure prints `level <id>: selected-hatch camera: ...` and returns 1), set `params.yawUnits` and `zoomShift = 0`. `CameraIntro` admission drops the `MM_CLIMB_002` condition; frontend entry, explicit-camera exclusion and the session intro preference remain. Plus the read-only seam below and an explicit `LevelObjects.h` include. |
| `src/app/AppEntry.h` | Read-only `LevelFrameView::displayedPose` (the `CameraParams` used for the frame, before the intro clock advances) and `cameraPickReference` (controller seed). Appended after existing members; no behaviour. |
| `src/__test__/test_levelobjects.cpp` | Metadata tests (below). |
| `src/__test__/test_cameraentry.cpp` (new) | Frozen/current demonstration driver. |
| `CMakeLists.txt` | `cameraentry_before` / `cameraentry_after` (120 s failure guards, dummy SDL drivers). |

`CameraController`, `CameraIntro`, `CameraInput`, `Camera.cpp`, the recovered
readers, the renderer, scenery admission, simulation, save, audio and archive
precedence are unchanged. `CameraParams` diagnostic defaults are unchanged.

Call path: `runApplication -> FrontendSession map click -> runApplicationLeg`
(`args.level` from the map, `args.sim = true`) `-> readSelectedHatchCamera(*blob)
-> params.yawUnits / zoomShift -> CameraIntro(armed) -> CameraController(params,
terrainHeight) -> controls.camera() -> cameraIntro.pose() -> renderFrame`. Retry
re-enters `runApplicationLeg`, which re-reads the selected yaw while
`FrontendSession::levelIntroEnabled()` is false; map activation sets it true again.
The direct `--level/--sim` and explicit `--camera` routes never take the new branch.

## Frozen identities and the interrupted run

Frozen copies under ignored `build/camera-entry-001/frozen/` match the original
interrupted run's entry manifest
`build/agent-runs/20261031T042327-46024-86f1ecf1/before-source-hashes.json` (and
`entry-identities.json`):

| Source | SHA-256 |
| --- | --- |
| `main.cpp` | `774d67572bb361879dc806cdce8f5dc2e5e3309f013d8ca328e61cf72a4b6a26` |
| `AppEntry.h` | `1aa581b733905961ac0752994d17b9a34c88f395e080fa03b27babf8438d2091` |
| `LevelObjects.cpp` | `a238e615e3007100947b0c6be4fdd2d514eb11edcc1df3f39d5335dce14e787f` |
| `LevelObjects.h` | `4ddd5d3b97c0c5f17cb209d2b12500e1b04b24cfecc543ce925999461351ddfa` |

The **resumed** run's entry manifest
(`build/agent-runs/20261031T043027-22828-21b1b8ea/before-source-hashes.json`) differs
for those four files (main `641b3660...`, AppEntry `01ae9fb1...`, LevelObjects.cpp
`9854556c...`, LevelObjects.h `98f45bb2...`) because it already contains the
partial implementation. It is retained as evidence of the resume point and was
**not** used as the baseline; the frozen files and `supervisor-before-source-hashes.json`
were not overwritten. HEAD was not substituted for the dirty working state.

## Executed before/after demonstration

One driver, `src/__test__/test_cameraentry.cpp`; the frozen build compiles
`build/camera-entry-001/frozen/main.cpp` with the frozen `AppEntry.h` (include
path first), the current build compiles `src/app/main.cpp`. Production files are
never swapped. Both runs use public `runApplication`, dummy SDL video/audio, a
0.25 s deterministic frame clock (`--frames 40` is the application's precondition;
each run ends through observer-queued `SDL_QUIT` long before the cap).

Route: no-save ordinary frontend -> Play Current Game click -> map click on the
actual `MM_BASH_002` node (node 6) -> level (frames 0..6 sampled, Escape, Retry) ->
frame 0 of Retry, Escape, Quit to map -> map click -> frame 0 -> `SDL_QUIT`.
**SOURCE-TEST setup (not natural gameplay, no new unlock policy):** node 6 is not
selectable on a fresh map, so the test writes node 2 (group 0, slot 2) as complete
into a temporary clone `SavedGame.dat` through `SavedGame::markComplete`
and the accepted node mapping; the unchanged `MapScene` rules then make node 6
selectable. Production saves and map availability are untouched.

Expectations are independent static-camera equivalents: the same application is
run with an explicit `--camera` file (which bypasses entry metadata and the intro)
whose operands the test calculates from the retained intro equations (350-unit
ease-out pull-back over 2 s; yaw swing after 1 s) without calling `CameraIntro`.
The controller wraps `yaw_units` into [0,256), so the intro's wrapped phase is
carried by `yaw_offset` with `yaw_units` 0 (bit-identical turns). Operands:

| Frame | Intro clock | yaw_offset | eye_distance |
| --- | --- | --- | --- |
| 0 | 0.00 | 101 | 727.516 |
| 3 | 0.75 | 101 | 514.23475 |
| 6 | 1.50 | 133 | 399.391 |

Results (full 640x480 composed frames, so HUD/panel/cursor included; the entry cue is
not yet visible through frame 6; sim ticks 1/5/8 agree across runs):

| Build | frame | vs entry-yaw229 static | vs canonical-yaw32 static | intro active |
| --- | --- | --- | --- | --- |
| frozen | 0 / 3 / 6 | 304,947 / 304,868 / 304,814 changed px | **0** / **0** / **0** | false |
| current | 0 / 3 / 6 | **0** / **0** / **0** | 304,947 / 304,868 / 304,814 changed px | true |

Current-only read-only pose seam (frames 0..6): `introActive`, `introSeconds` =
0.25 x frame exactly, yaw 229 bit pattern, zoom 0, `levelEyeHeight` = 96 (terrain
rows), `eyeDistance` 377.516, pick reference `{229, 48, 0.5}`. Retry frame 0:
intro inactive, clock 0, yaw 229. Re-entry frame 0: intro active, clock 0, yaw 229.
Frozen: neither Retry nor re-entry has an intro. Explicit camera files keep their
operands and never arm the intro (current seam); direct `--level MM_BASH_002 --sim`
keeps yaw32 / no intro in both builds. Callbacks per run: 3 frontend, 11 level
(8 + 2 + 1), 3 legs, 2 map selections, 1 queued quit, exit 0; static runs 7/1/4/7
callbacks, exit 0. Artifacts and commands under
`build/camera-entry-001/{before,after}/` (`measurements.txt`, camera JSON,
`frontend-frame{0,3,6}.png`, `static-*.png`).

## Metadata tests (registered `levelobjects`, 2,216 checks, 0 failures)

Synthetic Level 23 fixtures: two hatches whose base yaws (17.5, 229) differ from
their spawn words and model names, the first carrying a populated counted triple
array; selector 1 selects the second hatch; selector equal to the count, INT32_MAX,
and an empty list yield 128; selectors -1 and INT32_MIN are rejected; -300.5 and
1e300 are kept bit-exact; NaN and +/-inf are rejected only when selected;
truncation at 0, 4 and half the bytes and non-level input become contextual
errors; input bytes are unchanged; 64 trailing bytes are accepted. Installed
extraction reproduces the `MM_CLIMB_002` and `MM_BASH_002` facts above.

## Verification

`python tools/agent_control/verify.py --profile all` (registered MinGW-w64 GCC
`C:/progs/msys2/ucrt64/bin/g++.exe`): **exit 0** on the final sources. Main
**82/82** (446 s), recovery **84/84** (29 s), zero failures/disabled/skipped in
both JUnit files. All 80 existing main and 84 recovery registrations are preserved;
the only additions are `cameraentry_before` and `cameraentry_after` (14 s / 15 s).
Reference identity was checked by reading bytes only; recovery keeps its empty
`LEMMINGS_REFERENCE_EXE` and `native_int_array` as its sole native-prefixed source
test. An earlier full run failed only the two new tests (they omitted `--frames`,
the application's precondition for the test frame clock); the driver was corrected
and nothing else changed. This resumed session edited no existing assertion; no
existing expectation was weakened or superseded (`test_frontendentry.cpp` and other
pre-existing test diffs against HEAD predate this task or belong to earlier tasks,
and were not touched here): first-level camera/intro, pick, explicit-camera, direct-render, Retry and
re-entry assertions all pass unchanged. The existing non-first-level Retry test
(`levelmenuwindow`) never asserted the old no-intro state.

## Limitations, discrepancies, unknowns

* Serialized camera initialization and source-rendered poses only. Original runtime
  callbacks, the historical model callback, x87 equivalence, animation timing and
  full-frame fidelity are **not** verified. Natural complete `MM_BASH_002` play,
  finish and restart remain unverified (nothing was injected into outcomes).
* The Retry frame is checked through the pose seam, not by image: its entry cue
  already covers the frame, so a static-camera image would not be independent.
* Under the intro the yaw phase is wrapped at each frame (101/256 at frame 0), but
  after the intro the unwrapped phase (357/256) feeds `selectorTurns`. The tube
  pass-bound code wraps by 32, so the intended view is the same, but this
  post-intro handover was not separately demonstrated.
* `MM_BASH_002` is hatch 0 / selector 0; a level that selects a non-first hatch
  exists in the corpus (6 nonzero selectors) but no installed one was rendered. Only
  the synthetic fixture covers selection of a non-first hatch.
* The negative-selector and nonfinite-yaw rejections are portable safeguards, not
  original validation. Both would stop level entry with a level-qualified error.

## Next dependency

Independent read-only acceptance. After it, serialized teleport pairing is next only
once a concrete installed-level mismatch or affected route is established.
Evidence-preferred follow-up inside this milestone: render one of the six installed
levels with a nonzero selector through the same route to exercise non-first-hatch
selection on real data. D1/D3/digger, clipping, controller and capture parks,
native-006 deferral, oracle-001, STATUS gates and recovery historical claims remain
untouched.
