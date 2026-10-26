# C3ag — AA/AC retail arms enabled in configured CTest

2026-10-25. One CMake ENV-list correction, no production behavior or original-game
measurement. Packet: `build/tmp/c3ag-retail-env/round-note.md`.

## Facilitator check

Read the one-line CMake diff: existing `scenemeshsource` and
`scenebillboardsource` names join the configured `WEBOLUTION_GAME_DIR` property.
Own driver `build/tmp/c3ag-independent.ps1` removes the parent environment, configures
the preset and independently observes **443 billboard /31 mesh checks**, no skips.
The CTest JSON properties also include the configured install for both targets.

Unlike the worker's two-entry removal, the facilitator removes **only** the
billboard ENV entry. That test drops to48 generic checks and explicitly skips its
retail17 comparison; mesh remains31, not skipped. Restoring exact CMake bytes
returns443/31; SHA256 `337f210881d8f38c19eca7e95f668e9a1e3e8aa1c0a6e697aeb905f09a3b77e9`.
Logs are under `build/tmp/c3ag-independent/` (PowerShell UTF-16 output).
An initial shell-regex quoting error was corrected. An early full rebuild encountered
AF's transient in-flight source; it is not counted as a successful build. Once that
worker returned, own clean configure/rebuild and **39/39 CTest** passed at17:59:06.

The required post-change fresh score is in `build/tmp/c3-check-ag-unaccepted-af/`:
explicit repair4 reference and independent Pillow histogram both give
19715561/235008000=0.0838931483, with per-region and all10 C1/full-frame tables.
**This render contains the separately rejected, uncommitted initial AF candidate**:
it repeats the billboard traversal four times and uses an unsupported camera chain.
It is an unaccepted diagnostic, not a promoted C3 improvement attributable to ENV
wiring. The last accepted C3 production score stays0.0923687832. C1 gated panel
scores remain0.0160864/0.0161212; elev_up0.0250557, reported only. No gate changed.

No new native address claim, start, attach or install/game write. Separate process
and scheduled-task checks are empty. This landing stages only CMake, this note,
history/lessons and facilitator queue/archive bookkeeping; AF's source stays its
worker's responsibility. Existing retractions and prior coverage limits remain.

Dispatch17:44:17 MDT; landing time/elapsed recorded in the task archive. Next finish
AF's actual input-chain repair and AH's real-controller mock verification, then
camera-composed predicate and remaining known integration omissions.
