# Roles and models, per harness

This repo is driven from more than one agent harness. Roles are defined **by function**
here; each harness's own entry point maps them to its model names. This table is the canonical prose
source for vendor model names — do not hardcode one in `PLAN.md`,
`docs/FACILITATOR.md` or a rules file, because a string that is right in one client is
invalid in another.

Session architecture (queue, dispatch loop, permissions): **`docs/ORCHESTRATION.md`**.
In-flight work: **`docs/tasks.json`**.

Role behavior, task sizing and escalation policy live in [ORCHESTRATION.md](ORCHESTRATION.md).
Round and explore contracts live under [agents/](agents/README.md). This file owns only
client-specific model choices and dispatch wiring; literal pins also live in executable
adapter frontmatter/configuration, where the harness requires them.

## Harness mapping

| Harness | Facilitator / planner | Hard-task worker | Targeted worker (default) | Explore | Configured in |
| --- | --- | --- | --- | --- | --- |
| **Claude Code** | session launch model via `/facilitator` | session model (no pin; split instead of escalating) | session model via `.claude/agents/round-agent.md` (no pin) | session model via `.claude/agents/explore-agent.md` (no pin) | `/facilitator` → `.claude/commands/facilitator.md`; agents under `.claude/agents/`. No `model` in frontmatter and no `model` argument on dispatch. |
| **Codex** | `astra` | `sol`, dispatched explicitly for a recorded hard round | `luna` — default worker pin | `luna`, read-only brief | `AGENTS.md`; REA MCP in `~/.codex/config.toml`. No in-repo agent pin file. |
| **OpenCode** | `openai/gpt-6.1-sol` at `high` via `/facilitator` → `.opencode/agent/facilitator.md` (Mac clone: [`/facilitator-mac`](#opencode-on-the-mac-clone)) | `disasm-agent` (Sol) for disassembly reads; otherwise the round pin | `openai/gpt-6-luna` at `high` via `.opencode/agent/round-agent.md` | `openai/gpt-6-luna` at `high` via `.opencode/agent/explore-agent.md` | `.opencode/opencode.json` + `.opencode/commands/` + `.opencode/agent/*.md` |
| **Cursor** | **Claude Opus 5.5** via `/facilitator` (chat picker, e.g. `claude-opus-5-5-medium`) | same Cursor-pool worker as targeted — **do not** escalate to Opus; split the brief instead | Task → `round-agent` pinned to `grok-4.7-high-fast` | Task → `explore-agent` pinned to `grok-4.7-high-fast` | `/facilitator` → `.cursor/commands/facilitator.md`; workers in `.cursor/agents/*.md`; `.cursor/rules/*.mdc` stay **pointers only**. |

For **Claude Code**: there is **no model routing**. Workers carry no `model` frontmatter and are
dispatched without a `model` argument, so they run on the session model. Removed 2026-10-20: the
`haiku` and then `sonnet` pins kept misrouting through the Bedrock gateway (workers silently on Opus,
named teammates ignoring pins, intermittent `LLM Provider NOT provided` failures on the pinned
name). Split a hard brief rather than escalating. The checker rejects a `model` line in
`.claude/agents/*.md`.

For **Cursor**: start the facilitator chat on **Claude Opus 5.5**. Workers are pinned in
`.cursor/agents/` to **`grok-4.7-high-fast`**. Auto is a parent-chat mode only — it cannot
be set on a custom subagent — so the worker pin is explicit, not `inherit` (inherit would
keep workers on Opus). Hard rounds stay on the same Grok pin;
split the brief instead of escalating the model. When dispatching via Task, do not pass a
third-party `model` override.

Changed 2026-10-21 at the user's request: Cursor workers moved from `composer-2.5-fast` to
Grok. On A7m, the Composer worker launched the game four times against an authorization of
two, and used a raw camera write that the brief excluded. The ID `grok-4.7-high-fast` is the
newest Grok ID in mouv's Cursor storage. It was not checked with a dispatch; if Cursor
rejects it, record the blocker rather than switching back silently.

For OpenCode, `high` and `max` are provider-specific reasoning-effort variants, set with the
`variant` field. The facilitator uses **GPT 6.1 Sol** through OpenAI at `high`.
Changed 2026-10-22 at the user's request: `max` was spending too many tokens.
Both the round and explore workers use **GPT 6 Luna** through OpenAI
(`openai/gpt-6-luna`) at `high`; explore remains read-only and map-only. The general
session default in `.opencode/opencode.json` also uses Luna, retaining its `max`
variant, and is separate from the facilitator agent pin.
OpenCode has no separate hard-tier pin; an unusually hard round still uses the same round
pin, or the work is split instead.

For OpenCode, invoke `/facilitator` as a command. Its frontmatter binds the primary
`facilitator` agent; that agent pins `openai/gpt-6.1-sol` and `variant: high`,
overriding the model already selected in a session. The command
has no separate model override, so OpenCode resolves the agent's provider/model ID
and variant directly. Reading or attaching the Markdown file does not apply command
frontmatter. Verified the agent pin with local OpenCode 1.19.3 `debug agent facilitator`;
this makes no model request. OpenCode's command guide says the selected agent's
configured model takes effect when the command runs:
[command execution](https://dev.opencode.ai/v2/docs/commands/#execution).

Both worker pins are explicit and independent of the facilitator session. If a pinned model
is unavailable or rate-limited, record the blocker and report it; do not silently switch
pins.

Changed 2026-10-20 at the user's request: both OpenCode workers now use OpenAI Luna
instead of DeepSeek, and the general session default was changed to Luna too.
The facilitator remains on Sol. Use `opencode debug agent round-agent`,
`opencode debug agent explore-agent` and `opencode debug config` to verify the
resolved models and variants; configuration resolution does not prove provider
availability or a completed dispatch. Historical queue dispatch records are not
rewritten by this framework change.
Quit and restart OpenCode after changing agent configuration: an already-running
session keeps its loaded settings.

### OpenCode on the Mac clone

Added 2026-10-22 at the user's request. The Mac runs `opencode2` (2.0.16) from a local
clone at `~/progs/lemmings-webolution`, not over the sshfs mount. Its only provider is
`llmgw`: the gateway plugin removes every other provider, so the `openai/` pins above do
not resolve there. The Mac uses its own provider through a parallel set of agents.
mouv keeps the set above, unchanged.

| Role | Mac agent | Pin |
| --- | --- | --- |
| Facilitator | `/facilitator-mac` → `.opencode/agent/facilitator-mac.md` | `llmgw/gpt-6.1-sol` at `high` |
| Targeted worker | `round-agent-mac` | `llmgw/gpt-6-luna` at `high` |
| Explore | `explore-agent-mac` | `llmgw/gpt-6-luna` at `high` |
| Disassembly read | `disasm-agent-mac` | `llmgw/gpt-6.1-sol` at `high` |

The gateway offers `low`, `medium`, `high` and `xhigh` for these models, not `max`. Both
models answered through `llmgw` on 2026-10-22. On the Mac, dispatch the `-mac` agents
only; an `openai/` agent fails there. Environment and global-config overrides do not
replace a project agent's pin (checked 2026-10-22), which is why these are separate agents.

**Sync is through GitHub** (`origin`, branch `decompile`), at the user's request. This is
the one exception to "no push" in the standing run, and it applies to both clones:
`git pull --ff-only` before selecting work, and `git push origin decompile` after each round
commit. Never force-push. If the pull does not fast-forward, stop and report it.
`build/tmp/` is gitignored, yet it holds the briefs and round evidence that `tasks.json`
cites (about 1.4 GB on mouv). Git cannot carry it, so the Mac clone's `build/tmp` is a symlink to mouv's
copy through the sshfs mount (`~/mouv-progs/lemmings-webolution/build/tmp`). There is one
scratch tree, and the mount's `._*` and temp-file rules in
[MOUV_SSH.md](agents/MOUV_SSH.md) apply to it. Put Mac-only build output in `build/mac`.
A round that needs the original, a Windows build or a Windows-only script still runs on
mouv, per [running the original on mouv](agents/MOUV_SSH.md). Push before the mouv step,
and pull mouv's result back afterwards. The Mac clone builds natively (`cmake -S . -B
build/mac`); CTest passes there, but `lrtool` does not compile under Apple clang
(`constexpr std::sin`), and the authoritative build is still mouv's.

## Disassembly reads use a stronger worker

A round whose main work is reading the original's machine code (tracing what a function at
an address does, a draw order, a filter or a data flow) is where a wrong read costs the
most rounds later. User decision 2026-10-20: dispatch these on a stronger model, and mark
the brief and queue entry `disassembly read`. Scoring, rendering and capture rounds keep
the normal worker pin.

| Harness | Disassembly-read dispatch |
| --- | --- |
| **OpenCode** | Task → `disasm-agent` (`.opencode/agent/disasm-agent.md`), pinned to `openai/gpt-6.1-sol` at `high`; same round contract |
| **Codex** | `sol`, dispatched explicitly |
| **Claude Code** | unchanged: `round-agent` on the session model, no `model` argument |
| **Cursor** | unchanged: `grok-4.7-high-fast`; split the brief |

If a pinned worker is unavailable, record the blocker and report it; do not silently fall
back to the normal round pin.

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
