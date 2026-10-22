# Lemmings Webolution — Plan

Reset 2026-10-18. The previous plan and all 31 research rounds are preserved under `old/`.

**Read `docs/LESSONS.md` first.** It carries every fact worth keeping from the old plan, so
nothing below needs to re-explain the formats, the measured constants or the traps.

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

Kept, correct, and still building — **14 tests green**:

- `src/assets/**` — the container, LRZ, `.SPR`, `.WAV`, `.LVL` chunk walk, the stored
  collision grid, placed objects, hatch records.
- `src/sim/**` — `Terrain` and `World`: every constant measured, all nine classic skills,
  blocker fields, the 7x7 blast stencil, and every gadget — anti-grav, trampoline bounce,
  water/acid drown, teleport, SpeedPad, lever, splat-exemption zones and inverted gravity.
- `src/audio/Mixer` — opens an SDL device and plays retail cues. Kept but currently unwired.
- `src/shell/Progress` — unlock and progression logic, no SDL.
- `tools/harness/**` — 40+ re-runnable live-read scripts.

Moved to `old/`, to be rebuilt rather than retrofitted:

- `old/src/gfx/**` — the flat-first renderer. Its camera constants (`radius 60`,
  `distance 150`, `fovY 0.9`) are **guesses**, which is exactly what Phase A replaces by
  reading the original.
- `old/src/main.cpp`, `old/src/shell/Shell.*` — the placeholder entry point and menu.
- `old/src/__test__/test_cylinder.cpp`, `test_cylinderview.cpp`, `test_sprite.cpp` — each
  depended on `src/gfx`.

**Two debts this created, both paid in A4** (A4a restored the asset-side coverage and a
`webolution` target; A4e made it a 3D-default SDL app). Now 20 tests. Kept for the record:

1. **There is no runnable executable** until Phase A4. `src/main.cpp` is gone and the
   `webolution` CMake target with it.
2. **Asset-side test coverage regressed.** The three moved tests also covered
   `assets/Cylinder` tile naming (`gfx::parseTilePosition`, INN/OUT, row/col) and
   `assets/Sprite` state mapping (`gfx::spriteNameForState`). Re-establish that coverage once
   the new renderer decides where tile parsing and state→sprite mapping live. The old tests
   are in `old/src/__test__/` to copy from.

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

### A6. Sim lemmings in the app frame

`World` lemmings drawn by the app on the tube, each from the A3 selector, A5 raster and
light-map tint. The [A5m map](docs/scratch/phase-a5m-actor-integration-map.md) lists the
missing links; each is one round:

- **A6a pose and basis** — **landed** ([check](docs/scratch/phase-a6-actor-pose.md)):
  `src/app/ActorPose` reproduces native `+0x108` within 1e-4 on three live walkers via
  `0x43D56C` and the `0x43D2B8` template fallback. Basis residual 2.962e-5 is parked (E).
  Sim → `+8` writers/spawn/fall-Y are READ and parked (A6d10–A6d12);
  walk remainder and live Z 0.75 stay open.
- **A6b animation clock** — **landed** ([check](docs/scratch/phase-a6-actor-anim-clock.md)):
  `src/app/ActorAnim` carries remainder past `0.06`, scaled by strip rate; strip bind
  resets. Live walker series is integer-ms render slices, not the 200 ms sim tick.
  Outer budget writer unsettled; A6d must not feed `Lemming::frame`.
- **A6c composition** — **landed** ([check](docs/scratch/phase-a6-actor-tube-composition.md)):
  actors draw after INN + three OUT passes, list high→low, no Z-state writes.
  `[elem+0x18] >= 1.0` → `0x43A9BB`. Per-pixel Z vs tube unsettled.
- **A6d integration** — **landed** (accepted via A6f repair 4) ([compose](docs/scratch/phase-a6-injected-walkers.md),
  [original-0](docs/scratch/phase-a6-app-bbox-gate.md),
  [matched pause](docs/scratch/phase-a6-matched-state-bbox.md)): walkers draw after
  the tube and inject the live `+0x08` table. Walk temporal does not fix the
  slope walker. Live `+0x1D8` is **0.0** ([A6d5](docs/scratch/phase-a6-live-phase-1d8.md));
  clause 2 still **fails** actor 2 at native phase (0.101→0.112). Native view
  matches ours ([A6d6](docs/scratch/phase-a6-live-view-index.md)). Pose matches
  live `+0x108` ([A6d7](docs/scratch/phase-a6-live-pose-pair.md)). Overlay
  composite is A4y `> 0xEF` ([A6d9](docs/scratch/phase-a6-overlay-alpha-ef.md));
  actor 2 is unchanged (0.101→0.112) because the walk overlay is already 0/255.
  A6d24 scores the retained full frame at **0.21225567640250545 > 0.02**
  ([check](docs/scratch/phase-a6-full-frame-gate.md)); actors-on/off PNGs
  byte-match A6d3, so this is a newly scored failure, not a new regression.
  No named scene cause measured; next prioritize dominant global-error
  localization rather than another actor pixel fit. Actor 2 still fails clause 2
  (0.101→0.112). Screen-Y bias is parked after three rounds:
  incoming Y has no ±1, `+0x90` is the `0x43D56C` triple, and
  `0x4C6B88` is the per-draw stack-top copy
  ([A6d22](docs/scratch/phase-a6-xform-4c6b88.md)). A6d23's explicit
  live-matrix resume check agrees with compiled production screen Y within
  1e-4 px on three fresh paused actors
  ([check](docs/scratch/phase-a6-native-camera-pair.md)); no screenshot,
  no A6d3 pixel attribution or gate pass. Screen-Y lead is re-parked.
  `originY−1` stays an unshipped pixel fit
  ([A6d17](docs/scratch/phase-a6-actor2-bbox-pixels.md)).
  `floor(top)` does not move the matched
  frame ([A6d19](docs/scratch/phase-a6-first-row-floor.md)).
  Float top is `0x47AB30`; no integer first row
  ([A6d18](docs/scratch/phase-a6-billboard-top-row.md)).
  `originY−1` is a pixel fit — do not ship
  ([A6d17](docs/scratch/phase-a6-actor2-bbox-pixels.md)).
  Full-frame is reported, not gated, under the restated gate. Per-pixel actor Z versus tube is a
  negative READ
  ([A6d16](docs/scratch/phase-a6-actor-tube-pixel-z.md)).
  Render-clock dt is parked: `[+0xC8]` is the UV/homogeneous
  third component, not the A6b clock
  ([A6d15](docs/scratch/phase-a6-render-c8-dt.md)).
  Sim → `+8` parked after
  A6d10–A6d12 ([writers](docs/scratch/phase-a6-sim-plus8-writer.md),
  [spawn](docs/scratch/phase-a6-plus8-spawn-init.md),
  [fall Y](docs/scratch/phase-a6-plus8-first-y.md)). Z-order and overlay
  predicate do not help ([A6d8](docs/scratch/phase-a6-actor2-z-order.md),
  [A6d9](docs/scratch/phase-a6-overlay-alpha-ef.md)).

**Exit gate (restated 2026-10-20, with user sign-off, after A6d24):** on `MM_CLIMB_002`, a
paused original frame with at least three walkers, against ours with the same actor list
state injected:
1. the actors-on frame passes the A4d fixed metric on its fixed tube ROI `[100,80,280,410]`
   (PIL half-open xyxy, `tools/harness/score_a4_render.py`, ≤ 0.02), so adding actors does
   not break the accepted tube; and
2. in each actor's bounding box, ours-with-actors has lower error than ours-without-actors.

The full-frame score is reported, not gated, in A6. It becomes the A7 + Phase C gate.

**A6e under the restated gate** ([check](docs/scratch/phase-a6-restated-gate-rescore.md)):
clause 1 **passes** (tube ROI 0.015584142, actors-on identical to tube-only inside it);
clause 2 still **fails** on actor 2 only. A6 is not accepted.

**A6f fresh-pause attempt** ([check](docs/scratch/phase-a6-fresh-pause-gate-failed.md)):
the only authorized stock start aborted at entry pause on a scratch-ledger exception,
before the planned walk or screenshot. No fresh gate result; A6e's failure stands.
Optional production CLI removed. Further live work is blocked by the exhausted
one-start allowance and requires offline integrated write-audit validation first.

