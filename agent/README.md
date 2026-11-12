# Native reconstruction agent setup

Read root `AGENTS.md` first. Shared instructions apply to Codex and Claude;
`CLAUDE.md` imports them. The user's active repository is this checkout,
including `decomp-lemmings/`, rather than the old standalone copy.

## Files

| File | Purpose |
| --- | --- |
| `IMPLEMENTATION_PLAN.md` | Inspection findings, sequence and acceptance |
| `tools.windows.json` | Observed executable paths, versions, reference identity and boundaries |
| `tasks.json` | Task status, dependencies, scope and acceptance criteria |
| `completion.json` | Required native completion criteria; protected from worker edits |
| `../tools/agent_control/continuous.py` | Sequential planning, implementation, verification and independent review loop |
| `../tools/agent_control/supervise.py` | One bounded Claude Code or Codex CLI invocation |
| `../tools/agent_control/verify.py` | GCC source builds/tests for main and recovery trees |
| `../tools/agent_control/test_control.py` | Synthetic supervisor/process-tree tests |
| `../tools/agent_control/test_continuous.py` | Synthetic loop, review, retry and completion tests |

## Verify from PowerShell at the repository root

```powershell
python tools/agent_control/test_control.py
python tools/agent_control/test_continuous.py
python tools/agent_control/verify.py --profile all
```

The verifier uses absolute tool paths, prepends only the selected UCRT64 runtime
for its child processes, and forces SDL dummy video/audio. It reads the reference
EXE hash, parses assets in place, builds source executables and runs CTest. It
does not launch the original game, install hooks or execute native original-byte
oracles. Source builds live in `build/agent-native/`; CTest inventory and JUnit
files are retained there. This verifier follows the existing build rule's
process-local UCRT64 PATH approach; it does not change machine PATH or use it
for live-game harness Python. Nonzero exits and zero registered tests fail.

Build parallelism defaults to 12 jobs (`--jobs`). Tests run one at a time by
default (`--test-jobs 1`): the rendering-heavy `levelreturn` test approached its
then-current timeout under concurrent testing while the unchanged serial suite passed.
This changes scheduling, not test assertions, registration or timeouts. Increase
`--test-jobs` only when the machine can sustain the rendering workload.

Reference and asset paths in the registry are relative to this repository root:
`Lemmings Revolution/Lemmings Revolution.exe`, `lemmings.box` and `patch.box`
inside the same folder. The verifier resolves them independently of the caller's
current directory and refuses game paths outside the repository. The existing
local files were hash-checked against the former external copies. The local game
folder is ignored, and the EXE is not tracked; previously tracked installation
files remain a separate cleanup debt. Legacy harness defaults must use explicit
repo-local paths until migrated to this registry.

## Continuous work

Preview the next action without launching AI or changing the queue:

```powershell
python tools/agent_control/continuous.py --backend codex
```

Start the loop after reviewing existing edits:

```powershell
python tools/agent_control/continuous.py --backend codex --execute --allow-dirty
```

Use `--backend claude` for Claude Code. Optional `--review-backend claude`
or `--review-backend codex` selects the provider for separate planning and review
passes. Otherwise all passes use the worker provider in fresh invocations.

Models and effort are pinned in `tools.windows.json` under `agent_models`:

| Backend | Model ID | Effort |
| --- | --- | --- |
| Codex | `gpt-6.1-sol` | `high` |
| Claude | `claude-sonnet-5-5` | `high` |

These apply to implementation, planning and review, including single-task runs.
Preview and continuous-run results show the requested settings; worker logs
retain the explicit CLI arguments. Change the registry only while the supervisor
is stopped. Missing pins are rejected instead of using a CLI default. A conflicting
`CLAUDE_CODE_EFFORT_LEVEL` is rejected because it can override the CLI flag.
Provider access is not verified by preview; organization restrictions may cap
the requested effort. Exact model support still depends on the installed CLI
and account. Sonnet 5.5 requires Claude Code 2.1.284 or later. High effort is retained for
implementation, planning and review.

Model recommendations are advisory: routine -> Sonnet, difficult -> Opus,
exceptional unresolved research -> Fable with an explicit bounded approval.
The existing planning pass records the grade, model and reason in task scope;
there is no extra grading call or automatic pin change. Your explicit choice
always wins for the task, role or run you specify, regardless of grade. See
AGENTS.md, "Model recommendations and user overrides". The current milestone uses Claude Sonnet5.5/high. No run is started by a recommendation.

