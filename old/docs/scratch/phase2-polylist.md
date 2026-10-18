# PolyList kind 5 and kind 7 tails

Observation, not a guess at a vertex format. Re-run with
`python tools/harness/read_polylist_tail.py`. It reads `COMPILED\*.LVL` from
`lemmings.box`, with `patch.box` overriding a shared name the way
`AssetLibrary` does. That is 102 levels. The three checked by name are
`MM_CLIMB_002`, `MM_FLOAT`, and `VH13_KF_1`; the ranges below also hold on
the other 99.

## What the bytes are

The parser already named the first 52 bytes of a record: `u32` index at +0,
then little-endian `f64` u, v, r, g, b, a at +4, +12, +20, +28, +36, +44.

Kind 5 records are 160 bytes and kind 7 records are 212. Those lengths are
exactly more copies of the same 52-byte vertex, plus one trailing `u32`:

- kind 5: 3 × 52 + 4 = 160
- kind 7: 4 × 52 + 4 = 212

Reading the tail as further `f64`s on the old +4 alignment does not work.
That split lands in the middle of the next index word and produces
non-physical magnitudes (about 1e154, and denormals). The 52-byte repeat
does not.

Corpus, formula-matched blocks only (kind 5: 19,391 blocks / 110,347
records; kind 7: 11,161 blocks / 50,354 records):

- Every double on every vertex is finite.
- `r`, `g`, and `b` are integers in [0, 255] on every vertex of both kinds.
- `a` is in [0, 255] on every vertex. Kind 7 alpha is always a whole number.
  Kind 5 alpha is a whole number on 86,873 of 110,347 records and 178.5 on
  the other 23,474. 178.5 is the only non-integer value, and it is 255 × 0.7.
  It is copied onto all three vertices of those records.
- On more than 99% of records, every vertex in that record has the same `a`.
  `u`, `v`, `r`, `g`, and `b` differ from vertex to vertex inside a record.
- `u` and `v` sit in the same band as the leading vertex: mostly [-0.01, 1.01],
  with a minority up to about 2.99 and a few slightly negative (about -0.025).
  Kind 7 is the looser of the two (leading `u` inside the unit interval on
  about 87% of kind-7 records).
- Indices on every vertex are in [0, 1333]. Later vertices are not copies of
  the first (corner 1 equals corner 0 on 0% of kind-5 records; the largest
  such share is 1.3%, kind 7's fourth vertex). Successive indices often
  differ by a small integer, +1 being the most common single step, and many
  other steps occur.
- Against that level's surface pool, vertices 0..3 hit on 87.3%, 87.2%,
  87.2%, and 90.6% of records (vertex 3 only on kind 7; `test_polylist`).
  The leading vertex, already parsed before this pass, is the 87.3%. On
  `MM_CLIMB_002` some later indices reach 932 while that level's surface has
  897, so "index into the surface" is the same partial fact it was for the
  first vertex, not a new guarantee.
- The trailing `u32` (`endWord`, kind 5 at +156, kind 7 at +208) is not zero
  and not constant. Kind 5 ranges [0, 10456], kind 7 [0, 9296]. The first
  two dozen distinct values cover only 22,720 of 110,347 kind-5 records, so
  the rest are other values. It changes inside 14,863 of 19,391 kind-5
  blocks and 8,597 of 11,161 kind-7 blocks. It matches header `u2` on 0% of
  kind-5 records and 10.3% of kind-7 records, and matches `u3`, the kind, or
  the block count even less often. Frequent values include 248, 192, 368,
  944, and 1408, mixed with small integers.

One kind-5 record from `MM_CLIMB_002` at file offset `0x634bb`, as a worked
example of the same layout (the ranges above are the claim, this is one row):

- vertex 0: index 0, u = v = 0.989583, rgba = 255
- vertex 1: index 1, u = 0.471354, v = 0.888021, rgba = 255
- vertex 2: index 2, u = 0.872396, v = 0.580729, rgba = 255
- trailing u32 = 1

`MM_FLOAT` and `VH13_KF_1` use the same shape. Both kinds appear in each.
No non-finite double on either.

## What this does not say

The trailing `u32` is unread by this parser's names on purpose. Nothing in
the histogram identifies it, and it is not padding.

Three vertices versus four is the record shape. That is consistent with a
triangle and a quad, and this note does not claim the draw call does that.
Kind 4 (3,763 blocks) and kind 6 (3,691 blocks) do not match this stride and
were not decoded. The loader in the stock executable was not consulted: the
stride identity and the field ranges already agree on all 102 levels.

Normals, a second UV pair, and a D3D flexible-vertex-format layout were the
cheap wrong readings of "108 more bytes". The bytes are another copy of the
vertex that was already named.
