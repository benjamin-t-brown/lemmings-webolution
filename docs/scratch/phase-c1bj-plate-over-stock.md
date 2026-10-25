# C1bj: plates drawn over the stock frame raise the plate-only error

2026-10-25, `c1bj-plate-over-stock`, Cursor round-agent
`3ba69d52-2c22-4d9f-95d4-182efddb77a4`. Dispatched 12:51:00 MDT. Packet:
`build/tmp/c1bj-plate-over-stock/round-note.md`. Zero stock starts. Production
`SkillPanel.cpp` was not edited. Its SHA-256 is the C1bi hash
`3cb8c08eef21732d5719affd1f28f17f3e3b8d64d375bded63b930ace6d89edf`.

## What was measured

The scratch composite copies each stock baseline and draws the current base
plates and the selected quad onto it. Counts and icons are omitted. The
baseline already contains the original plates, and the worker recorded that
before the run. The sampler and the straight-alpha divide are the C1bi
hypothesis.

## Independently reproduced scores

Harness `channel_mae`, footprint SHA-256
`e96840e02c3046705c5d1dd19e947a47d7ea4aa65edbe34e31cccafd44b76892`.
Full footprint 23166 pixels, denominator 17721990. Plate-only is 18033 pixels.

| View | Full numerator | Plate-only numerator |
|---|---:|---:|
| zoom1 | 1123813 | 669750 |
| elev_up | 1145908 | 691289 |
| elev_down | 1114207 | 666306 |

PNG hashes match the packet. C1bi plate-only numerators, plates over our
frame, were 196523, 466081, and 194526. Drawing our plates onto the stock
frame is worse on every view.

## Facilitator check

`prepare_raw.py` writes the baseline RGBA, and `composite.cpp` calls
`drawSkillPanelPlates` then `drawSkillPanelSelectedPlate`. The source hash
matches the C1bi tree, whose suite was 34/34. This round did not change it.
The worker also reported 34/34.

Narrowing: this is a second plate layer on top of the stock plates, not our
plates over a plate-free stock backdrop. The rise does not by itself say
which of the two layers is wrong.

On the current elev_up render, the plate interior (16035 pixels) is 384401
of the 466081 plate-only numerator. The same interior with the plates omitted
is 1589553. Forcing plate alpha to 255, then restoring the sampler, raises
elev_up to 2963460 full and 2519199 plate-only. The derived transparency is
reducing a large under-layer difference, and an opaque plate is farther from
the stock frame. The icon-and-glyph share on elev_up is 114891. The pass
line is 354440 (0.02 of 17721990), and 580972 minus 114891 is still 466081.
Count tint and icon edges cannot pass elev_up on their own.
