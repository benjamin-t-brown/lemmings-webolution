# Restore the A1 camera fixtures for `actorcompose`

The smoke run failed five times on missing `build/tmp/a1-zoom-elevation/params-*.json`
after the scratch directory wipe. This is a test-input failure on installed level
`MM_CLIMB_002` (Just Climb, Playtest 7). The `kViews` entries now point to five
JSON files under `tools/harness/fixtures/camera_a1/`. The test calls
`renderRoute` → `loadCameraParams` → `buildCamera` → `TubeScene::render`, then
hashes the encoded empty-list PNG. These paths are test-only, so the Playtest 7
human win and other recordings have no affected production route to replay.
[Failed smoke log](../../build/agent-runs/20261120T062837-16728-3afe8bb1/verification.log#L230), [Playtest 7](../playtest/2026-11-15-playtest-7.md).

## Fixture evidence

| Fields | Values and source |
| --- | --- |
| `viewport`, `center`, `level_eye_height` | `[640, 480]`, `[319, 239]`, `96` from the retained `MM_CLIMB_002` state, shared fixture and A1 viewport note. Center globals are `0x0058D294` and `0x0058D298`. |
| `aspect`, `angle_degrees` | `1.3333333730697632` and `19.029999375343323`. The A1 lens record gives invariant captured aspect and turn parameter. The latter is `0.052861109375953674` at camera `+0x118`, multiplied by `360.0` so `Camera.cpp` divides back to the same turn value. Aspect is camera `+0x120`. |
| `yaw_units` | `32.0` from captured canonical control and the shared fixture. The five cases have no horizontal input. Retaining yaw across the zoom and vertical-only cases is supported by the A1 control mapping and verified by the hashes. |
| `yaw_offset`, `yaw_units_per_turn`, `height_bias`, `eye_distance`, `fixed_pitch_turns` | `128.0`, `256.0`, `17.392`, `377.516`, `-0.03135770272046` from the shared fixture and the documented operands at `0x004B1350`, `0x004B07A8`, `0x004B1808`, `0x004B1098`, and `0x0045CF83/88`. |
| `pose_pi`, `lens_pi`, `angle_units_per_turn` | `3.1415927410125732` for each PI field and `360.0` for the divisor from the shared fixture and separate operands at `0x004B0F88`, `0x004B1AB0` and `0x004B1348`. |
| `zoom_shift`, `elevation` | The A1 per-case table gives `canonical` `(0, 0.0)`, `zoom1` `(1, 18.5)`, `elev_up` `(1, 47.5)`, `elev_down` `(1, -10.5)` and `elev0_neg` `(0, 0.0)`. |

[Shared fixture](../../tools/harness/fixtures/camera_a1/camera-fixture-params.json), [retained state](../../tools/harness/fixtures/camera_a1/reference-state.json), [A1 operands](phase-a1-camera.md), [A1 control mapping](phase-a1-control-mapping.md), [A1 zoom and elevation](phase-a1-zoom-elevation.md), [A7q lens decode](phase-a7-second-level-mask.md), [camera lessons](../lessons/camera-tube-sprites.md), [task archive](../tasks-archive.json), [facilitator history](../FACILITATOR_HISTORY.md#L201).

Static Ghidra analysis of the registered EXE (`32675bdb49866e76502713251fdfd2b09078c2481c72ca22ff68ebf4b3737a2f`)
cross-checked the A1 pseudocode against the zoom shift and division at
`0x45CC61`–`0x45CC73`, the nonzero-zoom elevation branch at `0x45CF30`–`0x45CF4A`
and the lens setter at `0x47B042`. `0x47B061` builds the lens from camera fields.
The original zoom writer and exact x87 rounding remain open A1 questions.
The restored hashes establish fixture equivalence for the clone’s empty render.
[Static evidence](../../build/actorcompose-fixtures-001/rea-static.json).

## Checks and remaining smoke inputs

The targeted build and CTest run passed. All nine unchanged golden PNG hashes
matched, with **311 checks and zero failures**. A byte comparison against the
archived Git blob confirmed that the test source changed only at the five paths.
[Build log](../../build/actorcompose-fixtures-001/actorcompose-build.log), [CTest log](../../build/actorcompose-fixtures-001/actorcompose-ctest.log).

- `levelsweep_report` requires the untracked `result.txt` records that its 17 CTest chunk dependencies generate. `menurender` requires the registered EXE in the installed-data run. The configured retail archive consumers and the optional `savedgame` EXE check are listed in the audit. [All 95 smoke tests](../../build/actorcompose-fixtures-001/smoke-input-audit.md).
- `skillpanel` requires its scratch stream only with `WEBOLUTION_C1BC_STREAM_TEST` set. The remaining `actorcompose`, `actorpose` and `collisiongrid` scratch reads are optional diagnostics. `retaillevel` writes its scratch sweep. The standard gate has no remaining required prior `build/tmp` fixture. [Source scan](../../build/actorcompose-fixtures-001/smoke-input-source-scan.txt).

Handoff — ready for review. The five fixture files are commit-ready in the working tree. Staging requires a writable Git index.  
Handoff — the supervisor’s smoke profile is pending.

What to check — `MM_CLIMB_002` (Just Climb). Run the `actorcompose` smoke case with the installed assets and an empty `build/tmp`. Expect all nine empty-list hashes to match.
