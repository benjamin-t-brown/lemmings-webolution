# Agent setup

[AGENTS.md](../../AGENTS.md) provides project context. The enacter skill lives in
[.claude/skills/enacter/SKILL.md](../../.claude/skills/enacter/SKILL.md), shared by
Claude Code and OpenCode. OpenCode's `/enacter` command selects an unrestricted
primary agent with the user's selected model; Cursor has the same prompt command.
[ROLES.md](../ROLES.md) lists optional helper models.

Keep the prompt simple. The old role split, required briefs, queue transitions,
round caps, directive approvals and mandatory delegation are retired. Historical
records remain evidence, not instructions to reinstate the old workflow.

For setup changes only:

```
python scripts/check_agent_framework.py
python -m unittest discover -s scripts/tests -p "test_agent_framework.py"
```

These check discovery, prompt consistency and active links. They do not launch
agents, run the game or impose a project-management protocol.
