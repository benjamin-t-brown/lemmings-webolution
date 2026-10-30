# Current fidelity status

Updated 2026-10-28: [confirmation keyboard focus/No verified](scratch/frontend-confirmation-focus-2026-10-28.md).
**Confirm4 DOWN/UP/DOWN→Return No Menu5→Confirm6 Yes Map7→Menu8** agrees with the
production SDL route; five frames pass0.02. Native startup flags were zero in this
run; nonzero preservation was not exercised. Game closed; exact restoration
independently verified at 23:25:27Z; desktop released. No timing/movie acceptance.
Latest Windows build/retail CTest **70/70 PASS,306.13s**. [Loss/Retry/Quit](scratch/frontend-loss-retry-quit-2026-10-28.md), [No/Resume](scratch/frontend-no-resume-comparison-2026-10-28.md),
[row1 RETURN correction](scratch/frontend-return-negative-prediction-2026-10-28.md),
[two map states](scratch/frontend-native-second-unlock-2026-10-28.md) and earlier
failures remain retained. Not full frontend/B2/game acceptance.
Use [PLAN](../PLAN.md) for sequencing, [lessons](LESSONS.md) for mechanisms and
[acceptance](acceptance/legacy-gates.md) for unchanged signed gates.

## How to read this page

- **Implemented** means a production path exists, not that it matches the original.
- **Own-app tested** means unit/fixture/SDL application checks, not native fidelity.
- **Native verified** always has a stated route, state and coverage limit.
- **Open / failed / parked** remain unfinished. A historical accepted sub-gate is
  not acceptance of an entire subsystem. SOURCE/READ/DERIVED are not live observations.

## Product coverage

