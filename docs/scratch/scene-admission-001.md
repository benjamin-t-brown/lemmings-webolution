# scene-admission-001: structural scenery admission instead of a two-ID allowlist

2026-10-31 (quota-resumed run; original run `20261031T064853-24188-dcc18873`).
Status: **needs_review**; separate read-only acceptance is required. The queue
was not edited. No original executable, copied original instructions, oracle,
hook, debugger, capture, launcher or installation write was used. The reference
EXE (SHA-256 `32675bdb49866e76502713251fdfd2b09078c2481c72ca22ff68ebf4b3737a2f`)
is only checked by reading bytes. C3 stays FAIL; this is source-built rendering
support, not a native-fidelity claim.

## Runtime change

Milestone step: load an existing installed level and render its scenery during
play. Route: ordinary startup or direct `--level` -> `runApplicationLeg` ->
`AssetLibrary` reads `COMPILED\<ID>.LVL` -> `BackdropScene::build` -> mesh and
billboard traversal -> scheduled Primary, LegacyUnattributed, tube and Secondary
rendering.

* **Before (source-traced and measured on the frozen build):** `BackdropScene::build`
  returned a successful empty partial scene ("scenery adapter not demonstrated") for
  100 of the 102 effective installed levels, solely because of the
  `MM_CLIMB_002`/`MM_BASH_002` ID test at the old `BackdropScene.cpp:49`.
* **After (measured on the current build):** `build` attempts the existing structural
  extraction for every level ID. 10 levels pass and render (8 newly). 92 hit a
  form the production readers/renderer do not represent and still return the
  successful empty partial scene with a level-, role- and form-qualified
  diagnostic. No level is malformed (0 hard errors, 0 exact-end mismatches).

Admission is a portable renderer-capability gate. The original admits every level
(retained chain `0x004432C1 -> 0x00439FD3 -> 0x00437D48 -> 0x004749BF -> 0x0046BCA4`,
typed readers `0x00425E50`/`0x00426C6B`); that chain is retained static evidence, not a new
measurement, and nothing here is recovered native admission.

## Production change (`src/app/BackdropScene.cpp`, `.h` API unchanged)

* The ID allowlist is removed. No per-level table and no marker scan.
* `add` / `models` / `extract` return `std::expected<void,Failure>`, where
  `Failure{unsupported,message}`. A failure is "unsupported" only when the reader or
  adapter message contains `unsupported` (SceneNode type-2 branch 1, the nested/non-leaf
  Secondary dispatch check, unsupported model type / missing inline object).
  Everything else (truncation, Object 09 marker mismatch, tree-end mismatch,
  bad references) stays a hard `std::unexpected` exactly as before.
* On an unsupported form, `roots_` is cleared (never keep a supported subset, which would
  draw some roles and omit others) and one diagnostic is stored:
  `<ID> <Stage> <owner>[i] source=0x...: <reason>; scenery not admitted: empty partial scene`.
* Every existing check is unchanged: marker, `readSceneNode` exact `tree_end`, the Secondary
  leaf-domain check, `buildSceneMeshSource` validation, model `type==0` + inline object.
  Stage order, transforms, predicates, mixed secondary order, raster, SceneFrame,
  camera, simulation, audio and saves are untouched. No branch1 billboard, nested
  secondary dispatch or other new form was added.
* `main.cpp`'s other `MM_CLIMB_002` conditions (cell route) were left unchanged. They did not block the
  demonstrated route (`runApplication --level MM_W_012` reached the changed renderer).

## Step 1 census (recorded before the production edit; `build/scene-admission-001/census.txt`)

Driver: `test_sceneadmission_census` (CTest `sceneadmission_census`, 0.78 s), using the
production `AssetLibrary` overlay, `read_compiled_level`, `readSceneNode` with the exact
tree-end check, `buildSceneMeshSource`, the Secondary leaf-domain check and the model
type-0 requirement. 102 effective levels:

| outcome | levels | exact reason |
|---|---|---|
| ok | 10 | all roots parse with exact ends, adapters pass, 0 missing texture names |
| unsupported | 74 | Secondary `unsupported nonzero type-2 branch 1 at 0x...` (first failing role Secondary paired-secondary[0]) |
| unsupported | 18 | Secondary `unsupported nested/non-leaf secondary dispatch` |
| malformed | 0 | - |

Neither stop condition applied: 8 levels beyond the two admitted pass, and there is no
exact-end mismatch. Every failure is in the Secondary role; Primary and all model roots
pass in every level.

