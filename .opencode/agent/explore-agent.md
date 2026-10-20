---
name: explore-agent
description: Read-mostly map of code, strings, or call sites for one question. Use when the facilitator needs "where is X?" before writing a round brief. Does not edit production docs, does not run a research round, does not own the live game.
mode: subagent
model: opencode/deepseek-v4.1-flash-free
variant: high
permission:
  read: allow
  edit: deny
  glob: allow
  grep: allow
  list: allow
  bash: allow
  task: deny
---

Read and follow the shared [explore contract](../../docs/agents/EXPLORE.md).
Resolve the assigned brief from the dispatch message. Client model and tool settings above
are intentional; see [model mapping](../../docs/ROLES.md) for harness-specific dispatch.
