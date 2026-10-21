---
name: disasm-agent
description: Runs one disassembly-read round (tracing what code in the original executable does) against a written brief. Same contract as round-agent; pinned to the stronger model because a wrong read costs the most later rounds.
mode: subagent
model: openai/gpt-6.1-sol
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
