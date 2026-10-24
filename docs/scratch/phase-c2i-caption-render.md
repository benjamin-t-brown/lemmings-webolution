# C2i: caption panel drawn; with the caption exclusion removed the A7 gate FAILS on 3 of 10 views

2026-10-23. Directive Q3 round 4 of 4 (offline; the last). Worker note `build/tmp/c2i-caption-render/round-note.md`. New
`src/shell/PauseCaption.{h,cpp}`, `test_pausecaption`, `tools/harness/score_a7_caption.py`, a headless `--shot` overlay (`--pause-caption`,
sensitivity flags `--caption-alpha`, `--caption-fill`). No start, no debugger, no game process.

## What is drawn

| Item | Value | Tag |
| --- | --- | --- |
| Rectangle | [277,422,363,454) | MEASURED live (C2h); formula READ (C2f) |
| Setter sequence at `0x45D35F..0x45D3BD` | `0x44DCC0` (0.00225), `0x44DBC0` (six zero pushes), `0x44DD20` (128.0), `0x44DBA0` (1) | READ |
| Fill black, 128.0 as alpha on 0..255 | | HYPOTHESIS: `0x4017f0`, `0x44DBA0` and the blend in `0x46D460` are unread |
| Blend `(px*(255-a) + fill*a + 127)/255` | | OURS (no read) |
| 0.00225; quad doubles being final pixels; the "Paused" text (key `STR_PAUSED` at `0x4C3C18`, consumer unread) | | UNREAD; text not drawn |

## Result (caption exclusion removed, ten skill boxes kept; numerator before -> after; pass iff numerator*50 <= N*765)

initial 3007697 -> 2879334 (threshold 2742219, **fails**), canonical 3021039 -> 2892673 (**fails**), MM_DIG_000 2935068 -> 2882064 (threshold
2683804, **fails**); mouse, right, left and elev0_neg go from fail to pass; zoom1, elev_up, elev_down pass both ways. Region residual inside the
rectangle is down on 10 of 10 views (initial 522691 -> 412464). Sensitivity (reported, not chosen by): alpha 0 equals before; alpha 255 and a
white fill raise the numerators (white makes zoom1 fail); initial region residual 522691 / 412464 / 745819 / 823918 for alpha 0 / 128 / 255 / white.
Fill and alpha were set from the setters before scoring and not moved.

What remains in the box (initial: 137115 over the threshold): our render has no white "Paused" glyphs where the baseline does, and the bottom
rows are black where the baseline shows tube. Not apportioned per source.

## Facilitator check

Own pixel loop (`build/tmp/c2i-fac/check.py`, not `verify.wider`; mask baseline != no-cell-routes, skill boxes excluded, no caption exclusion) on
the worker's saved before/after PNGs: numerators, N and thresholds equal the worker's on all ten views, same three failures; region residual equal.
Re-read `0x44DBC0` head, `0x44DCC0` and `0x44DD20` on the exe (the 8-byte store at `+0x58`). Mutation alone: `alpha` 128 -> 127 in
`PauseCaption.h` fails three `pausecaption` checks; restored, sha1 equal. Full build and ctest on mouv 34/34. Sources equal on Mac and mouv by sha1.
Not verified: the worker's other eight single-input mutations, the sensitivity renders beyond what the score table shows, the crop image.
