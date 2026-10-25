# C1bd: plates drawn; frozen C1 gate fails on all three views

2026-10-25, `c1bd-plate-render`, OpenCode round-agent session
`ses_e8c03dd03ffeAvoHiaT87CcTxw`. Initial dispatch 10:54:15 MDT; repairs at
10:59:11 and 11:07:28. Three dispatches count toward this question's cap.
Worker packet: `build/tmp/c1bd-plate-render/round-note.md`, scorer and PNGs there.
Zero stock starts/attaches/install writes. This implements only the opt-in
`--skill-panel` fixture, not a general accepted HUD.

## Declared candidate, not fitted native state

Production loads `^tab` from the original archive and resizes its decoded72x32
pixels to128x32 via the C1bc conditional p53-nearest model. It draws ten plates
ahead of unchanged count/icon draws. **All-unselected state, pixel-center nearest
quad sampling and integer straight-alpha rounding remain hypotheses**, fixed
before scoring. Successful GDI/packed A8R8G8B8 pitch512 and original FPU state
remain conditional. Nearest precision variants have identical complete `^tab`
streams; no alternative was selected for its score. Selected texture is not drawn.

## Independently reproduced result

The frozen footprint SHA-256 remains
`e96840e02c3046705c5d1dd19e947a47d7ea4aa65edbe34e31cccafd44b76892`.
All rows use23166 pixels, denominator **17721990**. Each reference is that
view's A7p `baseline.png`; cameras are the retained corresponding params listed
in the worker note, with fresh route renders and no injected walkers.

| Gated view | Numerator | MAE | <=0.02 |
|---|---:|---:|---|
| zoom1 |609151|0.03437260713949167|fail|
| elev_up |981264|0.05536985406266452|fail|
| elev_down |610758|0.0344632854436776|fail|

Diagnostic non-gated numerators, same denominator: initial/canonical5142765,
mouse4997260, right5017468, left5432975, elev0_neg5182655.
`MM_DIG_000` is not scored: this packet did not identify a stock reference pair.
This does not assert that no such retained reference exists anywhere.

Disjoint decomposition: plate minus icon/glyph footprint **18033pixels**;
icon/glyph union **5133pixels**. Their errors sum exactly to each gate numerator:

| View | Plate-exclusive error | Icon/glyph error |
|---|---:|---:|
|zoom1|325738|283413|
|elev_up|669975|311289|
|elev_down|323854|286904|

These rank **regions**, not causes. Plate raster/filter/selection is the next
value question; count-zero text (C1v) is a known separate mismatch. Do not
attribute the plate-exclusive error to missing scenery without measurement.

## Facilitator check

`python build/tmp/c1bd-facilitator-check.py` uses an independent strict
rectangle-intersection mask and PIL channel histograms, not the worker scorer.
It reproduces all nine union/error counts and disjoint sums. I independently
rerendered the gated3; PNG bytes match the worker's. Fresh restored hashes:
zoom1 `144c6f925489842e624607c8cf2b07191952b19b156c87c889799243dc7e971b`,
elev_up `5e8a09a89937f60e4bcfcc6751d411ecf96ca1798c90b53a0f702be5d05a867d`,
elev_down `82aa3cf9e3a1fed5b296c5b5b32257ab29f100852588a2a1dfd25daafcab2b5e`.

I changed **only** plate compositor alpha to0, rebuilt the correct test binary
and app, and reproduced exactly one destination-pixel test failure. Own mutant
zoom1 error became **1936862/17721990 =0.10929145090365133**, worse than609151.
Restored via patch; `cmp` against my backup and SHA-256
`be606c6b5b5242fef894bc8c42c6b7f24cf7b614670318d59ac5b0abe746eae2`
prove source restoration, followed by fresh rebuild and **34/34 CTest**.
Real-game plus explicit C1bc stream opt-in independently pass **49checks/0failures**;
complete tab stream matches C1bc. Ordinary tests no longer require ignored scratch.
Synthetic interpolation protects p53 alpha151 against p24 coordinate150.

Narrowings/repairs: initial worker stopped after build without scores. Repair1
fixed float-coordinate precision and supplied the missing experiment. Repair2
removed mandatory ignored-fixture dependency and corrected overlapping region
labels. No fourth dispatch or fitted pixel correction is authorized by this task.
The worker's new4536-cell stratified scenery-gap sample is **not** the historical
4536-pixel diagnostic: I discard that comparability claim. My independent complete
inter-band gap minus footprint is **5058pixels**, out of the gate; its errors are
41327/51110/38688 on zoom1/elev_up/elev_down (MAE0.01068/0.01321/0.01000).
No mask or gate changed. Whole-frame outside-footprint is not labelled scenery.

No new original address claim beyond landed C1bc was needed. Independent process
check found no Lemmings/Frida process. All five install hashes (exe, patch.dll,
box, save, wrapper) match C1bc pre-run values. Earlier retractions remain intact.

## Next

C1 remains **not accepted**. Read/derive the plate draw's numerical texture-filter
and alpha state before changing the provisional raster, ranked by plate-exclusive
error. Known zero-count text can be corrected separately with the established
C1v branch and a scored regression. No gate amendment, live start or alternative
chosen by score follows from this failed candidate.
