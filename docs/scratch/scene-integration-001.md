# scene-integration-001: structural scenery for MM_BASH_002

2026-10-31 UTC. Status: **needs_review**; separate read-only acceptance is
required. This worker does not update the queue. No original executable,
copied original instructions, oracle, hook, debugger, capture, launcher or
installation write was used.

## Runtime change and evidence boundary

Milestone step: select an installed level and render its scenery during play.
The concrete improved route is `runApplication --level MM_BASH_002 --sim`.
Before, `BackdropScene::build` returned a successful empty scene for that ID.
This was a source-traced defect; the authentic frozen loader now also
demonstrates that behavior on the installed bytes. After, paired scenery and
supported hatch/exit inline models are extracted by serialized ownership and
rendered through the existing production adapters and shared tube schedule.

The demonstrated admission scope is **MM_CLIMB_002 and MM_BASH_002**. Other
IDs retain empty scenes with explicit partial/undemonstrated diagnostics.
Unsupported present forms in the admitted parser route produce errors;
camera-dependent adapter failures and missing resources produce diagnostics.
An absent secondary holder or optional entity model is a valid absence.

Retained static authority is the accepted [scene-roots-001](scene-roots-001.md)
research and its `accept` review in `agent/tasks.json`. Addresses refer to
repository-local `Lemmings Revolution/Lemmings Revolution.exe`, SHA-256
`32675bdb49866e76502713251fdfd2b09078c2481c72ca22ff68ebf4b3737a2f`.
The verifier checks that identity by reading bytes only. Static load selection:
`0x004432C1 -> 0x00439FD3 -> 0x00437D48 -> 0x004749BF -> 0x0046BCA4`.
The holder reads its complete recursive typed tree, two flags and optional
auxiliary list before the paired wrapper reads the secondary-presence byte.
The typed hatch/exit readers are `0x00425E50` / `0x00426C6B`; model factory
`0x00465924` selects inline model0 reader `0x00450F57` and its object holder.

The retained stage associations are primary slot+4 before the tube
(`0x00441879`), tube (`0x00441884`), and secondary slot+8 afterward
(`0x0044199B`). `0x0046B73F..0x0046BADF` establishes the distinct secondary
mixed-child order. Entrance/Exit remain **LegacyUnattributed**, preserving the
existing pre-tube software placement. Their native entity admission, model
depth bias and animation effects are not newly implemented or inferred.
No new disassembly/export campaign was needed.

## Implementation and production call path

`runApplication -> runApplicationLeg` in unchanged `src/app/main.cpp` resolves
`COMPILED\MM_BASH_002.LVL` through `levelFileNames` and `AssetLibrary::read`.
It builds the tube and calls `BackdropScene::build`. That function calls the
already-linked `lemmings::read_compiled_level` on the live caller-owned byte
storage. Only its documented `invalid_argument` is translated into the public
expected-error interface; allocation or other unrelated exceptions are not
hidden. The composed reader already supplies section/cursor error context.

`ObjectSourceExtent` is portable source metadata, not an original memory
layout or allocator reconstruction. The paired and standalone holder readers
capture marker begin, typed-root begin, exclusive recursive-tree end and
exclusive holder end directly from `LevelByteStream.cursor`. Existing reads,
raw floating-point bits, booleans, counts, names, auxiliary bytes, ordering and
bounds remain intact. Primary holder end precedes the secondary-presence byte;
secondary end includes its complete auxiliary data. `LevelModel` carries inline
holder metadata through its existing `SerializedObject` without another parser.

The backdrop takes Primary and optional Secondary from `level.objects` and
LegacyUnattributed contributions from each present model0 `.model.object` in
`gadgets.entities_425e50`, then `entities_426c6b`, in collection order. Names do
not select roots. Every selected root is reread by `readSceneNode` at the
captured typed-root begin and must end exactly at the independently composed
reader's tree end. Owned roots retain stage, typed collection/entity index and
holder extent. Every former absolute-offset stage selection is replaced by
that ownership; billboard ancestry also carries the owning root. Offsets remain
observer and diagnostic identities. Secondary admission rejects nested/non-leaf
forms outside the retained direct-child domain.

`renderStage -> buildSceneMeshSource / makeSceneBillboardSource -> existing
kind5 predicate and vertex adapters -> rasterizeSceneTriangle` remains the
render path. `scheduleSceneFrame` still runs Primary, LegacyUnattributed, tube,
Secondary against the same RGB/depth storage without an intervening clear.
Matrix operations, material/blend selectors, texture policy, camera operands,
packing, raster arithmetic and mixed secondary dispatch are unchanged.
Mesh-source failures are checked at build and diagnosed at traversal;
billboard-adapter failures are diagnosed with level/role/source context.

