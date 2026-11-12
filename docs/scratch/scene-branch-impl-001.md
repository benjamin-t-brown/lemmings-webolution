# scene-branch-impl-001: MM_DIG_000 type-2 branch-1 sprite scenery through the Secondary stage

2026-10-31. **Implementation task; finishes `needs_review`.** Source-built rendering support only.
It claims no original admission, no pixel agreement for flame phases (the original seed is wall-clock
derived and not reproducible), no x87 equivalence, no clipping or full-frame fidelity and no digger
gameplay. C3 stays FAIL. No original executable, copied original instruction, oracle, hook, debugger,
capture, launcher, installation or save write was used; the reference EXE was only read as bytes by
the verifier (SHA-256 `32675bdb49866e76502713251fdfd2b09078c2481c72ca22ff68ebf4b3737a2f`).
The contract implemented literally is section 10 of `scene-branch-research-002.md` (accepted as research only).

## Milestone step, route, failure, runtime change

* Milestone step: load an installed level and render its scenery during play.
* Route: `runApplication --level MM_DIG_000` -> `runApplicationLeg` (main.cpp) -> `AssetLibrary` reads
  `COMPILED\MM_DIG_000.LVL` -> `BackdropScene::build` -> Secondary traversal -> `renderStage(Secondary)` ->
  `dispatchSecondaryChildren` -> branch-1 draw -> `rasterizeSceneTriangle` -> shared tube composition
  (`scheduleSceneFrame`).
* Observed failure (census line, `scene-admission-001`; reproduced by the frozen driver below): `readSceneNode`
  rejected the Secondary tree at the first branch-1 node (`unsupported nonzero type-2 branch 1 at 0xa7572`) and
  `BackdropScene::build` returned a successful empty partial scene with one diagnostic.
* After: the 19 `fire` branch-1 nodes parse, are structurally admitted and draw animated additive sprite quads.

## Step 0: frozen entry sources

Copied once, before any production edit, to ignored `build/scene-branch-impl-001/frozen/src/{assets,app}/`;
never swapped into `src/`. SHA-256 equals the supervisor entry manifest
(`build/agent-runs/20261031T193658-47832-2e53d4d4/before-source-hashes.json`); the CTest
`scenebranchimpl_identity` re-checks them with `cmake/scenebranchimpl_identity.cmake`:

| File | SHA-256 (entry) |
| --- | --- |
| src/assets/SceneNode.cpp | 9e053b93...b40485 |
| src/assets/SceneNode.h | 9edf809e...7423c8a3 (see identity script for full values) |
| src/app/SceneBillboardSource.cpp / .h | 17e8bbcf...f626ae / 83494ea0...8ef8e2 |
| src/app/BackdropScene.cpp / .h | 682fe693...ed94 / 6f530a25...c11c |
| src/app/main.cpp | a2789bdf...6c94 |

Full 64-hex values are in `cmake/scenebranchimpl_identity.cmake`.

## Changed files

Production: `src/assets/SceneNode.{h,cpp}`, `src/app/SceneBillboardSource.{h,cpp}`, `src/app/BackdropScene.{h,cpp}`,
`src/app/CrtRand.{h,cpp}` (new generator unit), `src/app/main.cpp`, `cmake/webolution_sources.cmake`.
Tests/registration: `CMakeLists.txt`, `cmake/scenebranchimpl_identity.cmake` (new), `src/__test__/test_scenebranch_units.cpp` (new),
`src/__test__/test_scenebranch_impl.cpp` (new, frozen/current driver), and the superseded-assertion edits below in
`test_scenenode.cpp`, `test_backdropscene.cpp`, `test_sceneintegration.cpp`, `test_sceneadmission.cpp`,
`test_sceneadmission_census.cpp`, `sceneadmission_census.hpp`.

## Evidence mapping (address -> source)

