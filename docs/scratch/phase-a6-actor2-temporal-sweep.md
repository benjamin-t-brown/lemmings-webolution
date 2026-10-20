# Phase A6d4: no walk temporal beats tube-only; phase −0.125 is a pixel fit

Brief `build/tmp/a6d4-actor2-temporal-sweep-brief.md`. Packet
`build/tmp/a6d4-actor2-temporal-sweep/`. Cursor `round-agent` on `composer-2.5-fast`,
dispatch `10c1f137-ca0f-4a9a-89e0-b21f952c383f`. Offline.

## Result

- **Falsified:** walk temporal 0–11 on actor 2, others at 0, none beat tube-only
  MAE **0.10089786756453423**. Best temporal **6** is **0.10369049976893115**.
  Temporal 0 is **0.11246451442529874** (A6d3).
- **MEASURED (pixel fit, not a native read):** `composeActorPhase = -0.125`
  (this is `+0x1D8`; `spriteViewScores` then adds 0.0625) gives actor 2 MAE
  **0.08861160625866508**. Phases 0, ±0.0625, and +0.125 do not beat tube-only.
- **MEASURED:** at −0.125, all three A6d3 boxes beat tube-only
  (0.113→0.076, 0.106→0.095, 0.101→0.089). Empty-list hash still `66a8ea3b…`.
- **Unsettled:** native controller `+0x1D8` at that pause. −0.125 is an offline
  grid fit. Per-walker `walkTemporalIndex` is a sweep hook; production stays 0.

## Facilitator check

2026-10-20, Cursor facilitator.

- Independent webolution shots vs `original-live.png` reproduce actor 2
  −0.125 MAE exactly and confirm all three boxes improve. Tube-only hash
  `66a8ea3b…`.
- Isolated mutation: `composeActorPhase = 0` hashes `c4e6cf47…` (A6d3
  temporal-0 inject) and actor 2 MAE **0.11246451442529874**. Restored −0.125.
- `test_actorcompose` re-ran the sweep (pass). 23/23 CTest after restore.
- `composeActorPhase` is the A3 `+0x1D8` input, not `q`. Do not treat −0.125
  as READ.

## Parked

Native `+0x1D8` at the matched pause. Resume with one live read of that field
on the three walkers. Evidence: this note.
