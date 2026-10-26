# OpenCode adapter

Model choices and dispatch instructions are maintained in [ROLES.md](../docs/ROLES.md).
Executable pins remain in [opencode.json](opencode.json),
[facilitator](agent/facilitator.md),
[round-agent](agent/round-agent.md) and [explore-agent](agent/explore-agent.md).
Shared behavior lives in the [framework contracts](../docs/agents/README.md).
Use [AGENTS.md](../AGENTS.md) to select the role from the user request.

Run `/overlord` for strategy, pass evaluation and approved directives. Its
[command](commands/overlord.md) selects the [primary overlord](agents/overlord.md)
in the current session, retaining the selected model. It does not resume the queue
or dispatch workers. A skill or attached Markdown file does not switch the selected
agent. If the UI still says facilitator, invoke `/overlord` (use `/reload` if the
new command has not appeared). With an approved directive and the contract's ownership
checks, it may launch the matching facilitator as a child. Manual launches remain supported.
Enabling this framework never resumes a stopped project run.

Run `/facilitator` in a session opened from this repository. No arguments are
required: it resumes the queue or selects the next bounded task from the plan.
The [command](commands/facilitator.md) explicitly selects the primary facilitator
agent, whose own configuration pins the model and `high` variant even when the current
session uses another model. Optional arguments constrain that work.
`@.opencode/commands/facilitator.md` only
attaches prompt text; it does not apply executable command frontmatter.

Configuration-only checks (no model request or project dispatch):

```powershell
opencode debug agents
opencode debug config
```

Expect facilitator profiles to have `mode: all`, their configured model and `high`
variant, and the
`facilitator` command's agent binding without a command-level model override. Resolved configuration verifies
discovery and pins; it does not prove a live worker dispatch or provider availability.
Both worker agents use the OpenAI Luna pin at `high` documented in `ROLES.md`;
the general session default also uses Luna, retaining its `max` variant. Explore
retains read-only permissions. OpenCode V2 reloads configuration automatically;
`/reload` forces a reload. Existing session agent selection is separate from discovery.

Nested dispatch is limited to overlord -> facilitator -> worker:
`experimental.subagent_depth: 2` plus explicit profile allowlists permit this chain.
The overlord cannot directly dispatch workers; facilitators cannot dispatch other
facilitators; workers cannot delegate. Mac and mouv facilitators allow only their own
worker profiles. Model pins are unchanged. Configuration resolution and framework
tests do not establish a successful live nested dispatch; no project agents need to
be launched merely to validate these files.
