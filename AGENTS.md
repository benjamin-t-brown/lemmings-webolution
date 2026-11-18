# Lemmings Revolution reconstruction

## Objective
Recover faithful, readable C++23 with MinGW-w64 GCC on Windows; Emscripten comes later.
Use this checkout, including the ordinary source folder `decomp-lemmings/`.
Current milestone is playable v1: a person can play and win every installed level in the clone,
per [agent/IMPLEMENTATION_PLAN.md](agent/IMPLEMENTATION_PLAN.md). Work order comes from the human
playtest ledger under docs/playtest/. Completed integrations are listed in
[docs/STATUS.md](docs/STATUS.md); do not repeat them. `agent/completion.json` defines full project
completion; an empty queue does not complete the project.

## Playtest defects
User approved (2026-11-12) a playable-clone-first milestone. It sets original timing, full-frame
fidelity and original comparison aside without resolving them; a clone win is not fidelity evidence.
Defects listed in docs/playtest/ are demonstrated failures, and their fixes are authorized production
work even where they touch a parked area (skill behavior, animation and movement presentation, actor
occlusion, fast-forward, release/digger timing). Such a fix targets correct visible clone behavior,
uses retained static evidence where it exists, and labels provisional choices. It may correct an
existing test expectation only where that expectation encodes the demonstrated defect, with
before/after justification. A scripted win is regression evidence, not proof a level is playable.

## Boundaries
- Never execute the original game, copied original machine code or oracles, a debugger, hooks,
  capture or indirect launchers, and do not install tools. The only exception is an interactive
  lead session (below).
- Static inspection and source-built clone/test execution are allowed, including clone
  framebuffer/image output. Analyze only the registered reference EXE and adjacent assets under
  `Lemmings Revolution/`, resolved from this repository.
- Parked unless a playtest defect requires it: D1/D3/base clock, release/digger timing, timed
  Armageddon, shared timing beyond the accepted shared-dt path (walk/fall continuous, other states
  hybrid 5 Hz), separate gate clocks, original depth-test fidelity, clipping/controller/capture.
  Observing a clone failure does not authorize fixing a park. History and rationale:
  docs/plan-history/.
- Do not change completion criteria, historical failures/exclusions, model pins, permissions, or
  smoke-test assertions/timeouts to obtain a pass.
- No subagents, background work, concurrent writers, commit/push, reset, clean or stash. Preserve all pre-existing
  edits. Workers never edit the queue, verifier, controller, tool registry or these instructions.

## Evidence
Name the playtest entry, the installed level/route, the observed or source-traced failure, and the
production call path you changed. Keep static instructions, pseudocode, inference, source tests,
clone observations and original comparisons distinct. Retain exact binary identity and addresses.
Check arithmetic, layout, ordering and state transitions; do not invent semantics. Unknown fields
are not automatically uninitialized fields. Archive detailed evidence under build/ and keep the
report under docs/scratch/ concise, ending with a two-line handoff and a what-to-check note
(level, action, expected result).

## Builds and saves
Use the registered tool paths in agent/tools.windows.json and `.claude/rules/windows-msys2-build.md`;
do not rely on whichever compiler is first on PATH. All text is LF. Do not add original binaries or
assets to version control.
The clone's normal save is LocalAppData/LemmingsWebolution/SavedGame.dat and the installed game's
save is never written. Tests and workers use a separate temporary save path. Use the accepted
original-format save codec; do not silently import or reset saves; preserve unknown supported fields
and the prior good save on write failure. WEBOPROG is not retail compatibility evidence.

## Verification
User approved (2026-11-15) the smoke tier as the per-task gate.
- The supervisor runs `verify.py --profile smoke` after the worker exits (8 build jobs, 4 test
  jobs): the tests `cmake/test_tiers.cmake` labels smoke, including the 102-level sweep, plus the
  recovery suite. Report it as pending; do not duplicate it.
- Every other main test is legacy and runs only at milestone sign-off (`--profile all`). A legacy
  test broken by an intended change is removed with a one-line reason in the report; that is the
  only permitted way to drop a test.
- No before/after replay fixtures, frozen-source builds, identity checks or per-task scripted
  routes. Add a test only for logic a later change could silently break; it runs in seconds and
  stays legacy.
- Acceptance of a playtest fix is the user's next playtest; review checks the call path, the diff
  and the smoke result.

## Static analysis (REA)
Implementation, research, planning and review passes may use the project-owned static REA profile:
the registered reference hash, the Ghidra provider and named static tools only. Ghidra/Java analysis
is not game execution. Start from retained evidence, cross-check pseudocode against instructions,
and close analysis without snapshot arguments. No runtime, debugger, capture, metadata mutation or
snapshots. Planners and reviewers are read-only and do not build or test.

## Agent execution
The sequential controller is described in agent/README.md. Tasks finish needs_review; only a
separate review makes them done. Preserve retry history. Stop for missing access, required
approval, user parks or genuine evidence blockers; do not loop on them.
Planning records Difficulty (routine/difficult/exceptional), Recommended model (Sonnet/Opus/Fable)
and a short evidence-based reason. This is advisory: the user's explicit model or backend choice
always wins, recommendations never change pins, and a new Fable escalation needs explicit user
approval.

## Lead (interactive session only)
The user appointed the interactive session project lead (2026-11-12) with authority over sequence,
queue, controller, verifier, model pins, documentation and commits. The lead edits protected controls
only under `supervise.RepositoryLease`, with backups under build/ and no live runner, and does not push
without telling the user. agent/RUN_STATUS.md is generated; the lead reconciles the short plan/status
at milestones.
The user approved (2026-11-12) running the original game, with hooks, debugger, capture or REA
runtime tools where they help, in interactive lead sessions where the user can see what is launched.
Before each launch state what runs, why and for how long; report what actually ran. Back up before
changing the installation, never overwrite the installed save, and keep original binaries/assets out
of version control. This permits observation and comparison; it does not by itself reopen parked
fixes. Unattended workers, planners, reviewers and the controller never launch the original, and
oracle-001 stays blocked for the queue.
Preserve docs/STATUS.md's gate ledger, recovery PROGRESS/FUNCTIONS, the scoped 372/6,186
historical recovery baseline and accepted task records. Earlier instructions are archived under
docs/plan-history/.
