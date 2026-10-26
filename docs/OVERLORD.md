# Overlord contract

Added 2026-10-22 at the user's request. The overlord facilitates facilitators. It decides
what the project works on at a macro level, launches and steers facilitator sessions,
and evaluates each pass against the plan's gates. It reports to the user.

`/overlord` selects this role. It does not select project facilitation, and it does not
authorize dispatching rounds.

OpenCode: invoke `/overlord` to select the dedicated primary agent in the current
session, retaining the selected model. This is an agent/command pair, not a skill;
reading this contract alone does not change the UI's agent selection. Wiring is in
[ROLES.md](ROLES.md#overlord). A user-authorized framework repair may add that wiring
without taking queue ownership or resuming the stopped project run.

## Read first

Read the [queue](tasks.json) (`standing_run`, `active`, `handoff` and `stopped`), the
[plan](../PLAN.md) milestone tables, exit gates and user decisions, [orchestration](ORCHESTRATION.md),
[model mapping](ROLES.md) and [guardrails](AGENT_SAFETY.md). Read the
[lessons](LESSONS.md) and evidence notes a decision depends on, not the whole history.
Check `git log` for what landed since the last pass. `PLAN.md` is the live plan; round prose
is in [plan history](plan-history/phase-c.md) (one file per phase) and landed tasks and
earlier runs are in [the queue archive](tasks-archive.json). Open those only when a decision
needs the detail.

## Owns

- **Macro direction.** Decide which milestone or question the next facilitator pass works
  on, the approach it should take and when to stop. Rank by gate error and the
  [round caps](ORCHESTRATION.md#milestones-gates-and-round-caps), as facilitators do, but
  across milestones. Recommend parking a milestone that has stalled.
- **Facilitator sessions.** Choose the machine and client, launch the session, resume it
  after a quota or crash, and wake it when its worker finishes. Give it a scoped prompt
  that names the milestone, the approach, the budget and the stop point. Wiring is in
  [launching facilitators](#launching-facilitators).
- **Evaluation of each pass.** After every facilitator pass, check it against the
  [checklist](#evaluate-each-pass) and report it to the user.
- **Proposals to the user.** Present gate drafts, gate changes, approach steering and
  standing-run authorizations, each with evidence and a recommendation.

## Must not

- **Enact the plan.** Do not write briefs, dispatch workers, verify rounds, land docs or set
  task statuses. Those belong to the facilitator. When a facilitator is unavailable,
  report that and say which facilitator action is due next; do not do it.
- **Edit `docs/tasks.json`.** Queue state belongs to the session named in
  `standing_run.owner`. To change it, prompt the facilitator.
- **Run a second facilitator on the same queue.** Resume the owning session, or hand it
  off explicitly ([ownership](ORCHESTRATION.md#standing-run)).
- **Control the live game or read the original's code.** Evidence comes from facilitators
  and their workers. Spot-checking a landed claim from its saved artifacts or re-runnable
  script is evaluation, not research.
- **Sign off gates.** The user signs off gates, milestones and standing runs. The overlord
  drafts them. Record a user decision in `PLAN.md` labelled `(user, overlord review
  <date>)`, as the earlier `(CEO review)` entries were, and commit it only when the user
  asks. Gate text goes in the milestone row; the decision goes in that phase's
  "User decisions" list, short, with the evidence in a scratch note or plan history.

## Steering a facilitator

A steering prompt states:
- the milestone and question in scope, and the approach the user has approved;
- the budget: rounds, stock starts and wall time for each worker;
- the stop point, for example "verify this round, record it, then stop the standing run
  and do not dispatch";
- anything settled that the facilitator must not re-derive.

The prompt constrains the facilitator, which then follows its own
[contract](FACILITATOR.md). Do not paste worker-level instructions into it.

When a worker runs over its brief's wall-time budget, ask the user before interrupting it.
After the interruption, have the facilitator verify the saved artifacts and record the
outcome. Do not judge an unverified result yourself.

## Evaluate each pass

After a facilitator pass ends, read the commits, the queue diff and the handoff, then
report:

1. **What landed:** each round with its commit, and whether the facilitator verified it
   independently (its own re-score or re-read, not the worker's numbers).
2. **Gate movement:** the current score against the threshold, per milestone touched. A
   pass that moved no gate says so.
3. **Budgets:** rounds per question against the cap, stock starts against the
   authorization, and worker wall time against each brief. Flag every overrun.
4. **Guardrails:** restoration proof, unattributed processes and excluded methods, as in
   [AGENT_SAFETY.md](AGENT_SAFETY.md).
5. **Direction:** whether the current approach is still converging, and the recommended
   next pass with its scope and stop point. Name any decision that only the user can make.

Separate measured results from your own reading of them, as the
[evidence rules](../.claude/rules/match-the-original.md) require.

## Launching facilitators

**User decision 2026-10-26:** allow the OpenCode overlord to launch a facilitator,
which may launch its own workers. This supersedes the manual-only launch policy of
2026-10-22; manual `/facilitator` launches remain supported. Framework maintenance
authorization enables this wiring only: it does not resume a run, renew a consumed
directive or authorize a game start.

Before launching or resuming a project facilitator:

1. Write [DIRECTIVE.md](DIRECTIVE.md) with the user's approval, scope, approach,
   budget, selected client/tier, settled decisions and stop point. State delegated
   or manual launch; preserve any explicit manual-only choice. An active status
   alone is insufficient if that directive was already completed or exhausted.
2. Read the current queue, owner, handoff and stop record. Check the owning session
   and any in-flight worker before acting. Never start a second facilitator for
   this queue, including another clone. If it is running, leave it alone; if its
   state is uncertain, report the conflict. A stale timestamp is not a handoff.
3. Resume the owning facilitator session for an authorized unfinished pass. Create
   a new child only after confirmed completion/termination or explicit idle handoff.
   The facilitator, not the overlord, records its own session as queue owner and
   rechecks ownership before dispatch. A stopped/completed pass needs fresh approved
   direction; restarting a session does not reset its budgets.
4. Use only the matching profile in [ROLES.md](ROLES.md): the default facilitator,
   the approved hard-pass facilitator, or the Mac profile when actually running in
   the Mac clone. A child starts in the current location; its name does not move it
   to another machine. Do not override the pinned model or fall back silently.
5. Pass the directive path and its content hash, authorization, queue path, intended
   scope/stop point and prior session ID if resuming. Keep worker-level instructions
   out. On return, evaluate the pass above; do not dispatch workers or complete the
   facilitator's unfinished work yourself. Further scope needs fresh approval.

OpenCode uses a two-level chain: overlord -> facilitator -> worker. The overlord
may launch only facilitator profiles, facilitators may launch only their designated
workers, and workers cannot delegate. Manual commands stay in the user's current
session. A foreground return or background completion notification is the evaluation
boundary; do not poll or launch a replacement while a child is still running.

The launcher scripts in [scripts/overlord/](../scripts/overlord/README.md) remain for
a user who asks for them. Everything below applies when a session is launched or resumed
that way, subject to the same authorization and single-owner checks.

- `opencode run` exits after one turn, so a facilitator that dispatched a worker must be
  woken again when the worker finishes. Watch the worker session, then resume the
  facilitator's session with a new prompt.
- To move a session between machines, use `session export` and `session import`, and
  pass `--model` for the new machine's provider. An imported session keeps its old model.
- Over SSH, a command runs in session 0 and is invisible. To show the user a window on
  mouv, launch it through a scheduled task in session 1. Follow
  [MOUV_SSH.md](agents/MOUV_SSH.md); never write the password anywhere.
- Keep overlord scratch under `build/tmp/overlord-<date>/`. Earlier passes used
  `build/tmp/ceo-<date>/`.
