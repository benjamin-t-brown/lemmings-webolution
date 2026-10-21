# A6d23: paired native transform and compiled production screen Y

2026-10-20. OpenCode targeted `round-agent`, requested configured Luna/high per
[roles](../ROLES.md); Task session `ses_ea4856736ffev3CmVwYrwzv2Xv`. Task does not
independently expose the actual execution model. Brief
`build/tmp/a6d23-native-camera-pair-brief.md`, missing-evidence repair
`build/tmp/a6d23-native-camera-pair-repair1.md`. One stock start, no production edits.

## Result and scope

**MEASURED + offline comparison:** three walkers at a fresh `MM_CLIMB_002` pause,
canonical A1 zero-zoom camera, have cached screen Y within **1e-4 pixels** of the
unchanged compiled production `buildCamera` / `actorProjectionInput` /
`actorSpritePlacement` path on their captured model triples. Maximum absolute
error is **8.963683444562776e-05 px**. This threshold was recorded before the run.

| Actor | Native cached screen Y | Compiled production screen Y | Difference px |
| --- | ---: | ---: | ---: |
| `0x27D38E0` | 300.3686498472712 | 300.36856194197145 | -0.00008790529972202421 |
| `0x27D3150` | 310.25443079194247 | 310.254341155108 | -0.00008963683444562776 |
| `0x27D4870` | 291.39344352522187 | 291.3933629663713 | -0.0000805588505841115 |

**Not pixel acceptance:** no screenshot was acquired. These are not the A6d3 actor
positions or pause. Both snapshots (50 ms apart) agree byte-for-byte, including
level, UI pause, camera, matrix and actor rows; that is not an atomic observation
of an actor draw. No claim that the sampled global matrix was consumed by each
draw, no one-pixel cause diagnosis, and no A6 gate pass. The A6d3 slope-box failure
and unscored full-frame gate remain. Re-park this lead; no automatic further round.

## Raw inputs and original code

Capture `build/tmp/a6d23-native-camera-pair/capture_pair.json`, SHA-256
`f6fe6f0d2611385b9e9ccc838e3ac72f33c7f8b1eca0bab670b8951f4e1266ca`.
Stock authority digest matches [lessons](../LESSONS.md). Owned PID 37456;
loaded path `Compiled\MM_CLIMB_002.lvl`, `ui_state=1`, yaw 32, zoom 0,
elevation 0, center `(319,239)`. The 360-byte camera block hash is
`7e6e1a8a77c29f860f82dd8cde3f60c9f89ecf3f1352d3c2a6801cc9867d0fff`, the
canonical retained A1 camera. Other production camera parameters come from
`tools/harness/fixtures/camera_a1/camera-fixture-params.json`.

The 96-byte `0x4C6B88` record, twelve little-endian doubles, is:

```text
000000a0f05fdebf000000e0afc8b73f00000020544d353f0000000000000000000000000b10e53f00000060e30d18bf00000040f15fdebf00000060afc8b7bf000000a0534d35bf000000000000000000000020bf0839c000000080678bc73f
```

Hash `692c60eae75971b60b39cdfad09223274473292aac8e560cb5a78716362933e0`.
Root model `+0x108` is inline render-subobject `(root+0x78)+0x90`, not a
dereferenced pointer. All roots have vtable `0x4B10E8`; embedded render vtable
is `0x4B14A8`. Rows below retain model and cached triple raw hex independently
of ignored scratch artifacts. Cache is at `(root+0x78)+0xB8`, root `+0x130`.

| Actor | Model XYZ hex (3 f64) | Cache B8/C0/C8 hex (3 f64) |
| --- | --- | --- |
| `0x27D38E0` | `00000040b67f4cc00000000000004040000000605c4e1840` | `00000080e12a384000000060b7a923c0000000008d81c43f` |
| `0x27D3150` | `000000c0aaa449c00000000000004040000000a0d06e3940` | `000000405f8a284000000040c83226c00000008037f0c33f` |
| `0x27D4870` | `000000608c8a43c000000000ab5e4240000000608c8a4340` | `000000000000d8be000000c0483a20c0000000e091d2c33f` |

**READ, independently reread:** `0x40EB50` multiplies columns at offsets
`(0,0x18,0x30)`, `(8,0x20,0x38)`, `(0x10,0x28,0x40)` by XYZ. `0x40EBF0`
returns translation `matrix+0x48` (`0x40EBFA 83c048`), added by `0x40EAE0`.
`0x43CC71 68886b4c00` supplies this matrix; `0x43CC89 e8521efdff` calls
the transform into render `+0xB8` from render `+0x90`.

