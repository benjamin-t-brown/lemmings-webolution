# fluid-lever-assessment-001: visible lever-driven fluid on installed MM_W_012 (clone)

Final stage of the three-stage visible-fluid milestone. Source-built clone only. The original executable, copied bytes, debugger, hooks and
capture were never used; no static REA session was opened (no new binary question). Everything below is a **scripted clone observation**
through the public frontend route, not human play, not original timing and not original-fidelity evidence.

**Status (revision 2, after the first review): everything except one item passes. The repeat pull was NOT obtained through ordinary input
in either run. This revision replaces the earlier "not demonstrated" note with an explicit finding that explains why (section 4,
"Repeat pull finding"): for lever (183,60) of installed MM_W_012 the +1 face list is empty and the -1 list only re-sends "raise" to receivers that
are already raised or ramping, so a second pull has no distinct visible effect in this level. I am not claiming an ordinary-input repeat
demonstration; the reviewer decides whether the finding satisfies the repeat requirement.**

Supervisor verification (final, cited): `build/agent-runs/20261112T201615-7804-e27ce7a8/verification.log` for the first worker exit of this task
(the run that produced the evidence below): **main 148/148 passed** (2607.14 s total; `fluidlever_play_stable-camera` 222.74 s,
`fluidlever_restart_stable-camera` 0.24 s, `fluidlever_play_ordinary-camera` 230.72 s, `fluidlever_restart_ordinary-camera` 0.23 s, and the unchanged
`levergates`, `leverinitface`, `sharedtiming*`) and **recovery 84/84 passed** (27.48 s). This is source verification only: the four `fluidlever_*`
tests assert driver integrity (route executed, artifacts written, normal save untouched), not that the lever was pulled or that the fluid looks right
(those are read from the output files and images below). It is not original-fidelity evidence. This revision changed only this report, so that log
remains the latest full run until the supervisor runs after this exit; the earlier partial worker run is superseded and not relied on.

## 1. What was added (no production file changed)

| File | Purpose |
| --- | --- |
| `src/__test__/test_fluidlever_assessment.cpp` | observation driver: queues SDL events through the production `runApplication`, observes through the existing read-only `LevelFrameObserver`, writes PNGs and logs. Writes no World/gate/lever/terrain/stock/save state. |
| `CMakeLists.txt` | registers `fluidlever_play_{stable-camera,ordinary-camera}` and `fluidlever_restart_*` (one block before the targeted-play block, same gating on SDL2 + `WEBOLUTION_GAME_DIR`). |
| `docs/scratch/fluid-lever-assessment-001.md`, `build/fluid-lever-assessment-001/` | this report and the evidence. |

Baseline of `src/app/main.cpp` was not modified in this task (`git status` / the fluid-surface-impl-001 report already record the pre-task file), so
the permitted `git show HEAD:src/app/main.cpp` was not needed and was not run.

## 2. Route (reproducible)

Both variants use the same SDL input script; only the camera operands differ.

1. Menu: click "Play Current Game" at (320,363).
2. Map: click node 8 at pixel (273,195) (the pixel `MapScene::pickNode` maps to node 8 = `MM_W_012`; the driver checks the map level name).
3. Level runs with **no skill, no click, no injected walker**. The lever (183,60) is pulled by whichever lemming walks into it.
4. After the surface is settled for 3 level-seconds the driver clicks the ordinary fast-forward button (54,348) so the level can end.
5. Results screen: click Continue (320,360) -> map -> click node 8 again (**reload**) -> observe the second entry -> quit.
6. A second process (`restart`) opens "Play Current Game" over the same private save and checks the map.

Variants: `stable-camera` = `--frontend --native --camera build/fluid-lever-assessment-001/stable-camera/stable-camera.json`
(`{"viewport":[640,480],"yaw_units":160,"level_eye_height":96}`, the camera of the fluid-surface-impl-001 public route); `ordinary-camera` = bare
`--game-dir` (hatch yaw 132 from the level, no camera file).

