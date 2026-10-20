# Brief: <one-question title>

Facilitator fills this before dispatch. Copy to `build/tmp/<id>-brief.md` (or keep under
`docs/briefs/` only after the round is landed, if you want a permanent record).

- **Queue id:** (must match `docs/tasks.json` → `active.id`)
- **Harness / worker:** targeted | hard | explore
- **Why this tier:** one sentence. Targeted is the default; a hard round needs a recorded
  reason why the work cannot be split.
- **Owns live game:** yes | no
- **Owned files/paths:** explicit allowlist
- **Dependencies:** completed prerequisites or queue ids
- **Investigation/run budget:** concrete limit; report unresolved findings when reached
- **Continuation/repair:** prior artifacts and exact remaining defect, if applicable

## Required reading

List exact lesson headings, the relevant plan section, and linked evidence. Use Markdown
links relative to this brief's location (usually `build/tmp/`), including heading anchors.
Do not require the whole history by default. The worker contract's guardrails still apply.

-

## Already settled (do not re-derive)

Only established observations, with evidence links and scope limits. Keep interpretations
and proposed explanations in Hypotheses below.

-

## Question

One sentence. What would falsify the prediction?

## Prediction (before any live run)

Worker records its prediction and falsifier before the run; a facilitator suggestion is
not evidence. For an offline-only task, state why a live prediction is not applicable.

## Acceptance criteria

The single exact check the facilitator will re-run independently. One check, not a bundle.

-

## Optional leads

Addresses, scripts and possible methods. Mark any mandatory method as such and explain
the constraint; otherwise let the worker choose how to answer the question.

-

## Hypotheses

Unverified explanations, explicitly separated from observations. State "none" if absent.

-

## Must not touch

- `PLAN.md`, `docs/**` (no landing), `.claude/rules/**`, `AGENTS.md`
- …

## Deliverables

- Changed files, exact reproduction commands, prerequisites and artifact paths
- Expected/observed results, failed checks and limits on each headline claim
- Individual mutation results and exact restoration evidence where applicable
- Unfinished work and new questions separated from completed scope
- Candidate note under `build/tmp/…`
- Set queue `worker_output`; status → `ready-for-verify` or `blocked` + reason
- Game process state at end

## Report honestly

Measured vs read vs unsettled. No rounding, smoothing, or over-claim.
