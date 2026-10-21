# A7j — three stores at +0x25FF8; 3.5 does not enter the scale

2026-10-20. **READ.** Eleven `.text` uses of displacement `0x25FF8`. The
wider-mask score is unchanged.

## Stores

| VA | Function | What is written |
| --- | --- | --- |
| `0x45CB39` | `0x45C9AB` | `2.0 + 1.0 + 0.5` = **3.5** (`0x4B17F8`, `0x4B17E0`, `0x4B17E8`) |
| `0x45CBB2` | `0x45C9AB` | previous value **minus 2.0**, only when `[obj+0x1CB7C] == 0` |
| `0x45CEB3` | `0x45CC09` | previous value **minus `[obj+0x1C968]`** |

The other eight hits are loads or compares, including the scale block.
`0x45CFF9` calls `0x45C9AB` at `0x45D89D` and `0x45CC09` at `0x45DA48`, both
with `ecx = [ebp-0x94]`, the same slot as the later scale read. Both call
sites are before `0x45E113`. That does not by itself put every branch
through both calls.

`1.5` is the `0x45CBB2` result only when the value being reduced is `3.5`.
It is not a separate constant store.

## What those constants do at the scale compare

The setup at `0x45E113` compares `1.5` with `[G+0x25FF8]` and leaves to
`0x45E2BA` when the field is greater than `1.5`. A field of **3.5** takes
that leave, so the scale formula is not applied to it. The figure `7100`
is the formula on a value that does not enter the block.

A field of **1.5** does enter. It is also greater than `0.5`, so the scale
runs: `260 + round(760 * 1^2) = 1020`, which includes y `432`. Whether
`1.5` is the value still there at `0x45E113` depends on `0x45CEB3` and on
`[G+0x1C968]`.

## Facilitator check

`build/tmp/a7j-facilitator/verify.py` re-read the eleven hits, the `3.5`
sum, the conditional `fsub 2.0`, the `fsub [+0x1C968]`, both calls from
`[ebp-0x94]`, and the qword `760.0`. Expecting that qword to be `1.0`
failed. Restored hash
`3473c249ab0ddeb4026f39205d13d17c2ce9f82cb09f0630a221eefd8f59c29b`.

Build had no work. CTest 24/24. No game process.

## Left open

The double at the scale read is not yet one number. `[G+0x1C968]` is
written from `[ebp-0x28]` just before `0x45DA48`. The gate remains
`3031085/137520225 = 0.0220410125`.
