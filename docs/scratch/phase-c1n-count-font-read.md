# C1n: count font file, count text source and colour (offline, round 1 of 2)

Queue `c1n-count-font-read`. Worker packet `build/tmp/c1n-count-font-read/round-note.md`, `prediction.md`. Capstone via `read_steppers.py --at` on mouv (REA not used); no game,
no debugger, no stock start. Dispatched 11:31 MDT, packet ~11:49, landed with c1m.

## Result

- **A. Handle `+8` writer (READ).** Thunk `0x4403E5` pushes byte `0x30`, table `0x4BFC10`, name `0x4BFF78` (`_FontSmall`) and calls `0x440404`, which passes `this+8` to
  `0x474519` (`0x440427`). `0x474519` -> `0x449B30` (find by name) else `0x45FE10` (add), `0x4607D0` on a hit: the handle is the **pool index in manager `0x58CB48`, keyed by
  name**. 36 / 37 are the indices of `^Font` / `_FontSmall`; their being consecutive is INFERRED. The font path never touches the sheet registrar `0x446040`.
- **B. File (PARTLY READ).** `_FontSmall` takes flag 1 in `0x4743DD`: no `.alf` companion, no `INN_`/`OUT_` suffix. `lemmings.box` has one candidate: ordinal 656
  `TEXTURES\_FONTSMALL.BMP` (181 bytes), no `Lo` / `Hi` variant; `patch.box` has only `FONTH` / `FONTP`. UNREAD: its pixels and size (decoder `0x467DE8`), whether a loose-file
  or archive-order override applies (B1p / B1q premise), whether the stock process loaded 656.
- **C. Count text (READ).** `0x45A7D0..0x45A7FB` builds a two-character string, `count/10 + 0x30` then `count%10 + 0x30`: always two digits (0 -> "00"). `0x45A813` calls
  `0x4404FA(str, textobj)` on `[0x4CBF7C]`, then `0x44DF40(textobj, 66, y)` with y = 20, 60 ... 340 (matches c1l). `textobj+0xC` is an array of 0x1B0-byte glyph quads, one per
  character, which is why c1l saw a pointer, not digits. Icon 9 is skipped at `0x45A79A`; what writes its pointer is UNREAD.
- **`_FontSmall` glyphs (READ from `0x4BFC10`).** Height 14; '0'..'9' are 6 wide at x = 0, 6 ... 54, y = 0; first char `0x30`.
- **D. Colour (PARTLY READ).** No colour value is passed on the `0x4404FA` / `0x440C50` / `0x440C30` path read; the quads store geometry, UVs and the texture handle. The shown colour
  is the sheet's texels under any draw-time state: UNREAD. The rest of `0x440C50` was not read.

## Facilitator check

Re-decoded `0x45A7D0..0x45A801` (idiv 10, `+0x30` twice), `0x4403E5..0x4403FB` (push `0x30` / `0x4BFC10` / `0x4BFF78`), `0x45A79A` (`cmp [ebp-0x10], 9`). Scanned `lemmings.box`: the
`TEXTURES\_FONTSMALL.BMP` path is present. **Not re-derived:** the `0x474519` / `0x449B30` pool walk, the flag logic in `0x4743DD`, the `0x47226C` suffix logic, the 181-byte payload,
the `0x4404FA` glyph-quad body and the "no colour on this path" negative (a negative over the part read, not over the draw).

## UNREAD, next bodies

`0x467DE8` on ordinal 656 (size, pixels), the glyph-quad draw path for any tint (rest of `0x440C50`), and `0x45AE87` plus the icon-9 case.
