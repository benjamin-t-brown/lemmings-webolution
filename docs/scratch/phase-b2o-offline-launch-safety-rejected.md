# B2o — offline safety-boundary candidate rejected (2026-10-27)

New approved pass `offline-launch-safety-repair-2026-10-27`, directive SHA256
`d273f6141564d52b5c5cccbc370f4ed40f256e97baec8e221b37fd3d0db54307`.
One worker attempt only, 90min overall/≤60min worker with30min verification reserved,
zero stock starts. This is distinct from the completed [B2n review](phase-b2n-offline-recovery-review.md)
and consumed native pass; old counts, breach and frozen budgets remain unchanged.
Owner `ses_e80f12432ffebdYi3RdAhbYN8v`, parent `ses_e80f36524ffe7KzSDCj9yVcKCp`.
Packet prefix: `build/tmp/offline-launch-safety-repair-2026-10-27/`.

## Disposition: incomplete prototype, NOT accepted repair

Worker `ses_e80d558e2ffex59m3wEXW6punp` returned an isolated/unwired Python
launcher, worker and seven-test suite, explicitly reporting incomplete acceptance.
Facilitator independently confirms **mandatory guard-bypass controls are missing**
and an actual-boundary **approved-argument binding failure**. Verification fails;
the directive requires this pass to stop. No second worker, repair dispatch,
facilitator completion of the missing controls, production wiring or live trial.

Uncommitted candidate files retained for user inspection, **not approved for reuse**:

- `tools/harness/offline_entry_safety_candidate.py`
- `tools/harness/offline_entry_safety_worker.py`
- `tools/harness/test_offline_entry_safety_candidate.py`

Their SHA256 values in that order:

```
6defc77393b1db915240ea309fc83b342dfedb78f01d49f4914775242e6e5549
e51e8d006e7abe532fed37d9534ca93817caf7b580f8326c88690459db5c9a31
1671d78d40ba18efbbf7488d79738d07550e542fb1a70dccc4c2e4389ecf1b40
```

Candidate defaults have no process/Win32/desktop/capture/input adapter. Its built-in
`OfflineEffects` performs scratch bookkeeping and synthetic events only. No existing
capture/navigation module was imported or repaired. B1b/B2h seams remain existing
evidence, not newly qualified or wired to this candidate. Default main without
adapter refuses; TestOnly returns with no target effects in the tested paths.
These useful properties do not satisfy the full directive.

## Exact tests and coverage limits

After reading all three source files to confirm no native/desktop import/effect,
facilitator ran the entire returned suite, without altering it:

```
PYTHONDONTWRITEBYTECODE=1 python -m unittest tools.harness.test_offline_entry_safety_candidate -v
```

**7/7 pass**, facilitator log `facilitator-suite.log` (worker log separately under
`worker/unittest.log`). No game build/CTest or interactive app run was performed;
this is isolated, unwired Python-only code, not production/render behavior.

| Permanent test | What the passing result actually covers / does not cover |
| --- | --- |
| `test_actual_launcher_worker_success_and_testonly` | Actual launcher `run`→loaded worker `main`, synthetic success/receipt, launcher TestOnly zero events. Not approved-argument equality or bypass sensitivity. |
| `test_defaults_and_flags_fail_closed` | Default main without mocks refuses; unknown `--live` parser flag refuses. Does not bind supported arguments to an approval. |
| `test_stale_source_worker_failed_copy_and_dependency` | Different stale worker path refused before import, declared Python/API dependency mismatch refused. No actual failed-copy deployment transaction or independent source pin regression proved. |
| `test_worker_direct_testonly_does_nothing` | Direct worker TestOnly zero events. No isolated TestOnly-bypass mutant. |
| `test_singleuse_restart_concurrent_and_failed_capture_no_refund` | Direct `consume()` helper re-instantiation refuses; twelve competing helper calls yield one exclusive-create winner. Despite its name, no actual-boundary capture-failure/no-refund/crash test. |
| `test_evidence_exclusive_first_bytes_preserved_and_distinct_diagnostics` | Synthetic wrong-geometry and injected-exception branches differ; original receipt exclusive create refuses, preserves bytes. Not a bypassed non-overwrite implementation or native capture. |
| `test_isolated_guard_bypass_behavior_controls` | Misleading name: creates an identity-mutant file but does not load/run it; checks an unrelated deliberately wrong hash and builtin exclusive file open. No four behavioral bypass controls. |

### Required isolated controls: none completed as required

Independent AST inspection of the last test finds no candidate `run`/`main` call
or `exec_module` loading of its generated mutant. Direct read of test lines86–104
confirms the identity-mutant file is never executed. Its wrong-hash assertion can
pass regardless of the mutant's behavior. The other checks are not guard mutations.

| Guard disabled alone | Actual delivered control/result |
| --- | --- |
| Identity | Mutant file generated, not executed; generic wrong-hash refusal only. **Missing behavioral control.** |
| TestOnly | Positive inert-path test only. **No bypass mutation.** |
| Single-use | Positive helper exclusivity only. **No bypass mutation.** |
| Non-overwrite | Original adapter/builtin exclusive creation only. **No bypass mutation.** |

