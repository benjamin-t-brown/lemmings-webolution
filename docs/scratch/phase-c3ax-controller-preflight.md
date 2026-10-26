# C3ax initial/repair1 — real-host prelaunch evidence, narrowed scope

2026-10-25–26. Packets: `build/tmp/c3ax-controller-preflight/round-note.md`,
`repair1-round-note.md`, `controller_repair1.diff`, `initial/manifest.json`,
`repair1/` receipts. Two attempts; together with AW, **3/3** pass attempts used.
Zero stock starts/attach/injection/install changes. This does **not** certify a
launch-ready operational controller or authorize native observation.

## Result and correction

Initial AX ran a standalone prerequisite checker in real session1, serialized
Task Scheduler State with `.ToString()`, and reached its barrier with cleanup.
Facilitator returned it: hashing the old controller and duplicating its guard
did not exercise that controller's integration boundary. Initial eight files
are preserved byte-exact under `initial/`, all manifest hashes checked.

Repair1 imports the actual pinned entry helper and validates its runtime file,
exports, dependency and input hashes. The host receipt records string `Running`,
session1, helper RECORD_SIZE280/LEVEL_TABLE0x4E35D4, no conflicting resources,
terminal `REACHED_PRELAUNCH_NO_EXECUTION` and exact own-task cleanup. Worker PID
34480 exited and was absent; OS Process.ExitCode was null, so only persisted
main return0 plus wait/PID absence is claimed. Failed task-action path and first
wrapper exit-code-check attempts remain recorded; neither was a game start.

**Further narrowing:** the diff shows `main()` was reconstructed with the
operational suffix removed, not a minimal barrier/serialization edit to the
original main. The returned source genuinely exercises helper import, runtime
bindings, guards and cleanup, but cannot prove the unchanged C3aj operational
main would pass or preserve all original failure paths. C3aj remains unchanged
with its numeric-State serialization defect. The repair note's prose copies
the original SHA incorrectly in two places; actual source/receipt digest is
`a5f55e8d18b461bee5bed874f55b7e4b2d789c8010c855a450dc80369482e25b`.

## Facilitator check

Read original/deployed helper, returned sources/tests/wrapper and entire diff.
Independently inspected the six helper modules' top-levels: DLL handles and
ctypes declarations are established at import; no target process/input/memory
operations are called there. Rehashed current original/helper/payload/dependency
and install inputs. Returned barrier-only source SHA256 is
`a3a8e9ce26bec0f888c4d074b10d5e5a9e367298100db6cc0f0830a9155cc913`.

Own independent main-level fixtures (no worker fixture generator) pass five
outcomes: success, numeric-State refusal, wrong session, binding mismatch and
cleanup error. Own actual-source one-field mutation removed only the State
policy guard: numeric input reached the barrier, failing the independent
`NOT_REACHED` assertion. Exact source restored against backup and SHA above;
own checks and worker **9/9** tests pass again. No host execution of the mutant.

**Independently replayed the returned main on the actual host**, changing only
its task/output identity in memory. New Interactive/Limited task
`LemWeb-c3ax-facilitator-verify` ran Python PID26112 in session1 at23:57:41,
read actual `Running`, imported the actual helper, reached its terminal barrier
and unregistered itself. Separate PID/task absence and receipt checks pass.
No game/observer path exists in that returned source. Zero counters are
corroborated by source inspection, not treated as instrumentation of a native
launch. Own tools/receipts: `build/tmp/c3-preflight-facilitator/check_ax.py`,
`ax-host.py`, `ax-host.ps1`, `ax-host/`, `ax-*.log`. Worker receipts remain intact.

Cleanup postcheck: no Lemmings process, no `LemWeb-*` task, both returned-worker
and independent Python PIDs absent; seven installed-file hashes match AW pins.
No approval marker, attempt ledger, backup or live write was created. No slot
restoration applies. Fresh rebuild and **44/44 CTest** pass. No production change
or new image/gate measurement. C3 **0.0635481771 FAIL**, C1 zoom1/elev_down
**0.0160864/0.0161212 PASS** are retained baselines with unchanged references,
masks and floor inclusion.

## Concrete remaining blocker and recommendation

The old operational controller and the proposed AW observer have not been
integrated/reviewed as one launch-disabled, reproducible packet. The observer
is still a static design, with no executable script identity. Do not equate the
host-only copy's success with launch readiness. This bounded pass stops at its
three-attempt ceiling; no additional repair or stock start is authorized.

Recommend a separate **offline-only integration** brief: retain the original
controller path with a demonstrably terminal prelaunch barrier, repair its
actual serialization and verify that same path's host/ownership/failure logic;
pin and independently review a minimal observer implementation against AW's
design. Only after that packet should the user decide on one ordinary start /
one bounded REA-CLI attach including temporary injection. Runtime interface,
reachability, relocation, history/effective state and matched non-interference
remain separate unknowns, not defaults or synthetic successes.
