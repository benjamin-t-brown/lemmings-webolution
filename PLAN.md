# Lemmings Webolution — Plan

Reset 2026-10-18. The previous plan and all 31 research rounds are preserved under `old/`.

**Read `docs/LESSONS.md` first.** It carries every fact worth keeping from the old plan, so
nothing below needs to re-explain the formats, the measured constants or the traps.

**This is the live plan** (slimmed 2026-10-24, user, overlord review 2026-10-24): goals,
milestone tables with their signed-off exit gates verbatim, a short status per milestone,
and the user's decisions. Round-by-round prose lives in `docs/plan-history/` —
[A](docs/plan-history/phase-a.md), [B](docs/plan-history/phase-b.md),
[C](docs/plan-history/phase-c.md), [D](docs/plan-history/phase-d.md),
[E](docs/plan-history/phase-e.md) — moved there verbatim under the same headings. Open it
only when a decision needs the detail. A status cell is one or two lines: score, blocker
and a link to the latest note.

## The goal, restated

**Direction (user, overlord review 2026-10-27): one faithful, integrated retail-level
experience.** Convert accepted research into production rendering, controls and HUD;
prefer observable integration over standalone tooling. Approved campaign: C3 → C2 → B2 →
B3, with convergence fallback and budgets in [the directive](docs/DIRECTIVE.md).

