# Lemmings Revolution reconstruction

## Objective and current work
Recover faithful, readable C++23 with MinGW-w64 GCC on Windows; Emscripten comes later.
Use this checkout, including the ordinary source folder `decomp-lemmings/`.
Current milestone is playable v1: a person can play and win every installed level in
the clone, per [agent/IMPLEMENTATION_PLAN.md](agent/IMPLEMENTATION_PLAN.md). Work order comes
from the human playtest ledger under docs/playtest/. Workflow work is finished. Completed integrations are
listed in [docs/STATUS.md](docs/STATUS.md); do not repeat historical tasks.

Read this file, agent/RUN_STATUS.md, the current plan/status, relevant accepted reports, `agent/tasks.json`,
`agent/README.md`, and `agent/tools.windows.json` before changes.
The queue records individual task acceptance; `agent/completion.json` defines full
project completion. A baseline report or empty queue does not complete the project.

## Boundaries
- Never execute the original game, copied original machine code/oracles, debugger,
  hooks, capture or indirect launchers without explicit approval for that run.
  Existing live-workflow documentation and past grants are not new authorization.
- Static inspection and source-built clone/test execution are allowed. Analyze only
  the registered reference EXE and adjacent assets under `Lemmings Revolution/`.
  Resolve paths from this repository, not an external installation.
- D1/D3/base-clock, release/digger timing, clipping/controller/capture work remain
  parked. Observing and documenting a clone failure does not authorize fixing a park.
  Timed Armageddon remains parked; no coarse approximation has been chosen.
  The research-only exception for lever-timing-research-001 is complete. Its
  acceptance does not authorize the proposed 0.2-second gate implementation or
  reopen shared timing. gate-cadence-research-001 is also complete; its proposed
  second gate clock is not approved. shared-timing-impl-001 completed its narrow
  shared-dt integration exception. Preserve that accepted path; no broader timing
  reopening follows. Walk/fall use continuous budgets; other states remain hybrid
  5 Hz. Release/digger/skill retuning and other parks remain closed, except that
  fixes for playtest-ledger defects are authorized where they touch these areas;
  see Project lead direction.
  Linked receiver zone-mask behavior is accepted with documented limits.
  lever-initial-face-impl-001 is accepted; all four tested levers use stored faces.
  The visible-fluid milestone authorizes one bounded static research stage, its
  production render integration and a final clone interaction assessment.
  Source-built clone framebuffer/image outputs are authorized; original capture
  and unrelated clipping/controller/timing work remain parked. Research and
  implementation continue after acceptance; final assessment stops the loop.
- Do not change completion criteria, historical failures/exclusions, model pins,
  permissions or existing test assertions/timeouts to obtain a pass.
- No subagents, concurrent writers, auto-commit/push, reset, clean or stash.
  Preserve all pre-existing edits. Acquire `supervise.RepositoryLease` for manual
  protected edits, check live processes first, and retain backups under ignored build/.
  Workers must not edit queue, verifier or controller controls.

## Evidence and implementation
Name the milestone step, actual installed level/route, observed or source-traced
failure, and why a proposed change is the next dependency. Trace the production call
path and demonstrate before/after behavior. Synthetic fixtures supplement real
routes; they do not establish natural gameplay or full original fidelity.
Keep static instructions, pseudocode, inference, source tests, clone observations
and original comparisons distinct. Retain exact binary identity and addresses.
Check arithmetic, layout, ordering and state transitions; do not invent semantics.
Unknown fields are not automatically uninitialized fields.

Prioritize demonstrated gameplay/rendering failures over optional formats, audio
expansion and test-only work. Bounded prerequisite research needs an explicit task
and a concrete implementation dependency. The user-authorized playable-baseline-001, playable-expansion-001 and targeted-play-001
and fluid-lever-assessment-001 and level-sweep-001
assessments are observation/reporting exceptions to production-change requirements:
do not manufacture code changes just to satisfy the usual task template.
Archive detailed evidence; keep active summaries concise and link to it.
agent/RUN_STATUS.md is controller-generated from queue/results and is not acceptance
evidence itself. At each accepted milestone or priority change, reconcile the short
plan/status with the final review and verifier log. Workers must not edit these
shared controls; the coordinating agent performs the narrative handoff under lease.
The fluid run is complete and its narrative handoff is reconciled. Future task
reports retain concise handoffs; RUN_STATUS.md updates automatically.
Reports cite exact runs and retain earlier findings with explicit corrections.

## Builds, saves and preservation
Use registered tool paths and `.claude/rules/windows-msys2-build.md`; do not rely
on whichever compiler appears first on PATH. Use LF.
The supervisor owns mandatory full registered verification after worker exit.
Workers may run a permitted profile to answer a concrete development question,
but need not duplicate the full suite. Report pending supervisor verification
truthfully; do not repeatedly restart a suite killed by the client time limit. Build jobs default8;
the main suite builds -O2 with assertions and runs four test jobs (one is the fallback). Preserve source provenance for frozen/current demonstrations;
never replace an authentic pre-task baseline with resumed modified files.
Do not add original binaries/assets to version control.

Use the accepted original-format save codec. The clone's normal save is under
LocalAppData/LemmingsWebolution/SavedGame.dat; never overwrite the installed save.
Baseline/tests use a separate temporary clone save path and preserve normal user
progress. Do not silently import/reset saves. Preserve unknown supported fields and
the prior good save on write failure. WEBOPROG is not retail compatibility evidence.

## Agent execution and REA
Use the sequential controller described in `agent/README.md`. Tasks finish
needs_review; only a separate acceptance review makes them done. Preserve retry
history and inspect interrupted work before making it ready. Stop for missing access,
required approval, user parks or genuine evidence blockers; do not loop on them.

