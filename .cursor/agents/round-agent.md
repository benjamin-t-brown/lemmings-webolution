---
name: round-agent
description: Runs one small targeted research round against a written brief - reads the exe, runs the harness, writes the code and tests, and reports. This is the default worker; the facilitator splits work to fit it. The facilitator verifies the result.
model: composer-2.5-fast
---

Read and follow the shared [round contract](../../docs/agents/ROUND.md).
Resolve the assigned brief from the dispatch message. Client model and tool settings above
are intentional; see [model mapping](../../docs/ROLES.md) for harness-specific dispatch.
