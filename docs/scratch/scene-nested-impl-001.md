# scene-nested-impl-001: nested type-1 Secondary scenery on E11_KF_1

2026-11-01. Production implementation; finishes `needs_review`. Source-built rendering support only: no original
executable, copied original instruction, oracle, hook, debugger, capture, launcher, installation or save write was used.
Static evidence is the accepted `docs/scratch/scene-nested-research-001.md` (reference SHA-256
`32675bdb49866e76502713251fdfd2b09078c2481c72ca22ff68ebf4b3737a2f`, read as bytes only); no new REA lookup was needed
because every question in the scope was answered by that report's E1-E11.

## Milestone step and route

Milestone step: *load an existing installed level and render its scenery during play.* Route:
`runApplication --level E11_KF_1 --sim` -> `runApplicationLeg` -> `AssetLibrary` read of `COMPILED\E11_KF_1.LVL` ->
`BackdropScene::build` (structural admission) -> `renderStage(Secondary)` -> `dispatchSecondaryChildren` ->
`drawMesh` -> `rasterizeSceneTriangle` -> shared tube composition.

* **Before (observed, frozen driver):** `E11_KF_1 Secondary paired-secondary[0] source=0xb33dc: unsupported nested/non-leaf
  secondary dispatch; scenery not admitted: empty partial scene` - one diagnostic, no scenery in any role. 84 admitted /
  18 empty over the 102 effective levels.
* **After (observed, current driver):** E11_KF_1 is admitted with no diagnostics; Secondary submits 161 mesh records and 12
  billboards (345 raster calls, 102,025 accepted colour/depth stores) including the nested windmill; 102 of 102 levels are
  admitted, none is an empty partial scene.

## Changed files

| File | Change |
| --- | --- |
| `src/app/SceneDispatch.h/.cpp` | `binaryInsertionOrder()` (exact E10 algorithm); `dispatchSecondaryChildren` now orders only the root's direct children with it, then emits each child's descendants depth-first in file order; `SecondaryDispatchItem` gains `depth`, `parentOffset` (appended, additive); new `SecondaryDispatchOptions` (default = legacy leaf behaviour with no gates; `originalGates`, `hidden`). |
| `src/app/BackdropScene.cpp/.h` | The blanket `nested/non-leaf` guard is replaced by a structural predicate; dispatch calls pass `originalGates=true`; `setSecondaryHidden()` seam (never installed in production). |
| `src/__test__/test_scenenested_impl.cpp` (new) | One driver, frozen and current executables (see Demonstration). |
| `src/__test__/test_scenenested_units.cpp` (new) | Asset-free hand-computed ordering/traversal/gate checks. |
| `cmake/scenenestedimpl_identity.cmake` (new), `CMakeLists.txt` | 4 registrations: `scenenested_units`, `scenenestedimpl_identity`, `scenenestedimpl_before`, `scenenestedimpl_after`. |
| `src/__test__/test_sceneadmission_census.cpp`, `src/__test__/test_sceneadmission.cpp` | Intentionally superseded assertions (below). |

Ignored outputs: `build/scene-nested-impl-001/{frozen,before,after,entry-manifest.json,report.md}`.

## Evidence mapping (address -> source)

| Address / finding (research E-number) | Source |
| --- | --- |
| `0x0046B8F4..0x0046BA72`, insert helper `0x0046C2A0` (E10) | `binaryInsertionOrder` (SceneDispatch.cpp): `lo=0, hi=size-1`; `while(hi-lo>1)` `mid=(hi+lo)>>1`, `key[i]<key[idx[mid]] ? lo=mid : hi=mid`; `key<key[idx[hi]]` -> `hi+1`, `key>key[idx[lo]]` -> `lo`, else `lo+1`. Replaces `std::sort` for the root's direct children. |
| `0x0046B73F` key `z' + child[+0x108]`, only direct children ordered (E9, E11) | Keys unchanged; nested children never ordered. |
| `0x0046C6F6..0x0046C737`, `0x0046B715..0x0046B734` (E6, E7) | `emit` in `dispatchSecondaryChildren`: own item, then children in file order, depth-first, before the next ordered direct child. |
| `+0x110` gate `0x0046C56B`, `0x0046B6DA`, `0x0047D0D9`; constructor default `0x0046B645`; type-2 reader `0x0047B4DC` (E2, E6-E8) | `SecondaryDispatchOptions::hidden` (any node, subtree skipped) and `originalGates` (type-2 serialized byte 0 -> nothing; branch-0 billboards now honour it too). |
| `0x0046C5C4..0x0046C5E4` radius cull `z + [this+0x100] <= 0` (E7) | `depthOf(composed)+headDoubles[0] <= 0.0` for every type-1 node under `originalGates`; skips node and children. |
| `0x47D0F2` / `0x0046B6EB` double matrix push for type-2 owners (E8) | Evidenced, unexercised: a type-2 node with children is rejected (`unsupported type-2 node with children at 0x... (double matrix push ... is not implemented)`). |
| `0x45DA0D` cutme frame-time LOD (E4) | Not connected; cutme nodes stay visible (limit). |