**A6f repair3:** a renewed user-authorized attempt reused A6d3 and waived that stricter
audit, but failed before game creation on a missing copied windowed-config dependency
([check](docs/scratch/phase-a6-fresh-pause-gate-failed.md#repair3-renewed-attempt-also-failed-acquisition-before-game-creation)).
No new image/table/score; A6e still governs. No retry under repair3's stop-on-failure
rule. Further live invocation needs renewed authorization plus bounded offline
dependency/full-stderr preflight, not a return to the waived ledger tooling.

**A6f repair4:** the user carried over repair3's unused start. Dependency and
launcher checks passed; one fresh capture now passes the ROI and three component
calculations (ROI 0.015584142; every scored box improves)
([check](docs/scratch/phase-a6-fresh-pause-pixel-result.md)).
The `--walkers` loader is independently checked; the default render is unchanged.

**A6 accepted (2026-10-20, user decision).** The repair 4 frame meets the exit gate
as written. Its acquisition has the same limits as the A6d3 capture made with the same
script: `same_pause` is asserted, not re-snapshotted; only three actor rows are kept;
the boxes are difference components, not actor-bound rectangles. A6d3 was accepted as
evidence with those limits, so repair 4 is held to the same standard, not a stricter
one. The repair 4 walkers are all flat (Y = 32). The A6d3 slope walker's clause-2
failure (0.101 → 0.112) still stands and is not explained by this sample. Every
actor-2 lead is already parked at its round cap, and the failure leaves the gate
passing, so it is parked in Phase E rather than chased.

**Why the gate changed after a capture.** The first version demanded the *whole* 640x480
frame ≤ 0.02. That was mis-specified when written: A1/A2/A4d only ever passed on the fixed
ROI, and the original's frame carries the HUD and scenery that A7 and Phase C own. A6d24
scored 0.2123 actors-on versus 0.2123 tube-only (actors remove 6,806 of 49.9M error
units), so clause 1 could not pass inside A6 whatever the actors did. Clause 2 is
unchanged; actor 2 still fails it (0.101 → 0.112). This is the only permitted gate edit,
made because the gate measured another milestone's work, not because a result was close.

Residuals that leave both conditions passing are parked, not chased.

### A7. Scene completeness

The A2 wider/cavity/rim failures (`0.022030839 > 0.02`), the upper gold band, and the
omitted scenery and object models (entrances, exits, gadgets). **Exit gate (restated
2026-10-21, with user sign-off, after A7l):** the A2 wider changed-pixel diagnostic ≤ 0.02 on
the nine retained views and on one second level, with the pause caption panel
`[277,422,363,454)` (PIL half-open xyxy) removed from each view's mask. **Amended
2026-10-21, user sign-off, after A7p:** the ten skill-panel buttons are also removed, x
`[18,90)` by y `[12,44)`, `[52,84)`, `[92,124)`, `[132,164)`, `[172,204)`, `[212,244)`,
`[252,284)`, `[292,324)`, `[332,364)`, `[372,404)` (read live at `0x4C9208`, stride `0x1B0`).
Both are Phase C HUD. Each view's mask
comes from its own suppression pair (baseline versus four cell routes suppressed), and is
scored against that same baseline. If another screen-fixed HUD element (timer)
intersects a view's mask, it is reported to the user, not excluded silently.

**A7a offline baseline/localization** ([check](docs/scratch/phase-a7-wider-mask-localize.md)):
current tube-only production reproduces A4d at `3031085/137520225 = 0.0220410125`,
still failing. Under a predeclared disjoint 16×16 partition, the top five cells
carry only 25.93% of positive excess (concentration falsifier triggered); four
are in the lower y=432 band. No cause identified. Only initial has an identified
matching suppression pair; eight other view masks and a second-level pair remain
unavailable in the checked evidence. Next bounded offline question is lower-band
residual characterization, not a fitted raster fix. This is round 1 of that
localization question; no stock start is authorized. A7 is not accepted.

**A7b stable lower-band diagnostic** ([check](docs/scratch/phase-a7-lower-band-characterize.md)):
B=`[0,432,640,448)` contributes E302804 (9.99% of global masked error);
both restorations are pixel-identical there and both suppression conditions give
identical band pixels. Removing all B error would ideally give 0.0198391255,
but **no correction or cause is accepted**. Paused-caption composition is only
a native-trace lead. The localization family has reached three dispatches
including packet repair and is parked in Phase E; no fourth pixel-only round.
A7's unchanged actual score is still 0.0220410125 and its other masks are missing.

**A7c native exception, bounded negative** ([check](docs/scratch/phase-a7-pause-caption-native.md)):
fourth dispatch confirmed the known pause-state writes and `STR_PAUSED` key at
0x4C3C18 (pointer at0x4C35E4), but found no caption draw/resource/layout/coverage
chain. One broad event/state function was inspected, not a complete draw graph.
No supported composition test or fix; actual gate unchanged. Cause re-parked;
future evidence must bridge from the in-level draw loop, not repeat the pause
event-handler/key-only search. No automatic fifth round or live start.

**A7d Sol draw-loop exception, partial READ** ([check](docs/scratch/phase-a7-draw-loop-caption-bridge.md)):
frame0x441845 -> H=receiver+0x2018 ->0x44CD06 ->0x44D0ED ->0x44E150 reaches
a base text-list submission; one flag arm passes placement argument432. This
is not yet a Paused owner: the producer/content of H and its connection to pause
state1 remain missing. H+0x224 timer and H+0x758/+0x77C text records are distinct.
No font/coverage/pixel fix. Re-park at the exact base-list producer boundary after
dispatch5; no automatic sixth round. Actual gate and missing masks unchanged.

**A7e continuation, negative READ** ([check](docs/scratch/phase-a7-base-list-producer.md)):
the user moved the aborted sixth dispatch here. Builder `0x45D27B` uses the
same `0x45CFF9` receiver that is passed to frame `0x441845`, and then stores
pause state 1, but the list pointer it passes is `G+0x25C68`, not H
(`G+0x2018`). Lookup index `0x54` is not a read string. Switch case 1 does not
reach this call. Later calls on that arm, `0x44DF40` and `0x44E010`, use the
same `G+0x25C68` object. H's producer is still unknown. No eighth pass on
`0x45D27B`. Actual gate and missing masks unchanged.

**A7f direct-caller scan, negative READ** ([check](docs/scratch/phase-a7-caller-dest-offset.md)):
17 direct calls to `0x4404FA`, none with list immediate `0x2018`. The frame
path's builder calls are H+`0x758`, H+`0x77C`, and H+`0x224`. `0x44D316`
is entered with `[ebp-0x10]+0x2018` and then builds at `+0x758` / `+0x77C`;
that slot is not identified as the frame receiver. The open producer
question is whether `G+0x25C68` is copied into H. Gate unchanged.

**A7g immediate scan, negative READ** ([check](docs/scratch/phase-a7-25c68-to-h.md)):
ten `add ..., 0x25C68` sites. Six are `G+0x25C68` inside `0x45CFF9`, and
`0x45E33C` calls `0x44E150` on that object. None stores the pointer at
`+0x2018`. The other four use different frame slots. A copy that does not
re-encode the immediate, and `[ebp-0x10]` at `0x444B8F`, stay unread. Gate
unchanged.

**A7h draw arguments, READ** ([check](docs/scratch/phase-a7-pause-list-draw.md)):
G+`0x25C68` is placed from two runtime formulas and drawn by `0x44E150`
only while pause state is 1. No constant in that setup lies in band B.
`160/480/180/260` belong to G+`0x26000` via `0x47AD8C`, not to this object.
The gate is unchanged, so A7 is not accepted.

**A7i rectangle edges, READ** ([check](docs/scratch/phase-a7-rect-26000.md)):
unscaled, the vertical pair is 180..260 and misses band B. A scale branch
adds `round(760 * ([G+0x25FF8] - 0.5)^2)` to 260 before the push, so the
miss is not on every path. The gate is unchanged. A7 is not accepted.

**A7j field stores, READ** ([check](docs/scratch/phase-a7-25ff8-stores.md)):
`0x45CB39` writes `3.5`, which misses the rectangle setup. A later
subtract of `2.0` can leave `1.5`, and `1.5` would scale the top edge to
`1020`. `0x45CEB3` then subtracts `[G+0x1C968]`. The gate is unchanged.
A7 is not accepted.

**A7k divisor, READ** ([check](docs/scratch/phase-a7-1c968-source.md)):
the subtracted double is `[G+0x25C64] / 1000.0`. With that integer at `0`
and `V_init` at `1.5`, the scaled top is `1020` and includes band B. The
integer is not a file constant. The gate is unchanged. A7 is not accepted.

**A7l scale integer, READ** ([check](docs/scratch/phase-a7-25c64-writers.md)):
`[G+0x25C64]` is a runtime tick delta (`0x45D09D`) or a conditional `100` (`0x45D96B`),
not a file constant. Which one holds at pause needs a memory read. Gate unchanged.

**Review of A7a–A7l (2026-10-21).** A7c–A7l traced what draws the **Paused** caption over
band B. That caption is a translucent HUD panel; A2's own scenery inventory assigns the
pause banner to Phase C. It entered A7's mask only because the reference is a paused frame,
the same mis-specification A6's first gate had. A7f–A7l ran seven rounds after A7e's
"no automatic eighth dispatch" with no recorded round-cap exception. No round addressed the
eight missing view masks or the second level. The caption chain is **parked in Phase C**.

Facilitator re-score, separate summer (`build/tmp/a7-review/caption.py`, output
`caption.json`): the `initial` mask with the caption panel removed scores
**2624993/135798210 = 0.019330100**, passing. The panel held 2,251 mask pixels and
406,092 error units. Removing whole band B instead scores 0.020232 because the panel
spans y 422–453. Below the tube the original also draws a green and purple floor ring
that ours leaves black; that is scenery, in A7's scope.

**A7 restated gate, why.** The first gate counted a HUD panel that A7 does not own and
could not be met without Phase C work. The threshold, metric and mask method are unchanged;
only the HUD panel is removed. A7 is not accepted: eight views and a second level still
have no mask.

**A7m capture, narrowed** ([check](docs/scratch/phase-a7-view-masks.md)): four stock
launches (PIDs 63180, 62032, 34140, 16700) against an authorization of two.
The `initial`, `mouse`, and `right` folders are one PNG triple. That baseline's
fixed ROI against `original-0.png` is `6117452/45441000 = 0.1346240619704672`.
The production render against the same original remains
`708159/45441000 = 0.015584142074338152`. The fresh wider-mask row
`7444647/84031425 = 0.08859360649899725` scores the production render on that
drifted frame (camera doubles `+0x50` and `+0x60` moved). It is not a gate
score. The durable caption-excluded initial score remains
`2624993/135798210 = 0.019330100153750186`.

**A7n input path, negative** ([check](docs/scratch/phase-a7-input-view-masks.md)):
one authorized start, pid 23460. All eight baselines fail the fixed ROI
against their retained originals. The best prescribed cases are left
`6476525/45441000 = 0.14252602275478093` and right
`7028355/45441000 = 0.15466990163068595`. `canonical/baseline.png` is
byte-identical to the post-unpause banner frame. `elev_down` and `elev0_neg`
are one PNG. No prologue was changed to `C3`, and there is no wider-mask
score. The logged zoom i32 is `control+0x0C`; `0x45CC61` reads `[eax+0x10]`.
The camera block was not saved, so its differing offsets are unread. The
durable caption-excluded score is unchanged. Do not repeat this input list.
Another stock start is not authorized.

**A7n diagnosis (facilitator, hypothesis):** A7n paused about 1 s after entry
(timer 04:59, no lemmings out). Its logged camera hash `2b33a708…` is not the
reference `7e6e1a8a…` that A6f repair 4 reached after a 14 s walk, and its
`canonical` baseline sits about a quarter turn from the retained original. The
likely cause is an opening camera move that had not finished. A7o tests this:
14 s walk, pause, hash and canonical ROI checked before any view input. Under the
standing run `a7-complete` (2026-10-21) A7o has one start.

**A7o settled-camera attempt, negative** ([check](docs/scratch/phase-a7-settled-camera-masks.md)):
one authorized start, pid 58432, reached entry pause on `MM_CLIMB_002` and then
raised because `LRInput.tap` returns `None`. The 14 s walk, the post-walk hash,
and every view ROI were not measured. The saved entry-pause block at `0x4CBFC8`
is 360 bytes, SHA-256 `619ef313758bffa1630fe465d30df802d4447bab3c4caab27ea848d35a84702b`,
which is neither the reference `7e6e1a8a…` nor A7n's `2b33a708…`. No prologue
was changed to `C3`. There is no wider-mask score. The durable caption-excluded
score is unchanged. The opening-camera hypothesis is untested. This is round 3
of acquisition; a fourth round is justified because the stop was the return
check, and the A6f walk that produced `7e6e1a8a…` does not test `tap`'s return.

**A7p settled camera, eight views pass** ([check](docs/scratch/phase-a7-unpause-view-masks.md)):
one start, pid 26648. After a 14 s walk the camera hash is
`7e6e1a8a77c29f860f82dd8cde3f60c9f89ecf3f1352d3c2a6801cc9867d0fff` and the
canonical ROI is `156/45441000`. All nine fixed-ROI numerators are at or
under 908820. Eight caption-excluded wider scores pass. `zoom1` is
`5473596/228864285 = 0.023916339764415404` and fails. Its saved camera hash
matches `params-zoom1.json`. The logged skill-panel rectangles hold 21600 of
that mask's pixels and 1738618 error units; outside them the inequality
holds. Those pixels stay in the score. No timer rectangle was read. No second
level was captured. A7 is not accepted. The standing run stops on this
intersection rather than changing the exclusion.

**A7p facilitator check and gate amendment:** a separate PIL re-score
(`build/tmp/a7-review/a7p/rescore.py`) reproduces every A7p numerator, denominator
and skill-panel count exactly. Saved camera blocks hash `0d03f6d4…` at the entry pause and
`7e6e1a8a…` after the 14 s walk, so the opening-camera hypothesis is now measured. Install
hashes and the five prologues match their start values; no process remained. With the
skill panel also removed (user, 2026-10-21) the nine views score: initial 0.019216,
canonical 0.019314, mouse 0.017612, right 0.017604, left 0.017716, zoom1 0.017590, elev_up
0.012934, elev_down 0.011169, elev0_neg 0.017576. All nine pass. A7 still needs the second
level.

**A7q second level, gate passes** ([check](docs/scratch/phase-a7-second-level-mask.md)):
one start, pid 21464, reused A7p's entry/14s-walk/pause path unedited on `MM_DIG_000`. The
no-camera-input wider-mask score (own baseline, caption panel and ten skill-panel boxes
excluded) is `2606399/132499530 = 0.019671005625453916`, passing. The params-derivation
self-check reproduced `params-canonical.json` on all 16 renderer-read fields from A7p's own
saved state (needing two offsets beyond the brief's cited lines: camera-block `aspect` at
+288 and `angle_turn_parameter` at +280, located against
`tools/harness/fixtures/camera_a1/reference-state.json` and confirmed). 4 mask pixels fall
in an informational, unexcluded timer-area box (a placement-argument read, not a measured
rectangle) and stayed in the score. Camera movement on a second level, and whether
`world_rows == 96` is level-independent in general, are unmeasured.

**A7q facilitator check and gate acceptance:** an independent re-score
(`build/tmp/a7-review/a7q/verify.py`, separate code reading only the three saved PNGs)
reproduces every number exactly; a 6px x-shift of the render alone raises the numerator
from 2606399 to 5360684 (fails), confirming the checker discriminates. The installed exe
hash, the five prologues (all `0x55`), and the camera-block offset decode were
independently re-verified against the executable and the A1 camera fixture, not just
read back from the worker's own logs. Fresh `ctest --preset dev-debug` is 24/24; `ninja`
had no work; no stock process or scheduled task remained. **The amended A7 gate now
passes on this independent re-score**: all nine `MM_CLIMB_002` views (A7p) plus this
second level (A7q), all ≤0.02. A7 is accepted. The standing run `a7-complete` stops here
per its "gate passes" condition (`docs/ORCHESTRATION.md#standing-run`).

After A7, Phase D's playable slice comes before B and C: a retail level playable end to
end in 3D is the next visible result. Its milestones and gates are under Phase D.


## Phase B — the frontend

The original's own screens: backgrounds, buttons, fonts, `.LTX` localised text (the menus are
German), the screen state machine, level select and the map. Preserve the quirks —
**RETURN only when the menu highlight is exactly 0**, and a short ESC that fails to leave
pause. Frontend screen ids are already known (`0x004C416C`: 3 menu, 4 confirm, 5 map).

**PROPOSED milestones, drafted 2026-10-21 (CEO review). Not signed off: no B round may start
until the user approves or edits these gates.** The metric is the A4d fixed changed-pixel
score, scored with `tools/harness/score_a4_render.py` methods on a full 640x480 frame unless
stated. The threshold is 0.02, the same as A1/A2/A7.

| Milestone | Status | Proposed exit gate |
| --- | --- | --- |
| B1 frontend assets and text | proposed | Background, button and font sprites and `.LTX` German strings load. One reference screen (main menu, highlight 0) renders full-frame ≤ 0.02 against a stock screengrab |
| B2 screen state machine | proposed | Scripted key sequences (menu, confirm, map, into a level, back) visit the same screen ids (`0x004C416C`) in the same order as the original. Quirks preserved: RETURN only at highlight 0, short ESC does not leave pause |
| B3 level select and map | proposed | Map screen and level-select full-frame ≤ 0.02 on two unlock states. Selecting `MM_CLIMB_002` from the map reaches the D3 level |

## Phase C — the in-level HUD

The skill panel art and counts, and the counters the engine names itself: `^saved`, `^out`,
`^time`, the icon sets `GreyIcons` / `BlackIcons` / `AdditiveIcons`, and the cursor set
(`_curLem`, `_curNoLem`, `_curEnt`, `_curExit`, and the left/right variants). Panel indices
0, 1, 3, 4 and 7 are known; 2, 5, 6, 8, 9 are not.

The ten skill-panel buttons (x `[18,90)`, objects at `0x4C9208`) are excluded from the A7
gate for the same reason and are drawn here.

The **Paused** caption panel (`[277,422,363,454)` on the 640x480 frame) belongs here. A7c–A7l
traced it without finding its draw owner, parked 2026-10-21: `STR_PAUSED` key at `0x4C3C18`;
`G+0x25C68` list drawn by `0x44E150` only while pause state is 1; rectangle
`G+0x26000` via `0x47AD8C`, whose top edge scales with `[G+0x25FF8]` and
`[G+0x25C64] / 1000.0`, the latter a tick delta or a conditional `100`. Resume from a
live read of `[G+0x25C64]` at pause, in Phase C. Evidence:
[A7c](docs/scratch/phase-a7-pause-caption-native.md) through
[A7l](docs/scratch/phase-a7-25c64-writers.md).

**PROPOSED milestones, drafted 2026-10-21 (CEO review). Not signed off: no C round may start
until the user approves or edits these gates.**

| Milestone | Status | Proposed exit gate |
| --- | --- | --- |
| C1 skill panel | proposed | Panel indices 2, 5, 6, 8, 9 identified (jump table `0x0045A4DD`). The ten buttons and their counts render inside `x [18,90)` ≤ 0.02 against the A7p/A7q captures, with the A7 panel exclusion removed |
| C2 counters, cursor, pause caption | proposed | `^saved`/`^out`/`^time` and the Paused caption `[277,422,363,454)` drawn. The A7 nine-view + second-level score passes with **no** HUD exclusions. Cursor sprite set matches on hover/no-hover/exit |
| C3 full frame | proposed | The A6d24 full-frame score (0.2123 today) ≤ 0.02 on the A6 repair 4 pause with actors, the floor ring included |

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
| D1 release and walk-out | **gate held; not accepted (D1n partial)** | `MM_CLIMB_002`, no input, from level start: for 150 ticks (30 s), every lemming's cell X/Y, facing and state match the original each tick, and the released count and order match |
| D2 click-to-assign | **(a) accepted**, (b) queued | (a) For 20 recorded cursor positions, the in-level pick (AABB against `lemming+0x128`, cursor doubles at `0x4CBFB0`+0x16A0/+0x16A8) picks the same lemming as the original, or none when it picks none. (b) Climber, floater, blocker and digger each assigned once: the 50 ticks after each assignment match the actor list exactly, and the skill counts decrement the same way |
| D3 one retail level end to end | queued | `MM_CLIMB_002` playable in `webolution` (3D view, lemmings, click to assign). One scripted solution (timed assignments) gives the same saved count and the same end time in whole seconds as the original, and reaches a level-end state. The A6 and A7 frame gates still pass |

Notes for D1: our sim has a known one-tick divergence on a left-facing turn (`0x0041C67E`,
LESSONS "The simulation"). D1 cannot pass without fixing it, and the blocker-turn
expectation moves with it. The release cadence and the exit boxes are already measured
(`measure_release.py`, `measure_exit.py`).

**D1a first-divergence localization** ([check](docs/scratch/phase-d1-release-walkout-trace.md)):
a new offline sim-trace tool (`src/tools/SimReleaseTrace.cpp`) and a new live no-input
capture of `MM_CLIMB_002` (one stock start, 30 s / 150 ticks off the actor list) found the
gate's **first** disagreement at tick 1, in the released count, not the previously-known
left-turn tick — our sim's `World` releases its first lemming unconditionally on tick 1
(`sinceSpawn_ = releaseIntervalTicks` at construction), while the original visibly waits
`t = 2.020 s` (~10.1 ticks, neither tick 1 nor a full measured interval) before its first
release, then releases every `2.887 s` (14.433 ticks) thereafter — a non-integer gap no
`int releaseIntervalTicks` can track exactly. The left-turn divergence was never reached.
No behavioral fix was made (out of this round's scope); the original's actual
release-trigger mechanism (what determines the first delay, and whether the gap is
level-specific or derives from `RELEASERATE.PC` through a mapping this project has not
found — `LevelInfo` carries no release-rate index) is unread. D1 is not accepted.

**D1b release-trigger mechanism, read** ([check](docs/scratch/phase-d1-release-trigger.md)):
a per-hatch real-time countdown (`hatch+0xC0`) drives every release, compared against
`0.0` at `0x00425A95`. It is seeded once to a fixed `1.3 s` (`0x004B0F48`) the moment a
door sub-object (`hatch+0x44`) finishes opening, then every later release *adds back* the
per-level interval at `hatch+0xB8` (a rollover, not a reset) — two different numbers
sharing one field, not one shared threshold with only a different start time. `hatch+0xB8`
is resolved at load (`0x00425DE3`) by clamping a per-hatch rate (`hatch+0xA0`, already
present but unnamed in `findHatchRecords`'s own file-format comment, alongside `+0xA4`
min / `+0xA8` max) into a 99-row table loaded at runtime from `Compiled\ReleaseRate.pc`
(`0x0042609D`) — exactly `src/assets/GameData.h`'s `ReleaseRates`, not yet wired to a
per-level selector. On `MM_CLIMB_002` (rate 20, clamped to `[1,50]`), that row is
`2.8864866275534937 s`, matching D1a's measured mean gap (`2.8867 s`) to `0.0002 s`. The
first release's own `~0.72 s` residual (`1.3 s` seed vs. D1a's measured `2.020 s`) is left
to the door-opening animation's own duration, not yet measured. D1 is still not accepted.

**D1c door-open duration, derived** ([check](docs/scratch/phase-d1-door-open-duration.md)):
a live, read-only capture of the hatch object (located by vtable `0x004B0F50`) missed the
literal `+0xAC` arming flip (object-location overhead ate the `~0.72 s` window), but
directly measured the countdown's rollover landing in `(2.020, 2.030]` s — confirming
D1a's first release from a second, independent field — and a linear fit of the
pre-rollover countdown (slope `-0.997`, residual `<=0.006 s`) derives `arm_time ~= 0.723 s`;
`arm_time + 1.3 s ~= 2.023 s` matches D1a's `2.020 s` to `~3 ms`. D1's tick-1
release-timing divergence is now closed to a few milliseconds. Open, not blocking: whether
`0.723 s` is a fixed universal delay or itself a per-level animation length, and whether
the hatch's countdown is genuinely continuous (render-frame-driven, consecutive samples
step unevenly at roughly 144 Hz rather than in 200 ms chunks) or only reads that way —
flagged, not settled. D1 is still not accepted; next is a static read of the hatch step's
own call cadence, then an implementation round against this now-measured mechanism.

