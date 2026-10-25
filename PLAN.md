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

The old goal was *the simulation is correct*. It was reached, across **31 research rounds** —
every behavioural constant in `src/sim/World.h` is a read of the shipped bytes plus a live
series, all nine classic skills step, and every gadget in the game is settled and
implemented — and it produced something that does not look like Lemmings Revolution. It booted into a flat 2D debug view,
drew every lemming from sprite view 0 regardless of viewing angle, and showed a placeholder
menu in our own font.

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
| A1 camera and controls | **done** | Four static views and zoom/elevation clamps pass the fixed 0.02 metric (0.0116–0.0160); 473 control points |
| A2 tube fixed ROI | **done** (ROI) | Nine retained views + exposed INN region pass; wider/cavity/rim moved to A7 |
| A3 sprite view selection | **done** | 608 native branch cases, 42/42 live actor-bound index/mirror |
| A4 entry point, tube route, window | **done** | `webolution` runs 3D by default; 9/9 views on the production route (A4d); SDL controls (A4e) |
| A5 one actor billboard raster | **done** (zoom 0) | Both A4f fixtures beat absent-actor; footprint exact (A4v); POINT + `> 0xEF` (A4x/A4y). Zoom 1 residual parked (E) |
| A6 sim lemmings in the app frame | **done** | Restated gate on the repair 4 pause: tube ROI 0.015584142; all three actor boxes improve. A6d3 slope walker parked (E) |
| **A7 scene completeness** | **done** | Amended gate passes: nine `MM_CLIMB_002` views (A7p) + one second level (A7q), caption panel + ten skill-panel boxes excluded, all ≤0.02. See below |

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
| B1 frontend assets and text | **done** (B1w, own re-score 0.005244; the five panels are measured from the reference, not read). Accepted on condition (user, overlord review 2026-10-23): the panel draw is read and re-scored; a failing re-score reopens B1 ([note](docs/scratch/phase-b1w-menu-render.md)) | Background, button and font sprites and the `.LTX` strings for the reference language (English) load. One reference screen (main menu, highlight 0) renders full-frame ≤ 0.02 against a stock screengrab |
| B2 screen state machine | queued; parked at the three-round cap (B2a–c) and level-to-map (B2j/B2k, no key or click path named) ([Phase E](docs/plan-history/phase-e.md#b2-screen-state-machine)) | Scripted key sequences (menu, confirm, map, into a level, back) visit the same screen ids (`0x004C416C`) in the same order as the original. Quirk preserved: RETURN only at highlight 0. (Short-ESC clause dropped, user, overlord review 2026-10-23: retracted after B2h.) |
| B3 level select and map | queued; not started | Map screen and level-select full-frame ≤ 0.02 on two unlock states. Selecting `MM_CLIMB_002` from the map reaches the D3 level |

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
| C1 skill panel | **in progress**. Plate format parked at the cap; `+6` is unread ([C1ba](docs/scratch/phase-c1ba-bitcount-source.md), [Phase E](docs/plan-history/phase-e.md#c1-plate-alpha-format)). Gate not scored. | Panel indices 2, 5, 6, 8, 9 identified (jump table `0x0045A4DD`). The ten buttons and their counts render inside `x [18,90)` ≤ 0.02 against the A7p/A7q captures, with the A7 panel exclusion removed. **Measured over the panel footprint** (user, overlord review 2026-10-24): the union of the ten button plates, icon rectangles and count-glyph rectangles at their read positions, fixed and committed before any render it scores. Not the C1b `baseline != no-cell-routes` mask, which meets the panel in 0 pixels (C1t). **Gated views** (user, overlord review 2026-10-24, second amendment): A7p `zoom1`, `elev_up` and `elev_down`, each ≤ 0.02, rendered with their A7p camera params; `initial`, the other A7p views and `MM_DIG_000` are scored and reported, not gated, until C3 draws the backdrop behind the panel |
| C2 counters, cursor, pause caption | **in progress**. Counters and caption drawn; blend parked (C2k). Cursor readers found; the sprite choice is parked ([C2n](docs/scratch/phase-c2n-cursor-read.md), [Phase E](docs/plan-history/phase-e.md#c2-cursor-sprite)). | `^saved`/`^out`/`^time` and the Paused caption `[277,422,363,454)` drawn. The A7 nine-view + second-level score passes with **no** HUD exclusions. Cursor sprite set matches on hover/no-hover/exit |
| C3 full frame | queued; full frame 0.2098 (C3a: `rest` 84.7%, skill boxes 11.3%). Backdrop and entrance/exit reads parked after C3m; five bodies unread, no render ([Phase E](docs/plan-history/phase-e.md#c3-backdrop-scenery)) | The A6d24 full-frame score (0.2123 today) ≤ 0.02 on the A6 repair 4 pause with actors, the floor ring included |

**User decisions, Phase C.** Full text in [Phase C history](docs/plan-history/phase-c.md).
- **Backdrop before C1** (user, overlord review 2026-10-23): backdrop scenery, entrance and
  exit models come before C1. Since parked in Phase E (C3 scenery, 2026-10-24).
- **Milestone-scoped standing runs (user, overlord review 2026-10-24, "good i approve").**
  A run may take a milestone to its gate: the facilitator ranks and splits each next question,
  and an unread input is queued, not a stop. Budget: 3 rounds per question, 3 concurrent
  offline rounds, a pool of 3 stock starts (each dry-checked offline first, no retry after a
  failed launch), about 4 hours or 12 rounds per run. No debugger, no choosing by score.
  First use: the C1 directive of 2026-10-24.
- **C1 alpha-count run: 2 more starts and a launch-time attach (user, overlord review
  2026-10-24, "ya sure, 2 yes").** Two stock starts after c1ah spent the pool, so one failed
  launch can be repaired. A read-only attach may go in at launch, before the frontend loads
  textures (right after `Popen`, or with the game spawned suspended), when an offline round has
  placed the `^tab` upload before the level click. Everything else in the milestone-scoped
  budget is unchanged.
- **C1 alpha counts: attach at launch without placing the upload first (user, overlord review
  2026-10-24, "i approve").** c1ai–c1al could not place the `^tab` upload in time offline. The
  next start attaches at launch and stays attached through level entry and pause, logging
  continuously, with read-only hooks at the format callback, the upload entry, the mask zero
  test and the shift. The 2 unspent starts stand; the alpha-count question gets up to 3 more
  rounds.
- **C1 alpha counts after the c1ao exit (user, overlord review 2026-10-24, "yep all are good
  i approve").** c1ao's launch start exited during startup, after a 16-bit upload kept mask
  `0x8000` and before any shift hit. Up to 3 more alpha-count rounds; the start pool is 2 again
  (1 left plus 1). The `frida` 17.24.3 package c1an installed for mouv's Python stays; the
  facilitator updates `docs/agents/MOUV_SSH.md` to say so.
- **C1 alpha counts: offline first, menu-attach only, then a convergence check (user,
  overlord review 2026-10-25, "yep all are good i approve").** c1aq and c1as, spawned
  suspended, both died at startup (`0xC0000005` on c1as) before `^tab` uploaded. Spawn mode
  is retired. Up to 3 offline rounds and 1 live round; a pool of 1 start, attached at the menu
  before the level click (the c1s/c1ah recipe); a crash is a hard stop. If this run ends
  without a gate score, the overlord brings the user a choice: park the plate-alpha question
  or amend the C1 gate.
- **Long multi-milestone run (user, overlord review 2026-10-25, "approve").** After c1aw's
  crash: neither park nor amend yet; first re-read c1au's `+7` fill coverage (the withdrawal
  looks wrong), then render C1. Scope C1, then C2 cursor, then B3; C3 and B2 reopen only with
  a written gate reason. Caps park and the run continues; park-or-amend goes to
  `pending_questions`. About 8 hours or 24 rounds; a pool of 3 starts, menu attach through
  the REA CLI; a crash ends that question's live arm, a second ends starts for the run.
- **C1 gate measurement amended (user, overlord review 2026-10-24).** C1t showed the scorer could not see the panel: its mask inside the skill bands is x 86..89, the drawn icons x 26..76, overlap 0, so `skill_mae` stayed at the pre-panel 0.1077 / 0.0554. The threshold (0.02), region and captures are unchanged; the pixels scored become the panel's own footprint, built from read geometry (plates, c1s icon positions with the c1p/c1r frame sizes, count glyphs at the c1s text-object positions) and committed before the render. Pixels where a plate is translucent still include whatever the stock frame shows behind it; that share is reported, not masked out.
- **C1 gated views amended (user, overlord review 2026-10-24, second amendment).** The plates are translucent: all 18033 plate-only footprint pixels differ between the eight A7p stock views, and they carry 83% of c1w's 0.2918. On the 4536 scenery-only pixels between the plates, our render differs from the stock frame by 0.48–0.52 on `initial`, `left`, `right`, `elev0_neg` and `canonical` (sand, sky, balloon: parked C3 scenery) and by 0.0183 / 0.0209 / 0.0174 on `zoom1` / `elev_up` / `elev_down` (the tube wall we draw). So the gate is taken on those three views; the threshold, footprint and region are unchanged. The scenery error alone is near 0.02 there, so a pass is not assured. Overlord scripts `build/tmp/overlord-2026-10-24/split.py`, `constancy.py`, `gaps.py`, `views.py` (measured from the saved A7p PNGs; evaluation, not a new capture).

## Phase D — reconnect the simulation

Wire spawn, exits, skill assignment and the measured gadgets into the new renderer, and get a
retail level playable end to end in 3D. Starts as soon as A6 passes; it runs alongside A7
and comes before B and C.

**Milestones and exit gates (user sign-off 2026-10-21, before any D round).** Every gate is
scored against the original's actor list at `0x004CBB18`
(`measure_steppers.py --compare` is the per-tick oracle), never against our own sim. Do not
change a gate after seeing a result; a needed change stops the standing run.

| Milestone | Status | Exit gate |
| --- | --- | --- |
| D1 release and walk-out | **parked in Phase E (2026-10-22, user decision); not accepted**: D1o fails literally, 657 / 305 divergences with the continuous clock off / on ([Phase E](docs/plan-history/phase-e.md#d1-whole-milestone)) | `MM_CLIMB_002`, no input, from level start: for 150 ticks (30 s), every lemming's cell X/Y, facing and state match the original each tick, and the released count and order match |
| D2 click-to-assign | **(a) accepted** (20/20, D2e); (b) 3 of 4 (climber/blocker/floater), digger parked: 16/50, needs a cross-state percent clock ([Phase E](docs/plan-history/phase-e.md#d2b-digger-bite-timing)) | (a) For 20 recorded cursor positions, the in-level pick (AABB against `lemming+0x128`, cursor doubles at `0x4CBFB0`+0x16A0/+0x16A8) picks the same lemming as the original, or none when it picks none. (b) Climber, floater, blocker and digger each assigned once: the 50 ticks after each assignment match the actor list exactly, and the skill counts decrement the same way |
| D3 one retail level end to end | **parked in Phase E (2026-10-22, user decision); not accepted**: saved count 10/10; two stock runs of one solution end 75 s apart ([Phase E](docs/plan-history/phase-e.md#d3-whole-milestone)) | `MM_CLIMB_002` playable in `webolution` (3D view, lemmings, click to assign). One scripted solution (timed assignments) gives the same saved count and the same end time in whole seconds as the original, and reaches a level-end state. The A6 and A7 frame gates still pass |

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
- [C3 backdrop scenery](docs/plan-history/phase-e.md#c3-backdrop-scenery) — re-parked 2026-10-24 after C3k–C3m; five bodies unread, then render.
- [C3 entrance and exit models](docs/plan-history/phase-e.md#c3-entrance-and-exit-models) — re-parked 2026-10-24; Exit width gap open, no render.
- [C2 Paused caption text and blend](docs/plan-history/phase-e.md#c2-paused-caption-text-and-blend) — parked 2026-10-23 (C2j, C2k); device render state unread.
- [C2 cursor sprite](docs/plan-history/phase-e.md#c2-cursor-sprite) — parked 2026-10-25 at the cap (c2l–c2n); two loads pass the handle on, and the callees are unread.
- [B2 level-to-map](docs/plan-history/phase-e.md#b2-level-to-map) — parked 2026-10-23; writer of `[0x4C6A08]` unread.
- [B2 screen state machine](docs/plan-history/phase-e.md#b2-screen-state-machine) — parked 2026-10-23 at the cap; event producer and highlight accessor unread.
- [C1 live icon inputs](docs/plan-history/phase-e.md#c1-live-icon-inputs) — C1l read; superseded by C1m–C1x.
- [C1 icon-draw mechanism](docs/plan-history/phase-e.md#c1-skill-panel-icon-draw-mechanism) — parked 2026-10-22; `+0x3c` writer unread; selection highlight.
- [C1 upload call](docs/plan-history/phase-e.md#c1-upload-call) — parked 2026-10-24 at the cap (c1ai, c1ak, c1al). Constructor runs at startup; `0x471746` is not on that chain.
- [C1 launch-time alpha read](docs/plan-history/phase-e.md#c1-launch-time-alpha-read) — parked 2026-10-24 at the cap (c1am–c1ao). The start exited before the level; the shift site logged 0. One start remains and waits on an offline repair.
- [C1 plate alpha format](docs/plan-history/phase-e.md#c1-plate-alpha-format) — parked 2026-10-25 at the cap (c1ay–c1ba); the bit-count word is not in the image.
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
