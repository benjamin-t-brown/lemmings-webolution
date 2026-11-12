# Serialized collision grid to runtime view

`collision_grid_from_level` is a new ownership adapter, not an original
routine. It projects the recovered serialized world into `CollisionGridHead`.
The original reader `0x439FD3` passes `following_value` to `0x43AA63`, which
allocates `height << 8` integer cells at world `+0x41C` and stores the height
at `+0x418`. The reader subsequently loads `second_integer_array` into that
same array. The intervening arrays are at `+0x42C` and `+0x43C`.

REA pseudocode evidence for `0x43AA63`:
`ev_6d8c506a7cf95de4e615b442d3dde7ac8bd38f74387625f49a155b18d2248787`.
Its complete 46-byte body is pinned in `tools/verify_reference.py`.

The adapter copies the exact integer bit patterns and validates dimensions.
It builds a view on demand so that moving the storage cannot leave a stale
self-pointer. The validation and vector ownership are tooling choices.
All 105 installed level entries pass the dimension, count, and cell-bit
comparison on x64 and x86. This is serialized-data evidence; it does not
claim native execution of the original grid allocation.
