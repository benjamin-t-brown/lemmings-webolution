# Orchestrator–worker pattern (all harnesses)

Canonical for **Cursor, Codex, Claude Code, and OpenCode**. Model names stay in
[`ROLES.md`](ROLES.md). Evidence rules stay in `.claude/rules/`. This file is only the
session architecture.

## Why

A single long primary session that explores, edits, and verifies burns context, hits rate
limits, and mixes judgement with grind. Split roles. Keep one shared queue so an interrupted
round (capacity, approval, dead agent) is visible in every client.

## Roles

| Role | Owns | Must not |
| --- | --- | --- |
| **Facilitator** (architect / primary) | Next question, brief, `docs/tasks.json`, verification, landing into `PLAN.md` / `docs/**` / `.claude/rules/**` | Heavy exploratory diffs; promoting unverified numbers |
| **Round agent** (builder) | One brief: exe reads, harness, code, tests, candidate note under `build/tmp/` | Editing facilitator-owned docs; dispatching further agents; owning the live game after the brief ends |
| **Explore agent** (optional, read-mostly) | “Where is X?” maps: paths, symbols, call sites, string hits | Edits; live game control; landing docs; claiming measurements |
| **Reviewer** | **Is the facilitator.** A cheap lint/diff pass may precede verification; it never replaces address re-reads or pixel gates | Promoting claims into `docs/` |

Verification **never** delegates. Across 31 rounds, several claims were narrowed or retracted
on re-check — including facilitator-authored ones.

## Shared task queue

**File:** [`docs/tasks.json`](tasks.json) — facilitator-owned, tracked, one source of truth
for “what is in flight”. It is **not** a second plan. Goals and settled facts stay in
`PLAN.md` and `LESSONS.md`.

| `status` | Meaning |
| --- | --- |
| `queued` | Brief written; not started |
| `running` | Round agent owns the work (and the live game, if any) |
| `blocked` | Waiting on approval, capacity, REA session, user, etc. — `blocked_reason` required |
| `ready-for-verify` | Worker finished; facilitator has not landed yet |
| `landed` | Facilitator verified and promoted (or explicitly discarded) |

**Who may write which fields**

- Facilitator: create/update any field; alone sets `landed`, alone clears `active`.
- Round agent: may set `running` → `ready-for-verify` (or `blocked` with reason), and must
  set `worker_output` to its `build/tmp/…` note. Must not invent a new queue item or edit
  `PLAN.md` / `docs/scratch/` / rules.
- Explore agent: does not touch the queue unless the brief says to attach a map path under
  `worker_output` for an existing `running` item.

## Dispatch loop (every harness)

1. Facilitator reads `PLAN.md`, `LESSONS.md`, then **`docs/tasks.json`**.
2. If `active.status` is `blocked` or `ready-for-verify`, finish or unblock that — do not
   start a parallel research round that needs the live game.
3. Write the brief (see `FACILITATOR.md`). Put its path in `active.brief`.
4. Set `active.status` to `queued`, then `running` when the worker starts.
5. Dispatch **one** round agent with the brief. Optional explore first for map-only work.
6. On worker return: facilitator verifies (own scripts, own address reads, mutate-one-alone /
   pixel gate). Then land docs and set `landed`, or set `blocked` with a real reason.
7. If the worker dies early with nothing on disk: re-dispatch verbatim. If it dies late with
   edits on disk: do **not** re-dispatch — finish verification yourself.

## Per-harness wiring

| Harness | Facilitator session | Round worker | Explore worker |
| --- | --- | --- | --- |
| **Claude Code** | Session model = strong tier (`opus` role) | Subagent `.claude/agents/round-agent.md` (`sonnet`) | `.claude/agents/explore-agent.md` |
| **Codex** | `sol` | Dispatch with `model: gpt-6-luna` (or current Luna pin in `ROLES.md`) | Same cheap tier, read-only brief |
| **OpenCode** | Default in `.opencode/opencode.json` (`max`) | `.opencode/agent/round-agent.md` | `.opencode/agent/explore-agent.md` |
| **Cursor** | Strong primary chat; plan/delegate; verify | Task → `round-agent` (cheap tier when choosing a model) | Task → `explore` |

Cursor has no separate agent-definition store that pins models the way OpenCode/Claude do.
The split is by **convention** plus this file and `docs/tasks.json`. Do not paste evidence
rules into `.cursor/rules/` — pointers only.

## What this pattern deliberately rejects

- A second long-form plan inside the task file.
- Three workers (explore + build + review) **for every** round — use explore only when the
  question is “where?”, keep one round agent when the question is research/implementation.
- Cheap “reviewer” as the gate for original-game numbers.
- Copying `.claude/rules/` into another client’s rules directory.
