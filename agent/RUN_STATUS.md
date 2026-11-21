# Current run status (generated)

Derived from tasks.json and the latest loop result; do not edit manually.
Task acceptance is not project completion or proof of gameplay fidelity.
Read IMPLEMENTATION_PLAN.md for priorities and docs/STATUS.md for the fidelity ledger.

## Queue

- oracle-001: **blocked** — Propose a bounded original-game comparison
- native-006: **blocked** — Add expected-result coverage for texture pixel-format enumeration
- multi-hatch-release-001: **blocked** — Release lemmings per hatch on multi-hatch levels
- rat-hit-sound-002: **blocked** — A lemming punched off by a rat plays the original's sound, not Ohno
- web-mobile-warning-001: **needs_review** — Web: warn mobile players before they start (clone addition)
- water-motion-001: **ready** — Animated water on Just Climb: the pool and the pipe stream (playtest 25 Q5)
- windmill-motion-001: **ready** — Turning windmill blades on Escape to victory (playtest 25 Q6)
- disc-volume-warning-001: **ready** — CD image: a file shorter than its declared volume is a warning when both boxes are intact

## Recently completed queue entries

- main-export-001: Clean source export for the public main branch, with a GitHub Pages build (release)
  Accepted: 2026-11-21T03:53:13.543989+00:00
  Verification evidence: `build/agent-runs/20261121T034237-10280-b3b49126/verification.log`
  Verification evidence: `build\agent-runs\20261121T034237-10280-b3b49126\verification.log`
- hatch-rate-hold-001: Hatch release-rate clicks and holds survive a lemming passing under the cursor (playtest 25 Q1)
  Accepted: 2026-11-21T09:18:18.959012+00:00
  Verification evidence: `build/agent-runs/20261121T090624-14440-8cbc547a/verification.log`
  Verification evidence: `build\agent-runs\20261121T090624-14440-8cbc547a\verification.log`
- windows-release-001: Windows release zip on GitHub releases: boxes or a CD image beside the exe, with a clear missing-game-data message (playtest 25 Q3)
  Accepted: 2026-11-21T10:02:28.130576+00:00
  Verification evidence: `build/agent-runs/20261121T095547-14440-7d3c0d79/verification.log`
  Verification evidence: `build\agent-runs\20261121T095547-14440-7d3c0d79\verification.log`
- app-icon-001: Site favicon and Windows executable icon from the user-supplied yay icon (clone addition)
  Accepted: 2026-11-21T11:04:15.582429+00:00
  Verification evidence: `build/agent-runs/20261121T105115-14440-0637bbaa/verification.log`
  Verification evidence: `build\agent-runs\20261121T105115-14440-0637bbaa\verification.log`
- linux-build-001: Linux build and release archive, built and checked on GitHub Actions (clone addition)
  Accepted: 2026-11-21T10:37:11.390757+00:00
  Verification evidence: `build/agent-runs/20261121T102857-14440-b4257d13/verification.log`
  Verification evidence: `build\agent-runs\20261121T102857-14440-b4257d13\verification.log`

## Latest loop

- Result: `build/agent-runs/loop-f54931a07f1c4e5dbf9b25bd1873545c/result.json`
- Status: **blocked**
- Duration: 2h 03m 25s
- Completed in this invocation: hatch-rate-hold-001, windows-release-001, linux-build-001, app-icon-001
- Reason: review failed; inspect the saved logs

## Documentation handoff

At each accepted milestone or priority change, reconcile IMPLEMENTATION_PLAN.md and
docs/STATUS.md with the accepted review and its verification log. Keep detailed
per-run reports historical; append corrections rather than replacing their evidence.
A generated ledger does not update narrative conclusions or historical fidelity gates.
