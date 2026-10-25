# C3ab — explicit-input software scene raster

Implementation only, not native GPU fidelity. Packet:
`build/tmp/c3ab-scene-raster/round-note.md`. The separate SceneRaster API requires
texture, target/depth, vertices and every state option; it changes no app render.
POINT/LINEAR, WRAP/CLAMP, strict alpha, blend factors, depth comparisons/write,
FLAT/GOURAUD and alpha SELECTTEXTURE/MODULATE are explicit software operations.

## Facilitator check

Read full source/header/test. Independent C++ driver and Python exact-rational
oracle (`build/tmp/c3ab-facilitator-dump.cpp`, `c3ab-facilitator-check.py`) compare
**5,120 cases / 61,440 pixels**, varying all eight depth functions, write enable,
filter, addressing, alpha test, ten blend configurations, shade and alpha operation.
All RGBA values and depths match; zero halfway binary64 rounding discrepancies.
This verifies the declared component arithmetic, not the native raster.

Compiled private single-field `>` to `>=` alpha mutation. Equality-alpha fixture
fails with `strict equality boundary changed output/depth`. Restored private copy
to production SHA256 `8205172b36d80a47f3f07e418cae295c39a1748069f92c3a5452629be68b754a`.
Production source was untouched by this independent mutation. Worker also executed
an isolated permanent-test regression. Final shared-mutex build and CTest passed
**38/38**, completed 16:26:38 MDT.

Limits: inclusive edges can double-blend shared edges, sample offsets are caller
choices, Gouraud diffuse is perspective-interpolated, RGBA rounds nearest. Native
coverage/precision are uncertified. Fog/specular/mips/culling/clipping are outside
this API. No device defaults or historical pause state are established here.
Only this round's source/test and own build-wiring hunks land. App integration
must label conditional conventions and score explicit repair4; baseline remains
0.2050523 > 0.02. Zero original starts/attaches/native calls; no live resources.

Dispatched 15:57:12; worker completed 16:13:15; facilitator accepted 16:29:24 MDT
(32m12s dispatch-to-acceptance; worker implementation 16m03s).
