# B2q — boundary behavior verified; scope exception parked for review

2026-10-27, directive `offline-testonly-worker-boundary-2026-10-27`, SHA256
`9a2314b6b7c54a2a6fbb552bdc9cd1d86d3643300b0bbfc2dadd90cdbaa529dd`.
Owner `ses_e80f12432ffebdYi3RdAhbYN8v`, parent `ses_e80f36524ffe7KzSDCj9yVcKCp`.
Packet prefix `build/tmp/offline-testonly-worker-boundary-2026-10-27/`.

**Technical result:** TestOnly/direct-worker component behavioral checks verified.
**Procedural disposition:** parked for user review of a disclosed out-of-allowlist
temporary-state write; this is **not an unqualified clean pass**. Whole launcher
remains unwired/unaccepted. No repair dispatch, live trial or automatic next component.

## Scope exception — retained, not concealed

Worker reports its first test invocation inherited the old B2p test TemporaryDirectory
parent, creating transient test state under
`build/tmp/offline-source-argument-binding-2026-10-27/worker/`, outside this brief's
scratch allowlist. It reports `TemporaryDirectory.cleanup()` removed that state,
then corrected the test root to B2q before its final run. This violated the scoped
write boundary even though no game/desktop action or protected byte change resulted.
The opening blanket scope claim in the worker note is narrowed by this disclosure.

Facilitator independently confirmed **both the file set and hashes of all119 prior
offline artifacts are exact**, all236 earlier saved-evidence hashes exact, and no
`safety-*` temporary directory remains under the old B2p worker root. Cleanup's
historical execution itself was not independently observed; it is reported by the
worker, with present-state integrity independently checked. No existing source or
saved result was found changed there. No deletion of old evidence was authorized.

There is no second implementation attempt or facilitator fix. Behavioral proof is
recorded separately below; the component is parked pending user disposition of this
procedural exception and the aggregate round cap. It is not treated as a stock-start
breach or as evidence corruption, nor silently waived as a clean scoped execution.

## Implementation, before-source and fixed approval

Only these delivered source/test paths changed:

- `tools/harness/offline_entry_safety_candidate.py`
- `tools/harness/offline_entry_safety_worker.py`
- `tools/harness/test_offline_entry_safety_candidate.py`

Exact B2p before-source was copied before dispatch to `before/`; fixed manifests
`before-source-sha256.json` / `after-source-sha256.json` retain:

| Basename | Before SHA256 | After SHA256 |
| --- | --- | --- |
| `offline_entry_safety_candidate.py` | `4d124d8ee944636999b238e0dfb5b100e1e3e790f20200ebaf40eddb1096c936` | `74c161d37903ff88a7b8969fd7a48f579ca3f88897f890117601a21520144d13` |
| `offline_entry_safety_worker.py` | `bcfee1d0a4b549158ba928c721bc97100c099f958a48365d44e37c42844f3836` | `28124e977c3374f0ca2cd2f61406e66f5fec07828a1fec38425d8ceb6e0dbf70` |
| `test_offline_entry_safety_candidate.py` | `be848ede4e542317a09b5e538362f53d42d37e64386bf88f1c72166e07f848c3` | `d2005536f0d63eeac9b22922c66e88b91003d2558af9ea676553de1c0b274f85` |

`candidate.run:70-87` requires explicit effects/approval, validates fixed parsed
arguments, actual launcher path/hash, source/dependency identity and selected worker
digest **before TestOnly returns**. TestOnly does not import/call the selected worker.
`worker.main:15-29` independently validates approval/parsed semantics, executing
worker path and actual self digest before operations or TestOnly return.

**TestOnly model:** a source/argument-validated **synthetic preflight no-op**, not the
previous unconditional inert no-op. It is NOT deployment, actual controller, native
capture or live-use qualification. Zero exit alone still establishes none of those.
Approvals are fixed trusted synthetic fixture data; no stock authority is minted.
No hostile-Python, untrusted-adapter, arbitrary-code or time-of-check/load security
framework is asserted. Parsed semantics, not raw argv quoting/order, remain the
argument contract. Ordinary fixture pins are literal constants, not hashes derived
from whatever code happened to run.

