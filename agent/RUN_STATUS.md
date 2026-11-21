# Current run status (generated)

Derived from tasks.json and the latest loop result; do not edit manually.
Task acceptance is not project completion or proof of gameplay fidelity.
Read IMPLEMENTATION_PLAN.md for priorities and docs/STATUS.md for the fidelity ledger.

## Queue

- oracle-001: **blocked** — Propose a bounded original-game comparison
- native-006: **blocked** — Add expected-result coverage for texture pixel-format enumeration
- multi-hatch-release-001: **blocked** — Release lemmings per hatch on multi-hatch levels
- rat-hit-sound-002: **blocked** — A lemming punched off by a rat plays the original's sound, not Ohno
- main-export-001: **ready** — Clean source export for the public main branch, with a GitHub Pages build (release)

## Recently completed queue entries

- inverted-builder-fluid-001: An upside-down builder treats water ahead as a roof instead of empty space
  Accepted: 2026-11-20T19:12:32.572984+00:00
  Verification evidence: `build/agent-runs/20261120T185523-37944-57dd5e50/verification.log`
  Verification evidence: `build\agent-runs\20261120T185523-37944-57dd5e50\verification.log`
- antigrav-lift-quads-001: Anti-grav lift effects show their quads; draw them as smoothly as the original
  Accepted: 2026-11-20T19:47:20.821813+00:00
  Verification evidence: `build/agent-runs/20261120T192757-42720-28d85b6e/verification.log`
  Verification evidence: `build\agent-runs\20261120T192757-42720-28d85b6e\verification.log`
- switch-hover-highlight-002: Switch hover brightens the controlled elements like a hovered lemming instead of drawing rectangles
  Accepted: 2026-11-20T21:21:44.866718+00:00
  Verification evidence: `build/agent-runs/20261120T211150-55872-44197c94/verification.log`
  Verification evidence: `build\agent-runs\20261120T211150-55872-44197c94\verification.log`
- disc-unrar-001: RAR 2.0 extractor for the disc patch, and 1.0 patch.box support (web release, clone addition)
  Accepted: 2026-11-21T02:04:19.522193+00:00
  Verification evidence: `build/agent-runs/20261121T014241-49632-de00de90/verification.log`
  Verification evidence: `build\agent-runs\20261121T014241-49632-de00de90\verification.log`
- web-game-data-001: Web release: game data setup (installed files, CD image, archive.org download) (clone addition)
  Accepted: 2026-11-21T02:21:18.769648+00:00
  Verification evidence: `build/agent-runs/20261121T020419-49632-e97b4560/verification.log`
  Verification evidence: `build\agent-runs\20261121T020419-49632-e97b4560\verification.log`

## Latest loop

- Result: `build/agent-runs/loop-87751a2bb6ce4d48a7cf210b2737ef69/result.json`
- Status: **blocked**
- Duration: 1h 39m 07s
- Completed in this invocation: disc-unrar-001, web-game-data-001
- Reason: Attempt limit reached for main-export-001

## Documentation handoff

At each accepted milestone or priority change, reconcile IMPLEMENTATION_PLAN.md and
docs/STATUS.md with the accepted review and its verification log. Keep detailed
per-run reports historical; append corrections rather than replacing their evidence.
A generated ledger does not update narrative conclusions or historical fidelity gates.
