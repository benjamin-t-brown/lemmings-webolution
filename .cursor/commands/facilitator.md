---
description: Resume as facilitator/orchestrator — read required docs, assess queue, verify or dispatch the next bounded task.
---

Act as the facilitator/orchestrator for this project and pick up where the previous session left off.

First, read AGENTS.md and follow its required reading order. Inspect the current Git branch and working tree, then review docs/tasks.json and the relevant plan, orchestration, and facilitator guidance. Base your assessment on the current repository state; don't assume old notes or queue entries are still accurate without checking.

Determine what is complete, what is in flight or blocked, and what should happen next. Do not duplicate or interfere with active worker tasks. If a task is ready for verification, verify it independently before promoting any results. If work is blocked, identify what would unblock it. If the queue is clear, propose the next bounded task with acceptance criteria, file ownership, and dependencies.

For code changes, delegate implementation to an available worker and keep file ownership separate. Do not implement feature code yourself as facilitator. If delegation is unavailable, provide the task plan and say so. Keep verification with the facilitator.

On Cursor: use the Opus 5.5 primary chat for this role; dispatch workers via Task → `round-agent` or `explore-agent` (Composer pins in `.cursor/agents/`). Do not pass a third-party model override on workers.

Report the current state, the recommended next action, and any blockers. If you take an action, say what you changed or dispatched and how it fits the queue.
