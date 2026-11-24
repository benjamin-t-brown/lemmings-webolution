# Current run status (generated)

Derived from tasks.json and the latest loop result; do not edit manually.
Task acceptance is not project completion or proof of gameplay fidelity.
Read IMPLEMENTATION_PLAN.md for priorities and docs/STATUS.md for the fidelity ledger.

## Queue

- oracle-001: **blocked** — Propose a bounded original-game comparison
- native-006: **blocked** — Add expected-result coverage for texture pixel-format enumeration
- multi-hatch-release-001: **blocked** — Release lemmings per hatch on multi-hatch levels
- rat-hit-sound-002: **blocked** — A lemming punched off by a rat plays the original's sound, not Ohno
- gl-presenter-web-001: **blocked** — OpenGL phase 2b: WebGL2 presenter, Start-page renderer choice and software fallback
- gl-opaque-meshes-001: **ready** — OpenGL phase 3: tube and backdrop opaque meshes in comparison mode
- gl-scene-passes-001: **ready** — OpenGL phase 4: billboards, secondary sprites, pipes, fluid, counter gates and sky fill
- gl-actors-001: **ready** — OpenGL phase 5: actor bodies, attachments and displayed-frame queries on GL
- gl-effects-001: **ready** — OpenGL phase 6: explosion marks, drowning, debris and particles on GL
- gl-2d-play-001: **ready** — OpenGL phase 7: 2D compositor and ordinary play on the GL renderer
- gl-web-001: **ready** — OpenGL web: the finished GL renderer on WebGL2, with the parked presenter of gl-presenter-web-001

## Recently completed queue entries

- language-page-001: The main-menu Language button opens a Language page that offers English only (user request 2026-11-23)
  Accepted: 2026-11-24T16:44:01.937910+00:00
  Verification evidence: `build/agent-runs/loop-d9c559c67410437aacac3a3d93c3b841/0001-verify-ae4b2a/verification.log`
- shortcuts-simplify-001: Keyboard Shortcuts screen keeps only the Level Play rows, without their heading (user decision 2026-11-23)
  Accepted: 2026-11-24T16:56:17.035919+00:00
  Verification evidence: `build/agent-runs/20261124T164402-52984-37f7aaa3/verification.log`
  Verification evidence: `build\agent-runs\20261124T164402-52984-37f7aaa3\verification.log`
- level-menu-order-001: Level menu: Examine becomes Examine Level and Quit moves to the bottom (user request 2026-11-23)
  Accepted: 2026-11-24T17:25:18.214961+00:00
  Verification evidence: `build/agent-runs/loop-caa7257efdfe48258024b3a04cc7a5e7/0001-verify-9d6933/verification.log`
- gl-boundary-001: OpenGL phase 1: prepared-frame boundary with software as its first consumer
  Accepted: 2026-11-24T18:43:48.254285+00:00
  Verification evidence: `build/agent-runs/20261124T182912-57984-a803642f/verification.log`
  Verification evidence: `build\agent-runs\20261124T182912-57984-a803642f\verification.log`
- gl-presenter-native-001: OpenGL phase 2a: renderer selector and the native OpenGL 3.3 presenter with readback
  Accepted: 2026-11-24T19:18:16.428641+00:00
  Verification evidence: `build/agent-runs/20261124T190704-57984-548dcb21/verification.log`
  Verification evidence: `build\agent-runs\20261124T190704-57984-548dcb21\verification.log`

## Latest loop

- Result: `build/agent-runs/loop-3fa3669915584fa8a8e71df056d44ed5/result.json`
- Status: **blocked**
- Duration: 1h 53m 03s
- Completed in this invocation: gl-boundary-001, gl-presenter-native-001
- Reason: The retained headless browser attempts stopped before the game page ran. Acceptance still requires browser frames for the menu, map and Just Climb, a byte-for-byte logical readback comparison, and logs showing session, save lock and selected level continuity through both fallback cases. The desktop build, 95 smoke tests and 84 recovery tests passed.

## Documentation handoff

At each accepted milestone or priority change, reconcile IMPLEMENTATION_PLAN.md and
docs/STATUS.md with the accepted review and its verification log. Keep detailed
per-run reports historical; append corrections rather than replacing their evidence.
A generated ledger does not update narrative conclusions or historical fidelity gates.
