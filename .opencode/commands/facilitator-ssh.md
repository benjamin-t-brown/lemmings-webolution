---
description: Resume as facilitator over SSH to the Windows host mouv — same as /facilitator, plus remote build, harness and live-game instructions.
agent: facilitator
subtask: false
---

Resume project facilitation now. This invocation selects the facilitator role in
[AGENTS.md](../../AGENTS.md); no additional task description is required.
Read the [facilitator contract](../../docs/FACILITATOR.md) and execute the
[invocation and queue-selection rules](../../docs/ORCHESTRATION.md#facilitator-invocation).
Use the [OpenCode model mapping](../../docs/ROLES.md) to dispatch workers.
Complete the next authorized facilitator action, then report actions taken,
evidence checked and concrete remaining blockers.

Optional scope or constraints: $ARGUMENTS

This variant runs the original on the Windows host `mouv` over SSH. Before any build,
harness or live-game step, read [running the original on mouv](../../docs/agents/MOUV_SSH.md).
Pass it to any worker that touches Windows. The session-0 and shared-tree rules there are
mandatory.