The loop reviews existing `needs_review` work first, then runs eligible `queued`
or `ready` tasks with completed dependencies. When no eligible task remains,
the planner proposes one bounded next task or refines a `planning_required`
placeholder. New proposals must describe a missing runtime behavior and name a
production implementation file. Standalone test, documentation, digest or build
registration tasks are no longer eligible; checks support the runtime change.
The reviewer must trace its real call path and before/after behavior. After
implementation the verifier runs, and a separate read-only
reviewer checks every acceptance criterion before the controller marks it done.
Review is automated; original-execution approval remains exclusively yours.

By default it continues until completion, a blocker or cancellation, with no
overall time limit. Worker allowance is unchanged: default `--minutes 60` gives
50 minutes (shorter settings subtract one third, capped at ten minutes).
Verification now gets a fresh, separate 60-minute allowance after worker exit,
not the leftover worker budget. `--verification-minutes 90` can raise it, up to
120 minutes, for both post-worker and review/completion verification.
An explicit `--total-minutes` still caps every phase, including verification.
Individual CTest timeouts, assertions and serial execution remain unchanged.
Workers no longer need to duplicate the mandatory full suite; the supervisor
records authoritative verification and review reuses it only for unchanged content.
Actual verification timeout/cancellation/failure still stops or uses the existing
failure handling; this is not automatic repeated timeout retry.
Planning and review each default to ten minutes (`--plan-minutes` and
`--review-minutes`). Optional `--total-minutes 60` bounds the whole invocation;
`--max-cycles 10` limits dispatch cycles, not completed tasks. Zero cycles means
unlimited. `--max-attempts 3` bounds fixable verification/review failures per task;
authentication errors and interrupted workers do not automatically retry.
`--max-tasks 1000` bounds queue growth.

Completion requires every task done, evidence covering every criterion in
`completion.json`, full verification, and a separate completion review. An empty
queue or passing source tests alone cannot establish native fidelity. Work that
needs original execution or reopening a user park stops with a blocker.

Every phase update includes elapsed time since this invocation
started. The final console summary reports `Run duration`, status and the number
of tasks completed this run. `result.json` retains `started_at`, `finished_at`,
`elapsed_seconds` and a readable `elapsed` duration for completion, blockers,
timeouts and normal cancellation. Duration includes planning, implementation,
verification, review and controller overhead; it is not provider token time or
a cumulative total across restarts. It uses a monotonic clock so system clock
adjustments do not distort the measurement. A forced process kill can only leave
the last saved phase duration, not a final duration.

Queue progress and attempt counts persist between invocations. Completed tasks
are skipped; `needs_review` resumes at review, reusing verification only when
the source fingerprint matches. Interrupted tasks need inspection before being
returned to `ready`. Logs are under `build/agent-runs/loop-*/` and each worker's
own run directory. STOP and the repository lock apply to both run modes.

## Preview a single worker (no AI call)

```powershell
python tools/agent_control/supervise.py --backend claude --task TASK_ID
python tools/agent_control/supervise.py --backend codex --task TASK_ID --minutes 60
```

Replace `TASK_ID` with a reviewed ready task from `tasks.json`. Completed tasks cannot be dispatched again.
Preview prints the task, command, deadline split and future log location. It
does not create a run or modify the queue. An AI run requires `--execute`:

```powershell
python tools/agent_control/supervise.py --backend claude --task TASK_ID --minutes 60 --execute --allow-dirty
```

Use `--allow-dirty` only after reviewing the existing edits, including this
uncommitted setup. Without it execution refuses a dirty tree. Use the chosen
CLI's ordinary login separately if required; the supervisor does not read,
copy or install credentials. It can incur the selected provider's usage charges
when actually launched. CLI/model access has not been verified by previews.

## Runtime controls

- A single repository lock excludes other single-task and continuous runs.
  `supervise.py` handles one selected `ready` task; `continuous.py` advances the
  queue sequentially. Dependencies must be reviewed `done` before dispatch.
- `--minutes` accepts 1 through 60 and retains the existing worker allowance.
  `--verification-minutes` independently allows 1 through 120 (default 60).
  The continuous loop's explicit total deadline takes precedence over both.
- Windows creates the process suspended, puts it in a kill-on-close Job Object,
  then resumes it. Timeout, cancellation, launcher exit or supervisor crash
  closes the job and terminates its descendants. No visible helper window.
- Create `build/agent-runs/STOP` from another terminal to cancel. Ctrl+C in the
  supervisor also cancels. After review, remove STOP before another run.
- `build/agent-runs/<run-id>/` retains the prompt, CLI JSONL/stderr, before/after
  Git diffs/status and source hashes, verification logs and `result.json`.
  Source hashes also cover the imported recovery files. Hash
  manifests are not backups; preserve source before any later migration.
- Status transitions are `ready` -> `in_progress` -> `needs_review`, `blocked`
  or `interrupted`. Exit zero must also carry a successful final CLI event;
  the registered verifier must pass before `needs_review`. In continuous mode,
  an independent read-only review checks the actual acceptance criteria before
  `done`. Single-task mode leaves that review to the caller.
