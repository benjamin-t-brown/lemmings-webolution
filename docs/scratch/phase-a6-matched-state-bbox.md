# Phase A6d3: matched-state bbox — two boxes improve, slope walker regresses

Brief `build/tmp/a6d3-matched-state-bbox-brief.md`. Packet
`build/tmp/a6d3-matched-state-bbox/`. Cursor `round-agent` on `composer-2.5-fast`,
dispatch `502d6174-fb0f-47ad-a71d-2dea4f85f73e`. One live start (pid 37012).

## Result

- **MEASURED:** one `MM_CLIMB_002` pause (`ui_state` 1, 10 s walk) retains
  `original-live.png` plus three walker `+0x08` rows. Camera block SHA
  `7e6e1a8a…` matches the A1 reference-state bytes. `dgVoodoo.conf` restored;
  install hashes before/after equal.
- **MEASURED:** `defaultMmClimb002InjectedWalkers` is that live table
  (`60.24485,32,0.75`, `45.81248,32,0.75`, `32,34.47959,0.5`).
- **Falsified (A6 gate clause 2)** on that same screenshot:

  | Actor | Rect | Tube-only MAE | With actors |
  | --- | --- | ---: | ---: |
  | 0 | 467–480 × 282–300 | 0.113223 | **0.079130** |
  | 1 | 397–408 × 292–310 | 0.105638 | **0.089430** |
  | 2 | 315–326 × 283–301 | 0.100898 | **0.112465** |

  Actor 2 is the slope walker (`+0x08` Y 34.48, Z 0.5). Boxes are our overlay
  vs tube-only (three largest components), not native `+0x128`. Compose still
  uses walk **temporal 0**.

## Facilitator check

2026-10-20, Cursor facilitator.

- Independent MAE from the three 640×480 PNGs matches `bbox-mae.json` exactly.
  Actor 2: 198/198 box pixels already differ from the original under tube-only;
  painting our sprite changes 106 of them and **raises** MAE.
- Isolated `if (false && …)` on the `main` inject: default shot hashes
  `66a8ea3b…` (tube-only). Restored `main.cpp` byte-identical to backup.
- Fresh build, 23/23 CTest. No game. Empty-list initial hash unchanged.

## Parked

Matched-state clause 2 after three A6 integration rounds (A6d–A6d3). Resume
with an offline temporal/view sweep on actor 2 against `original-live.png`.
Evidence: this note.
