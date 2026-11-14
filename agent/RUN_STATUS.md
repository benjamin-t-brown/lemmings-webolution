# Current run status (generated)

Derived from tasks.json and the latest loop result; do not edit manually.
Task acceptance is not project completion or proof of gameplay fidelity.
Read IMPLEMENTATION_PLAN.md for priorities and docs/STATUS.md for the fidelity ledger.

## Queue

- oracle-001: **blocked** — Propose a bounded original-game comparison
- native-006: **blocked** — Add expected-result coverage for texture pixel-format enumeration
- multi-hatch-release-001: **blocked** — Release lemmings per hatch on multi-hatch levels
- actor-depth-001: **ready** — Occlude lemmings per pixel against the level's depth
- hatch-timing-001: **ready** — Release lemmings after the entry text and slow the hatch doors
- hover-feedback-001: **ready** — Highlight hovered lemmings and show the hatch's remaining count
- panel-icon-anim-001: **ready** — Animate skill icons on the panel when hovered or selected
- antigrav-presentation-001: **ready** — Draw lemmings correctly in anti-gravity zones
- exit-balloon-path-001: **ready** — Make the exit balloon spiral out as in the original

## Recently completed queue entries

- actor-occlusion-001: Hide lemmings that are behind the cylinder
  Accepted: 2026-11-13T21:57:26.620075+00:00
  Verification evidence: `build/agent-runs/loop-483497d2694f4fb2af53728ad6be337d/0001-verify-e08e18/verification.log`
- hatch-exit-presentation-001: Show hatch doors, lemmings dropping in and the exit balloon leaving
  Accepted: 2026-11-13T23:04:22.005168+00:00
  Verification evidence: `build/agent-runs/20261113T223906-34648-b23e7597/verification.log`
  Verification evidence: `build\agent-runs\20261113T223906-34648-b23e7597\verification.log`
- map-level-names-001: Show the level name when hovering a level on the map
  Accepted: 2026-11-13T23:32:25.505385+00:00
  Verification evidence: `build/agent-runs/20261113T230454-17836-b6c070eb/verification.log`
  Verification evidence: `build\agent-runs\20261113T230454-17836-b6c070eb\verification.log`
- walker-lane-001: Keep walkers on the right side of the path and slide across on turning
  Accepted: 2026-11-13T23:59:00.076901+00:00
  Verification evidence: `build/agent-runs/20261113T233225-17836-92532a4d/verification.log`
  Verification evidence: `build\agent-runs\20261113T233225-17836-92532a4d\verification.log`
- fluid-kinds-001: Draw the remaining level fluids and fix standing height leaving water
  Accepted: 2026-11-14T00:41:13.455523+00:00
  Verification evidence: `build/agent-runs/20261113T235900-17836-7342c0bd/verification.log`
  Verification evidence: `build\agent-runs\20261113T235900-17836-7342c0bd\verification.log`

## Latest loop

- Result: `build/agent-runs/loop-78d4616b6acb49468780649960f72b44/result.json`
- Status: **blocked**
- Duration: 2h 09m 57s
- Completed in this invocation: map-level-names-001, walker-lane-001, fluid-kinds-001
- Reason: Criterion 3 fails: M20_MT_1 (node 50) releases 45/53 instead of 53/53, below its save target of 48. Preserving its installed quotas, rates and time limit prevents the slow hatch emptying in time. Resolving this requires authorization for a bounded follow-up; the current task prohibits rate retuning. Criteria 1, 2 and 4 are supported. Passing verification does not waive the regression or establish original-game fidelity.

## Documentation handoff

At each accepted milestone or priority change, reconcile IMPLEMENTATION_PLAN.md and
docs/STATUS.md with the accepted review and its verification log. Keep detailed
per-run reports historical; append corrections rather than replacing their evidence.
A generated ledger does not update narrative conclusions or historical fidelity gates.
