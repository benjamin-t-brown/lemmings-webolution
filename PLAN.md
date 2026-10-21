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
end in 3D is the next visible result.


## Phase B — the frontend

The original's own screens: backgrounds, buttons, fonts, `.LTX` localised text (the menus are
German), the screen state machine, level select and the map. Preserve the quirks —
**RETURN only when the menu highlight is exactly 0**, and a short ESC that fails to leave
pause. Frontend screen ids are already known (`0x004C416C`: 3 menu, 4 confirm, 5 map).

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

## Phase D — reconnect the simulation

Wire spawn, exits, skill assignment and the measured gadgets into the new renderer, and get a
retail level playable end to end in 3D. Starts as soon as A6 passes; it runs alongside A7
and comes before B and C.

## Phase E — the remaining debts

Short, and explicitly parked rather than cancelled. **The gadget backlog is gone** — rounds
13–31 closed every one, including eleven clean negatives where a cue turned out not to be a
gadget at all. See the gadget table in `docs/LESSONS.md` before assuming anything here is
open.

- The left-facing turn tick (`0x0041C67E`), entangled with the blocker-turn expectation, so
  the two must be fixed together.
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
