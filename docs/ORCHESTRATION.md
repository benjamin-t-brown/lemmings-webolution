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

## Boundaries, repairs and handoffs

The facilitator scopes, dispatches, independently verifies and promotes results.
Workers own research, harness debugging, implementation and permanent repairs, including
research-only rounds. The facilitator may write bounded independent checks and temporary
verification mutations; this does not authorize completing a worker's unfinished task.

Failed verification returns to the worker with expected versus observed results and a
bounded repair brief. Missing evidence also returns to the worker. If checking requires
substantial new tooling or open-ended discovery, delegate that work separately, then
independently verify its result. Preserve all existing evidence and acceptance standards.
After two unsuccessful repairs of the same defect, reassess and split the brief or record
an escalation reason; do not silently take over implementation.

Workers return an evidence packet: changed files, exact commands and prerequisites,
artifact paths, expected/observed results, negative results, scope limits and restoration
proof. Ready-for-verify means the packet is complete, even when the result is negative.

For repairs, retain the queue id, record the repair brief/output paths, and set `running`
when the worker starts. Use `blocked` for an actual external blocker. Record dispatch or
session id and requested model when exposed by the harness, otherwise mark unavailable.
A role label or model pin alone does not prove that a worker was dispatched.

Current status belongs in `docs/tasks.json`; historical handoff prose is not another queue.

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

## File ownership

The facilitator owns `AGENTS.md`, `PLAN.md`, `docs/**` and `.claude/rules/**`.
Workers edit only their brief's explicit code/test/harness allowlist and candidate notes
under `build/tmp/`. The queue-field permissions below are the sole routine exception to
facilitator-owned docs. Explore workers are read-only, including the queue; the facilitator
records their map output. No worker dispatches further agents.

Framework maintenance is a separate user-authorized role and may edit framework files.
It does not authorize taking over project work or changing task state.

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
  queue item or edit `PLAN.md` / `docs/scratch/` / rules. May clear its own `live_owner`
  after releasing the live game; never clear another worker's ownership.
- Explore agent: returns its map; does not edit queue fields.

## Dispatch loop (every harness)

### Facilitator invocation

`/facilitator` is an instruction to resume project facilitation now, even without
arguments. Follow the required startup reading, then act on the queue. Do not stop
after reading the operating rules or ask the user to supply a task merely because
`active` is null or the waiting queue is empty.

- An existing `running` task retains its owner. Check that session's status and
  respect the handoff/termination rules before resuming its work.
- Resolve an existing `blocked` task or verify a `ready-for-verify` result first.
- When no task is active, start the next queued brief if present. When both are
  empty, use `PLAN.md`, `LESSONS.md` and the latest landed evidence to select and
  decompose the next unfinished question, write and lint its brief, and dispatch
  one worker using the client mapping. Selecting that question is the facilitator's job.

Optional command arguments constrain the work; they are not required. Stop for a
concrete ownership conflict, unavailable delegation/model, external blocker, or
a decision the project sources cannot resolve, and report the specific evidence.
An explicit request to review or repair the framework remains framework maintenance;
it does not invoke project facilitation.

### Round lifecycle

1. Facilitator follows the reading order in [FACILITATOR.md](FACILITATOR.md).
2. If `active.status` is `blocked` or `ready-for-verify`, finish or unblock that — do not
   start a parallel research round that needs the live game.
3. Decompose the question into the smallest independently verifiable round, then write the
   brief (see `FACILITATOR.md`). Put its path in `active.brief`.
4. Set `active.status` to `queued`, then `running` when the worker starts. Choose the
   targeted tier by default; use the hard tier only with a recorded escalation reason.
5. Dispatch **one** worker with the brief. Optional explore first for map-only work.
6. On worker return: facilitator verifies (own scripts, own address reads, mutate-one-alone /
   pixel gate). Then land docs and set `landed`, return a bounded repair, or set `blocked` with a real reason.
