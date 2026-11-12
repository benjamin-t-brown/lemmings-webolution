# Sprite renderer submission and device state

Reference executable SHA-256:
`32675bdb49866e76502713251fdfd2b09078c2481c72ca22ff68ebf4b3737a2f`.
The accompanying `.asm` records original bytes, mapped offsets and SHA-256 pins.
The `.metadata.json` records REA evidence IDs, procedure information and
pseudocode. The native runner checks the whole executable and all 18 executed
routine spans before making the private in-process executable mapping.

## Recovered scope

| Address | Bytes | Behavior |
| --- | ---: | --- |
| `414920` | 231 | Texture acquisition, binding, blend state, scratch reset, quad pointer preparation and clipping dispatch. |
| `414A10` | 47 | Acquire texture, release its cache reference, return acquired pointer. |
| `414B90` | 178 | Cache acquire specialization, composed with the existing source cache implementation. |
| `414D20` | 55 | Cache release specialization. |
| `414D00` | 30 | Entry indexing by stride 136. |
| `47261F` | 858 | Cached texture binding and stage/render state changes. |
| `414D80` | 211 | Cached blend mode changes. |
| `414B70` | 32 | Reset scratch used count, then scratch vector count. |
| `414D60` | 19 | Scratch count-reset wrapper. |
| `414EF0` | 20 | Set vector count to zero. |
| `414F10` | 22 | Pointer vector constructor wrapper. |
| `414F30` | 53 | Initialize count/capacity/growth/data to 0/0/1/null. |
| `414A60` | 257 | Resize pointer vector, preserving original allocation/copy/free order. |
| `414A40` | 25 | Pointer element indexing. |
| `4150A0` | 15 | Global pointer-vector cleanup entry. |
| `4150B0` | 19 | Cleanup wrapper. |
| `4150D0` | 19 | Cleanup wrapper. |
| `4150F0` | 71 | Free pointer storage and clear vector count. |

`sprite_render_submission.hpp/.cpp` implements these operations. Texture fields
are projected through live references so `TextureCreationResource` can be used
without duplicating its layout. Unknown model and vertex bytes are untouched.
The first model word is now established as the blend-mode argument. Texture
object +00 is the interface passed to device slot +98; +24 controls state
selection. The source deliberately uses observed slot numbers rather than
claiming a tested graphics backend.

## Device order and state

Binding skips everything if the texture object identity matches the cached
identity. Otherwise it caches that identity before calling device slot +98 with
stage zero and texture +00. It then compares live texture flags with cached
flags and, if different, caches the flags before selecting a branch.

Flags bit 3 has priority over bit 4. Otherwise the routine first writes the
three color-stage states, then rereads flags bits 0/1 to choose alpha testing.
The non-alpha-test branch disables render state 15 before rereading bit 2.
Alpha reference is reread immediately before render state 24. Device pointer is
reread for every COM call. All return values are ignored.

Blend mode skips calls if the requested mode is cached. The cache changes before
the first call. Mode zero writes `(27,0)`. Other modes write `(27,1)` followed
by `(19,2),(20,2)` for mode 2, or `(19,5),(20,2)` for mode 3, or
`(19,5),(20,6)` for every other nonzero mode. Later branching uses the original
mode argument even if a callback mutates the cached mode.

Submission reads model blend mode after binding callbacks. It resets scratch
words `58D598`, then `58D588`. Lazy initialization changes only bit zero of
`5D6160`, constructs vector `5D6170`, and registers cleanup `4150A0` with the
CRT. After resizing to four it writes pointers in order **0, 3, 2, 1**, then
calls clipping routine `46D3C1` once.

## Array behavior

Resize compares signed count and capacity. Growth 1 assigns exact requested
capacity. Other growth values add with 32-bit wrapping until capacity is
strictly greater than the requested count. Positive capacity allocates a plain
array of original stride four, with no cookie. After allocation callbacks it
rereads old data/count, copies live elements, frees the current old pointer,
then publishes the captured new pointer. Count is assigned last.

Nonpositive resulting capacity frees nonnull data then stores null. Cleanup
sets capacity zero before freeing nonnull data, nulls data after the callback,
and sets count zero last. With null data it preserves capacity. Growth remains
unchanged. Hooks can mutate fields subject to valid subsequent accesses.

## Verification

**Static:** all listed routines were inspected against their original bytes,
including low-byte lazy flag access, signed growth branches, callbacks followed
by live field rereads, and the thiscall clipping entry. REA also records the
downstream clipping/drawing routines as context; this slice does not claim them.

**Units:** x64 `-O2` and x86 `-O0` pass texture flag priority, identity caching,
balanced cache counts, scratch reset, lazy flags, quad ordering and cleanup
ordering checks.

**Native:** x86 `-O0` and `-O2` each pass **129,024 complete submission
comparisons**, plus **256 resize and cleanup pairs**. The original 18 routines
execute unchanged. The adapters are texture cache search/load (`414C50`,
`414E60`), heap allocation/free, CRT cleanup registration, final clipping, and
device COM methods. This is not a claim of executing clipping or a GPU driver.

The fixture compares all model bytes, every device/heap/cache/dispatch trace,
cache reference counts, array headers and normalized contents of all old/new
buffers. Cases cover all low-five-bit texture combinations, identity/flag cache
hits, negative/zero/positive blend modes, lazy and initialized arrays, three
growth values, cached/found/loaded resources, wrapping reference counts and
negative/zero resize arguments. Sixteen callback scripts cover changed device
pointers, flags, alpha reference, model blend mode, cache-entry storage,
allocation-time old data/count changes, free-time header changes, cleanup
registration changes and final clipping mutations.

## Bounds

This is normal-return recovery. Valid resource indices and live object/array
storage, successful allocations, and terminating growth are required. Host
pointer storage uses host pointer width while allocation hooks receive the
original x86 byte count. Allocation failure, exceptions and nonterminating
growth are outside the tested domain. Graphics interfaces and clipping remain
explicit boundaries; no visible rendering or playable-game claim is made.
