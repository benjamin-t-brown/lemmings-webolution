# Connected level completion -> results -> map (2026-10-28)

Offline continuation after [runtime cursor/hover](runtime-cursor-hover.md).
No original-game launch, attachment, desktop input, registry access, or installed
save/config writes. This delivers a connected SOURCE application loop, not a
new D3 timing or native full-frame acceptance.

**Follow-up:** [pending transition/latching](level-end-transition.md) supersedes
the initial immediate terminal boundary described below. Native end-count
identity, timer seed/strict thresholds and outcome latching are now statically
closed and integrated for the bounded SOURCE population adapter. Simulation and
hatches continue while pending; late rescues cannot upgrade a latched loss.

## Application behavior

`--frontend` now owns one `FrontendSession` across menu, confirmation, map,
level, results, and subsequent levels. The existing retail loader and SDL level
loop run as successive legs, not recursive application calls. A completed run
hands an immutable `LevelOutcome` across window teardown; the session renders
results and rebuilds its map from the same in-memory completion flags.

- **Success:** mark only the selected map node's completion flag1, before the
  Continue action. Existing MapScene mapping/graph derives unlocked neighbors.
  Do not use the older `shell::Progress` sequential-next-slot policy here.
- **Failure:** leave every earned completion flag unchanged.
- **Continue:** click the actual rendered button to return to the rebuilt map.
- **Try Again:** on loss, click to reload the same retail level with a fresh
  World, skill stock and camera/runtime state; progress survives. The
  [Quit/intro follow-up](loss-results-quit.md) disables the opening move on Retry.
  Native retry/fade timing is not reproduced by this immediate SOURCE reload.
- **New Game Yes:** clears only this owned session's flags and restores the
  initial map. No writes to the installed `SavedGame.dat` occur.
- **Loss-results Quit:** returns to the current-progress map, without reloading
  or exiting. This distinct route is resolved in the [follow-up](loss-results-quit.md).
- **Window close/frame cap:** do not synthesize a level outcome or award completion.

Default direct-level and headless fixtures keep their former behavior. Only
connected frontend play collects terminal outcomes. Frame limits apply per
frontend/level leg, as the prior entry handoff did. If a level finishes before
its requested capture, capture ownership passes to the results/frontend leg;
an unfinished or closed run is not treated as a completion.

The SOURCE terminal condition is the existing simulation's `finished()` (all
released, none alive) **or** `timedOut()`. Reaching the rescue target alone does
not stop play. `RuntimeSimState` has an opt-in terminal guard to stop a burst at
the exact terminal simulation tick; its default diagnostic behavior is unchanged.

**Not native equivalence:** native445498 uses remaining time or a count from a
separate object list plus a stored word, then a pending end effect/delay. That
list's semantic correspondence to our actors is unresolved. The delay, AVI
(`complete.avi`/`failed.avi` and conditional `congrats.avi`), best-score update,
and native end-of-game path are not implemented here. Do not label the SOURCE
end time as native completion timing.

## Static correction and renderer evidence

Packet: `build/tmp/enacter-results/findings.md`, raw assembly, `verify_chain.py`,
`read_assets.py`, and retained REA evidence. Main independently reran both
checkers successfully; target hash is
`32675bdb49866e76502713251fdfd2b09078c2481c72ca22ff68ebf4b3737a2f`.
These are static reads, not a live screenshot/input trace.
The retained dependency-free checker `python tools/harness/read_level_results.py`
passes12 raw-backed anchors without needing the scratch packet.

The old speculation that natural UI4 must advance to UI5 is corrected:

| Outcome | main mode | factory selector | UI | constructor / vtable |
| --- | --- | --- | --- | --- |
| Win |3|0x21|4|485DF0 /4B1C30|
| Loss |4|0x20|5|4862F0 /4B1CB0|

Main dispatch is45DDBE..45DF33 (win) and45DF5C..45E0B7 (loss). Thus the retained
D3 natural UI4 observation is consistent with win results, not an intermediate
step toward the loss screen. The native end gate publishes saved/needed and
selected group/slot, then selects main mode3/4 after its pending effect.

`ResultsScene` ports the bounded builders485E2B/486335:

