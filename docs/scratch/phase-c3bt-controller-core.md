# C3bt — prior core defects corrected, full core acceptance still fails

2026-10-27. **Not accepted; stop before integration.** The new candidate fixes all
**14/14** independently replayed C3bs control-outcome contracts. Worker tests pass
**11/11**. However **4 additional mandatory checks fail**(three mode/listing cases
and launch-receipt metadata): full independent result**14/18**. These are existing
hardware-only/receipt requirements, not new permission or weakened success rules.
Do not spend the conditional integration slot as another repair of this failure.
No live trial or further worker dispatch occurred.

## Authority, counting and remaining pools

Current directive SHA256
`f345edcf71ccc8fec3327c3024a69c6cc7fb2415fa6e2e1d702aeddf3d70fbed`.
Governing [automation authority](../DIRECTIVE-AUTOMATION-2026-10-27.md) SHA256
`d63e55a3838dcf4d267d20f52100cdf167a507448d883181f48098c2de8e9960`.
It is **active authority for this continuation**, not unrelated task text or a
retroactive change to C3bs. It remains byte-exact, unmodified and unstaged by us.

User requested continued automation after bounded test+original-game start/session
permission and next-pass preauthorization. Queue/handoff now record that authority,
its fixed**07:07:42–10:07:42UTC**window and parent evaluation boundary. No repeated
routine user approval is required within its limits; parent still issues each
successor directive. This offline directive grants no live execution.

Fresh cumulative task**19**, **window attempt1of4**spent; **3of4 remain**. Current
pass used only the first of at most2slots; task20/integration was **not dispatched**
because first-task acceptance failed. Historical observation/feasibility**21**,
installation**1**, actual surrogate trial**1**and offline repair18 remain separate.
Nothing resets old caps/failures by renaming. After this second unsuccessful
offline correction outcome, reassess rather than repeat under the integration slot.

Live window pools unchanged/unconsumed: **1 additional surrogate session/start**
and **1 conditional original-game session/start**. A failed live launch spends its
allowance; no retry, frontend switch or replacement target; failed live validation
ends this window's live route. Parent review of runtime validation and a supported
safety/source-binding plan plus a separately scoped directive remain prerequisites
for original-game use. C3bc refusal and excluded methods remain. Standing permission
and conditional successor preauthorization persist, but cannot activate on this
unaccepted candidate. No routine permission question was raised.

## Candidate identity and actual-path improvements

`build/tmp/c3bt-controller-core/controller.py`(249lines), SHA256
`3608e6aa342dfa8ff89d39d9c851abafc5d7fa753c32bab011a64f6cc9655cb6`.
Tests SHA256`4c8fb6c146dbce4bb01d284583df7f724d586d636510dc5187a2670a11fb9434`;
worker output SHA256`562a8a1224bdc1bc2090690501619d4a890277fa951df2f2f5924462899b669f`.
Worker packet:`build/tmp/c3bt-controller-core/round-note.md`.

Same candidate`run(transport,process,clock,receipt,config)`entry is tested, not an
alternate miniature state machine. Default CLI refuses; only`re`is imported.
Returning command/launch/poll/receipt seams receive a bounded timeout with
monotonic post-return checks and cleanup reserve. This cannot preempt a blocking
implementation; no concrete native adapter exists and none was tested.

Verified corrections to C3bs: exactly one dot-selected native PID/TID; fresh context
EIP agrees with stop; errors checked at each command response; generation/complete/
framed response envelopes; empty post-clear listing accepted only with that explicit
mock framing contract; correlated target creation/PID and consistent decimal/hex
typed exit code; visible final receipt failure. Positive exit events remain
**synthetic typed events**, not captured CDB exit evidence or a parser for actual
exit text. Target creation/TIDs are injected known mock identities, not discovery.
Raw BR missing-main-hit/early-cleanup replay still fails, clears then does not resume;
no native cause was investigated and BR is not redefined as successful.

## Facilitator check and remaining concrete blockers

Own safe command:

```text
python -B build/tmp/c3-controller-correction-pass/check-core.py
```

Worker's guarded11-test suite rerun independently with
`python -B build/tmp/c3bt-controller-core/test_controller.py`also passes. Own
checker uses separately written fakes, exact retained BR69–70 hardware rows and
BR multirow selected-thread shape, and executes the **actual unmodified candidate**.
Before execution, deny-only process/native modules, OS functions and audit hooks
are installed; candidate imports inspected as`re`only. **Zero native/process
boundary attempts**. Python test runners only, no debugger/target/WinAPI operation.

