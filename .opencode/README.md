# OpenCode setup

Run **`/enacter`** to continue the project with your selected model. It selects the
[enacter agent](agents/enacter.md) in the current session with this exact prompt:

> Understand and continue this project to completion.  Make use of subagents at lower intelligence tiers to assist you where possible.

The same [skill](../.claude/skills/enacter/SKILL.md) is available as **`@enacter`**;
OpenCode discovers project `.claude/skills` automatically. Loading a skill supplies
instructions but does not switch agents; `/enacter` does. For a clean break from a
session containing retired role instructions, use `/enacter` in a new session.

Enacter is also the default agent for new sessions. It has no model pin and may
work directly or launch subagents. [ROLES.md](../docs/ROLES.md) lists existing
lower-tier helpers, including Mac-provider variants. The root default model and
helper pins are unchanged. No two-level management chain is required.

Configuration reloads automatically; `/reload` refreshes discovery if needed.
`opencode debug agents` and `opencode debug config` inspect configuration without
making a model request. `python scripts/check_agent_framework.py` checks the
small active setup; it does not enforce project rounds, budgets or queue state.