| Address | Behaviour | Source |
| --- | --- | --- |
| `0x0047B44C`, `0x00468BF2` (`0x0047B4E5`) | branch byte is a boolean; branch 1 reads u32 f1..f4, name (capacity 0xFF), trailing word, no UV doubles | `SceneNode.cpp` `boundedString`, branch-1 arm; `branchWords`, `branchName` |
| `0x0047D0D9` | visibility 0 draws nothing | `BackdropScene.cpp` `drawSprite` (before the latch) |
| `0x0047D158..0x0047D35F`, `0x0047D40B..0x0047D477` | corners as branch 0, `+H/2` on y of all four | `makeSceneSpriteQuad` |
| `0x0047D4FB..0x0047D613` | screen slot formulas | `makeSceneSpriteQuad` `sx`/`sy` |
| `0x0047D6F9..0x0047D76B` | z permutation p1,p0,p3,p2 | `zs` |
| `0x0047D771..0x0047D81B` | `a0=(x0+..+x3)/4`, `a1=v1.y`, `a2=x2-x1`, `a3=x3-x0`, `a4=y1-y0` | `out.args` |
| `0x0047A565` | `a0+=int[X+0x30]`, `a1-=int[X+0x34]`, x/y vertex formulas with `w=int[X+0x38]`, `h=int[X+0x3C]`, `+h/4` since `[0x58D53C]==0` | `makeSceneSpriteQuad` |
| `0x00440C50` | UV rule `(P0..P3)`, mirror swaps S10/S18 | `uv`; `S10..S28` = `uBottom,uTop,uLeft,uRight` (the recovered reader stores `double_bits_10[0..3]` in that order) |
| `0x0047D391..0x0047D3F8`, `0x004040A0`, `0x0049C5AC` | first draw: `temporal = trunc(double(count-1) * rand()/32767.0)`, latch | `latchSpriteFirstDrawFrame`, `drawSprite` |
| `0x0049C84A`, `0x0049C840`, `0x00447580` | CRT LCG, srand, `srand(ftol(ms*32767.0))` | `CrtRand` |
| `0x0045D06C`, `0x004451E5` | seed points (main-loop entry, play start) | `main.cpp` `sceneryRandom().begin()` in `runApplication`, `.seed()` after the backdrop is built in `runApplicationLeg` |
| `0x0045CFF9`, `0x004423D2`, `0x00437BDE`, `0x004764C4` | `dt = min(ms,100)/1000`, zero outside play / paused, scene clock and node delta | `BackdropScene::advanceScenery`; `main.cpp` call before `renderFrame` |
| `0x00468833` | forward temporal advance (0.06, 1e-8, loop wrap, finished flag) | `advanceSpriteTemporal` |
| `0x0046840B` early return | admission requires exactly one directional image per temporal entry | `BackdropScene::build` `admitSprites` |
| `0x00414920`, `0x00414D80`, `0x0047261F`, `0x00440A10` | additive blend selector table, flags `&3` alpha-test row (GREATER, 0xEF), white diffuse | `spriteOptions`, `spriteTextureFlags`, `billboardRasterVertex(..., {255,255,255,255})` |

## Superseded assertions (exact list) and replacements

Every other assertion and timeout is unchanged.

| Assertion (before) | Replacement (now) |
| --- | --- |
| `test_scenenode.cpp:162` "unproved type-2 branch rejected" (message `unsupported nonzero type-2 branch`) | the same truncated record (branch byte, no payload) is still rejected, now as `truncated type-2 branch-1 field`; plus `testBranchOne` (parse, boolean byte, name 255 ok / 256 rejected, every truncated prefix rejected, exact-end visibility, branch 0 unchanged) |
| `test_backdropscene.cpp:35` real MM_DIG_000 is a one-diagnostic empty scene | real MM_DIG_000 is admitted with 19 sprites and no diagnostic; the unsupported-form check now uses the real bytes with the first record's sprite name `fire` -> `nofi` (missing sprite file): one level-qualified `Secondary` / `unsupported type-2 branch 1 sprite 'nofi'` diagnostic, empty partial scene |
| `test_sceneintegration.cpp:219` real branch-1 bytes diagnosed empty | admitted under any ID; the `nofi` variant is the diagnosed empty partial scene |
| `test_sceneintegration.cpp:234,238` real-ID MM_DIG_000 empty / `unsupported nonzero type-2 branch` | same `nofi` variant keeps "explicit partial" and names role and form |
| `test_sceneadmission.cpp` census expectation (independent classifier marked every branch-1 node unsupported; reason bucket `unsupported nonzero type-2 branch`) | `test_sceneadmission_census.cpp` gains an independent mirror of the branch-1 admission rule (written from the contract), `structuralOutcome(bytes, library)`; reason bucket `unsupported type-2 branch 1 sprite` |
| `scene-admission-001` counts "10 admitted / 74 branch-1 / 18 nested" | re-censused below; the registered `sceneadmission_before` driver (frozen allowlist) is unchanged and still passes |

