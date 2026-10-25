# C1 closure under the third gate amendment

2026-10-25, facilitator-only acceptance check; no new panel round or stock start.
Authorization: active DIRECTIVE step 0 and PLAN Phase C user decision. The
amendment precedes this check: only zoom1 and elev_down are gated at 0.02.

## Facilitator check

`python build/tmp/c1-close-check.py` freshly renders all ten views from the
landed app, constructs a separate strict rectangle/pixel-square intersection
mask, and sums PIL channel histograms. It imports no worker scorer. Frozen
footprint SHA-256 `e96840e02c3046705c5d1dd19e947a47d7ea4aa65edbe34e31cccafd44b76892`;
23166 pixels, denominator 17721990. SkillPanel.cpp SHA-256
`3cb8c08eef21732d5719affd1f28f17f3e3b8d64d375bded63b930ace6d89edf` matches
the directive. Fresh gated PNGs and elev_up match C1bi byte-for-byte.

| View | Numerator | MAE | Role |
|---|---:|---:|---|
| zoom1 |285083|0.0160863988750699|gate passes|
| elev_down |285700|0.016121214378294987|gate passes|
| elev_up |580972|0.03278254868668812|reported|
| initial |4790019|0.27028674545014414|reported|
| canonical |4790019|0.27028674545014414|reported|
| mouse |4651177|0.26245229796428055|reported|
| right |4673622|0.26371880358808464|reported|
| left |5098140|0.287673111202523|reported|
| elev0_neg |4829581|0.272519113259854|reported|
| MM_DIG_000 |1795145|0.10129477558671458|reported|

Each A7p reference is `build/tmp/a7p-unpause-then-masks/<view>/baseline.png`;
the diagnostic second-level reference is
`build/tmp/a7q-second-level-mask/no_camera_input/baseline.png`, using its
`params-mm_dig_000.json`. This resolves the earlier packets' missing reference
path, not their historical scores. A7q's capture note identifies the saved
baseline/suppression/restoration experiment. Exact camera paths, image hashes
and all scores: `build/tmp/c1-close-check/scores.json`.

An isolated +6-pixel horizontal shift of the fresh zoom1 image raises its
numerator to 1389089 (0.0783822245695884), failing the gate. This temporary
in-memory image test changes no source or saved original. Prior C1bi's isolated
source regression and restoration proof remain in its note; no production
behavior changed here. Fresh full build and CTest: **34/34 pass**. No Lemmings
process existed; no install file, live memory or capture was changed. No new
native-address claim is made; historical retractions and conditional raster
assumptions stand.

**C1 is done for the amended scope**, not for all views, all levels or the full
frame. Continue to C3 scenery under the current directive; elev_up and the other
large reported residuals are not masked or declared solved.
