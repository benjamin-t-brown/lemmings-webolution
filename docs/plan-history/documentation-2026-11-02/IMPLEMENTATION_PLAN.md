# Faithful native reconstruction plan

Build faithful, readable C++23 with MinGW-w64 GCC on Windows first; defer
Emscripten until the native version is accepted.
No original-game launch is authorized by this plan.

## Immediate implementation priority

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

Accepted save-research-001, save-codec-001 and native-022 remain done.
Consult their reports alongside older status descriptions. Emscripten remains later.

## Current state

- Main `src/` already builds an application with SDL2 presentation, software
  rendering, frontend routes, parsers and simulation. `docs/STATUS.md` retains
  specific fidelity passes, failures, unknowns and user parks. Reuse that ledger.
- `decomp-lemmings/` contains recovered routines, layout checks, a function
  ledger, tests and historical binary evidence. Its recorded 372/6,186 figure
  includes partial recoveries and is not complete fidelity or usable-game coverage.
- `decomp-lemmings/` belongs to this repository as ordinary source files.
- The main repository already tracks some installation files despite its
  no-assets policy. Inventory them separately and retain needed local evidence
  before removing them from version control.
- Use `tools.windows.json` for tool paths, versions and the reference digest.
  Confirm CLI authentication and provider access before assigning work that
  depends on them.
- The reference EXE and `.box` archives live in the ignored
  `Lemmings Revolution/` folder. Resolve game paths relative to the repository
  root. Migrate legacy harness defaults or supply explicit repo-local paths
  before use. Their existence does not authorize execution.

## Sequence and acceptance

1. **Establish preservation and native baseline.** Keep existing source and
   build trees intact. Use separate `build/agent-native/main` and `recovery`
   directories. Build with the registered x64 GCC, run the source tests and
   read-only retail corpus tests, and retain logs/JUnit. Passing these tests
   demonstrates source consistency; it is not new original-game agreement.

2. **Select one bounded reconstruction slice.** Reconcile `docs/STATUS.md`,
   recovery `docs/FUNCTIONS.md`, source and retained binary evidence.
   Choose an unparked routine or native-to-portable adapter with a small
   dependency closure. Record exact address, reference identity, data layout,
   arithmetic and effect ordering, callbacks and unknowns before implementing.
   Do not restart the existing renderer/frontend from the older recovery plan.

3. **Implement and verify incrementally.** Preserve behavior in readable C++;
   retain layout assertions, explicit unknown storage and allocator boundaries.
   Add tests for meaningful edge cases (aliasing, boundaries, integer wrap,
   field offsets, conversion order, x87 effects) as appropriate. Run native
   GCC source checks. Label each routine static-only, source-tested, partially
   compared or native-verified within an explicit scope. Never substitute
   assumed classic-Lemmings behavior for original evidence.

4. **Close native integration gaps.** Connect recovered loading, simulation,
   rendering, frontend, saves, input and audio through explicit interfaces.
   Preserve the existing fidelity gates and failures. Windows/DirectX/CRT calls
   become suitable library/platform adapters with a stated behavior contract;
   they are not targets for wholesale decompilation. Resolve user-parked work
   only when the user reopens it.

5. **Request a concrete original comparison only when needed.** Prepare exact
   binary, route, duration, tools, proposed installation changes, backup/restore
   and outputs before asking for run approval. Historical harness instructions
   and old approvals are not authorization for a new run. Copied original-byte
   oracle execution is also excluded from unattended defaults.

6. **Emscripten later.** Start portability work only after the native build
   meets agreed gameplay/render/input/save/audio acceptance. Then isolate OS
   services, timing, filesystem, presentation and threading for the browser.
   Existing Emscripten files are context, not a verified port.

## Work units and stop conditions

Use `continuous.py` to advance through queue items. Each implementation gets at
most 50 minutes of worker time plus 10 minutes for verification by default.
Separate read-only planning and review passes each have a ten-minute default
budget. The planner selects concrete next work within this native reconstruction
scope; it can refine placeholders marked `planning_required` after their
dependencies pass review. Avoid overlapping writers.

Implementation results remain `needs_review` until the independent reviewer
checks every acceptance criterion against files and verification evidence.
Fixable failures get at most three implementation attempts per task by default.
Stop for missing authentication/permissions, required game execution, repeated
failure, user parks or evidence that cannot be obtained. Interrupted workers
require inspection before resuming. Preserve partial work and the next step.
No auto-commit, auto-push, permission bypass or endless failure retries.

Use `tasks.json` for persistent task status and dependencies. Continue until
all tasks are done and `completion.json` passes full verification plus a separate
completion review, or until blocked/cancelled/a configured limit is reached.
Neither the worker nor controller can open the original-execution approval gate.

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