## Exact independent verification

Facilitator read all delivered code before execution. Full candidate suite:

```
PYTHONDONTWRITEBYTECODE=1 python -m unittest tools.harness.test_offline_entry_safety_candidate -v
```

**11/11 pass**, own `facilitator-suite.log`; worker's separate log retained. Existing
helper tests do not certify single-use/crash/evidence behavior. AST comparison finds
**all `OfflineEffects` methods unchanged** from B2p: no single-use/evidence subsystem
repair. No game build/CTest/app run or new pixel score/original-body read performed.

Separate facilitator checker, no worker-test helpers:

```
PYTHONDONTWRITEBYTECODE=1 python build/tmp/offline-testonly-worker-boundary-2026-10-27/independent-check.py
```

It uses literal known before/after source pins and independently fixed approved
semantics, actual launcher/worker entry points, fresh synthetic adapters/state, plus
an audit hook forbidding subprocess/system/startfile/DLL/network effects and installed
target access. Results `independent-result.log`, `independent-run-1/result.json`.
Repeat with a fresh safe `--run-name` to avoid overwriting the retained run.

**34 actual-boundary cases**:

| Cases | Independent observation |
| --- | --- |
| Matching approved launcher→worker and matching direct worker | Both return0 with consume/attempt/mock-capture/receipt; not refuse-everything. |
| Launcher TestOnly; worker argv TestOnly; worker keyword TestOnly | All return0 after non-operational identity audit, with no authorization marker, attempt creation, capture or receipt. |
| Unknown flags in launcher and worker, including TestOnly requests | Parser `SystemExit`, no operational effects. |
| TestOnly without approval at either boundary; direct worker without approval/adapter | Refuse before operational effects. |
| Direct worker missing each of approved_args, launcher_path, launcher, worker_path, worker, dependencies | All six refuse before consume/create/capture. Some are pre-effect key/source errors, not a claim of polished error reporting. |
| Direct worker changed attempt or mode, stale digest, changed executing worker, or disguised reference to unchanged approved worker file | Refuse before effects; checking an unrelated approved file cannot substitute for the executing worker. |
| Synthetic selected stale worker ignoring TestOnly | Launcher refuses source identity before loading/calling it; no operations. This never executes B2m. |
| B2p functional regression set | Matching positive plus all7 source/args mismatch cases retained: attempt, mode, worker argument, worker bytes, launcher bytes, disguised launcher path, missing argument approval. |
| Exact before-B2p direct-worker supported-argument bypass | Reproduces return0 with operations on unapproved attempt, in fresh state. After Q refuses the equivalent mismatch. |
| Exact before-B2p stale TestOnly request | Reproduces unconditional return0/no operations with no source qualification; Q rejects the stale source and validates successful synthetic TestOnly approval. |
| Two executed guard-only mutants | Both break their corresponding ordinary negative property with actual synthetic operations; details below. |

There are25 pre-effect refusal cases,3 valid TestOnly no-ops,2 valid operational
positives,2 preserved-before contrasts and2 guard-only mutant cases. All operations
are synthetic. B2p's ordinary operational source/argument **properties** are preserved;
its earlier counterfactual files/results remain immutable historical evidence.
Added independent worker validation can make a removed launcher guard redundant;
no claim each old launcher-only guard remains individually necessary on the new route.

## Two genuine isolated negative controls

Facilitator independently generated LF disposable worker copies. Only one conditional
changes in each; deliberately changed source is explicitly pinned in the synthetic
approval before execution. The other conditions remain valid; no unrelated sourcehash
or argument error masks either experiment.

