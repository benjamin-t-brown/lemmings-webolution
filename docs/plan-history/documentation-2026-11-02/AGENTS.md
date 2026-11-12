# Lemmings Revolution reconstruction

## Goal

`decomp-lemmings/` belongs to this project. Work in this checkout rather than
an old standalone copy.

Recover faithful, readable C++23 that builds with MinGW-w64 GCC on Windows.
Emscripten is a later phase. Preserve original behavior, arithmetic, layout,
ordering and state transitions where evidence establishes them. Binary identity
with the old compiler is not the acceptance criterion. Use system/portable
adapters for Windows and library boundaries; do not reconstruct the Windows CRT.

## Execution boundary

**Do not launch the original game without the user's explicit approval for
that run.** This applies to helpers, shell commands, REA runtime tools, native
capture, debugger/injection routes and indirect launchers. Do not install hooks,
swap DLLs, modify the installation, change saves, or attach to the game under
an unattended run. Existing documentation of live workflows is not approval.

Default work is static analysis, source edits, source-built unit tests and
read-only asset parsing. Executing copied original machine-code fragments in
offline native oracles also needs explicit authorization; the default verifier
does not run them. Unknown behavior stays unknown. This restriction applies
throughout this repository, including the moved recovery tree.

## Start and finish each task

Rendering and gameplay across existing levels take priority over further audio,
optional asset variants and test-only expansion. Source-tested startup, saves and
injected wins are useful integrations, not proof of complete gameplay fidelity.

Priority order:
1. Generalize scenery-root selection beyond MM_CLIMB_002 for one additional
   installed level using original serialized structure and retained evidence.
   BackdropScene::build currently returns an empty backdrop for other levels and
   uses fixed offsets for the first. Do not simply remove that guard or add another
   per-level offset table. Trace root selection, supported node types, stage and
   model roles; integrate the existing renderer and demonstrate actual submissions
   and rendered output, preserving the first-level baseline.
2. Load selected-hatch camera orientation and initialize entry for other levels.
3. Replace file-order teleport pairing with evidenced serialized link IDs, first
   establishing a concrete installed-level mismatch or affected route.
4. Connect missing Armageddon/fast-forward commands only with established semantics
   and respect for parked timing work. Do not treat World::nuke as original proof.
5. Further audio and optional formats follow these gameplay/rendering dependencies.

Preserve D1/D3/digger and clipping/controller/capture parks and historical gate
failures. Broader scenery loading does not reopen full-frame capture or clipping
campaigns. No original execution or oracle is authorized. Do not loosen completion
criteria, invent behavior, or discard accepted audio/save work. Report which actual
level/route improves, remaining limitations and the next implementation dependency.

One bounded scene-roots-001 static prerequisite is explicitly authorized to establish
root selection and a supported additional level before production integration.
It is queued as research, uses normal verification and independent review, and is
exempt only from production-file changes. It must deliver address-backed evidence,
an implementation contract and explicit unknowns. It is not permission for generic
research or repeated housekeeping. After acceptance, propose the bounded production
scene integration; missing critical evidence must be reported specifically.

The immediate priority is the playable native milestone: ordinary startup ->
select an existing installed level -> play -> finish -> retain progress after
restarting the clone. Read latest accepted task reports alongside docs/STATUS.md;
older status descriptions may predate completed integrations. Do not redo them.

For each new task, name the milestone step, an actual installed level/resource
or concrete application route, its observed or source-traced failure, and why
this is the next dependency. Include these in task scope and require a before/after
demonstration in acceptance. Distinguish observed failures from source inference.
Synthetic fixtures may supplement the demonstration, but optional formats,
absent asset variants and synthetic-only edge cases wait unless evidence shows
they block this milestone. After acceptance, report which milestone steps work,
which remain unverified, and the next dependency; do not automatically extend
the same parser.

Persistence needs an explicit save compatibility/storage contract. Do not assume
WEBOPROG equals retail compatibility, overwrite installed saves, or invent original
behavior. If evidence cannot resolve a necessary decision, report the concrete
blocker for the user instead of substituting unrelated easy tasks. Preserve user
parks and original-execution restrictions. Audio, wider fidelity and all existing
completion requirements remain necessary; this milestone does not replace them.

