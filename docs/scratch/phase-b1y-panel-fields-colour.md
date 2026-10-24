# B1y: menu panel colours and X pad read; top and height stay measured (READ, partial)

2026-10-23. Directive Q1 round 2 of 2 (last). Worker note `build/tmp/b1y-panel-fields-colour/round-note.md`. Offline; no stock start, no
debugger, REA not used. Worker read about 16 function bodies against a cap of 12 (about 5 are one-line accessors or thunks); reported, not excused.

## READ

- **Fill colours.** The setter `0x489ADC` stores its argument to `[+0x358]`, then calls `0x489A84` with (0xE0, 0x80, 0) when the flag is
  non-zero (`0x489B08`) and (0xE0, 0x0A, 0) when it is zero (`0x489B20`). These equal the measured 224,128,0 and 224,10,0 exactly.
- **Colour path.** `0x489A84` converts each channel with `and 0xff` / `fild`; `0x4017F0` is a plain 6-dword store; `0x44DBC0` copies the
  record into four slots. The doubles are 0..255 channel values and no tint or multiplier appears on this path. The constructor's 255,0,0
  fill is a default the setter overwrites.
- **Border.** 64.0 x3 from the constructor (`0x489A1C`); no later recolour found.
- **Fields +0x34..+0x40.** The layout loop `0x48AE38..0x48AE62` writes min/max over the label lines (`+0x34` = min(0, line X), `+0x38` =
  min(0, line Y), `+0x3C` / `+0x40` extents). The `+0x44` / `+0x48` setters are `0x489DF4`, `0x489E40`, `0x489E7B`; callers unread.
- **X pad.** `lea edx,[eax+ecx-0x14]` at `0x48A232` gives 0x14; with the border inset of 1 that is 19. READ by derivation only: it rests on
  `+0x44` equalling the label anchor.

## Still MEASURED-FROM-REFERENCE / UNREAD

- `panelTop` 1 and `panelHeight` 27 (need `[+0x48]`, the line Y from `0x48A720`, and `0x489850`, none opened).
- The double to framebuffer byte step (identity assumed from the exact match), which row gets the highlight flag (callers of `0x489ADC`),
  the edge convention, the glyph blend over the panel.

## Score

0.005244472528594771 (1232493 / 235008000, 185447 px), identical before and after, as expected: the read values equal the measured ones.
Not tuned. B1 stays accepted; the panel colours and X pad are now READ, top and height stay labelled measured. Q1 is landed at its cap.

## Facilitator check

Re-read raw bytes on mouv: `0x489B05..` (`6a 00 6a 00 68 80 00 00 00 68 e0 00 00 00` for the flag arm, `6a 00 6a 00 6a 0a 68 e0 00` for the zero arm),
`0x48A22E` (`8d 54 08 ec` = lea edx,[eax+ecx-0x14]), `0x489A18..` (64.0 doubles pushed with `68 00 00 50 40`), `0x489A84` (`25 ff 00 00 00`, `db 45`).
Own checker (not the worker's scorer) on a fresh render: 1232493 / 235008000, 185447 px. Mutation alone: `panelNormal` G 0x0A -> 0x0B fails
`menurender`; header restored by hash (`43bb7c68...`). Full ctest 32/32 on mouv; Mac and mouv hashes of the three owned files equal. Not verified:
the callers of the setters, the layout loop's min/max details, the `+0x44` equals anchor premise.
