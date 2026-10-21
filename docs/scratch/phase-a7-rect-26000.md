# A7i — unscaled 180..260 misses band B; a scale path can move it

2026-10-20. **READ.** The four constants are two horizontal endpoints and a
vertical pair. The wider-mask score is unchanged.

## Edges

`0x47AD8C` takes `this` in `ecx` and four doubles, then `ret 0x20`. The
caller pushes `[ebp-0x50]`, `[ebp-0x4C]`, `[ebp-0x54]`, `[ebp-0x58]`, so:

| Slot | Unscaled value | Use |
| --- | --- | --- |
| `[ebp+8]` | 160 | copied to `[ebp-8]`, the start of the horizontal step |
| `[ebp+0x10]` | 480 | `fld` then `fsub` the 160 double at `0x47ADAA` |
| `[ebp+0x18]` | 180 | passed through at `0x47AE46` |
| `[ebp+0x20]` | 260 | passed through at `0x47AE34` |

The step is `(480 - 160)` divided by the return of `0x47ACF9`. The walk
starts at 160 and moves toward 480. The other pair, 180 and 260, is not
subtracted. Their interval is `[180, 260]`, which does not meet y
`432..447`.

## The constants are not always the values pushed

`0x45E157` `jne 0x45E1F1` skips a block that does not write the four slots.
The fall-through converts `760.0 * ([G+0x25FF8] - 0.5)^2` (`0x4B1810` is
`760.0`, `0x4B17E8` is `0.5`) and subtracts that integer from 160 and 180
while adding it to 480 and 260, then jumps to the same push at `0x45E274`.
The horizontal pair uses `3040.0` at `0x4B1818` the same way. A second
compare at `0x45E11E` can skip the whole setup to `0x45E2BA`.

On the skip-scale branch the vertical pair stays 180..260 and misses the
band. On the scale branch the upper edge is `260` plus that delta, so a
large enough `[G+0x25FF8]` would include y `432..447`. That double is not
read here.

## Facilitator check

`build/tmp/a7i-facilitator/verify.py` re-read the four stores, the `fsub`,
both vertical `fld`s, `ret 0x20`, the scale adjustments, and the qword
coefficients. Expecting `[ebp-0x54]` to be `432` failed; the stored
immediate is `480`. Restored hash
`d05620f797c0b5147568c4ed704d0d4902be1161a381db44f5dc59a902b67585`.

The packet's band miss holds on the unscaled branch. It does not hold for
every path into the push. Build had no work. CTest 24/24. No game process.

## Left open

The value of `[G+0x25FF8]` decides whether this rectangle meets band B.
The gate remains `3031085/137520225 = 0.0220410125`.