Passing: `E06_KF_1, H17_MG_1, M06_KF_1, M08_KF_1, MM_BASH_002, MM_CLIMB_002, MM_E1_3_SKILLS,
MM_E1_4SKILLS, MM_W_012, VH22_JC`.

Nested/non-leaf (18): `E11_KF_1, E22_KF_1, H05_KF_1, H09_KF_1, H20_KF_1, H24_KF_1, M04_KF_1,
M11_KF_1, M16_MG_1, M21_KF_1, MM_BLOCK_BOMB_A, MM_FLOAT_CLIMB2, MM_H_003, VH04_JC1, VH05_JC1,
VH18_JC, VH19_JC, VH21_JC`.

Branch1 (74): all remaining levels, including `MM_DIG_000` (Secondary type-2 branch 1 at
0xa7572). The full per-level list with offsets is in `census.txt`.

Scenery tree identity (FNV of the tree bytes): the Secondary tree is byte-identical to
`MM_CLIMB_002`'s in all 10 passing levels. The Primary tree is identical in 8
(`H17_MG_1, M06_KF_1, M08_KF_1, MM_BASH_002, MM_CLIMB_002, MM_E1_3_SKILLS, MM_E1_4SKILLS,
VH22_JC`). Only `MM_W_012` (Primary `0xba4017a15be0763e`) and `E06_KF_1` (Primary
`0xd25740276befc295`) have a different Primary tree. So there are **three distinct supported
scenery groups**: the 8-member group, `MM_W_012`, and `E06_KF_1`. The newly admitted levels
differ mostly in their model roots (LegacyUnattributed), not in scenery.

## Before/after demonstration (`sceneadmission_before` 3.94 s, `sceneadmission_after` 19.44 s)

Two drivers are built from the same source: frozen (authentic pre-task
`BackdropScene.cpp/.h` copied to ignored `build/scene-admission-001/frozen`, identity checked by
`sceneadmission_identity` against the supervisor entry manifest; no production file swapped) and
current. Fixed camera: `after/fixed-camera.json` (640x480, yaw units 32, zoom shift 0).

* Frozen: `admittedCount=2 emptyCount=100 hardErrors=0 buildAll=0.73 s`.
  Current: `admittedCount=10 emptyCount=92 hardErrors=0 buildAll=1.32 s`
  (per-level build 4.3-10.3 ms; admitted levels about 4.3-6.2 ms).
* `MM_CLIMB_002` and `MM_BASH_002`: complete frozen vs current scene-only, composed scene-on
  and scene-off RGB and depth byte-equal (`completeFrozenCurrentRgbDepthEqual` for both).
  Preserved: Primary 923/0, LegacyUnattributed 172/0, Secondary 82/17 candidates, 1177 records,
  17 billboards. Scene-only hashes are `0xe68004ec4259751c` (MM_CLIMB_002) and
  `0x775f710cc70b57b5` (MM_BASH_002); the primary hash `0x1fd0bfb82ab39e95` is asserted in `sceneintegration` and `sceneadmission`.
* Newly admitted levels, per stage, from the scene-only render (full numbers in
  `after/measurements.txt`):

| level | Primary raster calls / accepted color+depth stores | Legacy calls / stores | Secondary calls / stores | culls (Primary+Legacy) | missing tex | rejected | scene-only nonblack px | scene on/off changed px |
|---|---|---|---|---|---|---|---|---|
| E06_KF_1 | 1196 / 354052 | 334 / 4870 | 170 / 56517 | 157+2 | 0 | 36+0+28 (projection) | 278820 | 134200 |
| MM_W_012 | 1200 / 363969 | 334 / 4853 | 170 / 55216 | 153+2 | 0 | 36+0+28 | 285878 | 133123 |
| H17_MG_1 | 1196 / 357059 | 378 / 8599 | 170 / 52708 | 157+3 | 0 | 36+0+28 | 278820 | 140621 |
| M06_KF_1 | 1196 / 357059 | 667 / 19536 | 170 / 49155 | 157+5 | 0 | 36+0+28 | 279364 | 144559 |
| M08_KF_1 | 1196 / 357059 | 667 / 10394 | 170 / 52486 | 157+5 | 0 | 36+0+28 | 278820 | 137840 |
| MM_E1_3_SKILLS | 1196 / 357059 | 335 / 5118 | 170 / 53105 | 157+1 | 0 | 36+0+28 | 278820 | 135554 |
| MM_E1_4SKILLS | 1196 / 357059 | 334 / 11277 | 170 / 53105 | 157+2 | 0 | 36+0+28 | 278820 | 128174 |
| VH22_JC | 1196 / 357059 | 333 / 9763 | 170 / 53105 | 157+3 | 0 | 36+0+28 | 278820 | 137357 |

  Every run is rendered twice and is deterministic (`deterministic=1`); raster rejections are 0 and
  all rejections are projection rejections. Scenery contributions (Primary+Secondary raster calls,
  e.g. 1366 for E06_KF_1) are reported separately from model contributions (LegacyUnattributed
  calls, e.g. 334). Entrance/Exit stays LegacyUnattributed. Observed, not
  explained here: Secondary accepted stores are lower in the composed (tube) run than in the
  scene-only run for every level.
  Artifacts: PNGs for scene-only, scene-on and scene-off for all ten levels under
  `build/scene-admission-001/after`, RGB/depth dumps for the two accepted levels.
