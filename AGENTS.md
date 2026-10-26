# Lemmings Webolution - agent entry point

A 1:1 reimplementation of Lemmings Revolution. The stock 640x480
`Lemmings Revolution.exe` is the authority. A claim needs original bytes at an address
or a re-runnable script; decompiler prose is not a measurement. Work on `decompile`.

## Select the role from the user's request

- `/facilitator` explicitly selects project facilitation and authorizes resuming the
  project from its queue and plan; no additional task text is required.
- An overlord-delegated facilitator pass selects the same role, bounded by its
  approved directive and the ownership checks in orchestration. Merely enabling
  delegation or reviewing the framework does not authorize project work.
- Project planning/research/implementation: read [facilitator contract](docs/FACILITATOR.md).
- `/overlord`, or a request to direct facilitators, set macro direction or evaluate their
  passes: read [overlord contract](docs/OVERLORD.md). This does not authorize facilitation.
- Assigned round: read [worker contract](docs/agents/ROUND.md) and the assigned brief.
- Assigned map-only question: read [explore contract](docs/agents/EXPLORE.md).
- Framework maintenance/review: inspect only relevant framework files. This does not
  authorize resuming game work, dispatching the project queue or controlling the game.

Read [orchestration](docs/ORCHESTRATION.md) for ownership and session boundaries.
Do not take over another session's running work without a confirmed handoff or termination.
Keep framework changes isolated while a project session is running; activate between rounds.

## Sources of truth

| Topic | Source |
| --- | --- |
| Current work and owner | [Task queue](docs/tasks.json) |
| Goals | [Plan](PLAN.md) |
| Round history | [Plan history](docs/plan-history/phase-c.md) (one file per phase) and [queue archive](docs/tasks-archive.json) |
| Established facts | [Lessons](docs/LESSONS.md) and linked evidence |
| Mandatory project guardrails | [Agent safety](docs/AGENT_SAFETY.md) |
| Client model pins and launch wiring | [Roles](docs/ROLES.md) |
| New task briefs | [Brief template](docs/briefs/TEMPLATE.md) |
| Framework maintenance and checks | [Framework guide](docs/agents/README.md) |

Role contracts specify reading scope; workers need not load all historical checkpoints.
`old/` preserves retired plans and notes. Old code references to `docs/scratch/phase3-*`
resolve under `old/docs/scratch/`. Do not rediscover settled work.
