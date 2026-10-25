# C3w — source-only scene integration map

2026-10-25. Read-only explore return retained by the facilitator at
`build/tmp/c3w-scene-integration-map.md`; no original-game measurement, REA,
code edit, render or stock start.

The existing `LevelIndex` string scan and `LevelObjects` placements are not a
recursive scene-node reader. `PolyList` is a next-tag/size-formula diagnostic
without node/material context. `TubeScene`'s optional PolyList route uses fixed
UV/tile binding, not accepted C3 submissions. `Raster::triangle` is RGB-only,
bilinear wrap with one depth-enabled switch and unconditional passing depth
writes, not the C3 explicit-state model. `main` composes TubeScene then overlays.

## Facilitator check

Read `PolyList.h/.cpp`, `Raster.h/.cpp`, `LevelObjects.h`; checked TubeScene/main
call sites and source-list locations. These verify the source-navigation claims,
not the original executable's behavior. Legacy16-byte-header/tail labels must
not be copied into the new source reader: accepted C3n layout preserves leading
normal indices and per-node material context. Do not rewrite existing diagnostics
as a side effect of the next task. No byte addresses or fidelity scores are added.

The worker recommended submission adapter, explicit-state raster and app wiring.
Facilitator split the missing **strict asset reader first**, then explicit-input
submission adapter, raster/state support and final app integration. The first
is independent of pending pause state/format values, so C3x implements it without
changing the current app render. No CTest rerun specific to this no-edit map;
the last source-unchanged full suite passed34/34 on C3q. No new gate acceptance.

Dispatch14:53:56; map verified14:59:05 MDT (**5m09s**).