* Public route: `runApplication --level MM_W_012 --sim scenario` under SDL dummy drivers
  reaches the changed renderer for one frame and exits through the observer-queued
  `SDL_QUIT` (`return=0, frames=1, queuedQuit=1`), scene-on versus scene-off differ by 131068 px.
  No timeout was raised.
* Time: full admitted-set render (10 levels, each 3 modes) took 15.8 s inside the 120 s test
  timeout; frozen took 3.1 s for its 2 levels.

## Superseded assertions (exact)

1. `test_backdropscene.cpp` (about line 29): the check that built `MM_CLIMB_002`'s bytes under the ID
   `MM_DIG_000` and expected an empty scene (an allowlist artifact). Replaced by (a) the same
   bytes under that ID are admitted with no diagnostics, and (b) real `MM_DIG_000.LVL` yields a
   successful empty scene with exactly one diagnostic naming `MM_DIG_000`, `Secondary` and
   `unsupported nonzero type-2 branch`.
2. `test_sceneintegration.cpp` (about line 234): "undemonstrated level explicitly partial" is kept, and an added
   check requires the diagnostic to name `MM_DIG_000`, `Secondary` and
   `unsupported nonzero type-2 branch` and not to contain `scenery adapter not demonstrated`.
3. Diagnostic wording `<ID>: scenery adapter not demonstrated; empty partial scene` is replaced by
   `<ID> <Stage> <owner>[i] source=0x...: <reason>; scenery not admitted: empty partial scene`.

All other backdropscene, sceneintegration, cameraentry, levelreturn and frontend
assertions and timeouts are unchanged. Malformed/truncated data stays a hard error for
`MM_BASH_002` and `MM_DIG_000`.

## Verification

`python tools/agent_control/verify.py --profile all` (registered MinGW-w64 GCC): **main 92/92 and
recovery 84/84 passed** in this run (main 495 s, recovery 30 s). New registered checks:
`sceneadmission_identity`, `sceneadmission_census`, `sceneadmission_before`,
`sceneadmission_after`. Reference identity verified by reading bytes only; recovery keeps an
empty `LEMMINGS_REFERENCE_EXE` and `native_int_array` as its only `native_`-prefixed source test.
This is source verification. The supervisor still has to run its own independent verification.

## Remaining limitations (unsupported-form inventory)

* 74 levels need Secondary type-2 branch 1 billboards (not implemented).
* 18 levels need nested/non-leaf Secondary dispatch (not implemented).
* These levels play with the successful empty partial scene and a diagnostic, as before.
* Not claimed: native admission, x87 equivalence, clipping, per-frame state or full-frame
  fidelity. Fixed-camera scene-only renders are not retail frames. Scenery for the 8
  new levels reuses (nearly) the same trees as `MM_CLIMB_002`, so the rendering breadth gain
  is mostly the Entrance/Exit and model roots plus two new Primary variants (`MM_W_012`, `E06_KF_1`).
* Unknown: whether the main.cpp cell route and camera-entry paths that key on `MM_CLIMB_002`
  or `MM_BASH_002` should be generalized for these levels; step 1 did not show they block the
  demonstrated route.

## Next dependency

Branch1 billboards would unlock 74 levels, and nested secondary dispatch another 18, and both
gate far more levels than anything else. Each needs its own bounded static-evidence task
(retained node form, address-backed) before production work. Armageddon/fast-forward remains
unchosen because its semantics are unestablished and it touches parked timing work.
