# Historical facilitator checkpoints and failure catalogue

Archived from the facilitator contract during framework maintenance. Historical claims,
counts and operational observations below are preserved, not newly verified. This is
reference material, not current instructions or queue state. Current facts live in
[LESSONS.md](LESSONS.md); current ownership lives in [tasks.json](tasks.json).

## The failure catalogue

Agent-side, by how often it actually happened:

1. **Over-generalising a correct measurement.** One model measured, all models claimed. Ask
   "how many distinct cases exist, and how many did they sample?" — usually answerable
   offline in minutes.
2. **Claiming the tests pin a result when they pin part of it.**
3. **A plausible series from the wrong object.** The worst case here: a billboard-quad probe
   was fitted to "a lemming" and produced clean, plausible numbers for **nine rounds** before
   anyone noticed the quad was not a lemming. Two measurements had to be retracted. Read the
   actor list at `0x004CBB18`.
4. **Reading past a section's raw data.** `.data` vsize `0x1185B8`, rawsize `0x7000` — file
   bytes end at `0x004C6000`.
5. **Near-tautological tests**, caught twice.
6. **Wrong shape, not wrong number** — see `docs/LESSONS.md`.
7. **Silent environment failures.** `goto_level` once reported entering one level while the
   loaded path stayed another, so the dump was of the wrong level. Insist briefs say "read
   back the loaded path before trusting a dump".

Facilitator-side — the predecessor's own errors, so you can expect them:

- **Wrote a lessons file from memory instead of from the tree.** It was wrong within twenty
  minutes: rounds 12–31 had run in other sessions and settled every gadget, including eleven
  clean negatives, while the fresh doc listed them as open work. **Read the tree and
  `.claude/rules/` before writing any status claim.**
- **Recommended a round that was already done.** Proposed a "Phase 2 close-out" from a stale
  plan list; all three items were done or residue. **Verify work is still open before
  dispatching it.**
- **Put a wrong formula in a brief** (`floor(worldH − y)`; the engine truncates). The agent
  caught it. Briefs are not authoritative; the exe is.
- **Attributed an observation to the wrong cause.** Supplied a correct grid read with a wrong
  explanation attached. Supply observations; let the agent find the cause.
- **Left stale text in owned files.** The plan said `climbSpeed` is 2 — the retired
  factor-of-2 doubling — long after it was measured as 1. Sweep for contradictions with the
  code periodically.

## Reset baseline and current round

The historical reset baseline was branch `decompile`, commit `562a099`, clean tree,
**14/14 tests**. Do not treat that historical commit or cleanliness as a current status report.

The simulation is **done and correct** — 31 rounds, every `Rules` field a read plus a live
series, all nine skills, every gadget including eleven clean negatives. It is kept.

The reset of 2026-10-18 retired the *visible* half: the flat-first renderer and the
placeholder shell are in `old/src/`. At that historical checkpoint there was no runnable
executable, and asset-side test coverage for tile naming and state→sprite mapping regressed
with three moved tests. A4a restored the headless executable and asset-side tests; the
current A4d/A4e state is recorded below and in `PLAN.md` and `docs/tasks.json`.

**A1 is complete for its tested scope.** The early
[camera checkpoint](scratch/phase-a1-camera.md),
[cell replay](scratch/phase-a1-cell-oracle.md) and
[projection/cache replay](scratch/phase-a1-projection-cache.md) retain real earlier
failures and corrections. The [true live-arena pass](scratch/phase-a1-live-arena.md),
[depth check](scratch/phase-a1-depth-clipping.md),
[control mapping](scratch/phase-a1-control-mapping.md), and final
[zoom/elevation check](scratch/phase-a1-zoom-elevation.md) supersede the old open-A1
handoff. Do not redispatch settled operands, the cell capture, or the shelved
access-watch prototypes. The zoom writer and wrapper-internal Z bits remain
explicit residual unknowns, not grounds to reopen all camera research.

**A2's declared fixed-ROI fixture and scenery inventory are complete.** Read the
[A2 facilitator check](scratch/phase-a2-tube.md) before A3/A4. It corrects the
brief's unflipped tile rows, implements both OUT banks and center filters, and
passes nine retained views plus an exposed inner-wall region. Isolated route
selection and draw-order mutations survive the old ROI; those choices are READ
evidence, not pixel-discriminated parameters. The wider changed-pixel diagnostic
still fails (`0.022030839 > 0.02`), as do cavity and upper-rim regions. Keep these
failures and the omitted-scene inventory for A4; do not call the full scene accepted.

