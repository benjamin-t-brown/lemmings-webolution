# Roles and models, per harness

This repo is driven from more than one agent harness. Roles are defined **by function**
here; each harness's own entry point maps them to its model names. This table is the only
place a vendor model name belongs — do not hardcode one in `PLAN.md`,
`docs/FACILITATOR.md` or a rules file, because a string that is right in one client is
invalid in another.

## The two roles

| Role | What it does |
| --- | --- |
| **Facilitator** — strong tier | Picks the next round, writes the brief, **independently verifies every headline claim**, owns `PLAN.md`, `docs/**` and the rules files, lands results. Verification never delegates. |
| **Round agent** — cheap/fast tier | Runs one round against a written brief: reads the exe, runs the harness, writes the code and tests, reports back. Does not land documentation and does not dispatch further agents. |

The split exists to save time and money: a round is bounded work against a written brief,
where the cheaper tier is fast enough to run many; the judgement — whether a claim is
actually supported, whether it over-generalises, whether a document has gone stale — is
where the strong tier earns its cost.

## Harness mapping

| Harness | Facilitator | Round agent | Configured in |
| --- | --- | --- | --- |
| **Claude Code** | `opus` | `sonnet` | `.claude/agents/round-agent.md` pins `model: sonnet`. The facilitator's model is whatever the session was launched with — it cannot be pinned from inside the repo. |
| **Codex** | `sol` | `luna` | `AGENTS.md`, plus `~/.codex/config.toml` for the REA MCP server. |
| **Cursor** | strong tier | cheap tier | `.cursor/rules/*.mdc`. No agent-definition mechanism, so the split is by convention. |

Claude Code's `Agent` tool accepts only `sonnet`, `opus`, `haiku` and `fable` — a Codex name
passed there is rejected outright. That is why the mapping is a table and not a sentence.

## One canonical copy of everything else

The evidence standard and the accumulated findings live in **`.claude/rules/`** and
**`docs/LESSONS.md`**. Those are canonical for every harness.

**Do not copy their content into another client's rules directory.** On 2026-10-18 the
Cursor mirror was found frozen at a pre-round-10 state: it still said `climbSpeed` was
"2 mask rows" — the retired factor-of-2 doubling this project calls its most repeated
mistake — offered `fallSpeed = 3` as a live hypothesis when it had been measured as 4, and
gave superseded crosshair offsets. Anything run in Cursor was reading actively wrong rules.

`.cursor/rules/*.mdc` are now **pointers with no rules in them**, so they cannot drift. If
you add a harness, add a pointer, not a copy.
