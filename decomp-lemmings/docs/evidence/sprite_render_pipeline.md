# Renderer composition check

Executable SHA-256:
`32675bdb49866e76502713251fdfd2b09078c2481c72ca22ff68ebf4b3737a2f`.

The recovered submission, clipping and packed draw routines now also have an
**uninterrupted original-chain comparison**. At x86 `-O0` and `-O2`, each of
20,000 cases enters original `414920`, executes texture binding, balanced
cached lookup, blend state, pointer preparation, all four clipping passes,
interpolation, scratch allocation and final `46D460` packing unchanged.
Only heap allocation/free, CRT cleanup registration and device COM calls are
adapted. The runner reuses the three slice runners' executable/span checks.

The source side composes `submit_sprite_geometry` ->
`clip_and_draw_sprite_polygon` -> `draw_sprite_polygon`. The fixture bridges
submission's two scratch reset words to the clip scratch view, then synchronizes
them after clipping. This bridge matters because the source state aggregates
project overlapping original globals into separate readable views.

Comparisons cover the complete heap/cleanup/device call sequence, allocation
sizes, final reference counts and array state, all model bytes, and every
submitted packed vertex. Numeric NaNs compare by classification; other packed
values compare bit-for-bit. Cases cover all32 low texture flag combinations,
seven blend modes, texture identity cache hits, four clipping growth sizes,
inside/crossing/fully rejected polygons, NaN coordinates and zero depths.
Textures are already cached in this composed test. Separate submission tests
cover cache misses and callback mutations.

x64 `-O2` and x86 `-O0` composition units also pass1,000 cases, producing363
device submissions. This is an in-process synthetic comparison, not a hybrid
gameplay run or a driver/visible-rendering test. Interpolated vertex tails remain
indeterminate as described in the clipping note; the final device operation
does not read them.
