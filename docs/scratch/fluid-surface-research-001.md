# fluid-surface-research-001: the installed moving-fluid render contract

**Revision 3 verdict (correction pass, 2026-11-12 UTC): BLOCKED. Section 7 supersedes
the implementation-ready verdict, initial/reload expectations and handoff in revision 2.
The installed mesh binding is retained. Effective alpha-blended depth-test/write/comparison
and device ZBIAS remain an essential unsupported input; existing clone defaults do not close it.
Do not advance to implementation. Earlier revisions below remain historical evidence.**

**Revision 2 verdict (attempt 2, 2026-11-11): prior blockers 1-3 CLOSED by static evidence plus the installed-record
measurement; an implementable slice is defined in section 6. Not blocked. Two items remain open and are labelled
non-essential in section 6.5. Sections 1-5 below are the attempt-1 report, retained unchanged; where section 6 differs,
section 6 is the correction.**

Attempt-1 verdict (superseded, retained): BLOCKED (essential evidence missing). Research is not accepted as enabling implementation.
Static work only (Ghidra via the project REA profile, `close_binary` called without a snapshot); the original was never run;
no production behaviour was edited. Reference SHA-256 `32675bdb49866e76502713251fdfd2b09078c2481c72ca22ff68ebf4b3737a2f`
(Ghidra 12.1.4). Retained evidence: `build/fluid-surface-research-001/static-trace.md` (full step/reader/constant-table trace)
and the registered driver `src/__test__/test_fluidsurface_measure.cpp` (CTest `fluidsurface_measure`).
Labels: DECOMP (pseudocode), BYTES (`read_bytes`), ASM (earlier accepted instruction reads), SOURCE (clone source read), INFER.

## 1. What is established