An opt-in full-frame `SceneRasterObserver` mask measures accepted stores in the
source-built demonstration. `rasterCalls` increments immediately at the actual
raster invocation. Accepted-color/depth counters wrap the existing accepted
event callback, preserving primary source/triangle identities and callbacks.
They count actual assignments, including repeated stores to one pixel, not
unique changed pixels. Without an observer they remain zero. The existing
`pixels` depth-change counter is preserved and is not used as write proof.
Projection rejections and raster errors are separately counted; instrumentation
does not alter coverage, clipping, blending, depth or ordering.

## Authentic before state and changed files

Before production edits, the working `BackdropScene.cpp` and `.h` were copied
byte-for-byte into ignored `build/scene-integration-001/`; a matching `app/`
include path retains the original header. Both hashes equal supervisor entry
snapshot `build/agent-runs/20261031T033622-46024-3808fd4d/before-source-hashes.json`:

| Frozen source | SHA-256 |
| --- | --- |
| BackdropScene.cpp | `ae05c1e0fdf03ec7d9ae0bec406823a79f98182c60da32657539e2dc40ebb34c` |
| BackdropScene.h | `280f13e716d955f4a079f5fe0be1c2354d595fb7c74daef62f9a4f12d050de7a` |

`entry-identities.json`, `snapshot-comparison.json` and the copied supervisor
manifest retain that check. HEAD was not substituted for the dirty working
state. Frozen/current CTest executables compile the same demonstration driver
against their own loader/header, with shared source-built rendering support.
Production files are never swapped. New tests are bounded by 120-second failure
guards; all prior assertions and timeouts remain unchanged.
The complete after-side comparison depends on the retained ignored snapshot
and before-side artifacts. Keep this task directory for review/reproduction;
missing authentic before inputs cannot be replaced by current HEAD or treated
as a successful before/after comparison.

Worker changes:

| File | Change |
| --- | --- |
| `decomp-lemmings/include/lemmings/object_base_prefix.hpp` | portable holder source extents |
| `decomp-lemmings/src/object_base_prefix.cpp` | cursor capture at existing read boundaries |
| `src/app/BackdropScene.h` | owned-root provenance, diagnostics and opt-in write/call statistics |
| `src/app/BackdropScene.cpp` | structural extraction, role/ancestry partition, explicit unsupported/failure diagnostics |
| `src/__test__/test_sceneintegration.cpp` | installed frozen/current render, extent/negative checks, complete first-level comparison and public SDL route |
| `CMakeLists.txt` | two bounded demonstrations and shared source-built support, preserving existing registrations |
| this report | evidence, executed outcomes, limitations and next dependency |

Other retained files and all accepted save/audio changes are preserved. The
supervisor snapshot precedes its own assigned-task transition to `in_progress`:
`agent/tasks.json` has controller run ID `20261031T033622-46024-3808fd4d` and
mtime `03:36:22 UTC`, before worker activity. The worker never edits it. This
explains its hash difference from the entry snapshot, separately from the five
modified pre-existing source/build files. All other 1,510 snapshot entries were
unchanged at the preservation check. New test/report files are recorded in the
status delta; extensive pre-existing staged/unstaged/untracked work is retained.

## Installed boundaries and supported ownership inventory

The archive entries are read in place, without installation changes. Retained
research identifies MM_BASH_002 as 709,367 bytes, SHA-256
`65455693fa887f4474c66c2795526cb3b2bef2616449a5e1dc55debb9d7e5cd2`,
and MM_CLIMB_002 as 699,117 bytes, SHA-256
`95f84f86fb53ab29ce2ce3aed794cf69a06e5754d42ef78657f3af1a5db4c567`.
The demonstration reads those actual named installed entries and asserts the
accepted independent root positions/inventories; production has no such table.

MM_BASH_002 measured LVL-relative locations:

| Ownership | Marker | Typed root | Tree end | Holder end | Nodes / mesh records / billboards |
| --- | --- | --- | --- | --- | --- |
| Primary pair | `0x5BE41` | `0x5BE4E` | `0x93F78` | `0x93F7E` | 4 / 923 / 0 |
| Secondary pair | `0x93F7F` | `0x93F8C` | `0x9B54B` | `0x9F453` | 30 / 82 / 17 |
| Hatch0 `.model.object` | `0x9F4CE` | `0x9F4DB` | `0xA0335` | `0xA341B` | 4 / 16 / 0 |
| Exit0 `.model.object` | `0xA34DA` | `0xA34E7` | `0xAD24D` | `0xAD253` | 2 / 156 / 0 |

