# A5m: source map for actor integration, not an integration implementation

Queue `a5m-actor-integration-map`; repository-only explore continuation completed
on Task session `ses_ea5fce916ffeFd04fZ0nxC4RYU`. Response receipt:
`build/tmp/a5m-actor-integration-map-response.md`. No original-executable
investigation, live measurement, source implementation or new pixel claim.

## Accepted repository map

Paths/ranges below refer to the post-A4y worktree, not executable addresses.

| Producer / consumer | Source | What exists |
| --- | --- | --- |
| Simulation actors | `src/sim/World.h:75-123,985-989` | Integer terrain x/y, facing, state, frame, verticalDir, sheetType; const actor-list accessor |
| State name mapping | `src/gfx/SpriteState.h:28-54` | State to SPR action name; Splatter explicitly unverified; Teleporting/Exited/Dead return empty |
| SPR data and lookup | `src/assets/Sprite.h:66-96`; `Sprite.cpp:107-112` | Temporal entries containing directional frames, action rate, find-by-name and load API |
| Direction chooser | `src/gfx/SpriteView.h:19-35`; `.cpp:7-84` | Scores require camera-to-actor world vector, stored orientation basis and phase; selection requires directional count and prior mirror |
| Placement | `src/app/ActorSprite.h:24-44`; `.cpp:17-88` | Requires 3D world position, camera/lensed transform, selected frame, mirror and scale delta; outputs screen rectangle and biased depth |
| Actor raster | `src/app/ActorSprite.cpp:90-148` | Writes RGBA Image; uses rectangle/UV/mirror/tint/alpha test, not placement.depth |
| Tube raster | `src/app/Raster.h:35-60`; `.cpp:58-106` | Triangle depth test/write against private depth_ and RGB framebuffer; no actor/depth-sharing API |
| App frame | `src/app/main.cpp:134-229,318-323`; `TubeScene.h:77-85` | Loads tube, builds camera, clears/renders TubeScene and presents RGB; no World or actor step |
| Fixture inputs | `src/tools/actorfixture.cpp:48-116` | Loads LEMMINGSHI, selects explicit action/temporal/direction; CLI world position, mirror/scale; produces standalone overlay |
| Tint provider | `src/assets/LightMap.h:34-45`; `actorfixture.cpp:90-104` | Existing world-X/Y lookup and level load; called in fixture, not app |

Bounded source call-site search confirms `drawActorSprite` callers in the fixture
and tests, not the app. `spriteNameForState` and the A3 score/selection helpers
have test callers but no production simulation-to-render chain. This does not
mean those APIs lack tests or all caller types are absent.

## Missing links and implementation prerequisites

- **Terrain to 3D world pose:** Lemming x/y are terrain coordinates, not a ready
  array of actor world X/Y/Z. Missing is the cylindrical position conversion,
  including the established terrain/file Y convention, not just a guessed Z.
  Producer would be a presentation adapter; consumer is actorSpritePlacement.
- **Orientation and mirror state:** facing/verticalDir do not directly provide
  the basis, phase and prior-mirror inputs that SpriteView consumes. Their
  mapping must be established before connecting that adapter.
- **Temporal progression:** Sprite exposes rate and temporal arrays, but World
  has no general render-animation clock. Header frame is described as ticks;
  World.cpp:303-311 adds milliseconds for Drown, while :748 increments frame
  after the general state switch. Do not treat this field as a uniform SPR
  temporal index or repair simulation in this map round.
- **Action/sheet binding:** spriteNameForState plus SpriteSheet::find exist;
  production must bind sheetType/detail, handle empty/unsupported states, and
  select/load the resulting texture without assuming every state is verified.
- **Composition/depth:** actor output is RGBA Image, tube output RGB Framebuffer.
  There is no current shared actor-depth path; the two depth representations
  must not be equated merely because both fields are named depth. What the
  original does at that boundary is not established by this source map.
- **Tint and scale:** fixture supplies world position to light lookup and uses
  explicit scale delta (default -1). App needs the correct producer inputs;
  no permission to hard-code fixture values for all actors follows.

## Facilitator check (2026-10-20)

Independently opened every cited source range and both selector implementations,
read the actor pixel loop and tube depth loop, and searched helper call sites in
src. Corrected the worker's `actionfixture.cpp` typo and narrowed “only caller”
to exclude tests. Added the mixed-unit frame counter correction and expanded
the missing-Z description to the whole terrain-to-cylinder pose conversion.

The worker's proposed Lemming+camera-to-all-selection adapter is **not yet an
authorized implementation specification**: missing basis/phase/time/world-pose
provenance would force guesses. A test asserting only non-empty output would
not discriminate wrong selection or placement. Smallest follow-up after the
already-queued A4z fixture: establish one upright Walker's terrain-to-world pose
and orientation inputs against existing accepted A3 actor evidence, with exact
predicted inputs and a falsifier, before expanding to clocks/all actions/app.
That follow-up still needs its own bounded brief and evidence review.

No production changes: rebuild, CTest, source mutations and executable-byte
rereads are not evidence gates for this repository map and were not run. No live
operations or restoration writes were made. A4y's two-fixture acceptance and the
retained A2/A3 wider failures are unchanged. Existing dirty work was preserved.

## Queue handoff

The old free-model session was explicitly cancelled (parent Task cancellation
and final MessageAbortedError checked in OpenCode's DB); termination was not
inferred from age. The continuation used the current documented explore-agent
configuration, verified by `opencode debug agent explore-agent`, and passed
`python scripts/check_agent_framework.py --brief build/tmp/a5m-actor-integration-map-continue1.md`.
Actual execution model is not independently exposed by its Task return.

A4z remains the next queued brief, with its A4y dependency satisfied. Its current
brief assumes a Mac mount/SSH execution path; a local-Windows facilitator must
adapt and lint that environment section before dispatch rather than blindly
running the SSH instructions. No new live owner is assigned by this map.
