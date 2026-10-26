# Level-frame map — established paths and gaps

C3at, 2026-10-25; production stage update C3bd, 2026-10-26.
[Mapping check](scratch/phase-c3at-frame-map.md), [ordering check](scratch/phase-c3bd-stage-order.md).
This consolidates accepted evidence; it is **not a native draw census**.
READ/DERIVED/OBSERVED retain the linked notes' scope. SOURCE means current
repository code only, not original execution. Unknown is not a device default.

## Native conditional frame spine (READ, not a repair4 trace)

[C3u](scratch/phase-c3u-pause-scene-reachability.md) establishes:

```
u32 G+0 == 2 -> frame 0x441845
  u8 G+0x5139 != 0
    -> primary holder root slot +4 at 0x4749A7
    -> tube 0x43A4EC at 0x441884, including eligible entities/actors
    -> secondary holder, if u8 G+0x51E8 != 0, root slot +8 at 0x474955
```

G is 0x4C6A08. Widths/conditional call order are known; **repair4 gate values,
last writers, admitted objects and emitted/visible draws are unknown**. Pause
ui_state is separate. Successful-load (mode,enable)=(2,1) is conditional DERIVED,
not memory evidence for the saved PNG. Entrance/Exit have separate model-holder
paths and additional tube-entity eligibility conditions, not automatic emission.

[C3y](scratch/phase-c3y-secondary-slot8.md) establishes secondary slot+8 sorts
children by composed/rotated translation plus child bias and calls each child's
slot+4. [C3ao](scratch/phase-c3ao-secondary-order.md) integrates retained distinct
finite keys. Ties, NaN/corrupt/empty arms and runtime cache effects remain outside
that acceptance. Root slot+8 is not equivalent to root slot+4 rendering.

## Production spine (SOURCE)

`main.cpp` clears once and uses `scheduleSceneFrame`: supported primary,
legacy-unattributed model contributions, TubeScene (including actors), then
supported mixed secondary. `BackdropScene::renderStage` partitions retained
primary root59644 and secondary root91782; Entrance/Exit roots9ccd1/a0cdd retain
their former pre-tube timing, explicitly **not primary**. Headless image conversion is
followed by optional skill panel, HUD and caption (`main.cpp:449–525`). Those
extras are not the general SDL-window HUD scheduling path.

**Corrected in C3bd:** software secondary scenery now follows the tube, preserving
the mixed-secondary dispatcher. Independent production-order mutation fails, but
all fixed screenshots are unchanged. This is conditional SOURCE integration, not
proof of historical runtime admission or an attributed pixel-error cause. Separate
model-holder timing/admission remains unsupported, rather than classified by exclusion.

## Family/stage map

