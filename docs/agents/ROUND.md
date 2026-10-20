# Round worker contract

Execute only the assigned brief, then stop. Shared ownership, queue permissions, repair and
recovery rules are in [orchestration](../ORCHESTRATION.md); client pins are in
[roles](../ROLES.md). Do not dispatch agents or expand scope.

## Reading order

1. Read [mandatory guardrails](../AGENT_SAFETY.md).
2. Read the brief and matching item in [tasks.json](../tasks.json). Confirm the id and
   ownership; if another worker owns it, report the conflict rather than taking over.
3. Read the exact lesson headings, plan section and evidence sources listed in the brief.
   Search [LESSONS.md](../LESSONS.md) for related terms before investigating. Follow linked
   evidence for any claim you depend on. Read broader context if the findings require it.
4. Read build/REA/live-probe instructions only when using those capabilities.

If the brief lacks relevant reading references, locate the relevant sections with a bounded
search and report the gap. Do not substitute a facilitator hypothesis for original evidence.
Record your own prediction and falsifier before a live run. Report at the investigation/run
budget if unresolved; do not compensate by doing sibling tasks.

## Work and handoff

Implement, test and repair within the owned paths. Keep candidate prose under `build/tmp/`.
Return the evidence packet required by orchestration, including exact commands, failed
checks, original-byte reads, isolated regressions and restoration logs where applicable.
Distinguish measured, read and unsettled results without rounding or broadening claims.

Set only permitted queue fields for your matching item: `worker_output`, status to
`ready-for-verify` or `blocked` with reason, and clear your `live_owner` on release.
Do not create tasks, mark landed, clear active, or promote documentation. Report game and
instrumentation state at handoff; release only resources owned by this task.
If interrupted, leave exact completed/remaining work and artifact paths for continuation.