## Executed results

Source: `build/scene-branch-impl-001/{before,after}/measurements.txt`, `hashes.txt`, `levels.txt` (ignored `build/`),
produced by the registered CTests `scenebranchimpl_before` (frozen sources, 10 s) and `scenebranchimpl_after`
(current sources, 39 s). Resumed run: the frozen copies and `before/` were not recaptured; their SHA-256 equal the
original `before-source-hashes.json` of run `20261031T221341-32848-ec0f5ebf` for all seven files.

### (a) Frozen vs current, all 102 effective installed levels

* Frozen: 10 admitted, 92 empty. `MM_DIG_000 admitted=0`, one diagnostic:
  `MM_DIG_000 Secondary paired-secondary[0] source=0x98f84: unsupported nonzero type-2 branch 1 at 0xa7572; scenery not admitted: empty partial scene`.
* Current: `MM_DIG_000 admitted=1`, no diagnostic.
* The ten previously admitted levels: scene-only, scene-on and scene-off RGB and depth are byte-identical to frozen
  (`frozenCurrentEqualHashes=30`). MM_CLIMB_002 keeps Primary 923/0, Legacy 172/0, Secondary 82/17, 1177 records,
  17 billboards and primary hash `0x1FD0BFB82AB39E95` (asserted by the existing checks, which still pass).

### (b) MM_DIG_000 on actual bytes, fixed camera (`after/fixed-camera.json`), injected generator state 1

* Per role: Primary 975 raster calls (0 sprite), 292018 colour/depth stores; LegacyUnattributed 336 calls, 12194 stores;
  Secondary 628 calls = 598 mesh + 30 sprite, 340203 stores of which 508 sprite and 339695 mesh. Rejected: 65 / 0 / 133.
  Missing sprites/textures: 0.
* 19 branch-1 `fire` submissions listed separately (`submission,N,offset=...` lines): 15 draw (2 triangles each, 30
  raster calls) and 4 are culled for non-positive depth (offsets 0xbc7c7, 0xbc6ea, 0xbcdd2, 0xbccf5); 0 rejected.
  Chosen temporal frames under state 1: 0,8,2,12,8,7,5,13,12,11,2,12,10,7,4,0,1,5,2; mirror 0 for all.
* Artifacts (ignored): `after/MM_DIG_000-scene-{only,on,off}.{rgb,depth,png}`, `…scene-only-stateB.png`,
  `application-first-frame.png`, `application-last-frame.png`. Scene-only sprite pixels 508; scene-on vs scene-off
  changed pixels 122307.
* Determinism: two renders under state 1 are byte-identical (RGB `0xa05aeb68e161ebc5`, depth `0xcf33f34e4fa31598`).
* Mutation: state 12345 changes the chosen frames (3 8 11 10 2 10 12 2 13 10 6 6 9 9 4 6 10 4 11); 348 pixels differ,
  0 outside sprite pixels.
* Frame advance (hand-computed literals): paused does not change, one-frame step, capped at 100 ms, changed after advance.
* Orientation (computed from the literal formulas on the measured fire record, not assumed):
  a0..a4 = 287.0007, 372.0518, 31.6744, 31.6744, 58.9955; a2 and a4 positive. Mirror 0: image-left edge appears at screen
  right, image-top edge at screen up (horizontally flipped relative to the texture); mirror 1: left at right, top at
  down (rotated 180 degrees). All 19 MM_DIG_000 records have mirror 0. `texture_name_flags` for `_FIRE0.BMP` is
  asserted equal to 1.

