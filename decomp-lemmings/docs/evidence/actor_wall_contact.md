# Four-cell actor wall-contact query

Authority: installed executable SHA-256
`32675bdb49866e76502713251fdfd2b09078c2481c72ca22ff68ebf4b3737a2f`.
Routine `0x0042EB48`, file offset `191304` (`0x2EB48`), 245 bytes,
SHA-256 `605e7b64017fca65620c71ba4b31b4f23eb291de3303273d6e05868260913f20`.
Full bytes: [actor_wall_contact.asm](actor_wall_contact.asm).
REA static dossier:
`ev_1686b18576462dddcaa20271848194b5579de31e3a8a3f2d0cdc791d337ddd66`.

The source is `actor_has_wall_contact` in `src/actor_geometry.cpp`. It samples
position plus `pair_360`, then reads four cells. For index 0 through 3, Y is
sample Y plus the signed wrapping product of vertical direction and index,
stored to binary64, clamped and converted with x87 low-32 truncation. X is
sample X plus horizontal direction, stored to binary64, wrapped and truncated.
Y conversion occurs before X conversion, as in the original.

The result starts true. A cell with bit `0x80` leaves it unchanged; otherwise it
is bitwise-ANDed with the cell's nonzero predicate. All four probes execute,
even after the result becomes false. Thus four `0x80` cells report contact;
any zero cell makes the result false. Actor category and primary behavior type
are not consulted, and the actor is not modified.

Static disassembly establishes arithmetic widths, stores, read order and mask
behavior. Unit tests cover zero/solid/bit-0x80 combinations and repeated sampling
when vertical direction is zero. The native geometry oracle runs the unchanged
routine and original sample/grid/coordinate/conversion helpers without adapters
in this query, comparing 10,512 cases at O0 and O2 and checking all actor bytes.
Cases include all 256 four-cell combinations of zero/one/0x80/0x81 in both
vertical directions, X seam wrapping, fractional offsets, clamped Y, negative
and extreme vertical directions, and unrelated terrain values. Units pass on
x64 and x86. Valid positive grids and terminating wrap inputs are required;
this is not a full-game runtime comparison.
