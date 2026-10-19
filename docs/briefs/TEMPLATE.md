# Brief: <one-question title>

Facilitator fills this before dispatch. Copy to `build/tmp/<id>-brief.md` (or keep under
`docs/briefs/` only after the round is landed, if you want a permanent record).

- **Queue id:** (must match `docs/tasks.json` → `active.id`)
- **Harness / worker:** round-agent | explore-agent
- **Owns live game:** yes | no

## Already settled (do not re-derive)

-

## Question

One sentence. What would falsify the prediction?

## Prediction (before any live run)

-

## Leads in hand

Addresses, scripts, scratch notes:

-

## Must not touch

- `PLAN.md`, `docs/**` (no landing), `.claude/rules/**`, `AGENTS.md`
- …

## Deliverables

- Candidate note under `build/tmp/…`
- Set queue `worker_output`; status → `ready-for-verify` or `blocked` + reason
- Game process state at end

## Report honestly

Measured vs read vs unsettled. No rounding, smoothing, or over-claim.
