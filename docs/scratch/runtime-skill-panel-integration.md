# Runtime skill-panel integration

Offline production integration on `decompile`; no original-game launch, physical
input, desktop focus acquisition or installed-data writes. Existing user setup
edits were left untouched. Lower-tier helpers implemented the cached renderer
and state adapter; the main agent wired and independently checked the result.

## Implemented

- Windowed `--sim` caches `RetailSkillPanel` once and draws it before the existing
  runtime counters/caption. Counts come from the same `World` as the actors.
- Keys 1–8 and clicks on the first eight visible plates select the corresponding
  skill and update the selected plate. The physical index mapping reuses
  [C1a](phase-c1-panel-indices.md); no simulation or assignment rule changed.
- Clicks on the two command plates are consumed but do not execute fast-forward
  or Armageddon, or accidentally assign a lemming behind them.
- The existing `--shot --skill-panel` wrapper still uses initial level counts
  and selection 0. Default non-sim composition is unchanged.
- `--frames N --window-shot FILE` records the final presented **CPU frame**.
  With `SDL_VIDEODRIVER=dummy`, it exercises the actual loop without a desktop
  window. It is not a display readback or a native capture.

## Evidence boundaries

This is **SOURCE runtime integration**, not native UI acceptance. The renderer
retains C1's fixed icon poses, measured positions, conditional texture sampling
and unresolved blend assumptions. It does not implement native icon animation,
disabled-state artwork, cursor feedback or sound. Half-open hit regions matching
the visible plates are an explicit app policy, not a new executable read. Empty
skills remain selectable under this policy; `World::assign` rejects unavailable
stock. Nonpositive counts are hidden and values above 99 capped for display;
this does not claim native formatting for synthetic unlimited counts. Slot 8
keeps the fixture's zero count. No Phase D park or native gate was reopened.

## Verification

Commands use `scripts/Invoke-Ucrt64.ps1`. Scratch outputs are under
`build/tmp/enacter-runtime-panel/`; permanent tests live in `src/__test__/`.

- Baseline: **49/49** CTest pass. Restored full build: **51/51** pass, 77.14s,
  including retail-data portions of the new panel tests (`final-tests.log`).
- `skillpanel`: cached resources survive archive teardown; selection/count
  changes stay inside the respective plate/glyph rectangles; wrapper and cached
  default composition agree.
- `runtimeskillpanel`: all eight mappings, rejected indices, hit boundaries and
  gaps, special-command consumption, successful/failed assignment stock, count
  clamping, stable repeated draws, and depletion/selection pixel changes.
- `runtimewindow`: actual SDL dummy loop at frames 1 and 30 (zero and one current
  sim tick) matches the headless sim+panel+HUD composition byte-for-byte. Non-sim
  window/fixture frames also agree. Both routes share rendering primitives:
  this proves wiring agreement, not independent native pixels or physical SDL
  input delivery. It does not validate the parked simulation clock.
- Pre-change and restored default native-size screenshots, with and without
  `--skill-panel`, are byte-identical (`before-*.png` / `after-*.png`).
- Fresh retained-native C1 footprint comparisons, unchanged mask SHA-256
  `e96840e02c3046705c5d1dd19e947a47d7ea4aa65edbe34e31cccafd44b76892`:
  zoom1 **285083/17721990 = 0.0160863988750699**; elev_down
  **285700/17721990 = 0.016121214378294987**. Both remain below 0.02;
  `score_panel.py` / `c1-scores.json` retain the replay. No new C2/C3 acceptance.

Four isolated source mutations were compiled and rejected, then restored:

| Mutation | Discriminating failure |
| --- | --- |
| Omit the main-loop panel draw | `runtimewindow`, tick-0 frame mismatch |
| Render cached initial counts instead of caller counts | `skillpanel` glyph-region check and runtime depletion-pixel check |
| Always draw selection 0 | selection-region and runtime selection-pixel checks |
| Let the two command plates fall through | fast-forward consumption check |

The three mutated production files match their pre-mutation SHA-256 records.
The first focused-test command failed because shell quoting exposed a regex pipe;
separate `-R runtime` / `-R skillpanel` invocations resolved it. This was a test
invocation error, not a production/test failure. Full restored tests are green.

Next integration gap remains menu/confirm/map navigation and supported level
entry. This panel work does not resolve B2/B3's retained evidence gaps, the
native cursor, full-frame rendering failures, or parked simulation fidelity.
