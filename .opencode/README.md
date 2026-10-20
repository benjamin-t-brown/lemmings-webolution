# OpenCode adapter

Model choices and dispatch instructions are maintained in [ROLES.md](../docs/ROLES.md).
Executable pins remain in [opencode.json](opencode.json),
[facilitator](agent/facilitator.md),
[round-agent](agent/round-agent.md) and [explore-agent](agent/explore-agent.md).
Shared behavior lives in the [framework contracts](../docs/agents/README.md).
Use [AGENTS.md](../AGENTS.md) to select the role from the user request.

Run `/facilitator` in a session opened from this repository. No arguments are
required: it resumes the queue or selects the next bounded task from the plan.
The [command](commands/facilitator.md) explicitly selects the primary facilitator
agent, whose own configuration pins the model and `max` variant even when the current
session uses another model. Optional arguments constrain that work.
`@.opencode/commands/facilitator.md` only
attaches prompt text; it does not apply executable command frontmatter.

Configuration-only checks (no model request or project dispatch):

```powershell
opencode debug agent facilitator
opencode debug agent round-agent
opencode debug agent explore-agent
opencode debug config
```

Expect the primary facilitator's configured model and `max` variant, and the
`facilitator` command's agent binding without a command-level model override. Resolved configuration verifies
discovery and pins; it does not prove a live worker dispatch or provider availability.
Both worker agents use the OpenAI Luna pin at `high` documented in `ROLES.md`;
the general session default also uses Luna, retaining its `max` variant. Explore
retains read-only permissions. Quit and restart OpenCode after changing these pins
so the running session loads the new agent configuration.
