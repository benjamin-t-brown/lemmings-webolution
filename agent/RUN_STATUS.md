# Current run status (generated)

Derived from tasks.json and the latest loop result; do not edit manually.
Task acceptance is not project completion or proof of gameplay fidelity.
Read IMPLEMENTATION_PLAN.md for priorities and docs/STATUS.md for the fidelity ledger.

## Queue

- oracle-001: **blocked** — Propose a bounded original-game comparison
- native-006: **blocked** — Add expected-result coverage for texture pixel-format enumeration
- multi-hatch-release-001: **blocked** — Release lemmings per hatch on multi-hatch levels
- rat-hit-sound-002: **blocked** — A lemming punched off by a rat plays the original's sound, not Ohno
- web-landscape-001: **ready** — Phones play the web build in landscape (mobile request V1)
- touch-camera-001: **ready** — Touch drag rotates the camera and pinch toggles zoom (mobile request V2)
- touch-assign-001: **ready** — Touch assigns at touch start, hold gives a precision cursor, hatch slide sets the rate (mobile request V3)
- touch-buttons-001: **ready** — Touch Menu button at top centre, and Examine in the level menu (mobile request V4)

## Recently completed queue entries

- level-music-start-001: Level music starts at once on Try again; the original waits for the title cue (playtest 28 T1)
  Accepted: 2026-11-22T17:22:21.793823+00:00
  Verification evidence: `build/agent-runs/20261122T170522-46400-840a02ad/verification.log`
  Verification evidence: `build\agent-runs\20261122T170522-46400-840a02ad\verification.log`
- rate-hold-speed-001: Holding a hatch changes the release rate far slower than the original (playtest 28 T2)
  Accepted: 2026-11-22T17:36:54.633656+00:00
  Verification evidence: `build/agent-runs/20261122T172512-46400-c72b5974/verification.log`
  Verification evidence: `build\agent-runs\20261122T172512-46400-c72b5974\verification.log`
- release-exe-name-001: Release downloads ship the game as lemmings_webolution(.exe)
  Accepted: 2026-11-22T17:50:52.249810+00:00
  Verification evidence: `build/agent-runs/20261122T174609-46400-a3e654c4/verification.log`
  Verification evidence: `build\agent-runs\20261122T174609-46400-a3e654c4\verification.log`
- teleport-occlusion-001: Teleport portal model and teleport effect draw with occlusion errors (playtest 29 U1)
  Accepted: 2026-11-22T20:08:20.179775+00:00
  Verification evidence: `build/agent-runs/20261122T195326-44268-3e28422a/verification.log`
  Verification evidence: `build\agent-runs\20261122T195326-44268-3e28422a\verification.log`
- teleport-hover-partner-001: Hovering a teleport highlights it and its sister portal (playtest 29 U2)
  Accepted: 2026-11-22T20:32:19.858401+00:00
  Verification evidence: `build/agent-runs/20261122T202408-44268-a5900692/verification.log`
  Verification evidence: `build\agent-runs\20261122T202408-44268-a5900692\verification.log`

## Latest loop

- Result: `build/agent-runs/loop-61557c473400430eae54881cef831563/result.json`
- Status: **blocked**
- Duration: 0h 57m 41s
- Completed in this invocation: teleport-occlusion-001, teleport-hover-partner-001
- Reason: The latest playtest items, U1 and U2 on The Lone Ranger, both have accepted production fixes. Its promoted recording ended at 0/90 before those fixes and identifies no separate gameplay defect. The next dependency is human play on the corrected build to confirm both fixes and continue the level. The queue has no planning-required placeholder, and the ledgers provide no new unparked production defect to schedule. Playable v1 stands at 37 of 102 human wins. Full completion still requires the final

## Documentation handoff

At each accepted milestone or priority change, reconcile IMPLEMENTATION_PLAN.md and
docs/STATUS.md with the accepted review and its verification log. Keep detailed
per-run reports historical; append corrections rather than replacing their evidence.
A generated ledger does not update narrative conclusions or historical fidelity gates.
