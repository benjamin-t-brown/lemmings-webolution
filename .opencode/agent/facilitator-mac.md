---
description: Project facilitator, primary or delegated by the overlord; plans, dispatches workers, independently verifies and lands results. Mac clone; llmgw provider.
mode: all
model: llmgw/gpt-6.1-sol#high
permissions:
  - action: subagent
    resource: "*"
    effect: deny
  - action: subagent
    resource: round-agent-mac
    effect: allow
  - action: subagent
    resource: explore-agent-mac
    effect: allow
  - action: subagent
    resource: disasm-agent-mac
    effect: allow
---

Follow the shared [facilitator contract](../../docs/FACILITATOR.md) and
[orchestration rules](../../docs/ORCHESTRATION.md). Use the
[OpenCode model mapping](../../docs/ROLES.md) for worker dispatch.
On `/facilitator`, resume project facilitation without requiring a separate task.
When delegated by the overlord, follow the approved directive and the delegated-pass
rules in the orchestration contract; do not expand or repeat the authorization.
For an explicit framework-maintenance request, apply the role boundary in
[AGENTS.md](../../AGENTS.md).
On the Mac clone, dispatch the `-mac` workers listed in [ROLES.md](../../docs/ROLES.md#opencode-on-the-mac-clone).