Totals: 40 nodes, 1,177 mesh records and 17 billboards. Scenery and models stay
separate. Inline holder ends are new source-reader measurements, not values
inferred from tree ends; the hatch also has substantial auxiliary consumption.
Each paired/inline holder is independently replayed through
`read_serialized_object` and compared with the composed reader's boundaries.
The complete secondary auxiliary serialized byte vector is retained identically
by both holder-reading paths.

## Executed before/after rendering

Both sides use the same fixed 640x480 source camera: center319/239, yaw32,
offset128, zoom0, eye height96, bias17.392, distance377.516 and the existing
CameraParams lens/pitch operands. The public application receives an explicit
numeric camera file. This isolates scenery from the next selected-hatch camera
orientation dependency; it is not a claim about ordinary MM_BASH_002 entry pose.

The frozen MM_BASH_002 loader executes its real empty-return branch: zero roots,
records, billboards and scene writes, black scene-only output and **zero**
composed scene-on/off changed pixels. Frozen first-level legacy/secondary
accepted-store zeros in its report mean **unobserved**: that authentic loader
only forwards the raster observer for Primary. Complete first-level buffers,
not those unobserved counters, establish preservation.

Current MM_BASH_002 measurements (full target mask; counts are store events):

| Stage | Mesh/billboard candidates | Actual raster calls | Scene-only color/depth stores | Composed color/depth stores | Predicate culls | Projection rejections / raster errors |
| --- | --- | --- | --- | --- | --- | --- |
| Primary scenery | 923 / 0 | 1,196 | 357,059 / 357,059 | 357,059 / 357,059 | 157 | 36 / 0 |
| Legacy Entrance/Exit | 172 / 0 | 334 | 14,461 / 14,461 | 14,461 / 14,461 | 2 | 0 / 0 |
| Secondary scenery | 82 / 17 | 170 | 53,105 / 53,105 | 32,372 / 32,372 | 0 | 28 / 0 |

The source-built production tube schedule invokes 752 tube triangles for
MM_BASH_002; accepted tube pixel events are 237,445 with scenery and 247,416
without it. Named-texture misses and tube missing-resource lists are zero on
this fixture. Primary+Secondary independently establish nonzero **scenery**
submissions and accepted writes; model writes are not used as that proof.
Composed scene-on/off RGB differs at **132,558 pixels**. Bounded accepted-event
samples retain source/triangle/pixel and before/after RGBA/depth values alongside
the counters. Successful raster returns and candidate callbacks are reported
separately from accepted events.

The public `runApplication --level MM_BASH_002 --sim --native --geom route
--camera <fixed-camera.json> --no-audio` scenario runs with SDL dummy video and
audio drivers, observes frame0, queues its own `SDL_QUIT`, and returns0 normally
after one observed frame. It repeats with `--scene-off`. Their composed public
frames differ at **130,282 pixels** (HUD composition explains why this is a
different count from the isolated scene/tube fixture). No map unlock, World
mutation, injected win or selected-hatch camera rule is introduced.

Artifacts under `build/scene-integration-001/{before,after}/` include each level's
`*-scene-only.png`, `*-scene-on.png`, `*-scene-off.png`, complete `.rgb` and
`.depth` buffers, and `measurements.txt`. The after directory also retains
`application-scene-on.png`, `application-scene-off.png` and the numeric camera
file. Source-built PNGs were visually inspected: the added-level composed view
contains scenery surrounding the tube and separately placed models. These are
clone renders, not original-game capture or a new native pixel comparison.

## Preservation and bounded supporting checks

The current first-level complete scene-only, composed scene-on and scene-off
RGB **and depth** files compare byte-for-byte equal to the authentic frozen
loader's output. Its Primary RGB/depth hash remains `0x1FD0BFB82AB39E95`.
All existing `backdropscene` assertions remain, including 923/0 Primary,
172/0 Legacy, 82/17 Secondary, 1,177 total records, 17 billboards, the 465
per-record kind5 decisions (306 pass), retained billboard values and mixed
secondary key/order expectations. Its existing check count remains24.

