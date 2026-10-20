# Phase A6d19: `floor(top)` does not move the matched actor-2 box

Brief `build/tmp/a6d19-first-row-floor-brief.md`. Packet
`build/tmp/a6d19-first-row-floor/`. Cursor `round-agent` on
`composer-2.5-fast`, dispatch `16cf6263-4fa9-4a17-889e-159999d0f150`.
Offline. `ceil` restored.

## Result

- **MEASURED:** `y0 = floor(p.top)` only. A6d3 actor-2 MAE stays
  **0.11246451442529874** (tube **0.10089786756453423**). Actors 0 and 1
  after-tube MAE stay **0.07912965756103010** and **0.08943025021456394**.
- **MEASURED:** matched-camera `inject-floor-top.png` SHA-256
  `15016c20…` equals the A6d17 **ceil** temporal-0 PNG. **0** full-frame
  pixels differ. Worker “frame differs” compared that file to production
  hash `c4e6cf47…`, which is a different render.
- **MEASURED:** the same floor binary **does** move the initial-view
  boxes vs `original-0.png` (with-actors 0.130954→0.133419,
  0.092795→0.094001, 0.097425→0.098351). All three still fail.
- **MEASURED:** `test_actorsprite` fails under floor
  (`integer-centre coverage draws only y=2`; fixture top 1.3).
- **Restored:** `ActorSprite.cpp` SHA-256 `977bda40…` matches the backup.
  `ceil(p.top)` is the first-row bound again.

Prediction that `floor` beats actor 2 is **falsified**.

## Facilitator check

2026-10-20, Cursor facilitator.

- Hash of `src/app/ActorSprite.cpp` equals
  `build/tmp/a6d19-first-row-floor/ActorSprite.cpp.bak`.
- PIL: floor PNG vs `a6d17` `inject-temporal0.png` is identical, including
  the three A6d3 boxes. Actor-2 MAE **0.11246451442529874**.
- Floor log initial-view MAEs differ from the restored ceil run above.
  `test_actorcompose-floor.log` still ends 61/61 with actor-2
  **0.11246**.
- No game. Do not ship `floor`.

## Next

Park first-row `floor`/`ceil`. The matched-camera 1px win is still the
float `originY − 1` fit, not the cover bound. Evidence: this note,
[A6d17](phase-a6-actor2-bbox-pixels.md),
[A6d18](phase-a6-billboard-top-row.md).
