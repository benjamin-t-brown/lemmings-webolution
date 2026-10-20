# Orchestrator–worker pattern (all harnesses)

Canonical for **Cursor, Codex, Claude Code, and OpenCode**. Model names stay in
[`ROLES.md`](ROLES.md). Evidence rules stay in `.claude/rules/`. This file is only the
session architecture.

## Why

A single long primary session that explores, edits, and verifies burns context, hits rate
limits, and mixes judgement with grind. Split roles. Keep one shared queue so an interrupted
round (capacity, approval, dead agent) is visible in every client.

## Roles

| Role | Tier | Owns | Must not |
| --- | --- | --- | --- |
| **Facilitator / planner** (architect / primary) | strong | Next question, **task decomposition**, brief, `docs/tasks.json`, verification, landing into `PLAN.md` / `docs/**` / `.claude/rules/**` | Heavy exploratory diffs; promoting unverified numbers |
| **Hard-task worker** | mid | One escalated brief that genuinely cannot be split to the targeted tier | Expanding scope; landing docs; dispatching further agents |
| **Targeted worker** (default builder) | cheap/fast | One small brief: exe reads, harness, code, tests, candidate note under `build/tmp/` | Editing facilitator-owned docs; dispatching further agents; owning the live game after the brief ends |
| **Explore agent** (optional, read-mostly) | cheapest | “Where is X?” maps: paths, symbols, call sites, string hits | Edits; live game control; landing docs; claiming measurements |
| **Reviewer** | — | **Is the facilitator.** A cheap lint/diff pass may precede verification; it never replaces address re-reads or pixel gates | Promoting claims into `docs/` |

Verification **never** delegates. Across 31 rounds, several claims were narrowed or retracted
on re-check — including facilitator-authored ones.

## Task sizing: cheap by default

The facilitator decomposes before dispatching. A round should be one small question a
**targeted** worker can finish in one pass: one falsifier, one ownership area, a bounded
read/run list, and an acceptance check the facilitator can repeat quickly. If it is not
that small, split it into separate queue items.

Escalate a brief to the **hard** tier only when it genuinely cannot be split — ambiguous
cross-cutting reverse engineering, a targeted attempt that failed for a reason other than
size, or a change whose failure mode needs broad judgement. Put the reason in the brief and
in `docs/tasks.json`. A failed targeted round is a signal to split the brief first, not
automatically to escalate it.

Model names live only in [`ROLES.md`](ROLES.md); this file names tiers, not vendors.

## Shared task queue

**File:** [`docs/tasks.json`](tasks.json) — facilitator-owned, tracked, one source of truth
for “what is in flight”. It is **not** a second plan. Goals and settled facts stay in
`PLAN.md` and `LESSONS.md`.

| `status` | Meaning |
| --- | --- |
| `queued` | Brief written; not started |
| `running` | Worker owns the work (and the live game, if any) |
| `blocked` | Waiting on approval, capacity, REA session, user, etc. — `blocked_reason` required |
| `ready-for-verify` | Worker finished; facilitator has not landed yet |
| `landed` | Facilitator verified and promoted (or explicitly discarded) |

**Who may write which fields**

- Facilitator: create/update any field; alone sets `landed`, alone clears `active`.
- Targeted or hard worker: may set `running` → `ready-for-verify` (or `blocked` with
  reason), and must set `worker_output` to its `build/tmp/…` note. Must not invent a new
  queue item or edit `PLAN.md` / `docs/scratch/` / rules.
- Explore agent: does not touch the queue unless the brief says to attach a map path under
  `worker_output` for an existing `running` item.

## Dispatch loop (every harness)

1. Facilitator reads `PLAN.md`, `LESSONS.md`, then **`docs/tasks.json`**.
2. If `active.status` is `blocked` or `ready-for-verify`, finish or unblock that — do not
   start a parallel research round that needs the live game.
3. Decompose the question into the smallest independently verifiable round, then write the
   brief (see `FACILITATOR.md`). Put its path in `active.brief`.
4. Set `active.status` to `queued`, then `running` when the worker starts. Choose the
   targeted tier by default; use the hard tier only with a recorded escalation reason.
5. Dispatch **one** worker with the brief. Optional explore first for map-only work.
6. On worker return: facilitator verifies (own scripts, own address reads, mutate-one-alone /
   pixel gate). Then land docs and set `landed`, or set `blocked` with a real reason.
7. If the worker dies early with nothing on disk: re-dispatch verbatim. If it dies late with
   edits on disk: do **not** re-dispatch — finish verification yourself.

## Per-harness wiring

| Harness | Facilitator session | Targeted worker (default) | Hard worker | Explore worker |
| --- | --- | --- | --- | --- |
| **Claude Code** | `/facilitator` (`.claude/commands/`); planner model from `ROLES.md` | Subagent `.claude/agents/round-agent.md` | Same subagent, dispatched with the hard-tier model from `ROLES.md` | `.claude/agents/explore-agent.md` |
| **Codex** | Planner model from `ROLES.md` | Dispatch with the targeted worker name from `ROLES.md` | Dispatch with the hard worker name from `ROLES.md` | Same targeted tier, read-only brief |
| **OpenCode** | `@.opencode/commands/facilitator.md`; default in `.opencode/opencode.json` | `.opencode/agent/round-agent.md` | No separate pin; split the work or use the same worker | `.opencode/agent/explore-agent.md` |
| **Cursor** | `/facilitator` (`.cursor/commands/`); Opus 5.5 primary chat | Task → `round-agent` (`.cursor/agents/`, Composer) | Same `round-agent` pin — split instead of escalating model | Task → `explore-agent` (`.cursor/agents/`, Composer) |

Cursor workers are pinned in **`.cursor/agents/*.md`** to a Cursor-pool model
(`composer-2.5-fast`). Auto applies only to the parent chat, so workers must **not** use
`model: inherit` when the facilitator is on Opus — that would bill every round as Opus.
Do not pass a third-party `model` override on Task for workers. Prefer
`explore-agent` over the built-in `explore` subagent so the pin holds. Do not paste
evidence rules into `.cursor/rules/` — pointers only. Model names: `docs/ROLES.md`.

## What this pattern deliberately rejects

- A second long-form plan inside the task file.
- One large round that only the strongest model can follow. Split it instead.
- Three workers (explore + build + review) **for every** round — use explore only when the
  question is “where?”, keep one targeted worker when the question is
  research/implementation.
- Cheap “reviewer” as the gate for original-game numbers.
- Copying `.claude/rules/` into another client’s rules directory.