| Family / production consumers | Established input and mechanism | First unsupported/divergent connection and remaining state |
|---|---|---|
| Tube route — `TubeScene::render`, `CellMesh`, `Raster` | LVL collision/grid-derived mesh0/mesh4/INN; [A4c](scratch/phase-a4-cell-mesh-rule.md), [A4d](scratch/phase-a4-cellmesh-route.md). Camera world→view→screen; INN→OUT-right→OUT-left→OUT-centre; HI tile convention within accepted views. | Exact per-frame record admission/resource/effective state is not joined to a native submission stream. Broader clipping/door branches/levels and depth relation remain open. |
| Tube diagnostics — `TubeScene` wall/surface/PolyList/cellmesh and flat modes | Stored/reconstructed geometry through optional software paths; [C3w](scratch/phase-c3w-source-integration-map.md) is SOURCE navigation. | These are not interchangeable native route families. PolyList diagnostic lacks node/material context; fixed UV/tile binding is not C3 material fidelity. |
| Scene meshes — `BackdropScene`, `SceneMeshSource`, `SceneRaster` | Guarded retained MM_CLIMB_002 roots; node/ancestor/view positions, indexed attributes/materials [C3n](scratch/phase-c3n-mesh-submission.md); conditional predicate [C3ai](scratch/phase-c3ai-predicate-context.md); supported root stages [C3bd](scratch/phase-c3bd-stage-order.md). | Actual admission/cache state and separate model-holder timing remain unknown; clipping/type1 dispatch/precision remain incomplete. Raw diffuse is adopted [C3an](scratch/phase-c3an-raw-diffuse.md), **not** the complete f32 pack helper [C3ak](scratch/phase-c3ak-vertex-pack.md). |
| Scene billboards — `SceneBillboardSource`, BackdropScene slot triangles | Retained node/ancestor/shared camera S/P chain [C3af](scratch/phase-c3af-billboard-chain.md). Conditional caret flags-8 policy [C3al](scratch/phase-c3al-caret-alpha.md); secondary mixed order [C3ao](scratch/phase-c3ao-secondary-order.md), after tube [C3bd](scratch/phase-c3bd-stage-order.md). | Runtime admission/clipping, resource version, effective states and exact precision remain unknown. Reference binary32 diagnostics are not runtime x87 proof. |
| Actors — `TubeScene.cpp` prepare/draw/composite helpers, `ActorPose`, sprite helpers | Native terrain sample→mesh pose, camera-relative view/frame→screen rectangle/light tint. [A3](scratch/phase-a3-sprite-view.md), [A6a](scratch/phase-a6-actor-pose.md), [A6c](scratch/phase-a6-actor-tube-composition.md): native actor list after tube passes, reverse list order. | Fixed/World/loaded injection is not a full matched native actor list; temporal input defaults to 0, not a universal animation clock. General runtime state/pose correspondence and actor/tube depth are unresolved. Zero-zoom POINT/EF is scoped [A4x](scratch/phase-a4-actor-filter-state.md), not all zooms. |
| Skill panel — `drawRetailSkillPanel`, headless main | Plate/selected/count/icon image composition; positions/white glyphs [C1s](scratch/phase-c1s-frida-icon-hook.md), plate/cache sample [C1ad](scratch/phase-c1ad-plate-blend-live.md), zero-count skip [C1v](scratch/phase-c1v-plate-draw-read.md). | General runtime invocation and effective filter/material/cache behavior are not this optional compositor. C1 is [done in amended scope](scratch/phase-c1-close-amended-gate.md); historical C1t failure is not current status. |
| HUD — `HudText`, headless `--hud-ticks` | Separate simulated World values, ^Font glyphs and counter/timer placement [C2g](scratch/phase-c2g-counters-timer-render.md). | Native value/object feed, runtime invocation and effective glyph/material state are not observed. Simulation-derived HUD values are not repair4 memory values. |
| Caption — `PauseCaption`, headless `--pause-caption` | Geometry formula [C2f](scratch/phase-c2f-caption-placement-read.md), sampled rectangle [C2h](scratch/phase-c2h-caption-live-read.md). | Native pause gate/text consumer and final effective blend/composition are not closed. Software black/128-alpha remains a [hypothesis](scratch/phase-c2i-caption-render.md), not new native evidence. |

## Shared state/submission boundary

Mesh packing/source reads end at a candidate Device3-layout TL fan call;
[C3au](scratch/phase-c3au-comparison-boundary.md) selects that pre-wrapper boundary
conditionally for a thin comparison. [C3av](scratch/phase-c3av-thin-comparison.md)
tests synthetic recording/comparison mechanics only; it is not a native observer.
[C3q](scratch/phase-c3q-scene-device-state.md)
records state/cache **requests**, not successful effective pause-device state.
Complete resource create/update/version/lifetime and pre-frame state history,
clears/viewport, effective depth/blend/fog/filter and omissions are not supplied
by this map. No native defaults or source-object identities are invented.

Detailed eight-row inventory, exact source ranges, cited address list and initial
rejected map are retained in `build/tmp/c3at-frame-map/`; accepted corrected
candidate is under `repair1/`. Keep future shared-mechanism work attached to its
earliest unsupported/divergent edge here, with raw provenance and fixed gates.
The [next capture proposal](RENDER_CAPTURE_PROPOSAL.md) requires fresh review and approval.
