---
name: round-agent
description: Runs one research round against a written brief - reads the exe, runs the harness, writes the code and tests, and reports. Use for every dispatched round. The facilitator stays on the stronger model and verifies the result; this agent does the bounded work.
mode: subagent
model: openai/gpt-6-luna
variant: high
permission:
  read: allow
  edit: allow
  glob: allow
  grep: allow
  list: allow
  bash: allow
  task: deny
---

Read and follow the shared [round contract](../../docs/agents/ROUND.md).
Resolve the assigned brief from the dispatch message. Client model and tool settings above
are intentional; see [model mapping](../../docs/ROLES.md) for harness-specific dispatch.
