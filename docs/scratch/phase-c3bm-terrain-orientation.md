# C3bm — low terrain skirt orientation check

2026-10-26 local. **Scoped conditional negative; the user's overall visible-sand
question remains indeterminate.** No production correction or other source edit.
Directive SHA256`371b0b96cf5100d9d9973ec765ab0dbe5886a820c8bd916f817ce3fb72195e53`;
sole cumulativeattempt12, prior11 retained, no repair. Historical orientation
task C3ar was interrupted and explicitly discarded incomplete, not a settled
negative. Its brief and partial outputs were recovered/read-only. This is the
second historical orientation dispatch, not a reset or a new historical question.

Worker packet:`build/tmp/c3bm-terrain-orientation/round-note.md`.
Facilitator check:`build/tmp/c3-terrain-orientation-pass/independent.py` and
`independent.json`. Authority exe SHA256 remains
`32675bdb49866e76502713251fdfd2b09078c2481c72ca22ff68ebf4b3737a2f`;
raw LVL SHA256`95f84f86fb53ab29ce2ce3aed794cf69a06e5754d42ef78657f3af1a5db4c567`.

## Exactly which terrain was tested

Primary root`59644` has **three sibling leaves**:`596c7`(sand7),`8adc9`(base),
`8e29c`(top). Base/top are **not descendants of sand7**. Root child word`596c3`
is3; leaf child counts are0. Do not reuse the early partial C3d shorthand as a
confirmed nested hierarchy. Accepted C3n/C3aa source layout remains the basis.

Within node`596c7`, test only the exclusive connected source-index component
**564..623:60 vertices,72 records,256 corners**. Position bytes are
`5cbaf..5d14f` half-open; exact record/edge lists are in the packet. No record
outside that component references these indices.

This identity is geometric, not a guess from a texture name or screen location:
it has a closed12-edge inner boundary winding one full turn around the cylinder
axis, radius60.45679634205102..60.45682290337225 at Y≈-11.81934568, expanding into
an irregular low skirt. Overall radius60.4568..221.2799; world bounds:

|Axis|Minimum|Maximum|
|---|---:|---:|
|X|-163.43870757840597|178.213320743899|
|Y|-39.28431749343872|4.070850610733032|
|Z|-216.40554071935804|162.26561476655945|

Sand material strings corroborate, but do not select, this shape. Aliased,
degenerate and multiply incident edges exist: **not a clean-manifold claim**.
Tube geometry is separate LVL pool`f4b..9de7`,448 lattice/897 surface vertices,
radius50/60,height96, consumed by the TubeScene path. Base/top siblings and all
other source components remain outside the test. No material string is promoted
to native attribution of the user's wooden platforms; their inputs stay exact.
This is **not all of the sandy scenery visible in repair4**, nor native pixel
attribution of this component.

## Original-supported intermediate, not parser/renderer self-consistency

Reuse [C3n](phase-c3n-mesh-submission.md), [C3aa](phase-c3aa-scene-mesh-source.md)
and [C3bk](phase-c3bk-scale-bias.md). Freshly authenticate11 retained raw original
windows; re-read stack/affine/XYZ instructions. Holder`474974` pushes B; root
`46b6eb` pushes node+8 and child slot4 dispatch is`46b731`. Mesh`46c55d` pushes
its local affine,`46be80` copies the composed matrix,`46c641` calls`40eae0`.
Stack`43bc10` and retained wrapper`43bca0` lead through`482210`/`4128e0` to
the row-vector Hnode*Hancestors*B chain. XYZ helper`40eb50` reads matrix columns
at0/18/30,8/20/38,10/28/40;`40eae0` adds translation at+48. Fresh supplemental
wrapper read is verification of an already-established body, not new discovery.
Worker used **zero new substantive native bodies**, no new REA query/session.

Raw matrices at`8ad4c` and`59648` give Hnode=identity+translation
`(35.512168884277344,-1.8937009572982788,-81.18522644042969)`, Hroot=10I:
the fixed original-supported world relation is **10*(local+t)**, no reflection.
This is a conditional source-pose chain, not actual repair4 runtime H recovery.

