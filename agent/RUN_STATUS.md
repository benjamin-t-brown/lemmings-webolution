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
- gl-perf-001: **ready** — OpenGL performance: native GL frame cost far below software, no per-frame GPU stalls
- gl-web-001: **blocked** — OpenGL web: the finished GL renderer on WebGL2, with the parked presenter of gl-presenter-web-001

## Recently completed queue entries

- gl-opaque-meshes-001: OpenGL phase 3: tube and backdrop opaque meshes in comparison mode
  Accepted: 2026-11-24T20:51:21.233408+00:00
  Verification evidence: `build/agent-runs/20261124T204419-9332-0a66a12f/verification.log`
  Verification evidence: `build\agent-runs\20261124T204419-9332-0a66a12f\verification.log`
- gl-scene-passes-001: OpenGL phase 4: billboards, secondary sprites, pipes, fluid, counter gates and sky fill
  Accepted: 2026-11-24T21:15:05.567841+00:00
  Verification evidence: `build/agent-runs/20261124T205121-9332-0115e591/verification.log`
  Verification evidence: `build\agent-runs\20261124T205121-9332-0115e591\verification.log`
- gl-actors-001: OpenGL phase 5: actor bodies, attachments and displayed-frame queries on GL
  Accepted: 2026-11-24T22:01:28.644790+00:00
  Verification evidence: `build/agent-runs/20261124T215024-9332-7b711479/verification.log`
  Verification evidence: `build\agent-runs\20261124T215024-9332-7b711479\verification.log`
- gl-effects-001: OpenGL phase 6: explosion marks, drowning, debris and particles on GL
  Accepted: 2026-11-24T22:34:16.462364+00:00
  Verification evidence: `build/agent-runs/20261124T220128-9332-97a88ddf/verification.log`
  Verification evidence: `build\agent-runs\20261124T220128-9332-97a88ddf\verification.log`
- gl-2d-play-001: OpenGL phase 7: 2D compositor and ordinary play on the GL renderer
  Accepted: 2026-11-24T23:13:50.148535+00:00
  Verification evidence: `build/agent-runs/20261124T223416-9332-0e6f18d1/verification.log`
  Verification evidence: `build\agent-runs\20261124T223416-9332-0e6f18d1\verification.log`

## Latest loop

- Result: `build/agent-runs/loop-2d1ab42e918440b586ca92e9a3f497ba/result.json`
- Status: **blocked**
- Duration: 3h 50m 34s
- Completed in this invocation: gl-opaque-meshes-001, gl-scene-passes-001, gl-actors-001, gl-effects-001, gl-2d-play-001
- Reason: The native build and smoke gate passed. The current Emscripten build’s headless browser attempt timed out at Runtime.enable before Start. Acceptance requires an interactive lead run of the retained harness with browser logs, canvas frames, parity diagnostics, and fallback results for gl-web-001.

## Documentation handoff

At each accepted milestone or priority change, reconcile IMPLEMENTATION_PLAN.md and
docs/STATUS.md with the accepted review and its verification log. Keep detailed
per-run reports historical; append corrections rather than replacing their evidence.
A generated ledger does not update narrative conclusions or historical fidelity gates.
