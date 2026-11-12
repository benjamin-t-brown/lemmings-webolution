# Actor pointer registries

Reference executable SHA-256:
`32675bdb49866e76502713251fdfd2b09078c2481c72ca22ff68ebf4b3737a2f`.

## Static reading

`0x4233A0` (188 bytes) appends a pointer by reference to the array header
`count +0`, `capacity +4`, `growth +8`, `data +0xC`. It is used by the global
base-entity registry `0x4CBB18` and by world registration `0x43B517` (31 bytes)
at coordinate-world object `+0x1E88`. The latter passes a stack-local copy of
the pointer argument. These registries differ from the world actor pointer
registry at world object `+0x1FA0`.

When count reaches capacity, append first changes capacity with wrapping
32-bit addition, then allocates `capacity << 2` bytes. It copies entries against
the current count/data, releases the old buffer, installs the replacement, and
only then reads the input pointer by reference and increments the current count.
Allocation/release callbacks must preserve valid storage and header values.
The portable callback receives original byte size and allocates that many
32-bit slots as host-width pointer slots; x64 storage is therefore wider.

`0x43B536` (114 bytes), with accessors `0x43BB80` (16) and `0x423380` (25),
finds the first exact pointer match and invokes `0x43BB90` (79). Removal shifts
later entries left, decrements count and leaves the stale final slot untouched.
Duplicates after the first match remain. The search's final test is inequality
with count, not a general bounds check; a negative count can decrement again.
Valid nonnegative headers and live indices are the intended calling domain.

[Assembly](actor_registry.asm) includes exact bytes, mapped offsets and REA
evidence IDs. The runner and main verifier pin all six routines. Unknown
ownership/destructor behavior remains outside these pointer operations.

## Unit checks

Tests cover growth before allocation, live input-reference mutation, release,
duplicate removal, stale tail preservation, missing values and wrapper append.

## Native comparison

20,000 cases pass at O0 and O2 using unchanged original append, wrapper,
search/accessors and shifting removal. Only allocation and release are adapted.
Comparisons include complete source/alternate/replacement arrays, header fields,
input pointers, requested sizes and ordered allocation/release snapshots.
The cases exercise callbacks changing count/data/growth/input between reads,
no-growth append, missing/null/duplicate pointers, first/middle/last removal,
and the observed negative-count search edge. Storage remains valid throughout;
out-of-range writes, allocation failure and use-after-release aliases are not
claimed. This is offline original-byte execution, not a live registry capture.
