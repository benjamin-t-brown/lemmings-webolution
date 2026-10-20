# Phase A6d7: ActorPose matches live `+0x108` within 1e-4 at the 10 s pause

Brief `build/tmp/a6d7-live-pose-pair-brief.md`. Packet
`build/tmp/a6d7-live-pose-pair/`. Cursor `round-agent` on `composer-2.5-fast`,
dispatch `8f9e64e2-e0ed-43c1-b5f6-3a5e15cd4f94`. One memory-only start (pid 37020).
No `dgVoodoo.conf` write.

## Result

- **MEASURED:** `actorPoseFromNativeSample(+0x08, facing)` vs live `+0x108`
  on three walkers, tolerance 1e-4:

  | actor | +0x08 Y | +0x108 Y | max \|Δ\| |
  | --- | ---: | ---: | ---: |
  | `0x27b3720` | 32.0 | 32.0 | 2.188e-6 |
  | `0x27b4c50` | 32.0 | 32.0 | 6.776e-7 |
  | `0x27b44c0` (slope) | 36.560 | 36.560 | 8.551e-7 |

  Worst over nine components **2.188e-6**. No Y fudge.
- **READ:** this pause's slope `+0x08` is `(32, 36.560, 0.5)`, not the A6d3
  inject `(32, 34.480, 0.5)`. Those are **different 10 s pauses**.
- **Not established:** that the A6d3 bbox miss is an unpaired `+0x08`.
  A6d3 captured screenshot and `+0x08` together. Do not replace that table
  with this pause's `+0x08` without a new original.

## Facilitator check

2026-10-20, Cursor facilitator.

- Independent `test_actorpose` reprint: 2.188e-6 / 6.776e-7 / 8.551e-7.
- Isolated `modelPosition[1] += 1` fails all three A6d7 `model[1]` checks
  (delta 1.0) plus the A6a table. Restored. 23/23 CTest.
- No game; install hashes in the packet match; no `LemWeb-*`.

## Parked

Matched-pause pose pair (round 1; gate met). Actor 2 bbox vs `original-live.png`
is not an `ActorPose` miss. Resume with Z/raster on the **A6d3** pair, not
another pose dump. Evidence: this note.