### (c) 102-level re-census

* 84 admitted (the 10 plus 74 newly admitted), 18 empty. Newly admitted: 74 levels, the entire previous branch-1 class
  (list in `after/measurements.txt`), each with `missingTextures=0`; 71 have 19 fire sprites, 3 (E10_KF_2, E20_MG_1,
  MM_E4_4_SKILLS) have 3. Every one of the 74 rendered scene-only without error; this is source-built rendering support
  only and no pixel claim is made for them.
* Remaining 18 first-failure reasons: all `Secondary: unsupported nested/non-leaf secondary dispatch`
  (E11_KF_1, E22_KF_1, H05_KF_1, H09_KF_1, H20_KF_1, H24_KF_1, M04_KF_1, M11_KF_1, M16_MG_1, M21_KF_1, MM_BLOCK_BOMB_A,
  MM_FLOAT_CLIMB2, MM_H_003, VH04_JC1, VH05_JC1, VH18_JC, VH19_JC, VH21_JC). These remain diagnosed empty partial scenes.
  Nothing in this task promises their conversion.

### (d) Public application route

`runApplication --level MM_DIG_000 --sim` under SDL dummy video/audio: 6 frames, return 0, observer-queued SDL_QUIT,
2.19 s; `advanceCalls=6`, `advancesWithTime=6`, `temporalSteps=114`, `firstDrawLatches=19`. With the scenery switched
off: 6 frames, 0.83 s, no advance calls. Application-level changed pixels versus scenery off: 114436. The seed state at
first draw is printed in run diagnostics. No timeout was raised: `scenebranchimpl_before` 10.2 s and
`scenebranchimpl_after` 38.6 s against the registered 120 s; `scenebranch_units` 0.01 s against 60 s. Build of all 102
levels 1.53 s (84 admitted), 7.5 ms typical per level; the 84-level scene-only render pass takes 30.7 s.

## Verification

`python tools/agent_control/verify.py --profile all`, run twice on the final tree (the first in the interrupted worker
before the timeout, the second in this resume, after which nothing in `src/` or CMake changed): reference SHA-256 verified
by reading bytes only; main 97 registered tests passed (93 existing plus `scenebranch_units`, `scenebranchimpl_identity`,
`scenebranchimpl_before`, `scenebranchimpl_after`), 543 s; recovery 84 registered tests passed (unchanged count), 29.8 s.
This is source verification, not new native fidelity evidence.

## Limits, discrepancies and next dependency

* The original seed is wall-clock derived (`0x00447580`), so original flame phases are not reproducible; production seeds
  from the application's monotonic millisecond clock at application start and at play start, and tests inject state.
* No scene-duration wrap is parsed. The clip rate is taken from the parsed clip. No fast-forward factor is connected
  (priority 4). Multi-image directional selection (`0x00418350`, `0x00480AC8`) and nested Secondary dispatch are not
  implemented; admission rejects those forms with level-qualified diagnostics instead of drawing a subset.
* The render state is a conditional reading: `_FIRE0.BMP`, alpha test StrictGreater 0xEF, Modulate with white diffuse,
  additive blend from selector 2 via the `0x00414D80` table, `options()` filter/address/depth policy.
  `conditionalCaretFlags8Policy` is not used.
* The corner orientation signs above are consequences of the literal formulas; whether they match the original on screen
  is not independently established. No pixel agreement with an original render, no x87 equivalence, no clipping or
  full-frame fidelity, no digger gameplay. C3 stays FAIL.
* Discrepancy: the contract's "74 branch-1 levels convert" was not promised and happened for all 74 only because their
  fire sprite/texture/single-image invariants hold; the report treats this as a measured result.
* Next dependency: nested/non-leaf Secondary dispatch for the 18 remaining levels (needs its own bounded static contract),
  then multi-image directional selection if any other sprite is encountered, then priority 4 (Armageddon/fast-forward) only
  with established semantics.
