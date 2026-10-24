# C3a: full-frame residual breakdown on the A6 repair 4 pause (measure only)

2026-10-23. Directive Q2, one round. Worker note `build/tmp/c3a-residual-breakdown/round-note.md`. New `tools/harness/score_c3_regions.py`. Offline; no
start, no debugger, no game process, no `src/` change. Render from a `git archive` of `95941d4` with `--hud-ticks 70 --pause-caption`.

## Result (reference `original-live.png`, sha256 `603f0…0e03`; numerator is the A6d24 formula; gate numerator <= 4,700,160)

Full frame **49,297,573 / 235,008,000 = 0.2098**. A plain render (no HUD, no caption) scores 49,881,783, so the C2 draws removed 584,210.

| Region | Numerator | Pixels | Share | Bound source |
| --- | ---: | ---: | ---: | --- |
| skill boxes (10) | 5,546,337 | 23,040 | 11.25% | READ live `0x4C9208` (A7 amendment) |
| counters + timer (3) | 836,236 | 2,436 | 1.70% | OURS: glyph extents our renderer prints; positions READ (C2g) |
| caption box | 412,464 | 2,752 | 0.84% | MEASURED live (C2h) |
| actor boxes (3) | 44,746 | 630 | 0.09% | A6d3 overlay-derived boxes, not native rects |
| tube ROI | 708,159 | 59,400 | 1.44% | A4d fixed ROI `[100,80,280,410)` |
| rest | 41,749,631 | 218,942 | 84.69% | complement |
| **sum** | 49,297,573 | 307,200 | | exact; the 18 rectangles are disjoint |

Floor ring: **UNBOUNDED** (PLAN prose only, no rectangle); its pixels are inside `rest`. On `rest`, 94.5% of the numerator sits on pixels where our render is exactly (0,0,0); 99.5% of the skill-box numerator does. That locates black in our render, not a cause. Nothing here attributes a cause to any share.

Limits: our HUD reads Out 5, Saved 0, 04:46 against the original's 3, 0, 04:50 (tick count unmatched). The plain control is 1 unit above A6d24's 49,881,782: five pixels differ by 1 in blue, all outside every HUD box; not investigated. Skill-box, caption and tube-ROI values are unchanged by the C2 draws.

## Facilitator check

Own pure-Python pixel loop (`build/tmp/c3a-fac/check.py`, not the worker's script): all six numerators and pixel counts equal the worker's on both renders (49,297,573 and 49,881,783), disjointness asserted. Reference sha256 equals the A6d24 note. `rest` black share recomputed independently: 39,470,888 / 41,749,631 = 0.9454. The five-pixel difference from A6d24 reproduced against `a6d24-full-frame-gate/actors-on.png`. Mutation alone (tube ROI y1 410 -> 400, run from a copy): tube and `rest` rows change, partition still sums exactly; worker's file sha1 unchanged. Not verified: the build from the snapshot, the glyph-extent boxes against the original, the A6d3 actor boxes.
