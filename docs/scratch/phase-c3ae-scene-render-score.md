# C3ae initial/repair1 — partial conditional scene integration and measured score

2026-10-25. **C3 still FAILS.** This lands a runnable, explicitly conditional
software candidate and its diagnostic score, **not complete brief acceptance,
native submission equivalence, or a finished read/DERIVED scene**. The remaining
known integration omissions below must not be relabeled as merely unknown GPU
precision. No fitted parameter or gate/mask change is accepted.

Packet `build/tmp/c3ae-scene-render-score/round-note.md`, initial candidate retained
under `before-repair1/`. Initial wrong-actor/no-panel legacy-raster render was
rejected. Repair actually uses MeshSource/SceneRaster/BillboardSource, distinguishes
304 null names from missing assets, adds a retail integration test, and preserves
unsupported-level and scene-off legacy behavior.

## Facilitator check

Read every changed production/test/wiring file and pre-score condition manifest.
Fresh app commands in `build/tmp/c3-facilitator-render.ps1` use the **repair4**
camera, `injected-plus8.json`, `--walkers --skill-panel --hud-ticks70` (with normal
CLI spacing) and pause caption. The scene-off PNG exactly reproduces the old
correct baseline SHA256 `13b1bd501803c3bf3e210cac651b1968083b26c74a852925e04e5e6f44e3829e`.
Fresh scene-on and restored PNGs both equal worker SHA256
`b364b0f24270a9d34b65c5c708b3c32e3a44030886ca10fd98159232319d3885`.
Scene-on changes121287 pixels relative to that scene-off control.

Required `score_c3_regions.py --reference` replay and a **separate Pillow
histogram/crop checker** (`build/tmp/c3-facilitator-image-check.py`, importing no
production scorer) agree on the entire table below. Reference SHA256 remains
`39ba74328224a44ab1ae392790dd191303eb399492162d164c104a577fb99053`.

| Region | Numerator | Pixels |
| --- | ---: | ---: |
| rest, floor included |19234589|218942|
| skill boxes |1484307|23040|
| tube ROI |682387|59400|
| counters/timer |154713|2436|
| caption |136273|2752|
| historical actor diagnostic boxes |15134|630|
| **Full frame** |**21707403**|**307200**|

**21707403 /235008000 =0.09236878319035947**, versus baseline
48188934 /235008000 =0.2050523131127451. Unchanged gate0.02 requires numerator
<=4700160, so this is not close-out. Rows partition exactly; actor boxes remain
historical C3a diagnostics, not new repair4 native actor bounds.

Independent single yaw32->33 replay produces23875747 (+2168344 error) and changes
284258 pixels. All parsed private-camera fields were restored to the original;
capture camera/source were never written. Restored PNG hash matches scene-on.
Source SHA256 `900f18be8ce965aecd666af9bd3a102e75a3d23cc1e1799386f58062dc876d33`
matches worker's exact source restoration. Final shared-mutex build and **39/39
CTest pass** completed17:19:21. The independent visual perturbation, rather than
the worker's broad `predicateRejected>0` assertion, establishes pixel sensitivity.

Fresh app statistics reproduce4 roots/1177 mesh records/1595 submitted software
triangles (including billboard triangles),304 null records/440 null triangles,
17 billboard sources,265 predicate rejects,0 missing named resources and275901
depth-written pixels. Fresh output reports63 other rejections, **not packet64**.
These are component diagnostics, not certified native submissions or visibility.
Roots are guarded by caller level ID, minimum extent and parsed end offsets;
the error text says "hash guard" but **no full file hash check is implemented**.
Texture decode/caching occurs per render, not an accepted once-built resource cache.

## C1 fresh reports, frozen footprint unchanged

`build/tmp/c3-facilitator-c1-report.py` independently rerenders all10 views into its
own directory and computes histogram errors over the frozen23166-pixel footprint.
All match repaired worker results; denominator17721990 for each:

| View | Numerator | Score | Status |
| --- | ---: | ---: | --- |
| zoom1 |285083|0.0160864|gated, passes|
| elev_down |285700|0.0161212|gated, passes|
| elev_up |444037|0.0250557|reported, fails0.02|
| initial |1488006|0.0839638|reported|
| canonical |1488007|0.0839639|reported|
| elev0_neg |1486685|0.0838893|reported|
| mouse |1676110|0.0945780|reported|
| left |1397223|0.0788412|reported|
| right |1601744|0.0903817|reported|
| MM_DIG_000 |1795145|0.1012948|reported, unchanged unsupported backdrop|

Worker initially claimed camera JSONs unavailable; facilitator supplied exact
existing closure paths, no conversion or new capture needed. Worker also reports
an initial copied script wrote into prior closure output; do not treat that
directory's modification times as immutable old evidence. Prior committed scores
and our new separate fresh output establish the comparison; original reference
captures/footprint were not changed.

## Important narrowing: known incomplete integration, not just runtime unknowns

- Billboard traversal supplies physical camera directly for each node, **omitting
  parsed ancestor composition**. Source S is identity and projection uses physical
  focal/view-Z space; this does not replay the accepted C3o17-node source tuples.
- `collectEyes` starts with identity instead of the accepted camera-composed
  context. Its265 rejections do **not** reproduce C3p's conditional159-skip
  reference census. The worker's "diagonal S cancels" statement does not verify
  equivalence of the full input chain. Do not promote it as established.
- C3y bias-dependent sorted child dispatch and type1 radius/near dispatch are
  omitted. Native near/screen clipping, f32 packing/RHW reload, context bias and
  material-cache request state are not replayed. Physical view-depth sharing is
  an explicit software convention, not a derived effective device Z model.
- The raster options are declared software choices: LINEAR/WRAP, LESS/write,
  Gouraud, MODULATE, sample0.5; billboard `>239` is **not** a proof of C3r caret8
  material state. Successful-normal-route request tables Q/R remain authoritative;
  don't mistake this candidate for their complete implementation.
- Color code rescales source values in[0,1] to255, whereas C3n packing truncates
  original raw channels. Accepted source table contains3 channels exactly1.0;
  this is another known packing mismatch, not a new measured color convention.
- AA/AC retail targets are **still absent** from the configured CTest ENV list.
  New backdrop retail test runs; generic39/39 does not prove those two retail
  arms ran. Their earlier explicit-install independent checks remain valid.
- The floor stays included in every full-frame score. T's conditional source-family
  bound `[69,0,569,480)`,55571 pixels, is retained as context only; it neither
  establishes this candidate's exact visible floor nor becomes a score exclusion.

The actual frame visibly retains upper-band/floor and background errors. By the
fixed partition, `rest` dominates, then skills and tube. **Next fix known
transform/dispatch/packing/state integration before fitting texture precision or
requesting further original readers.** Actual pause gates/S/format remain unknown
because AD's live controller was rejected in drycheck. No starts/native/DLL/REA
calls or live resources were used by this task. Separate game/task check is empty.

## Landing / budget

Accept the runnable conditional candidate, independently measured score/C1
nonregression and explicit remaining defects; do not close the original brief or
C3 fidelity gate. Original plus repair count as2 attempts; with AD they exhaust
24/24 for this run. No further repair is dispatched under that budget. All earlier
retractions and successful-normal-route conditions remain.

Dispatch16:34:28, repair16:52:49, narrowed acceptance17:22:12 MDT:
**47m44s total**, **29m23s repair-to-acceptance**, including facilitator checks.