| Actual executed copy | Ordinary negative-property result |
| --- | --- |
| TestOnly guard `if ns.test_only or test_only:` → `if False:`; fixed approved TestOnly=True, actual True | `inert` property raises AssertionError: return0 reaches consume/attempt/mock-capture/receipt. Source and argument checks remain enabled and valid. |
| Worker approval comparison `if approved != actual:` → `if False:`; valid source/path/dependency fields, approved attempt `approved`, actual `UNAPPROVED` | `refused` property raises AssertionError: return0 reaches consume/attempt/mock-capture/receipt. TestOnly=False and source checks remain enabled. |

Independent LF hashes:

```
TestOnly-only       8629dbe7c66c1c90c730fbcc57479d54405feb2d124073095c7c6ea83cdf94d8
worker-approval-only 6699e8b6287ddfb158da6b102d24761a78c99d5f3a7fd9e7133d5c125d1f4f1b
```

Copies, states and receipts are retained in `independent-run-1/`. These are real
executed boundaries, not generated-but-unused files or unrelated wrong-hash tests.
The worker also executed its own two controls; its generated Windows newline
artifacts (`6556883f…` / `2b91ca95…`) are preserved, not rewritten as independent
evidence. Delivered source/tests and facilitator-generated copies use LF.

All facilitator mutations were disposable copies; delivered sources were never
temporarily edited. Before snapshots and delivered after hashes remain exact after
verification; no source rollback/restoration is owed. Code stays uncommitted for
user inspection, not production wiring.

## Integrity, budget/cap accounting, release and stop

`verify-integrity.py`, brief lint and `git diff --check` preserve exact stopped B2p
run/all older archive entries,16 gate texts,16 untouched baseline paths,3 before
sources,119 prior offline artifacts and236 earlier saved-evidence files. Independent
old file-set comparison also finds exactly119 files/no extra prior-state files now.
No reset/stash/checkout, new original binary research or old-packet rewrite occurred.

This is implementation round/attempt **3 of the same safety effort**, retaining
B2o broad failure1 and B2p scoped pass1. B2n decision review1round/2attempts remains
separate and visible: conservative total **4 offline rounds/5 worker attempts**
including review/current. No renaming resets caps. The explicit user-approved cap
rationale was the exact orthogonal gap B2p identified, with fixed before source and
two known falsifiers serving future B2/B3 gate measurement—not new instrumentation.
No immediate native gate movement is claimed. No automatic fourth implementation
round/fifth total round or new component follows. Prior implementation elapsed time
including closure2643.523s and all old unused pools remain historical, not credit.

Fresh activation message `msg_17f660b4d002GnR4RYTXGVMxwr`,22:05:56.444Z; deadline
22:50:56.444Z, worker return tightened to22:35:56.444Z. Exact API dispatch/terminal
and final full closure clocks are in queue/packet `final-clock.json`; the worker's
handwritten packet time22:18:40.220Z is NOT used as terminal proof. One attempt,
no repair dispatch, no interruption or overrun. No stock starts, attaches, game/
foreground/input/desktop actions, screenshots, sampling, install/save/config writes,
scheduled tasks, debugger/Frida/hooks/live adapters or stock-authority tokens.

Worker `ses_e80945d23ffeZFV0gzd2biKTPz` independently succeeded/idle; active endpoint
only parent/self. No owned live resource or live restoration owed. Historical host
cleanup/install hashes are not fresh checks. HEAD `e649d10`; all changes uncommitted,
no push. Scope exception is not erased by technical verification.

**Retained, not fresh measurements:** C3 `.06354818984885621` FAIL versus `.02`;
C1 zoom1 `.0160863988750699` / elev_down `.016121214378294987` PASS. All16 gates/
masks/references/exclusions unchanged. B2/B3 unaccepted, C2 native gate unpassed,
B1 conditional. D1n landed/D1o failing; Phase D/D3/digger-clock parks unchanged.

**Stop/park for user review.** Recommend reviewing the disclosed path-scope deviation
and this three-implementation-round effort before any further authorization. Whole
launcher remains unaccepted: single-use/crash durability, evidence retention and
deployment/live compatibility are still unqualified. No follow-on or live trial.
