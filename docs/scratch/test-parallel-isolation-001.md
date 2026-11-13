# test-parallel-isolation-001: test isolation and scheduling for a later four-job trial

Task scope: prepare both CTest suites for a later measured four-job trial. Nothing here
is a measured speedup, and parallel operation is **unvalidated** until that trial. The
verifier defaults (12 compile jobs, 1 test job, `--timeout 360`) are unchanged.

Method: static reading only. CMake registration (`CMakeLists.txt`,
`decomp-lemmings/CMakeLists.txt`), the test sources under `src/__test__` and
`decomp-lemmings/{tests,tools}`, the last full serial verification log
`build/agent-runs/20261112T212549-7804-08300df1/verification.log` (148 main tests,
2604.30 s real; 84 recovery tests, 28.33 s) and the CTest inventory that run wrote
(`build/agent-native/main/agent-test-inventory.json`). No test or verifier was run by this
worker; no original binary, oracle, debugger or capture was touched.

Retained machine-readable evidence (ignored, under `build/test-parallel-isolation-001/`):
`resource-map.tsv` (per-group resources, owners, sharing, status) and
`dependency-graph-before-after.txt` (registered edges before/after, critical paths).

## Edits (two, both source-traced; neither was observed failing under parallel load)

1. `src/__test__/test_box.cpp`, `scratchDir()`: scratch moved from `<TMP>/webolution_test` to
   `<TMP>/webolution_test/box`. `main()` ends with `fs::remove_all(scratchDir())`, which deleted the
   shared parent that `test_cueplayback` also uses (`<TMP>/webolution_test/cueplayback`). The
   documented cleanup is now confined to box's own directory. Assertions, names, counts unchanged.
2. `CMakeLists.txt`, `targetedplay_prepare`: added `DEPENDS fluidlever_play_stable-camera;
   fluidlever_play_ordinary-camera`. `fluidlever_play_*` scans
   `build/targeted-play-001/runs/*/save` at start (`findSourceSave`, newest run whose save has a
   known FNV) and copies it; the `targetedplay_*` chain creates and rewrites a run there. The serial
   registration order already ran both fluid plays first, so they only saw completed runs; the edge
   preserves that under parallel scheduling. Save writes are temp-file + replace (`FileSaveSink`),
   so a torn read is unlikely, but the selection and copy would otherwise depend on timing.
   Inference (not run): the node-0 save that `targetedplay_prepare` copies has FNV `c05382077a330a32`,
   which is not one of the fluid driver's two accepted FNVs, so an in-flight run only becomes eligible once
   `targetedplay_play_1` writes its win. The race is narrow, and the edge removes it entirely.
   **Cost:** the chain `fluid play (224 s) -> prepare -> play_1 (770 s)` becomes the critical path,
   about 995 s instead of 771 s. If the benchmark shows that matters, the coordinator can drop the
   edge and accept the narrow window; this is a trade the coordinator should decide, not this task.

No RESOURCE_LOCK, RUN_SERIAL or PROCESSORS was added: no audited resource needed a lock, and none
has contention data yet.

## Inventory

### Main suite (148 tests; serial seconds from the 20261112T212549 log)

