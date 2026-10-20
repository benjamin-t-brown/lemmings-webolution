# Phase A6d6: native view matches `selectSpriteView` at phase 0

Brief `build/tmp/a6d6-live-view-index-brief.md`. Packet
`build/tmp/a6d6-live-view-index/`. Cursor `round-agent` on `composer-2.5-fast`,
dispatch `d5369a1d-9563-4a59-bcc0-34a174ed9f6c`. One memory-only start (pid 61580).
No `dgVoodoo.conf` write.

## Result

- **READ:** three walkers, controller `actor+0x128`:

  | inject | +0x1B8 | +0x1B4 | +0x1CC | +0x1D8 |
  | ---: | ---: | ---: | ---: | ---: |
  | 0 | 1 | 1 | 8 | 0.0 |
  | 1 | 2 | 1 | 8 | 0.0 |
  | 2 (slope) | 2 | 1 | 4 | 0.0 |

- **MEASURED:** phase-0 `selectSpriteView` on the A6d3 inject poses is
  **(1,1), (2,1), (2,1)** — same as native. No view override shipped.
- **Falsified:** actor 2 bbox error is a wrong view index at this pause.
- Native slope temporal **4** was already in the A6d4 sweep (MAE 0.11255, still
  worse than tube-only 0.10090). Actor 2 at temporal 0 remains 0.11246.

This is round 3 of 3 on actor-2 sprite (temporal, `+0x1D8`, view). Parked.

## Facilitator check

2026-10-20, Cursor facilitator.

- `actor+0x128` matches the three reported controllers.
- `inject-replay.json` is (1,1)/(2,1)/(2,1); live views match. Default inject
  does not set `forcedViewIndex`. `composeActorPhase` stays 0.0.
- `test_actorcompose`, `test_actorpose`, `test_spriteview` pass. 23/23 CTest.
- No game; install hashes unchanged; no `LemWeb-*`.

## Parked

Actor-2 sprite selection at the matched pause (temporal, phase, view). Resume
only with a new question (pose `+0x08`/`+0x108` pair, Z, or raster). Evidence:
this note.
