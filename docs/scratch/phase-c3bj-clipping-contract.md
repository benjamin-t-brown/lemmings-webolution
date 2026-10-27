# C3bj — shared clipping rule compared; integration inputs remain open

2026-10-26 local. **Accepted READ/SOURCE comparison, no production correction.**
Cumulative attempt8; seven prior attempts remain counted. Approved delegated Sol
pass, directive SHA256
`eb777498f9abad5b8c9ec11e686b778a32dd36077311cbac5f7700b3cc1842ec`.
Packets: `build/tmp/c3bj-clipping-contract/round-note.md` and
`build/tmp/c3-clipping-pass/independent-comparison.json`.

## Result and limits

Production's scene mesh path is **not algorithmically equivalent** to the bounded
native near/screen clipper. `BackdropScene.cpp:77–83` fans first and rejects a
triangle if any physical view Z is nonpositive. The native near route clips the
polygon, retaining positive portions and inserting crossings before projection.
`SceneRaster.cpp:121–150` bounds pixel iteration but generates no clipped vertices;
its header explicitly requires already-clipped inputs. This is an upstream
integration gap, not a violation of that raster API's promised clipping behavior.

Screen clipping is not generally replaceable by framebuffer bounding when the
subsequent depth is affine in screen coordinates. Independently derived synthetic
example, **not an actual repair4 draw**:

- A=(-2,0,Z2,U0), B=(2,0,Z4,U1), C=(2,4,Z4,U1); clip X>=0.
- New corners (0,2) and (0,0) have Z=8/3, U=1/3.
- At (1/2,1/2), unclipped affine Z is13/4=**3.25**; clipped fan Z is**3**.
  Both paths yield U=5/11 and red100 when corner red=220*U.
- A separately compiled facilitator driver against unchanged `SceneRaster.cpp`
  reproduces both depths and equal color. This establishes a SOURCE intermediate
  discriminator, **not GPU/native execution, pixel causality or visibility**.

The C3n reduced candidate agrees on six near cases, eight individual screen-plane
cases and four ordered stages, within their finite cache-miss/software arithmetic
domain. Its2077 constructed records remain **uncertified native submissions**.
Four selected C3bi corners outside retained bounds make clipping relevant, not the
cause of C3 error. No expanded census or source-removal sweep ran.

## Original rule, independently re-read

Authority digest remains `32675bdb49866e76502713251fdfd2b09078c2481c72ca22ff68ebf4b3737a2f`.
Own raw bytes/disassembly: `build/tmp/c3-clipping-pass/own-pe/`.

- Conditional near route `46D2DA..46D371` calls `46CB3C`, replaces the nonempty
  pointer polygon, then `46D372..46D3C0` projects before screen clipping.
- `46CB97 dd4110`, `46CB9A dc1d38034b00`, `46CBA2 f6c441` and the following
  branch establish finite **Z>0** admission, not Z>=epsilon. Closing edge first;
  insert crossing before the retained current vertex. Exact zero is outside;
  positive below epsilon is retained. Duplicate zero-corner intersections stand.
- Near intersection `46D620..46D69A` uses H+(L-H)*t with t=-HZ/(LZ-HZ).
  `46CC03..46CC15` and `46CC97..46CCA9` overwrite generated Z from raw f64
  `4B19D0`, bytes`8dedb5a0f7c6c03e` =0.000002. XYZ, screen cache coordinates,
  UV, RGB and alpha interpolate. `46D858..46D892` ANDs source cache-byte truth
  values; the reduced candidate is not general cached-record emulation.
- `46D3C1..46D45F` uses signed-i32 `fild` bounds in order: Y<=bottom at
  `58D288`, Y>=top at`58D28C`, X<=right at`58D284`, X>=left at`58D280`.
  Finite equality is inside. Four bodies `46CCE2`, `46CE60`, `46CFDE`, `46D15C`
  alternate pointer buffers and stop when empty. These global **values are not
  inferred from the file**; `.data` raw backing ends at`4C6000`.
- Screen helpers `46D910..46DB76` and `46DB80..46DDE6` sort H/L by clipping
  coordinate C. For bound B, t=(B-HC)/(LC-HC). Other screen coordinate uses t.
  XYZ/UV/RGB/alpha instead use
  p=(B-HC)*HZ/((B-HC)*HZ+(LC-B)*LZ). A zero denominator leaves p=0.
  The six-dword XYZ copies at`46DA5D..46DA65`/`46DCCD..46DCD5` include Z.
  Subtract/scale/add/copy helper bytes were also re-read, not inferred from names.