Prior14cases all meet their outcome contracts: positive and multirow selection;
savedBRfail/no further`g`; NoRunnable abort; source intent rejected; unknown debugger
return unknown/fails; operation timeout; partial launch/no retry; context/clear/list
errors rejected; inconsistent or wrong-target exit rejected; final receipt failure
does not return success. Code also retains a primary exception when final receipt
fails; that combined-failure branch was read, not a separate independent probe.

Additional required cases:

|Actual-entry probe|Observed candidate behavior|Requirement unmet|
|---|---|---|
|Initial listing omits processor type/size fields|Returns success|Parser only reads enabled ID/address, so cannot verify two hardware execute-size1 observations.|
|Pre-clear listing omits those fields|Returns success|Cleanup precondition likewise does not prove expected observation mode.|
|Listings specify read/write or write-size4 instead of execute-size1|Returns success|Wrong processor access type/size accepted.|
|Positive trial with known argv/cwd|Returns success, but debit and final durable rows omit both|Launch-command receipt regression: BS had these fields, BT does not. Configuration was available; not an unavoidable unknown.|

The first three fixtures only edit listing text, **never arm actual software or
data breakpoints**. This is a mock acceptance failure, not evidence that any
excluded live method was used. It directly checks brief acceptance3's exact owned
hardware listing and unchanged hardware-only policy. The receipt case checks
acceptance5 and retained exact-command/receipt requirements.

`_breakpoint_listing`lines29–39 matches`id e address`and ignores the following
processor type/size. Initial and pre-clear checks at159/184 therefore accept equal
ID/address sets despite wrong modes. Microsoft's primary
[bl documentation](https://learn.microsoft.com/en-us/windows-hardware/drivers/debuggercmds/bl--breakpoint-list-)
(revision`e618b55a431869daca39a8c0fc5598e89956f8ae`) explicitly distinguishes
processor type/size fields; its example without these fields is not processor-type
proof. Retained BR hardware rows actually include`e 1`; candidate worker positive
`LIST`fixture omits those fields. Do not treat the fixture as measured HW validation.

Receipt state constructed at109–113 drops argv/cwd, although config is passed to
launch at149. No stdout/stderr acquisition is implemented either; do not imply an
integrated real-output receipt. Framing completeness/generation remain trusted mock
metadata, not actual transport proof. All adapter/integration work remains unrun.

Independent isolated old-defect mutations reintroduce first-row selection, ignored
context error, and missing exit correlation/consistency, plus swallowed receipt
failure: five regression observations discriminate those defects. A separate
**single-change in-memory listing-parser verification mutation**requiring`e 1`
makes all three mode-negative cases fail while preserving the genuine hardware-row
positive. These are temporary verification diagnostics only; **the handed candidate
file remains byte-exact**, not repaired by the facilitator. No additional dispatch.

Raw independent results/receipts/commands and source-lineage diff:
`build/tmp/c3-controller-correction-pass/core-independent-results.json`and
`core-lineage.diff`. Results JSON SHA256
`486e91f7a2e24f4d46ff4cbc534e53edc0b2f4d97eec3adc0a1e0917506164e3`.
Detailed checks are local shared ignored scratch, not implied synced by Git.

## Preservation, resources, clocks and stop

Entire BR/BS/controller-verification packets remain unchanged, as do fixed package/
game/source inputs and all ten initial dirty paths, including governing automation
directive. Candidate/tests unchanged after tests/mutations. Selective staging
excludes unrelated dirty hunks and authority-file edits. Historical notes/archive
remain intact. No production/renderer/framework change, native/original-file
research, source-binding continuation, install/update/security/policy/ACL/elevation/
host-task action or live process inspection/control. Only pure guarded mock runners;
no live owned resource acquired or cleanup required. Worker terminal/idle confirmed.

Retained gates, without render/rescore: C3**0.06354817708333334 FAIL**against0.02;
C1 zoom1/elev_down**0.0160863988750699 / 0.016121214378294987 PASS**. Cylinder,
platforms/shared camera, references/masks/floor and C3bc/native parking unchanged.

Pass/task start**07:32:15Z**, task19 dispatch**07:35:40.210376Z**; task deadline
**08:17:15Z**, pass deadline**09:02:15Z**, macro deadline**10:07:42Z**, none reset.
Actual verification/landing/end clocks and remaining pools in queue/archive.
**Stop with partial verified improvement and full acceptance failure; parent
reassessment before any successor. No integration-slot repair or live activation.**