The live suppression probe establishes that the upper gold band persists without
the tube walker; its exact background model/resource remains unassigned. Every
temporary byte and camera write was restored, the owned process stopped, and the
executable/save/wrapper hashes match the starting snapshot. No debugger or hardware
breakpoints were used. This round's actual REA session was closed. The build and
all 14 CTest tests pass. Existing unrelated uncommitted harness/configuration and
A1-note work was preserved; do not infer a clean committed checkpoint from this.

**A3 is complete for the tested selection scope.** Read the
[A3 facilitator check](scratch/phase-a3-sprite-view.md): the old “walk 12 / fall 7
views” assertion reversed the SPR axes. Those are LO temporal counts; walk/fall
have five directions per temporal entry, block/cartwheel eight, shrug one. The
new score/index/mirror helper passes 608 raw native instruction cases and 42 live
actor-bound predictions, with controlled eight-entry graphics and camera sector
crossings. Both stock-raster candidate crops and restores are exact; isolated
index and mirror changes are pixel-visible. The stricter worker sensitivity
`>0.02` diagnostic remains failed. Production sprite rasterization is not accepted.
The original's stored basis and phase inputs must be wired faithfully in A4;
platform sin/cos is not an exact x87 boundary-rounding claim.

The facilitator independently verified eleven isolated source regressions and
exact restores, then rebuilt and passed all 15 CTest tests. Final worker and
facilitator memory writes were restored while suspended, both owned processes
stopped, and executable/save/wrapper hashes match baseline. Earlier failed worker
attempts and invalid derived scores are retained and narrowed in the A3 note.
The worker's actual REA session is closed; no debugger was used in this round.

**Current checkpoint, independently verified 2026-10-19: A4e complete for its bounded
scope.** The user confirmed that the prediction-only predecessor worker was interrupted.
One replacement worker completed the same slot; the facilitator independently verified
and landed the [window/control check](scratch/phase-a4-window-controls.md).
`src/app/main.cpp` now opens the 640x480 SDL window, defaults to the accepted `route`,
and retains headless `--shot`. The fixed 144 Hz input accumulator consumes queued deltas
and key edges once; the controller covers the accepted revolve, zoom and flip mapping.
The title and `--help` describe the core controls. Tile-position and sprite-state tests
are restored. A4 remains open for actor/simulation presentation and the omitted scene.

The facilitator passes 19/19 CTest, 54 retained-stock input comparisons, a native
window/SPACE/WM_CLOSE smoke, and nine independent A4d rerenders with unchanged scores and
PNG hashes. The native initial frame is exactly the explicit route headless frame.
Fourteen isolated controller/timing/event regressions are discriminated; the zoom-exit
one initially survived until the worker strengthened its test. Each source mutation was
restored exactly and the focused test passed. The old default wall independently fails
the pixel gate. No stock process was started; the stock hash is unchanged and no owned
process remains. See the linked note for READ corrections and verification artifacts.

**A4f landed 2026-10-19 as geometry-verified, pixel-negative.** Read the
[A4f check](scratch/phase-a4-actor-billboard.md). Corners match native within one ULP;
seven isolated mutations fail; 20/20 tests; nine tube views unchanged. Pixels fail the
absent-actor gate because the live actor vertex RGB is 152 while `0x440A10` initialises
255; a comparison-only 152/255 scale passes, a U flip does not. Do not hard-code 152.
`ctrl+0x1B0` UV reordering and the unmirrored branch remain READ-only.

**A4g landed narrowed** ([check](scratch/phase-a4-actor-tint-writer.md)): the setter
`0x406260` and a fluid-only fade were found; walk/block 152 is still unexplained. The worker
wrongly said the level file was missing, and the facilitator corrected it.
**A4h landed as a clean negative.** `0x4682a6` copies the actor's controller *into* eight
prototypes; it never writes the actor. Static leads around `0x406260` are exhausted. **A4i/A4j
(GDB hardware watches) named no writer.** A4j showed the watch missing a polled write, so
A4i's "set once" is retracted to inconclusive.
**A4k–A4q solved the tint** (see the tint-writer note). It is a stored per-level grey light
map applied by `0x403820`, and the A4f gate now passes with `LightMap`. Lessons for briefs:
- Frida via REA works where GDB did not.
- Hooks derived from the *first* backtrace can be on the wrong stack (level-load vs walker).
- Worker mutation restores that preserve the old mtime leave ninja objects stale. Rebuild
  after touching the restored files before trusting a CTest result.
