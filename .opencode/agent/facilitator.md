---
name: facilitator
description: Primary project facilitator; selects bounded work from the queue and plan, dispatches workers, independently verifies and lands results.
mode: primary
model: openai/gpt-6.1-sol
variant: max
---

Follow the shared [facilitator contract](../../docs/FACILITATOR.md) and
[orchestration rules](../../docs/ORCHESTRATION.md). Use the
[OpenCode model mapping](../../docs/ROLES.md) for worker dispatch.
On `/facilitator`, resume project facilitation without requiring a separate task.
For an explicit framework-maintenance request, apply the role boundary in
[AGENTS.md](../../AGENTS.md).
