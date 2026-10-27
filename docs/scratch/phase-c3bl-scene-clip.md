# C3bl — bounded clipping component; no production adoption

2026-10-26 local. Fresh implementation extension, cumulative attempts10–11
(initial+one repair); all nine prior attempts remain counted. Directive SHA256
`7fc778e5da4f49e29ed99a1bc8517effcd02d3bf661ccc69a9cf156da70bb4a5`.
**Retained: verified ordinary-finite component behavior. Production integration
stopped at unsupported native clip-bounds binding.** C3 remains failed.

Packets:`build/tmp/c3bl-scene-clipping/round-note.md` (rejected initial),
`build/tmp/c3bl-scene-clipping/repair1/round-note.md`, and
`build/tmp/c3-clip-impl-pass/independent-clip.json`.

## Component versus production

Adds`SceneClip.h/.cpp`, permanent`test_sceneclip`, and only helper/test CMake
wiring. `BackdropScene.cpp` and its test are byte-identical to the pre-pass
copies. The helper is compiled but **never called by production rendering**.
No mesh/billboard/order/material/depth/raster/packing caller changed.

The reusable API takes physical **preprojection** view XYZ and six caller-order
attributes, plus explicit native scale, bias, native near epsilon, projection
X/Y coefficients, centers and signed bottom/top/right/left bounds. It returns
clipped screen XY, interpolable physical view XYZ, separately stored biased
native Z, and attributes. `viewZ` is not automatically the biased physical
comparison depth; the C3bk conversion for that is`biasedNativeZ/nativeScale`.
No RHW/f32 packing or diffuse packing is adopted by this component.

Declared mathematical path, grounded in [C3bj](phase-c3bj-clipping-contract.md)
and [C3bk](phase-c3bk-scale-bias.md):

1. Strict near admission on`scale*viewZ>0`, closing edge first. Sort crossing
   endpoints by pre-bias depth, interpolate physical XYZ/attributes, and overwrite
   only generated physical Z with`nativeEpsilon/scale`. Positive below-epsilon
   input survives. Original-compatible native epsilon is the read2e-6; the API
   requires an explicit value rather than supplying a draw default.
2. Project **after** near clipping with explicit coefficients/centers. Set native
   Z once to`scale*viewZ-bias`. This is an explicitly declared binary64 SOURCE
   arithmetic model, not x87/PC/RC or operation-by-operation native equivalence.
3. Clip bottom/top/right/left, inclusive equality. Sort H/L by clipping coordinate.
   Clipped coordinate is bound; other screen coordinate uses ordinary t.
   XYZ/attributes/stored biased Z use the established p. Zero denominator selects
   H (p0), independently of traversal direction.

Fresh/cache-miss inputs only: no cache-hit emulation, native submission/admission,
device-state or historical repair4 claim. The helper permits explicit source-model
center doubles; correspondence to the native signed-i32 center fields is only
within that integer-center subset. The software vertex-count guard4096 and
magnitude guard are not native limits.

### Binding stop, not an observation prerequisite

Physical source mesh XYZ and the conditional scale/bias law are established.
The C3bk fresh mesh cache reset supports a conditional cache-miss model; lack of
historical memory does **not** by itself prohibit that model. Current CameraBasis
can supply its own SOURCE projection operands without claiming native memory.

But no supported source relationship from the native signed clip globals to
production viewport/CameraBasis bounds was established in the accepted evidence
used here. **That specific bounds binding is unsupported.** No framebuffer bounds
were guessed, no broader input-discovery campaign ran, and integration stopped.
Actual historical P/H/V/b/centers/cache/PC/RC, native route success and effective
device state remain separate unknowns. Final packing stays separate/unadopted;
C3bd/C3ao order, C3am conversion and legacy-unattributed Entrance/Exit stand.

## Initial rejection and repair

Independent compilation of the initial component reproduces two inserted(0,0)
screen corners where C3bj requires first(0,2), then(0,0). The initial screen helper
used the clipping coordinate to interpolate the **other** coordinate. Its API
also accepted already-projected XY, which cannot generally implement fresh near
clipping before projection; zero-denominator endpoint ordering was unprotected.
Initial green self checks did not establish the mechanism. The initial
component was **rejected**, not accepted and then silently changed.