- Background `TEXTURES\\SMALL_BACK.BMP`, not a modal over the level or map.
- Centered red title at(320,220), UK50 for win; UK49 for loss, with its `^`
  second line atY249 using the read29px font line height.
- Centered red `You Needed N` (UK121) at(320,280) and `You Saved N` (UK122)
  at(320,310). These are counts, not percentages, time, or invented score text.
- Row0 atY320 is a hidden empty widget; no blank panel is painted. Win shows
  Continue(UK47) atY349. Loss shows Try Again(UK48), Continue(UK47), Quit(UK43)
  atY349/378/407. Reuse the established MenuRender panel/font primitives.
- Win handler486031's Continue pushes map selector23 at486090; loss handler
  486510's Continue does the same at4865AC. Retry writes its reload request at
  486568, handled through4605D0(1). The [Quit follow-up](loss-results-quit.md)
  resolves its separate request/cleanup route to map23, not process exit.
- Win builder485E55 writes exactly1 to the selected native group/slot's
  completion field, before drawing results or waiting for Continue. Loss has
  no equivalent clearing store. The clone normalizes these into its already
  mapped102 flags and never invokes the native save writer.

Native row0 is initially focused and subsequently hidden. Focus repair was not
traced, so **Return does not automatically Continue in this adapter**. Explicit
button clicks use the same SOURCE rendered-panel geometry as other frontend
screens. No visible row is initially highlighted by guess. Hover/focus lifecycle,
physical DirectInput equivalence and final native pixel sampling remain open.

## Tests

- `levelreturn`: actual SDL application under dummy driver. It navigates New
  Game/confirm/map, assigns Climber using SDL clicks on the rendered actors,
  waits for all ten rescues, verifies the win result/flag before Continue,
  returns to map with node0 completed and nodes3/4 unlocked, enters node0 again
  with fresh stock, assigns nobody, reaches timeout loss and returns to the
  byte-identical previously completed map. No callback mutates the World.
- The test-only `ApplicationRunOptions::frameSeconds=2` supplies deterministic
  elapsed time for bounded runs. The existing clock takes up to ten normal
  simulation ticks per frame; it does not fabricate results or alter actors.
  Normal CLI execution leaves this option zero and keeps the existing clock.
- `results`: independently assembles expected win/loss images from fixed read
  strings/positions and common drawing primitives, checks changed count pixels,
  button geometry, UI4/5/vtables, no invented Return shortcut, invalid/duplicate
  outcome rejection, fresh loss, retained completion through loss/retry, and
  New Game reset. This is composition/wiring coverage, **not a native image oracle**.
- `runtimehud`: terminal bursts stop at timeout rather than overshooting; an
  already resolved World is not advanced. The pre-existing pause/clock tests stay.

A scratch SOURCE solution helper also verified ten-tick assignment opportunities
at ticks20,30,50,60,80,90,100,120,130,150; first rescue at390, all ten resolved
at514 (102.8sim seconds). No-assignment reaches timeout1500 with ten active and
zero saved. These numbers describe our existing simulation and do not supersede
the parked native timing discrepancies.

Isolated sensitivity checks fail for: omitted completion write, loss erasing
earned progress, wrong title anchor, ending as soon as the target is met, and
advancing beyond the terminal clock tick. Production files were restored against
`build/tmp/enacter-results/pre-mutation.sha256` before final regression tests.

Final restored build/CTest: **65/65 PASS**,161.28seconds; `levelreturn`56.64s,
`results`0.46s. Packet log: `full-suite.log`. An initial test build exposed two
test-only issues (a switch-case local crossing labels and missing explicit
string_view-to-string conversions); those were corrected before these passes.
Follow-up actual-app check also sends Return on win results and confirms it
does not invent initial focused Continue (`levelreturn` PASS58.68s).
`FrontendRenderDump` now accepts explicit `win-fixture`, `loss-fixture` and
`completed-map-fixture` stages for offline inspection; all three built/rendered
successfully into this packet. These are known-value SOURCE composition images,
not native captures or proof of a simulated solution.

Remaining scope after the follow-up: general end-population mapping and native
event-sliced timing, movies/fades, result keyboard focus
and native transition-frame acceptance, native progress persistence, broader level/skill applicability, and the
parked simulation/full-frame fidelity gaps. A playable connected first-level
loop is not completion of the 1:1 project.
