# A4a — first production frame; the stored file does not reproduce the live cell mesh

2026-10-19, branch `decompile`. **A4a checkpoint, not full A4 acceptance.**
The round produced the first runnable production `webolution` target and answered the
source-mapping question negatively: the tube skin and camera work from stored data, but the
per-cell runtime mesh does not. All nine retained A2 views still fail the unchanged
fixed-ROI gate; the best file-derived frame is `0.037812306` against `0.02`.

Authority: stock 640x480 `Lemmings Revolution.exe`, SHA-256
`32675bdb49866e76502713251fdfd2b09078c2481c72ca22ff68ebf4b3737a2f`.
The round was offline: no live game process was started, and none remains.

## What landed

- `webolution` target in `CMakeLists.txt`; `src/app/{Camera,Raster,TubeScene,main}.{h,cpp}`
  in `WEBOLUTION_GFX_SOURCES`. SDL2 is linked when found, but the interactive loop is not
  written yet (see limits).
- `src/gfx/TilePosition.h` and `src/gfx/SpriteState.h` restore the retired asset-side
  coverage; `test_tileposition` and `test_spritestate` are registered.
- The executable loads a retail level from `WEBOLUTION_GAME_DIR`, builds the tube skin from
  the level's 48+48 tile names, and renders a 640x480 frame through the A1 camera:
  `webolution --level MM_CLIMB_002 --native --camera <A1 params> --shot <png>`.
- `--flat` is a debug unwrap; the default is the 3D view. No `build/tmp` capture is read by
  production code.

## Core result: the stored surface pool matches, the per-cell topology does not

Independently re-read from the retained `MM_CLIMB_002.LVL` and the retained A1 live capture:

- `CylinderMesh::surface` is **897 vertices** and byte-identical to the live source bank,
  SHA-256 `4e3a52f579c663fc0e97d217d8ea5d780c246f9c6bb33c112534ac6b20c3b630`.
- The true live arena has **450 unique nondegenerate quads**: **229** outer-wall and
  **221** thickness. The production renderer builds the constant tube wall from
  `CylinderMesh::vertices`, which matches the pool.
- The level's **946** kind-5/kind-7 `PolyList 09` records are **not** that topology:
  **8 of 450** unique index sets match exactly; **10 of 450** if cyclic rotations and
  reversals are allowed; **0 of 450** match by full index+UV signature.
- The worker note's “16 of 450” index-set count is **not reproduced** and is narrowed to
  the figures above. The worker's capture-side “389 mesh0 quads” is not a set of distinct
  quads: 384 of those 389 records are zero/degenerate. The arena's 450 unique quads are the
  correct denominator. The worker's own `0/389` full-signature result is consistent with
  `0/450`.
- `SurfaceMesh` lattice faces from `grid0` are depth-occluded in eight of nine views;
  drawing the `PolyList` records as-is makes every view worse.

One file-side presence rule **is** established: building a 32x6 outer-wall occupancy from
the live arena gives **156** present cells. Every one is solid in the stored 256x96 collision
grid; `present & empty = 0`, and 9 solid cells have no runtime wall. The rule is implemented
as `--wall solid`; it is a presence rule, not the missing topology.

## Pixel gate

Unchanged: 640x480, ROI `[100,80,280,410]`, normalized mean absolute RGB error `<= 0.02`.
All nine retained A2 views fail. The facilitator re-rendered all nine independently and
recomputed the scores from the retained originals; every value matched the worker's table
to the printed precision.

| View | Baseline `wall full` | Result |
| --- | ---: | --- |
| initial | 0.037812306 | fail |
| mouse | 0.036884047 | fail |
| right | 0.033855549 | fail |
| left | 0.048969015 | fail |
| canonical | 0.037815717 | fail |
| zoom 1 | 0.064587069 | fail |
| elevation up | 0.072452323 | fail |
| elevation down | 0.050969147 | fail |
| zoom-0 elevation input | 0.037972756 | fail |

The A2 diagnostics remain failed on `initial`: cavity `0.128997489`, upper rim
`0.347863254`. The A2 wider changed-pixel failure and the A3 sensitivity failure are
retained, not re-scored away.

## Facilitator verification

- Rebuilt with `.\scripts\Invoke-Ucrt64.ps1 "cmake --build --preset dev-debug"` and ran
  `ctest --preset dev-debug`: **17/17 pass**.
- Re-ran the executable for all nine views and scored them with a separate direct pixel
  loop. The initial frame is byte-identical across reruns
  (`7d957fcad2b7047e…`).
- Compared the production camera matrix to
  `tools/harness/camera_fixture.candidate_camera` for the retained initial params: the
  12 view values, eye world vector and focal lengths match, with only last-bit double
  differences (one translation entry is `-37.297943310292411` in the production port and
  `-37.297943310292418` in the Python fixture). The pixel failure is not a camera-port
  failure.
- Isolated mutation: `eye_distance 377.516 -> 450.0` in a copy of the retained params makes
  `initial` **0.093566427** (strictly worse); re-rendering the untouched params reproduces
  the baseline `0.037812306` and the same PNG hash.
- No Lemmings process is running. The installed executable still hashes
  `32675bdb49866e76502713251fdfd2b09078c2481c72ca22ff68ebf4b3737a2f`.
  `git diff --check` is clean.

## Limits

- **Headless only.** Without `--shot` the binary renders and exits; there is no SDL window,
  event loop, revolve/zoom input, or discoverable control yet. That is the remaining part of
  A4's entry-point/controls deliverable.
- No sprites, actors, hatch/exit models, background/sky/sand/hose, upper gold band, or HUD.
- The raster uses a CPU screen-space depth key and bilinear wrap sampling; it does not
  reproduce the original's device blend/scissor/mip state.
- The file-side per-cell topology producer is still unlocated. This is the critical-path
  gap: without it, a smooth tube will not match the original's cut-in geometry.

## Next question

Where does the original build the per-cell quad arrays at `0x4ECB00` (indices, UVs, colour,
mesh0/mesh4 selection) from the stored level data? Use the retained arena as an oracle and
static code/data-flow reading to find the producer; do not substitute captured arrays in the
production path. A separate, later round should add the SDL window/event loop and the
verified revolve/zoom controls.
