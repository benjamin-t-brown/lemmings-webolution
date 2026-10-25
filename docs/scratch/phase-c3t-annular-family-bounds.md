# C3t — conditional base/top family bounds, not exact floor visibility

2026-10-25. Offline source-table arithmetic; no production change or native read.
Packet `build/tmp/c3t-floor-ring-bounds/round-note.md`. This is a **partial**
answer to the floor question, not a claim that the complete floor contribution
has been identified or bounded in the original's paused visible frame.

## Accepted diagnostic

The selected source families are first-root `thenull/base` and `thenull/top`:
128 kind-5 records, 384 source corners. Exact source record offsets are in
`selection.json` / `source-polygons.csv`. Base node/vertex offsets are
`0x8ADC9/0x8ADD1`, records `0x8BA20..0x8E180` stride160; top node/vertex offsets
`0x8E29C/0x8E2A4`, records `0x8EEF3..0x91653` stride160. Both have64 records.
Materials are **sand_2/sand_1**, not null. Local XZ radii are roughly6; narrow
local Y, names, colours and parentage support an annular-family interpretation.
Calling them the complete native **floor ring is still an inference**.

Under accepted source-pose/reference-camera transforms and **C3n candidate
clipping**, selected polygons generate171 fan triangles. Coordinate extrema
are `[69.52857971191406,0,568.4714965820312,479]`; conservative half-open pixel
cover is **[69,0,569,480)**. A closed-pixel-square/triangle coverage mask has
**55571** bytes set and SHA-256
`b60b3ebf75b11293fc47419ea43357636f4ffd42d092367fe6937583d26b91cc`.
It includes potentially culled/occluded geometry and is not a GPU raster mask.
Neither128 source records nor171 fan triangles are native visible draw counts.

Format clarification: `polygon-mask-640x480.bin` contains **307200 unpacked
bytes**, each0/1, in row-major order. Despite the candidate key/name
`bits_lsb_first_sha256` and note wording, it is **not a packed LSB-first bitset**.
The byte hash above is authenticated; no consumer should infer bit packing.

## Facilitator check

`python -B build/tmp/c3t-facilitator-check.py` separately reads all128 source
record indices/colours and384 local vertices from the LVL, re-reads node and
ancestor matrices from raw file offsets, and applies them sequentially with the
accepted camera. They match C3n transformed fields within1e-9. Selection offsets
and frozen artifact hashes match. No new PE fact is added, so new native-address
reads are not applicable.

The independent mask algorithm clips each fan triangle against the four closed
pixel-square halfplanes, **not the worker's separating-axis method**. It
reproduces all55571 pixels, byte-for-byte, and the coordinate bound. Moving only
one extreme projected vertex +10 in a private copy increases xmax by10 and
changes143 mask pixels. Restoring that point restores the complete geometry;
original LVL/table remain unchanged. Results in
`build/tmp/c3t-facilitator-check/result.json`. No screenshot or score was read
for source selection or this check.

Fresh wrapper build and **34/34 CTest passed**. No Lemmings process, starts,
attaches, instrumentation or live writes. No scorer, region or gate edit.
Earlier C3n clipping/precision limits and C3b's unresolved semantic floor
identification are not retracted. Use this only as an additional source-family
diagnostic; do not relabel C3's full floor requirement complete. It still lives
inside the unchanged full-frame gate, never an exclusion.

Dispatch14:26:41; worker completed14:34 (minute precision); accepted14:38:14 MDT,
**11m33s** dispatch-to-acceptance. C3 baseline stays0.2050523, above0.02.
Next: verify returned C3r texture packet; C3q device-state worker is still running.