Claude/Codex implementation and explicitly authorized research workers, plus
plan/review/review_completion roles, may use the existing project-owned static REA
profile. It restricts target hash, Ghidra provider and named static tools. No runtime,
debugger, capture, metadata mutation, snapshots or blanket permission bypass.
Read schemas; start from retained evidence; cross-check pseudocode against instructions;
close analysis without snapshot arguments. Ghidra/Java analysis is not game execution.
Planner/reviewer file access remains read-only, without build/test permission; a Codex
planner/reviewer reads files with read-only commands in its read-only sandboxed shell;
their evidence stays in structured responses. Hooks/plugins and inherited settings
remain isolated. Do not alter launch profiles or import user-global configuration.
Permissions and process-lifetime controls are not a complete OS security boundary.

## Model recommendations and user overrides
During the existing planning pass, record Difficulty (routine/difficult/exceptional),
Recommended model (Sonnet/Opus/Fable), and a short evidence-based reason in the
proposed task scope and plan reason. This is advisory, not automatic model routing.
Routine documentation, established fixes and implementation of an accepted contract
start with Sonnet. Ambiguous behavior, cross-system reasoning or conflicting evidence
suggest Opus. Reserve Fable for a specific unresolved reasoning/reverse-engineering
question after a focused attempt; recommend a bounded time/spending allowance.
Quota/authentication, slow tests, tool failures and interruptions are not reasons to
escalate models. Return to the cheaper recommendation when the hard dependency is
resolved. Do not add another grading agent or provider call.
The user's explicit model choice always overrides the recommendation, at any grade,
for the scope they specify (task, role or run). Recommendations never modify pins,
block an authorized override or authorize a paid escalation. Obtain explicit user
approval before a new Fable escalation; an existing explicit Fable authorization
remains valid for its scope. Apply requested pin changes only at a safe stopped
boundary under RepositoryLease, show the resulting preview, and preserve history.
Queued tasks that bypass planning can be graded by the coordinating agent at handoff;
no extra planner call is required. Current milestone uses Claude Sonnet5.5/high by user request. Codex remains an
explicit provider override. No automatic escalation.
Sonnet remains the default Claude recommendation, not an automatic provider switch.

## History
Earlier instructions/plans are archived under docs/plan-history/documentation-2026-11-02/.
They explain past work and authorizations, not the current dispatch order.
Preserve docs/STATUS.md's gate ledger, recovery PROGRESS/FUNCTIONS, the scoped
372/6,186 historical recovery baseline, and accepted task records.

## Approved fluid milestone exception
The user approved working clone fluid integration with explicitly provisional
fluid depth behavior. Exact original inherited depth-test/write/comparison/device
bias remains unresolved and is not an acceptance prerequisite for this clone
milestone. Preserve full-project completion criteria and historical fidelity gates.
Start with an isolated clone Less/write-on policy, preserve traced software bias,
and require actual clone pixel/occlusion inspection plus ordinary lever interaction.
This is not permission for original execution or general depth/clipping repairs.
Existing research may be independently accepted under the revised scoped criteria;
do not erase its prior blocked reviews or present them as original-fidelity passes.

## Authorized workflow run
User authorized test-parallel-isolation-001 as a bounded exception to gameplay
production-change requirements. It may modify test resource isolation and CMake
scheduling, preserving test semantics/timeouts/counts and protected controls.
Acceptance means ready for a later parallel benchmark, not measured speedup.
The supervisor continues serial verification and stops after independent review.
The coordinator owns any later verifier/preflight/cache changes under an idle lease.

## Playable v1 direction
User approved (2026-11-12) a playable-clone-first milestone. It sets original timing,
full-frame fidelity and original comparison aside without resolving them: completion
criteria, the gate ledger and every park above are unchanged, and a clone win is not
fidelity evidence. This is not approval for original execution or for any observation
task. The user then authorized level-sweep-001 (all-level clone smoke sweep) as one
observation task; further assessments still need explicit user authorization.

## Original-game execution: interactive sessions only
User approved (2026-11-12) running the original game, with hooks, debugger, capture or
REA runtime tools where they help, in interactive coordinating sessions where the user
can see what is launched. This replaces per-run approval for those sessions only.
Unattended workers, planners, reviewers and the controller are unchanged: they never
launch the original or its oracles, keep the static REA profile, and oracle-001 stays
blocked for the queue. Before each launch state what runs, why and for how long; report
what actually ran. Back up before changing the installation, never overwrite the
installed save, and keep original binaries/assets out of version control. The approval
permits observation and comparison; it does not by itself reopen parked fixes.

## Project lead direction
The user appointed the interactive Fable session project lead (2026-11-12) with authority
over sequence, queue, controller, verifier, model pins, documentation and commits. The
lead still edits protected controls only under RepositoryLease with backups and no live
runner, and does not push without telling the user. Workers and reviewers are unchanged:
no commits, no control edits, no original execution.
Playable v1 is judged by human playtests. Defects listed in docs/playtest/ are demonstrated
failures and their fixes are authorized production work even where they touch a parked
area (skill behavior, animation and movement presentation, actor occlusion, fast-forward).
Such a fix targets correct visible clone behavior, uses retained static evidence where it
exists, labels provisional choices, and may correct an existing test expectation only where
that expectation encodes the demonstrated defect, with before/after justification. It is
not a fidelity pass: matching original timing, the gate ledger and completion criteria are
unchanged. A scripted win is regression evidence, not proof a level is playable.
