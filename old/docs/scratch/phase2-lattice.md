# Lattice array placement

Observation, measured from `lemmings.box` (102 `COMPILED\*.LVL` entries).
The three levels checked by name are `MM_CLIMB_002`, `MM_FLOAT`, and
`VH13_KF_1`; `MM_BUILD` is included because the handoff named it. The
corpus counts below were recomputed on all 102, not taken from a comment.

The Cylinder token is the byte string `u32 8, "Cylinder"`. There is exactly
one per level, and `LevelIndex`'s length-prefixed string scan finds that
same offset in all 102. "String end" below is 12 bytes after the token.

The walk from the string end matches `CollisionGrid.cpp`: 5×`u32`, one
height `u32`, one `f64`, then `u32 N` and `N` words. `N` is 49858 in all
102. The collision count `C` is 24576 = 256×96 in all 102. The `u32`
immediately after the collision payload is 12288 in all 102.

## Offsets

File offsets, hex. `grid0 old` is where the previous reader started: the
first `u32` 0xFFFFFFFF within 128 bytes of the string end. `tube` is the
first word of `u32[N]`. `collision` is the first cell, after the count `C`.

| | MM_CLIMB_002 | MM_FLOAT | VH13_KF_1 | MM_BUILD |
| --- | --- | --- | --- | --- |
| file bytes | 699117 | 1345510 | 1328564 | 1290618 |
| header[3] | 110592 (27648 words) | 21568 (5392) | 20736 (5184) | 20480 (5120) |
| Cylinder token | 0x9E4B | 0xE89D | 0x113F1 | 0xA8A0 |
| string end | 0x9E57 | 0xE8A9 | 0x113FD | 0xA8AC |
| grid0 old | 0x9E7B | 0xE8D1 | 0x11425 | 0xA8D0 |
| old, relative to tube | +0 | +4 | +4 | +0 |
| N's word | 0x9E77 | 0xE8C9 | 0x1141D | 0xA8CC |
| tube [start, end) | [0x9E7B, 0x3A983) | [0xE8CD, 0x3F3D5) | [0x11421, 0x41F29) | [0xA8D0, 0x3B3D8) |
| R (12-byte records) | 0 | 0 | 22 | 0 |
| collision cells | [0x3A98B, 0x5298B) | [0x3F3DD, 0x573DD) | [0x42039, 0x5A039) | [0x3B3E0, 0x533E0) |
| 12288-count word | 0x5298B | 0x573DD | 0x5A039 | 0x533E0 |

`header[3] == header[4]` in all 102. `header[3]` is a multiple of 64, from
20480 to 111936, 30 distinct values. It does not occur as a `u32` in the 24
words that begin at the Cylinder token, in any level. `N*4` is 199432, which
is none of those 30 values.

## What the N words are

Observation. Every word of `u32[N]` is either 0xFFFFFFFF or strictly less
than that level's surface vertex count, in all 102 levels. There is no other
word class in the array. The last non-empty word is at index 49856 or 49857
in all 102, so the array is occupied through its end, not padded after a
shorter used region. Non-empty slots occur in every tenth of the array
(corpus totals roughly 6k–13k per decile).

`(highest + 1) / surfaceCount` on the whole array is at worst 0.993
(`M18_MT_1`). Unique indices cover the whole surface pool on the four named
levels except `VH13_KF_1` (595/617) and, in the wider sample, a few others
in the high 0.96s.

key+1 median distance is within 1 of thickness 10 in all 102. key+16 median
is within 1 of the inner chord (≈9.80) or the outer chord (≈11.76) in all
102. That is the same lattice test `test_cylinder` already applies, run on
the whole array rather than on the header[3] window.

Hypothesis, not claimed: the engine draws from all 49858 words. The file
stores one counted array of surface indices. What the renderer keeps in
memory was not re-read for this note.

## The old window is not a boundary

The previous reader copied `header[3]` bytes from the first 0xFFFFFFFF, then
another `header[3]` bytes as grid1.

The first 0xFFFFFFFF is word 0, 1, 2, or 4 of the tube (offsets +0, +4, +8,
+16). Counts: 37, 30, 34, and 1 level. The skipped leading words are
themselves surface indices:

- 37 levels skip nothing (word 0 is already 0xFFFFFFFF)
- 30 skip `(0,)`
- 33 skip `(0, 4)`
- 1 skips `(4, 0)`
- 1 skips `(0, 4, 10, 7)`

So on 65 levels the window started late and dropped real indices.

The window also ends early. On 92 levels both header[3] slices fit inside
the tube, and the tail after them still holds surface indices that appear in
neither slice (median 641 such vertices, max 1442 on `H08_MT_1`). On
`MM_FLOAT`, grid0's unique indices are 516 of 1558 vertices; the tail adds
901 that grid0 and grid1 never mention. `MM_BUILD` grid0 holds 33 of 834;
the tail holds 752 more.

