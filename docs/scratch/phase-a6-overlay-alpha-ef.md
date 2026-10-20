# Phase A6d9: overlay composite uses A4y `> 0xEF`; actor 2 still fails

Brief `build/tmp/a6d9-overlay-alpha-ef-brief.md`. Packet
`build/tmp/a6d9-overlay-alpha-ef/`. Cursor `round-agent` on `composer-2.5-fast`,
dispatch `45b03c44-815a-4c8a-b577-aa6f074be937`. Offline.

## Result

- **READ:** `compositeActorOverlay` now skips texels unless
  `actorSpriteAlphaTestPasses(a)` (`a > kActorAlphaTestReference`, `0xEF`).
  Production default `overlayCompositeLegacyNonZero` is false.
- **MEASURED:** on the A6d3 pair, A4y inject MAE equals the old `a != 0`
  inject. Actor 2 stays **0.11246451442529874** vs tube-only
  **0.10089786756453423**. Actors 0–1 still improve (0.113→0.079, 0.106→0.089).
- **Falsified:** the overlay predicate change makes actor 2 strictly better
  than tube-only.
- Empty-list initial hash still `66a8ea3b…`. Inject hash still `c4e6cf47…`.

## Facilitator check

2026-10-20, Cursor facilitator.

- Re-read `0x00470F39`: `c7 05 e0 24 4c 00 ef 00 00 00` (`mov dword
  [0x4c24e0], 0xef`). Matches A4y and REA/ghidra `read_bytes`.
- Independent Python MAE (`build/tmp/a6d9-overlay-alpha-ef/facilitator/mae_check.py`,
  no worker import) on `original-live.png` vs a fresh
  `webolution --shot` pair at the A6d3 camera:

  | actor | tube | A4y inject | beats |
  | --- | ---: | ---: | --- |
  | 0 | 0.11322272498743087 | 0.07912965756103010 | yes |
  | 1 | 0.10563808014788408 | 0.08943025021456394 | yes |
  | 2 | 0.10089786756453423 | 0.11246451442529874 | no |

  Current inject SHA `c4e6cf47…` equals the pre-A6d9 `with-injected.png`.
- Isolated mutation: production overlay forced to `if (a == 0) continue`.
  Fresh `legacy-inject.png` SHA is the same `c4e6cf47…`; actor 2 MAE stays
  0.11246451442529874. Mutated `test_actorcompose` stayed green (54/54) —
  the walk overlay is already 0/255 from `drawActorSprite` (`out[3] = 255`).
  The synthetic `0xA0` check copies the predicate and does not call
  `compositeActorOverlay`. Restored `TubeScene.cpp` SHA
  `77760fac42473703ffd61fb96b7d4e2a63eb5f52110d91240e02d0f45ed77bd5` (LF).
- Fresh rebuild after touch (Copy-Item had kept the backup mtime).
  **23/23 CTest**. No game processes.

## Parked

Actor-2 overlay predicate on the A6d3 pair. The A4y test is the production
rule even though it is pixel-invisible on this walk path. Resume actor 2
only with a new lead (not another `a != 0` / blend / −0.125 sweep).
Evidence: this note, [A6d8](phase-a6-actor2-z-order.md).