Raw affine depth exceeds cached C8 by `0.0003000000410978698`,
`0.0003000031249522084`, `0.0003000087521898287`. The worker left the reason
unsettled. **Facilitator correction:** `0x43CCB5 dc2598144b00` subtracts the
double `0x4B1498 613255302aa9333f` = 0.0003; `0x43CCC4 81c1b0000000`
passes `render+0xB0` as this to `0x43CCCA e8d1000000` (`0x43CDA0`). Stores
`0x43CDAD 894818` / `0x43CDB3 89501c` write its argument at this `+0x18`,
which **aliases render `+0xC8`**. The subsequent divisor really is this
biased cache. `0x43CD30 dee9` subtracts its Y quotient from screen center.
Thus the pure-affine-cache prediction was false; omitting the existing bias
is not a new discrepancy or a reason to fit another offset. Exact native
intermediate rounding and draw-time correlation are still outside this check.

## Facilitator check

Independently decoded all raw triples and both snapshots, verified their hashes,
identity/level/camera/center and 20 executable byte pins, and reread the complete
`0x40EAE0`, `0x40EB50`, `0x40EBE0`, `0x40EBF0`, `0x43CC68`, `0x43CDA0` bodies
using `read_steppers.py`. A separate compiled C++ probe calls unchanged production
helpers; it does not reuse the worker's Python camera transcription. Results
above reproduce its headline (last-bit differences on two rows are retained).

Scratch probe/checker/report:
`build/tmp/a6d23-native-camera-pair/facilitator_probe.cpp`,
`facilitator_verify.py`, `facilitator-verified.json`. After a full build:

```powershell
.\scripts\Invoke-Ucrt64.ps1 "g++ -std=c++23 -O0 -Isrc build/tmp/a6d23-native-camera-pair/facilitator_probe.cpp src/app/Camera.cpp src/app/ActorSprite.cpp build/cmake/dev-debug/CMakeFiles/webolution_objects.dir/src/assets/*.obj build/cmake/dev-debug/CMakeFiles/webolution_objects.dir/src/sim/Terrain.cpp.obj -o build/tmp/a6d23-native-camera-pair/facilitator_probe.exe"
python build/tmp/a6d23-native-camera-pair/facilitator_verify.py
```

Two earlier scratch link attempts failed on missing AssetLibrary/Terrain
dependencies; neither modified production. Separate single-term in-memory
diagnostics fail the frozen comparison: `originY-1` gives maximum error
`1.0000896368344456` px; canceling the depth bias gives
`0.13705720737249294` px. Restored baseline is identical; Camera.cpp and
ActorSprite.cpp hashes unchanged (`d7afd279...` and `977bda40...`). These are
projection falsifiers, **not pixel mutations or passing screenshot gates**.
Fresh full build and **24/24 CTest pass**.

Initial worker return only found missing fields in old captures and was rejected
as incomplete, not accepted as a transform negative. Repair preserved that initial
note verbatim and performed the one authorized start. No further start was used.

## Live cleanup and audit narrowing

- Current executable/save/config hashes independently equal capture before/after;
  no dgVoodoo.conf change. Own final process and `LemWeb` task checks return no rows.
  Worker intentionally terminated its owned game; observed exit 1. No hooks/debugger.
- Correct the worker's **128-byte** label: the slot record is **280 bytes** at
  `0x4E35D4`. Its four retained hex fields are identical, including actual post-write
  readback.
- The script replaces `entry.write_mem`, but `select_level` is imported from
  `goto_level` and uses that module's writer. Therefore the one logged write is
  the **restoration call**, not the initial selection call. Do not call this a
  complete raw write ledger. Source review shows `select_level` returns the saved
  original record and the entry `finally` passes that to the audited restoration;
  recorded pre-restore bytes equal that original and the readback. This supports
  exact slot restoration, but no separate initial-write event is retained.
- `restored_hex` is copied from that retained readback after process termination,
  not another post-termination live read. The raw readback during restoration is
  the evidence. No actor/terrain writes or camera write appear in the source path.
- No screenshot; no full-frame error or actor-box error measured. Production
  injection and A6 gate remain unchanged. No commit authorization assumed.
