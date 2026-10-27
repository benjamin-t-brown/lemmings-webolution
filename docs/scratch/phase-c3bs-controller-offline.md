# C3bs — offline controller candidate NOT accepted; successor blocked

2026-10-27. **Offline acceptance FAILED.** Worker tests pass10/10, but independent
actual-candidate probes satisfy only**7/14** declared contracts. Seven exposed
gaps include selected-thread parsing, ignored diagnostic/cleanup errors, weak exit
identity/code validation and silently lost final receipts. No further repair
dispatch or live validation occurred. Preserve the partial candidate unchanged;
do not promote worker mock success into controller/native readiness.

Fresh cumulative task**18**after17, directive SHA256
`18d862a82f14abd42cf1499e0bd5d95b17d4f8a7bf4ce0c436ee6e15b2269d84`.
Historical observation/feasibility**21**, installation**1**, actual surrogate
trial**1**remain separate. This pass is one offline repair attempt, **zero**
debugger sessions/native or surrogate starts/attaches/injection/process control.
Python mock-test execution was permitted and is not a target start.

## Candidate identity and scope

`build/tmp/c3bs-controller-offline/controller.py`(187lines), SHA256
`b2904cf4acf97a3e2b5c658b56dfa822edc831d25a7dde62879ca41b85d6af46`.
`test_controller.py`SHA256
`3e1f42d225ba4847d7386ad938dd590a59d22aa250dba0909299de62c0ffc6c3`.
Worker raw output`test-output.txt`SHA256
`0c2eb41f6e613330fa2dc2839966fbfed854349a016aac58428591e67a40927d`.
Worker packet/limitations:`build/tmp/c3bs-controller-offline/round-note.md`.

This candidate refactors C3br's central command loop into
`run(transport,process,clock,receipt,config)`keyword seams. Tests call this same
candidate entry, not a separate behavior model. It has no import-time launch;
default`main()`hard-refuses, and`run`requires explicit`live=False`and injected
dependencies. **No concrete live process/transport adapter exists.** It does not
wire actual breakpoint setup, command-stream/prompt framing, native TID discovery,
stdout/stderr capture or identity-safe release. These are limitations, not proof
the actual launcher is integrated/repaired for use. Complete old/candidate diff
saved as facilitator`lineage.diff`. Old C3br launcher/source/logs/binary/receipts
were not changed or executed.

## Worker results reproduced, with narrower meaning

Worker's guarded10-test suite rerun successfully with:

```text
python -B build/tmp/c3bs-controller-offline/test_controller.py
```

It covers a simplified positive protocol; saved C3br stop fragments plus explicitly
synthetic selected-thread replies; early cleanup remains failure; NoRunnable,
wrong address/one-row TID, failed/garbled disarm, source intent without exit,
unknown debugger return, timeout/partial launch and default refusal. It also
executes five in-memory source perturbations. The “fixed ordinal” perturbation
replaces a marker branch with`if True`; it is behavioral discrimination, **not**
literal replay of the original seventh-prompt implementation. Do not overstate it.
Mock replies like`No breakpoints`and one-row thread lists are synthetic assumptions,
not actual CDB output dialect/framing verification. No live disarm/exit observed.

## Facilitator check: independent actual-path discrimination

Own checker`build/tmp/c3-controller-offline-pass/independent-check.py`reads and
compiles the exact unmodified candidate, invokes its actual`run`with separately
written transport/process/clock/receipt fakes and checks emitted commands/results.
Before candidate execution, it installs audit and deny-only native/process module
and OS-function guards; inspected candidate imports are only`json`and`re`.
Zero guarded native/process boundary attempts. Worker suite installs its own
pre-import guards; neither checker imports/executes the old launcher.

```text
python -B build/tmp/c3-controller-offline-pass/independent-check.py
```

Raw results, all emitted commands and progressive receipts:
`build/tmp/c3-controller-offline-pass/independent-results.json`and`.log`.
Seven conforming probes: minimal positive mock; saved C3br stays failed with clear
then no further`g`; NoRunnable abort; sourceEXIT_INTENT rejected; unknown debugger
return staysnull/fails; late operation rejected; partial launch has no retry.

