# Retained legacy acceptance gates

These are the retained signed gates, copied verbatim from [the legacy plan](../../PLAN_OLD.md).
Their bounded and amended scopes are **not full 1:1 completion**. This documentation
reorganization does not change any gate. The draft completion roadmap adds work; it does
not revise these gates. Current implementation status belongs in [STATUS](../STATUS.md),
and the current plan is [PLAN](../../PLAN.md).

## Phase A — 3D view

| Milestone | Exit gate |
| --- | --- |
| A1 camera and controls | Four static views and zoom/elevation clamps pass the fixed 0.02 metric (0.0116–0.0160); 473 control points |
| A2 tube fixed ROI | Nine retained views + exposed INN region pass; wider/cavity/rim moved to A7 |
| A3 sprite view selection | 608 native branch cases, 42/42 live actor-bound index/mirror |
| A4 entry point, tube route, window | `webolution` runs 3D by default; 9/9 views on the production route (A4d); SDL controls (A4e) |
| A5 one actor billboard raster | Both A4f fixtures beat absent-actor; footprint exact (A4v); POINT + `> 0xEF` (A4x/A4y). Zoom 1 residual parked (E) |
| A6 sim lemmings in the app frame | Restated gate on the repair 4 pause: tube ROI 0.015584142; all three actor boxes improve. A6d3 slope walker parked (E) |
| A7 scene completeness | Amended gate passes: nine `MM_CLIMB_002` views (A7p) + one second level (A7q), caption panel + ten skill-panel boxes excluded, all ≤0.02. See below |

### Retained Phase A qualifications

The cells above are verbatim historical wording, including scores and “See below.”
The accompanying amendments in `PLAN_OLD.md` specify:

- A6: tube ROI `[100,80,280,410]` ≤ 0.02 actors-on, with each actor box better
  with actors than without. Full frame was reported, not gated, and becomes A7/C.
  [Amendment and scope](../plan-history/phase-a.md#a6-sim-lemmings-in-the-app-frame).
- A7: exclude the pause caption `[277,422,363,454)` and the ten skill-panel boxes,
  x `[18,90)`. [Amendments and scope](../plan-history/phase-a.md#a7-scene-completeness).

## Phase B — frontend

Unless a gate says otherwise, the metric is the A4d fixed changed-pixel score, scored
with `tools/harness/score_a4_render.py` methods on a full 640x480 frame. The threshold
is 0.02.

| Milestone | Exit gate |
| --- | --- |
| B1 frontend assets and text | Background, button and font sprites and the `.LTX` strings for the reference language (English) load. One reference screen (main menu, highlight 0) renders full-frame ≤ 0.02 against a stock screengrab |
| B2 screen state machine | Scripted key sequences (menu, confirm, map, into a level, back) visit the same screen ids (`0x004C416C`) in the same order as the original. Quirk preserved: RETURN only at highlight 0. (Short-ESC clause dropped, user, overlord review 2026-10-23: retracted after B2h.) |
| B3 level select and map | Map screen and level-select full-frame ≤ 0.02 on two unlock states. Selecting `MM_CLIMB_002` from the map reaches the D3 level |

**2026-10-28 evidence conflict, not a gate rewrite:** B2's blanket RETURN quirk
is contradicted by [verified row1 RETURN → Map](../scratch/frontend-return-negative-prediction-2026-10-28.md).
The historical wording above remains verbatim pending explicit correction. Do
not implement that disproved row1 restriction or claim B2 passes as written.

## Phase C — in-level HUD

| Milestone | Exit gate |
| --- | --- |
| C1 skill panel | Panel indices 2, 5, 6, 8, 9 identified (jump table `0x0045A4DD`). The ten buttons and their counts render inside `x [18,90)` ≤ 0.02 against the A7p/A7q captures, with the A7 panel exclusion removed. **Measured over the panel footprint** (user, overlord review 2026-10-24): the union of the ten button plates, icon rectangles and count-glyph rectangles at their read positions, fixed and committed before any render it scores. Not the C1b `baseline != no-cell-routes` mask, which meets the panel in 0 pixels (C1t). **Gated views** (user, overlord review 2026-10-24, second amendment; third amendment 2026-10-25): A7p `zoom1` and `elev_down`, each ≤ 0.02, rendered with their A7p camera params; `elev_up`, `initial`, the other A7p views and `MM_DIG_000` are scored and reported, not gated, until C3 draws the backdrop behind the panel |
| C2 counters, cursor, pause caption | `^saved`/`^out`/`^time` and the Paused caption `[277,422,363,454)` drawn. The A7 nine-view + second-level score passes with **no** HUD exclusions. Cursor sprite set matches on hover/no-hover/exit |
| C3 full frame | The A6d24 full-frame score (0.2123 today) ≤ 0.02 on the A6 repair 4 pause with actors, the floor ring included |

“Today” in the C3 cell belongs to the historical gate text, not current status.

## Phase D — simulation

All three gates use the original's actor list at `0x004CBB18` as the comparison oracle;
`measure_steppers.py --compare` is the per-tick oracle. For D1, 150 samples is the
retained comparison protocol, **not** a revival of the retracted discrete 5 Hz engine
assumption.

| Milestone | Exit gate |
| --- | --- |
| D1 release and walk-out | `MM_CLIMB_002`, no input, from level start: for 150 ticks (30 s), every lemming's cell X/Y, facing and state match the original each tick, and the released count and order match |
| D2 click-to-assign | (a) For 20 recorded cursor positions, the in-level pick (AABB against `lemming+0x128`, cursor doubles at `0x4CBFB0`+0x16A0/+0x16A8) picks the same lemming as the original, or none when it picks none. (b) Climber, floater, blocker and digger each assigned once: the 50 ticks after each assignment match the actor list exactly, and the skill counts decrement the same way |
| D3 one retail level end to end | `MM_CLIMB_002` playable in `webolution` (3D view, lemmings, click to assign). One scripted solution (timed assignments) gives the same saved count and the same end time in whole seconds as the original, and reaches a level-end state. The A6 and A7 frame gates still pass |
