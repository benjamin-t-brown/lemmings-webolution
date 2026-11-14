# Current run status (generated)

Derived from tasks.json and the latest loop result; do not edit manually.
Task acceptance is not project completion or proof of gameplay fidelity.
Read IMPLEMENTATION_PLAN.md for priorities and docs/STATUS.md for the fidelity ledger.

## Queue

- oracle-001: **blocked** — Propose a bounded original-game comparison
- native-006: **blocked** — Add expected-result coverage for texture pixel-format enumeration
- multi-hatch-release-001: **blocked** — Release lemmings per hatch on multi-hatch levels
- hover-feedback-001: **ready** — Highlight hovered lemmings and show the hatch's remaining count
- fluid-hazard-001: **ready** — Kill ordinary lemmings that enter water
- lever-render-001: **ready** — Draw levers and recheck the empty containers on Introducing Water Lemmings
- panel-icon-anim-001: **ready** — Animate skill icons on the panel when hovered or selected
- antigrav-presentation-001: **ready** — Draw lemmings correctly in anti-gravity zones
- gravity-gadgets-001: **ready** — Show gravity flippers and kill lemmings that fall into the ceiling
- map-hover-text-001: **ready** — Make the map hover text match the original's three-line block
- exit-balloon-path-001: **ready** — Make the exit balloon spiral out as in the original

## Recently completed queue entries

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
- actor-depth-001: Occlude lemmings per pixel against the level's depth
  Accepted: 2026-11-14T02:06:11.987258+00:00
  Verification evidence: `build/agent-runs/20261114T014716-38924-40c75173/verification.log`
  Verification evidence: `build\agent-runs\20261114T014716-38924-40c75173\verification.log`
- hatch-timing-001: Release lemmings after the entry text and slow the hatch doors
  Accepted: 2026-11-14T03:42:28.757908+00:00
  Verification evidence: `build/agent-runs/20261114T030531-38924-3dfc5b73/verification.log`
  Verification evidence: `build\agent-runs\20261114T030531-38924-3dfc5b73\verification.log`

## Latest loop

- Result: `build/agent-runs/loop-6789c90f47134d8b83bc7c23d8c1798b/result.json`
- Status: **running**
- Duration: 3h 00m 22s
- Completed in this invocation: actor-depth-001, hatch-timing-001

## Documentation handoff

At each accepted milestone or priority change, reconcile IMPLEMENTATION_PLAN.md and
docs/STATUS.md with the accepted review and its verification log. Keep detailed
per-run reports historical; append corrections rather than replacing their evidence.
A generated ledger does not update narrative conclusions or historical fidelity gates.
