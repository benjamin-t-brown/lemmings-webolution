# D2d: one live overlap, scored with the real tie keys

Queue id `d2d-tie-live`. Round 1 of 3. One stock start, pid 6256. Worker note:
`build/tmp/d2d-tie-live/round-note.md`. Row:
`build/tmp/d2d-tie-live/overlap.json`.

## Question

On one cursor where two eligible lemmings overlap, do the live
`[state+0x14]` priorities and the live squared distance to `screen+0x58`
select the lemming at `[screen+0x16D8]`?

## Answer

Yes, on this one cursor. Both priorities are 0. The smaller squared distance
selects actor index 3, and that is the original hover.

The level is `Compiled\MM_CLIMB_002.lvl`. The cursor doubles are `(448.0,
290.3998718261719)`. The reference doubles at `screen+0x58/+0x60/+0x68` are
`32.0`, `48.0`, `0.5`. Both lemmings have state vtable `0x004B0D28` and
`[state+0x14] = 0x12`. The live dword at `0x004C0618` is 0, matching the file.
Facing filters are 0. `hovered_button` is -1.

Index 2 is at `(60.53015899658203, 32.0, 0.25)`, squared distance
`1070.0324723702506`. Index 3 is at `(53.0374641418457, 32.0, 0.75)`, squared
distance `698.6373975194438`. One cursor was tried, 4.6 seconds after two
lemmings were eligible.

This does not pass D2 part (a). It reads only table index `0x12`. It is not
sample 15.

## Facilitator check (2026-10-22)

Decoded the saved little-endian hex, not the worker's decimal strings. The
reference is `32.0, 48.0, 0.5`. The positions match the decimals above. The
sum of squares of `(lemming+8) - reference` is `1070.0324723702506` and
`698.6373975194438`. Both boxes overlap the cursor under the inclusive ±4
test. Both priorities are 0, so the smaller distance wins: index 3, pointer
`0x02734c50`, which is `[screen+0x16D8]`. `0x004C05D0 + 4 * 0x12` is
`0x004C0618`.

Install hashes match the authority bytes. No Lemmings process was running.
`stock_starts_consumed` was left at 0; pid 6256 ran, so it is 1.

**Landed.** D2 part (a) is not accepted.