**D1d hatch step clock, read** ([check](docs/scratch/phase-d1-hatch-step-clock.md)):
`0x00425A95` is reached only through virtual dispatch (vtable `0x004B0F50`+`0x14`), via a
generic per-object step dispatcher (`0x004423D2`) called from exactly three sites inside
`0x0045CFF9` — an unconditional per-pass "leftover" call carrying real wall-clock time, a
queue-drain call, and a paused fallback with a fixed `dt=0.0`. `0x0045CFF9` itself has
exactly one caller, once per rendered frame. **Verdict: the release countdown is
frame-driven, the same continuous-accumulator shape as the project's already-documented
`+0x1CC` animation clock, not the lemming stepper's discrete 200 ms tick** — explaining
D1c's uneven per-sample deltas as the real mechanism. The `dt` getter
(`[hatch+0x44]+0x48`) narrows to firing only once per hatch (the arm transition), not on
the recurring decrement; its concrete target, and whether D1c's `0.723 s` generalizes
beyond `MM_CLIMB_002`, are unresolved by static reading (an independent REA/Ghidra pass
hit the identical wall). D1 is still not accepted; next is an implementation round against
this now-fully-measured shape (continuous real-seconds accumulator, not an integer tick
count), using the general, confidently-known constants (`1.3 s` seed,
`RELEASERATE.PC`-derived interval) and leaving the unresolved `arm_time` generalization as
an explicit open parameter.

