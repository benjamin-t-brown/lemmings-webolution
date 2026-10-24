# C2f: the caption panel is a quad at G+0x25C90; the formula is READ, the inputs are not yet measured

2026-10-23. Directive Q3 round 2 of 4 (offline). Worker note `build/tmp/c2f-caption-placement-read/round-note.md`; decode
`decode_caption.py` (selftest 27/27, pins 17 exe byte windows). No stock start, no debugger.

## READ

- `0x44E150` places nothing: it is the generic cell-list draw (flag, loop cells last-to-first through `0x414580` / `0x414920`).
  `0x44DF40` stores `[this+0x14]` / `[this+0x18]` and shifts the glyph cells; the pair places the **text**, not the panel.
- The panel is a quad of doubles at `G+0x25C90` (`0x4EC698`), written by one call, `0x45D35A call 0x44DC50`, drawn by `0x44DE60` when
  `[G+0x572C] == 1` (`0x45E31C..0x45E331`). `0x44DC50` stores a -> `+0x20/+0x88`, b -> `+0x28/+0x160`, c -> `+0xF0/+0x158`,
  d -> `+0x90/+0xF8`.
- Arguments (`0x45D2A8..0x45D340`): with `half(v)` = `cdq; sub; sar` (truncate toward zero), `P40 = [0x58CB40]`, `P44 = [0x58CB44]`,
  `W` = return of `0x47ACF9`, `H` = return of `0x47AC50`:
  `x0 = half(P40) - half(W) - 8`, `y0 = P44 - 2H`, `x1 = half(P40) + half(W) + 8`, `y1 = P44 - H + 3`.
- `W` sums `(dbl[cell+0x100] - dbl[cell+0x30]) * sheetW` over the cells and truncates; `H` is the max of
  `(dbl[cell+0xA0] - dbl[cell+0x38]) * sheetH`, truncated (`0x49C5AC` rounds toward zero). Reading the doubles as UV extents is a hypothesis.

## Measured from reference (not read)

Target `[277,422,363,454)` from the A7 baseline. Solving backward gives P44 = 480, H = 29, half(W) = 35; forward with P40 = 640,
P44 = 480, H = 29, W 70 or 71 returns the target exactly; W 72 or H 30 miss. This agrees with the formula's shape, it does not show
`G+0x25C90` is the panel the baseline calls the caption.

## UNREAD

Who writes `0x58CB40/44` (P44 = 480 is implied only); the cell and sheet values behind W and H; what fills `G+0x25C68`; the
`0x44DE60` / `0x46D460` raster path (are the quad doubles final pixels); the panel colour and alpha path.

## Live read this unlocks (round 3, one start, memory reads only)

Extend the c2e dump of `G+0x25C90` from 0x40 to 0x170 bytes; `G+0x25C68` (0x80) and `0x58CB40` (0x40) already cover the rest.
`rect_from_quad` gives the rectangle from the dump, `rect_from_placement` is the cross-check.

## Facilitator check

Re-read on mouv: `0x45D2A8..0x45D340` (the four pushes, `lea edx,[eax+ecx+3]`, `lea edx,[ecx+eax+8]`, the `half` sequences, P40/P44 loads),
`0x44DC50` whole body (all eight `fstp` offsets), `0x45E31C..0x45E342` (gate, quad draw, then `0x44E150`), `0x44E150` whole body (no
`+0x14/+0x18` read). Own forward scan of `rect_from_inputs` for W 70..72 x H 29..30: only (70|71, 29) hits the target. Reran the selftest on mouv:
27 passed. Mutation alone: `p44 - h + 3` -> `+ 4` in `rect_from_inputs` fails "forward W=70 H=29 ... -> target"; restored, sha1 equal (an
earlier mutation attempt did not apply and its 27/27 is discarded). Not verified: the helper bodies `0x47ACF9` / `0x47AC50` line by line, the other 14 pinned windows.
