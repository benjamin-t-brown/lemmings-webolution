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

**Two debts this created, both deliberate and both to be paid in Phase A:**

1. **There is no runnable executable** until Phase A4. `src/main.cpp` is gone and the
   `webolution` CMake target with it.
2. **Asset-side test coverage regressed.** The three moved tests also covered
   `assets/Cylinder` tile naming (`gfx::parseTilePosition`, INN/OUT, row/col) and
   `assets/Sprite` state mapping (`gfx::spriteNameForState`). Re-establish that coverage once
   the new renderer decides where tile parsing and state→sprite mapping live. The old tests
   are in `old/src/__test__/` to copy from.

## Phase A — the 3D view

The whole phase is "make it look and move like the original". One round per item.

- **A1. Camera and projection.** Read the original's field of view, eye distance from the
  axis, eye height, near/far planes, and how it maps mouse and keys to rotation. Deliverable:
  a camera whose frame matches a captured original frame at a known level, with the
  threshold stated.
  **In progress, not complete:** the [verified camera checkpoint](docs/scratch/phase-a1-camera.md)
  retains the operand reads, standalone pixel fixture, individual mutation/restore proofs,
  and a failed left-key view. Initial, mouse and right-key static regions pass the fixed
  criterion; the left view and depth/clipping remain open. Finish that verification before
  treating A1 as closed. The standalone harness does not pay the entry-point debt.
  The [active draw-path check](docs/scratch/phase-a1-cylinder-draw.md) now corroborates a
  bounded inner-bank submission. The [cell-fixture check](docs/scratch/phase-a1-cell-oracle.md)
  subsequently reproduced the retained failure; its isolated selection mutations
  were unprotected. The [device-emission check](docs/scratch/phase-a1-cell-emission.md)
  now corroborates the selected quad's ordered coordinates and UVs, while its depth
  representation differs from the candidate's. The subsequent
  [projection/cache replay](docs/scratch/phase-a1-projection-cache.md) reproduces the
  retained matrices and selected depth fields under a diagnostic arithmetic model;
  its new operands have not cleared isolated pixel mutations. Next render and test
  that sourced candidate, then resolve remaining clipping and device-state gaps,
  keeping the camera and pixel criterion fixed.
- **A2. The tube.** INN/OUT texture addressing, draw order, inner surface before outer so
  the far side shows through gaps, and whatever scenery the original draws that we skip.
- **A3. Sprite view selection.** `.SPR` stores multiple **views** per action — `walk` 12,
  `block` 8, `fall` 7 — because a lemming's apparent angle depends where it stands on the
  tube. We hardcoded view 0. Read how the original picks the index from angular position.
  *This is the single largest "the sprites look wrong" fix.*
- **A4. Entry point and controls.** A new `src/main.cpp` and renderer, **3D by default**
  with the flat unwrapped view as a debug flag — the inverse of the old arrangement. Revolve
  must be a first-class, discoverable control. Pay debt 1 and debt 2 here.

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
retail level playable end to end in 3D.

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

## How we work

**One round, one question.** A round that half-answers three things is worth less than one
that closes a single thing. One agent owns the running game at a time.

**Facilitator and task agents are different models, deliberately.**

| Role | Model | Does |
| --- | --- | --- |
| Facilitator | **Sol** (the Opus role) | Picks the next round, writes the brief, **independently verifies every headline claim**, owns `PLAN.md`, `docs/` and `.claude/rules/`, lands results |
| Task agent | **Luna** (the Sonnet role) | Runs one round: reads code, runs the harness, writes the sim/renderer change, writes tests, keeps candidate prose outside `docs/` until facilitator verification |

**User-authorized correction for Codex:** use Sol in place of Opus and Luna in place of
Sonnet. This supersedes the original model names; it does not change the division of work
or the verification standard. Dispatch task agents with an explicit `model: gpt-6-luna`.
Keep unverified candidate prose in `build/tmp/` so no original-game number reaches `docs/`
before the facilitator completes verification; the facilitator lands accepted scratch notes.

The reasoning: a round is bounded
work against a written brief, where a mid-tier model is fast and cheap enough to run many
rounds; the facilitation — judging whether a claim is actually supported, noticing an
over-generalisation, catching a stale document — is where the stronger model earns its cost.
**Verification is not optional and does not delegate**: the project's results are sound
because headline numbers were re-derived independently, and several rounds had a claim
narrowed or retracted on re-check.

The facilitator owns `PLAN.md`, `docs/**` and `.claude/rules/**`. Task agents own their
candidate scratch note outside `docs/`, the code and the tests.

Every brief states: the already-settled facts (so nothing is rediscovered), the round's
question with every lead already in hand, the evidence standard, the mutate-one-thing-alone
test requirement, the environment traps, a must-not-touch list, and "report honestly — do
not round, smooth or over-claim".