| # | Finding | Label / address |
| --- | --- | --- |
| 1 | The visible fluid is **not** the zone-mask rectangle. Each `entities_42acac` gate (vtable `0x4B1020`, ctor `0x428680`) owns child **models**: `[+0x158]` (the object reached through the base's `+0x44` -> `+0xB4`), `[+0x15C]` (stream model via factory `0x465924`: type 0 `0x450BA0`, type 1 `0x43CE00`, type 2 `0x465AD0`), optional `[+0x160]`, `[+0x164]`. The zone bits `0x200` are a separate collision/exemption layer written by `0x4311D0/0x43127E`. | DECOMP `0x42ACAC`, `0x465924` |
| 2 | Surface geometry is therefore model data, separate from terrain zone bits. Its height drives node translations; it does not draw the terrain rectangle. | DECOMP `0x42B24D` |
| 3 | Per update (`0x42B24D` after the accepted height machine): wave phase `[+0x148] += dt` wrapped at the period; lane `x = (u32[+0x16C] & 3)/4 + phase/period`; wave `w = trig(2*pi*x) * amp`, clamped so `h + w >= 0`; `w = 0` when `h == 0`. | DECOMP; kind-0 constants BYTES+DECOMP (period 2.0, amp 0.5, lane count N = 4) |
| 4 | Four vec3 node translations are rewritten each update: `slot[k] = posOff + (h + w) * ampVec` with `slot4 <- ([+0x200], [+0x260])` and `slot2 <- ([+0x1B8], [+0x218])` on lane `L`, `slot5 <- ([+0x1E8], [+0x248])` and `slot3 <- ([+0x1D0], [+0x230])` on lane `L' = ((v+1)&3)/4`. Slots live at `([+0x158 object]+0x118)[idx]` (24 bytes each). Slots 2/3 are copied to slots 6/7 of the `[+0x15C]` object. | DECOMP |
| 5 | Per-part colour (vec3 at part record `+0x18`, stride `0x38`, `0x42CC10`) is driven by `col = ((trig(2*pi*(x-0.25))+1)/2)^2` from `[+0x170]`; per-part alpha (`0x42CC40`) and uniform object tint (`0x42CB70`) exist but the alpha branch is guarded by `T.alphaPeriod != 0`, which is **0 for kind 0** -> alpha animation is not run for MM_W_012 gates. A light-cell tint (`0x4C6A10` grid, `v^4 * 0.75`, clamp 255) is written to `[+0x15C]`'s child. | DECOMP, BYTES `0x4BF6E0` |
| 6 | Reset state: reader zeroes `[+0x148]`/`[+0x150]`, `done=2`; height `= [+0xA0] = 0` for the initial request 1 (all 19 gates, accepted lever-links measure). With `h == 0` the wave `w` and `col` are forced to 0, so the **initial image is the un-waved model at its authored offsets**. Reload = a fresh `loadRetailLevel` (accepted in lever-gates-impl-001: receiver state lives in `LevelSetup`). | DECOMP; accepted reports |
| 7 | Height ramp for the lever (183,60) gates (ids 5, 6; `entities_42acac[12]`, `[11]`): 0 -> 24.5 at `T.rate = 2.0` per `dt` second (accepted shared-dt path): 24.5/2.0 = **12.25 s** of dt (arithmetic from accepted constants; before the 100 ms/step cap and level-clock splitting that the accepted path already handles). The step does not run a request-4 lowering for this level (no lever sends type 5). | arithmetic on accepted values |
| 8 | Secondary objects `0x4CBB08` (list `[+0x194]`, resource name from `[+0x190]`): last entry placed at `+0xD8`/`+0x120` from `0x43D56C` and given visibility `e = ((8 - [+0x98]) + h)*32 - 1 > 0`; `0x42B0B8` registers the list through `0x441E26`. These are the emitter/spray objects; their content is **unmeasured**. | DECOMP |
| 9 | Clone today: `BackdropScene::build` extracts model roots from `entities_425e50` and `entities_426c6b` only (`src/app/BackdropScene.cpp:158-179`). **No `entities_42acac` model is a scene root**, so nothing of the gates is drawn at any height. The existing `FluidGate` state (`src/sim/World.h`, `World.cpp::FluidGate::step`) already carries height, request/done/armed and the AABB, and is stepped from `updateSegment` after `stepLeverContacts`. | SOURCE |

Optional effects (not required for a faithful first visible surface): alpha animation (kind 0 skips it anyway), the per-part colour wave,
the light-cell tint, the secondary spray/emitter objects, and all kind-1 plumbing. They remain explicit unknowns, not decoration to invent.

## 2. What is missing (essential) -> why the verdict is blocked

1. **Installed asset identity for MM_W_012.** The names, types and node/part counts of `model` / `model_15c` / `model_160` / `model_164`, the
   `resource_name` string, `[+0x16C]`, `[+0x140]` and the ten vec3 values (`+0x170`, `+0x1B8..`, `+0x218..`, `+0x278`, `+0x290`) are
   parsed by the unchanged structured reader but their values were not read in this invocation: the measurement driver was written and registered
   (CTest `fluidsurface_measure`, output `build/agent-native/main/fluid-surface-research-001/gate-records-MM_W_012.txt` once the supervisor's `main` verification
   runs it), but this worker has no permitted way to build and run a single target (Bash is limited to the two verifier commands; a full `main` profile took
   2111 s of test time in the last accepted run, longer than the 600 s client limit and the remaining budget), and the installed box cannot be searched
   reliably with the grep tool (NUL handling). **No asset name is claimed in this report.**
2. **Slot/part/child index mapping.** Which `SceneNode`/mesh/material the `+0x118` slot indices 2..7, the `+0x1C0` child array (indices 0, 1) and the
   `+0x38`-stride part records (parts 0..2) correspond to in the clone's `SceneNode` tree is not traced; `0x412C90`, `0x4130C0`, `0x413640`, `0x42CD10`
   and the class-0 reader `0x450BA0`/`0x450F57` vt+0x50 were not decompiled. Without it a node transform cannot be bound to a mesh, and a rectangle would
   have to be invented.
3. **Trig function** (`0x49FF45`: sin vs cos) and the three vec3 helper semantics (`0x401830`, `0x4027C0`, `0x402680`) are inferred, not read.
4. **Transparency / depth / draw order** of the gate models: no render-state evidence was gathered (no submission trace from `[+0x158]` to the original's
   renderer; the clone's existing LegacyUnattributed pass, sorted after Primary and before Secondary, is the only available analogue).

Items 1-2 are essential. They are the "resource/geometry/material" part of the acceptance criterion; the dynamic height/transform part is established (findings 3-7), but a
height contract without a bound mesh cannot be implemented without fabricating geometry, which the task forbids.

## 3. Mapping to existing clone code (for the follow-up, once the blocker clears)

| Contract part | Existing clone point | Change needed (next task) |
| --- | --- | --- |
| Gate model roots | `src/app/BackdropScene.cpp:158-179` (`models(...)` lambda; today only `entities_425e50/426c6b`) | Admit `entities_42acac` `model_15c` (and `model` if non-null) as LegacyUnattributed roots with their own owner label; extend `BackdropScene::RootSource` owner string only |
| Per-gate dynamic transform | `renderStage(...)` -> `buildSceneMeshSource` (`SceneMeshSource.cpp`), `SceneDispatch` | A per-root transform/offset callback (analogous to `setSecondaryHidden`) fed from the sim; no change to `SceneRaster` |
| Live height | `FluidGate` in `src/sim/World.h`, owned via `LevelSetup`/`World` | Read-only accessor of `height`, wave phase (accumulate `[+0x148] += dt`, wrap at 2.0), `[+0x16C]` |
| Frame submission | `src/app/main.cpp` (gameplay render path that already calls `BackdropScene::render`) | pass the sim's gate snapshot; reload already rebuilds the scene |
| Tests / images | `src/__test__/test_backdropscene.cpp`, `test_levergates.cpp` | baseline frame (initial, gates at 0), after-lever frames (rising mid-ramp, settled 24.5), reload frame, all offscreen `Raster` PNG outputs from the clone |

Visual acceptance stages for the follow-up: **initial** (all gates h = 0: un-waved authored position), **rising** (lever (183,60) pulled, gates ids 5/6 at e.g. 6.0 and 12.25 of ramp; gates at x[152,168) change, all others identical to initial), **settled** (h = 24.5; waved), **reload** (fresh `loadRetailLevel`: byte-identical to the initial image). Each stage is a source-built clone image; none is original-fidelity proof.

## 4. Handoff to the next stage (concise)

* Do not start `fluid-surface-impl-001` as queued. First close blockers 1-2: run `fluidsurface_measure` (the supervisor's `main` verification does), read
  `build/agent-native/main/fluid-surface-research-001/gate-records-MM_W_012.txt`, then decompile `0x412C90`, `0x4130C0`, `0x413640`, `0x42CD10`, `0x450BA0`
  and its vt+0x50 reader to bind slot 2..7 / child 0,1 / part 0..2 to the parsed `SceneNode`s of the gate model. If the model is a plain type-0 node tree the binding
  is a bounded follow-up; if it needs the original's class-0 reader semantics beyond the existing `SceneNode` parser, record that as the next blocker.
* Reuse (accepted, unchanged): gate state machine and shared elapsed-dt path (lever-gates-impl-001, shared-timing-impl-001), stored faces (lever-initial-face-impl-001),
  zone-mask painters, and BackdropScene LegacyUnattributed machinery. Kind-0 constants above are measured; kind 1 is untouched.
* Preserve: zone-mask behaviour and all parks (D1/D3/base clock, release/digger, clipping/controller/capture, timed Armageddon).

## 5. Verification state

Changed files: `src/__test__/test_fluidsurface_measure.cpp` (new, read-only driver; skips without `WEBOLUTION_GAME_DIR`; asserts 19 gates and the stored fields of gates 11/12 only),
`CMakeLists.txt` (registers CTest `fluidsurface_measure`), `docs/scratch/fluid-surface-research-001.md`, `build/fluid-surface-research-001/static-trace.md`.
The driver has **not been compiled or run** by this worker; it is unexecuted code. A compile error would fail the supervisor's `main` verification, which is the
signal to fix it. Full registered verification is pending supervisor execution. No production behaviour changed.

## 6. Revision 2 (resume of run `20261112T031426-15184-7c293a32`): model binding and render contract

Inputs: the retained attempt-1 trace, the focused measurement
`build/agent-native/main/fluid-surface-research-001/gate-records-MM_W_012.txt` (CTest `fluidsurface_measure`, passed in
isolation per `build/agent-control-upgrade/fable-resume-20261112T033715Z/focused-measurement.log`), and new static reads
recorded in `build/fluid-surface-research-001/static-trace.md` section F (addresses, evidence ids, table bytes).
Static REA only, closed without a snapshot; the original was not run; no production behaviour edited.

### 6.1 Corrections to sections 1-2
| Earlier statement | Correction (evidence: static-trace F) |
| --- | --- |
| Finding 4 "node translations" | They are **mesh vertex positions**. `0x412C90(node+0x118, i)` indexes the node's vertex list (24 B each). |
| Finding 5 "part record, stride 0x38" | A "part" is a **corner of a kind-5 triangle record** (`SceneCorner`: index, UV, RGB @0x18, alpha @0x30). `child` = block[1] record 0, `child2` = block[1] record 1 of the `Fluid` mesh. |
| Finding 6 "initial image is the un-waved model at its authored offsets" | **Incomplete.** Draw `0x42C5C9` negates normals 0..2 of the `Fluid` node whenever `h == 0`, so the facing predicate is inverted at height 0 (6.2 item 5). |
| Blocker 1 (asset identity unread) | Closed: measured, 6.2. |
| Blocker 2 (slot/part/child mapping) | Closed: 6.2. |
| Blocker 3 (trig, vec helpers inferred) | Closed: `cos(2*pi*x)`; subtract / scalar-multiply / add confirmed (trace F5). |
| Blocker 4 (transparency/order) | Largely closed: blend selectors, depth bias and frame order read (6.2 items 6-8). Depth-write state for blended draws was not read (6.5). |
| Section 3 "admit `model_15c` as LegacyUnattributed roots" | Superseded by 6.3: gates are drawn in their own three passes **after the tube**, and the essential root is the base `model` (`Fluid`), not `model_15c`. |

### 6.2 The contract (MM_W_012; all 19 gates share one structure; lever (183,60) drives `entities_42acac[12]` id 5 and `[11]` id 6)
1. **Geometry (installed, separate from zone bits).** Base model = inline unnamed type-0 model whose root is one type-1
   node `Fluid`: 6 vertices, 3 normals, material `''` (no texture) with blend selector 1, block[0] kind 4 x 2 triangles
   (shared colour), block[1] kind 5 x 2 triangles (per-corner colour). The reader forces the root head matrix to identity:
   vertices are world coordinates.
2. **Dynamic vertices.** Every update: `vertex[2] = pos_off[0] + (h+w1)*amp[0]`, `vertex[4] = pos_off[3] + (h+w1)*amp[3]`,
   `vertex[3] = pos_off[1] + (h+w2)*amp[1]`, `vertex[5] = pos_off[2] + (h+w2)*amp[2]`; vertices 0,1 are never written.
   Installed `amp[k] = (0,1,0)`; `pos_off` per gate is in the measurement (gate 12: (41.5573,4,-41.5573),
   (48.7752,4,-32.5906), (41.7078,4,-27.8682), (35.5469,4,-35.5469); gate 11: (32.5906,4,-48.7752),
   (41.5573,4,-41.5573), (35.5469,4,-35.5469), (27.8682,4,-41.7077)). Vertices 2,3 are the outer-radius edge (~58.5),
   4,5 the inner edge (~50).
3. **Wave.** `phase += dt; while (2.0 < phase) phase -= 2.0`. `x1 = (value_16c & 3)/4 + phase/2`,
   `x2 = ((value_16c+1) & 3)/4 + phase/2`; `w = cos(2*pi*x) * 0.5`; if `h == 0` then `w = 0`; if `w + h < 0` then
   `w = -h`. Installed `value_16c`: gate 11 = 19, gate 12 = 20 (adjacent gates share an edge lane).
4. **Top-surface colour (always overwritten before draw).** `col = ((cos(2*pi*(x-0.25))+1)/2)^2`, 0 when `h == 0`;
   `c = (0,0,192) + (255,255,63)*col`. Lane 1 -> block[1] rec0 corners 0,1 and rec1 corner 1; lane 2 -> rec0 corner 2 and
   rec1 corners 0,2. Corner alpha and block[0] RGBA stay as stored (the alpha branch is skipped for kind 0).
5. **Visibility at height 0.** Effective normals = stored when `h != 0`, negated when `h == 0` (`0x42C5C9`), fed to the
   usual facing predicate `dot(eye - firstCornerVertex, normal) > 0`, which kind 4 shares with kind 5.
6. **Material state.** No texture; blend selector 1 = SRCALPHA / INVSRCALPHA; kind 4 flat-shaded with the record's shared
   RGBA on all three corners; kind 5 Gouraud.
7. **Depth bias.** Model `+0xF8` (third stored double: 0.00015 base, 0.0003 `+0x15C`, 0 pipe) is added to `[0x58D2B0]`
   for the duration of that model's draw (`0x451865`).
8. **Submission order.** Frame `0x441845`: tube, then for all gates `+0x160/+0x164`, then for all gates the base
   `Fluid` model, then for all gates the `+0x15C` model, then list `G+0x1FD0` and the secondary holder.
9. **Stages.** Initial: gates 11/12 load with request 1 -> `h = low = 0`, `w = 0`, `col = 0`, normals negated.
   Rising: `h` grows 2.0 per dt-second to 24.5 (12.25 s), top edge at `y = 4 + h + w`. Settled: `h = 24.5`, wave and
   colour keep animating with `phase`. Reload: a fresh level load re-reads vertices/normals, `phase = 0`, `[+0x1B1] = 1`.

### 6.3 Mapping to the clone and the bounded implementation slice (`fluid-surface-impl-001`)
| Step | File | Change |
| --- | --- | --- |
| A | `src/assets/SceneNode.cpp:132-155` | Accept mesh kind 4: stride 96, 3 corners, shared RGBA read before the corners exactly like kind 6. Today the parser rejects kind 4, so no gate model can parse. |
| B | `src/app/BackdropScene.cpp:41,236` | Apply the facing predicate to kind 4 as to kind 5 (`0x46AE20` vs `0x46AFA0`). Do not change kind 6/7 handling. |
| C | `src/app/BackdropScene.h/.cpp` (`build`, `renderStage`) | New root source for `entities_42acac[i].model` in a new stage (e.g. `Stage::FluidGate`), root matrix forced to identity; per-root override hook (like `setSecondaryHidden`) supplying vertices 2..5, the six block[1] corner colours and the normal sign; pass `model.double_bits[2]` as the bias argument of `adaptPackedSceneMeshVertex` (the literal `0.0` at line 240 today). Blend: existing `options(false, blendSelector==1)`. |
| D | `src/sim/World.h`, `World.cpp` (`FluidGate`) | Add `wavePhase` (advanced in `step(dt)` with the 2.0 wrap), `lane` (`value_16c`) and read-only access; no change to the accepted height machine or zone painting. |
| E | `src/app/main.cpp:664-668`, `scheduleSceneFrame` | Render the gate stage after the tube callback and before Secondary, from the live `LevelSetup::gates` snapshot. |
| F | `src/__test__/test_backdropscene.cpp`, `test_levergates.cpp` | Parser fixture for kind 4; offscreen clone images for the four stages below. |

Image criteria (source-built clone only; none is original-fidelity proof): **initial** = gates 11/12 at `h = 0`;
**rising** = after the ordinary lever (183,60) pull, frames at two heights show the `Fluid` top edge of gates 11/12 at
`y = 4 + h + w` with every other gate's pixels unchanged; **settled** = `h = 24.5`, two frames a known dt apart differ
only by wave/colour; **reload** = byte-identical to the initial image.

### 6.4 Optional versus essential
Essential (in the slice): steps A-E for the base `Fluid` model, items 1-8 above.
Optional, evidenced but not required for a first visible surface, and to be reported as omitted if not done:
`+0x164` `pipe_supply.a3d` static model (gates 0, 6, 11, 13; parses once kind 4 exists); `+0x15C` additive overlay
(blend selector 2 = ONE/ONE, vertices 6,7 follow base 2,3, its colour comes only from the light-cell tint, which needs the
`0x4C6A10` light grid); `^water_glow` secondary objects (`+0x190/+0x194`); kind-1 gates. Do not substitute invented
geometry or colour for any of these.

### 6.5 Open items (stated, not hidden)
1. **Stored vertex/normal/RGBA values were not read by this worker.** The driver now also writes
   `fluid-surface-research-001/gate-meshes-MM_W_012.txt` (gates 11, 12: vertices, normals, head, every record's
   corners/RGBA, and whether stored vertices 2..5 equal `pos_off`). That extension is **uncompiled and unexecuted**:
   Bash here is limited to the verifier profiles and a `main` run exceeds the 600 s client limit. The supervisor's
   verification will build and run it; a compile error there is a defect in my change. The implementation does not
   depend on these values (it parses them from the installed level), but the height-0 appearance does: with negated
   normals a face is drawn only if it was back-facing before, so whether anything of `Fluid` shows at `h = 0` depends on
   the stored normals and the camera. Read the dump before fixing the "initial" image expectation.
2. **Possible second dispatch.** `docs/lessons/actor-rendering.md:209-214` (accepted) says the tube walk over
   `0x4C8898` also dispatches `0x42C5C9`. I did not trace which gates enter that list (`0x43B517` call site
   `0x42A241`). The post-tube three-pass loop in `0x441845` is certain; a duplicate in-tube draw would matter for
   alpha-blended pixels and is unverified.
3. Depth-write/z-test state for the blended gate draws and the exact consumer of the bias in `0x46D372` were not
   re-read; the slice reuses the clone's existing mesh options.
4. `0x42C714` (surface height sampled from the Y of vertices 4/5) feeds actor logic; it is outside this render milestone.

### 6.6 Handoff
Next stage `fluid-surface-impl-001`: implement 6.3 A-F for the base `Fluid` model first; read
`gate-meshes-MM_W_012.txt` from the supervisor's run before writing the initial-image assertion; keep zone-mask
behaviour, the shared-dt path, stored lever faces and all parks unchanged. Timing, clipping and capture stay parked.

### 6.7 Verification state (revision 2)
Changed in this invocation: `src/__test__/test_fluidsurface_measure.cpp` (mesh dump added, no new failing
assertion), `docs/scratch/fluid-surface-research-001.md`, `build/fluid-surface-research-001/static-trace.md`.
No verifier profile was run by this worker. The only completed check is the pre-existing isolated run of the
**previous** driver revision (`focused-measurement.log`, 1/1 passed). Full registered verification is pending
supervisor execution.

## 7. Revision 3: focused review corrections; essential depth blocker retained

Milestone step: **research**, installed `Compiled\MM_W_012.lvl`, ordinary lever
`(183,60)`, serialized receivers id 5 / 6 = stream indices 12 / 11. The source-traced
failure remains: the production scene extracts neither receiver root, and its parser
and mesh adapter reject kind 4. Accepted gate height/mask motion therefore produces
no fluid pixels. Binding those existing meshes is the next rendering dependency,
but the effective depth contract must be supported before implementation can start.

Inputs read: the complete blocked review
`build/agent-runs/loop-e52c854be95f4df7b8b00893f57a831a/0001-review-6c20ae/last-message.txt`,
its retained review evidence, trace sections A-F, both supervisor-generated
`build/agent-native/main/fluid-surface-research-001/gate-{records,meshes}-MM_W_012.txt`,
accepted lever/link/init/timing/gate/initial-face reports and the retained C3q/A1/C3bk
depth findings. New static evidence is indexed in trace section G and retained in
`build/fluid-surface-research-001/correction-3/rea/` (48 complete Evidence records).
Same registered reference SHA-256 as above; Ghidra 12.1.4; no original execution.
REA was closed with no snapshot arguments. Difficulty: difficult; advisory model:
Opus for inherited render-state ambiguity; user override remains Codex gpt-6.1-sol/high.

### 7.1 Depth: what is established and exactly what blocks

The gate draw is `0x441845` post-tube loop -> vt+0x10 `0x42C5C9` -> `0x4323EB`
-> model draw `0x451865` -> holder/node/mesh -> `0x469D71` -> material binder
`0x43BD80`. Its empty material calls `0x472979`, then selector 1 calls `0x414D80`.
Fresh pseudocode and instructions corroborate C3q: null binding disables alpha test
and binds null; it resets selector cache to 4, then selector 1 requests
ALPHABLENDENABLE=1, SRCBLEND=5 (SRCALPHA), DESTBLEND=6 (INVSRCALPHA).
These routines do **not** set ZENABLE, ZWRITEENABLE, ZFUNC or ZBIAS. Successful API
effects are conditional, not statically observed. Alpha 128 is stored on both kinds;
it is not evidence of a depth-write disable.

The **software projection bias** is closed: `0x45186F..0x45189E` adds model+0xF8
to context `0x58D2B0`, draws, then subtracts it. Base Fluid contributes +0.00015;
`0x46DE50` **subtracts** that context from transformed native Z after computing XY.
Thus `Znative = s*Zview - b`; the existing packed-mesh adapter accepts this native
bias and converts the packed result back by `/s`. This is not an API ZBIAS setting,
not +0.00015 physical view units, and not permission to infer the inherited b.
C3bk's camera reset gives zero on its clean prefix; tube -/+0.0003 brackets are
outside the post-tube gate scope. Add/subtract brackets are arithmetic, not a
bitwise save/restore guarantee for arbitrary inherited operands.

**Essential blocker:** the supplied executable/static call path leaves effective
`ZENABLE` (including Z/W-buffer mode), `ZWRITEENABLE`, `ZFUNC` and device `ZBIAS`
inherited at this draw. C3q explicitly retains these as unknown inputs; A1's
bounded state-call scan is not an exhaustive dynamic-operand absence proof and
its historical wrapper observations do not supply these values. No retained
gate-specific state observation resolves them. The allowed target excludes
analysis of a wrapper/DLL, and original execution/state observation is unauthorized.
Choosing Less/write-on from `BackdropScene::options` would invent the missing
contract. It can change fluid/terrain and later-surface occlusion, so it is essential.
No unsupported approximation, depth/clipping repair or new observation is proposed.

### 7.2 Stored geometry, first update and reload

The measured base mesh has alpha 128, upward top normal `(0,1,0)`, fixed vertices
0/1 at Y=4, and moving vertices 2..5 at Y=28.5/29. `pos_off[k].y=4` is a separate
serialized field. These are valid stored values, not uninitialized fields.
Arithmetic consistency (not a reconstruction of authoring history): with h=24.5,
phase=0 and lanes 19/20, the established wave equation gives outer-edge Y
`(28.5,29)` for gate 11 and `(29,28.5)` for gate 12 at dump precision. Red lanes
are approximately `(0,63.75)` / `(63.75,255)` at integer channel resolution;
stored near-zero `9.30937e-28` is not exactly reproduced by binary64 libm
(`9.08361e-28` using the traced promoted-f32 pi literal, 3.1415927410125732).
Use that literal in the future animation contract; do not replace it with M_PI.
This is consistent with a baked raised sample, not proof of its authoring history,
bitwise x87 equivalence or h=24.5 as the loaded runtime state.

| Stage | Static/installed expectation |
| --- | --- |
| Raw deserialization, before any update/draw | `0x42ACAC` reads the meshes unchanged, forces root matrices to identity, resets phase=0 and normal-sign latch=1, then sets runtime h=low=0 for request 1. It does not call `0x42B24D` or replace vertices with offsets. |
| First world update | `0x4423D2` tests receiver+0x38 (measured true), calls vt+0x14, and always reaches the wave/colour tail, even if request==done or dt=0. For unactivated ids 5/6, vertices 2..5 become their offsets at Y=4, top RGB becomes `(0,0,192)`, alpha stays 128. Side triangles collapse; the annular top remains a flat radial strip. |
| First normal gameplay draw | `0x45CFF9` loads then updates before `0x441845`; running frames have a trailing update, paused frames an update(0). `0x4432C1` enables +0x5139 after post-load hooks. `0x42C5C9` changes the normal-sign latch when `(h!=0)` changes, so the first h=0 draw negates normals 0..2 once. For an eye above Y=4 the downward top fails the facing predicate. Do not expect a raised blue pool for these two drained receivers. |
| Rising / settled | Once h>0 the draw restores stored normals once; update geometry uses `4+h+w` and the established colour lanes. Height settles to 24.5 by the accepted compare-before-add recurrence; wave/colour continue after settling. |
| Reload | Fresh read restores the same serialized raised mesh, h=0, phase=0 and latch=1; the first update/draw repeats the drained geometry/sign transition. Compare equivalent stages and dt histories. Raw load and post-update frames are different stages. |

This establishes normal-loop ordering from static instructions, **not an observed
original frame**. The distinct builder `0x428A47` has a dt=0 priming call; do not
transfer it to installed deserialization, which takes `0x448BD0 -> 0x42ACAC`.
Clone seam: `main.cpp:679` currently makes a pre-loop render before any elapsed
update; ordinary window frames update at :1053 then render at :1073. A future
integration must label that diagnostic/raw-load frame separately and establish
the production render snapshot for each stage without adding a gate clock or
changing accepted world timing. `--sim-ticks` uses the legacy wrapper, which does
not step gates; it cannot establish the post-update surface.

### 7.3 Registration is not a second draw

The installed base-model reader **does register** the receiver through
`0x43231D -> 0x43B517`, adding it to tube list `0x4C6A10+0x1E88`. However the
two dispatch arms at `0x43A9BB` and `0x43AA47` both require receiver+0x28=true;
the latter also requires the camera column interval. All 19 measured receivers
store +0x28=false (and +0x18=0); +0x38=true is the independent **update** guard.
Thus registration does not establish duplicate alpha blending on this installed
route. The traced normal path skips the tube dispatch and submits one base Fluid
draw in the unconditional post-tube receiver pass, subject to model/facing/pixel
admission. Preserve its source order; do not add
a duplicate in-tube Fluid pass. This is scoped to the installed flags/traced path,
not a claim about every constructed receiver or an untraced later flag mutation.

### 7.4 Corrected implementation and image handoff (conditional; do not start)

Preserve section 6.2's established geometry/slot/material mapping, with the
ordering and depth qualifications above. Correct section 6.3 as follows:

* `src/assets/SceneNode.cpp:132-155`: admit kind 4, serialized stride 96,
  three corners and shared RGBA (memory record stride 0x70 is different).
* **`src/app/SceneMeshSource.cpp::visit:45-46` must also admit kind 4.** Its
  independent 5/6/7 guard otherwise still rejects Fluid after parsing succeeds.
* `src/app/BackdropScene.cpp`: include kind 4 in facing, flat shade its shared
  RGBA; keep kind 5 Gouraud. Extract base receiver roots, force the traced root
  identity, bind source stream index to the corresponding live gate, override
  vertices 2..5 and six top colours, and implement normal-sign transitions.
  Use model+0xF8 in `adaptPackedSceneMeshVertex`'s native-bias input. **The section
  6.3 instruction to reuse `options(false, blendSelector==1)` is withdrawn:**
  depth state remains blocked; no general raster repair is authorized.
* `src/sim/World.h/.cpp` plus loader `src/sim/RetailLevel.cpp`: retain a render
  snapshot/lane and phase advanced once per accepted shared-dt segment, including
  idle/settled gates. Do not place animation after `FluidGate::step`'s current
  request==done early return; original `0x42B29E` jumps to the animation tail.
  Keep the height/mask recurrence and stored lever faces unchanged.
* `src/app/main.cpp`, `src/app/SceneFrame.h` (`scheduleSceneFrame`): submit
  the base Fluid pass after tube and before Secondary, in receiver source order;
  feed the same world snapshot in ordinary frames and explicitly labelled
  diagnostics. Rebuild the roots and runtime binding on reload. `SceneRaster`
  is an existing integration endpoint, not evidence of native depth policy.

**Replace the earlier unchanged-other-gates requirement with matched-time
controls.** Planned image evidence (not rendered in this research pass): two
fresh MM_W_012 clone runs with the same camera, dt **segment sequence**, elapsed
time since load, animation/random inputs and state, differing only in the selected
lever activation. At each rising sample (two distinct measured heights) and
settled sample, retain control and activated full frames plus receiver state and
source-labelled surface submissions/writes. Compare non-target geometry/colour
snapshots at equal phase; do not require their pixels unchanged across time.
Gates 8..10 start raised at h=22.5 and animate in both runs. Other scenery/actors
may animate; actor differences or changed occlusion must be accounted for rather
than attributed to unrelated gate motion. A test-only isolated draw is insufficient.

Initial/reload acceptance compares **post-first-update** stages with the same
camera and post-reset dt/phase history, independently confirming h=0 for ids 5/6,
loaded h=22.5 for gates 8..10, restored stored lever faces and normal signs.
Byte-identical full-frame reload is conditional on identical all-scene inputs;
it is not an unconditional consequence of reload. Also retain raw-load geometry
as provenance, not the expected normal gameplay image. Settled paired frames
must distinguish continuing wave/colour from height progress. Original comparison,
natural lever reachability, final image visibility/occlusion and full fidelity are
not established here; the later authorized clone assessment still owns those checks.
Optional overlays/pipes/emitters remain the explicit omissions in 6.4, not invented
replacements for the base surface.

### 7.5 Checks, changed files and next action

Changed this pass: this report (current verdict plus appended revision), retained
`build/fluid-surface-research-001/static-trace.md` (append G), and ignored evidence
under `build/fluid-surface-research-001/correction-3/` (pre-edit copies, full static
Evidence, index/arithmetic/provenance check). No production, measurement driver,
CMake, registered assertion, timeout, control, shared-instruction, queue or save edit. The active
supervisor lease PID 36960 was present/live; no second lease/writer was started.

`check_evidence.py` checks the supervisor's pre-dispatch manifest (1591 entries,
excluding its expected queue transition), the 19 retained receiver records, the
phase-zero consistency arithmetic and all 48 REA identities. Output: `checks.json`.
Its queue-transition and near-zero arithmetic check corrections are retained in
`check-initial-failure.txt`; this is a scratch provenance check, not CTest or runtime proof.
No new build or clone render ran. The existing CTest `fluidsurface_measure` remains
registered. Historical full verification at
`build/agent-runs/loop-e52c854be95f4df7b8b00893f57a831a/0001-verify-245da6/verification.log`
passed **143 main / 84 recovery**, including the mesh-dump driver. It superseded
revision 2's pending status for that earlier tree, not for this correction.
Fresh mandatory main/recovery verification and independent review remain pending
with the supervisor after worker exit.

**Next action: review the correction as BLOCKED. Do not auto-start
`fluid-surface-impl-001`.** The sole essential contract blocker identified by this
pass is effective inherited depth state at the alpha-blended base-Fluid submission.
It needs admissible retained state evidence or an explicitly authorized way to
obtain it; the current static scope does not supply it. No park is reopened, no
original run is requested or performed, and no queue/control change is made here.
