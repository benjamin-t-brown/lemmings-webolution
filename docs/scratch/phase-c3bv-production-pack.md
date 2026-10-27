# C3bv — final-pack mechanism connected to production meshes

2026-10-27. **Accepted conditional SOURCE correction, not native frame acceptance.**
Campaignattempts1/3/4 (initial+two repairs), packing-questionattempts2–4 including
prior C3ak. Fourth exception addressed only the missing actual-call discriminator;
no history/cap reset. Packets `build/tmp/c3bv-production-pack/{round-note.md,
repair1/round-note.md,repair2/round-note.md}` remain intact.

## Implemented and scoped

BackdropScene's mesh submission now calls accepted `packSceneVertex` through
`adaptPackedSceneMeshVertex`: projected XY and UV become f32; native-domain
Z=`SOURCE Pzz*physicalZ-SOURCE bias` becomes f32; RHW comes from reloaded f32Z.
Packed diffuse is decoded in the established R/G/B/A convention. The shared
software depth buffer receives `roundedBiasedNativeZ/Pzz`, **not** f32 physicalZ
and not a conversion that cancels the draw bias.

Caller Pzz comes from the existing camera projection diagonal, not an unconditional
`.3/640` or the billboard triple-product inverse. The current path declares SOURCE
bias0 and UV offsets0; it does not establish native inherited bias/material offsets.
C3bk's clean-reset first-primary default remains conditional, not a universal
holder default. Mesh clipping/bounds binding, native admission/cache/PC/RC/effective
device state remain unknown. Existing un-clipped positive-depth candidates are not
certified actual post-clip native submissions. No billboard/tube/actor/state/order
mechanism changed; no parameter selected by image score.

## Failures and repairs retained

Initial adapter incorrectly passed raw diffuse to SceneRaster and cancelled bias
on conversion back to physical units; it also lacked actual-production regression
and fresh gates. Facilitator returned both defects, not an acceptance. Repair1 fixed
the numerical defects, but still supplied only a helper-level mutation and called
an unset retail environment an unavailable fixture. Installed data were present.
Last repair set the documented environment and added an actual BackdropScene
primary RGB+depth checksum assertion. Its one-call bypass preserves old diffuse
semantics and fails that assertion. No third repair authorized; no missing checks
were silently completed as facilitator implementation.

The helper image-inequality test also varies raw-versus-decoded color in its old
fixture; it is **not** isolated evidence that XY/Z rounding alone changes pixels.
The actual-production bypass and independent numeric replay supply the accepted
mechanism discrimination. Initial link failure and repair1's noncompiling first
mutation remain recorded, not counted as behavioral failures.

## Facilitator check

Independent Python struct/integer oracle executes actual compiled adapter for
**301 cases/3010 fields**, including asymmetric channels, signed/overflowing raw
diffuse values within the supported signed32 domain and nonzero bias. All agree.
Own PE parser re-reads aligned packing/Z-reload/truncation/UV, projection-scale
and bias windows. `46D516 d95a08`, `46D51C d94008`, `46D51F dc3dd8194b00` and raw
0.25 corroborate stored-f32 Z before RHW division; C3ak/bk's precision limits stand.
First incomplete/misaligned read windows are retained alongside corrected windows;
no instruction claim uses their invalid prefix.

Own isolated **BackdropScene call-site** bypass rebuilds successfully, then the
actual retail-fixture test fails its primary RGB+depth output assertion. Restore
is byte-exact, BackdropScene SHA256
`ae05c1e0fdf03ec7d9ae0bec406823a79f98182c60da32657539e2dc40ebb34c`.
Own final full rebuild and **47/47 CTest PASS** (64.72s). Earlier simultaneous
render/link checking caused Windows output-exe sharing denial; retained as a
facilitator sequencing failure. Sequential restored build/tests/renders pass;
no source repair or stock retry was involved.

Independent scripts/logs: `build/tmp/integrated-retail-2026-10-27/pack-check/`,
`check-pack.py`, `score-fixed.py`, `suite-final-first-pair.log`.

## Fresh fixed gates and convergence decision

|Gate|Fresh error/denominator|Result|
|---|---|---|
|C3 repair4, floor included|14934333/235008000 = **0.06354818984885621**|FAIL,0.02|
|C1 zoom1 frozen footprint|285083/17721990 = **0.0160863988750699**|PASS|
|C1 elev_down frozen footprint|285700/17721990 = **0.016121214378294987**|PASS|

C3 numerator worsens by3 from14934330; exactly3 RGB pixels change against the
retained baseline. Zoom1 PNG is byte-identical; elev_down has1 changed pixel
outside its frozen footprint, so its gated numerator is unchanged. No gate,
camera/reference/mask/exclusion/floor change. These are precision-mechanism effects,
not a materially improved full frame or native fidelity acceptance.

At3 campaign C3 attempts (two repairs included), this verifies a production-linked
correction rather than another unused component. **Move to observable C2/B2/B3
integration now**, preserving C3 FAIL and clip/controller parks; do not spend more
campaign attempts polishing these three pixels. Next decisions are runtime HUD
and ordinary cursor/native-navigation observations, not controller repair.

Worker confirmed succeeded/idle on configured Luna/high with no override. Zero
stock starts, debugger/Frida/injection/install/config/save/game writes or live
resources; no restoration owed. Original16:21:30Z round deadline retained across
repairs; exact dispatch/landing/elapsed clocks and budgets are in queue/archive.
