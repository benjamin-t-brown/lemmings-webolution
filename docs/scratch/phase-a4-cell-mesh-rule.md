# A4c — the collision-grid cell-mesh rule, decoded and reproduced

2026-10-19, branch `decompile`. **A4c checkpoint, not a pixel-fidelity pass.**
The per-cell mesh rule is decoded, a stored-data generator reproduces the retained live arena
exactly, and production now implements it. The nine retained views still fail the unchanged
`0.02` gate; the remaining gap is the draw/texture binding, not the cell topology.

Authority: stock 640x480 `Lemmings Revolution.exe`, SHA-256
`32675bdb49866e76502713251fdfd2b09078c2481c72ca22ff68ebf4b3737a2f`.
No live game process was started or driven. REA (Ghidra) was opened read-only and closed.

## Verdict

`FUN_004615E9` is the **gap pass**, not a face emitter. For each solid rectangle produced by
the collision grid, it scans the cells just outside the rectangle's left, right and top edges
and records runs of **non-wall** cells. `FUN_00463410` then assembles two banks:

- **mesh0** — the rectangle's outer quads in stored order, then right-edge gaps, then
  top-edge gaps;
- **mesh4** — the same rectangle quads re-sorted (`x1` descending, `yStart` ascending), then
  left-edge gaps, then the same top-edge gaps.

`FUN_00461E1D` builds the eight per-column scratch lists at `0x4ECA90` and merges identical
vertical runs across consecutive columns into the 22-byte rectangle records at `0x4ECA80`.

**The key correction:** the `u32[49858]` array on the `Cylinder` walk
(`LevelIndex::grid0`) is the engine's **`(x, y, layer) -> pool index` table** at
`world+0x42C`, not a face list or a mere permutation. Key
`layer + (x + y*0x101)*2` (`FUN_0043C930`, `0x43C93A 69c001010000`,
`0x43C948 8d044a`); value is the `CylinderMesh::surface` index or `0xFFFFFFFF`. The loader
adds `world+0x42C` (`0x43A07C 81c12c040000`) and calls `0x43B640` (`0x43A082`). On
`MM_CLIMB_002`, exactly **897 of 49,858** slots are populated, values `0..896`; the rule
touches exactly those slots.

## Offline reproduction

`tools/harness/generate_cellmesh.py --compare` reads only the stored `.LVL` (collision grid,
`grid0` key table, surface-pool size). The retained A1 live arena is used only as an oracle.
The facilitator re-ran it:

```text
30 static pins                             ok
key table slots 49,858   populated 897
mesh0 389   mesh4 387
distinct keys touched 897   unresolved 0
key support == stored table support        True
mesh0 389/389 quads match, 0 bad in 0 cells
mesh4 387/387 quads match, 0 bad in 0 cells
ALL QUADS MATCH (indices, UVs, colours, order, both banks)
```

The comparison covers all four corner indices, all four UV pairs, the colour double and the
quad order. The captured colours are `(255,255,255,0)`, `(165.75,165.75,165.75,0)` and
`(216.75,216.75,216.75,0)`; the engine writes one double repeated three times.

## Production integration

- `src/assets/CellMesh.{h,cpp}` — the rule, reading `LevelIndex::grid0`, `CollisionGrid` and
  the surface-pool size. It reports `keySlots`, `keySlotsTouched`, `unresolved` and
  `keySupportMatches`.
- `src/__test__/test_cellmesh.cpp` — file-derived: exact key support, zero unresolved,
  `mesh0 == 389`, `mesh4 == 387`, the quads/cell histogram, 163 dual-bank cells, 802 pool
  indices referenced by the banks, every index inside the pool. Nothing reads `build/tmp`.
- `src/app/TubeScene.{h,cpp}`, `src/app/main.cpp` — `--geom cellmesh` draws `mesh0` then
  `mesh4`, textured from the outer tile at `(material % 8, material / 8)`.
- `cmake/webolution_sources.cmake`, `CMakeLists.txt` — source and test registration.

## Independent verification

- `cmake --build --preset dev-debug` clean; **18/18 CTest pass** (was 17/17).
- One-term mutation: `g.end = y` -> `g.end = y + 1` in `CellMesh.cpp` made `test_cellmesh`
  fail (`key slots 897 touched 820 unresolved 224`, four invariants failed) while the quad
  counts stayed 389/387. The original source was restored byte-for-byte
  (SHA-256 `c23717463b016005c81ea6f08f7de07094fdbbfad8ac740ef991e905aadbfb4c`), rebuilt and
  the test passed. The quoted counts alone would not catch this; the key-support invariant
  is the discriminating term.
- Re-rendered all nine retained views with `--geom cellmesh` and independently recomputed
  the ROI errors from the PNGs and retained originals. `wall` was re-rendered too and still
  matches the A4a baseline exactly.

| View | `wall` | `cellmesh` |
| --- | ---: | ---: |
| initial | 0.037812306 | 0.037620585 |
| mouse | 0.036884047 | 0.036884047 |
| right | 0.033855549 | 0.033855549 |
| left | 0.048969015 | 0.048613037 |
| canonical | 0.037815717 | 0.037623996 |
| zoom 1 | 0.064587069 | 0.061990911 |
| elevation up | 0.072452323 | 0.065215334 |
| elevation down | 0.050969147 | 0.050969147 |
| zoom-0 elevation input | 0.037972756 | 0.037781035 |

**0/9 pass.** The cell mesh is better or equal in every view and identical in three, which is
consistent with it being the runtime topology, but it does not close the gap. A2 cavity
`0.128997489 -> 0.126905779` (better) and upper rim `0.347863254 -> 0.352763857` (worse);
both still fail. The A2 wider and A3 sensitivity failures remain.

No live process remains; the install hash is unchanged; `git diff --check` is clean.

## Limits

- Verified on **`MM_CLIMB_002` only**. No all-level sweep. The door path
  (`0x400` cells, `R x 12` records) is implemented but unexercised: this level has zero door
  cells and `R == 0`.
- The **draw pass and texture binding are inferred**, not measured. `--geom cellmesh` draws
  the constant wall first and then adds both cell banks for every cell; it does not apply
  A2's mesh0/mesh4 route selection, INN/OUT bank choice or center filters. That is the likely
  reason the pixel score barely moves, and it is the next thing to settle.
- `FUN_00463100`'s whole-block wall quad (`entry+8`) is recorded as key support but not drawn
  by `--geom cellmesh`; the 95 pool vertices it uses are not in the two banks.
- The colour operands are read from `FUN_004608BB` and match the captured arena; they are not
  claimed to vary by level.
- No pixel-fidelity claim. The retained A2/A3 failures stand.

## Next question

Wire the decoded cell mesh into A2's actual submission route: which pass consumes
`entry+0` vs `entry+4`, which bank uses INN vs OUT tiles, the side/center filters and the
draw order, and whether the constant wall should be replaced or retained where the cell bank
is absent. Re-score the nine views under the unchanged gate, with an isolated mutation.

## Reproduction

```text
python tools/harness/generate_cellmesh.py --compare
.\scripts\Invoke-Ucrt64.ps1 "cmake --build --preset dev-debug"
.\scripts\Invoke-Ucrt64.ps1 "ctest --preset dev-debug"
python tools/harness/score_a4_views.py --label cellmesh --geom cellmesh --render
```

The generator's static pins are in `tools/harness/generate_cellmesh.py`; local render/report
artifacts are under ignored `build/tmp/`.