- An externally changed queue is never overwritten. A stale lock after a
  supervisor crash must be inspected against the recorded PID/run before
  manual removal. Interrupted tasks need review before returning to `ready`.
- Changed supervisor code, tool registry or shared instructions cause verification
  to be refused. This catches accidental edits; it is not tamper-proof isolation.

## Permissions and limitations

Claude implementation workers have file tools and narrowly approved verifier
commands (main/recovery/all/help) plus the existing HEAD-main.cpp read.
Planning/review retain read-only file tools. All authorized roles use the explicit
project-owned static REA profile; hooks/plugins and inherited settings stay isolated.
No blanket shell approval or permission bypass is added. Current scope is in
AGENTS.md; exact launch behavior is in supervise.py and the profile tests.

Codex workers use `exec`, workspace-write and approval policy `never`, without bypass
flags. The launcher explicitly selects `windows.sandbox="elevated"`, matching
the configured Windows sandbox on this computer. This names the sandbox
implementation; commands inside it remain unprivileged. It is required because
`--ignore-user-config` also omits the user's Windows sandbox selection. Review
and planning still use read-only permissions. Missing sandbox setup must fail
rather than fall back to unrestricted execution.

Codex ignores user configuration (auth still uses the CLI's normal auth
store), so user MCP/plugins/model-routing customizations are not carried into
this unattended profile. Planning/review use read-only access and structured
output. Model and effort are explicitly pinned in the registry. Missing access must fail rather
than relaxing permissions. Project instructions forbid launching the original.

The Job Object enforces process lifetime. It is not a filesystem/network or
command-security boundary. In particular Codex still has shell tools; textual
instructions cannot technically guarantee it will never execute an arbitrary
readable EXE. Use a separately isolated account/VM with execute access withheld
from game files if that guarantee is required. Do not treat a tool registry as
an enforceable allowlist. CLI behavior should be rechecked after upgrades.

Preview does not launch an AI worker. Executed runs do not change global
configuration or authorize commits/pushes or original-game execution. The former recovery
gitlink was subsequently replaced with ordinary tracked files at the user's
request; see `recovery-tree-repair.md`.

## References consulted

- [OpenAI AGENTS.md guidance](https://learn.chatgpt.com/docs/agent-configuration/agents-md)
- [OpenAI CLI command reference](https://learn.chatgpt.com/docs/developer-commands?surface=cli)
- [Microsoft Windows Job Objects](https://learn.microsoft.com/en-us/windows/win32/procthread/job-objects)
- [Claude Code programmatic use and structured output](https://code.claude.com/docs/en/headless)
- Installed `codex exec --help` and `claude --help` for these exact CLI versions.

## Visible-fluid milestone
Preview: `python tools/agent_control/run_fluid_milestone.py`
Start/resume eligible work: `python tools/agent_control/run_fluid_milestone.py --execute`
Research -> implementation -> clone interaction assessment, each independently
verified/reviewed. First two continue, final stage stops. No overall time limit;
worker 50 minutes, verifier 90 minutes, review 20 minutes, three attempts/task,
18 dispatch cycles maximum. A cycle cap preserves work for inspection/resume.
Provider failures, interruptions, STOP and genuine blockers stop; the launcher
does not reset them or restart completed work. Ctrl+C / build/agent-runs/STOP
use the existing cancellation mechanism. No loop is started by setup or preview.
Models, compile jobs12 and serial tests are unchanged. Full test timeouts and
assertions are unchanged. Stage reports carry handoffs; RUN_STATUS.md refreshes
automatically. Narrative docs are reconciled at the final milestone stop.

Current milestone defaults to Claude Sonnet5.5/high for workers and review.
`--backend codex` explicitly selects the retained OpenAI pin; no automatic fallback.
The visible-fluid milestone was accepted on2026-11-12. Its launcher now returns
without launching work because all three tasks are done. Original depth fidelity
remains open. The next documented priority is safe test parallelism and shorter
verification cycles; see IMPLEMENTATION_PLAN.md. It is not yet queued or enabled.
Current defaults remain12 compile jobs /1 test job; no model pins changed.

## Prepared test-parallelism run
Preview: `python tools/agent_control/run_test_parallelism.py`
Start: `python tools/agent_control/run_test_parallelism.py --execute`
Defaults to Claude Sonnet5.5/high; `--backend codex` is an explicit override.
Only test-parallel-isolation-001 is eligible. It audits shared state and adds
necessary test/CMake isolation, verifies serially, and stops after review.
Four-job benchmarking and enabling a new default happen afterward, with the
runner stopped. No controller permissions are widened; completed runs are no-ops.
