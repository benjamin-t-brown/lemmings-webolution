# C1bi: linear plate sample does not bring elev_up under 0.02

2026-10-25, `c1bi-linear-plate`, Cursor round-agent
`24bb055f-b48f-4b6f-a219-3f442b0f37e2`. Dispatched 12:36:37 MDT. Packet:
`build/tmp/c1bi-linear-plate/round-note.md`. Zero stock starts.

## What changed

`drawSkillPanelPlateQuad` now blends the two texels around
`s = x * 128 / 72` with weights `(72 - rem) / 72` and `rem / 72`, truncating.
The nearest index `min(127, (x * 128 + 36) / 72)` is the same coordinate's
rounded pick and is not the sample position. Y stays the integer row. The
straight-alpha divide is unchanged. Request 2 is the DERIVED LINEAR name
from C1be. These weights are a hypothesis.

## Independently reproduced scores

Frozen footprint SHA-256
`e96840e02c3046705c5d1dd19e947a47d7ea4aa65edbe34e31cccafd44b76892`,
23166 pixels, denominator 17721990. Own re-renders match the worker PNGs.

| Gated view | Numerator | MAE | <=0.02 |
|---|---:|---:|---|
| zoom1 | 285083 | 0.0160863988750699 | pass |
| elev_up | 580972 | 0.03278254868668812 | fail |
| elev_down | 285700 | 0.016121214378294987 | pass |

C1bh on the same denominator was 286491, 580420, and 287373. zoom1 and
elev_down improved by 1408 and 1673. elev_up got worse by 552.

Plate-only pixels, the plate rectangles minus icons and glyphs, are 18033.
Their numerators are zoom1 196523, elev_up 466081, elev_down 194526. The
icon-and-glyph remainder on elev_up is 114891. Even if that remainder were
zero, elev_up would stay above 0.02.

## Facilitator check

Restoring only the nearest pick fails the new test twice, once for the base
plates and once for the selected quad (`got 255`, expected 198). Own nearest
zoom1 is SHA-256
`36274bf1def1b448abfc86827055b2b5d517d10ade4d6601795bd5588af1c580`,
the C1bh image, numerator 286491. Restored linear source SHA-256
`3cb8c08eef21732d5719affd1f28f17f3e3b8d64d375bded63b930ace6d89edf`.
Full CTest on that source is 34/34. No Lemmings process remained.

Omitting both plate draws, then restoring them, makes the footprint worse:
elev_up 2055954 and its plate-only part 1682908, against 580972 and 466081
with the plates drawn. The plates are carrying the view. This kernel is not
the residual. Header names for the landed blend pair 5 and 6 are
`D3DBLEND_SRCALPHA` and `D3DBLEND_INVSRCALPHA`, which is the divide already
in the sampler. That naming was not re-read from the exe.
