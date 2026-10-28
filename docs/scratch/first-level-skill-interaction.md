# First-level skill interaction through the application

Offline continuation after the connected frontend/intro work. No new stock
process, attach, desktop input, screenshot of the desktop, or install/save/config
write. The earlier desktop window was not reused.

## Two concrete corrections

Retained native evidence in `tools/harness/assign_skill.py:99–111` identifies
**455611** as rejecting skill-panel hover when stock<=0. The runtime adapter
previously let every skill plate change selection. `RuntimeSkillSelection` now
requires positive stock, while still consuming clicks on unavailable plates
and the two unsupported command plates. Number-key selection uses the same
availability rule as a **SOURCE adapter**, not a recovered native key binding.
The generic `selectIndex` fixture helper remains separate from available-stock
selection. Negative synthetic stocks remain hidden/unselectable in this UI;
native unlimited-stock behavior is not newly established.

Retained assignment chain **455657 → 455756** resumes UI1→0 only after a
successful assignment (`assign_skill.py:77–88,135–137`). The app previously
stayed paused. Its click helper now returns the actual `World::assign` result;
the event handler resumes only on success. Failed/empty/duplicate assignments
and panel selections do not resume. Existing SOURCE accumulator-reset policy
on resume is unchanged; this does not reopen native simulation timing.

The level loop also stops before rendering another frame after SDL_QUIT, rather
than processing one extra simulation/render iteration after close.

## Actual-application test, not just a simulation call

`AppEntry.h` adds an optional read-only `LevelFrameObserver`, empty in normal
runs. It observes the composed frame, World, pause/selection and projected actor
rectangles; it does not set gameplay state. The test queues SDL events only into
its own process, and refuses to run unless `SDL_VIDEODRIVER=dummy`.

`levelinteraction` invokes the production application on retail `MM_CLIMB_002`
with **20 diagnostic starting ticks**, then exercises five composed frames:

1. Pause; click Climber, unavailable Floater, unavailable key2, and a command plate.
2. Check selected Climber, stock10 and pause; click an empty in-screen position.
3. Check no stock/pause change; click a projected active actor.
4. Check that actor now has `canClimb`, stock is9, pause is off, and rendered
   count pixels changed; pause again and repeat the assignment click.
5. Check the rejected duplicate preserves stock9 and pause, then close.

The callback never calls `World::assign` or edits a lemming. Assignment passes
through the actual SDL event, panel routing, projected-rectangle picking and
World mutation path. A separate unit test covers depletion and both positive
and nonpositive stock selection. Earlier retail panel pixel checks remain.

The diagnostic seed bounds this test; it is **not** a natural menu→spawn timing
test. The existing `frontendentry` test separately covers natural menu→level
handoff and the opening camera. Code comments previously called `--sim-ticks`
headless-only even though it also seeds direct-level windows; comments now state
the existing behavior accurately. Frontend entry still explicitly resets ticks.

## Verification and remaining limits

Four isolated mutations fail: omitting successful-assignment resume; resuming
failed assignments; allowing zero-stock selection; rendering after close. The
stock mutant fails both the unit test and actual application test. Production
files were restored against `build/tmp/enacter-level-interaction/pre-mutation.sha256`.

Restored full build/CTest: **57/57 PASS**,99.09seconds; the actual level
interaction test took1.69seconds (`final-tests.log` in the same packet).

This was not full native picking acceptance. The caller at this checkpoint supplied
placeholder priority/distance keys and lacked native eligibility/facing filters.
The later [overlap continuation](first-level-overlap-selection.md) replaces those
keys for supported states and adds necessary Climber exclusions; full eligibility
is still open. D2b
already recorded an overlap where those placeholders choose the wrong actor;
`docs/scratch/phase-d2-pick-capture.md` and `phase-d2-tie-keys.md` retain the facts.
The integration test avoids earlier overlapping projected candidates. Projection
is not proof of native hover eligibility or visible pixels through occlusion.
The retained Climber50/50 native tick trace is independent evidence, not a new
result from this test. Other skills, complete click/hold/double-click behavior,
cursor feedback and first-level end-to-end success remain open.