**D1e release timing, implemented** ([check](docs/scratch/phase-d1-release-timing-implementation.md)):
wired additively into `World` (new optional `LevelSetup` fields; every existing
`releaseIntervalTicks`-based setup is byte-for-byte unaffected). D1a's own unmodified
`diff_trace.py`, against D1a's own unmodified live capture and a freshly regenerated sim
trace using the measured constants, now first diverges at **tick 10** (an `existence`
alignment artifact — sim has no row yet where live already shows the lemming existing one
tick early) instead of **tick 1**; spawn-count transition ticks agree at every shared
point, drifting by 0-4 ticks from discretizing the original's continuous accumulator onto
our 200 ms grid. The already-known left-turn divergence (`0x0041C67E`) was never reached
in this comparison and remains the other open D1 blocker. D1 is still not accepted.

**D1f left-turn fix, implemented** ([check](docs/scratch/phase-d1-left-turn-fix.md)):
`World::stepWalker` now takes its post-turn step within the same tick when the walker
*was* facing Left (not, as this round's own brief mis-stated, when it turns *to* face
Left — corrected and reported by the round itself before any test literal changed).
Offline `compare_tick` reproduces round 10's own cited numbers exactly, fixed and
reverted; one live start matched 40/40 ticks with no regression, though it did not
itself freshly cross the specific left-wall scenario within the printed window (flagged
plainly, substituted with round 10's own already-landed live measurement). One test
literal changed as a direct, justified consequence (`testBlockerTurnsWalkers`'s second
sub-case, `131`→`132`); the first sub-case and `testWallTurn` are unaffected. **Both
named D1 blockers are now fixed.** D1 is still not accepted — a fresh, comprehensive live
re-check of the full gate (150 ticks, no input, every lemming) is the next D1 round.

**D1g full gate re-check, both fixes confirmed, one residual found**
([check](docs/scratch/phase-d1-full-gate-recheck.md)): a fresh live capture (new stock
start) confirms the tick-10 alignment artifact still appears first, and — checked for the
first time — lemming 0's wall turn at column 64 matches exactly (though that turn
exercises the already-good "right-facing" case, not D1f's new branch). Scanning the full
150-tick window past tick 10 found 167 more disagreements: 161 are D1e's already-known
spawn-tick drift propagating forward, and 6 are a new, smaller, self-healing single-tick
`facing`-only mismatch at every "walker was facing Left, turns Right" event — exactly
D1f's own target scenario, though `x` matches exactly before and after and an independent
full scan found zero reappearance of the stall D1f fixed. Whether this is a genuine sim
gap or an artifact of the harness's single global-phase alignment (D1a's own open
limitation: individual lemmings have stable but different phases) is unresolved. D1 is
still not accepted; the next D1 round is this one narrow, fully offline question — no new
live capture needed.