Priority is missing native runtime functionality. New tasks must implement or
connect a concrete behavior in production C++, with a traced application/runtime
call path and a before/after scenario. Choose the earliest unparked milestone dependency with sufficient evidence.
Loading and rendering changes must demonstrate why that route needs them.

Tests, evidence notes and build edits support that implementation. Do not spend
successive tasks on standalone test expansion, ledger cleanup, additional byte
hashes or coverage bookkeeping. If a prerequisite prevents implementation, name
the exact runtime change it blocks and stop for that decision. Do not manufacture
code changes or infer unknown game behavior to satisfy this priority. Unknown
field meanings do not imply uninitialized storage: preserve evidenced stores.

Read `agent/README.md`, `agent/tasks.json`, `agent/tools.windows.json`,
`docs/STATUS.md` and relevant findings before changing code. The current
roadmap is `agent/IMPLEMENTATION_PLAN.md`; the older root `PLAN.md` link is
currently broken. Preserve user-parked D1/D3/digger work unless reopened.
Keep the existing `.claude/rules/windows-msys2-build.md` build conventions.

- Use the current tool registry, not whichever GCC appears first on PATH.
- Prefer a small evidence-backed routine or subsystem boundary per task.
- Treat decompiler output as a draft; retain address and exact EXE SHA-256.
- Check x87 precision/conversion order, calling convention and field offsets.
- Keep unknown fields explicit, layout assertions and allocator boundaries.
- Separate static evidence, source tests, native comparisons and inference.
  A passing source test does not prove agreement with the original game.
- Preserve historical failures/exclusions in `docs/STATUS.md` and recovery
  `docs/FUNCTIONS.md`; do not inflate coverage or equate partial with complete.
- Do not reset, clean, stash, overwrite, auto-commit or push another worker's
  work. Inspect the initial dirty state. Keep logs under ignored `build/`.
- Do not add game binaries/assets to version control. Existing tracked copies
  are a migration debt, not permission to copy more.
- Use `Lemmings Revolution/Lemmings Revolution.exe` and the adjacent `.box`
  archives relative to this repository root. The local game folder is ignored;
  do not use the old installation outside the repository as the default. Resolve
  paths against the repository root, not the caller's current directory.
- Use LF line endings. Run `python tools/agent_control/verify.py --profile all`
  for the baseline, or document the narrower check and its limits.
- Record findings, changed files, actual verification and next step. Queue work
  finishes as `needs_review`; an agent exit code alone never marks it done.
  A separate read-only review must check every acceptance criterion before the
  continuous controller marks the task `done`.

## Supervised runs

Use `tools/agent_control/continuous.py` to plan, implement, verify and independently
review successive queue items until completion or a blocker. Preview is the
default. `supervise.py` remains available for one selected task. Use one worker
at a time, no subagent spawning or concurrent writers. Each implementation and
verification cycle is capped at 60 minutes; planning and review have separate
bounded budgets. The controller may retry fixable failures up to its attempt
limit, but never automatically restarts timed-out or interrupted workers.

The planner may add bounded unparked reconstruction work or refine an explicitly
`planning_required` placeholder, preserving dependencies. It cannot change
completion criteria, reopen user parks, remove existing tasks or authorize
original execution. `agent/completion.json` defines project completion; an empty
queue is insufficient. Full verification and an independent completion review
must pass. Stop for missing credentials, permissions, repeated failures or
required game execution; preserve work and record the blocker.

The supervisor bounds process lifetime and records evidence. Its prompt rules
are not an OS security boundary against arbitrary commands. Never use a bypass
permissions flag to make an unattended run succeed.

`decomp-lemmings/` is now an ordinary source folder in this repository. Its
files are staged for the next parent-repository commit; do not recreate a
submodule or nested repository. See `agent/recovery-tree-repair.md` for the
repair, file backup and verification. The old standalone copy is untouched.

## Save reconstruction decision

Use the original SavedGame.dat format for long-term compatibility. The clone
stores its own copy under the Windows per-user application-data directory
(LocalAppData/LemmingsWebolution/SavedGame.dat); never write the installed save.
Keep serialization independent from the storage adapter for the future web port.
Reading the installed save and reference executable as static data is authorized.
No original execution, native-byte oracle, hooks or live capture is authorized.
Do not assume existing WEBOPROG helpers or their unlock rules are faithful.

