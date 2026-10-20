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
| **A6 sim lemmings in the app frame** | **next** | See below |
| A7 scene completeness | open | See below |

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
- **A6d integration** — **started** ([compose](docs/scratch/phase-a6-injected-walkers.md),
  [original-0](docs/scratch/phase-a6-app-bbox-gate.md),
  [matched pause](docs/scratch/phase-a6-matched-state-bbox.md)): walkers draw after
  the tube and inject the live `+0x08` table. Walk temporal does not fix the
  slope walker. Live `+0x1D8` is **0.0** ([A6d5](docs/scratch/phase-a6-live-phase-1d8.md));
  clause 2 still **fails** actor 2 at native phase (0.101→0.112). Native view
  matches ours ([A6d6](docs/scratch/phase-a6-live-view-index.md)). Pose matches
  live `+0x108` ([A6d7](docs/scratch/phase-a6-live-pose-pair.md)). Overlay
  composite is A4y `> 0xEF` ([A6d9](docs/scratch/phase-a6-overlay-alpha-ef.md));
  actor 2 is unchanged (0.101→0.112) because the walk overlay is already 0/255.
  Remaining: full-frame 0.02. Actor 2 still fails clause 2
  (0.101→0.112). Screen-Y bias is parked after three rounds:
  incoming Y has no ±1, `+0x90` is the `0x43D56C` triple, and
  `0x4C6B88` is the per-draw stack-top copy
  ([A6d22](docs/scratch/phase-a6-xform-4c6b88.md)).
  `originY−1` stays an unshipped pixel fit
  ([A6d17](docs/scratch/phase-a6-actor2-bbox-pixels.md)).
  `floor(top)` does not move the matched
  frame ([A6d19](docs/scratch/phase-a6-first-row-floor.md)).
  Float top is `0x47AB30`; no integer first row
  ([A6d18](docs/scratch/phase-a6-billboard-top-row.md)).
  `originY−1` is a pixel fit — do not ship
  ([A6d17](docs/scratch/phase-a6-actor2-bbox-pixels.md)).
  full-frame 0.02. Per-pixel actor Z versus tube is a
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

**Exit gate (fixed now, not after capture):** on `MM_CLIMB_002`, a paused original frame
with at least three walkers at the A1 canonical camera, against ours with the same actor
list state injected:
1. the full 640x480 frame passes the A4d fixed metric (≤ 0.02), and
2. in each actor's bounding box, ours-with-actors has lower error than ours-without-actors.

Residuals that leave both conditions passing are parked, not chased.

### A7. Scene completeness

The A2 wider/cavity/rim failures (`0.022030839 > 0.02`), the upper gold band, and the
omitted scenery and object models (entrances, exits, gadgets). **Exit gate:** the A2
wider changed-pixel diagnostic ≤ 0.02 on the nine retained views and on one second level.

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
- **Actor filter/alpha state at zoom 1** (parked 2026-10-20 after seven rounds, A4z–A4af).
  The full-scale zoom 1 fixture fails (27 candidate-only pixels, support MAE 0.0284529
  against 0.0151558); A4af READ that nonzero zoom skips the pre-draw `0x470F29(0)` and
  draws under the last `0x470F29(1)` (LINEAR, `0xA0`). Resume with the written offline
  replay brief `build/tmp/a4ag-zoom1-linear-a0-offline-replay-brief.md`, only if the A6
  gate fails at zoom 1. Evidence: [A4z](docs/scratch/phase-a4-actor-third-fixture.md),
  [A4aa](docs/scratch/phase-a4-zoom1-filter-provenance.md),
  [A4ae](docs/scratch/phase-a4-pre-actor-subcalls-static.md),
  [A4af](docs/scratch/phase-a4-tube-child-renderers-filter-static.md).
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
  not a native store. Resume only with a live compare of that
  stack top to our matrix on the matched camera. Evidence:
  [A6d20](docs/scratch/phase-a6-screen-y-bias.md),
  [A6d21](docs/scratch/phase-a6-plus90-vector.md),
  [A6d22](docs/scratch/phase-a6-xform-4c6b88.md).
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