**D1h per-lemming-phase check, mixed result** ([check](docs/scratch/phase-d1-full-gate-recheck.md#d1h-follow-up-the-per-lemming-phase-hypothesis-mixed-result-2026-10-21)):
re-aligning each flagged lemming with its own measured phase (not the harness's one
pooled global phase) flips 3 of the 6 residuals to match exactly; the other 3 each have a
distinct, fully-characterized reason they don't (zero residual, a 0.01 s near-miss bounded
by raw-sample spacing, a wrong-signed residual at the phase method's own wraparound
boundary). **Correction surfaced in the same round, confirmed independently**: only 3 of
the 6 flagged ticks are the D1f-fixed "turns Right" scenario; the other 3 are the
already-good "turns Left" case, misattributed in D1g's own text (now corrected there too).
Every one of the 6 now has a complete, understood explanation, and an independent
full-trace scan found zero reappearance of the stall D1f fixed. **Both named D1 blockers
remain fixed and verified; the only open item is this one small, well-understood,
self-healing single-tick `facing`-label residual**, which traces to the live-capture
harness's alignment precision (not the simulation), not conclusively a sim defect. See
Phase E for the parking decision.

**D1 direction (user, 2026-10-21) and facilitator observation:** the D1 gate stays as
written. The facing-label question is deferred until the spawn drift is fixed. The drift is
not 200 ms discretization. In both saved captures the `released` counter steps every
2.88–2.90 s, while lemmings enter the actor list only on a ~1.02 s cycle
(`2.04 + k * 1.02` s within one 0.02 s sample in D1g; ~1.013 s in D1a). `World` spawns on
the counter. This is an observation from the saved meta files; the mechanism is D1i's
question.

**D1i actor-list delay explained: harness artifact, not an exe mechanism**
([check](docs/scratch/phase-d1-release-to-actor.md)): the ~1.02 s cycle above does not
exist in the game. The Lemming constructor (`0x0042DE51`) publishes the new pointer into
the `0x004CBB18` actor list strictly before it increments the `RELEASED` counter at
`0x004CBA78`, a few instructions later inside the same synchronous call — no clock read, no
animation-frame check, on the path (`0x00425A95`→`0x0042DE51`→`0x004230A2`→`0x004233A0`,
addresses and bytes read and independently re-read). The ~1.02 s period is
`tools/harness/measure_d1_release_trace.py`'s own `t - last_enum > 1.0` re-enumeration
throttle landing on its `1/hz = 0.02 s` sampling grid; an offline model replaying that
throttle against each capture's own real per-pass timestamps reproduces all twenty saved
`first_seen_seconds` values exactly (independently reproduced from scratch, plus a
discriminating no-throttle control that fails as expected). `released_count` in the D1
gate means the counter, which the exe read shows is the same quantity as the actor-list
size at any instant in the original — they diverge only in this project's own measurement.
**New, unactioned finding**: `measure_d1_release_trace.py::build_table()` builds its
per-tick `released`/`existence` columns from this same throttled `first_seen`, so D1e/D1g's
161-tick spawn-drift residual may be partly or wholly this harness bias rather than a sim
timing defect — open for D1j, not yet checked.

**D1j harness throttle fixed; mixed, precisely-characterized result**
([check](docs/scratch/phase-d1-harness-throttle-fix.md)): a landed fix (targeted tail
discovery off the already-unthrottled `RELEASED` read, not a full re-scan) eliminates the
throttle's detection lag outright — `first_seen` now matches the counter to
floating-point precision for all 10 lemmings in a fresh capture, independently confirmed.
Re-running D1a's/D1g's own unmodified comparators against this fix is a **mixed** result,
not a clean "fixed": the raw divergence count did **not** shrink (167 → 537 naturally,
275 even after correcting for a second, independent harness bug found as a byproduct —
below), but the metric the fix actually targets, each lemming's own spawn-tick drift
magnitude, **did** collapse, from up to 4 ticks (9/10 lemmings) down to at most 1 tick
(8/10 lemmings, consistent with the structural floor of discretizing a continuous release
onto a 200 ms tick, not with the throttle explaining nothing). The raw count does not track
drift magnitude once forward-propagation is accounted for (one tick of drift at spawn
produces one `x`-mismatch row per remaining tick). The already-parked 6-entry `facing`
family (D1g/D1h) is unchanged, as predicted. **D1 is still not accepted.**

A second, previously unknown harness bug was found as a byproduct, not fixed this round:
`build_table()`'s `n_min` (how many tick-grid points precede the capture's own `t=0`) is a
step function of the inferred per-capture `phase` landing on one side of zero or the other,
so two otherwise-equivalent captures' tick labels can differ by a full 200 ms — confirmed
by relabeling this round's own capture one tick earlier and reproducing D1g's exact
tick-10 first-divergence shape. This is queued next as D1k: fix the phase/`n_min` boundary,
then re-run a clean, uncorrected full-gate comparison. No new live capture is needed —
both D1g's and D1j's raw samples are already saved.

