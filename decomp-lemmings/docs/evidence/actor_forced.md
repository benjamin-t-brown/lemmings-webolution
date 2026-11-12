# Forced actor state 0x25

Reference: installed `Lemmings Revolution.exe`, SHA-256
`32675bdb49866e76502713251fdfd2b09078c2481c72ca22ff68ebf4b3737a2f`.
The scheduler selects this state when actor byte `+0x5D` is set. Its original
source name and gameplay label remain unknown.

## Static evidence

| Address | Length | Recovered behavior | REA evidence |
| --- | ---: | --- | --- |
| `0x416BF0` | 48 | Base behavior metadata, type `0x25`, extra byte zero | `ev_27a51af50789ad6240403cc965a6cc535df2d0157ca6559cd292baa73b39184e` |
| `0x416C3C` | 13 | Empty entry | See original bytes |
| `0x416C49` | 587 | Effect/entity creation and return 2 | `ev_e0024308700400a403e81e89d3500fb094a772bd2ea17fe45f3491e27a89bcd9` |
| `0x401320` | 13 | Empty leave, shared by other states | See original bytes |

[Assembly](actor_forced.asm) records every original instruction byte. The runner
pins each function and the executable identity before loading an offline copy.
Mapped offsets and hashes are also in `tools/verify_reference.py`.

The routine copies actor position before any callbacks, randomizes Z first by
`random + 2 + Z`, then Y by `random + 1.5 + Y`. Each expression rounds once to
double; the original obtains 1.5 from integer global `0x4B10B8` (3) divided by
double `0x4B03F8` (2). Those immutable image constants are pinned.

It allocates/constructs effect `0x416ED0` (0x148 bytes), copies the live actor
category to effect `+0xD8`, sets count 1, transforms the live actor position
before the captured offset, and subtracts the transformed points. Direction,
live position, pair (4,0.5), mode 8 and advancement by the current time budget
follow in that order. It then reads live Y and X for the world sample, copies
the sample to child 0, and creates/inserts a 0x48-byte entity at the captured
offset. It returns 2 without consuming or zeroing the caller's time budget.

Random generation, world mapping/sampling, graphics/effect operations and
allocation/registration are explicit callbacks. `set_category_d8` represents
the direct four-byte write and must perform only that write. The world mapping
and sampling callbacks can use the separately recovered implementations.
Effect allocation/constructor and child lookup must return valid objects; the
original unconditionally dereferences them. Entity allocation may return null
and that null is still passed by reference to insertion. Constructor exception
cleanup and arbitrary overlapping object storage are outside this recovery.

## Unit checks

The unit fixture checks constructor metadata, empty entry/leave, random call
order, captured coordinates, terminal result and unchanged time budget. It
also changes actor position/category/time at every observable effect boundary
and checks that captured offset coordinates remain unchanged.

## Native comparison

`native_actor_forced_oracle` executes all 587 original update bytes, the
constructor, empty entry/leave, original behavior base constructor, point
subtraction and x87 integer conversion. Only the documented random, world,
allocation, graphics and registration callees are adapted. The direct category
write remains an original instruction and is observed by the next count call.

40,000 cases pass at both O0 and O2. Half use the unchanged original
`0x404080 -> 0x4040A0 -> 0x49C84A` random chain and compare its final seed;
half adapt random calls to test mutations at those boundaries. Comparisons include the complete actor and
behavior buffers, category write, return value, time-budget bits and ordered
callback arguments. Cases cover all mutation stages, null entity allocation,
extended-precision random fractions, negative and overflowing coordinate
conversions, signed zero, quiet NaNs and infinities. NaN payloads in computed
callback arguments are normalized; untouched actor/time bytes are compared
exactly. FPU traps and signaling NaN arithmetic are not claimed.

This is controlled original-byte comparison, not a live-game effect rendering
comparison or a complete recovery of the effect class. The random generator
is recovered separately in [game_random](game_random.md).