The new demonstration additionally checks cursor metadata for every paired and
inline holder, nonempty auxiliary byte preservation, absent secondary, absent
optional model, and truncated paired/standalone/inline flags or auxiliary data.
A one-byte prefix-name shift moves every root by one byte and retains ownership;
this supplements the real additional-level route. A model1 replacement produces
an explicit typed-model error; a serialized inline type3 produces a
LegacyUnattributed source-offset error. Actual installed MM_DIG_000 bytes,
supplied to the admitted adapter ID solely for this negative check, produce the
explicit Secondary type2 branch1 error. Calling with its real MM_DIG_000 ID
retains an explicitly undemonstrated empty partial scene. Neither test admits
that level or implements branch1. A malformed billboard matrix demonstrates
that the previously silent adapter failure appears in role/source diagnostics.

## Verification

Executed verbatim from the repository root, using registered UCRT64 MinGW-w64
GCC and the verifier's dummy SDL drivers:

```text
python tools/agent_control/verify.py --profile all
```

Final-source full run: **exit0**, main **80/80** (426.36 s), recovery **84/84**
(29.88 s), and `git diff --check` passed. Both JUnit files report zero failures,
disabled or skipped tests. Main preserves all previous 78 registrations and adds
only `sceneintegration_before` / `sceneintegration_after`; recovery preserves
all 84 unchanged registrations. The reference setting remains empty in recovery,
and `native_int_array` is its sole `native_`-prefixed source test. Reference EXE
identity was checked by reading bytes only. The exact registered compiler is
`C:/progs/msys2/ucrt64/bin/g++.exe` (x86_64-w64-mingw32).

Read back the executed JUnit and complete LastTest logs. The new demonstrations
passed in 1.78 s / 4.52 s. Existing `backdropscene` reports 24 checks/0 failures,
the exact stage census, 1,177 records, 17 billboards and its existing 278,820
changed-pixel scene fixture. Six complete frozen/current first-level buffer
comparisons (three modes times RGB/depth) pass. These are source-render checks,
not original-game agreement. Logs, inventories and measurements are copied to
`build/scene-integration-001/verification-final/`; `verification-final.json`,
`registration-preservation.json`, `source-preservation.json`, `status-delta.txt`
and `artifacts.txt` retain the final evidence and 41 rendered/data/report files.
The ignored task directory also contains a copy of this final report.

Two earlier full passes also succeeded: first 80/80 + 84/84 (444.06 s / 33.18 s),
then expanded checks 80/80 + 84/84 (430.34 s / 30.00 s). Their logs and measurements
remain in `verification-first/` and `verification-expanded/`. The first main
binary predates the detailed diagnostics, accepted-event samples and supporting
checks; the expanded binary predates the final guard preserving invalid raster
observers that lack accepted callbacks. The final pass includes that correction.
Submission/write counts and deterministic outputs remain unchanged across the
passes. No failing demonstration or denied command was hidden or retried.
All seven task text files were checked LF-only. A final documentation-only
`git diff --check` also exits0; it emits the pre-existing installation backup/
configuration CRLF warnings seen before this task. Those files were not
normalized because installation writes and unrelated changes are forbidden.

## Milestone limits and next action

The accepted startup/selection, original-layout clone save/restart wiring and
cue routing are retained (their latest accepted reports/reviews take priority
over older STATUS descriptions). This task adds source-built MM_BASH_002 loading
and rendering during the public simulated play route. Startup and first-level
selection have prior bounded source-built support; this task uses direct
selection for MM_BASH_002. Completion/progress/restart have the existing
source-built, partly injected-outcome support; natural MM_BASH_002 play through
finish and restart is **unverified**. Injected wins and passing source tests do
not establish natural complete gameplay, retail-save compatibility or native
fidelity.

Unknowns retained: selected-hatch orientation/entry for this level, original
per-frame admission/cache/device state, native entity/model timing and depth
bias, animation-time matrices, secondary tie/NaN order, general nested-secondary
behavior, branch1 billboards/other model forms, x87 equivalence, clipping,
clock/release/digger timing and full-frame fidelity. D1/D3/digger and
clipping/controller/capture parks, native-006 deferral, oracle-001, STATUS gates,
recovery FUNCTIONS/PROGRESS historical claims and the 372/6,186 baseline remain
unchanged. No completion criteria were relaxed.

Next action: independent read-only acceptance of this task. After acceptance,
the next ordered implementation dependency is **selected-hatch camera
orientation and entry initialization for other levels**, starting with the now
rendered MM_BASH_002 route. Teleport link IDs and evidenced commands remain later.