Admission predicate (`BackdropScene::build`): Secondary root must be type 0; every node type 0, 1 or 2; only type-0/type-1 nodes
may own children. No level-ID table, marker scan or per-level offset. `admitSprites`, `buildSceneMeshSource`, `collectEyes`,
`collectBillboards`, mesh/billboard builders, rasterizer, SceneFrame schedule, camera, `main.cpp` and the Primary /
LegacyUnattributed stages are unchanged. Type-0 nodes below the root are allowed by the stated predicate and are traversed
with their children in file order (none exist in the install).

## Executed results (source-built; all numbers from `build/scene-nested-impl-001/{before,after}/measurements.txt`)

### (a) Frozen vs current over all 102 effective levels

* Frozen identity: `scenenestedimpl_identity` hashes the seven frozen files against the supervisor entry manifest
  (`build/agent-runs/20261101T162013-27548-5d48acb9/before-source-hashes.json`, copied to `build/scene-nested-impl-001/entry-manifest.json`);
  it fails on mismatch and passed. SHA-256: BackdropScene.cpp `7d54a42e...`, BackdropScene.h `50259077...`, SceneDispatch.cpp
  `92531486...`, SceneDispatch.h `1cd33387...`, SceneNode.cpp `ac09bfed...`, SceneNode.h `75294911...`, main.cpp `ccbd4a62...`.
  The frozen sources are compiled into `test_scenenestedimpl_before` under renamed entry points (`readSceneNode`,
  `dispatchSecondaryChildren`); no production file was swapped.
* Frozen: 84 admitted / 18 empty (`Secondary: unsupported nested/non-leaf secondary dispatch` x18); E11_KF_1 is the diagnosed
  empty partial scene with exactly one diagnostic.
* Current: **102 admitted / 0 empty**. All 18 targets admitted (E11_KF_1, E22_KF_1, H05_KF_1, H09_KF_1, H20_KF_1, H24_KF_1,
  M04_KF_1, M11_KF_1, M16_MG_1, M21_KF_1, MM_BLOCK_BOMB_A, MM_FLOAT_CLIMB2, MM_H_003, VH04_JC1, VH05_JC1, VH18_JC, VH19_JC,
  VH21_JC), each rendered scene-only with `missingTextures=0`. No first-failure reason remains. Source-built admission only; the
  original loads every level.

### (b) E11_KF_1 fixed camera (`buildCamera(CameraParams{})`)

| Role | candidates mesh/billboard/sprite | raster calls | accepted colour = depth stores | rejected (proj/raster) | missing |
| --- | --- | --- | --- | --- | --- |
| Primary | 1385/0/0 | 1841 | 354,180 | 4 (4/0), 268 predicate-rejected | 0 |
| LegacyUnattributed | 172/0/0 | 336 | 8,760 | 0 | 0 |
| Secondary | 161/12/0 | 345 | 102,025 | 0 | 0 |

* Nested submissions listed apart: `cube1_trans` (0xb60e7) 12 records, 23 candidate triangles, 2,513 accepted stores;
  `windmill_roll_0.2_trans` (0xb6ed3) **5 records**, 10 candidate triangles, 2,615 accepted stores; the other 27 direct
  children 96,897 stores.
