# Phase A6d17: actor-2 box is extra-coverage-free; originY−1 is a pixel fit

Brief `build/tmp/a6d17-actor2-bbox-pixels-brief.md`. Packet
`build/tmp/a6d17-actor2-bbox-pixels/`. Cursor `round-agent` on
`composer-2.5-fast`, dispatch `6bbde40b-82ed-456e-ba70-8b4d6e90eaa2`.
Offline. No production composite change.

## Result

- **MEASURED:** actor-2 rect 315–326 × 283–301 vs
  `original-live.png`: tube-only MAE **0.10089786756453423**,
  temporal-0 inject **0.11246451442529874**. **106** pixels
  `tube ≠ inject`; **92** unchanged. **71** painted pixels worsen L1
  vs original.
- **MEASURED:** **extra_coverage = 0** (`orig == tube` on no painted
  pixel). Original already differs from the tube everywhere we paint.
- **Narrowed:** the worker’s **integer_shift = 94** uses
  `bestNeighborL1 + 6 < alignedL1`. Independent exact RGB match to a
  ±2 neighbor is only **7 / 106**. The 94/12 CTest pins are that
  classifier, not a native field.
- **MEASURED (facilitator, pixel fit):** isolated `originY -= 1.0` in
  `ActorSprite.cpp` makes temporal-0 actor-2 MAE **0.09760348583877995**
  (beats tube). Actors 0 and 1 after-tube also improve
  (0.079→0.065, 0.089→0.080). Restored hash-exact; do **not** ship.

Prediction that tube-retention plus shift would dominate is **half
right**: no tube retention; a 1-row-up screen origin beats the box,
but that is not a cited native store.

## Facilitator check

2026-10-20, Cursor facilitator.

- Independent PIL load of `original-live.png` / `tube-only.png` /
  `inject-temporal0.png` reproduces MAE and painted=106 / extra=0 /
  worsen=71.
- Global painted compare best at **(0,−1)** mean L1 101.5 vs 111.6
  aligned. Exact neighbor matches: 7.
- Isolated `originY − 1.0` (one field): actor-2 temporal-0 **0.09760**
  beats tube; 13 checks fail that pin the old 0.11246. Restored
  `ActorSprite.cpp` hash `977BDA40…`; touched after Copy-Item so ninja
  rebuilt. **61/61** `test_actorcompose`. Full CTest after restore.
- No game. No production hook.

## Next

READ how `0x47A75D` converts billboard top / originY into the first
drawn row. Do not add `−1` to `originY`. A4f already has ULP-exact
float corners on two fixtures — a blanket −1 would fight that.
Evidence: this note, [A4f](phase-a4-actor-billboard.md),
[A4v](phase-a4-actor-support-residual.md).