No “mutant rejected” result is fabricated. Facilitator did not implement these
missing worker deliverables. Test setup (`test…py:25`) also derives source/worker
hashes from whichever source is current, not independently pinned approved
fixtures; this is a test-provenance limitation, not alone proof every runtime hash
comparison is absent. No source mutation/restoration was performed by facilitator.

## Facilitator check — independent actual-boundary falsifier

Source read shows launcher argument parsing (`candidate.py:63-83`) and worker
parsing (`worker.py:8-21`) never compare supported arguments with approved expected
arguments. `OfflineEffects.audit_identity` (`candidate.py:23-28`) compares only
caller-supplied source/worker hashes and a declared dependency dictionary. There
is no approved-argument binding mechanism in this candidate.

A bounded separate checker, **not the worker suite/helpers**, independently pins
the returned file hashes in `candidate-hashes.json`, constructs a synthetic approval
fixture with `approved_argv` for attempt `approved-attempt`, then invokes the actual
candidate `run` and actual loaded worker with attempt `UNAPPROVED-ARGUMENT`. All
effects remain candidate synthetic events/scratch files. An audit hook additionally
refuses subprocess, system/startfile, DLL loading and network-connect effects.

```
PYTHONDONTWRITEBYTECODE=1 python build/tmp/offline-launch-safety-repair-2026-10-27/verify-rejection.py
```

Expected contract: supported-argument drift refuses before effects. **Observed:**
return0, `identity-ok, identity-ok, consumed, attempt-created, mock-capture, receipt`,
receipt branch `success`, attempt `UNAPPROVED-ARGUMENT`. The independent fixture's
approval field is ignored; this is not claiming `approved_argv` was a documented
candidate schema field. Rather, static read and observed execution establish that
the required argument-approval contract has no implementation. Result preserved
in `independent-rejection.json`; synthetic state/receipt in
`independent-rejection-state/`. This alone falsifies acceptance. No real target or
desktop effects, source mutation, new capture logic or repair resulted.

Further unverified clauses (not silently accepted): full relevant-dependency
deployment identity, actual-boundary crash/restart and capture/owned-exit failure
retention, source/attempt/result identity completeness, and all four bypass tests.
Exclusive create/fsync is useful tested local-file behavior, not universal
power-loss/parent-directory/cross-host durability. A failed overall candidate is
not license to finish those clauses in facilitator verification.

## Integrity, clocks, resources and next decision

Independent preservation check:

```
python build/tmp/offline-launch-safety-repair-2026-10-27/verify-integrity.py
python scripts/check_agent_framework.py --brief build/tmp/offline-launch-safety-repair-2026-10-27/brief.md
git diff --check
```

Preserves exact previous stopped review object and all older archive entries,
all16 gate texts,16 untouched baseline paths (including existing seam source and
dirty framework/directives/review evidence),236 prior saved evidence files. Source
hashes recorded above remain unchanged through verification. Pre-existing dirty
PLAN/LESSONS/history hunks remain, with only this pass's scoped status additions.
No old B2m launcher/helper was executed or imported, including TestOnly; no new
original-binary research, installed-file access, target/desktop actions, screenshots,
tasks, hooks/debuggers, app integration, binding research, Phase D or gate changes.

Fresh activation uses approved-resume message `msg_17f25f631002a9BHXHrpnwZ3H0`,
2026-10-27T20:55:56.734Z — **not original session creation**. Absolute deadline
22:25:56.734Z; worker return tightened to21:55:56.734Z. Dispatch request
21:00:48.018Z; worker created21:01:04.268Z, succeeded/idle21:03:59.885Z:
**191.867s request→idle**,175.617s created→idle. One attempt/one review round;
zero starts/attaches/foreground/input/desktop exercises. No overrun; unused time
does not authorize a second attempt. Exact final overall/round/report clocks are
in queue and packet `final-clock.json`, including documentation/cleanup.

Worker terminal status independently checked; active endpoint lists only parent
overlord and facilitator. No owned live resources, processes/tasks/instrumentation
or restoration owed. Historical stock cleanup/hashes are not fresh measurements.
All changes **uncommitted**, HEAD remains `e649d10`, no push. Candidate remains
unaccepted/unwired; no follow-on worker or live trial.

**Retained, not fresh measurements:** C3 `.06354818984885621` FAIL versus `.02`;
C1 zoom1 `.0160863988750699` / elev_down `.016121214378294987` PASS. B2/B3
unaccepted, C2 native gate unpassed, B1 conditional. D1n landed, D1o failing and
Phase D/D3/digger-clock parks unchanged. Masks/references/exclusions unchanged.

**Smallest next decision:** leave this prototype parked/rejected for user review.
If the user wants another pass, it needs new explicit scope/authority to finish
approved-argument/source/deployment binding, the four actual-boundary isolated
controls and remaining failure-path evidence; unused clock is not that authority.
Do not advance to capture, input binding, integration or a live trial on these tests.
