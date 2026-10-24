# C1q: the `_FontSmall` bitmap and count colour (offline, round 2 of 2)

Queue `c1q-font-bitmap-read`. Worker packet `build/tmp/c1q-font-bitmap-read/round-note.md`, `prediction.md`. Capstone via `read_steppers.py --at` plus the worker's `disasm_at.py`; the exe's own decoder
`0x467DE8` was run in unicorn. No game, no debugger, no Frida, no stock start. Dispatched 11:54 MDT, packet ~12:12, landed 12:15.

## Result

- **Bitmap (READ by emulation).** Ordinal 656 `TEXTURES\_FONTSMALL.BMP`, 181 stored bytes, decodes through `0x467DE8` to 630 bytes, equal to the record's declared size (checked by the caller at
  `0x450405`). A standard BMP: **64 x 16, 4 bpp**, 16-entry palette, palette offset 118. Only indices 0 (black, 864 texels) and 1 (white, 255,255,255, 160 texels) are used. Ten digit cells of 6 columns at
  x = 0, 6 ... 54, no ink at x >= 60; ink in rows 3-10 counted top-down. This agrees with the `0x4BFC10` table (height 14, cells 6 wide).
- **Tint (PARTLY READ).** `0x440C50` is an 8-dword UV setter with no colour store. The quad constructor `0x440A10` sets every vertex colour and alpha to 255.0; `0x46D460` packs those, which with the defaults
  is `0xFFFFFFFF` (derived from the reads, not observed). So nothing read tints the texels: **white digits if nothing later writes the quad colour**, an inference.
- **Override (PARTLY READ).** `patch.box` has no `FONTSMALL` member; `lemmings.box` has 656 `_FONTSMALL.BMP` and 657 `_FONTSMALLTEST.BMP`; no loose `Textures` file in the install. The comparator choosing 656 over 657, the
  registration-order premises (B1p / B1q / B1r) and whether the stock process loaded 656 are UNREAD.

## UNREAD, named

Any later writer of the quad colour fields (`[textobj+0xC] + k*0x1B0 + 0x40 / 0x58`), the dim or flash for zero-count icons; whether palette index 0 is a colour key or alpha at texture creation (`0x47218A`) and whether the
loader flips the BMP; D3D blend state at the draw `0x46D5F3`. Live remainder: the quad colour fields and the packed `[vertex+0x10]` of a count quad.

## Facilitator check

Parsed the worker's decoded file `fontsmall_656.dec` with a fresh script: 630 bytes, `BM`, 64 x 16, 4 bpp, pixel offset 118, palette 0 = black and 1 = white, index counts 864 / 160, file-order ink rows 5-12 (top-down 3-10),
columns 0-58, none at x >= 60, per-cell ink counts 18, 10, 15, 15, 16, 17, 19, 12, 19, 19. Re-decoded the head of `0x440C50`. **Not re-derived:** the unicorn run of `0x467DE8` itself (I checked its output is a
well-formed BMP of the declared size, which is strong but not independent of the emulation), the rest of `0x440C50`, `0x440A10`, `0x46D460`, and the 656-over-657 choice.