Front/back is signed horizontal depth **relative to the cylinder axis at the same
height**:`wx*B[2]+wz*B[8]`, not screen X or an absolute-depth/elevation mixture.
B/P come from raw-hex A1 fixture operands, whose provenance is retained reference,
**not repair4 memory**. Pzz>0; dividing by it expresses physical-equivalent depth.
The other arm uses unchanged current CameraBasis.worldToView, the same API matrix
passed at`BackdropScene.cpp:71`, **built from camera_a1/camera-fixture-params.json**.
Both view arms are A1-fixture checks, **not saved repair4 camera-parameter replay**.
The world-chain negative is camera-independent within the stated H assumptions;
the signed-depth comparison is limited to these fixture operands.
Representative source-depth extrema plus564:

|Vertex|Original-supported world XYZ|Fixture signed depth/Pzz|Current A1-fixture signed depth|
|---|---|---:|---:|
|573|(-136.23218243425583,-15.974127054214478,138.94567446060222)|-190.8156013155615|-190.81559330312245|
|570|(147.12009222392865,-14.635652303695679,-146.34986289980588)|203.4998263017469|203.49981757934972|
|564|(58.90562696834486,4.070850610733032,62.77633516461236)|-2.684036294501852|-2.68404206932739|

**All60 fixture front/back signs agree.** This establishes the tested conditional source
orientation, not native instruction rounding or bitwise equality.

## Facilitator check and discriminating control

Read candidate scripts/probe plus actual source consumer. Separate checker imports
no worker geometry/oracle/expectations. It reuses the **accepted C3n layout table**,
freshly re-reads raw corner-index words and matrices, verifies connectivity and
the12-edge cycle, and independently evaluates rational **4×4 homogeneous matrices**.
Freshly compiled probe calls actual Camera/SceneMeshSource/SceneNode/Cylinder code;
all256 corners match fixed world, current A1-fixture V and retained-native-B intermediates.
Maximum differences respectively5.68434e-14/2.27374e-13/5.68434e-14 are software
algebra differences, **not native FPU tolerances**. Original instructions and raw
operands supply the expected relation; parser/renderer agreement alone is not
being treated as native evidence. No old C3ar landmark/counter/conclusion adopted.

Own independent **single terrain-only front/back control**, not a score-selected
flip: reflect horizontal world positions in the plane through the cylinder axis
whose normal is(B[2],B[8]), then invert the known H-chain into a private LVL copy.
Only120 component X/Z scalar slots change(819 differing bytes); fixed original
expectations stay unchanged. **256/256 corners rejected;256/256 front/back signs
flip**. All2979 other primary corners, tube header, camera and all other input
bytes remain exact. Worker also checked signed XZ triangle orientation reverses.
Normals are intentionally unchanged: this is a coordinate discriminator, **not a
renderable correction or predicate experiment**. No screenshot trial/sweep ran.
Both private LVL copies restore to original bytes; own actual C++ replay restores
**exact baseline output text**. Originals and production were never mutated.

**Narrowing:** no exact native per-platform/submission/pixel attribution, no all-
terrain negative, no actual repair4 H/V/P/b/cache/admission or x87 precision claim.
Visibility/order could change appearance without this source chain reversing;
those mechanisms were not investigated. The broad user hypothesis is therefore
**still indeterminate**, despite the supported negative for this low skirt.

## Gates, preservation and stop

Fresh own build/CTest **45/45 PASS**,68.36s; framework lint passes. No production
change and **no new render/capture**. A fresh integer rescore of **retained previous-
pass PNGs** reproduces C3`14934330/235008000=0.06354817708333334` **FAIL**;
C1 zoom1`285083/17721990=0.0160863988750699`, elev_down
`285700/17721990=0.016121214378294987` **PASS**. **No gate movement.** PNG hashes,
references/masks/exclusions/cameras/floor remain unchanged; these are not freshly
rendered scores.

Own pre-pass ledgers preserve175 fixed/source/install inputs,283 selected prior
evidence files, and9 unrelated dirty paths outside own isolated PLAN status/link
and Phase C append. Worker separately preserves474 files. No source/test/CMake,
install/save/reference edit. Zero starts/attaches/injection/host tasks/packages,
no owned live/debug/REA resource; separate host inventory0game/0LemWeb tasks.
Worker`ses_e84d3c359ffeByyuXM62sQQPas` confirmed succeeded/idle, configured
disasm-agent Sol/high, no override/fallback. One dispatch, no repair/follow-on.

Start02:19:16Z; dispatch02:24:04.889438Z; worker release02:38:11.888819Z.
Original overall deadline03:04:16Z never reset. Actual verification, landing,
cleanup/end clocks and commits are in queue/archive. Stop at scoped result;
directive consumed, cumulative12 with historical C3ar explicitly retained.
