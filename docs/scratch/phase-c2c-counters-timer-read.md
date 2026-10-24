# C2c: counters and timer draw (READ, offline)

2026-10-23. Directive Q2 round 1 of 3. Worker note `build/tmp/c2c-counters-timer-read/round-note.md`. No stock start, no
debugger, no source edit. REA not used; bytes from `read_steppers.py`.

## READ

- **Positions are absolute 640x480 pixels.** `0x44DF40` writes x,y to the first glyph's `+0x14` / `+0x18` and shifts the rest by
  the same delta; nothing adds an origin. So 520,15 / 520,45 / 560,430 (C2a) are screen pixels.
- **The count text is built into the same lists as the labels.** `0x44D316` (only caller `0x444B98`) formats `%d` (`0x4C01F4`,
  `0x4C01F8`) of its argument into list `H+0x758` and of `movsx word [0x4CBAA4]` into `H+0x77C` through `0x4404FA`, then
  `0x44E0B0(1)` on each. Saved is the signed word `0x4CBAA4`.
- **Counts are re-placed by `0x44D22C`** (only caller `0x44318D`): right end near x 620 (`0x26C` minus a glyph metric at
  `0x44D288`), Out at y `0xF`, Saved at y **`0x32` = 50** (`0x44D2B7`), not 45. The C2a y of 45 is superseded for Saved whenever
  this function runs.
- **Saved colour is not black.** `0x44D395..0x44D39C` compares `saved` with `word [0x4CBAB0]`: below target the colour is three
  doubles of 255.0 (`push 0x406fe000`); at or above it, two 63.0 (`0x404f8000`) and one 255.0, through `0x4017F0` into
  `0x44E060`. That is white, then (255,63,63) if the doubles are R,G,B in order (order UNREAD).
- **Timer text:** `[0x4CBAAC]` split by `idiv 60`, rebuilt only when seconds or minutes change (cache `0x4C01D4` / `0x4C01D8`),
  negative stored back as 0; submit skipped when `[0x4E33C8] == 1`.

## UNREAD

- Whether `0x44D316` replaces the `Out` / `Saved` words that `0x44CD06` built or appends to them (needs `0x4404FA` list start
  `0x440365` / tail `0x44038B`). That decides word-plus-number versus number alone.
- The Out count's source (caller `0x444B98`), the trigger and double of `0x44D22C` (caller `0x44318D`), writers of `0x4CBAAC`
  and `0x4CBAB0`, the default glyph colour, the meaning of glyph `+0` set by `0x44E0B0`, metrics `[g+0x20]` / `[g+0xf0]`,
  whether `[0x4E33C8]` is pause, callers of `0x48A720`, and `0x44D0ED` (centred text at y `0x19C`/`0x1B0`/`0x1B5`).

Predictions: default black failed for Saved; separate count list failed; left-edge absolute positions held.

## Facilitator check

Re-read on mouv: `0x44DF7F..0x44DFDD` (absolute stores), `0x44D316..0x44D38E` (both formats, `0x4CBAA4`, both `0x4404FA`
calls into `+0x758` / `+0x77C`), `0x44D388..0x44D3FF` (target compare, colour pushes; `0x406fe000` and `0x404f8000` decode to
255.0 and 63.0), `0x44D22C..0x44D2E5` (`0x26C`, `push 0x32` at `0x44D2B7`). All match. Narrowed: the worker read about 16 bodies
against a cap of 12 (mostly short accessors), and R,G,B order, the replace/append question and the Out source are open, so no
renderer value is settled by this note. Caller scans for `0x44D316` (`0x444B98`), `0x44D22C` (`0x44318D`) and `0x44CD06` (`0x441A7F`) re-run with my own `E8 rel32` scan on `.text`: identical, one caller each.