On the other 10 levels, `2 * header[3]` runs past `N`. grid1 then contains
the record count, the records, and the front of the collision grid. Those
ten are `MM_CLIMB_002`, `MM_BASH_002`, `MM_W_012`, `MM_E1_3_SKILLS`,
`MM_E1_4SKILLS`, `M08_KF_1`, `H17_MG_1`, `M06_KF_1`, `VH22_JC`, `E06_KF_1`.
This is the "grid1 carries larger values in 10 levels" note: the larger
values are the collision grid, not a second index array. `MM_CLIMB_002`'s
old grid1 ends at 0x3FE7B, which is 21744 bytes after the collision cells
begin at 0x3A98B. `MM_FLOAT`'s old grid1 ends at 0x19151, which is 156300
bytes short of the collision cells at 0x3F3DD. Zero of 102 end on the
collision grid. They are not its neighbours; they are a slice of the
preceding array, or an overrun into it.

The `u32[N]` payload is not identical across levels. Compared with the first
level in archive order, the other 101 differ (several at byte 0 of the
payload). `CollisionGrid.h` calls this array "the tube, which is the same
in every level" and points at `Cylinder.h`. That identity is the ring mesh,
which is a different object earlier in the file. This `u32[N]` is per-level.
That header was not edited here.

## What changed in the loader

`indexLevel` now reads `u32 N` at string-end + 32 and copies `N` words into
`grid0`, starting at the tube payload. `grid1` is left empty. `header[3]`
is not used as a length.

On the four named levels, `detectRingStride` on this array returns 8224
with median distance exactly 16. The old window cannot hold a stride of
8224 when `header[3]/4` is 5392 or 5120, which is why `MM_FLOAT`'s window
scored 1568 and `MM_BUILD` / `VH13_KF_1` scored none.

## Corpus numbers that moved

`test_corpus` surveys the lattice raster. Only the raster side moved; the
stored-grid side is unchanged (81 releasable, median solid 39.7%, median
drop 9). `test_collisiongrid` was not edited.

| | header[3] window | whole u32[N] |
| --- | --- | --- |
| non-empty masks | 101 (VH13_KF_1 empty) | 102 |
| median solid | 3.9% | 27.5% |
| median entrance drop | 132 rows | 26 rows |
| releasable levels | 22 of 101 | 93 of 102 |
| entrances counted | 143 | 144 |
| entrances with floor | 132 | 144 |
| floorless entrances | 11 | 0 |

The extra entrance is `VH13_KF_1`'s. Empty masks are skipped before entrance
counts (`LevelStats.cpp`), so that level contributed nothing until it
rasterised. The density check used to require the stored grid to be five
times the raster. 39.7 is not five times 27.5; the check is now "the stored
grid is still denser". These pins are the new change detector. Nothing in
`SurfaceMesh` was tuned to produce them.

`test_surface`'s MM_CLIMB_002 checks still pass, including ring stride 8224
and the ledge cells. `test_world`'s climber on that mask still passes.
`test_cylinder` key-adjacency checks pass on the longer array.

`LevelStats.h` and `LevelStats.cpp` no longer cite `VH13_KF_1` as an empty
raster. `Cylinder.h` no longer explains the file as two equal u32 arrays.

## Independent check of the stride

Recomputed on `MM_CLIMB_002`, `MM_FLOAT`, and `MM_BUILD`, from the file,
without `SurfaceMesh`. Every non-empty word is a surface index (0 out of
range). Median distance between the vertices named by slots `i` and `i+d`:

| d | MM_CLIMB_002 | MM_FLOAT | MM_BUILD |
| --- | --- | --- | --- |
| 1 | 9.98 | 9.98 | 9.98 |
| 2 | 1.47 | 1.47 | 1.47 |
| 16 | 11.75 | 9.84 | 9.84 |
| 514 | 1.00 | 1.00 | 1.00 |
| 8224 | 16.00 | 16.00 | 16.00 |

`+1` is the thickness pair. `+514` is one world unit of height and `+8224`
is one ring (16). `16 * 514 = 8224`, and `8 * 2 = 16`, so the ring stride
`detectRingStride` returns is 16 rows of the same map `FUN_0043C930` indexes
as `arg3 + 2 * (arg1 + 257 * arg2)` (`2 * 257 * 97 = 49858`). `key+16` is
eight of those columns, one 11.25° step. Observation from the distances;
the argument order of that function is the neighbour note's, not re-proven
here.

## Not claimed

What `header[3]` / `header[4]` mean. They are equal and a multiple of 64.
They are not the lattice length, and they are not a count stored next to
the Cylinder chunk.

The 12288 count after the collision payload is the byte map at `+0x664`
(see `phase2-neighbours.md`), not a second index array. Reading those bytes
as words produces values such as `0x05060605`, which is four byte entries
packed together. It is not `grid0`.
