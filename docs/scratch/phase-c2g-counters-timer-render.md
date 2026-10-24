# C2g: Out / Saved / timer rendered and scored on A7 (Q2 round 3 of 3)

2026-10-23. Worker note `build/tmp/c2g-counters-timer-render/round-note.md`. Offline; no stock start, no debugger, REA not used. Four further
bodies read (`0x44D860`, `0x414770`, `0x4145A0`, `0x414580`), cap met. New `src/shell/HudText.{h,cpp}`, `test_hudtext`,
`tools/harness/score_c2_hud.py`, and a headless `--shot` overlay in `main.cpp` (`--hud-ticks N`, diagnostics `--hud-out N`, `--hud-colour R,G,B`).

## What is drawn

| Item | Value | Tag |
| --- | --- | --- |
| Counter right end | 620 (`be 6c 02 00 00` at `0x44D288`); left = 620 - text width | constant READ; right end versus centre INFERRED (`[g+0x20]` / `[g+0xF0]` unread) |
| Out y / Saved y | 15 / 50 (`6a 0f`, `6a 32`) | READ |
| Timer | `[0x4CBAAC] idiv 60`, `%02d:0%d` / `%02d:%02d` at 560,430 | READ |
| Text | digits replace the words | READ (C2d) |
| Saved colour | white below target, 255,63,63 at or above; the sim's `saveTarget` stands in for word `0x4CBAB0` | doubles READ; R,G,B order and the target relation UNREAD |
| Out and timer colour | white | UNREAD default |
| Out value | sim spawned - saved - dead (5 at 70 ticks) | the original's sum over `this+0x5070` is UNREAD |

The reference reads Out 4 on eight captures and 5 on elev_down and elev0_neg. Both are reported; nothing was painted per view. The `[0x4E33C8]==1`
timer skip is not modelled.

## Result

- **Amended A7 gate** (caption and skill panel excluded): every view passes. Numerator unchanged on 7 views; lower on zoom1 (3734978 -> 3578270),
  elev_up (2497768 -> 2287116) and elev_down (2168594 -> 2095758). The other views' HUD boxes hold no gate-mask pixels, so the gate is nearly blind to
  this change. MM_DIG_000 is 2606399 both ways, still passing.
- **Region residual** (all pixels in each glyph box, supplementary): down in 27 of 27 view-and-region cells, e.g. initial Out 215195 -> 151413, Saved
  187416 -> 102145, timer 904324 -> 569086. The boxes include backdrop the render does not draw, so the drop is glyph ink only.
- Ink overlap with the baseline digits is 0.68-0.85 at zero shift; y may be 1 px off on some cases; no shift applied.
- White versus black text is reported as a sensitivity only (black raises the residual where the backdrop is lit); white stays UNREAD-default.
- The Q2 gate clause "no HUD exclusions" cannot pass on this directive; the skill-panel exclusion stays (C1 held).

## Facilitator check

Own scorer (not `verify.wider()`): re-rendered all ten views before and after from `build/c2g/webolution.exe`, masked as caption + ten skill boxes +
baseline == no-cell-routes; numerators and N match the worker to the unit on every view and the PNGs are byte-identical to the worker's. Re-read
`0x44D22C` (`6a 0f`, `be 6c 02 00 00`, `6a 32`) and the format strings at `0x4C01DC` / `0x4C01E8` / `0x4C01F4`. Mutation alone: timer y 430 -> 429 fails
`hudtext`, restored by hash; a first restore kept the old mtime so ninja served the mutated binary, fixed by touching the header and rebuilding.
Full ctest on mouv 33/33 (`dev-debug` preset). Hashes of the seven changed files equal on Mac and mouv; no CRs. No game process.
Not verified: the three region boxes' extents, ink overlap, the `0x44D860` / `0x414770` readings.
