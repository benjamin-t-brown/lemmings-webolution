# A7l — the scale integer is a timer delta, or a conditional 100

2026-10-21. **READ.** The pause-frame value is not a file constant. The
wider-mask score is unchanged.

## Writes

Eight `.text` encodings of displacement `0x25C64`. Two store:

| VA | Value |
| --- | --- |
| `0x45D09D` | `eax - [G+0x25C60]`, where `eax` is the return of `0x4657D1` and `ecx` was loaded from `[G+0x557C]` |
| `0x45D96B` | the integer from `round(0.1 * 1000.0)` when the compare or the flag says so |

`0x4C08E8` is the double `0.1`. `0x4B1758` is `1000.0`. The product is
`100`. `0x45D927` compares the current dword with that `100`. When the
dword is greater than `100`, `0x45D940` jumps to the store. When it is
not, `[G+0x1C9B3]` nonzero also stores `100`; a zero byte skips the store.
`0x45DA1B` later `fild`s the same dword. Nothing in the eight hits writes
it between `0x45D96B` and that `fild`.

`0x4657D1` calls the import pointer at `[0x4B0220]` and subtracts
`[ecx+8]`. That is a runtime tick, not a constant.

The other six hits are reads, including the `fild` at `0x45DA1B`.

## Band

Integer `0` with `V_init` `1.5` still scales the top to `1020`. Integer
`100` subtracts `0.1`, leaving `1.4` on that same branch, and
`760 * (0.9)^2` still lifts `260` through y `432`. Integer `100` does not
rescue a `V_init` of `3.5`, which stays above `1.5` and skips the setup.
Which of those values is present at pause is not in the file.

## Facilitator check

`build/tmp/a7l-facilitator/verify.py` re-read the eight hits, the
subtract store, the `0.1 * 1000` store, the later `fild`, and both
qwords. Expecting the divisor to be `1.0` failed. Restored hash
`e6e8eb911c4cdc5a6e969e416ca4dc76d3fa0d4789826fb5c2da2a480c1aebfe`.

Build had no work. CTest 24/24. No game process.

## Left open

The pause-frame integer needs a memory read. The gate remains
`3031085/137520225 = 0.0220410125`. Eight other views and a second level
still have no mask, so that read would not by itself finish A7.
