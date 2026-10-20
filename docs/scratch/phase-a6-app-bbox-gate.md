# Phase A6d2: walkers on in `main`; bbox gate vs `original-0.png` fails

Brief `build/tmp/a6d2-app-bbox-gate-brief.md`. Packet
`build/tmp/a6d2-app-bbox-gate/`. Cursor `round-agent` on `composer-2.5-fast`,
dispatch `8d13050b-e2f5-4442-86fb-ba60b140464d`. Offline; no game start.

## Result

- **MEASURED:** `webolution` on `MM_CLIMB_002` + `--geom route` now injects the
  three A6a walkers. `--no-injected-walkers` is tube-only.
- **MEASURED:** empty-list 9/9 PNG hashes unchanged (`test_actorcompose`;
  initial still `66a8ea3b…`).
- **Falsified (A6 gate clause 2) vs `original-0.png`:**

  | Actor | Rect | Tube-only MAE | With actors |
  | --- | --- | ---: | ---: |
  | 0 | 463–476 × 281–300 | 0.035167 | **0.114519** |
  | 1 | 391–404 × 291–310 | 0.013474 | **0.095552** |
  | 2 | 312–325 × 272–291 | 0.014167 | **0.085888** |

  Each box has ~110/247 pixels changed by the overlay. Tube-only MAE is
  already > 0, so the original differs there — painting our walk temporal-0
  sprite still **increases** error.

## Facilitator check

2026-10-20, Cursor facilitator.

- Independent recompute from the worker's `tube-only.png` / `with-injected.png`
  and `tools/harness/fixtures/camera_a1/original-0.png` (all 640×480)
  reproduces the three MAE pairs exactly.
- Isolated mutation: `if (false && …)` around the `main.cpp` inject
  assignment. Default render (no `--no-injected-walkers`) hashes
  `66a8ea3b…` (tube-only / empty-list). Restored `main.cpp` byte-identical
  to the pre-mutation backup; default render hashes `cf0d16fb…`
  (with-injected). 23/23 CTest after restore.
- **Narrowed reference:** `original-0.png` is the A1/A4d camera-fixture
  original, not a pause at the A6a 10 s walk. The A6 gate requires the
  **same actor-list state**. This failure does not prove the compose path is
  wrong; it proves these poses do not match that screenshot. Resume only
  with a matched-state original (or poses taken from that fixture's time).

## Parked on this reference

Using `original-0.png` as the A6 bbox original for the A6a 10 s samples.
Evidence: this note and `bbox-mae.json`.