One bounded repair, cumulativeattempt11, retained the initial source/test/note/
failure probe under`repair1/original/`. It supplied the preprojection API,
corrected field-specific interpolation, sorted endpoints, and added permanent
near→projection/asymmetric-X/asymmetric-Y/bias/zero-denominator regressions.
Initial source SHA256`a622f15eb0e73b8eee753daeaf37f4ba2ed473c5d8549434d271016afbfd5628`
and its failure log remain intact. The original task deadline was not reset.

## Facilitator check and explicit narrowing

Read the repaired header/source/test and both CMake hunks. Independently
authenticated30 original clipping/helper windows against the authority; reused
the accepted semantic reads, not new decompiler claims. A separate stdin driver
compiles **actual repaired C++**, while a separate Fraction edge-plane oracle
imports no worker implementation/expectations. **18 cases,756 fields** pass:
near-positive/negative/zero/tiny, near-before-projection, asymmetric planes,
nonzero scale/bias/centers/projection, both zero-denominator traversal directions,
four ordered planes and equality. Its numerical bound is a software-calculator
comparison only, not a native tolerance or precision acceptance.

Two **independent isolated source mutations**, each restored alone:

- Replace screen p by t: permanent CTest fails and the separate oracle fails
  actual output fields. Restoring the single rule restores passing output.
- Replace generated`epsilon/scale` by`epsilon`: the independent scaled case fails
  (`2e-6` versus`1e-6`). **Permanent CTest remains green**: its repaired positive
  fixtures use scale1. Do not claim CTest protects this conversion. The separate
  rerunnable scaled regression/mutation artifact supplies that discrimination.

Restored source SHA256
`ac6db8e76f09737daa6c6fcf2924dd3d2aa3faba945156652d3244ab497b55bb`;
restored CTest and independent18-case replay pass. Logs/scripts:
`build/tmp/c3-clip-impl-pass/{mutate-verify.py,mutation-results.json,verify-clip.py}`.

**Domain narrowing:** the advertised rejection of every intermediate magnitude
above1e100 is **not accepted**. Code checks supplied and resulting scalar fields,
not every temporary. Explicit scale1e100 with three view-Z=-1e100 inputs computes
classification temporaries of magnitude1e200 and returns an empty polygon, not a
domain error (`domain-limitation.json`). The useful acceptance is ordinary-finite
rule behavior in the tested source contract, **not exhaustive API overflow/domain
validation**. The header's near-sort shorthand “biased Z” is also not the accepted
near contract: actual code sorts view Z **before bias**, as required. These limits
are retained honestly; no third repair or general numerical certification ran.

## Fresh gates, preservation and stop

Own restored full build: **45/45 CTest pass** (68.77s); framework **32/32 pass**.
Fresh independent integer/Pillow fixed-gate re-renders:

| Gate | Fresh error / denominator | Verdict |
|---|---|---|
|C3 repair4, floor included|14,934,330 /235,008,000 =0.06354817708333334|**FAIL**,0.02|
|C1 zoom1 footprint|285,083 /17,721,990 =0.0160863988750699|PASS|
|C1 elev_down footprint|285,700 /17,721,990 =0.016121214378294987|PASS|

All three PNGs equal retained baselines byte-for-byte. **No gate movement** and
no changed production render path; no reference/mask/exclusion/camera/floor change.
Own21 fixed/source/install inputs and563 prior evidence files match initial
hashes. All9 unrelated dirty paths survive outside isolated own PLAN lines and
Phase C append. Source changes are confined to the component/test/two CMake files;
BackdropScene and its test are exact. No stock starts/attaches/injection/host tasks,
install writes/packages/live resources; host inventory0Lemming/0LemWeb tasks.
Worker`ses_e84fd4312ffeAKONw2wYsggL7q` confirmed succeeded/idle, configured
round-agent Luna/high without override; no REA target remains open.

Original task dispatch01:38:41.527727Z, repair01:49:55.087400Z, repair worker
completion01:58:17Z; unchanged deadline02:23:41.527727Z includes verification and
landing. Overall start01:35:10Z. Actual verification/landing/end clocks and commits
are in queue/archive. Stop after the bounded component and fresh gate report at
the unsupported production binding. Directive consumed; two attempts spent,
cumulative11. No automatic integration, discovery, observation or follow-on.
