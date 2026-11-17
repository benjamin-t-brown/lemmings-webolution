# Current run status (generated)

Derived from tasks.json and the latest loop result; do not edit manually.
Task acceptance is not project completion or proof of gameplay fidelity.
Read IMPLEMENTATION_PLAN.md for priorities and docs/STATUS.md for the fidelity ledger.

## Queue

- oracle-001: **blocked** — Propose a bounded original-game comparison
- native-006: **blocked** — Add expected-result coverage for texture pixel-format enumeration
- multi-hatch-release-001: **blocked** — Release lemmings per hatch on multi-hatch levels
- letsgo-text-nuke-ohno-001: **ready** — Restore the earlier Let's Go text; no Oh No on nuke
- fall-out-death-001: **ready** — Kill lemmings that fall into the hole on The Abyss
- crab-water-surface-001: **ready** — Crabs must not kill water lemmings walking on the surface above
- multi-hatch-simultaneous-001: **ready** — Release from all hatches simultaneously
- actor-occlusion-002: **ready** — Occlusion: round object on Loggerheads; lemming-versus-lemming order

## Recently completed queue entries

- ui-sounds-001: Add UI and assignment sounds; move Let's Go to first spawn
  Accepted: 2026-11-16T21:02:03.191807+00:00
  Verification evidence: `build/agent-runs/20261116T204700-1620-d14520ad/verification.log`
  Verification evidence: `build\agent-runs\20261116T204700-1620-d14520ad\verification.log`
- basher-occlusion-001: Occlusion pass: bashers in walls, turn-around at walls, feet through bridges
  Accepted: 2026-11-16T23:45:36.161457+00:00
  Verification evidence: `build/agent-runs/20261116T233514-46016-01e79e9a/verification.log`
  Verification evidence: `build\agent-runs\20261116T233514-46016-01e79e9a\verification.log`
- bash-particles-001: Show debris particles while bashing
  Accepted: 2026-11-16T23:58:45.291318+00:00
  Verification evidence: `build/agent-runs/20261116T234536-46016-089a2247/verification.log`
  Verification evidence: `build\agent-runs\20261116T234536-46016-089a2247\verification.log`
- builder-brick-anim-001: Show the brick growing out of the builder's pack
  Accepted: 2026-11-17T00:19:49.519636+00:00
  Verification evidence: `build/agent-runs/20261116T235845-46016-d0990c17/verification.log`
  Verification evidence: `build\agent-runs\20261116T235845-46016-d0990c17\verification.log`
- exit-balloon-bob-001: Make exit balloons bob slowly
  Accepted: 2026-11-17T00:38:44.614062+00:00
  Verification evidence: `build/agent-runs/20261117T003037-46016-c45dfbea/verification.log`
  Verification evidence: `build\agent-runs\20261117T003037-46016-c45dfbea\verification.log`

## Latest loop

- Result: `build/agent-runs/loop-e180d140b32b49c3b30eaaa3a58976ca/result.json`
- Status: **blocked**
- Duration: 1h 03m 32s
- Completed in this invocation: basher-occlusion-001, bash-particles-001, builder-brick-anim-001, exit-balloon-bob-001
- Reason: plan failed; inspect the saved logs

## Documentation handoff

At each accepted milestone or priority change, reconcile IMPLEMENTATION_PLAN.md and
docs/STATUS.md with the accepted review and its verification log. Keep detailed
per-run reports historical; append corrections rather than replacing their evidence.
A generated ledger does not update narrative conclusions or historical fidelity gates.
