# fluid-surface-impl-001: visible linked fluid on installed MM_W_012

Milestone step: **render integration** of the user-approved three-stage visible-fluid milestone.
Installed level `MM_W_012`, lever `(183,60)`, serialized receivers ids 5 / 6 (stream indices 12 / 11).
Source-built clone only; the original executable, debugger, hooks and capture were never used.
Static REA was not needed: the accepted research contract (`fluid-surface-research-001.md` sections
6.2, 7.2-7.4 and the retained `gate-{records,meshes}-MM_W_012.txt` measurements) was sufficient.

**Status of this report (read first).** Both workers of this task (attempt 1 and the retry that wrote
Section 7's "Retry" part) had Bash limited to the verifier commands and one `git show`; neither could
build or run a single target, and the verifier has no per-test filter, so the code and the registered test
were written and corrected by reading only. Attempt 1 was verified by the supervisor
(`build/agent-runs/20261112T182432-7804-97d53813/verification.log`): everything compiled, 143 of 144 tests
passed, and `fluidsurface_impl` failed with 11 check failures. Section 7 lists each failure, its diagnosis
and the retry's correction. The retry's corrections have **not** been executed by the retry worker; they are
pending the next supervisor run. Nothing below is claimed as passing unless Section 7 says it ran.

## 1. Failure and why this is the next dependency

Source-traced baseline (research 7.0/9): `BackdropScene::build` extracted only `entities_425e50` /
`entities_426c6b` models, the parser and `buildSceneMeshSource` rejected mesh kind 4, and no receiver
model was a scene root. The accepted gate height / zone-mask motion therefore produced **no fluid
pixels** at any height. Retained "missing surface" baseline images are produced by the registered test
(`baseline-no-fluid.png`: the production scene with no live fluid state, i.e. the pre-task output).

## 2. What changed (production call path)

| Contract step (research 6.3 / 7.4) | File | Change |
| --- | --- | --- |
| A kind-4 parse | `src/assets/SceneNode.cpp` | kind 4: serialized stride 96, 3 corners, shared RGBA read before the corners (like kind 6). |
| A' mesh adapter | `src/app/SceneMeshSource.cpp` | admits kind 4 (its independent 5/6/7 guard). |
| B facing predicate | `src/app/BackdropScene.cpp` | `passesKind5` now covers kinds 4 and 5 (shared predicate). |
| C roots + stage | `BackdropScene.{h,cpp}` | `Stage::FluidGate`; `fluidRoots_` kept apart from `roots_` (existing root accounting and fail-closed extraction unchanged); `FluidRecord` (pos_off, amp, `[+0x170]`, model+0xF8 bias) read from `Entity42acac`; `applyFluidState` (vertices 2..5, six top colours, normal sign); `renderFluid` (identity root matrix, flat null-material, SRCALPHA/INVSRCALPHA from the material selector, bias = `double_bits[2]` through `adaptPackedSceneMeshVertex`'s native-bias input). |
| D live state | `src/sim/World.{h,cpp}`, `RetailLevel.cpp` | `FluidGate::{lane, wavePhase, animated}`; `step()` = unchanged `stepHeight()` + `advanceWave(dt)` (`phase += dt` wrapped at 2.0, **also when request == done / dt == 0**, original `0x42B29E` jumps to this tail). No new clock; height/mask recurrence untouched. |
| E submission | `src/app/SceneFrame.h`, `src/app/main.cpp` | five-stage `scheduleSceneFrame` overload (primary, legacy, tube, **fluid**, secondary); `renderFrame` feeds `world->setup().gates` each frame. The four-stage form is unchanged. |

`BackdropScene::render()` (used by older tests) draws the fluid stage only when a live state was
supplied, so every pre-existing caller is unchanged.

Admission is structural, never by level ID or coordinates: a receiver is admitted when it is kind 0 (the
measured wave constants), its base model is an inline type-0 object, the first type-1 node has >= 6
vertices, >= 3 normals and a kind-5 block[1] whose corners reference vertex slots 2..5. A receiver that
fails is omitted with a `FluidGate` diagnostic and **never** empties the other scene roles. Kind-1
receivers, the `+0x15C` additive overlay, `+0x164` pipes and the `^water_glow` emitters are the explicit
omissions of research 6.4 (no installed MM_W_012 receiver is kind 1; all 19 are kind 0).

Animation, as implemented (research 6.2, constants measured for kind 0): `x1 = (lane&3)/4 + phase/2`,
`x2 = ((lane+1)&3)/4 + phase/2`; `w = 0.5*cos(2*pi*x)` with the traced float literal
`pi = 3.1415927410125732`; `w = 0` at `h == 0`, clamped to `-h`; vertex 2 and 4 use lane 1, 3 and 5 lane 2;
`col = ((cos(2*pi*(x-0.25))+1)/2)^2` (0 at h == 0), `c = v170 + (255 - v170)*col`; normals 0..2 are negated
whenever `h == 0`; until the first update the stored deserialized mesh is drawn.

## 3. Provisional depth policy (isolated, unverified)

`BackdropScene::FluidDepthPolicy` with `kProvisionalFluidDepthPolicy = LessWriteOn` (the clone's existing
mesh default: `SceneDepthFunction::Less`, depth write on). **This is not original evidence.** The original's
inherited ZENABLE / ZWRITEENABLE / ZFUNC / device ZBIAS remain unresolved (research 7.1). The traced
**software** projection bias (model+0xF8 = 0.00015, subtracted from native Z by `0x46DE50`) is passed
separately and is not an API ZBIAS. `LessWriteOff` and `DepthDisabled` exist only for focused diagnostics
(`setFluidDepthPolicy`); production never selects them. The test records matched-time images of all three
at the settled stage (`settled-depth-*.png`). Changing the default later must be documented as a revision.

## 4. Registered checks (CTest `fluidsurface_impl`, `src/__test__/test_fluidsurface_impl.cpp`)

* UNIT (labelled): `applyFluidState` against literal values from the retained measurement dump (gate 12,
  lane 20, h 24.5: Y `(29, 28.5)`, top colours `(63.75,63.75,207.75)` / `(255,255,255)`), h = 0 collapse,
  normal negation, raw-until-first-update, half-period phase.
* REAL: all 19 receivers admitted in stream order; ids 5/6 = indices 12/11.
* Stable camera chosen by actual raster stores of the settled receiver pair (production camera params,
  yaw search 0..252 step 4), then one camera for every stage.
* Stages through the production scene and shared-dt World: baseline (no fluid state), initial (12 updates,
  before contact; drained receivers draw **nothing**, as research 7.2 predicts), rising at 3.0 s and 7.0 s,
  settled at 17.0 s, repeat pulls after settle, pause (repeatable frame), reload (byte-identical to the
  initial frame at identical inputs), fast-forward (16 ms vs 100 ms segmentation give identical phase and
  heights; 5.0 s wraps to phase 1.0).
* Matched-time control: a second fresh world with the same dt history and no pull. Control draws no
  surface for id 5; activated ids 5/6 submit triangles and write pixels at the live height; receivers whose
  live state is equal in both worlds submit identically; every differing pixel lies inside the
  independently projected footprint of the receivers whose state differs.
* Depth diagnostic at matched settled time (three policies, pixel counts and diff counts recorded).
* Unrelated scene regression: `MM_CLIMB_002` backdrop output is unchanged by fluid state.
* PUBLIC: `runApplication(--level MM_W_012 --sim --native --camera build/fluid-surface-impl-001/public-camera.json)`
  observed through the per-frame view, A/B against `--scene-off`: live gate state (ids 5/6 drained and
  animated, ids 10/11/12 = stream indices 8..10 loaded raised 22.5) and at least half of the pixels written
  by the fluid stage at each frame's own pose differ from the scene-off run. The camera file carries the
  same stable camera as the staged shots (the standalone default camera does not see the raised receivers,
  see Section 7).

Existing assertions were not weakened. One fixture value changed: `test_scenemeshsource.cpp` used mesh
**kind 4** as its "unsupported hand-built kind" example; kind 4 is now a supported kind by contract, so the
fixture uses kind 3. The assertion text and expectation (an unsupported kind is an error) are unchanged.

## 5. Reproducible clone steps (next stage)

```
python tools/agent_control/verify.py --profile main      # builds + runs CTest incl. fluidsurface_impl
# evidence: build/fluid-surface-impl-001/{observations.txt,*.png}
# manual, ordinary route (SDL window or headless):
webolution --game-dir "<Lemmings Revolution dir>" --level MM_W_012 --sim --native \
           --frames 120 --window-shot build/fluid-surface-impl-001/app-frame.png
```
The final assessment should walk a lemming to lever (183,60) through ordinary input and compare the
frame before/after; expect the receivers at x 152..168 to rise 0 -> 24.5 over ~12.25 s of dt, with the
two other levers' receivers unchanged.

## 6. Residual limits (not hidden)

* Original depth state is unresolved; the default policy is provisional and plausible images do not settle it.
* The `+0x15C` additive overlay, `+0x164` pipes and spray emitters are not drawn; kind-1 receivers are not
  drawn (diagnosed). The colour channel lane assignment is by vertex slot (2,4 lane 1; 3,5 lane 2), which is
  equivalent to the traced corner mapping on the installed layout.
* Kind 4 is now parsed everywhere `readSceneNode` is used; no installed level other than the receiver
  models contains it (all 102 levels were admitted before this task).
* The pre-loop diagnostic frame of `main.cpp` uses the same snapshot (raw stored mesh before any update).
* Paused frames do not advance the phase in the clone (the original runs an update(0)); the phase is
  unchanged either way because dt = 0.
* Natural lever reachability, original comparison and full fidelity are not established here.

## 7. Verification state

### Attempt 1 result (supervisor, `build/agent-runs/20261112T182432-7804-97d53813/verification.log`)

Compilation of all 736 steps succeeded; 143/144 tests passed; `fluidsurface_impl` failed (11 checks of 93).
Retained output from that run: `build/fluid-surface-impl-001/observations.txt` and the PNGs. Measured there
(source-built clone, camera yaw 160, eye height 96 selected by raster stores of the settled pair):

* initial: 10062 fluid pixels changed vs the no-fluid baseline, all attributable to accepted stores of the loaded-raised
  receivers; ids 5/6 (h = 0) draw nothing. This passed.
* rising-a (h 6.032), rising-b (h 14.032), settled (h 24.5): 3350 / 6527 / 10749 pixels differ from the matched-time
  control; footprint, identical-state and phase checks passed. Reload byte-identity, pause, fast-forward phase and
  the MM_CLIMB_002 regression passed.
* depth diagnostic, settled, matched time: fluid pixels changed LessWriteOn = LessWriteOff = 20730 (identical
  images, 0 differing pixels: nothing later in the fluid stage re-tests against fluid depth in this view);
  DepthDisabled = 21033, i.e. **303 pixels are hidden by terrain under Less** in this view. This is plausibility
  evidence about the clone's own depth choice, not original depth fidelity.

Failures and diagnoses (all in the new test's expectations; none was a production-code defect found by reading):

| Failed check | Diagnosis | Retry correction |
| --- | --- | --- |
| UNIT lane-1/lane-2 colours | tolerance 1e-6 was too tight: the traced f32 pi makes `cos(2*pi*-0.25)` = -4.4e-8, so the 0.25 lane weight lands about 5.6e-6 below 63.75 | tolerance 1e-3 colour units (sub-quantum), reason commented in the test |
| default policy is LessWriteOn | `shoot()` leaves the last diagnostic policy (DepthDisabled) on the shared scene object before the check | default is checked right after build; the diagnostic block restores the default |
| reload / PUBLIC "gate id 8 at 22.5" (8 checks) | mislabel: the research's "gates 8..10" are **stream indices** 8..10. By id they are 12/11/10 (high 22.5); id 8 is stream index 6 with high 24.5 (`gate-records-MM_W_012.txt`) | checks ids 10/11/12 |
| PUBLIC: half of fluid-stage pixels differ from `--scene-off` | the ordinary standalone default camera (yaw 32, eye 96) sees the raised receivers at only 1930 mask pixels over 8 frames and 0 of them differ from the scene-off run. The cause was not measured; my inference (unverified) is terrain depth occlusion and/or HUD overpaint at those few pixels, since the staged shots used a different camera and drew 10062 pixels. `renderFrame` feeds the same snapshot as the staged shots, so I do not infer a missing draw call | both public runs get the stable camera through the existing `--camera` file |

### Retry (this worker): nothing was built or run

The corrections above are source edits to `src/__test__/test_fluidsurface_impl.cpp` only; no production file changed
in the retry. They are **unexecuted** and pending the supervisor's next `main` run. If the PUBLIC pixel check still
fails with the stable camera, the next dependency is a per-frame diagnostic from `renderFrame` (fluid submissions and
pixels in the stderr line) rather than loosening the 50 % threshold.

### Attempt 1 worker's own observations (historical)

Pending supervisor execution unless stated.

**Observed by this worker (one `python tools/agent_control/verify.py --profile main` run, client-backgrounded at
the 600 s limit; output `C:\Users\bbrown\AppData\Local\Temp\claude\C--progs-lemmings-webolution\42a746e9-910b-4d84-8435-603d4241cf83\tasks\b3rnnyp0r.output`):**

* The full `main` CMake build of 736 steps completed **with no compile or link error**, including the edited
  `World.cpp`, `RetailLevel.cpp`, `SceneNode.cpp`, `SceneMeshSource.cpp`, `BackdropScene.cpp`, `main.cpp`
  (all targets that compile them) and the new `test_fluidsurface_impl`, which linked. The only diagnostic from
  the new test is `-Wmissing-field-initializers` for the designated `ApplicationRunOptions{.frameSeconds}`
  (same pattern as the existing `test_teleportapp`).
* CTest (144 tests, `--parallel 1`) had passed tests #1..#34 when this worker stopped waiting; it was at
  `targetedplay_play_1` (a multi-minute test). **`fluidsurface_impl` had not run, so no image, observation file
  or assertion result of this task was seen by this worker.** Per instructions the suite was not restarted.

**Not observed (all pending the supervisor's run):** `fluidsurface_impl` pass/fail; the baseline / initial /
rising / settled / reload / depth-diagnostic PNGs and `observations.txt` under `build/fluid-surface-impl-001/`;
the existing suites' results with the production edits (`levergates`, `leverinitface`, `backdropscene`,
`scenemeshsource`, `sceneintegration_*`, `sceneadmission_*`, `scenenode`, `retaillevel`, `world`); the recovery
profile. If `fluidsurface_impl` fails, its message names the failing expectation; the most likely
environment-dependent expectations are (a) a camera yaw that sees the settled receiver pair
(`bestScore > 0`), (b) `maskTotal > 0` in the public-route frames, and (c) the footprint/identical-state
checks if the injected walker touches other levers (those receivers are then listed, not assumed unchanged).
Treat such a failure as a finding about visibility or the test's assumptions, not as a reason to weaken the
check without a documented revision.

## 8. Handoff for the next stage (`fluid-lever-assessment-001`)

1. Read `build/fluid-surface-impl-001/observations.txt` and the PNGs first; confirm `fluidsurface_impl` passed in
   the supervisor's verification log before trusting this report's claims.
2. Expectations from the research contract: ids 5/6 draw **nothing** until the lever is pulled (drained,
   `h = 0`, normals negated); receivers at stream indices 8..10 (ids 12/11/10, loaded raised, 22.5) show water from
   the start; use `--camera build/fluid-surface-impl-001/public-camera.json` to see them (the default view does not); after the pull
   ids 5/6 rise 0 -> 24.5 at 2.0 per dt-second (about 12.25 s) with the top edge at `4 + h + w`.
3. Assess through ordinary input (walk a lemming to lever (183,60) with the real controls); the synthetic
   walker used here is a labelled fixture only.
4. Terrain/front-back occlusion: compare `settled-depth-less-write-on.png` with the two diagnostic policies;
   any change of the provisional default must be a documented revision, and images remain plausibility
   evidence, not original-fidelity proof.
5. Open items carried forward: original depth state; `+0x15C` overlay, `+0x164` pipes, spray emitters;
   kind-1 receivers.