7. After confirmed worker termination, inspect saved artifacts. Verify completed work.
   For incomplete work, dispatch a continuation brief identifying existing edits, remaining
   work and ownership. Do not restart blindly or finish implementation yourself. An old
   queue timestamp alone does not establish that a worker has died.

## Milestones, gates and round caps

Added 2026-10-20 after Phase A4 spent eleven rounds on vertex tint, eight on material and
alpha state that was mostly pixel-invisible, and seven on one zoom 1 residual.

- **Work to a fixed gate.** Each `PLAN.md` milestone states its exit gate before its rounds
  start. Do not change a gate after seeing a capture. A milestone is done when its gate
  passes.
- **Rank by gate error.** The next question is the one expected to remove the most error
  against the current gate, not the most interesting open residual.
- **Three rounds per question.** Count repairs and continuations. After three, park the
  question in `PLAN.md` Phase E with its evidence links and a resume brief, or record in the
  next brief why a fourth round is expected to move the gate.
- **Park residuals under the gate.** A residual that leaves the gate passing, or a
  correction that is pixel-invisible on every fixture, gets a unit test if it is cheap and
  then a Phase E line. It does not get its own investigation line.
- **Tooling needs a gate justification.** A round that builds or repairs instrumentation
  (hooks, tracers, capture rigs) states which gate measurement it unblocks. Tooling that
  only serves a parked question is parked with it.
- **One facilitator owns the queue.** Two concurrent facilitator sessions must not both
  dispatch from `docs/tasks.json`. Hand off explicitly or stop one first.
- **Commit between rounds.** Land each verified round as its own commit (when the user has
  authorized commits) so the next round starts from a clean, attributable tree.

## Standing run

A standing run is a user authorization for a facilitator to keep going round after round
toward one milestone gate without returning to the user between rounds. The active one, if
any, is `standing_run` in [tasks.json](tasks.json). When it is set, `/facilitator` runs this
loop:

1. Resolve the active task as usual: verify `ready-for-verify`, repair or unblock `blocked`,
   or dispatch `queued`.
2. Verify with the full [verification loop](FACILITATOR.md#the-verification-loop--this-is-the-job),
   land the docs and set the task `landed`.
3. Commit that round alone on `decompile`: name the paths, add attribution, no push, no
   `checkout`, `reset` or `stash`.
4. Pick the next question by gate error and the round caps, write and lint its brief, queue it,
   and dispatch. Do not stop to report between rounds.
5. Repeat until a stop condition holds.

**Granted by a standing run** (only what its entry lists): stock-game starts, within each
brief's own per-round budget, and per-round commits. Everything in
[AGENT_SAFETY.md](AGENT_SAFETY.md) and the evidence rules still applies.

**Stop and report to the user** when any of these holds:
- the milestone gate passes on the facilitator's own independent re-score;
- the gate, its mask or its exclusions would need to change;
- a guardrail breach: install hashes not restored, a process you cannot attribute, a write
  left unrestored, or a worker exceeding its brief's start budget or using an excluded method;
- the round cap is hit and no other question is ranked;
- delegation or the pinned model is unavailable, or another session owns the queue;
- the session is running out of context. Leave the queue in a resumable state and say so.

At a stop, write `standing_run.stopped` (date, reason, last commit) and report the gate
score, the rounds run and the starts used.

Only the session named in `standing_run.owner` dispatches. Another session that wants to
take over records the handoff there first.

## Client adapters

[ROLES.md](ROLES.md) owns model choices and client-specific launch/dispatch wiring.
Client agent files retain required frontmatter and point to shared contracts under
[agents/](agents/README.md). Do not copy the contracts into each client.

## What this pattern deliberately rejects

- A second long-form plan inside the task file.
- One large round that only the strongest model can follow. Split it instead.
- Three workers (explore + build + review) **for every** round — use explore only when the
  question is “where?”, keep one targeted worker when the question is
  research/implementation.
- Cheap “reviewer” as the gate for original-game numbers.
- Copying `.claude/rules/` into another client’s rules directory.
