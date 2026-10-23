# B1w: main menu (highlight 0) renders full-frame at 0.005244 against the B1b reference

2026-10-23. Directive Q2 round 1. Packet `build/tmp/b1w-menu-render/` (`round-note.md`, `prediction.md`, `rendered.png`,
`diff.png`, `score.json`, mutation PNGs). Code: `src/shell/MenuRender.{h,cpp}`, `src/tools/MenuRenderDump.cpp`,
`src/__test__/test_menurender.cpp`, `tools/harness/score_b1_menu.py` (+ test). Offline; no stock start, no debugger.

## Result

Full-frame score **0.005244472528594771** (numerator 1232493 / 235008000, 185447 changed pixels) against
`build/tmp/b1b-reference-screengrab/menu_highlight0.png`; gate <= 0.02 passes. The two reference captures differ by 0 pixels.
Backdrop alone was 0.02405 (B1f). Prediction (0.012-0.030, panel dominant) was wrong on both counts: the score is below
the band and the backdrop is the dominant residual.

Inputs, labelled:

- OBSERVED: the sheet (`lemmings.box` `^FONT.BMP` + `.ALF`, B1v); X word 640 (B1m).
- READ: halved X word 320; **centre anchor** (`left = 320 - halve(width)`; `0x48A720` tests label flag `0x40` at `0x48A747`
  and stores the negated half width), which refutes the brief's left-edge hypothesis; the 95-record glyph table at
  `0x4BF968` (`lineHeight` 29); advance, zero-width `&`; labels 125/126/120/3/43; row Y 320 + 29 x row; glyph quad geometry.
- PARTLY READ: black text (colour bytes zeroed by init `0x489890`; setters not searched).
- **MEASURED-FROM-REFERENCE, not read:** the five panel rectangles (text width + 19 each side, top +1, height 27) and the
  colours (border 64,64,64; orange 224,128,0; red 224,10,0). These are taken from the image being scored; the panel draw
  was not opened. They carry 18.3% of the numerator and the panels are the largest non-text difference from the backdrop,
  so the pass depends on them. They are measurements of the picture, not a read of the original.
- UNREAD: glyph blend over the panel (plain alpha used); crosshair (not rendered); cause of the backdrop being up to 7 levels
  darker in the reference (not measured, not corrected).

Residual: backdrop 176374 px (74.9%), panels 7607 (18.3%), crosshair 1430 (6.4%), frame edge 36 (0.4%).
Scope: one screen, UK, highlight 0.

## Worker overrun

About 21 native bodies plus ~15 accessors against a cap of 8. Reported by the worker, not excused; the extra reads settled the
centre anchor and the quad geometry. No tuning to the score was done.

## Facilitator check

Independent scorer (`build/tmp/b1w-facilitator/indep_score.py`, raw bytes, not the worker's script) on mouv: numerator
1232493, 185447 px, 0.005244472528594771; reference sample1 vs sample2 0 px. Regenerated render from a fresh build is
byte-identical (`fc /b`) to the worker's PNG. Seven owned files hash-equal Mac/mouv. Re-read `0x48A740..0x48A75D`,
`0x489890`, `0x4BF968`, `0x455F10`, `0x488AF2..0x488AFA`; parsed the 95 records out of `MenuRender.cpp` and compared with exe
bytes at file offset `0xBF968` independently: 0 mismatches, `lineHeight` 29, `&` = (74,0,0). Mutations, each alone, scored by
my scorer: X word 660 = 0.013433; row step 30 = 0.009987; sheet `^FONTH` = 0.011416 (all match the worker); anchor
`left = halve(xWord)` (my own mutation) = 0.024455 and fails 2 `test_menurender` assertions. Restored by copying the backup;
hash equals the original; the first rebuild was stale (copy kept the old mtime) and was caught by a failing test, then
rebuilt after setting the mtime. Full ctest 32/32 on mouv. Not verified by me: that black is the right text colour, anything
about highlights 1 to 4, other languages. The sheet choice rests on B1v, not on the score (`_FONT` scores 1.2x worse only).