First complete save-research-001, a bounded, explicitly authorized prerequisite
to the production save reader/writer and restart integration. This task is exempt
from the requirement to change production C++; it must produce address-backed
evidence, unknowns and an actionable implementation scope. Use the normal bounded
worker, verification and independent review. Research acceptance does not mean
save compatibility or the playable milestone is complete.

The planner may not invent generic research tasks to escape production priorities.
Additional bounded research requires a concrete implementation dependency and an
explicitly authorized queued task. For this save task, use retained static exports
with Claude's Read/Glob/Grep profile; do not enable shell/MCP or relax permissions.
If exports do not answer a required question, identify exact addresses/data needed.
After research is accepted, plan a production codec task from its evidence, then
connect it to the current frontend state and demonstrate completion surviving
a clone restart. Unknown fields must be preserved where supported, not guessed.
Do not silently import, reset or overwrite the installed save. Isolate tests with
temporary save paths. Validate malformed/truncated input and failed writes; retain
the prior good clone save on failure. Byte-preserving round trips supplement,
but do not replace, evidence of the original reader/writer behavior.


## Worker verification and reports

Claude implementation workers may run the registered verifier through the exact
command python tools/agent_control/verify.py --profile all (also main or recovery).
They may read HEAD main.cpp with git --no-pager show --no-ext-diff --no-textconv
HEAD:src/app/main.cpp. Register demonstrations and asset measurements as bounded
source-built CTest tests; the verifier supplies the registered compiler and dummy
SDL drivers. Preserve frozen source provenance and all pre-task uncommitted work.
Run verification before finalizing reports so executed evidence is available.
A worker must not edit the verifier, queue, controls or original files to make
a permitted command do something else. Original execution remains forbidden.
Planning/review remain file-only; they consume worker and supervisor evidence.

## Authorized next scenery prerequisite

scene-roots-001, scene-integration-001, camera-entry-001, native-023 and
scene-admission-001 are accepted. The earlier first-level-only restriction is
historical: structural support now admits ten installed levels. The immediate
next dependency is scene-branch-research-001, explicitly authorized by the user.
It is one bounded static investigation of Secondary type-2 branch-1 drawing and
nested Secondary dispatch, using retained disassembly. Deliver an address-backed
implementation contract, beginning with MM_DIG_000 scenery; this does not reopen
digger gameplay. Research acceptance is not implementation or rendering acceptance.
The planner may propose the resulting bounded production implementation only when
critical behavior is established. No generic research expansion, original execution,
or reopening of D1/D3/digger/clipping/controller/capture parks is authorized.

## Finish the authorized scenery investigation

The user authorized scene-branch-research-002 to finish the same scenery
investigation after scene-branch-research-001 delivered partial findings. This
supersedes the earlier one-pass restriction for this concrete dependency only.
Follow the necessary static call/data dependencies until the MM_DIG_000 branch-1
rendering contract is implementable or a genuine evidence/access blocker is
established. Retained but unread routines, an arbitrary pass boundary, or a list
of remaining questions do not satisfy completion. Preserve the accepted first
report as partial evidence; do not represent its acceptance as implementation approval.

For this continuation, bounded read-only asset-extraction source tools/tests and
their CTest registration are explicitly permitted. They may read installed bytes
and write evidence under ignored build/scene-branch-research-002 only; do not
change production parsers/rendering, existing assertions/timeouts, model pins,
controller permissions or completion criteria. Claude can execute these through
the already permitted verifier. No broader shell access is needed. Capture actual
MM_DIG_000 records, offsets, names and fields rather than relying on a text view
of binary data. Normal verification, independent review, run limits and retry
limits remain. A genuine blocker must state the exact missing evidence, attempts
made and why remaining authorized static routes cannot resolve it; mark it blocked,
not done. No original execution or user-park reopening is authorized.

## Static REA access for Claude and Codex research

Explicitly user-authorized research implementation tasks receive the project-owned
rea_static MCP profile. Normal implementation workers and file-only planning/review
keep their existing launch restrictions. The profile exposes only named static
inspection/decompilation tools and restricts open_binary to the registered reference
SHA-256 with Ghidra. Runtime/debugger/capture, other targets, metadata mutation,
snapshot writes and server-initiated client execution are not exposed.