This is a historical checkpoint. Read `docs/tasks.json` for the current owner, status and next action.

Foreground acquisition is automated as recorded in the rules. If future probes
use hardware breakpoints, delete them while the original is stopped and verify
cleanup rather than trusting a detach message.

## Two standing cautions

**A stale 385 MB worktree is registered.** `git worktree list` shows
`.claude/worktrees/plan-continue` at `c9e9b1d` on branch `worktree-plan-continue`, left by an
earlier agent. It is gitignored. Removing it would reclaim the space, but it was left alone
because that branch may hold work nobody has reviewed. Do not commit it.

**Preserve quirks.** German menus, RETURN only when the menu highlight is exactly 0, a short
ESC that fails to leave pause, the drawn crosshair disagreeing with the tested cursor. All
real, all in the rules file, and an agent's instinct is to smooth them out. Do not let it.

## Phase A checkpoint log (moved from PLAN.md, 2026-10-20)

PLAN.md's Phase A section carried every A1–A4af checkpoint inline until the 2026-10-20
milestone restructure. Preserved verbatim below (links rebased). Current milestones and
gates live in [the plan](../PLAN.md#phase-a--the-3d-view); established facts in [lessons](LESSONS.md).

- **A1. Camera and projection.** Read the original's field of view, eye distance from the
  axis, eye height, near/far planes, and how it maps mouse and keys to rotation. Deliverable:
  a camera whose frame matches a captured original frame at a known level, with the
  threshold stated.
  **Complete for the tested scope:** the
  [verified camera checkpoint](scratch/phase-a1-camera.md) retains the operand reads,
  standalone pixel fixture, and individual mutation/restore proofs. Initial, mouse, right and
  left static regions now pass the fixed criterion with the live mesh arrays. The
  entry-point debt remains A4.
  The [active draw-path check](scratch/phase-a1-cylinder-draw.md) now corroborates a
  bounded inner-bank submission. The [cell-fixture check](scratch/phase-a1-cell-oracle.md)
  subsequently reproduced the retained failure; its isolated selection mutations
  were unprotected. The [device-emission check](scratch/phase-a1-cell-emission.md)
  now corroborates the selected quad's ordered coordinates and UVs, while its depth
  representation differs from the candidate's. The subsequent
  [projection/cache replay](scratch/phase-a1-projection-cache.md) reproduces the
  retained matrices and selected depth fields under a diagnostic arithmetic model;
  its new operands have not cleared isolated pixel mutations. The subsequent
  [sourced candidate](scratch/phase-a1-sourced-candidate.md) rendered the captured
  radius-60 shell and per-pass depth: it fails `initial` (0.0249) and `left` (0.0304), and
  its isolated bias mutations are pixel-invisible. It also corrects the mesh-quad decode to
  file offset 16, which means the earlier "passing" cell candidate was the smooth cylinder
  alone. The subsequent [diffuse/decode check](scratch/phase-a1-diffuse-decode.md)
  settles the decode from the object headers (the mesh4 windows are one allocation off) and
  applies the exact per-quad diffuse (`0x46D460`, chop-rounded). The best candidate still
  failed before the live capture. The [live-arena check](scratch/phase-a1-live-arena.md)
  then read the true mesh0 arrays and passed all four views: initial `0.015574`, mouse
  `0.016047`, right `0.014810`, left `0.015663`. The fixed pixel gate is met. The
  [depth/clipping check](scratch/phase-a1-depth-clipping.md) then found no numeric
  near/far planes in the executable, no Z/clip render-state writes, and a normalized 0..1
  viewport; the device is the shipped wrapper and depth ordering is effective in the frames.
  The [control mapping](scratch/phase-a1-control-mapping.md) reproduces 473 live points
  with one 2.9e-4 timing outlier and restores the reference camera block byte-for-byte. The
  [zoom/elevation check](scratch/phase-a1-zoom-elevation.md) then passed canonical,
  zoom 1 and both elevation clamps (0.0116–0.0159); the lens is invariant and the eye-vector
  scaling is verified. **A1 is complete for the tested scope.** Residual unknowns: the
  gameplay `control+0x10` writer is unlocated (behaviour measured), and the wrapper's
  internal Z bits are unreadable. A2 and A3 checkpoints follow below; next is A4. The
  entry-point debt remains A4.
- **A2. The tube.** INN/OUT texture addressing, draw order, inner surface before outer so
  the far side shows through gaps, and whatever scenery the original draws that we skip.
  **Complete for the declared fixed-ROI fixture and scenery inventory:** the
  [A2 facilitator check](scratch/phase-a2-tube.md) corrects the texture-row
  flip (192/192 captured cells; the old formula disagrees on 128), implements the
  source-derived OUT bank selection and center filters, and retains INN-before-OUT
  as READ evidence. All nine retained views and an exposed INN region pass; 54/54
  isolated default restores match byte-for-byte. The upper gold band survives live
  suppression of the whole tube walker, establishing a separate scene contribution.
  **No whole-scene acceptance:** the wider changed-pixel diagnostic still fails
  at `0.022030839 > 0.02`; cavity and upper-rim regions fail too. A4 must carry
  those failures and the omitted scenery/object inventory into integration. The
  entry-point and asset-test debts remain A4. The A3 checkpoint follows.
- **A3. Sprite view selection. Complete for the tested selection scope:** the
  [A3 facilitator check](scratch/phase-a3-sprite-view.md) corrects the old SPR
  axis claim: walk's 12 and fall's 7 LO entries are temporal; their directional
  lists have five images. Block/cartwheel have eight directions; shrug has one.
  The score/index/mirror helpers match 608 native branch cases and 42 live
  actor-bound predictions, including controlled eight-entry graphics and camera
  sector crossings. Both shared-stock-raster candidate crops and restores are
  exact; isolated index and mirror mutations change pixels. The stricter >0.02
  sensitivity diagnostic still fails. Replacement rasterization, sprite placement
  and animation integration remain A4; this does not accept a full scene.
- **A4. Entry point and controls.** A new `src/main.cpp` and renderer, **3D by default**
  with the flat unwrapped view as a debug flag — the inverse of the old arrangement. Revolve
  must be a first-class, discoverable control. Pay debt 1 and debt 2 here.
  **A4a checkpoint:** the [first production frame](scratch/phase-a4-renderer-core.md)
  now exists as a headless `webolution` target; 17/17 tests pass and the retired asset-side
  coverage is restored. It loads a retail level and draws the tube from stored data at the
  verified A1 camera, but **0/9 retained views pass** (best `0.037812306`). The stored
  surface pool matches the live bank, yet the per-cell runtime mesh is not the stored
  `PolyList`/`SurfaceMesh` data (8/450 exact index sets, 0/450 full index+UV). Next is the
  producer of the per-cell quads at `0x4ECB00`; the SDL window/event loop and revolve
  controls follow. See the A2/A3 retained failures.
  **A4b checkpoint:** the [producer check](scratch/phase-a4-cell-mesh-source.md)
  pins the level-load chain (`0x43A41A -> 0x461344 -> 0x463070(0x4ECB10)`;
  `0x43A44E -> 0x4608BB`; `0x460A41 -> 0x462264 -> 0x4615E9`;
  `0x46227E -> 0x463410`; `0x4622ED -> 0x464F9C`) and confirms the topology input
  is the stored 256x96 collision grid via `0x40DE60`. The exact scan in
  `FUN_004615E9` and the part table `FUN_00461E1D` remain undecoded, so no production
  rule landed and the A4a `0/9` result stands. Next: decode those two functions and
  validate an offline generator against the retained arena.
  **A4c checkpoint:** the [cell-mesh rule](scratch/phase-a4-cell-mesh-rule.md) is
  decoded and reproduces the retained arena exactly from stored data (`389/389` mesh0,
  `387/387` mesh4 by indices, UVs, colour and order). `LevelIndex::grid0` is the
  `(x, y, layer) -> pool index` table, not a face list; `FUN_004615E9` is the gap pass.
  `src/assets/CellMesh` and `test_cellmesh` land the rule; 18/18 tests pass. Nine retained
  views still **0/9** (best `initial` `0.037620585`, a small improvement). The remaining
  gap is the draw/texture binding: route mesh0/mesh4 through A2's INN/OUT banks, side and
  center filters, and decide where the constant wall remains. The door path is
  implemented but unexercised; only `MM_CLIMB_002` is validated.
  **A4d checkpoint:** the [route check](scratch/phase-a4-cellmesh-route.md) submits
  the decoded cell mesh through the `0x43A4EC` walker's real order — INN, OUT right
  mesh4, OUT left mesh0, OUT centre — and replaces the constant wall. The `HI` tile
  variant is required (plain 0/9, `HI` 9/9). All nine retained views now pass the
  fixed gate (`initial` 0.015584142, worst `left` 0.015679210); the production frame
  matches A2's passing fixture at 2.6e-5 full-frame. Swapping mesh0/mesh4 or disabling
  INN drops to 6/9. The A2 wider/cavity/rim and A3 sensitivity failures remain; the
  centre filter is not discriminated by this ROI. Next: SDL window/controls, then
  simulation + A3 actor rendering, then the omitted scene.
  **A4e checkpoint:** the [window/control check](scratch/phase-a4-window-controls.md)
  adds the 640x480 SDL window, discoverable grab/revolve/zoom/flip controls and a
  fixed 144 Hz input accumulator. Normal launch defaults to the accepted tube route;
  `--shot` remains headless. The facilitator independently passes 19/19 tests,
  54 retained-stock input comparisons, native window/SPACE/close smoke, and all
  nine unchanged A4d scores/PNG hashes. Fourteen isolated controller/timing/event
  regressions are discriminated after strengthening an initially ineffective
  zoom-exit test; the old default wall independently fails the pixel gate.
  Byte reads narrow the zoom setup to entry only and add its 0.1 ratio deadzone.
  No new stock live measurement or full-scene acceptance. Next bounded proposal:
  one actor billboard placement/raster fixture using A3 selection, before whole
  simulation integration; ownership/dependencies/acceptance are in the linked note.
  **A4f checkpoint:** the [actor billboard check](scratch/phase-a4-actor-billboard.md)
  matches native corners within one float ULP for two mirrored fixtures; seven isolated
  placement/UV/alpha/scale mutations fail the focused test; 20/20 tests and nine
  unchanged tube views. Pixels fail the absent-actor gate: the live actor's vertex RGB
  is 152, not the constructor's 255, and its writer is unfound. Next: find that writer
  (static), then rerun the pixel gate.
  **A4g checkpoint:** the [tint-writer check](scratch/phase-a4-actor-tint-writer.md)
  finds the uniform-RGB setter `0x406260` and a fluid-only fade, but nothing yet that
  gives walk/block 152. A4h: the `0x4682a6` link copies the actor into prototypes
  (clean negative). Static leads are exhausted. A4i/A4j GDB hardware watches named no
  writer and were shown to miss a write (see the linked note).
  **A4k–A4q checkpoint (2026-10-19): actor tint solved, and the A4f pixel gate passes.**
  Frida bisects (A4k–A4o) found the writer: `0x403820`, on the lemming's update, copies
  `palette[map[y·128 + (x & 0xFF)/2]]` into all four vertex records. A4p showed the map
  (12288 B) and palette (256 × 3 f64) are stored LVL data after the collision grid. A4q
  (`src/assets/LightMap.{h,cpp}`, MODULATE in `drawActorSprite`) passes both fixtures:
  0.00323 / 0.00362 against absent-actor 0.00619 / 0.00685.
  **A4r checkpoint (static):** the [render-state check](scratch/phase-a4-actor-render-state.md)
  reads RGB `MODULATE(TEXTURE, DIFFUSE)` on all state-writing material paths in
  `0x47261F`, conditional alpha handling and the blend-selector table at `0x414D80`.
  Device3 names are inferred from paired vtable layout and call shape; caches skip
  assignments. The actor's material flags and blend selector are still unresolved,
  so actor-specific alpha/blend behavior and effective device state remain open.
  No production behavior change; independent operand/SDK check and 20/20 CTest pass.
  Next bounded question: establish those actor flags and selector. Other open items:
  an observed colour change across cells, the `_strcmp` guard at `0x42E622`, the
  sprite-pixel residual, and wiring actors into the app renderer.
  **A4s checkpoint (offline):** the [material-binding check](scratch/phase-a4-actor-material-binding.md)
  binds the two retained A4f controllers to the submit objects: selector 0 on
  both, resource handles 119/120 from their direction records. Selector zero
  encodes blend-disable on a cache miss; effective device state is unmeasured.
  Resolved resource `+0x24` flags remain unresolved, not proven runtime-only.
  Independent fixture joins, original-byte rereads and 20/20 CTest pass; no
  production behavior change. Next bounded proposal: correlate these actor
  submit objects/handles with resolved descriptor flags in a read-only capture
  (or continue static producer provenance). Raster changes remain gated on that
  evidence; other retained failures and actor integration remain open.
  **A4t checkpoint (live, read-only):** the [material-flags check](scratch/phase-a4-actor-material-flags.md)
  measures resolved descriptor `+0x24` = `0x2` on both lemming sheets
  (`$LemSheetHi15`, `$LemSheetHi12`) for five walkers and a controlled eight.
  That is A4r row 3: texture-alpha select, alpha test GREATER with reference
  `0xA0`, selector 0 (blend disabled). Encoded path only; effective device state
  and caches at draw time are unobserved. Independent raw-byte recompute,
  rereads, 20/20 CTest and a clean live side pass; no production change.
  Next bounded question: implement that alpha test (`> 0xA0`, no blend) in
  `drawActorSprite` behind a test that fails without it, and rerun the A4f gate
  to see whether the sprite residual moves.
  **A4u checkpoint (offline):** the [alpha-test check](scratch/phase-a4-actor-alpha-test.md)
  implements that rule (`> 0xA0`, opaque replace, `--no-alpha-test` diagnostic).
  It is pixel-invisible on both fixtures: the frames' alpha is strictly 0/255, so MAE
  and composed hashes equal A4q's and the ~0.08 support residual is unmoved. The facilitator
  passes 20/20 CTest, an own gate rerun and two isolated mutations. The five fixture's
  sheet (`$LEMSHEETHI13`) has unmeasured flags. Next bounded question: the support
  residual's source (sampling/filtering) in an offline crop diff, before wiring actors
  into the app renderer.
  **A4v checkpoint (offline):** the [support-residual check](scratch/phase-a4-actor-support-residual.md)
  finds a half-pixel convention: integer pixel centre for coverage and UV, nearest
  texel. Native and candidate footprints now match exactly; support MAE falls to 0.00143 /
  0.00152. It is a measured fit on two fixtures. `0x470F29`'s filter/alpha-reference
  pairing (`0xEF` point, `0xA0` linear) against A4t's live `0xA0` leaves the filter in
  force untraced. The facilitator passes a reproduced decomposition, an own gate rerun, a
  repaired V-axis test, isolated mutations and 20/20 CTest. Open: which `0x470F29`
  branch (or dgVoodoo override) is in force at actor draw, and a third fixture for the
  centre rule.
  **A4w checkpoint (live, read-only):** the [Hi13 flags check](scratch/phase-a4-actor-hi13-flags.md)
  measures `$LemSheetHi13` `+0x24` = `0x2`, and all 24 lemming sheets in the pool read
  `0x2` on `MM_CLIMB_002`. A4u's row 3 is measured for both fixtures' sheets. Independent
  raw-byte recompute, unchanged install hashes and a clean live side pass.
  **A4x checkpoint (static + live, read-only):** the [filter-state check](scratch/phase-a4-actor-filter-state.md)
  reads a bracket around the actor draw: `0x470F29(0)` before, `0x470F29(1)` after. Actors
  draw under POINT filtering with alpha reference `0xEF`; the sampled `0xA0` is the
  between-draw value; dgVoodoo filtering is `appdriven`. This agrees with A4v's
  nearest-texel fit. One indirect vtable hop (`[actor+0x340]`) is consistent but not
  reread. Next bounded question: change the actor alpha reference to `0xEF` behind a test
  that fails without it (pixel-invisible on the fixtures, so the unit test carries it),
   then wire actors into the app renderer.
   **A4y checkpoint (offline):** the [alpha-reference check](scratch/phase-a4-actor-alpha-reference.md)
   changes the reference to `0xEF` and retains strict `>`. Both the old-value and
   inclusive-compare mutations fail on destination pixels, not a constant assertion.
   The facilitator's separate exhaustive checker passes all 256 alpha values;
   20/20 CTest and both A4f gate results/image hashes are unchanged from A4v.
   Binary-alpha fixtures cannot discriminate this correction; synthetic raster tests
   carry it. No new live/effective-device claim or full-scene acceptance. Next queued
   action: A5m's read-only actor integration map. A4z's third-fixture dependency on
   A4y is satisfied; wiring actors into the app and retained A2/A3 failures remain open.
  **A5m checkpoint (source map only):** the [integration map](scratch/phase-a5m-actor-integration-map.md)
  confirms the app still draws only the tube. Existing state-name, A3 selection,
  placement, tint and actor raster helpers are not connected to World. Missing
  links include terrain-to-cylinder world pose, orientation basis/phase/prior mirror,
  a render-animation clock (Lemming::frame has state-dependent use), and shared
  actor/tube depth and composition. No adapter implementation or new original-game
  claim is accepted. Next queued action remains A4z; adapt its old SSH-specific
  brief for the dispatching environment first.
  **A4z checkpoint (bounded negative):** the [third-fixture check](scratch/phase-a4-actor-third-fixture.md)
  adds a passing controlled half-scale sample, but the matched-camera full-scale
  zoom1 sample fails: 27 candidate-only pixels, support MAE 0.0284529 against
  0.0151558. Native size proves 0.5 versus 1.0 scale; replay corners differ by at
  most two float ULPs. Byte reread narrows A4x: nonzero zoom skips the pre-draw
  POINT/EF setter, leaving inherited zoom1 state open. No raster change, 20/20
  CTest; clean process/install checks, but enter_fast lacks raw level-slot
  restoration bytes. Next: static zoom1 filter/alpha provenance, not an
  evidence-free raster patch. A5m's integration prerequisites remain open.
  **A4aa checkpoint (bounded static):** the [zoom1 provenance check](scratch/phase-a4-zoom1-filter-provenance.md)
  verifies frame/tube calls and their unresolved indirect boundaries; no direct
  filter-setter call appears in the inspected frame/tube bodies, but callees are
  not cleared. Last pre-actor filter/device alpha reference remains unestablished.
  Setter local defaults are chosen before device calls; the material caches can
  skip ALPHAREF requests independently of the global. Heuristic whole-graph
  absence claims were rejected. No production change; 20/20 CTest. Next bounded
  action: actor-correlated live observation of zoom1 filter requests, not a
  between-draw read or a pixel-fitted fix.
  **A4ab checkpoint (failed live attempt):** the [request-trace check](scratch/phase-a4-zoom1-filter-trace-failed.md)
  records an owned stock crash in D3DImm after the five-hook load interval,
  with zero actor-bound draw windows/request tuples retained. Specific hook
  causation is unisolated; the filter question remains unanswered. Independent
  verification reproduces a dormant Number.add binder defect; entry-slot bytes
  and six install hashes are restored, but live hook-unload/code readback was
  not achieved. No retry, no renderer change; 20/20 CTest. Next is offline binder
  regression repair and instrumentation-design validation before another live
  attempt, not a blind reduced-hook retry.
  **A4ac checkpoint (offline):** the [actor-binder check](scratch/phase-a4-actor-binder-offline.md)
  repairs and regression-tests the scratch actor binder under `node:vm` without
  touching `hooks_a4ab.js`: numeric `u32` array addresses are converted via
  `ptr(arrAddr)` before indexed slot reads, matched `snapshotActors` returns
  `{ actors, match }` so `DRAW` does not treat a match as `BIND_MISS`,
  `selection.ctrl` uses pointer `.toString()` instead of `hexN(ptr(...))` (`'0x0'`),
  and binding accepts only `actor+0x128` rather than the unmeasured
  `action_obj+0xb0` alias. Four isolated single-line mutations fail the test
  suite and restore byte-for-byte; 20/20 CTest pass.
  **A4ad checkpoint (offline):** the [hook-announce/stack check](scratch/phase-a4-hook-stack-announce-offline.md)
  repairs and regression-tests the remaining non-binder defects in
  `hooks_a4ad_candidate.js` (`1445dad5...`) while preserving `hooks_a4ab.js` and
  `hooks_a4ac_candidate.js` byte-identical: `HOOK_INSTALL` is emitted strictly
  after `Interceptor.attach` returns (with `HOOK_ATTACH_FAIL` on throw), and
  mid-call `0x470F86`/`0x470FA3` records `retaddr: null`,
  `stack_convention: 'pre_call_stdcall_4'`, `device` (`[esp+0]`) and
  `(stage, state, value)` (`[esp+4..+12]`) while preserving function-entry
  `retaddr` at `0x470F29`. Four isolated single-feature mutations fail the
  `node:vm` suite and restore byte-for-byte; 134/134 PE bytes at
  `0x470F29..0x470FAE` and 20/20 CTest pass.
  **A4ae checkpoint (static):** the [pre-actor subcalls check](scratch/phase-a4-pre-actor-subcalls-static.md)
  audits `0x437BA3` (`0x441879`) and the `0x43A90D..0x43AA63` (`0x4C6A10+0x1E88`)
  downward loop (`0x43A9BB`/`0x43AA47`) in `0x43A4EC` against `Lemmings Revolution.exe`.
  `0x437BA3` (`-> 0x47496D -> 0x45FDB0/0x43BBE0`) has zero direct `0x470F29` calls
  and zero `0x4C24E0` references across its depth-4 `E8` tree (only indirect
  `0x4749A7: call [edx+4]`). Crucially, auditing all 5 `.text` call sites of
  `0x43B517` (`0x42A241`, `0x42E153`, `0x42FCD1`, `0x43212A`, `0x43231D`) proves
  that **lemmings (`vtable 0x4B10E8`, `+0x10 = 0x42EEF2`) are pushed into
  `0x4C6A10+0x1E88` (`0x4C8898`) and drawn INSIDE `0x43A4EC` at `0x43A9BB`/`0x43AA47`**
  (whereas `0x4C6A08+0x1F70` at `0x44190D` holds `0x4B1020` objects from `0x448BD0`).
  **A4af checkpoint (static):** the [tube child renderers & filter check](scratch/phase-a4-tube-child-renderers-filter-static.md)
  audits `0x42C5C9`, `0x4323EB`, `0x42EEF2`, `0x47A75D`, and all `.text` occurrences of
  `0x470F29`, `0x4C24E0`, and `call [reg+0xA0]` (`SetTextureStageState`). `0x42EEF2`
  (`Lemming::Render`) dispatches `lemming+0x78` (`vtable 0x4B14A8+4 = 0x43CC68`) at
  `0x42F0DC`, which adds `0xB0` (`ecx = lemming+0x128`) and calls `0x47A75D` at
  `0x43CD4E` (one of only two `.text` callers of `0x47A75D`, the other being HUD loop
  `0x45B784`); `0x42C5C9` tail-calls `0x4323EB` (`0x42C709`), and neither calls
  `0x470F29`. Across the entire `.text` section, `0x470F29` is the **only** function
  that requests stage-0 `MAGFILTER`/`MINFILTER` (`0x10`/`0x11`, out of 24 `+0xA0` sites)
  or writes `[0x4C24E0]` (out of 3 `0x4C24E0` sites), has 0 pointer references in the PE,
  and has 10 `E8` call sites: 2 init/option sites (`0x47022B`, `0x470F1E`) and 4
  `(entry, exit)` brackets (`0x44E165..0x44E1A5`, `0x454C89..0x454CAF`,
  `0x45B6D9..0x45B706`, `0x47A776..0x47AB8B`, all `ecx = 0x58CD80`), every one of which
  restores `0x470F29(1)` on exit (`0x47AB8B` is unguarded on the single `ret 0x10` path
  `0x47AB93` of `0x47A75D`). Thus at nonzero zoom (`[0x58D53C] != 0`), `0x47A75D` skips
  `0x47A776: call 0x470f29(0)` and runs under the preceding `0x470F29(1)` state
  (`MAGFILTER=2, MINFILTER=2`, `[0x4C24E0]=0xA0` when `[0x58CD80+0x3A1] != 0`). Next
  bounded offline step (`A4ag`): replay the retained A4z full-scale (`zoom1`) fixture
  under `{POINT, LINEAR} x {0xEF, 0xA0}` to test whether `0x470F29(1)` (`LINEAR` + `> 0xA0`)
  accounts for the 27 extra candidate pixels and `0.02845` support MAE.
