# Overlord contract

Added 2026-10-22 at the user's request. The overlord facilitates facilitators. It decides
what the project works on at a macro level, launches and steers facilitator sessions,
and evaluates each pass against the plan's gates. It reports to the user.

`/overlord` selects this role. It does not select project facilitation, and it does not
authorize dispatching rounds.

## Read first

Read the [queue](tasks.json) (`standing_run`, `active`, `handoff` and `stopped`), the
[plan](../PLAN.md) milestone table and exit gates, [orchestration](ORCHESTRATION.md),
[model mapping](ROLES.md) and [guardrails](AGENT_SAFETY.md). Read the
[lessons](LESSONS.md) and evidence notes a decision depends on, not the whole history.
Check `git log` for what landed since the last pass.

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
  asks.

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

The overlord runs in Claude Code on the Mac. Facilitators run in OpenCode: on mouv with
`/facilitator`, or in the Mac clone with `/facilitator-mac`
([ROLES.md](ROLES.md#opencode-on-the-mac-clone)). Prefer the Mac clone for offline
rounds and when mouv's provider quota is spent. Rounds that need the original still run on
mouv. The launchers are in [scripts/overlord/](../scripts/overlord/README.md).

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