The launcher uses explicit MCP/settings files, no inherited settings sources, disabled
hooks and slash commands. Java/Ghidra paths are process-local; no machine settings
are changed. These config files and the proxy are hash-protected controls. Static
analysis may create analysis databases and start Ghidra/Java; it does not execute
the game. The profile is not an OS security boundary for other worker tools.
Read the available schemas, retain evidence, check decompiler output against
instructions, and close the analysis session without snapshot arguments.

Codex research workers receive the same static proxy through an explicit MCP
launch table and exact per-tool approvals. Other Codex roles keep their existing
configuration; no user-global MCP registration is changed.

## Static REA during implementation

The user also authorizes the same static-only REA profile for assigned Claude and
Codex implementation workers (including queue tasks with no explicit kind). This
supersedes research-only availability described above. Start with accepted research
and existing code; use targeted decompilation, instruction and cross-reference
lookups for concrete questions inside the task. Substantial unresolved dependencies
still require a blocker; this does not authorize unbounded research or new scope.
The identical target/hash restriction and tool allowlist apply. Planner/reviewer
access is now authorized as described below. Original execution, runtime debugging and capture
remain prohibited; model pins and all other permissions remain unchanged.

## Static REA for planning and independent review

The user authorizes the same target/hash-restricted static REA profile for Claude
and Codex plan, review and review_completion passes. This supersedes earlier
file-only role descriptions. Other generic read-only calls do not gain MCP access.
Planners may resolve narrow feasibility questions; reviewers may independently
cross-check claimed binary behavior. Start with retained evidence and stay within
the existing pass budget; missing substantial research remains a blocker.

File access stays read-only, with no new shell/build/test permissions. Hooks and
plugins remain isolated. Cite addresses and findings in the structured response;
the controller retains tool results in the phase response.jsonl. Do not write
new evidence files. Close analysis without snapshots. Static evidence does not
prove runtime fidelity. Original execution, debugger/capture, model pins, queue,
acceptance criteria and existing parks remain unchanged.

## Authorized nested scenery prerequisite (2026-11-01)

scene-branch-impl-001 is accepted: the retained census admits 84/102 levels,
with 18 still rejected for nested/non-leaf Secondary dispatch. These counts
supersede the earlier ten-level state; they do not establish full game fidelity.
The user authorizes scene-nested-research-001 as the next bounded static REA
investigation of that concrete rendering dependency. Establish traversal,
visibility and ordering from reference instructions and actual installed records.
Read-only asset extraction source/tests and their bounded CTest registration are
permitted, writing evidence only under build/scene-nested-research-001; no
production rendering/parser changes in the research task. Preserve assertions,
timeouts, controls, model pins, accepted work and all execution/timing parks.
After independent acceptance, the planner should propose the corresponding
bounded production integration for an identified installed level, with actual
application rendering evidence and regression preservation of the 84 admitted
levels. Do not introduce generic research or invent unresolved semantics.
Research acceptance is not runtime acceptance. Normal worker/verification/review
budgets apply; genuine missing evidence must be reported as a concrete blocker.

## Authorized gameplay command prerequisite (2026-11-01)

scene-nested-impl-001 is accepted and all 102 effective installed levels are
admitted to scenery rendering; this is not whole-game fidelity acceptance.
The next authorized task is gameplay-commands-research-001: bounded static REA
research into Armageddon and fast-forward, followed after independent acceptance
by evidence-backed production command integration where compatible with parks.
It may trace timing reads/writes, command reset/latch logic and simulation-loop
interactions needed to define the commands. It may not fix/recalibrate the base
simulation clock, hatch release timing, digger cadence, D1/D3, clipping or capture.
If faithful integration needs a parked change, document that exact dependency
and stop that implementation path for a user decision; do not silently reopen it.
Research may change reports and necessary bounded read-only extraction tests and
CTest registration, not production behavior, controls or existing assertions and
timeouts. Preserve original-execution restrictions, all model/permission settings,
accepted work, historical gates and completion criteria. Research acceptance does
not prove command behavior at runtime. Normal verification and review apply.
