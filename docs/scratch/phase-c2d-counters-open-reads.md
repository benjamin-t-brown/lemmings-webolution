# C2d: the counter digits replace the words; Out source and colour order read (READ, offline)

2026-10-23. Directive Q2 round 2 of 3. Worker note `build/tmp/c2d-counters-open-reads/round-note.md`. No stock start, no debugger,
no source edit. REA not used.

## READ

- **Replace, not append.** `0x4404FA` calls `0x440940` (`mov dword ptr [eax], 0`) on the list before building; `0x4145A0` (the list
  count) returns that same `[list+0]` slot. So once `0x44D316` runs, `H+0x758` / `H+0x77C` hold only the digits; the `Out` / `Saved`
  word glyphs from `0x44CD06` are gone.
- **Out source.** The `0x44D316` argument is the sum of words at `this+0x5070 + idx*2` over the entries of the list at
  `this+0x1FB0` (`0x444ABC`, loop `0x444B08..0x444B69`, call `0x444B98`). A second sum over `this+0x5084` is passed but unused by
  `0x44D316`. `0x444ABC` is gated on `byte [0x4E33B9] == 0`. It has 8 callers (`0x4182AD`, `0x425A19`, `0x425D56`, `0x426086`,
  `0x42E00D`, `0x42E211`, `0x444D07`, `0x444D59`).
- **`0x4CBAB0` has no direct-address writer**: one `.text` reference, the read at `0x44D398`.
- **Colour order.** `0x4017F0` stores its stack arguments in memory order and the pushes are right to left, so the object holds
  255, 63, 63 at or above the target and 255, 255, 255 below. `0x44E060` stores nothing; it calls `0x403820(glyph, colour)` per glyph.

## UNREAD

- Whether those three doubles are R, G, B (`0x403820` unread), and the default glyph colour of Out, the timer and the words
  (`0x440C30`, `0x414870`, `0x440C50`).
- What `this` at `0x444ABC` is, what the `+0x5070` / `+0x5084` tables and the `+0x1FB0` list hold, who writes them, who writes
  `0x4CBAB0` and `0x4CBAAC`, and what triggers `0x44D22C`.

Prediction check: replace held; "doubles stored in push order" falsified; "`0x4CBAB0` written from load" unsupported.
Worker read 12 bodies at the cap, including one stray.

## Facilitator check

Re-read on mouv: `0x4404FA` head, `0x440940`, `0x4145A0`, `0x4017F0`, `0x44E060`, `0x44D39F..0x44D3CC` (pushes 63.0, 63.0, 255.0 in
that order, so first argument 255.0). Own `.text` scan: `0x4CBAB0` one hit at `0x44D398`; `0x4CBAA4` four; `0x4CBAAC` seven; callers
of `0x444ABC` identical to the worker's eight. Not verified: the `0x444ABC` loop bytes and the `+0x5070` / `+0x5084` offsets.