| Group | Tests | Serial s | Mutable resources | Ordering / sharing | Kind |
|---|---|---|---|---|---|
| Pure/offline units | ~60 (box, sprite*, scene*, hud*, cursor*, pick*, simclock, world, terrain, ...) | < 1 each | none, or read-only installed assets | independent | assertion |
| Scratch under `<TMP>` | box, cueplayback, progress, mapscene, savedgame, savepersist, savedemo_after/before, levelreturn, cameraentry_* | 0.05 to 228 | fixed names unique per test (see `resource-map.tsv`); cameraentry adds a clock stamp | box/cueplayback fixed (edit 1); the rest never shared | assertion |
| Fresh-dir claimers | spritestate, bitmap, skillpanel, actorcompose | 0.1 to 3.4 | `create_directory` claim of a new `run-*` dir; never reuse/remove | independent | assertion |
| Fixed build dirs, single writer | actorcompose (`build/tmp/a*`), collisiongrid (`build/native-010/011`), retaillevel (`build/tmp/d3a-...`) | 0.5 to 5 | written only by that test; retained inputs read-only | independent | assertion + dump |
| Measurement dumps | *_measure, *_census (7) | 0.05 to 30 | own dir (`build/<task>` or cwd = build dir) | independent | measurement (artifact writers; assertion depth not audited per test) |
| Before/after pairs | sceneintegration, sceneadmission, scenebranchimpl, scenenestedimpl, cameraentry, fastforwardapp, audiodemo, savedemo, teleportdemo, levelselection | 0.3 to 44.5 | each side owns `<root>/<side>` or its own file | scenebranchimpl_after and scenenestedimpl_after READ the before side's `levels.txt`/`hashes.txt`: already ordered by DEPENDS | assertion + report |
| SDL dummy window/app | frontendwindow, frontendentry, level{interaction,overlap,visibility,cursor}, levelmenuwindow, entrycuewindow, audioapp, teleportapp, fluidsurface_impl, runtimewindow, frontend_menu | 1 to 65 | own output dir each; no normal-save writer | independent | assertion |
| Fluid assessment | fluidlever_play/restart x {stable-camera, ordinary-camera} | 224 + 0.2 each | `build/fluid-lever-assessment-001/<variant>` (`remove_all` of that variant only), private save copy | variants independent; play -> restart ordered; reads targeted-play-001 runs (edit 2) | **observation driver** |
| targetedplay chain | prepare, play_1, restart_1, play_2, restart_2 | 771 total | `build/targeted-play-001/{current-run.txt,runs/<stamp>}`; reads retained `playable-expansion-001/runs/20261103-003501` save | one linear chain | **observation driver** |
| playablebaseline | survey, play/restart route 1-3 | 141 total | `build/playable-baseline-001/route-N` (own remove_all) | routes independent after survey | **observation driver** |
| playableexpansion | survey, play/restart 1-4 | 463 total | one save under `build/playable-expansion-001/runs/<stamp>` | one linear chain (survey -> play_1 -> r1 -> ... -> r4) | **observation driver** |

Observation drivers (fluidlever, targetedplay, playablebaseline, playableexpansion): per their
CMake comments the exit code reflects driver integrity (route ran, artifacts written, normal save
untouched), not the clone outcome. A green result does not mean the lever was reached or a level won.

### Recovery suite (84 tests, 28.33 s serial)

Independent. Unit executables write nothing. `resource_lifecycle_boundaries` is the only writer: it
uses `tempfile.TemporaryDirectory(prefix='resource_boundary_', dir=<program dir>)`, unique per process.
`installed_*` tests read `lemmings.box`/`patch.box` read-only (shared reads allowed); the largest are
`installed_texture_payloads` 17.1 s and `installed_level_prefixes` 7.5 s. No edit needed.

### Cleanup ownership audit

Every `remove`/`remove_all` in `src/__test__` targets that test's own resolved root (levelreturn, savedemo,
savepersist, savedgame, fluidlever `<variant>`, targetedplay/playableexpansion step dir,
playablebaseline `route-N`, levelselection own save file, progress/mapscene own file, cueplayback own
files, the two cmake scripts' own outputs), except `box` before edit 1.

## Heavy resources and limits (measured serial times; contention not measured)

- Five groups are 79% of serial time: targetedplay 771 s, fluid 448 s, playableexpansion 463 s,
  playablebaseline 141 s, levelreturn 228 s. Wall-clock floor at four jobs is
  `max(critical path, 2604/4 = 651 s)`; the critical path is now about 995 s (edit 2) or 771 s without it.
- These drivers pace themselves in **wall-clock** time (the natural-play fluid/targeted/playable drivers have
  120 s and 60 s real-time watchdogs; levelreturn's three natural runs take 190 to 239 s). CPU contention can
  change both frame pacing and watchdog outcomes, not just duration. Timeout margins at serial speed:
  levelreturn 228 of 360 s (tightest); audioapp 65 of 120 s; scenenestedimpl_after 44 of 120 s;
  scenebranchimpl_after 44 of 120 s. Timeouts were not changed.
- The `ctest` default `--timeout 360` only applies to tests without a TIMEOUT property.
- Cost data is not trustworthy for ordering: `build/agent-native/main/Testing/Temporary/CTestCostData.txt`
  holds each test's last duration divided by its run count (for example fluidlever_play 2 runs / 111.8 s, while
  `levelreturn` 128 runs / 1.8 s against 228.4 s measured), so `-j4` will start newer tests before older heavy ones.
  Order affects wall time only, not correctness.
