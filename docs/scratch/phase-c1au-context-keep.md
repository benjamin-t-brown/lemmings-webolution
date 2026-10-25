# C1au: context bytes are read; the kept mask is conditional

Queue `c1au-context-keep`. Offline round 2 of 3 on the alpha-count question. No stock start. Worker [Read context keep rule](6e969aa5-a12e-4b43-aa9b-dfceb90e800d). `prediction.md` 00:17:10, before the disassembly. REA bounded the callers. Dispatched 00:15 MDT.

## Result

Context `+4` is 1 when `[object+0x24] & 0x18` is nonzero. `+5` is 1 when `[object+0x24] & 7` is nonzero. `+6` is 1 when the low 16 bits of `[ebp-0x192]` are `<= 8`, unless the `+5` path overwrites it. `+7` is 1 when `[ebp-0x140] & 4` is 4. For a `^` name the flag is 8, so `+4` is 1 and `+5` stays 0.

The worker's claim that `+7` is 0 on every entry is withdrawn. No instruction in `0x472F99` writes `[ebp-0x140]`. The `0x7c` fill at `[ebp-0x18c]` ends at `[ebp-0x110]`.

If `+6` is 0 and `+7` is 0, the c1aj popcount arm keeps hits.log line 29, mask `0x0000F000`. If `+7` is 1, no logged record passes. The log is not a `^tab` upload.

## Facilitator check

Re-disassembled `0x473146` through `0x4731CF`, the fill at `0x473046`, the store of `0x20` at `0x473063`, the store of `0x10` at `0x47313F`, and the bit-count load at `0x472518`. `0x4731C5` jumps over the store of 0, so the store of 1 at `0x4731C1` is not dead. `^tab`'s flag 8 skips that block at `0x47319F`. A byte search of `0x472F99`..`0x473B1E` found the displacement of `[ebp-0x140]` only in the read at `0x473146`.

A separate read of the exe checked 17 sites. Expecting `88 45 f4` at `0x473157` failed; the file bytes are `88 45 f3`. Restored, the same check passed. The saved `tab.bmp` is 6966 bytes, 72 by 32, biBitCount 24. Applying the landed c1aj tests to the 20 `$LemBits0` records kept line 29 when `+7` is 0 and kept nothing when `+7` is 1. `prediction.md` is timestamped before `round-note.md`. No Lemmings or Frida process. No source change, so the suite was not rebuilt.

Narrowing: `+6` compares the word at GetObjectA buffer `+0x12` with 8. The BMP biBitCount is 24, and `0x472518` loads the header word at `+0x0E` for the section. This check does not re-derive the Win32 field name. The numeric mask stays conditional on the unwritten dword.

## Stop

Offline round 2 of the alpha-count question. The formula is read. The kept mask is not. The gate is not scored.

## Correction

C1ax rejects the coverage sentence in the result above. The fill does write `[ebp-0x140]`, and the dword at the read is 0. The withdrawal of "always 0" stands in this note as what c1au concluded. The corrected read is [C1ax](phase-c1ax-fill-coverage.md).
