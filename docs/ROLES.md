# Enacter and optional subagents

Enacter uses the model selected by the user. Its prompt is:

> Understand and continue this project to completion.  Make use of subagents at lower intelligence tiers to assist you where possible.

Work directly or delegate as useful; no intermediary role or formal brief is needed.
Use a cheaper model for well-scoped work and a stronger model when the problem
benefits from it. The existing helpers are available, not mandatory stages:

| Client | Research / implementation | Read-only exploration | Disassembly help |
| --- | --- | --- | --- |
| OpenCode on mouv | `round-agent`: `openai/gpt-6-luna`, high | `explore-agent`: `openai/gpt-6-luna`, high | `disasm-agent`: `openai/gpt-6.1-sol`, high |
| OpenCode Mac clone | `round-agent-mac`: `llmgw/gpt-6-luna`, high | `explore-agent-mac`: `llmgw/gpt-6-luna`, high | `disasm-agent-mac`: `llmgw/gpt-6.1-sol`, high |
| Cursor | `round-agent`: `grok-4.7-high-fast` | `explore-agent`: `grok-4.7-high-fast` | Same helper, or work directly |
| Claude Code | `round-agent`, inherits session model | `explore-agent`, inherits session model | Same helper, or work directly |
| Codex | Choose an available lower-tier model for the assignment | Same, read-only | Choose a stronger model if useful |

Claude Code's previous explicit worker pins misrouted through its gateway, so they
remain unpinned. OpenCode helpers keep their independent pins; launching a Mac
helper does not move work to another machine. Use the provider available locally.
The main agent is not forced to delegate or blocked from solving a helper's problem.

The Mac clone is `~/progs/lemmings-webolution`. Sync with mouv through GitHub on
`decompile` using fast-forward pulls and ordinary pushes when authorized; never
force-push. Shared scratch uses the existing `build/tmp` sshfs link. Windows-only
work runs on mouv; [MOUV_SSH.md](agents/MOUV_SSH.md) has the connection details.

The former role contracts are retired under `old/docs/agent-framework/`. Past
directives and queue entries retain their historical budgets and results but do
not impose an approval loop on enacter.
