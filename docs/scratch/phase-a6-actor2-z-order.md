# Phase A6d8: before-tube equals tube-only — Z-order does not fix actor 2

Brief `build/tmp/a6d8-actor2-z-order-brief.md`. Packet
`build/tmp/a6d8-actor2-z-order/`. Cursor `round-agent` on `composer-2.5-fast`,
dispatch `f9376238-48c2-40e4-b812-228e4e029001`. Offline.

## Result

- **MEASURED:** `composeActorsBeforeRoute=true` makes all three A6d3 boxes
  **equal** tube-only (walkers fully covered by the route in those rects).
- **MEASURED:** after-tube still 0.113→0.079, 0.106→0.089, **0.101→0.112**.
  Production stays after-tube. Empty-list hash `66a8ea3b…` unchanged.
- **Falsified:** drawing actors before the tube fixes actor 2.

## Facilitator check

2026-10-20, Cursor facilitator.

- Independent `test_actorcompose` reprint matches `mae-table.tsv` exactly.
- `SceneOptions::composeActorsBeforeRoute` default remains false.
  `TubeScene.cpp` production path unchanged.
- 23/23 CTest.

## Parked

Actor-2 Z-order on the A6d3 pair. Resume with composite/raster (overlay
replace vs A5 alpha test), not another before/after sweep. Evidence: this note.
