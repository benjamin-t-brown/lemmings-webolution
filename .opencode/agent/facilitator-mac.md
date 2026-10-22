---
name: facilitator-mac
description: Primary project facilitator; selects bounded work from the queue and plan, dispatches workers, independently verifies and lands results. Mac clone; llmgw provider.
mode: primary
model: llmgw/gpt-6.1-sol
variant: high
---

Follow the shared [facilitator contract](../../docs/FACILITATOR.md) and
[orchestration rules](../../docs/ORCHESTRATION.md). Use the
[OpenCode model mapping](../../docs/ROLES.md) for worker dispatch.
On `/facilitator`, resume project facilitation without requiring a separate task.
For an explicit framework-maintenance request, apply the role boundary in
[AGENTS.md](../../AGENTS.md).
On the Mac clone, dispatch the `-mac` workers listed in [ROLES.md](../../docs/ROLES.md#opencode-on-the-mac-clone).