Save handling: the driver copies a **private** retained clone save (`build/targeted-play-001/runs/20261103-184055/save/...`, 1504 bytes, FNV
`163ecc74695712a2`, produced by targeted-play-001's public wins) into `build/fluid-lever-assessment-001/<variant>/save`. Provenance caveat: this is
the save as left after targeted-play-001's node 3 **and** node 8 wins, so node 8 is state 2 (completed, still selectable); this run did **not**
re-earn node 3 (that Blocker win is unreliable in recent runs: all seven `targetedplay_play_1` runs stamped 20261112 recorded LOSS in their `play-result.txt`, so I did not depend on it). The installed save and the clone's normal save
`%LOCALAPPDATA%\LemmingsWebolution\SavedGame.dat` (absent) were checked before/after: unchanged in both variants.

Reproduction command (supervisor-style, from the repo root; builds and runs all registered tests, including these):
`python tools/agent_control/verify.py --profile main`. Single test, after a build, with `SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy
WEBOLUTION_GAME_DIR=<Lemmings Revolution dir>`: `test_fluidlever_assessment play stable-camera build/fluid-lever-assessment-001` then
`... restart stable-camera ...` (same for `ordinary-camera`). Each play run takes ~230 s wall (Debug build, software raster).

## 3. Observed results (run in this worker; files under `build/fluid-lever-assessment-001/<variant>/`)

Both variants gave the same simulation: lever toggle at tick 205 / level clock 41.03 s, settle at 53.43 s (12.40 s after the pull; research
predicted about 12.25 s of dt, within the 0.1 s frame step), win 20/20 (need 10), `saveCount=1`, `saveError=-`.

Level ID, lever and receivers (from `events.csv` first-frame): `MM_W_012`, 4 levers, 19 gates; lever (183,60) `found linked=1 serializedId=2
initialFace=1 state=1`; receivers 5 and 6 `h=0 req=1 done=1 armed=1` at load. (The report of fluid-surface-impl-001 maps ids 5/6 to stream indices 12/11.)

| Time (level clock) | Event | id5 / id6 height (high 24.5) | zone-mask cells |
| --- | --- | --- | --- |
| 0.23 s | initial | 0 / 0 (drained) | 528 |
| 41.03 s (tick 205) | lever (183,60) state 1 -> -1, first accepted contact | 0.062 / 0.062, request 0 | 528 |
| 44.03 s | rising-1 (+3.00 s) | 6.062 / 6.062 | 624 |
| 48.03 s | rising-2 (+7.00 s) | 14.062 / 14.062 | 752 |
| 53.43 s | settled (request == done == 0) | 24.5 / 24.5 | 912 |
| 56.43 s | settled + 3 s | 24.5 / 24.5 | 912 |
| reload entry, 0.23 s | fresh level load | 0 / 0, lever state 1, request 1 | 528 |

Zone-mask growth 528 -> 912 = +384 = 2 receivers x 192 cells, consistent with the accepted lever-gates contract. Rise rate 2.0 per level-second
(6.062 at +3.00 s, 14.062 at +7.00 s). Wall time is **not** game time: the Debug software renderer produced ~5 frames/s and the production 100 ms stall cap
makes the level clock run slower than the wall clock (pull at wall 131 s for clock 41 s). The clone's route is deterministic here: both variants
pulled at the identical tick 205.

### Image index

Files in `build/fluid-lever-assessment-001/{stable-camera,ordinary-camera}/`, listed with state in each `image-index.txt` (frame, tick, clock,
lever state, both receivers, mask cells, camera yaw / eye height / zoom shift). `NN-entryK-*.png`:

| File | State | Inspected by this worker (Read) |
| --- | --- | --- |
| `01-entry1-initial` | before any lever contact | yes, both variants |
| `02-entry1-lever-pull-1` | the pull frame (h = 0.062) | yes, both variants (ordinary: thin blue sliver at the bottom of the right-hand tank) |
| `03-entry1-rising-1` | moving, h 6.062 | yes, both |
| `04-entry1-rising-2` | moving, h 14.062 | yes, both (ordinary: about half full) |
| `05-entry1-settled` | settled, h 24.5 | yes, both |
| `06-entry1-settled-plus-3s` | stability | yes, stable only (revision 2): lower tank full and unchanged, upper-left tank full |
| `07-entry2-reload-initial` | reload | yes, both (ordinary shows the "Let's Go!" banner, lower tank empty) |
| `08-entry2-reload-plus-frames` | reload, running | yes, stable only (revision 2): lower tank empty again, upper-left tank full, top-right counters 1 / 0 (early in the fresh run) |
| `map-before.png` | frontend map with earned nodes | yes, stable only (revision 2): a small cluster of green and yellow nodes at the left of the node field, all others plain/locked (consistent with the logged `states_at_map`; I did not map pixels to node numbers) |
| `results-screen.png` | "You Totally Stormed That Level!", needed 10, saved 20 | yes, stable only (revision 2) |

Not viewed by this worker: `06`/`08` of the ordinary-camera variant and its `map-before`/`results-screen`; their numbers are in the index and logs only.
Independent reviewer inspection is still required by the task; this worker's inspection is as listed.

### What the images show (worker's reading, not a measurement)

* **Stable camera (yaw 160)**: the lower U-shaped tank between two riveted dark pillars (the lower of the two tank frames, the one the
  accepted contract places the receivers 5/6 in) is **empty** in `01` (open dark-frame floor visible) and **filled with blue translucent
  fluid** with a visible top edge: a thin sliver at the pull (`02`), about a quarter full at +3 s (`03`), about half full at +7 s (`04`),
  full to the frame top at settle (`05`). The upper-left tank holds water throughout (it belongs to the loaded-raised receivers 10-12, whose
  state never changes); that provides a same-frame reference for material/colour and shows the lever-driven tank reaches the same appearance
  when raised.
* **Ordinary camera (yaw 132, no camera file)**: the same lower tank is visible at the lower right between the right pillars, partially behind a pillar. It is
  empty in `01`, a low surface in `03`, and filled in `05`. The upper tank is the full-width one in the centre. The "Let's Go!" banner is
  present in `01` (hence the large pixel-difference numbers in that variant's `events.csv`).
* **Placement / occlusion (plausibility under the provisional LessWriteOn policy only)**: fluid sits inside the tank volume, stops at the
  pillars (the pillars and the frame floor are drawn over it), the lemmings walking above and beside are drawn over/in front as expected, and I saw no
  z-fighting, no fluid outside the tank, and no fluid hovering when the tank is empty. This does not prove original depth fidelity; the
  fluid-surface-impl-001 depth diagnostic (303 pixels hidden under Less vs depth-disabled in the settled view) is the only quantitative
  occlusion evidence and is unchanged.
* **Synchronization**: the surface height in the images tracks the logged receiver height at each capture (empty at h = 0, ~1/4 at 6.06, ~1/2 at
  14.06, full at 24.5). It is a visual correlation from 5 frames, not a pixel-exact measurement.
* **Reload**: `07` matches `01` visually: tank empty, lever state 1, receivers 0/request 1. Numerically the reload frame differs from
  entry 1's initial frame by 36 viewport pixels (lemming/scenery animation timing) in the stable variant and by **0 pixels** in the ordinary-camera variant (the
  first stable frame lands on the same tick 7 / phase 1.531).

Pixel differences logged (`diff-vs-initial` events) are over viewport rows 40-329 and include lemmings, scenery animation and the cue banner;
they are **not** a fluid-only measure. The fluid-only matched-time control is the registered fluidsurface_impl test.

## 4. Pass/fail matrix

| # | Required check | Result | Evidence |
| --- | --- | --- | --- |
| 1 | Ordinary clone route on installed MM_W_012 reaches and activates lever (183,60) with receivers 5/6 | **PASS** (both variants) | `events.csv`: lever-toggle tick 205, no skill, no injected walker, no World write |
| 2 | Surface visibly changes with the lever (initial -> moving -> settled) | **PASS** | images 01 -> 05, height log |
| 3 | Moving and settled states with camera identification | **PASS** | stable camera yaw 160 / eye 96, ordinary yaw 132 / eye 96, in each index line |
| 4 | Synchronization with gate/mask state | **PASS** | heights 0.062 / 6.062 / 14.062 / 24.5, mask 528 / 624 / 752 / 912, id5 = id6 |
| 5 | Repeat pull | **NOT obtained through ordinary input; explicit finding instead (see "Repeat pull finding")** | natural flow produced exactly one toggle (`lever_toggles=1`, `repeat_toggles=0`) in both variants; the installed data make a repeat pull visually inert for this lever |
| 6 | Reload / exit | **PASS** | win -> results -> map -> node 8 again: fresh load, receivers drained, lever state 1; 40-53 frames observed |
| 7 | Restart in a fresh process | **PASS** | `restart-result.txt`: VERIFIED, node 8 map state 2, save FNV unchanged |
| 8 | Saves preserved | **PASS** | private save FNV `163ecc74695712a2` at start and end; normal save absent before/after; installed save not touched |
| 9 | No invisible / test-only draw path accepted | **PASS** | images come from the production `LevelFrameView::image` of `runApplication`, composed with HUD |
| 10 | Supervisor main/recovery verification | **CITED (earlier worker exit)**: main 148/148, recovery 84/84 | `build/agent-runs/20261112T201615-7804-e27ce7a8/verification.log`; source verification only; a new supervisor run follows this revision |

### Repeat pull finding (revision 2)

Mechanics, from the production code and the real installed record (`World.cpp:161-171` `leverAccepts`, `:1101-1123` `stepLeverContacts`,
`:282-288` `activateLeverFace`; `test_levergates.cpp:304-308,332-333`, which loads installed `MM_W_012` and asserts these facts):

* Lever (183,60), serialized id 2, initial face +1, cooldown 0. List -1 (face -1) = (id 5, type 4), (id 6, type 4) = "raise" to receivers 5/6.
  **List +1 is empty** (`lev.facePlus.empty()`, a static fact of the installed level data, not a fixture).
* A contact needs a live Walker inside the 2x6 box whose facing is -1 while `state == 1`, or +1 while `state == -1`. The accepted facing becomes the new state and
  that face's list is sent.
* Pull 1 (state 1 -> -1, facing -1) sends raise to 5/6. This is the toggle that occurred at tick 205 in both runs.
* A second pull needs a walker facing +1 in the box. It flips the state to +1 and sends the empty +1 list: **nothing reaches any receiver**. A third
  pull (facing -1 again) re-sends type 4 to receivers that are already ramping or raised; request is already 0, so the height and mask are unchanged
  (source test with a synthetic walker, `test_levergates.cpp:412-424`, fixture only: "repeat during the ramp did not reset the height"; the settled case is
  the "repeat pulls after settle" stage of fluid-surface-impl-001, also fixture only).
* Therefore, in this level and for this lever, no repeat pull can change the fluid surface in either direction. The linked fluid can only be raised from a
  drained start; nothing in the installed lever data drains it again (no type 5 appears in the installed lever lists, `test_levergates.cpp:328`).

Observation: in both natural runs only one toggle occurred (`lever_toggles=1`). After the first pull the lever is at -1 and accepts only +1 facers, and the
walkers all arrive facing -1, so none re-triggered it. This is inference from the toggle log (frames.csv holds no per-lemming positions or facings), not a
measured walker census.

What was not done, and why: a genuine second contact needs a lemming turned to face +1 inside the box. The route would be an ordinary clone input script
(select Blocker with key 4, hover-confirm a walker left of the lever, click, as the `leverMode` plan in `test_targetedplay.cpp` was designed to do, then observe the
turned walkers re-cross the box). That plan has never been run on node 8 (targeted-play-001 needed only its natural attempt). This invocation could execute only the
registered verifier and file tools (an attempt to edit the driver through the shell was denied), so a driver change could not be compiled or run, and an unrun change
would risk the supervisor's mandatory build. I therefore did not add it. Predicted outcome if it is run later: lever states 1 -> -1 -> 1 -> -1 with no change to
receivers 5/6 after the first pull (prediction from the mechanics above, **not an observation**).

Result for acceptance: the repeat state is reported as a documented finding (no distinct visible repeat effect exists for this lever), supported by real installed data
plus fixture-level source tests; **an ordinary-input second contact was not demonstrated**. If the reviewer requires an observed second toggle, that requires a new bounded driver
task; it is not a defect of the clone fluid.

## 5. Failures, limits and unknowns (not hidden)

* Original depth state (ZENABLE/ZWRITEENABLE/ZFUNC/device ZBIAS) is still unresolved; the images support only "plausible under the
  provisional LessWriteOn policy" for the two camera views, not original fidelity. Any change of that policy must be documented as a revision.
* Not drawn (accepted omissions of the research): the `+0x15C` additive overlay, `+0x164` pipes, `^water_glow` spray emitters, kind-1 receivers. Drowning
  behaviour, lemming-fluid interaction, other levers' receivers and all other levels are untested here.
* Hybrid timing: walk/fall continuous, other states 5 Hz; unchanged. Fast-forward was clicked after the settled observations only.
* The node-3 prerequisite was inherited from a retained save, not re-earned in this run (section 2).
* Only two cameras were observed. The ordinary camera sees the lever-driven tank only at an oblique angle at the lower right.
* Images not viewed by this worker are listed under the image index (ordinary-camera `06`, `08`, `map-before`, `results-screen`).
* No ordinary-input second lever contact was observed (Repeat pull finding, section 4).
* The driver's exit code checks driver integrity only; the pass/fail above is read from the output files, not asserted by CTest.

## 6. Milestone summary and handoff

On installed `MM_W_012`, through the public frontend route and natural play, a lemming pulled lever (183,60), receivers 5/6 ramped 0 -> 24.5 at
2.0 per level-second with the zone mask growing by 384 cells, and the lower tank visibly filled from empty to full in the production frame, in
both a fixed stable camera and the ordinary level camera; a fresh level entry and a fresh process both returned the receivers to the drained
state with saves intact. Supervisor main 148/148 and recovery 84/84 passed for the evidence run (log cited in the header; source verification only).
Open for acceptance: the repeat-pull requirement, which is answered by a finding (the installed +1 list is empty, so a repeat pull has no distinct visible effect for
this lever) rather than an observed ordinary-input second contact; independent image inspection; and the unchanged fidelity gaps (original depth state, omitted
overlays, drowning, other levels, hybrid timing). Next action: independent acceptance review, then the controller stops (no further work planned). If the
reviewer insists on an observed second toggle, the only route is a new bounded driver task (Blocker-turn plan above); no gameplay change is implied.
