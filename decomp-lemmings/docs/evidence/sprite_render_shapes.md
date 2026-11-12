# Adjacent sprite shape verification

Executable SHA-256:
`32675bdb49866e76502713251fdfd2b09078c2481c72ca22ff68ebf4b3737a2f`.

`sprite_render_shapes.cpp` recovers trapezoid `47A565`, anchored rectangle
`47A75D`, and filtering state `470F29`. REA output is retained in the companion
JSON. x87 stores, integer shifts, first-byte orientation access and calling
conventions were checked against instructions.

The anchored shape uses only the first byte at model+1B4. Its vertical offset
depends asymmetrically on orientation: the nonzero orientation adjusts Y again
for half/double scale, whereas the zero orientation does not. This is retained
from the original and is not normalized into a symmetric anchor formula.
Trapezoid mode0 adds a quarter of the piece's native height; it does not use
the rectangle's general scale selection.

Filtering writes the shared alpha-reference global before issuing stage states
16/17. A zero initial mode disables it before resource selection. The anchored
routine enables it after submission regardless of the initial mode, so the
final operation can observe callback changes to capability/device state.

**Verified:** x86 `-O0` and `-O2` each compare60,000 original shape calls
(30,000 each shape), including original filtering and indexing helpers.
Only final submission and device COM are adapted. Complete model bytes,
submitted coordinates/raw endpoint bits, filtering traces and callback mutations
are compared. Numeric NaNs compare by classification; raw endpoint bits compare
exactly. x64 `-O2` and x86 `-O0` units also pass. These are synthetic native
comparisons, not a hybrid gameplay or physical-device test.
