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

- **A6a pose and basis** — actor `+8` to model position `+0x108` (`0x43D56C`) and the
  `facing·0.1` basis, as an adapter from `sim::Lemming`. Queued.
- **A6b animation clock** — the `0.06` temporal advance at controller `+0x1CC` and its
  reset; a render clock separate from `Lemming::frame`. Queued.
- **A6c composition** — where the actor loops `0x43A9BB`/`0x43AA47` sit in the tube
  walker's order and what depth state they draw under. Queued, static.
- **A6d integration** — wire World → pose → selection → clock → placement → raster into
  `main.cpp`, composited with the tube per A6c. Briefed after A6a–c land.

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