**D1k phase/n_min boundary fixed; D1g and D1j now agree on first divergence**
([check](docs/scratch/phase-d1-phase-nmin-fix.md)): `n_min` (which tick-grid point is
"tick 1") was a sign test on a per-capture `phase` value that is always within half a
tick of exact zero by construction, with zero margin at the test's own boundary.
Replacing it with a nearest-grid-point choice (`round(-phase/TICK_SECONDS)`, provably
always `0` under the current `phase` definition, independently confirmed over 200,000
sampled values) makes D1g's and D1j's captures agree exactly on first divergence (tick
11, `released_count`, sim=1 live=0) where they previously disagreed in both tick and
field. **D1g's own historical figures change as a direct, flagged consequence**: its
167-total/tick-10 result becomes 653-total/tick-11 under the corrected grid — the old
number measured a mis-aligned grid, not a better one. The raw-count gap between the two
captures (653 vs 537) persists even with aligned grids, consistent with D1j's
already-landed finding that count tracks propagation length, not drift magnitude; this
is not re-litigated. **D1 is still not accepted.** Both known harness biases (D1i/D1j's
throttle lag, D1k's grid alignment) are now fixed; the next D1 round is a fresh, clean,
full live gate recheck against the fully-fixed harness.

**D1l: first full gate measurement with both harness biases fixed — does not pass, every
residual named**
([check](docs/scratch/phase-d1-full-gate-recheck-fixed-harness.md)): a fresh live
capture reproduces the predicted tick 11/`released_count` first divergence and the
≤1-tick spawn-drift floor exactly (3/10 lemmings, magnitude 1). **New finding**: the
faller→walker landing tick drifts −1 or −2 ticks for **all 10 lemmings**, including the 6
with zero spawn drift — a second, previously-unisolated discretization-floor mechanism
(a continuous landing event quantized onto the 200 ms tick), not the same quantity as
spawn-tick drift though structurally analogous. Of 657 total divergences in the 150-tick
window, every one is attributed: 14 to the already-parked `facing`/turn-rounding family
(Phase E), 643 to forward propagation from spawn-tick drift (3/10 lemmings) or
landing-tick drift (6/10 lemmings) — **0 unexplained**. Read against the gate's literal
text, every named field (X, Y, facing, state) and the released-count clause shows at
least one disagreement; order does not. **D1 does not pass, and every residual is now a
named, bounded, deterministic discretization-floor or already-parked quantity — none is
an open, untraced defect.**

This is recorded as a pending question for the user (`docs/tasks.json`'s
`standing_run.pending_questions`), not decided here: whether to amend D1's gate to
exclude this now-fully-characterized discretization-floor class of residual (the same
shape of amendment already granted to A7 for HUD elements), to invest in closing it
outright (which would mean re-deriving the sim's spawn/landing timing at sub-tick
precision or changing the tick architecture — not a bounded round), or to leave D1
parked. The standing run stops here per
[its gate-change stop condition](docs/ORCHESTRATION.md#standing-run).

**D1 direction after D1l (user, 2026-10-21, CEO review): gate held as written, not amended.**
Review observation, a hypothesis and not a measurement: D1l's own raw 50 Hz samples look
continuous, not stepped. X changes on 7224 of 7464 consecutive 20 ms samples. The walk-out
fits `32 + 5.0·(t − land)` within −0.022..+0.115 over 2684 samples. The fall at 20.0 u/s from
Y 42 lands at release + 0.50 s for 10/10 lemmings. On its spawn tick our `World` has already
fallen one full 4-unit step. Re-run with `python build/tmp/ceo-2026-10-21/continuous_fit.py`.
If the original integrates real `dt` per frame, as D1d already found for the hatch, then
"discretization floor" is the wrong name. The residual is a model mismatch, and the fix is
the sim's clock, not the gate. Neither the "5 Hz, measured three ways" lesson nor any D1
round has tested a fixed step against a continuous one: the speeds are 5.0 and 20.0 per
second, and a fixed 0.2 s step turns them into 1 and 4. **D1m** reads the lemming step's
`dt` source and replays the saved captures with a zero-parameter continuous model. If the
answer is continuous, D1n changes `World`'s clock. If it is stepped, the gate question goes
back to the user with D1m's evidence.

**D1m: continuous, on both a structural read and a behavioural replay**
([check](docs/scratch/phase-d1-stepper-clock.md)): the walk stepper (`0x0041BDD4`) takes
its time budget as a mutable, re-checked, byref double that it can spend across more than
one lattice column per call and writes the decremented remainder back through the same
pointer — a shape with no reason to exist under a fixed 0.2 s step, independent of any
live measurement. A zero-fitted-parameter continuous replay (release, fall 20.0 u/s, land,
walk 5.0 u/s, no wall-turn logic) against all three saved captures (D1l, D1j, D1g)
collapses the `y`-field mismatch count 76–100% (29→4, 28→0, 10→1) — the landing-tick-drift
floor D1l isolated as its own mechanism nearly disappears once fall/land timing is modelled
continuously — while the full-150-tick total is honestly worse (the model was deliberately
given no wall-turn logic, as scoped). The re-examined "5 Hz, measured three ways" lesson
does not survive: two of the three rest on the retracted billboard-quad method and the
third measured the hatch, not the lemming (full correction in
[LESSONS.md](docs/LESSONS.md#the-simulation--measured-and-worth-keeping)). Open, disclosed
gap: the exact caller chain from the frame dispatcher to the Lemming's own stepper was not
traced within budget; the verdict rests on the stepper's own loop shape, not a completed
chain. **D1 is still not accepted.** Per the pre-authorized branch, the next round is
**D1n**: a bounded `World`/`SimReleaseTrace` clock change, re-scored offline against all
four saved captures, before any live recheck — not a re-ask of the discretization-floor
gate question, since a confirmed model mismatch calls for fixing the model.

**D1n: implemented, partial result — landing-tick drift reduced, not closed; spawn-tick
drift unchanged, now structurally explained** ([check](docs/scratch/phase-d1-continuous-stepper-clock.md)):
an additive, opt-in continuous sub-tick clock (`LevelSetup::continuousStepperClock`,
default `false`, byte-for-byte old behaviour when off — confirmed by `diff -q` against each
capture's own checked-in `sim_trace.csv`) lets a landing faller hand its unconsumed
real-seconds budget to the walker it becomes within the same tick. The landing-tick-drift
family D1l isolated **shrinks substantially on D1g/D1j/D1l**: roughly half of the
previously-drifting lemmings now land on the exact tick, the rest from magnitude up to 3 to
at most 1 (aggregate totals more than halve: 653→303, 537→183, 657→302). **D1a gets
nominally worse** (162→514) — explained, not papered over: D1a's own live capture predates
the already-landed D1i/D1j harness-throttle-lag fix, so it is the one capture whose own
ground truth is independently known to be mistimed; a more temporally faithful sim
naturally diverges more from a mistimed reference. Spawn-tick drift is **unchanged on every
capture**, for a now-understood structural reason: `spawn()` fixes which tick a lemming
exists on before the budget it receives is even computed, so no fall-budget reduction can
move it. A real bug in the round's own first draft (an unguarded landing handoff that
regressed D1a before the `hasTrackedPhase` guard was added) is disclosed in the check, not
hidden. 27/27 tests pass (4 new, 0 edited); the facilitator's own mutate-one-alone (a
one-line leftover-budget off-by-one) was not caught by the unit suite but was caught by the
offline capture comparator (252 of 1500 rows diverge), confirming that comparator — not the
unit suite alone — is the real discriminator for this class of change. **D1 is still not
accepted.** Whether this partial, honestly-measured improvement changes the calculus on the
user's pending D1-gate-amendment decision (`docs/tasks.json`
`standing_run.deferred_questions`) is for a future round or the user to weigh — not decided
here.

### D2. Click-to-assign

**D2a: the pick rule read and ported, round 1** ([check](docs/scratch/phase-d2-pick-rule.md)):
`src/app/Pick.{h,cpp}` ports the original's in-level cursor-to-lemming pick
(`0x00454D33`/`0x00454CB9`/`0x00455F20`/`0x0045A0E0` chain, `0x00454D6C`'s full
accept/skip/tie-break body). **Falsifies the brief's own hypothesis**: the tie-break is
**priority (desc, from a per-state category table) then squared distance (asc, strict)**,
not list order — list order only breaks an exact tie on both keys, keeping the earlier
node. All four box edges are inclusive (`<=`/`>=` with the cursor's own ±4px adjustment
already folded in), confirmed by re-disassembling `0x00454CB9` directly, not by trusting
the cited old note's prose. Two skip gates not in the brief's "already settled" list were
found and excluded from the ported function with reasons: a facing-filter UI pair
(`screen+0x16b1`/`+0x16b2`) and the skill-stock validity gate (`0x0045A0E0`) — both depend
on runtime UI/skill state, not lemming-vs-cursor geometry, so D2(a)'s own geometry gate
does not model them; the brief explicitly named the skill-stock gate as something to
report, not omit silently. Two upstream widget-hit-test loops (lists `0x4C88F8`/`0x4C8908`)
can suppress the pick before the lemming loop even runs if a UI panel is hit first —
flagged for the D2(a) live capture to avoid. Two genuine gaps, reported not guessed: the
priority-category table's contents (`0x004C05D0`) and what `screen+0x58` holds (the
distance reference point) are both runtime facts this round's static reads cannot settle.

**D2b: 20 live cursor positions, round 2** ([check](docs/scratch/phase-d2-pick-capture.md)):
pid 39764 reached `Compiled\MM_CLIMB_002.lvl` after the highlight scan found one
candidate. Twenty live sprite-box positions give 19 agreements with
`[screen+0x16D8]` and 1 disagreement. Sample 15 overlaps two lemmings, both
state vtable `0x004b0d28`, both passed to the port with priority 0 and distance
0; the port keeps index 2 and the original stores index 6. Eleven positions
pick none and nine name a lemming. An earlier start, pid 47392, never left the
main menu and took no rows. D2 part (a) does not pass. The priority table and
`screen+0x58` remain the open tie keys. Part (b) was not run.

**D2c: tie-key operands, round 1** ([check](docs/scratch/phase-d2-tie-keys.md)):
the priority load at `0x004550CC` indexes `0x004C05D0` with `[state+0x14]`,
and `state` is `[lemming+0x340]`, not the vtable. The distance reference is
the three doubles at `screen+0x58`, `+0x60`, and `+0x68`, subtracted from
`lemming+8`. The walker constructor writes index `0x12` beside vtable
`0x004B0D28`, and that file dword is 0. Equal vtables are not forced to share
a priority. Sample 15's winner is still open, because the live field and the
positions were not saved. D2 part (a) does not pass.

**D2d: one live overlap with the real tie keys, round 1**
([check](docs/scratch/phase-d2-tie-live.md)): one cursor on `MM_CLIMB_002`
overlaps two walkers. Both have `[state+0x14] = 0x12` and priority 0, live and
in the file. The reference at `screen+0x58` is `(32.0, 48.0, 0.5)`. Squared
distance selects index 3 (`698.64`) over index 2 (`1070.03`), and index 3 is
the lemming at `[screen+0x16D8]`. D2 part (a) still does not pass: this is one
overlap, not the 20 positions, and it reads only table index `0x12`.

**D2e: 20-position rescore attempt, clean negative — the stock exe crashed before the
scan, round 1 of 3** ([check](docs/scratch/phase-d2-rescore-crash.md)): a new
`measure_d2_rescore.py` feeds every eligible candidate's real live priority and real
squared distance (never a forced 0) into the landed `pickHoveredLemming` through a
driver built from an unedited HEAD snapshot of `Pick.{h,cpp}`. The one authorized live
start (pid 23616) reached the main menu, then died with an access violation
(`0xC0000005`) while the harness was acquiring foreground for the first menu `Down` —
before any key was sent, before the level was entered. No retry, per the brief's
"no second Popen." No 20-position data was collected; D2 part (a) remains exactly where
D2d left it. Restoration confirmed (process gone, all three install hashes unchanged,
independently re-verified).

**D2e repair 1: second crash, earlier in the sequence, round 2 of 3**
([check](docs/scratch/phase-d2-rescore-crash.md)): same unmodified script, same offline
self-tests reproduced exactly. The one authorized live start crashed again with the same
`0xC0000005` exit code, this time *before* the harness ever attached to the process
(during the fixed boot wait), earlier than round 1's crash (which reached the main menu
and died during foreground-acquisition polling). No data collected; restoration again
clean. A facilitator-only, read-only event-log/WER pass found no crash report for either
specific incident, a long pre-existing history of this exe crashing on this machine in
unrelated prior sessions, and the host cycling logon/sandbox-restart events roughly
every 40-70 seconds through both crash windows -- an unproven environmental correlation,
not a diagnosed cause. Per the round cap, the worker correctly did not self-dispatch a
third attempt. D2 part (a) still does not pass.

**D2e repair 2: third attempt reaches the level; 20/20 agree; D2 part (a) accepted**
([check](docs/scratch/phase-d2-rescore-crash.md)): same unmodified script, round 3 of 3
(the question's last pre-authorized round). No third crash -- the one authorized start
attached immediately, entered `MM_CLIMB_002` on the first try, and filled all 20 rows
with zero discards: 10 original-none, 4 two-overlap rows, **20/20 agreement** between
the landed `pickHoveredLemming` (fed live priority and live squared distance) and the
original's own stored pick. The facilitator independently reproduced this from scratch
with a standalone checker that imports nothing from the project's own harness or ported
code, empirically re-deriving the exact overlap formula (the cursor box inset 4px on
every edge, then an inclusive AABB test) before it matched all 20 rows exactly -- not
merely re-running the worker's own driver. This also caught and narrowed an error in the
round's own report: a nonzero live priority dword (`2`, at `state+0x14 = 19`, matching
the file table exactly) **was** read this round, on three candidates that simply never
overlapped a sampled cursor box -- the first nonzero live priority value seen in the
entire D2 investigation, though the priority-beats-distance tie-break branch remains
untested live. **D2 part (a) is accepted.** D2 part (b) is untouched and remains open.

