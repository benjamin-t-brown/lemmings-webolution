# World mesh coordinates and radial depth

The reference SHA-256 and exact PE-backed byte ranges are checked by
`tools/verify_reference.py` and the native comparison runner.

| Address | File offset | Bytes | Role |
| --- | ---: | ---: | --- |
| `0x0043D56C` | 251244 | 1228 | Map logical XYZ to the world mesh |
| `0x0043DA38` | 252472 | 373 | Recompute depth after a Y change |
| `0x0043E580` | 255360 | 30 | Select a 0x600-byte mesh row |
| `0x0043EA20` | 256544 | 27 | Select one of two 0x300-byte radial layers |
| `0x004027C0` | 10176 | 81 | Scale a stored double triple |
| `0x00402680` | 9856 | 94 | Add stored double triples |

The loader `0x0043D0F0` stores its first integer at world +0x28 and its
first block array at +0x18. The array accessor reads its backing pointer at
+0x0C. Each 0x600-byte row has 32 triples for layer zero and 32 for layer one.
These are exactly the serialized `WorldGeometryPrefix::first_blocks`.
The new `world_coordinate_mesh` function makes a view over those bytes;
it is a tooling adapter, not an original routine.

Mapping truncates X to signed integer, divides by eight toward zero, and
masks the sector index to 31. The next sector wraps from 31 to zero. Y is
truncated and divided by sixteen. Values at/above the height select the
last row; the next row is clamped to that row too. The source retains the
original subtraction/division and stored-double rounding at every vector
operation: interpolate radial depth at four corners, interpolate both rows
in X, then interpolate in Y. It does not replace this with a fused formula.

The depth helper maps the original XY at depth zero and one, then maps the
new Y with the input depth. It chooses world X when its radial displacement
is larger in magnitude than world Z, otherwise Z, and divides the moved
displacement by the selected radial displacement. This is a geometric
parameter calculation, not a terrain raycast. There is no degenerate
denominator guard in the original.

REA pseudocode: `ev_46801e9b385932e916a942fa3ff117a369ea3159c63bd5880912539c9a09e922`.
Instruction records: `ev_717758b9238256f00f1248337373c39fea6d23c6650ec0fd8ad10ea65f763cad`
and `ev_3dbe1d46dc21821bf2498faaefa12a2f1ab9410c5a8d486b7b4a7c916abc8b6e`.
Helpers: `ev_8116b4888f85c1eee7e86ce4bdc183ee6be55040675f64306689a6d2793d623b`,
`ev_7e6791242c52f71b604bfcd0ff3dc03620929531c5c8c247c68749df4ba89e2e`,
`ev_e81cee6841a61b91f5d34088a6dd0a9fca8c641eec247a06a5efdbfb576dc6e8`,
`ev_b6843870df6cc52b7d5504912d0936172a8fc8fcdb8d87d13944949b9322b145`.

## Verification and limits

Unit cases cover interpolation, the sector seam, final-row behavior and
depth changes. The x86 harness executes both original routines and every
dependency unchanged, with no call adapters. At both O0 and O2 it matches
20,000 synthetic mapping/depth pairs and 10,500 pairs from all 105 installed
levels. The comparison checks double bits, treating NaN results as a class.
It reads installed assets in place. No asset data is checked into the repo.

The public API rejects invalid mesh dimensions and out-of-range row access;
the original has unchecked memory access there. Comparison coverage is
finite coordinates and finite mesh data, including negative/outside X,
sector/row boundaries and extrapolated depth. Exceptional CRT handling and
full-game FPU initialization are not established by this harness.
