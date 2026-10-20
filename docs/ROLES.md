# Roles and models, per harness

This repo is driven from more than one agent harness. Roles are defined **by function**
here; each harness's own entry point maps them to its model names. This table is the only
place a vendor model name belongs — do not hardcode one in `PLAN.md`,
`docs/FACILITATOR.md` or a rules file, because a string that is right in one client is
invalid in another.

Session architecture (queue, dispatch loop, permissions): **`docs/ORCHESTRATION.md`**.
In-flight work: **`docs/tasks.json`**.

## The roles

| Role | Tier | What it does |
| --- | --- | --- |
| **Facilitator / planner** | strong | Picks the next round, **splits it into small independently verifiable tasks**, writes the brief, updates `docs/tasks.json`, **independently verifies every headline claim**, owns `PLAN.md`, `docs/**` and the rules files, lands results. Verification never delegates. |
| **Hard-task worker** | mid | Runs one **escalated** brief that cannot be reduced to the targeted tier: cross-cutting reverse engineering, ambiguous evidence, or a targeted attempt that already failed. The facilitator records why it was escalated. Does not land docs or dispatch agents. |
| **Targeted worker** — default | cheap/fast | Runs one **small** round against a written brief: reads the exe, runs the harness, writes the code and tests, reports under `build/tmp/`. This is the default tier for every round. Does not land documentation and does not dispatch further agents. |
| **Explore agent** — read-mostly | cheapest | Answers one “where is X?” map question. No production edits, no live multi-step measurement, no doc landing. Optional before a brief; not a substitute for a round. |

## Task sizing: cheap by default

The facilitator's first job is decomposition, not dispatch. A brief should be small enough
for the **targeted** tier to finish in one session: one question, one falsifier, one
ownership area, a bounded command/read list, and a candidate note that fits comfortably in
the worker's context. If it cannot be stated that way, split it into more queue items.

Signals a brief is still too big:

- it spans more than one subsystem or file area;
- it needs more than a handful of live runs, or unbounded search;
- it has no single acceptance check, or the check depends on taste rather than a
  measurement;
- it contains “and also”.

Escalate to the **hard** tier only when the work genuinely cannot be split: ambiguous
cross-cutting reverse engineering, a targeted attempt that failed for a reason other than
size, or a change whose failure mode needs broad judgement. Record the escalation reason in
the brief and the queue item. A failed targeted attempt is a signal to split the task
first, not automatically to escalate it.

The split exists to save time and money: many small targeted rounds are cheaper and easier
to verify than one large round, and a dead worker loses little work. Judgement — whether a
claim is actually supported, whether it over-generalises, whether a document has gone stale
— stays on the strong tier.

A cheap “reviewer” may lint or summarize a diff. **It is not verification.** Address
re-reads and pixel/behaviour gates stay with the facilitator.

## Harness mapping

| Harness | Facilitator / planner | Hard-task worker | Targeted worker (default) | Explore | Configured in |
| --- | --- | --- | --- | --- | --- |
| **Claude Code** | `opus` via `/facilitator` | `sonnet`, dispatched explicitly for a recorded hard round | `haiku` via `.claude/agents/round-agent.md` | `haiku` via `.claude/agents/explore-agent.md` | `/facilitator` → `.claude/commands/facilitator.md`; agents under `.claude/agents/`. Facilitator model is the session launch model. If the harness cannot override the agent pin, dispatch the same brief to a general worker on `sonnet`. |
| **Codex** | `astra` | `sol`, dispatched explicitly for a recorded hard round | `luna` — default worker pin | `luna`, read-only brief | `AGENTS.md`; REA MCP in `~/.codex/config.toml`. No in-repo agent pin file. |
| **OpenCode** | `openrouter/~deepseek/deepseek-flash-latest` at `max` via `@.opencode/commands/facilitator.md` | same round pin (no separate hard tier) | `openrouter/deepseek/deepseek-v4.1-flash` at `high` via `.opencode/agent/round-agent.md` | `opencode/deepseek-v4.1-flash-free` at `high` via `.opencode/agent/explore-agent.md` | `.opencode/opencode.json` + `.opencode/commands/` + `.opencode/agent/*.md` |
| **Cursor** | **Claude Opus 5.5** via `/facilitator` (chat picker, e.g. `claude-opus-5-5-medium`) | same Cursor-pool worker as targeted — **do not** escalate to Opus; split the brief instead | Task → `round-agent` pinned to `composer-2.5-fast` | Task → `explore-agent` pinned to `composer-2.5-fast` | `/facilitator` → `.cursor/commands/facilitator.md`; workers in `.cursor/agents/*.md`; `.cursor/rules/*.mdc` stay **pointers only**. |

For **Cursor**: start the facilitator chat on **Claude Opus 5.5**. Workers are pinned in
`.cursor/agents/` to **`composer-2.5-fast`** (Cursor included pool). Auto is a parent-chat
mode only — it cannot be set on a custom subagent — so the worker pin is Composer, not
`inherit` (inherit would keep workers on Opus). Hard rounds stay on the same Composer pin;
split the brief instead of escalating the model. When dispatching via Task, do not pass a
third-party `model` override.

For OpenCode, `high` and `max` are provider-specific reasoning-effort variants, set with the
`variant` field. The round worker uses **DeepSeek V4.1 Flash** through OpenRouter at `high`;
the explore worker uses the OpenCode-provider **DeepSeek V4.1 Flash Free** at `high` for
map-only work. The facilitator stays on the existing Flash Latest `max` configuration.
OpenCode has no separate hard-tier pin; an unusually hard round still uses the same round
pin, or the work is split instead.

Both worker pins are explicit and independent of the facilitator session. If a pinned model
is unavailable or rate-limited, record the blocker and report it; do not silently switch
pins.

Verified 2026-10-19 against the local OpenCode model catalog and the
[official model list](https://dev.opencode.ai/v2/docs/console/models/). The free DeepSeek
entry advertises a 200,000-token context and 32,000-token output limit, with zero input,
output and cache-read prices. Free availability is time-limited by the provider, so
re-check the IDs when updating these pins.

Claude Code's `Agent` tool accepts only `sonnet`, `opus`, `haiku` and `fable` — a Codex name
passed there is rejected outright. That is why the mapping is a table and not a sentence.

## One canonical copy of everything else

The evidence standard and the accumulated findings live in **`.claude/rules/`** and
**`docs/LESSONS.md`**. Those are canonical for every harness.

**Do not copy their content into another client's rules directory.** On 2026-10-18 the
Cursor mirror was found frozen at a pre-round-10 state: it still said `climbSpeed` was
“2 mask rows” — the retired factor-of-2 doubling this project calls its most repeated
mistake — offered `fallSpeed = 3` as a live hypothesis when it had been measured as 4, and
gave superseded crosshair offsets. Anything run in Cursor was reading actively wrong rules.

`.cursor/rules/*.mdc` are **pointers with no rules in them**, so they cannot drift. If you
add a harness, add a pointer, not a copy.
