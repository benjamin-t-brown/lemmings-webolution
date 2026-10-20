# OpenCode harness pins

- **Primary session** (facilitator): `opencode.json` → DeepSeek Flash Latest at `max`.
- **Round worker**: `agent/round-agent.md` → `openrouter/deepseek/deepseek-v4.1-flash`
  (DeepSeek V4.1 Flash via OpenRouter) at `high`, `task: deny`.
- **Explore worker**: `agent/explore-agent.md` → `opencode/deepseek-v4.1-flash-free`
  (DeepSeek V4.1 Flash Free) at `high`, `edit: deny`, `task: deny`.

Dispatch the named `round-agent` or `explore-agent` so its explicit model pin is used.
Workers use their explicit pins independently of the primary session's model.
If a pinned model is unavailable, report the blocker rather than choosing a fallback.
Both workers use `high` effort.

Shared queue and dispatch loop: `docs/tasks.json`, `docs/ORCHESTRATION.md`.
Model table for all clients: `docs/ROLES.md`. Do not copy `.claude/rules/` here.