- Projection `46DDF0..46DE66` retains the accepted cache gate, signed centers,
  XY ratios before subtracting camera+770 bias, then sets cache byte1.
  Screen weights therefore consume **already-biased Z**.

These are finite ordinary-case READ semantics, not NaN/exception/precision or
actual route/admission acceptance. Final packing remains the settled C3ak rule;
its app adoption gap is not silently repaired here.

## Correction decision and exact unknowns

**No production correction selected.** Choosing the missing input values would
exceed the independently supported comparison:

1. Actual per-record transform/projection stack, animated ancestors and native
   positive scale s relating pre-bias Z to physical Z. Generated near epsilon
   converts to epsilon/s; applying2e-6 directly in physical units invents s.
2. Per-draw bias b at camera+770. Screen interpolation uses s*physicalZ-b;
   cancellation requires known b/s. Zero bias is only this synthetic example.
3. Signed centers+754/+758, clip-global values, projected-cache+58 and cached XY,
   actual pre/post-clip records. C3n/A1 saved context is not these repair4 inputs.
4. Runtime x87 PC/RC and accumulated clipping precision. The worker's initial
   unequal-depth serial case exceeded its overly strict software calculator
   bound; failure is preserved. No native tolerance was widened or established.
5. Native admission/submission/Device3-wrapper route success and effective depth
   enable/write/function/bias/range/format/quantization/clear state. C3bi's physical
   software-depth coherence is not effective native-depth equivalence.

An explicit-input native-domain clipper is possible; integrating it with guessed
scale/bias/bounds/cache is not justified by this result. Observer refusal remains
intact. No automatic f32 adoption, depth tuning, order/material rollback or live
work; Entrance/Exit stay legacy-unattributed. These unknowns are stop inputs, not
authorization for another round.

## Facilitator check

Authenticated all30 worker byte windows at26 distinct addresses against fresh
own PE reads, then semantically re-read predicates/order, intersections, field
helpers and projection. Additional raw count/index accessor reads corroborate
the closing-edge walk. Separate Fraction checker imports **no worker arithmetic**:
reciprocal-depth/homogeneous reconstruction independently reproduces all18
cases/stages and determinant-derived headline barycentrics. A separate C++ driver,
fed independently derived corners, confirms3.25/3 with red100/100.

Own isolated mutation changes only screen interpolation weight p to ordinary t
in a scratch candidate copy. The checker fails the actual left-crossing field
assertion (`2.0` instead of`0` for source X); exact restoration returns passing
output and SHA256`3fdd353b3e3abb7d477bd0a4d3cc511ca5c837a06fec50b871518324ef71b041`.
This protects the comparison, not a production regression or native fidelity.
No production file was edited. Logs/scripts are in`build/tmp/c3-clipping-pass/`.

Fresh full build and **44/44 CTest pass** (final65.07s); framework **32/32 pass**.
Own independent integer/Pillow scorer re-renders the fixed gates before/after:

| Gate | Error / denominator | Verdict |
|---|---|---|
|C3 repair4, floor included|14,934,330 /235,008,000 =0.06354817708333334|**FAIL**,0.02|
|C1 zoom1 footprint|285,083 /17,721,990 =0.0160863988750699|PASS|
|C1 elev_down footprint|285,700 /17,721,990 =0.016121214378294987|PASS|

Each PNG equals its retained baseline byte-for-byte. **No gate movement**; no
reference/mask/exclusion/camera/floor change. Own21 fixed/source/install inputs,
264 prior evidence files and the worker's217-file inventory independently match.
All9 initial dirty paths are preserved, excluding only isolated own PLAN lines
and Phase C append. Fresh host inventory:0Lemming processes,0LemWeb tasks.
Worker succeeded/idle confirmed through current API; own REA status open=false.
Zero starts/attaches/injection/scheduled-task creation/install writes/packages,
and no live resources or restoration writes owed.

Owner`ses_e854d421effeWpblsoJtCyVVFT`, parent`ses_e85509715ffeYQGoROQjsGEKnk`;
self metadata confirms configured facilitator Sol/high. Worker
`ses_e854837a5ffeF4t1ch6PTSaKGa` used configured disasm-agent Sol/high, completed
00:29:02.068382Z (724.772254s after dispatch). One attempt this pass, cumulative8;
no repair or correction. Original45-minute task deadline01:01:57.296128Z remains
unchanged. Actual verification/landing/pass-end times are in queue/archive.
Stop at verified comparison/exact integration blocker; directive consumed despite
active heading. Unused attempt9 is not permission to continue.
