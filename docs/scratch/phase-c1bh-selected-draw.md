# C1bh: selected quad drawn; elev_up still fails the gate

2026-10-25, `c1bh-selected-draw`, Cursor round-agent
`19df188d-ef4c-478e-910d-879d7b3db6a5`. Dispatched
12:08:58 MDT, packet ready at 12:19. Worker note:
`build/tmp/c1bh-selected-draw/round-note.md`. Zero stock starts.

## What was drawn

`drawRetailSkillPanel` loads `TEXTURES\^TAB_SELECTED.BMP`, resizes it with
the existing conditional resizer, draws the ten base plates, then one quad
at index 0, then the count text. The rectangle is 18, 12, 90, 44. Index 0
and enable 1 are the C1bg derived fixture values. The pixel-center nearest
sample and straight alpha are still the hypothesis shared with the base
plates. They were not refit.

## Independently reproduced scores

Frozen footprint SHA-256
`e96840e02c3046705c5d1dd19e947a47d7ea4aa65edbe34e31cccafd44b76892`,
23166 pixels, denominator 17721990. Own re-renders match the worker PNGs
byte for byte. The scorer is harness `channel_mae`.

| Gated view | Numerator | MAE | <=0.02 |
|---|---:|---:|---|
| zoom1 | 286491 | 0.01616584819199198 | pass |
| elev_up | 580420 | 0.03275140094312207 | fail |
| elev_down | 287373 | 0.01621561686921164 | pass |

C1bf on the same denominator was 414292, 784866, and 415022. zoom1 and
elev_down now pass. elev_up does not. The gate is not met.

## Facilitator check

Removed only the call `drawSkillPanelSelectedPlate(dest, selected)`.
`skillpanel` then failed once, on `retail panel includes the selected quad
inside 18,12,90,44` (61 checks). Own mutant zoom1 is SHA-256
`5a6a0f64849c674333478eed8604660cd5d25267d29a494d3187ae8a358c491c`,
byte-identical to the C1bf zoom1 image, and scores 414292/17721990.
Restored source SHA-256
`069cfb19c7bfbf3c2438148a7e1d3f2745432b69d45a4c4185f04bb0985d40bf`.
Full CTest after that rebuild is 34/34. No Lemmings process was left
running. The sampler was not promoted to a native measurement.