* Dispatch order equals the independently written literal `trees/E11_KF_1.plan.txt` order (29 entries); the rasterizer's
  node submission order equals the plan restricted to nodes with records. cube1_trans is followed immediately by its child
  (depth 2, `parentOffset` 0xb60e7); all others depth 1.
* Culls: 29 dispatched with gates, 29 without; the radius cull and visibility gate removed nothing at this camera.
* Ties: 9 bit-identical key pairs among 28 direct children; `std::sort` differs from the evidenced order in 5 positions.
* Artifacts (ignored): `after/E11_KF_1-scene-{only,on,off}.{png,rgb,depth}`, `after/application-{first,last}-frame.png`.
  Scene-only non-blank pixels 292,734; scene-on vs scene-off changed pixels 130,359; scene-only RGB `0x1e8a0876ee92870c`,
  depth `0x9bf501996536d7bf`; two renders byte-identical (RGB and depth, scene-only and composed).
* Mutations: hiding `cube1_trans` -> 27 dispatched, body and blades gone, Secondary raster calls 345 -> 312, 3,870 pixels
  change; hiding only the child -> 28 dispatched, only the blades gone, calls 345 -> 335, 2,116 pixels change, zero accepted
  blade stores remain. The std::sort mutation is the order comparison above (unit check below shows the same on the retained
  key list). A synthetic type-2-with-children tree (E11_KF_1 bytes with the type-2 leaf 0xb7576 given child count 1 and a
  copy of itself as its child) is a successful, diagnosed empty partial scene:
  `E11_KF_1 Secondary paired-secondary[0] source=0xb33dc: unsupported type-2 node with children at 0xb7576 (...); scenery not admitted: empty partial scene`.

### (c) Public route

`runApplication --level E11_KF_1 --sim --native --geom route --camera <fixed> --no-audio` under SDL dummy video/audio: 4
frames, return 0, observer-queued SDL_QUIT, 1.71 s; with `--scene-off` 0.78 s. Changed pixels versus scene-off on the
sampled first frame: 127,329. No external timeout fired or was raised.

### (d) Unit checks (`scenenested_units`, hand-computed literals)

Ties: `{5,5}`->`0,1`; `{1,1,1}`->`0,2,1`; `{3,3,3,3}`->`0,3,2,1`; `{10,5,5}`->`0,2,1`; `{5,5,1}`->`0,1,2`; `{1,5,5}`->`1,2,0`;
the retained E11 key list -> `4,27,26,25,24,0,1,19,18,21,20,23,22,17,5,14,7,6,9,8,3,2,12,13,16,15,11,10` (triple 23,22,17 included),
which differs from `std::sort`; 80 tie-free LCG keys equal `std::sort(key >)`. Nested children are emitted depth-first and not
reordered (a nested child whose key would sort first stays in file order); depth/parentOffset; hidden owner/child; type-2
visibility byte 0/1 (and the legacy default ignoring it); type-1 `z + r <= 0` cull of node and subtree (boundary equal culls,
0.5 over is kept), nested child culled on its composed depth.

### Preservation

* Ten originally admitted levels (E06_KF_1, H17_MG_1, M06_KF_1, M08_KF_1, MM_BASH_002, MM_CLIMB_002, MM_E1_3_SKILLS,
  MM_E1_4SKILLS, MM_W_012, VH22_JC): scene-only, scene-on and scene-off RGB and depth hashes equal frozen (30 comparisons).
  MM_CLIMB_002 keeps 923/0, 172/0, 82/17, 1177 records and primary hash `0x1FD0BFB82AB39E95`.
* The 74 branch-1 levels (including MM_DIG_000): **no pixel changes**. Every scene-only RGB+depth hash equals frozen
  (`TIEUNCHANGED` x74 in `after/measurements.txt`) even though the research found a differing tie order in each: the tied pairs
  do not overlap in a way that changes the final colour/depth at this fixed camera. This is a measured outcome for this camera
  only, not a claim that no camera produces a difference. No branch-1 assertion depended on the old tie order, so none was changed.
* Existing tests `backdropscene`, `scenedispatch`, `sceneintegration_*`, `scenebranchimpl_*` (MM_DIG_000 19/15/4 sprites)
  passed without modification.

