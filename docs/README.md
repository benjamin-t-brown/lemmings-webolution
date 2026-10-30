# Documentation map

## Start here

The objective is unchanged: **1:1 Lemmings Revolution, stock 640x480 executable,
branch `decompile`**. The short entry path is:

1. [PLAN](../PLAN.md): completion sequence and next work package.
2. [LESSONS](LESSONS.md): essential traps and a topic index, not the whole history.
3. [Safety](AGENT_SAFETY.md) and the relevant topic for the task.

Read [STATUS](STATUS.md) when choosing work or making a completion claim. Do not
load every research note, directive or historical phase into each session.

## Current references

| Question | Canonical place |
| --- | --- |
| What are we trying to finish, and what comes next? | [PLAN](../PLAN.md) |
| What is implemented, native-verified, failing or parked? | [STATUS](STATUS.md) |
| What exactly did the old signed gates require? | [Acceptance gates](acceptance/legacy-gates.md) |
| What is already known about this subsystem? | [Topic index](LESSONS.md) and its linked evidence |
| What counts as evidence? | [Match the original](../.claude/rules/match-the-original.md) |
| How should a rendering defect be investigated? | [Rendering method](RENDERING_METHOD.md), [frame map](RENDER_FRAME_MAP.md) |
| How do I build/run it? | [README](../README.md), [Windows build rules](../.claude/rules/windows-msys2-build.md) |
| How do I safely observe the original? | [Safety](AGENT_SAFETY.md), evidence rule foreground section; [REA setup](REA_SETUP.md) if using REA |
| How can work be delegated? | [Roles/models](ROLES.md); delegation is optional, not a mandatory hierarchy |

## Evidence and history — consult on demand

These paths retain evidence, retractions and provenance. They are **not an active
queue, current authorization or a requirement to recreate the old workflow**.
Leave preserved artifacts in place: mass-moving them breaks citations and scripts.

- [PLAN_OLD](../PLAN_OLD.md): pre-reorganization snapshot, including old status,
  gate amendments and user decisions. Its “live plan” wording is historical.
- Phase histories: [A / rendering](plan-history/phase-a.md),
  [B / frontend](plan-history/phase-b.md), [C / HUD and scene](plan-history/phase-c.md),
  [D / simulation](plan-history/phase-d.md), [E / debts and parks](plan-history/phase-e.md).
  Read later evidence before treating an old park's technical status as current.
- [Scratch evidence](scratch/): individual reads, experiments, checks, failed
  attempts and integration notes. Filenames are stable evidence IDs in practice.
- [Facilitator history](FACILITATOR_HISTORY.md), [briefs](briefs/),
  [queue history](queue-history/), [tasks](tasks.json), [task archive](tasks-archive.json):
  historical process records, not a second live plan.
- [DIRECTIVE](DIRECTIVE.md), the dated `DIRECTIVE-*.md` files and
  [authorization history](plan-history/phase-c-authorizations-2026-10-27.md): past
  grants/results. An unused budget is not current authority. Preserve their bytes
  and disclosures; current work follows the user's request and standing guardrails.
- [PIVOT](../PIVOT.md) and [capture proposal](RENDER_CAPTURE_PROPOSAL.md): historical
  proposals; [RENDERING_METHOD](RENDERING_METHOD.md) is the adopted method.
- [Retired framework](../old/docs/agent-framework/) and [old tree](../old/): earlier
  contracts/implementations. Their existence does not prove production integration.

## Maintenance

Use one home per kind of information:

| New information | Update |
| --- | --- |
| Changed objective, dependency order or next package | Replace the relevant PLAN paragraph. |
| Changed implementation/verification status | Replace the STATUS row; link the dated check and state its scope. |
| Reusable fact or correction | Update the relevant lessons topic; give an address/script/evidence link and label READ/DERIVED/OBSERVED/unknown. Keep a retraction next to the superseded claim. |
| Detailed experiment or investigation | One focused note under `scratch/`, with question, evidence, result, limits and follow-up. Link it from the topic/status; do not copy its narrative into both. |
| Acceptance definition or amendment | Put it in `acceptance/`, with the decision and provenance. Never silently relax a gate after observing failure. |

Size budgets for frequently loaded documents (replace or extract; do not compress
into unreadable shorthand): **PLAN ≤120 lines/10 KB; LESSONS ≤120 lines/10 KB;
STATUS ≤150 lines/16 KB; this index ≤120 lines/10 KB**. Topic references are loaded
on demand; split a growing topic by mechanism, not by a new session's date.

Before landing documentation changes, run `python scripts/check_docs.py` for the
short documents' budgets, links/anchors and UTF-8/LF. Separately check preservation
of moved evidence and unchanged gate wording. A documentation check is not a game
test. `scripts/check_agent_framework.py` targets the retired role
framework and is not the completion-plan validator; do not restore deleted roles
just to satisfy it.

The 2026-10-28 reorganization separates navigation/status from retained technical
and historical detail. It does not erase failures, alter the reference, certify
native fidelity, renew historical run grants or change user work elsewhere.
Lesson excerpt ranges refer to `docs/LESSONS.md` at commit
`7eaf326fea82ab44adee51ea62b07249f2eed85a`; all 1,716 source lines were retained.
