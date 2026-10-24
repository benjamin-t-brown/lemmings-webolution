# C1p: the Lemmings `.spr` sheet table, parsed offline and matched to c1l's live frame records (round 2 of 2)

Queue `c1p-spr-sheet-read`. Worker packet `build/tmp/c1p-spr-sheet-read/round-note.md`, `prediction.md`, scripts and extracted `.SPR` files. Capstone via `read_steppers.py --at` on mouv (REA not used); no game, no
debugger, no Frida, no stock start. Dispatched 11:54 MDT, packet ~12:24, landed 12:26.

## Result

- **Box members (READ).** `lemmings.box` member 446 `SPRITES\LEMMINGSHI.SPR` (113,015 bytes) and 447 `...LO.SPR` (56,803); both are plain, uncompressed.
- **Parse `0x483EF0` (READ).** `0x484160` -> table A: u32 count (15), per record u32 nsub; per sub u32 nframes; per frame (`0x48` bytes, `0x485ce0`) four u32 source rect, four f64 UV, u32 dx, dy, w, h, then a texture-name string
  passed to `0x474519`; then the record name and an f64 scale (**1.0 in Hi, 0.5 in Lo**). Records: bash, block, build, climb, dig, drown, explode, fall, flip, mine, shrug, float, umbopen, walk, cartwheel
  (index = c1l `+0x1C8`, sub = `+0x1CC`, frame = `+0x1B8`). Table B (count 1, n 0) follows; 384 trailing bytes (48 f64, 16 triples) are not consumed by `0x483EF0` and their reader is UNREAD.
- **Match against c1l's MEASURED frame records.** All seven generic icons (`+0x1FC` = 6): dx, dy, w, h and all four UV doubles equal the live F in **`LemmingsHi.spr`, 7/7**. Icon 0 = climb / sub 4 / frame 2, `Sheets\$LemSheetHi21`, (8,7,23,30);
  icon 6 = mine / sub 21 / frame 2, `Sheets\$LemSheetHi5`, (32,2,47,49). **`LemmingsLo.spr` fails** (icons 4 and 6 index past its sub counts; icon 0 gives (9,3,23,31)). So c1l's run used the **Hi** sheet:
  DERIVED from the match, not a read of `[0x58CD80+0xCC]`, which stays unread. Hi, and the geometry and UV of the ten buttons' seven generic icons, are therefore known offline.
- **Texture id to sheet name (PARTLY READ):** the seven live ids pair with names (icon 0: 0x81 = Hi21; icon 6: 0x7d = Hi5), a measured pairing, not a formula; the id assignment `0x45fe10` / `0x4607d0` is unread.
- **Sheet pixels (UNREAD).** The `TEXTURES\SHEETS\$LEMSHEETHI<n>.BMP` box members do not start with `BM` (member 523 `HI21`, 37,589 bytes, begins `02 da 8d 60 ea 04 ...`); the encoding is not identified. The font record (c1q) begins `01 b4 00 76` and was
  decoded by `0x467DE8`; whether the sheets use the same decoder with another leading tag is a lead, not a read.

## Facilitator check

Wrote a fresh parser from the layout alone (no reuse of the worker's `parse.py`) and ran it on the worker's extracted files against `build/tmp/c1l-icon-live-read/readings.json`: Hi 15 records, scale 1.0, **7/7** match on dx, dy, w, h and UV
(1e-12); Lo scale 0.5, 0/7 with two index errors. The extracted Hi file hash prefix equals the worker's (`0386bb98b773c174`). Bytes left after table A = 392 = the worker's 384 plus the 8-byte table B. **Not re-derived:** the box member table
(the worker's extract), the decode chain addresses `0x484160..0x485ce0`, and the unconsumed 384-byte tail. A fit of 7/7 on both geometry and eight-byte doubles is strong, but it is a match to measured values; the code was read, not re-traced
instruction by instruction.

## What the render still lacks

The sheet pixels (decoder for the `$LEMSHEETHI*.BMP` members), the icon draw x, y (the hook's `0x47A75D` arguments; the count x, y are MEASURED), the tint writers, and the Hi / Lo free-memory value.