- Options if contention matters, to be decided from measurement: `PROCESSORS 2` on the heavy render tests (reserves
  slots) or `RUN_SERIAL` on levelreturn. Not applied, since there is no data.

## Unknowns (explicit, not assumed safe)

- Whether any heavy test changes outcome (not just time) under CPU contention: unmeasured.
- Per-test memory: unmeasured. Host per plan: 32 logical processors, ~31 GiB.
- `runtimewindow` and `frontend_menu` run the `webolution` CLI, whose `main()` installs the default clone save
  store (LocalAppData). Static reading says save writes occur only on level completion or New Game, which those
  scripts do not drive. Concurrent reads are harmless; I did not trace every CLI path.
- Assertion depth of the measurement/census drivers was not audited test by test.
- `<TMP>`: the verifier does not set TMP/TEMP; fixed names assume no unrelated process uses `webolution*` there.
- CTest `DEPENDS` is order-only: it is ignored when a subset is selected (`-R`, `-I`), and a failed
  predecessor does not skip the dependent.
- Two CTest sessions at once over any build dirs (main, a dev preset, another worktree) would collide on
  `build/<task>/...` and `<TMP>` fixed names: these roots belong to `CMAKE_SOURCE_DIR`, not to the binary dir.
  Blocked by procedure: run exactly one ctest at a time.
- Repeated full runs add a run directory each to `build/targeted-play-001/runs` and
  `build/playable-expansion-001/runs` (kept by design, never deleted). `findSourceSave` picks the newest
  FNV-matching run, so its recorded path differs between runs while the copied content is FNV-identical.

## Proposed later benchmark (coordinator, after the loop exits; not run, not validated)

Prerequisites: idle runners (no supervisor loop, no other ctest/ninja), clean tree except the intended
content, `RepositoryLease` held, normal save `%LOCALAPPDATA%/LemmingsWebolution/SavedGame.dat` hash recorded
before and after, `build/{targeted-play-001,playable-expansion-001}/runs` listing recorded before each run.
Same content for all runs; `--jobs 12` throughout (compile is already incremental after the first).

```
python tools/agent_control/verify.py --profile all --jobs 12 --test-jobs 1   # serial reference
python tools/agent_control/verify.py --profile all --jobs 12 --test-jobs 4   # four jobs
python tools/agent_control/verify.py --profile all --jobs 12 --test-jobs 4   # repeat four jobs
```

`--test-jobs` already exists in `verify.py`; no controller edit is required. After each run, copy before the
next one overwrites them: `build/agent-native/{main,recovery}/agent-results.xml`,
`agent-test-inventory.json`, `Testing/Temporary/LastTest.log`, and the console log. Compare:
(1) test-name sets equal (148 main, 84 recovery) and every test Passed; (2) per-test seconds from the junit
file, with the heavy groups listed first; (3) artifact checks on deterministic outputs only:
`build/scene-*-001/*/hashes.txt`, `levels.txt`, `build/fluid-surface-impl-001/*.png` hashes,
measurement dumps, and the outcome lines (`RESULT:`, `outcome`, `blocked`, `anomaly`) of the observation
drivers' `play-result.txt`/`restart-result.*`, not their wall-time columns; (4) normal save hash unchanged.
Report `Total Test time (real)` per profile. A difference in an observation driver's outcome between serial
and four jobs is a contention finding; mark those groups for `PROCESSORS`/`RUN_SERIAL`, do not average it away.

## Checks

- Static audit only (above). Whole-suite serial verification is pending the supervisor run after this task exits;
  no verifier was started by this worker (a full main run exceeds the 600 s client limit).
- Expected after the edits: 148 main / 84 recovery tests, identical names, one added DEPENDS group.
  The supervisor's `agent-test-inventory.json` will confirm.
- Nothing in this report claims a parallel run happened.