### Intentionally superseded assertions (exact list)

1. `src/__test__/test_sceneadmission.cpp` (sceneadmission_after): `admitted.size()==84&&emptyCount==18` ("84 admitted, 18
   nested/non-leaf levels remain empty partial scenes") -> `admitted.size()+emptyCount==names.size()&&admitted.size()==102&&emptyCount==0`.
2. `src/__test__/test_sceneadmission_census.cpp`: the independent census predicate (root type 0 and every direct child a
   childless type-1/2 leaf -> "unsupported nested/non-leaf secondary dispatch") -> structural mirror (type-0 root, node types 0..2,
   no type-2 owner of children -> "unsupported secondary node form"). It feeds `sceneadmission_after`, `scenebranchimpl_after`
   and `scenenestedimpl_after`, which require production admission to equal this independent outcome.
3. Not changed: `test_scenenested_measure.cpp` asserts the historical 18-level census with its own copy of the old predicate and
   does not call production admission; `test_backdropscene.cpp`, `test_sceneintegration.cpp`, `test_scenebranch_impl.cpp` needed no edit.
   The unsupported-form replacement is the new synthetic type-2-with-children check in `scenenestedimpl_after`.

## Verification

`python tools/agent_control/verify.py --profile main` and `--profile recovery` were run separately on the final tree
(a single `--profile all` run would exceed the 600 s tool limit: the main CTest alone took 637 s, of which
`levelreturn` is 234 s). Both ended with `git diff --check` clean and the reference SHA-256 verified by reading bytes only.

* main: **102 registered tests passed, 0 failed** (the 98 existing plus `scenenested_units`, `scenenestedimpl_identity`,
  `scenenestedimpl_before`, `scenenestedimpl_after`), total test time 637 s. New tests: `scenenested_units` 0.04 s,
  `scenenestedimpl_identity` 0.03 s, `scenenestedimpl_before` 35.8 s, `scenenestedimpl_after` 46.3 s, all against the
  registered 60/60/120/120 s timeouts (none raised). `sceneadmission_after` 20.0 s, `scenebranchimpl_after` 44.8 s.
* recovery: **84 registered tests passed, 0 failed** (unchanged count; `LEMMINGS_REFERENCE_EXE` empty, `native_int_array` the sole
  `native_`-prefixed source test), 31 s.
* Full 102-level render pass in the current driver: 39.9 s; build of all 102 levels 1.6 s.
* Only the registered verifier was used; the shell was not used to compile or run anything else. This is source verification,
  not native fidelity evidence.

## Limits and discrepancies

* Serialized pose only: the windmill blades (`windmill_roll_0.2_trans`) are not animated.
* The clipped type-1 draw `0x469EAA` is not reconstructed; the per-vertex `z>0` rejection stays (clipping park).
* The `cutme` frame-time LOD (`0x45DA0D`) is not connected; cutme nodes stay visible. No newly admitted level contains one, and
  the 71 branch-1 levels with ten cutme nodes each are unchanged by this task.
* Type-2 owners with children (double matrix push) are rejected, not implemented.
* The type-1 radius cull is applied to every type-1 node as in the original; it removed nothing on E11_KF_1 and nothing in the
  ten preserved levels (byte-identical), so the "restrict to nodes with children" fallback was not needed.
* The default dispatch options are intentionally the legacy ones (no gates) so that the existing synthetic fixture
  (`test_scenedispatch`, a type-2 node with a zero visibility byte and a type-1 node at z=0, radius 0) keeps its accepted
  assertion; production always passes `originalGates=true`.
* The other 17 levels are admitted and render with `missingTextures=0` but only E11_KF_1 has per-node demonstrations; equal output is not promised.
* No pixel agreement with an original render, no x87/clipping/full-frame/timing fidelity; C3 stays FAIL; no digger gameplay.
  Natural play-through of E11_KF_1 is unverified. Source-built admission is not original admission.

## Next dependency

No scenery first-failure reason remains among the 102 installed levels. Next milestone dependency per the task list is
priority 2 (selected-hatch camera orientation for other levels) or, only after a user-authorized static research task,
Armageddon/fast-forward. Blade animation would need its own bounded static research (animation matrices).