The reset's historical *simulation is correct* claim described 31 rounds of retained
rules/gadget work, not current end-to-end acceptance. [D1m's later timing retraction](docs/LESSONS.md#the-simulation--measured-and-worth-keeping)
withdraws discrete 5 Hz stepping; D1/D3 and D2's digger clock remain user-parked. Keep
that work, but do not call simulation fidelity complete. The reset replaced a flat debug
view, fixed sprite direction and placeholder menu with a 3D-first production path.

**The new goal is visible and tactile fidelity: a screenshot of ours should be hard to tell
from a screenshot of the original, and it should play in 3D on the rotating tube.**

That is a falsifiable goal, which is why it is phrased that way.

## Method change: read the code, diff the pixels

Phase 3 could measure behaviour by watching memory while the game ran. Rendering mostly
cannot — you cannot measure a projection matrix by watching a lemming walk. So:

- **Static reading is now primary.** [REA](https://github.com/morluto/rea) is installed and
  verified against the stock exe; see `docs/REA_SETUP.md`. Use it to read the render setup,
  the sprite view selection, the frontend state machine and the HUD layout.
- **The oracle becomes the screenshot.** `tools/harness/screengrab.py` captures the original
  through a desktop BitBlt. The new comparison is *our frame against its frame at the same
  level and camera*. An image diff will never be pixel-exact through a D3D11 wrapper, so the
  first round that uses it must **set the "close enough" threshold explicitly** rather than
  discover it later.
- **The old standard still holds.** Cite the address and the bytes. A decompiler line is not
  a measurement. Where a thing is READ and not MEASURED, label it so.

## Where the tree stands after the reset

The 2026-10-18 reset inventory and the two debts it created (both paid in A4) are in
[the Phase A history](docs/plan-history/phase-a.md#where-the-tree-stands-after-the-reset).

## Phase A — the 3D view

Restructured 2026-10-20 into milestones with **fixed exit gates**. A milestone is done when
its gate passes, not when its questions run out. Residuals that do not move the gate are
parked in Phase E with their evidence, not chased. The A1–A4af checkpoint prose is in
[the history log](docs/FACILITATOR_HISTORY.md#phase-a-checkpoint-log-moved-from-planmd-2026-10-20);
the settled facts are in `docs/LESSONS.md`.

| Milestone | Status | Exit gate |
| --- | --- | --- |
| A1 camera and controls | **Implemented:** camera/input. **Verified (scope):** done, retained views/clamps. **Next deliverable:** preserve. | Four static views and zoom/elevation clamps pass the fixed 0.02 metric (0.0116–0.0160); 473 control points |
| A2 tube fixed ROI | **Implemented:** tube route. **Verified (scope):** done, ROI only. **Next deliverable:** preserve. | Nine retained views + exposed INN region pass; wider/cavity/rim moved to A7 |
| A3 sprite view selection | **Implemented:** chooser. **Verified (scope):** done, index/mirror cases. **Next deliverable:** preserve. | 608 native branch cases, 42/42 live actor-bound index/mirror |
| A4 entry point, tube route, window | **Implemented:** SDL window/controls and headless path. **Verified (scope):** done, A4d/e. **Next deliverable:** preserve. | `webolution` runs 3D by default; 9/9 views on the production route (A4d); SDL controls (A4e) |
| A5 one actor billboard raster | **Implemented:** actor raster. **Verified (scope):** done, zoom 0 fixtures. **Next deliverable:** preserve; zoom 1 parked. | Both A4f fixtures beat absent-actor; footprint exact (A4v); POINT + `> 0xEF` (A4x/A4y). Zoom 1 residual parked (E) |
| A6 sim lemmings in the app frame | **Implemented:** actor adapter/optional sim. **Verified (scope):** done, amended repair4 frame only. **Next deliverable:** preserve; D gates separate. | Restated gate on the repair 4 pause: tube ROI 0.015584142; all three actor boxes improve. A6d3 slope walker parked (E) |
| **A7 scene completeness** | **Implemented:** tube/actor composition. **Verified (scope):** done, amended exclusions/two levels. **Next deliverable:** preserve. | Amended gate passes: nine `MM_CLIMB_002` views (A7p) + one second level (A7q), caption panel + ten skill-panel boxes excluded, all ≤0.02. See below |

**User decisions, Phase A.** Full text in [Phase A history](docs/plan-history/phase-a.md).
- **A6 exit gate restated (2026-10-20, with user sign-off, after A6d24):** tube ROI
  `[100,80,280,410]` ≤ 0.02 actors-on, and each actor box better with actors than without; the
  full frame is reported, not gated, and becomes the A7 + Phase C gate
  ([text](docs/plan-history/phase-a.md#a6-sim-lemmings-in-the-app-frame)).
- **A6 accepted (2026-10-20, user decision)** on the repair 4 frame, with the A6d3 capture's
  limits; the A6d3 slope walker is parked in Phase E.
- **A7 exit gate restated (2026-10-21, with user sign-off, after A7l)** to exclude the pause
  caption `[277,422,363,454)`, and **amended (2026-10-21, user sign-off, after A7p)** to
  exclude the ten skill-panel boxes, x `[18,90)` ([text](docs/plan-history/phase-a.md#a7-scene-completeness)).

## Phase B — the frontend

The original's own screens: backgrounds, buttons, fonts, `.LTX` localised text (English in the
reference install), the screen state machine, level select and the map. Preserve the quirks —
**RETURN only when the menu highlight is exactly 0**. (The "short ESC that fails to leave
pause" quirk was retracted 2026-10-23: ESC at 2/5/10/25/40 frames all left pause, B2h.)
Frontend screen ids read as `0x004C416C`: 3 menu, 4 confirm, 5 map — a monotonic
object-creation counter, not a chosen enum (B1 recon; detail in
[Phase B history](docs/plan-history/phase-b.md#phase-b--the-frontend)).

**Milestones and exit gates, drafted 2026-10-21 (CEO review), signed off as drafted by the
user 2026-10-22.** The metric is the A4d fixed changed-pixel score, scored with
`tools/harness/score_a4_render.py` methods on a full 640x480 frame unless stated. The
threshold is 0.02, the same as A1/A2/A7.

| Milestone | Status | Exit gate |
| --- | --- | --- |
| B1 frontend assets and text | **Implemented:** menu renderer, not runtime frontend. **Verified (scope):** conditional acceptance, [B1w](docs/scratch/phase-b1w-menu-render.md) .005244; panels measured. **Next deliverable:** read/re-score panels; failure reopens B1. | Background, button and font sprites and the `.LTX` strings for the reference language (English) load. One reference screen (main menu, highlight 0) renders full-frame ≤ 0.02 against a stock screengrab |
| B2 screen state machine | **Implemented:** not wired into app. **Verified (scope):** partial reads/live pause, not gate. **Next deliverable:** reproducible menu/confirm/map/level/back; [prior cap-parks](docs/plan-history/phase-e.md#b2-screen-state-machine) retained. | Scripted key sequences (menu, confirm, map, into a level, back) visit the same screen ids (`0x004C416C`) in the same order as the original. Quirk preserved: RETURN only at highlight 0. (Short-ESC clause dropped, user, overlord review 2026-10-23: retracted after B2h.) |
| B3 level select and map | **Implemented:** progress/assets, no integrated map. **Verified (scope):** gate unscored; [object/pick parks](docs/plan-history/phase-e.md#b3-map-screen-object). **Next deliverable:** map selection and two unlock-state comparisons. | Map screen and level-select full-frame ≤ 0.02 on two unlock states. Selecting `MM_CLIMB_002` from the map reaches the D3 level |

**User decisions, Phase B.** Full text in [Phase B history](docs/plan-history/phase-b.md).
- **English reference accepted (user, 2026-10-22):** the German-menu quirk was a harness
  language change; leave the install's language unchanged.
- **Approach steering (user, CEO review 2026-10-22):** measure the menu image by template
  matching retail sprites and glyphs, render that layout and score the unchanged 0.02 gate;
  read code only for what the image cannot show.
- **B1 accepted on condition (user, overlord review 2026-10-23)**, as in the B1 row.
- **B2 short-ESC clause dropped (user, overlord review 2026-10-23)**: retracted after B2h
  ([note](docs/scratch/phase-b2h-live-esc-sweep.md)).

## Phase C — the in-level HUD

The skill panel art and counts, and the counters the engine names itself: `^saved`, `^out`,
`^time`, the icon sets `GreyIcons` / `BlackIcons` / `AdditiveIcons`, and the cursor set
(`_curLem`, `_curNoLem`, `_curEnt`, `_curExit`, and the left/right variants). C1a identifies
all physical panel indices: 0 climber, 1 floater, 2 bomber, 3 blocker, 4 builder,
5 basher, 6 miner, 7 digger, 8 fast-forward, 9 Armageddon (READ, not new live assignments).

The ten skill-panel buttons (x `[18,90)`, objects at `0x4C9208`) and the **Paused** caption
panel (`[277,422,363,454)`) are excluded from the A7 gate and drawn here. The A7c–A7l caption
trace and the C2a/C2b counter reads are in
[Phase C history](docs/plan-history/phase-c.md#phase-c--the-in-level-hud).

**Milestones and exit gates, drafted 2026-10-21 (CEO review), signed off as drafted by the
user 2026-10-22.**

| Milestone | Status | Exit gate |
| --- | --- | --- |
| C1 skill panel | **Implemented:** optional headless panel. **Verified (scope):** amended gate PASS .0160864/.0161212 ([closure](docs/scratch/phase-c1-close-amended-gate.md)); other views fail. **Next deliverable:** runtime composition, preserve gates. | Panel indices 2, 5, 6, 8, 9 identified (jump table `0x0045A4DD`). The ten buttons and their counts render inside `x [18,90)` ≤ 0.02 against the A7p/A7q captures, with the A7 panel exclusion removed. **Measured over the panel footprint** (user, overlord review 2026-10-24): the union of the ten button plates, icon rectangles and count-glyph rectangles at their read positions, fixed and committed before any render it scores. Not the C1b `baseline != no-cell-routes` mask, which meets the panel in 0 pixels (C1t). **Gated views** (user, overlord review 2026-10-24, second amendment; third amendment 2026-10-25): A7p `zoom1` and `elev_down`, each ≤ 0.02, rendered with their A7p camera params; `elev_up`, `initial`, the other A7p views and `MM_DIG_000` are scored and reported, not gated, until C3 draws the backdrop behind the panel |
| C2 counters, cursor, pause caption | **Implemented:** [runtime World HUD/P pause](docs/scratch/phase-c2p-runtime-hud.md), cursor policy only. **Verified (scope):** own SDL events/state/pixels; native C2 gate unpassed, [cursor live arm parked](docs/scratch/phase-c2q-native-cursor.md). **Next deliverable:** qualified B2/B3 navigation. | `^saved`/`^out`/`^time` and the Paused caption `[277,422,363,454)` drawn. The A7 nine-view + second-level score passes with **no** HUD exclusions. Cursor sprite set matches on hover/no-hover/exit |
| C3 full frame | **Implemented:** [native-domain final pack connected](docs/scratch/phase-c3bv-production-pack.md), conditional SOURCE path. **Verified (scope):** FAIL .06354819 (+3 error); C1 PASS. **Next deliverable:** C2/B2/B3 integration; retain C3 gaps, no pixel polishing. | The A6d24 full-frame score (0.2123 today) ≤ 0.02 on the A6 repair 4 pause with actors, the floor ring included |

**User decisions, Phase C.** Full text in [Phase C history](docs/plan-history/phase-c.md).
- **Integrated retail progress (user, overlord review 2026-10-27, "go ahead"):** C3 → C2 → B2 → B3; 8h/16 attempts/8 starts, tooling ≤2 attempts/90min; C3 reassess at 3 attempts/2h. [Directive](docs/DIRECTIVE.md); no Phase D or gate reopening.
- **Backdrop before C1** (user, overlord review 2026-10-23): backdrop scenery, entrance and
  exit models come before C1. Since parked in Phase E (C3 scenery, 2026-10-24).
- **Standing runs and C1 alpha-count extensions (user, overlord review 2026-10-24/25):** milestone budgets, launch/menu-attach changes, failed-start renewals and convergence decisions retained in the [authorization history](docs/plan-history/phase-c-authorizations-2026-10-27.md); no old pool renewed here.
- **Long multi-milestone / C1 DERIVED-value runs (user, overlord review 2026-10-25):** retained [in full](docs/plan-history/phase-c-authorizations-2026-10-27.md). DERIVED requires a supported chain; disagreeing live reads replace it.
- **C1 third gate amendment and closure → C3 (user, overlord review 2026-10-25):** `elev_up` reported, not gated; zoom1/elev_down remain ≤0.02. [Decision/evidence](docs/plan-history/phase-c-authorizations-2026-10-27.md); footprint/threshold unchanged.
- **C3 render-now and known-gap steering (user, overlord review 2026-10-25):** conditional rendering before more reads; then isolated supported fixes and bounded observation. [Full historical budgets/conditions](docs/plan-history/phase-c-authorizations-2026-10-27.md), not current controller authority.
- **C3 reference re-capture, scene pose and tube ends (user, overlord review 2026-10-25,
  "approved").** Run `c3-known-gaps-pause-values` stopped 16/16 at 0.0635482 with 0 starts:
  the one-shot was rejected before Popen on a numeric ScheduledTask state. The user saw the
  original's fourth walker and out-count 4 missing from ours (5), a possibly mis-posed scene,
  and missing cylinder ends. The overlord confirmed the walker cause: `capture_matched_pause.py`
  persists only `actors[:3]`. Next Sol run: host-test the controller, then spend one start to
  read the pause values and re-capture repair4 with the cursor clear of the hatch, the full
  actor list and HUD counters. **Reference swap authorized** if the new capture matches repair4
  outside the cursor/hatch, fourth-walker and counter boxes; threshold, metric, partition and
  floor rule unchanged; repair4 reported alongside. Then the scene pose, the tube ends, and the
  rest of the known gaps. About 6 hours or 16 rounds, pool of 3 starts.
  Re-ranked the same day (user, "get the big rocks better before refining"; "approved"): edge
  scenery (57% of C3ao's error) and the orange discs ahead of the tube ends, run offline beside
  the live arm; the scene-pose check tests handedness (user lead: mirrored geometry/textures).
- **C1 gate measurement amended (user, overlord review 2026-10-24).** C1t showed the scorer could not see the panel: its mask inside the skill bands is x 86..89, the drawn icons x 26..76, overlap 0, so `skill_mae` stayed at the pre-panel 0.1077 / 0.0554. The threshold (0.02), region and captures are unchanged; the pixels scored become the panel's own footprint, built from read geometry (plates, c1s icon positions with the c1p/c1r frame sizes, count glyphs at the c1s text-object positions) and committed before the render. Pixels where a plate is translucent still include whatever the stock frame shows behind it; that share is reported, not masked out.
- **C1 gated views amended (user, overlord review 2026-10-24, second amendment).** The plates are translucent: all 18033 plate-only footprint pixels differ between the eight A7p stock views, and they carry 83% of c1w's 0.2918. On the 4536 scenery-only pixels between the plates, our render differs from the stock frame by 0.48–0.52 on `initial`, `left`, `right`, `elev0_neg` and `canonical` (sand, sky, balloon: parked C3 scenery) and by 0.0183 / 0.0209 / 0.0174 on `zoom1` / `elev_up` / `elev_down` (the tube wall we draw). So the gate is taken on those three views; the threshold, footprint and region are unchanged. The scenery error alone is near 0.02 there, so a pass is not assured. Overlord scripts `build/tmp/overlord-2026-10-24/split.py`, `constancy.py`, `gaps.py`, `views.py` (measured from the saved A7p PNGs; evaluation, not a new capture).

## Phase D — reconnect the simulation

Spawn, exits, skill assignment and gadgets are connected to the optional sim path, but
retail end-to-end fidelity is not accepted. User-directed D1/D3/digger-clock parks remain;
the current C3 → C2 → B2 → B3 campaign does not reopen them.

**Milestones and exit gates (user sign-off 2026-10-21, before any D round).** Every gate is
scored against the original's actor list at `0x004CBB18`
(`measure_steppers.py --compare` is the per-tick oracle), never against our own sim. Do not
change a gate after seeing a result; a needed change stops the standing run.

| Milestone | Status | Exit gate |
| --- | --- | --- |
| D1 release and walk-out | **Implemented:** spawn/walk/optional clock. **Verified (scope):** NOT accepted, 657/305 divergences clock off/on. **Next deliverable:** strategic decision; [user-parked](docs/plan-history/phase-e.md#d1-whole-milestone). | `MM_CLIMB_002`, no input, from level start: for 150 ticks (30 s), every lemming's cell X/Y, facing and state match the original each tick, and the released count and order match |
| D2 click-to-assign | **Implemented:** pick/assignment. **Verified (scope):** (a) 20/20; (b) 3/4 skills, digger 16/50. **Next deliverable:** strategic decision; [digger parked](docs/plan-history/phase-e.md#d2b-digger-bite-timing). | (a) For 20 recorded cursor positions, the in-level pick (AABB against `lemming+0x128`, cursor doubles at `0x4CBFB0`+0x16A0/+0x16A8) picks the same lemming as the original, or none when it picks none. (b) Climber, floater, blocker and digger each assigned once: the 50 ticks after each assignment match the actor list exactly, and the skill counts decrement the same way |
| D3 one retail level end to end | **Implemented:** optional sim/play path. **Verified (scope):** NOT accepted; saved10/10, end-time spread75s. **Next deliverable:** strategic decision; [user-parked](docs/plan-history/phase-e.md#d3-whole-milestone). | `MM_CLIMB_002` playable in `webolution` (3D view, lemmings, click to assign). One scripted solution (timed assignments) gives the same saved count and the same end time in whole seconds as the original, and reaches a level-end state. The A6 and A7 frame gates still pass |

**User decisions, Phase D.** Full text in [Phase D history](docs/plan-history/phase-d.md).
- **D1 direction (user, 2026-10-21):** the D1 gate stays as written; the facing-label
  question waits for the spawn drift fix.
- **D1 direction after D1l (user, 2026-10-21, CEO review):** gate held as written, not
  amended.
- **D1o: one fresh live recheck, then parked (user decision, 2026-10-22, CEO review).**
- **D3f: recorded real assignment times, replayed exactly; then parked (user decision,
  2026-10-22, CEO review):** no gate amendment and no further fitted click-model round.

## Phase E — the remaining debts

Short, and explicitly parked rather than cancelled. **The gadget backlog is gone** — rounds
13–31 closed every one, including eleven clean negatives where a cue turned out not to be a
gadget at all. See the gadget table in `docs/LESSONS.md` before assuming anything here is
open.

One line per parked item; the evidence and the resume brief are under each link in
[Phase E history](docs/plan-history/phase-e.md).

- [B1 texture loader / render](docs/plan-history/phase-e.md#b1-texture-loader-and-render) — parked 2026-10-23 at the cap (B1p–B1r); since answered by B1t–B1w, B1 done.
- [B1 source, both arms](docs/plan-history/phase-e.md#b1-source-both-arms) — B1v observed one `^Font` source; B1w render passes.
- [C3 backdrop scenery](docs/plan-history/phase-e.md#c3-backdrop-scenery) — C3bv final-pack SOURCE adoption .06354819 FAIL; C3 reassessed after3attempts, move to C2/B2/B3. Clip/native state/admission gaps retained.
- [C3 entrance and exit models](docs/plan-history/phase-e.md#c3-entrance-and-exit-models) — re-parked 2026-10-24; Exit width gap open, no render.
- [C3 draw-correlated observation](docs/plan-history/phase-e.md#c3-draw-correlated-observation) — finalone-shot rejected numericTaskState4 beforePopen; bounded offline enum-bridge fix needed,0starts/no pause values ([check](docs/scratch/phase-c3-pause-one-shot.md)); no new method permission needed.
- [C3 observer](docs/plan-history/phase-e.md#c3-minimal-observer-readiness) — core parked for window; no live route (C3bu).
- [C3 clip](docs/scratch/phase-c3bl-scene-clip.md) — attempt11; component only.
- [C2 Paused caption text and blend](docs/plan-history/phase-e.md#c2-paused-caption-text-and-blend) — parked 2026-10-23 (C2j, C2k); device render state unread.
- [C2 cursor sprite](docs/plan-history/phase-e.md#c2-cursor-sprite) — C2o policy read/implemented under fresh cap exception; runtime composition/native hover and retained exit-marker producer still open.
- [B2 level-to-map](docs/plan-history/phase-e.md#b2-level-to-map) — parked 2026-10-23; writer of `[0x4C6A08]` unread.
- [B2 screen state machine](docs/plan-history/phase-e.md#b2-screen-state-machine) — parked 2026-10-23 at the cap; event producer and highlight accessor unread.
- [C1 live icon inputs](docs/plan-history/phase-e.md#c1-live-icon-inputs) — C1l read; superseded by C1m–C1x.
- [C1 icon-draw mechanism](docs/plan-history/phase-e.md#c1-skill-panel-icon-draw-mechanism) — parked 2026-10-22; `+0x3c` writer unread; selection highlight.
- [C1 upload call](docs/plan-history/phase-e.md#c1-upload-call) — parked 2026-10-24 at the cap (c1ai, c1ak, c1al). Constructor runs at startup; `0x471746` is not on that chain.
- [C1 launch-time alpha read](docs/plan-history/phase-e.md#c1-launch-time-alpha-read) — parked 2026-10-24 at the cap (c1am–c1ao). The start exited before the level; the shift site logged 0. One start remains and waits on an offline repair.
- [C1 plate alpha format](docs/plan-history/phase-e.md#c1-plate-alpha-format) — parked 2026-10-25 at the cap (c1ay–c1ba); the bit-count word is not in the image.
- [B3 map screen object](docs/plan-history/phase-e.md#b3-map-screen-object) — parked 2026-10-25 after b3a–b3d; the jump index is another argument, not the creation counter.
- [B3 level pick](docs/plan-history/phase-e.md#b3-level-pick) — parked 2026-10-25 at the cap (b3e–b3g); the name is an archive path, and the filename pushes call `0x49C760`.
- [B1f glyph matching](docs/plan-history/phase-e.md#b1f-glyph-matching) — parked 2026-10-22; do not revive the fitting method.
- [B1h Quit registration](docs/plan-history/phase-e.md#b1h-quit-registration) — parked 2026-10-22; superseded by B1i–B1w.
- [D2(b) digger bite timing](docs/plan-history/phase-e.md#d2b-digger-bite-timing) — parked 2026-10-22; architectural, multi-round.
- [D3 whole milestone](docs/plan-history/phase-e.md#d3-whole-milestone) — parked 2026-10-22 after D3f, user decision.
- [D1 whole milestone](docs/plan-history/phase-e.md#d1-whole-milestone) — parked 2026-10-22 after D1o, user decision.
- [D1 facing-label residual](docs/plan-history/phase-e.md#d1-facing-label-residual) — parked 2026-10-21 after D1h; harness precision.
- [Other debts](docs/plan-history/phase-e.md#other-debts) — 14 of 19 `0x00430BA1` call sites; platform targets; intro, music and FMV.
- [A7 lower-band cause](docs/plan-history/phase-e.md#a7-lower-band-cause) — open after A7l; A7 accepted with the caption excluded.
- [Actor filter/alpha at zoom 1](docs/plan-history/phase-e.md#actor-filter-and-alpha-at-zoom-1) — parked 2026-10-20 after A4z–A4af.
- [A6d3 slope walker clause 2](docs/plan-history/phase-e.md#a6d3-slope-walker) — parked 2026-10-20; 0.101 → 0.112.
- [A6 sim → `+0x08` mapping](docs/plan-history/phase-e.md#a6-sim-to-0x08-mapping) — parked 2026-10-20 after A6d10–A6d12.
- [A6 screen-Y bias](docs/plan-history/phase-e.md#a6-screen-y-bias) — parked 2026-10-20 after A6d20–A6d23.
- [A6 render-clock dt](docs/plan-history/phase-e.md#a6-render-clock-dt) — parked 2026-10-20 after A6d13–A6d15.
- [A6 first-row floor vs ceil](docs/plan-history/phase-e.md#a6-first-row-floor-vs-ceil) — parked 2026-10-20 after A6d18–A6d19.
- [A6 per-pixel actor Z](docs/plan-history/phase-e.md#a6-per-pixel-actor-z) — parked 2026-10-20 after A6d16.
- [A6 actor-2 sprite](docs/plan-history/phase-e.md#a6-actor-2-sprite) — parked 2026-10-20 after A6d4–A6d9.
- [Actor basis precision](docs/plan-history/phase-e.md#actor-basis-precision) — parked 2026-10-20; 2.962e-5.
- [Frida hook tooling](docs/plan-history/phase-e.md#frida-hook-tooling) — parked 2026-10-20; since proved by C1o/C1s.

## How we work

**One round, one question.** A round that half-answers three things is worth less than one
that closes a single thing. One agent owns the running game at a time.

**Work to the gate.** Rank the next question by how much full-frame error it removes against
the current milestone's gate. Three rounds on one question, then park it or re-justify it in
writing. Pixel-invisible residuals are parked. Tooling rounds need a gate justification. One
facilitator owns the queue; commit between rounds. Details:
[ORCHESTRATION](docs/ORCHESTRATION.md#milestones-gates-and-round-caps).

**Orchestrator–worker across every harness.** The facilitator plans and verifies; a round
agent runs one brief; an optional explore agent maps “where is X?”. Shared queue:
`docs/tasks.json`. Full loop and permissions: `docs/ORCHESTRATION.md`. Model names per
client: `docs/ROLES.md` only — do not pin a vendor string here.

Keep unverified candidate prose in `build/tmp/` so no original-game number reaches `docs/`
before the facilitator completes verification; the facilitator lands accepted scratch notes.
**Verification is not optional and does not delegate.**

The facilitator owns `PLAN.md`, `docs/**` and `.claude/rules/**`. Task agents own their
candidate scratch note outside `docs/`, the code and the tests.

Every brief states: the already-settled facts (so nothing is rediscovered), the round's
question with every lead already in hand, the evidence standard, the mutate-one-thing-alone
test requirement, the environment traps, a must-not-touch list, and "report honestly — do
not round, smooth or over-claim".

**Landing (2026-10-24).** Land a round's detail in its scratch note, append its round prose to
the phase file in `docs/plan-history/`, and change the milestone's status cell here in one
or two lines. User decisions go here, short, linking to their full text. `PLAN.md` stays
under the size limit in `scripts/check_agent_framework.py`.