| Area | Implemented / retained verification | Remaining completion work |
| --- | --- | --- |
| Assets and levels | Archive/patch overlay, images, sprites, levels/collision, meshes, scene and WAV readers; [format reference](LESSONS.md#the-data-formats--solved). | Maintain retail corpus coverage; distinguish parser success from in-game behavior. |
| Rendering | Software 3D tube, camera, scene/actors and SDL presentation. A1–A7 accepted within amended scopes. [Frame map](RENDER_FRAME_MAP.md). | C3 full-frame FAIL; conditional scene state/admission/clipping, scenery/entrance/exit, actor residuals and complete HUD composition remain. |
| Frontend/map | Opt-in entry, [main-menu focus/Play Current Game](scratch/frontend-menu-focus-navigation-2026-10-28.md), [confirmation focus/Return-No](scratch/frontend-confirmation-focus-2026-10-28.md), [cursor composition](scratch/frontend-cursor-return-coverage.md); [two native map states](scratch/frontend-native-second-unlock-2026-10-28.md), correct102 flags/states and cursor1/10. B3 bounded PASS. | Rows2–4 actions, persistent save loading, mouse row selection/hover, physical focus/input, exact cursor colors and remaining return branches. Not full frontend/B2 acceptance. |
| Connected game flow | [Pending-end state](scratch/level-end-transition.md) and [in-level menu](scratch/in-level-menu.md) integrated. Native [loss7→Retry7→loss9→Quit Map11](scratch/frontend-loss-retry-quit-2026-10-28.md), [No/Resume return](scratch/frontend-no-resume-comparison-2026-10-28.md), abandonment and [win7→Map8](scratch/frontend-native-second-unlock-2026-10-28.md) agree within their bounds. | Native loss Continue/UI5 retention, paused/keyboard Resume, delays/event slicing, results focus/movie behavior, options and complete population mapping remain. |
| HUD / controls | [Runtime skill stock/selection](scratch/runtime-skill-panel-integration.md), [counters/pause](scratch/phase-c2p-runtime-hud.md), [cursor/hover](scratch/runtime-cursor-hover.md). C1 two-view gate passes. | Native C2 acceptance, frontend/gameplay pointer handoff, icon animation, object markers, complete skill eligibility, fast-forward and Armageddon runtime commands. |
| Simulation | World/terrain, movement, skills and researched gadgets exist; [timing retraction](LESSONS.md#the-simulation--measured-and-worth-keeping). | Native clock/release/digger timing; accepted full-level replay; remaining interaction coverage. D1/D3/digger work user-parked. |
| Entry presentation | [Opening camera](scratch/level-entry-camera-intro.md) and [Let's Go cue](scratch/level-entry-start-cue.md) statically recovered and connected for bounded routes. | Other-level metadata, matched native animation timing and stretched-text pixels. |
| Saves / settings | Connected completion is in memory; `src/shell/Progress.*` has a custom `WEBOPROG` helper, not retail-save compatibility. | Persistent session progression, continue/load, settings and restart behavior; decide save-file compatibility before implementation. Never reset the installed save for normal clone play. |
| Audio / movies | WAV reader and `src/audio/Mixer.*` exist; mixer is not in production source inventory. | Sound/music routing, native cue/timing/volume behavior, intro media and FMVs. Old shell existence is not runtime integration. |
| Delivery / platforms | CMake/Ninja UCRT64 Windows build and SDL-dummy tests; [build/run guide](../README.md). | Normal frontend startup is still opt-in; default is a diagnostic actor fixture. Emscripten is not a verified port; no other platform is accepted. |

Source entry: [main.cpp](../src/app/main.cpp);
production wiring: [source inventory](../cmake/webolution_sources.cmake);
test registration: [CMakeLists.txt](../CMakeLists.txt).
Tests that skip retail data cannot establish corpus coverage.

## Retained gate ledger

Scores below retain their linked observation/re-score dates; editing this page is
not a new measurement. Exact gate wording and rules live in [legacy-gates](acceptance/legacy-gates.md). Do not copy
them into another active table. A “pass” here retains every original exclusion.

| Gate | Retained status / scope | Evidence or next missing comparison |
| --- | --- | --- |
| A1 | PASS: four camera views/clamps; 473 control points. | [Phase A history](plan-history/phase-a.md) |
| A2 | PASS: fixed tube ROI, nine views + exposed INN. | Wider scene was not included; [Phase A](plan-history/phase-a.md). |
| A3 | PASS: 608 branch cases, 42/42 actor-bound index/mirror cases. | Selection evidence, not every animation/frame; [Phase A](plan-history/phase-a.md). |
| A4 | PASS: production 3D route and SDL controls, A4d/e. | [Phase A](plan-history/phase-a.md) |
| A5 | PASS: two zoom-0 actor fixtures. | Zoom-1 filter/alpha remains a [residual](plan-history/phase-e.md#actor-filter-and-alpha-at-zoom-1). |
| A6 | PASS: amended repair-4 pause, tube ROI 0.015584142 and three actor boxes. | Not gameplay acceptance; [slope walker](plan-history/phase-e.md#a6d3-slope-walker) unresolved. |
| A7 | PASS: nine first-level views plus one second-level view, amended HUD exclusions. | Full-frame completeness still requires C2/C3; [Phase A](plan-history/phase-a.md). |
| B1 | CONDITIONAL PASS: menu highlight 0, 0.005244. | Panel geometry/colour re-score condition; [B1w](scratch/phase-b1w-menu-render.md), [Phase B](plan-history/phase-b.md). |
| B2 | OPEN; bounded [loss/Retry/Quit](scratch/frontend-loss-retry-quit-2026-10-28.md), [No/mouse Resume](scratch/frontend-no-resume-comparison-2026-10-28.md) and earlier routes PASS. Blanket nonzero-RETURN quirk **contradicted** by row1→Map4/UI3. | [Failed negative prediction](scratch/frontend-return-negative-prediction-2026-10-28.md); signed wording unchanged pending correction. Loss Continue, broader Resume/input and startup progress parity remain open. |
| B3 | Bounded PASS: fresh/completed level-select map full frames0.007834584 /0.007834665; normal first-node selection reaches `MM_CLIMB_002`. Correct102 flags/states, natural completion rather than injected flags. | [Two-state comparison and scope](scratch/frontend-native-second-unlock-2026-10-28.md). Does not establish all map hover/input behavior, exact cursor colors or gameplay timing. |
| C1 | PASS: amended zoom1/elev_down footprint, 0.0160864 / 0.0161212. | Runtime stock connected, native icon/input acceptance separate; [runtime panel](scratch/runtime-skill-panel-integration.md). |
| C2 | OPEN: runtime HUD and bounded cursor adapter own-app tested. | No-exclusion view set and native hover/no-hover/exit; [HUD](scratch/phase-c2p-runtime-hud.md), [cursor](scratch/runtime-cursor-hover.md). |
| C3 | FAIL: 0.06354818984885621 versus 0.02. | [C3bv](scratch/phase-c3bv-production-pack.md); diagnose a shared mechanism, not three-pixel polishing. |
| D1 | FAIL / USER-PARKED: 657 / 305 divergences with clock off/on. | [Timing park](plan-history/phase-e.md#d1-whole-milestone); D1n landed, D1o still fails. |
| D2 | PARTIAL: pick 20/20; 3/4 skill sequences; digger 16/50. | [Digger clock USER-PARKED](plan-history/phase-e.md#d2b-digger-bite-timing); recent app picker work is not a new native pass. |
| D3 | FAIL / USER-PARKED: saved 10/10, end-time spread 75 s. | [Whole-level replay](plan-history/phase-e.md#d3-whole-milestone); correct saved count alone is insufficient. |

## Known debts and how to resume

- Rendering: use [frame map](RENDER_FRAME_MAP.md) and [method](RENDERING_METHOD.md),
  then the relevant [Phase E residual](plan-history/phase-e.md). Scenery, actor
  raster/pose and caption/cursor residuals remain even where amended gates passed.
- Simulation: keep the timing retraction and D1n/D1o distinction. The new roadmap
  identifies the dependency but does not automatically reopen the user parks.
- Retail breadth: retained gadget results include clean negatives, not missing
  gadgets to invent. Older object-overlap coverage and platform/media debts are
  in [other debts](plan-history/phase-e.md#other-debts); inventory reachable routes.
- Observation: retired launcher/controller proposals and component safety proofs
  do not establish an accepted general live-capture route. Reuse the successful
  dated comparisons within their limits; the earlier bounded capture task is fulfilled and
  the desktop released. Any later native task needs a fresh safe,
  exclusive live access before any fresh capture; preserve failed artifacts too.

Maintain one row per gate/area, replacing stale status with a dated evidence link.
Keep this file under **150 lines / 16 KB**. Detailed findings belong elsewhere;
never append a running activity log here.
