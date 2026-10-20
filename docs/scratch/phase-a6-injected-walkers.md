# Phase A6d: three injected walkers after the tube (test path)

Authority: settled A6a–A6c; no new executable reads. Brief
`build/tmp/a6d-injected-walkers-brief.md`. Packet
`build/tmp/a6d-injected-walkers/`. Cursor `round-agent` on `composer-2.5-fast`,
dispatch `4306f1f4-2de3-4bb8-8adf-fb7055ae031f`.

## Result

- **MEASURED (repo):** on `drawCellRoute`, an optional injected list is drawn
  **after** the four A4d passes, high→low, through `actorPoseFromNativeSample`
  → walk temporal 0 → A3 `selectSpriteView` → `drawActorSprite` → RGB replace
  where overlay alpha ≠ 0. No shared depth buffer.
- **MEASURED:** empty list leaves the initial-view encodePng SHA-256
  `66a8ea3b44df2216…` — the same hash A4e recorded for the production route
  frame. The other eight empty-list hashes are pinned in `test_actorcompose`.
- **MEASURED:** on the A1 **initial** camera, each of the three A6a placement
  rectangles differs from the tube-only frame.
- **Unsettled:** `main` still passes an **empty** list, so the running app does
  not show these walkers. Sim → `+0x08`, phase `+0x1D8`, and per-pixel Z vs
  tube are unchanged. Bbox MAE against retained `original-0.png` was not run.

## Facilitator check

2026-10-20, Cursor facilitator.

- Read `drawInjectedWalkers` / `maybeCompose`: after-route unless the
  diagnostic `composeActorsBeforeRoute` flag. Composite is replace-if-alpha,
  not blend.
- Independent production-path mutations (not the diagnostic flags):
  - disable the after-route `maybeCompose` → 3/3 actor-rect checks fail;
  - force `composeSkipActorPose` always on → 3/3 correct-pose rects fail
    (walkers are not in the A6a `+0x108` boxes).
  Restored `TubeScene.cpp` hash-exact `53760420…`. Fresh full build **23/23**.
- The worker's skip-pose *unit* check is inverted vs its prediction (it
  requires pixels in the raw-`+8` boxes). The acceptance rects still fail
  when pose is skipped, so the headline is protected.
- `main.cpp` does pass `&*library` into `render` but never fills
  `injectedWalkers`.
- Offline only; no game process.

## Next

Turn the injected table on in the production `MM_CLIMB_002` route frame and
score actor-bbox error against the retained A4 originals. That is the A6
gate's second clause.