**D2f: one live climber assignment, 45/50 ticks match, one bounded residual found**
([check](docs/scratch/phase-d2-skill-assign-capture.md)): the first round of D2 part
(b), climber only. Reused already-proven tooling throughout (`assign_skill.py`'s panel
click, `measure_d1_release_trace.py`'s sampling) for a live climber assignment on
`MM_CLIMB_002`, then compared the assigned lemming's 50-tick post-assignment trace
against a fresh `sim::World` seeded at the live-measured cell via the already-existing
`World::place()`. **45 of 50 ticks match exactly** (the full walk-approach and the full
14-tick climb run, X/Y/facing/state all exact), independently reproduced by the
facilitator from a from-scratch recompile and relink, byte-identical output. The one
divergence: our `World`'s climb stepper stays in `State::Climber` one tick longer than
the original at the topout cell before handing off to `State::Hoister` — both reach the
same cell at the same tick, but only the state label lags by one tick, which then
propagates as a constant one-tick shift for the remaining captured rows (hoist duration
and post-hoist walk speed are both still exactly right). `climbSpeed`/`climberTopOut`
are independently reconfirmed live, unaffected. D2 part (b) is not yet settled (one of
four skills attempted; this skill has an open residual). Queued next: a disassembly
read of the native climb/hoist handoff ordering, offline-only (no new live capture
needed — replays round 1's own saved trace), to confirm and fix the one-tick lag.

**D2f repair 1: the one-tick lag read with bytes and fixed; climber now 50/50**
([check](docs/scratch/phase-d2-skill-assign-capture.md)): offline only, zero stock
starts. Disassembly of `0x004137CA` (the climb stepper's outer loop) and `0x004139BF`
(the check function it calls) shows the native stepper moves the lemming's Y
(`0x0041388B`, the write-back) **before** calling the check function, which reads that
already-moved position at entry (`0x4139DD`→`0x42F35F`) for both the ceiling-hit and
topout probes, and installs the Hoist state (`0x417C80`/`0x42E696`) inside that same
call — move, check, and transition all happen in one call, same tick. `World.cpp`'s
`stepClimber` is fixed to match: move first, then check. A new unit test
(`testClimbTopsOutSameTickAsHoistHandoff`) fails against the old ordering and passes
after; one existing test's literal (`lastClimbingY`, `41`→`42`) changed as a named,
explained consequence, not a silent edit. Re-checked against round 1's own unmodified
live capture, offline: **50 of 50 ticks now match exactly** (was 45/50).
**Correction, beyond this round's own question**: this also settles the ceiling-hit
check's timing as post-move, correcting `old/docs/scratch/phase3-round10-steppers.md`'s
"tests the ceiling first" characterization — that note did not have this call-graph
split in view. Climber is now a full match; D2 part (b) overall remains open (floater,
blocker, digger untested).

**D2g: one live blocker assignment, 50/50 ticks match on the first attempt**
([check](docs/scratch/phase-d2g-skill-assign-blocker.md)): second of four D2(b) skills,
reusing D2f's exact pattern. On `MM_BLOCK_001_A`, panel index 3, one live assignment
holds the sampled lemming frozen at `(137, 14, 1, blocker)` for all 50 ticks, matching
an offline `sim::World` seeded via `World::place()` + `assign(0, Skill::Blocker)`
exactly, no repair needed. `tools/harness/measure_d2b_skill_trace.py` gained
`--skill`/`--skill-name` options (purely additive; climber's prior default behaviour is
unchanged). Blocker is a full match; floater and digger remain untested.

### D3. One retail level end to end

**D3a: production retail-level loader, round 1** ([check](docs/scratch/phase-d3-retail-level-setup.md)):
`webo::sim::loadRetailLevel`/`loadRetailLevelReport` (`src/sim/RetailLevel.{h,cpp}`) build
a `LevelSetup` for any retail level from the archive alone — terrain, entrances, release
timing, exits/teleports/speed-pads/levers, totals and skills, each from an already-tested
reader. **All 102 retail levels load**, matching a prediction written before coding. D1b's
open "no retail rate falls outside its clamp" is now checked and holds (0 of 144 hatch
records across all 102 levels). `SimReleaseTrace` rebuilt on the new loader is
byte-identical to its pre-change output on D1g's own invocation (sha256
`7653aedb4108d06aa0ad8026a70df645fe60273638db358638f2368ee7dde664`, both before and after).
Named loss, not hidden: 33 of 102 levels have more than one hatch, but `LevelSetup` carries
one shared release-timing triple — every hatch's own position still becomes an `Entrance`,
and every hatch's own rate/min/max is still read and reported, but only the first hatch's
cadence drives the shared accumulator. `releaseArmSeconds` (`0.723061695938398`) stays the
MM_CLIMB_002-only DERIVED constant for every level, per the brief's own instruction, not
re-derived elsewhere. Deliberately no `old/src`-style guessed fallback for a hatch-less or
exit-less level — none exists in the retail corpus, so this path is untested because it
never fires, not because it was masked.

**Facilitator check on D2a and D3a (2026-10-21):** independently re-read the cited
addresses against the installed exe — `0x00454CB9` (the whole box-overlap-test function,
byte-for-byte match) and `0x004551E4`-`0x00455203` (the priority/distance tie-break,
byte-for-byte match). Independently confirmed the new hatch tail-word offsets
(`src/assets/LevelObjects.cpp`'s `kTailAt = kTripleAt + 6`, i.e. hatch+0x9A+6 = +0xA0,
+0xA4, +0xA8) match D1b's already-landed offsets exactly. Ran one isolated
mutate-one-alone test per round, each rebuilt and re-tested in the full tree (both rounds
landed into the same working copy): swapping `RetailLevel.cpp`'s entrance `e.x`/`e.y`
assignment is caught by `test_retaillevel`'s MM_CLIMB_002 check (`FAIL  entrance at
hatchSpawnCell (32, 53) (got (53, 32))`); loosening `Pick.cpp`'s left-edge comparison from
`<=` to `<` is caught by `test_pick`'s exact-boundary-hit case (`FAIL: left edge:
cursor.right-4 == lemming.left is still a hit`). Both files restored to their
worker-delivered SHA-256 and a fresh full rebuild plus `ctest --preset dev-debug` passes
**26/26** (both `retaillevel` and `pick` included), confirming the two concurrently
dispatched rounds' shared `CMakeLists.txt`/`cmake/webolution_sources.cmake` edits merged
without conflict, exactly as their dispatch note anticipated. No Lemmings Revolution
process ran for either round. D3b (app sim-loop integration, depends on D3a) and D1n
(World clock change) do not share an owned file and may now run concurrently.

**D3b: `sim::World` ticking inside `webolution`, round 1** ([check](docs/scratch/phase-d3-app-sim-loop.md)):
`webolution --sim`/`--sim-ticks <N>` (off by default) builds a `World` via D3a's
`loadRetailLevel` and draws its live lemmings through the existing
`drawInjectedWalkers`/`terrainSampleFromLemming` path (`TubeScene::injectedWalkersFromWorld`,
one entry per `alive()` lemming). A new `SimClock` (`src/app/SimClock.*`) drives the
windowed loop's ticking off real time, independent of the 144 Hz camera clock, capped at 10
ticks (2.0 s) per frame with any backlog carried, not dropped. Every pre-existing command
line renders byte-identically (A6 and A7 PNG hashes unchanged); `--sim --sim-ticks N` at
N = 0, 15, 75 matches an independent `SimReleaseTrace` run on the same level exactly. Named
limitation, reported as its own prediction anticipated: `drawInjectedWalkers` only has a
`"walk"` sprite action, so non-`Walker` alive states (10 of 181 sampled live-lemming-ticks
were `Faller`) reuse that same strip at a fixed phase — no sprite invented, no animation
clock wired to `World::tick()`. Ran concurrently with D1n (file-disjoint); its own checks
were relative (before/after the same build), not against a frozen `World.cpp`. 27/27 tests
pass. D3c (click-to-assign in the app) depends on this round and D2a and may now be queued.

**D3c: click-to-assign wired offline, round 1** ([check](docs/scratch/phase-d3-click-to-assign.md)):
`TubeScene::injectedWalkerScreenBoxes` exposes the sprite box `drawInjectedWalkers`
already computes, and `test_clicktoassign` selects lemming 0 from that box and
selects nothing from the same box shifted +10000 in x. Tick 25 on `MM_CLIMB_002`
has two alive lemmings. The windowed `--sim` path calls `World::assign` on a
left click, with keys 1–8 as a debug skill stand-in, not the original panel.
That window path was not executed. A6 and A7 PNG hashes are unchanged and
`--sim-ticks` 15 and 75 stderr are unchanged. 28/28 tests pass. This does not
grade D2's or D3's gate. The live 20-cursor comparison remains D2b.

## Phase E — the remaining debts

Short, and explicitly parked rather than cancelled. **The gadget backlog is gone** — rounds
13–31 closed every one, including eleven clean negatives where a cue turned out not to be a
gadget at all. See the gadget table in `docs/LESSONS.md` before assuming anything here is
open.

- **D1 single-tick `facing`-label residual** (parked 2026-10-21 after D1h). Both named D1
  blockers (release timing, D1a-e; the left-turn tick, `0x0041C67E`, D1f — the line
  formerly here listing it as open is retired) are fixed and independently verified. One
  smaller residual remains: a self-healing, single-tick `facing`-only mismatch at turn
  instants, found on a fresh live capture (D1g) and fully characterized by D1h — every
  instance traces to the live-capture harness's one pooled global phase differing from
  that lemming's own measured phase (D1a's own "Open limitation"), not to a defect in
  `World`; `x` is exact on every tick before and after, and an independent full-trace
  scan found zero reappearance of the stall D1f fixed. Closing it fully would need
  sub-20 ms live sampling or a per-lemming-aware harness (a tooling investment, not a sim
  fix) and needs its own gate justification before a future round is spent on it. D1's
  gate, as literally written ("every lemming's cell X/Y, facing and state match... each
  tick"), does not pass with this residual present; whether to amend the gate to exclude
  a self-healing, x-unaffected, harness-precision-bounded label mismatch, or to invest in
  harness precision to close it outright, is a user decision (`standing_run.pending_questions`),
  not a gate change made unilaterally by the facilitator. Evidence:
  [D1a](docs/scratch/phase-d1-release-walkout-trace.md),
  [D1b](docs/scratch/phase-d1-release-trigger.md),
  [D1c](docs/scratch/phase-d1-door-open-duration.md),
  [D1d](docs/scratch/phase-d1-hatch-step-clock.md),
  [D1e](docs/scratch/phase-d1-release-timing-implementation.md),
  [D1f](docs/scratch/phase-d1-left-turn-fix.md),
  [D1g/D1h](docs/scratch/phase-d1-full-gate-recheck.md).
- 14 of the 19 call sites of the object-overlap predicate `0x00430BA1` are still
  unidentified. Anti-grav, exit, teleport, SpeedPad and lever account for five.
- Panel indices 2, 5, 6, 8 and 9 (jump table at `0x0045A4DD`). Needed for Phase C.
- Platform targets: Linux, macOS, WebAssembly. The old GLES3/WebGL2 claim was a code
  property that was never actually run.
- Intro, music and FMV. The `.avi` codecs are still unidentified.
- **A7 lower-band cause** (open after A7l: the scale integer is a runtime tick).
  B=`[0,432,640,448)` has stable E302804 over maskN3491. Removing every B
  error would reach 0.0198391255 on the one scored view; eight other views
  and a second level still have no mask. `[G+0x25C64]` is either
  `0x4657D1([G+0x557C]) - [G+0x25C60]` or a conditional `100`. `0x4657D1`
  calls `[0x4B0220]` and subtracts `[ecx+8]`. A paused read of that dword
  can place the rectangle; it cannot finish A7 while the other masks are
  missing. Do not repeat the writer scan. Evidence:
  [A7a](docs/scratch/phase-a7-wider-mask-localize.md),
  [A7b](docs/scratch/phase-a7-lower-band-characterize.md),
  [A7c](docs/scratch/phase-a7-pause-caption-native.md),
  [A7d](docs/scratch/phase-a7-draw-loop-caption-bridge.md),
  [A7e](docs/scratch/phase-a7-base-list-producer.md),
  [A7f](docs/scratch/phase-a7-caller-dest-offset.md),
  [A7g](docs/scratch/phase-a7-25c68-to-h.md),
  [A7h](docs/scratch/phase-a7-pause-list-draw.md),
  [A7i](docs/scratch/phase-a7-rect-26000.md),
  [A7j](docs/scratch/phase-a7-25ff8-stores.md),
  [A7k](docs/scratch/phase-a7-1c968-source.md),
  [A7l](docs/scratch/phase-a7-25c64-writers.md).
- **Actor filter/alpha state at zoom 1** (parked 2026-10-20 after seven rounds, A4z–A4af).
  The full-scale zoom 1 fixture fails (27 candidate-only pixels, support MAE 0.0284529
  against 0.0151558); A4af READ that nonzero zoom skips the pre-draw `0x470F29(0)` and
  draws under the last `0x470F29(1)` (LINEAR, `0xA0`). Resume with the written offline
  replay brief `build/tmp/a4ag-zoom1-linear-a0-offline-replay-brief.md`, only if the A6
  gate fails at zoom 1. Evidence: [A4z](docs/scratch/phase-a4-actor-third-fixture.md),
  [A4aa](docs/scratch/phase-a4-zoom1-filter-provenance.md),
  [A4ae](docs/scratch/phase-a4-pre-actor-subcalls-static.md),
  [A4af](docs/scratch/phase-a4-tube-child-renderers-filter-static.md).
- **A6d3 slope walker clause 2** (parked 2026-10-20 when A6 was accepted on the repair 4 pause).
  Actor 2 at `+0x08` `(32, 34.4796, 0.5)` scores 0.10089786756453423 tube-only versus
  0.11246451442529874 actors-on. View, temporal and `+0x1D8` (A6d4–A6d6), pose, screen Y
  (A6d7, A6d20–A6d23) and first row (A6d17–A6d19) are all parked. `originY − 1` beats the
  box but is a pixel fit with no address; do not ship it. Resume only with a new native read
  of how a slope (non-integer Y) walker is drawn, or when a slope walker is needed by a later
  gate. Evidence: [A6d3](docs/scratch/phase-a6-matched-state-bbox.md),
  [A6e](docs/scratch/phase-a6-restated-gate-rescore.md),
  [repair 4](docs/scratch/phase-a6-fresh-pause-pixel-result.md).
- **A6 sim → `+0x08` mapping** (parked 2026-10-20 after A6d10–A6d12).
  Motion stores, hatch spawn `(+0x9A,+0x9C,0.5)`, and fall Y
  `0x41627F` = `0x40DF50([+0x10] − [+0x44]×step)` are READ. No complete
  sim mapper. Resume with first-tick arm order, walk remainder, or Z 0.75
  init — not another hatch `+0x9A` pass. Evidence:
  [A6d10](docs/scratch/phase-a6-sim-plus8-writer.md),
  [A6d11](docs/scratch/phase-a6-plus8-spawn-init.md),
  [A6d12](docs/scratch/phase-a6-plus8-first-y.md).
- **A6 screen-Y bias** (parked 2026-10-20 after A6d20–A6d22).
  Incoming `0x47A75D` Y is `cy − C0/C8` with no ±1. Render `+0x90`
  is the 24-byte `0x43D56C` result. `0x4C6B88` is a 96-byte copy of
  the `0x58D5A0` stack top (`0x46BE99`), not a second proven miss
  in `worldToLensedView`. `originY − 1` still beats actor 2 and is
  not a native store. A6d23 completed the bounded live-matrix comparison:
  compiled production versus cached screen Y is within 1e-4 px on three
  actors at a fresh canonical-camera pause; no screenshot or A6d3 attribution.
  The native 0.0003 subtraction stores back to render +0xC8 through an alias.
  Re-park; resume only with new paired draw/pixel evidence exposing a projection
  discrepancy, not another static writer search or fitted offset. Evidence:
  [A6d20](docs/scratch/phase-a6-screen-y-bias.md),
  [A6d21](docs/scratch/phase-a6-plus90-vector.md),
  [A6d22](docs/scratch/phase-a6-xform-4c6b88.md),
  [A6d23](docs/scratch/phase-a6-native-camera-pair.md).
- **A6 render-clock dt / `[+0xC8]`** (parked 2026-10-20 after A6d13–A6d15).
  `[game+0x26028]` is a wrap phase. `0x42F0DC` is `0x43CC68`, which
  consumes `[this+0xC8]` as `1.0 / C8` UV scale after `0x40EAE0`
  writes the `+0xB8/+0xC0/+0xC8` triple. That is not the A6b clock
  (`vtable+0x2C` → `0x43CF30` → `0x468833`). Keep the integer-ms
  stand-in. Incoming seconds to `0x42E2BF` stay unread; resume only
  if a temporal change is shown to move the A6 gate. Evidence:
  [A6d13](docs/scratch/phase-a6-render-clock-dt.md),
  [A6d14](docs/scratch/phase-a6-walk-budget-ptr.md),
  [A6d15](docs/scratch/phase-a6-render-c8-dt.md),
  [A6b](docs/scratch/phase-a6-actor-anim-clock.md).
- **A6 first-row `floor` vs `ceil`** (parked 2026-10-20 after A6d18–A6d19).
  No `fistp` on submit. `floor(top)` is pixel-identical to `ceil` on the
  A6d3 matched frame and fails the A4v y=2 fixture. Initial-view boxes
  move and still fail. Resume only with a float screen-Y term, not
  another cover bound. Evidence:
  [A6d18](docs/scratch/phase-a6-billboard-top-row.md),
  [A6d19](docs/scratch/phase-a6-first-row-floor.md).
- **A6 per-pixel actor Z versus tube** (parked 2026-10-20 after A6d16).
  Submit path has no actor-vs-tube fragment compare. Closest reject is
  A4y `ALPHAREF 0xEF`. Wrapper depth stays A1 (not an exe read).
  Resume only if wrapper depth is shown to move the A6 gate. Evidence:
  [A6d16](docs/scratch/phase-a6-actor-tube-pixel-z.md),
  [A6c](docs/scratch/phase-a6-actor-tube-composition.md),
  [A1 depth](docs/scratch/phase-a1-depth-clipping.md).
- **A6 actor-2 sprite at matched pause** (parked 2026-10-20 after A6d4–A6d9).
  Temporal 0–11 cannot pass; live `+0x1D8` is 0.0; native view/mirror match
  `selectSpriteView`. −0.125 is a discarded pixel fit. Pose pair matches
  live `+0x108`. Before-tube is fully occluded. Overlay `> 0xEF` is the A4y
  READ and is pixel-identical to `a != 0` on this 0/255 walk path. Resume
  only with a new lead (not chooser, Z-order, overlay, or blend). Evidence:
  [A6d4](docs/scratch/phase-a6-actor2-temporal-sweep.md),
  [A6d5](docs/scratch/phase-a6-live-phase-1d8.md),
  [A6d6](docs/scratch/phase-a6-live-view-index.md),
  [A6d7](docs/scratch/phase-a6-live-pose-pair.md),
  [A6d8](docs/scratch/phase-a6-actor2-z-order.md),
  [A6d9](docs/scratch/phase-a6-overlay-alpha-ef.md).
- **Actor basis precision** (parked 2026-10-20 after three A6a rounds). The facing basis
  misses its 1e-6 falsifier by up to 2.962e-5. Native values are float32 and the chain
  `0x43D56C`/`0x401830`/`0x480AC8` is not replayed. It is invisible unless a view index sits
  on a sector boundary. Resume only if the A6 gate shows a view-index mismatch. Evidence:
  [A6a](docs/scratch/phase-a6-actor-pose.md).
- **Frida hook tooling for live render-state traces** (parked with the above). A4ab's live
  trace crashed the stock game in D3DImm; A4ac/A4ad repaired the binder and stack layout
  offline only, never re-run live. Evidence: [A4ab](docs/scratch/phase-a4-zoom1-filter-trace-failed.md),
  [A4ac](docs/scratch/phase-a4-actor-binder-offline.md),
  [A4ad](docs/scratch/phase-a4-hook-stack-announce-offline.md).

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
