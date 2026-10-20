# Agent framework maintenance

## Canonical sources

| Concern | Owner |
| --- | --- |
| Entry point and role selection | [AGENTS.md](../../AGENTS.md) |
| Roles, ownership, sizing, dispatch, queue and repair | [ORCHESTRATION.md](../ORCHESTRATION.md) |
| Independent verification procedure | [FACILITATOR.md](../FACILITATOR.md) |
| Round and explore execution | [ROUND.md](ROUND.md), [EXPLORE.md](EXPLORE.md) |
| Client model choices and wiring | [ROLES.md](../ROLES.md) |
| Mandatory project guardrails | [AGENT_SAFETY.md](../AGENT_SAFETY.md) |
| Evidence authority | [Canonical evidence rules](../../.claude/rules/match-the-original.md) |
| Brief structure | [Template](../briefs/TEMPLATE.md) |
| Historical checkpoints | [Facilitator history](../FACILITATOR_HISTORY.md) |

Client frontmatter is executable configuration; retain its literal model and permission
settings (Claude Code workers deliberately have no model; see [ROLES.md](../ROLES.md)). Client bodies point to shared contracts. Change a shared rule in its owner,
not in every client. Model changes require checking client support separately; this
refactor does not change pins or claim current provider availability.

The safety page summarizes mandatory traps and links to the canonical evidence rules.
It is not a replacement for reading evidence relevant to a claim. Historical findings
belong in lessons and evidence notes, not role contracts.

## Checks

Run from the repository root with the normal Python interpreter:

```powershell
python scripts/check_agent_framework.py
python scripts/check_agent_framework.py --brief build/tmp/example-brief.md
python -m unittest discover -s scripts/tests -p "test_agent_framework.py"
```

The checker reads files only. It checks local Markdown links and heading anchors in
the active framework, adapter contract pointers/model declarations, queue structure,
live-owner consistency, and new brief fields and ownership. Historical notes and legacy
completed briefs are not linted as current contracts. An in-flight legacy brief is checked
for existence; strict brief validation is opt-in before a new dispatch.

Facilitator commands must link to the canonical invocation/queue-selection rule.
The OpenCode check also requires a primary facilitator agent, its explicit
model/variant, and matching command binding in the current session, so a command
cannot silently inherit an unrelated session model.

For machine validation, write the brief's **Owned files/paths** as comma-separated
repository-relative paths or globs in backticks, and **Owns live game** as `yes` or `no`.
List candidate outputs there too. Queue-field permission is supplied by orchestration,
not by granting ownership of `docs/tasks.json`. Reading references use ordinary Markdown
links, resolved relative to the brief.
Explore briefs use `none` for owned paths. Checked worker areas are `src/`, `tools/harness/`,
`build/tmp/`, `scripts/`, `cmake/` and `CMakeLists.txt`; extend the checker deliberately
when a new legitimate code area is introduced.

Checks cannot prove evidence quality, runtime dispatch, semantic agreement between prose,
or compliance by an agent. The facilitator still reviews those. Keep proposed framework
edits isolated during another session's work; compare against current files before applying.