Seven **nonconforming** probes:

|Probe|Candidate result|Independent finding|
|---|---|---|
|Realistic multirow thread listing|Rejects otherwise intended sequence as marker-hit excess|`_selected_tid`lines23–28 allows optional dot and takes first match: unselected maina4f8 instead of selected later58d0. Raw multirow shape is retained in C3br log79–81.|
|Fresh`r`response contains error|Returns success|`context`is stored, not validated; only`g`event goes through error parser.|
|Post-clear list says`No breakpoints`plus error|Returns success and continues|Lines125–129 test rows/phrase, not full response/error validity.|
|`bc *`reports error|Returns success given subsequent mock empty list|Command response is never checked for failure. Fail-closed error contract unmet.|
|Exit decimal0/hexC0000005 disagree|Returns success|Lines151–154 ignore hex value rather than cross-checking.|
|Arbitrary PID exit line|Returns success without correlation|No target PID/creation identity is established in candidate; cannot prove line belongs to owned target. This is missing correlation, not a measured unrelated process exit.|
|Final durable receipt sink raises|Returns success while final write lost|Lines173–179 silently swallow receipt-finalization failure; last durable row remains active, result is not reliably recorded.|

Two **temporary independent single-change verification mutations**, each compiled
only in memory, show actual behavioral discrimination without repairing the handed
back candidate: require the selected-thread dot, and separately reject error text
at the command-response boundary. The first makes the realistic intended sequence
pass; the second makes each of the three error probes fail closed. Each change is
isolated, all four affected contracts discriminate, and original candidate SHA256
is unchanged before/after. No mutated source was written back or promoted.

Additional read limits: clock checks bound returning mock transport operations,
but launch/poll/receipt boundaries have no enforced operation timeout; unreturned
blocking native calls are not tested/preempted. No live adapter means no proof of
prompt freshness/chunking, actual dialect, loaded engine or runtime thread coverage.
Requiring predetermined main/later native TIDs is an injection seam, not runtime
discovery of a not-yet-created thread. These gaps further preclude execution readiness.
The main-thread missing-hit cause remains unexplained; no native investigation.

## Standing authorization and stop boundary

Preserve user's sequence: **“approve”**offline repair; **“i authorise debugger
sessions or program starts from now on”**; clarified **“Tests and original game”**
with bounded caps/safety and conditional native use; **“and i preauthorize the next
pass”**. This is standing bounded permission, not unlimited dispatch or scope now.

Conditional successor remains preauthorized **but unconsumed and blocked by this
failed acceptance**: parent must accept independently verified offline repair,
issue successor directive and recheck idle ownership/hash. That specific successor
would allow default-Sol, one debugger session/one disposable IA32 start/45minutes
including verification/cleanup, no retry/extra repair dispatch, **zero original game**;
no additional user launch approval is needed once its conditions actually hold.
No successor is dispatched in this pass. Original-game starts remain out of scope
both here and in that successor; runtime validation plus a supported safety/source-
binding plan and separately scoped directive are prerequisites. Do not erase the
standing permission or imply it satisfies those prerequisites.

C3bc refusal, original safety/source-binding readiness and orientation/clipping
parking remain. Retained C3**0.06354817708333334 FAIL**against0.02; C1 zoom1/elev_down
**0.0160863988750699 / 0.016121214378294987 PASS**. No build/render/rescore, production/
renderer/framework change, original-file research, install/update/security/policy
override/ACL/elevation/host task or process control. Only permitted offline Python
runners/mocks; no live owned resource acquired, so no live cleanup/control needed.

Preservation manifests in`build/tmp/c3-controller-offline-pass/`check prior evidence,
entire failed C3br packet, fixed package/game/source inputs, initial nine dirty
deltas and a subsequently observed unrelated untracked automation directive. That
additional file was not read as task authority, edited, staged or executed.
All old history retained. Own candidate/test packet remains byte-exact.
Actual clocks/terminal worker status and task accounting are in queue/archive.
Start**07:04:34Z**, original deadline**07:49:34Z**unchanged. **Stop and return blocker
for parent evaluation; no additional repair or conditional successor consumed.**
