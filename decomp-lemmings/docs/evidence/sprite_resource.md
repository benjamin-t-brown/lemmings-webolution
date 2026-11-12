# Sprite animation resources

Reference executable SHA-256:
`32675bdb49866e76502713251fdfd2b09078c2481c72ca22ff68ebf4b3737a2f`.
Source: `sprite_resource.hpp` / `sprite_resource.cpp`.

## Static recovery

The root reader `0x483EF0` reads a clip array and a second array of tagged
triple arrays. Clips contain frame arrays, frames contain piece arrays, and
each piece contains four 32-bit values, four binary64 values, another four
32-bit values, and a texture name. A clip's frames precede its name and speed.
The inline clip name capacity is 128; texture names use capacity 260. Both
delegate to the already recovered `0x47D940` string reader.

`0x47C643` reads a **type**, then uses `0x47C810` to determine the triple count:
type 0 means 16, type 1 means 256, and any other value means zero. The allocator
`0x47C6A8` has a separate oddity: values other than 0/1/2 allocate 16 triples,
although the reader fills none. Allocation layout is outside this serialized
representation. The triples resemble palette values in installed resources;
their semantic role is left unnamed pending recovery of their consumers.

The serialized representation retains every numeric bit, including signaling
NaNs. Original x87 loads/stores may quiet signaling NaNs when constructing
runtime objects. Texture lookup `0x474519` and GPU resource construction are
not implemented. Counts that cannot fit the remaining input or the original
signed count domain are rejected instead of reproducing invalid allocations.
`sprite_animation_clips` exposes the clip name, speed, and frame count to the
recovered playback API. Its returned vector owns the metadata.

REA evidence IDs (also retained in the local evidence bundle):

| Routine | Evidence ID |
| --- | --- |
| 483EF0 | `ev_35924634faf5670eccd0f5a985f200c4f558e92d1e578e1eb631ff6dfad5cfc3` |
| 484160 | `ev_1556e0e6651032a3deca39fdd1ecbbbb7889e95a7083f59fbc822448d6845f12` |
| 4841E0 | `ev_f4287bfe7365b76339599016fe55a14e24630f40763869bd537a34fa8e83e1de` |
| 485800 | `ev_b8a70edcec23f69f7f412dd1c6987c534f9adaab860542fad79cc4b20d4d0dd2` |
| 485840 | `ev_9cff623b63a59fcdc382ac00604baa3f1f308e2b7dd061f45132625dc58f5a91` |
| 485A80 | `ev_dfbf7db1f1276d420be8b2729c97b819d23b9c89fa627f44739081c86994bd79` |
| 485AA0 | `ev_f6abd337b6c49bb20429c66ec358ccc21b7b6ec2fc7af5b131c34b93ed59dc2a` |
| 485CE0 | `ev_2f6d8dc42f2185022b46a396b90efa4b6af37d3b15e07e1274259cf8e309be9b` |
| 47C643 | `ev_807b28d2a93b9dc19c925087182e2d8a2000eaeaff06c7fd519a9feca80db957` |
| 48252D | `ev_c230e1bbb750437a1bf3f72d3cccfe43b0587c6327e498da6e4c966deb408b34` |
| 480BB0 | `ev_282a2d33ab4209541d2571cb2c1f5c2c18494b956ba4c149028ef3c048736994` |
| 47C6A8 | `ev_b69104cccc505c2f38b159eb8c8967cf1b18933090f4cffc0c38618cb27a89bf` |
| 47C810 | `ev_f99245f604d67ac7c5ce0e3e63c2fcccf48c1ddca473589cac89dfe2fb6a038e` |

## Byte pins

Each row is `(VA, PE-mapped file offset, byte count, SHA-256)`. Mapping and
digests were computed from the verified executable, not a flat RVA assumption.

```python
    (0x00483EF0, 540400, 44, "4085809ff4717a079aaa755c5cc685f3947662685fdc8b39dfbeaea07ffdf74e"),
    (0x00484160, 541024, 120, "e86ea86a512f55cfbf60bf5cb30566a23c84f683bcb1871572bf5eb73544f88d"),
    (0x004841E0, 541152, 115, "7d0784e4ebe44406f6a8ba6658eaf70e48780ab0b7b4af04d124f0f269ad3ff9"),
    (0x00485800, 546816, 56, "88d296305d52000fa827a6c63aa4e9a3e62f094878698fbab3794f7c3da30a22"),
    (0x00485840, 546880, 117, "8357651e4a1e9f4c9426e9e18976c2dd4c31ca363eadf879419d7c21792d6df2"),
    (0x00485A80, 547456, 27, "b889ac8800cb2d68d5a3ee270635888787a3afdc92b88f27cc172629f14ef9bb"),
    (0x00485AA0, 547488, 117, "9f05a8d9ba38c9a50065bb54b7eb52b9122094aaa0633e8500963d4de5e45044"),
    (0x00485CE0, 548064, 269, "f823e4183fb5cf6dddcddf56827596e577eb4923ac6739de17c92a45db59e887"),
    (0x0047C643, 509507, 101, "6fe386b3451c2143242a73ac9bec30e0c9dacd77ff3e11020b04b595b93e87e3"),
    (0x0048252D, 533805, 30, "0e251290ebe7640b7655f043e1ad35014e1cfb34afa25376d6927fd91fe15145"),
    (0x00480BB0, 527280, 54, "657dbef004ae2a8d230c1d82eb021ca4dfaedfcf59599a6e9c8d666594c0f454"),
    (0x0047C6A8, 509608, 210, "7e1446a537f4298771470f35a0482f32b4e0f3750eb8b8ffb4b4235317442e72"),
    (0x0047C810, 509968, 43, "8a0197f8967529dc61c70dbabdbf3d5fda58c65ff481a1b3d974cf37a008fcab"),
```

## Verification

Synthetic unit cases cover nested/empty arrays, all triple type branches,
integer extremes, raw NaN payloads, negative zero, embedded NUL names, the
exact string capacity, oversized string declarations, every truncation of a
populated record, impossible counts, trailing bytes, and playback metadata.

Independent Python parsing compares every numeric field (per-frame hashes),
name, array count, speed, metadata conversion, and exact EOF with the C++
archive reader. All **19** installed SPR entries match: **132 clips, 1,526
frames, 7,354 pieces, six type-0 triple arrays, and 96 triples**. Source assets
are read in place; none are copied into the repository. The installed files
exercise only type 0 triple arrays, so type 1 and other types have synthetic
coverage. The unit and installed fixture checks pass on both x64 and x86.
This fixture comparison does not execute the original game code.

The separate x86 native piece oracle executes unchanged `0x485CE0`, scalar
readers `0x402540`/`0x402560`/`0x402580`, string reader `0x47D940`, and skip
helper `0x47DAC8`. Only the raw stream read and texture lookup are adapters.
**20,000** cases match every numeric field, texture name, final cursor,
lookup count, and lookup destination `+0x40`. Cases include random integer
and binary64 bits, both zero signs, subnormals, infinities, quiet/signaling
NaNs, embedded NUL bytes, empty names, capacity 259/260 boundaries, and
oversized declarations. The comparison explicitly projects stored signaling
NaN bits through x87's quieting behavior. It does not claim native execution
of the outer allocation loops or texture loading.
