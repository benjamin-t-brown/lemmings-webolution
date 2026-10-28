# B2p — source/parsed-argument binding, operational route only

2026-10-27, separately approved `offline-source-argument-binding-2026-10-27`.
Directive SHA256 `be2ae51e77c15c01024813abfaa982e4032fbc42c374df29e0d059f4bdc946d5`.
One worker attempt,45min total,zero game/desktop actions. Owner
`ses_e80f12432ffebdYi3RdAhbYN8v`, parent `ses_e80f36524ffe7KzSDCj9yVcKCp`.
Packet prefix: `build/tmp/offline-source-argument-binding-2026-10-27/`.

**Result: component verified for the non-TestOnly operational launcher→worker path.**
The whole prototype remains unaccepted/unwired. This is not TestOnly, direct-worker,
single-use/crash, non-overwrite/failure-receipt, deployment/dependency, native capture,
physical-input or fidelity certification. No live readiness or follow-on authority.

## Change and independently fixed contract

The same three candidate/test files from [rejected B2o](phase-b2o-offline-launch-safety-rejected.md)
were allowlisted. Exact before bytes were copied **before dispatch** to packet
`before/`, independently checked against these fixed SHA256 values:

| File basename | Before SHA256 | Delivered after SHA256 |
| --- | --- | --- |
| `offline_entry_safety_candidate.py` | `6defc77393b1db915240ea309fc83b342dfedb78f01d49f4914775242e6e5549` | `4d124d8ee944636999b238e0dfb5b100e1e3e790f20200ebaf40eddb1096c936` |
| `offline_entry_safety_worker.py` | `e51e8d006e7abe532fed37d9534ca93817caf7b580f8326c88690459db5c9a31` | `bcfee1d0a4b549158ba928c721bc97100c099f958a48365d44e37c42844f3836` |
| `test_offline_entry_safety_candidate.py` | `1671d78d40ba18efbbf7488d79738d07550e542fb1a70dccc4c2e4389ecf1b40` | `be848ede4e542317a09b5e538362f53d42d37e64386bf88f1c72166e07f848c3` |

Paths remain under `tools/harness/`. Before/after manifests are retained separately.
No original failure packet was changed or old B2m launcher/helper executed/imported.

`candidate.run:74-91` now compares parsed semantics against externally supplied
`approved_args`: attempt, TestOnly boolean, mode and resolved worker path. It checks
the executing launcher's `__file__` against approved launcher path, approved source
digests through `audit_identity`, then the **selected** worker digest before loading
it. Matching semantics are passed to the actual loaded worker. Approval is trusted
synthetic fixture data, not a user signature, stock-start token or deployment system.

Permanent test fixture contains literal after-source digests (`test…py:16-18`),
not `digest(current source)` as its ordinary expected value. Independent facilitator
fixtures use the literal before/after pins above and fixed approved semantics,
separate from actual invocation. Fixed semantics deliberately approve
`approved-attempt/ok` while actual negative input requests `UNAPPROVED-ARGUMENT` or
other changed supported values. Neither fixture copies approval from actual argv.

**Coverage boundary:** comparison is parsed semantics, not exact argv spelling,
ordering or quoting. TestOnly still returns early (`candidate.py:70-71`), without
performing this approval validation or operational effects; it is **not** a qualified
preflight/approval result. Direct worker calls do not execute the launcher argument
check. These paths remain unqualified, explicitly outside the successful operational
launcher→worker component. Hostile in-memory/process mutation, file races between
checks/load, untrusted approval authors/adapters, dependency closure and live
deployment are not certified. No claim all argv paths now validate approval.

## Full candidate suite and independent differential checks

After reading all returned source for effect safety, facilitator reran:

```
PYTHONDONTWRITEBYTECODE=1 python -m unittest tools.harness.test_offline_entry_safety_candidate -v
```

**8/8 pass**, own `facilitator-suite.log`. The worker's separate log/note remain in
`worker/`. No game build/CTest, interactive app, new binary/body read or pixel score
was needed/performed for this unwired Python-only change. Passing legacy helper
tests does not close any other safeguard.

The facilitator wrote a separate checker, not importing worker test helpers:

```
PYTHONDONTWRITEBYTECODE=1 python build/tmp/offline-source-argument-binding-2026-10-27/independent-check.py
```

It runs actual `run`→loaded `worker.main`, with only synthetic `OfflineEffects`
and fresh scratch state. An additional audit hook refuses subprocess/system/
startfile/DLL-loading/network effects and installed-target file access. Results:
`independent-result.log` and `independent-run-1/result.json`. To repeat without
overwriting evidence, pass a fresh `--run-name` (letters/digits/underscore/hyphen;
all output remains under this packet).

| Independent case | Observed |
| --- | --- |
| Matching approved after source/semantics | return0, actual worker success; `consumed`, `attempt-created`, `mock-capture`, receipt. Not a refuse-everything implementation. |
| Supported attempt drift, B2o falsifier semantics | `argument-identity-mismatch`, zero operational events. |
| Supported mode drift | Same refusal, zero operational events. |
| Supported selected-worker-path drift with otherwise valid worker bytes | Same refusal, zero operational events. |
| Selected worker content changed, path validly approved | `source-identity-mismatch`, zero operational events; one non-operational identity audit event is not token consumption. |
| Changed executing launcher source, its path correctly declared | `source-identity-mismatch`, zero operational events. |
| Changed executing launcher while approval points at unchanged original source path | Same refusal, zero operational events; hashing a different approved file cannot hide the executing copy. |
| Missing argument approval | `argument-identity-mismatch`, zero operational events. |
| Preserved exact B2o before source with old approved-vs-actual argument drift | return0 and actual mock capture/receipt success, reproducing the old regression independently in fresh state. |

Thus seven independent after mismatch cases refuse before consumption, attempt
creation or synthetic capture. The old B2o artifact/state is untouched. The after
case uses valid after-source pins plus equivalent fixed approved semantic values,
not merely stale before hashes or malformed approval to manufacture rejection.

## Two executed isolated mutants — tests genuinely fail

Facilitator independently generated disposable copies from the pinned delivered
launcher. Each differs in **one guard line only**; both retain the other binding
and unrelated safeguards. Their deliberately changed launcher bytes are explicitly
approved by fixed mutant pins before execution, so generic selfhash rejection does
not mask the intended test. The selected worker mismatch/argument mismatch remains.

| Executed independent mutant | Property/test result |
| --- | --- |
| Only selected-worker source comparison `if sha(args.worker) != expected['worker']:` changed to `if False:` | The ordinary source-mismatch refusal property now **raises AssertionError**: return0 with consume/attempt/mock-capture/receipt. The loaded worker is the actual delivered worker plus a harmless source-comment change, not a substitute fake main. |
| Only `if actual != expected.get('approved_args'):` changed to `if False:` | The ordinary argument-mismatch refusal property now **raises AssertionError**: supported attempt drift returns0 with consume/attempt/mock-capture/receipt. Source identity and selected-worker check remain intact. |

Normal negatives pass the same `require_refusal` property (exact mismatch error and
zero operational events); each guard-only mutant fails it behaviorally. This is not
an unexecuted mutant file or unrelated wrong-hash assertion. Receipt/state artifacts
and executed mutant files are retained under `independent-run-1/`.

Independent LF mutant SHA256 values:

```
source-only   468eb053022c4068ea76eb27a72dba55b0dd206590e2e915713f16f01a60314d
argument-only 004c521b62d41c2f2378de41bfd559114e1e73e5fdd1717b1468f9c9652b8830
```

The worker also executed its own two mutants; its retained generated copies use
Windows CRLF and consequently have different hashes (`135f51ec…` / `fdf66dd4…`).
Those artifacts are preserved, not rewritten as independent evidence. The
facilitator's new copies use LF; actual delivered candidate/worker/test source is
LF. Worker test mutant digest fixtures are therefore host/newline-specific; no
cross-platform test-portability acceptance is claimed.

## Facilitator check, restoration and limits

- Independently re-read every new boundary/fixture and compared the before source.
  AST comparison confirms `OfflineEffects.__init__`, `consume`, `create_attempt`,
  `geometry`, `capture`, `append_receipt` unchanged. No other safeguard implemented.
  Worker metadata field naming changed to match the launcher approval contract.
- Independently ran the full eight-test suite, seven mismatch cases, positive path,
  preserved-before regression and both actual guard-only mutants. Fixed pins and
  exact executed paths/results are in the checker/JSON, not inferred from a model's
  report or from source grep alone.
- All mutations used disposable copies; delivered source was never temporarily
  changed by facilitator. Before three snapshots remain exact, delivered after
  hashes remain exact after tests/mutants. No restoration to old B2o working source
  is appropriate: the scoped implementation remains uncommitted for user review.
- `verify-integrity.py` independently preserves the exact stopped B2o run/all older
  archive entries,16 gate texts,14 untouched baseline paths,63 prior offline evidence
  files and236 earlier saved evidence files. Old counts, breach and frozen unused
  pools unchanged. Brief lint/diff checks pass. No unrelated dirty hunk discarded.

Mandatory scope exclusions remain: no TestOnly/single-use/crash/evidence/failure-exit/
dependency deployment certification, original research, native observation, physical
mouse/SDL binding, app entry, observer, C3 or Phase D work. No external effects,
stock authority, install/save/config writes, scheduled tasks, debugger/Frida or live
trial. The source/argument fixture remains a trusted offline contract, not an
authorization framework for an untrusted caller or a production controller.

## Clock, release, unchanged gates and stop

Fresh activation: authorization message `msg_17f3b5568002u6xRZVZIs7OaAp`,
21:19:17.366Z (not session creation). Overall deadline22:04:17.366Z; tightened worker
return21:49:17.366Z reserves15min. Dispatch request21:24:58.811Z; created
21:25:16.614Z; succeeded/idle21:31:12.918Z: **374.107s request→idle**,356.304s
created→idle. One attempt/one round, zero game starts/attaches/foreground/input/
desktop exercises; no overrun. Queue and final-clock.json include full final
verification/documentation/cleanup time. No worker interrupted or repair dispatched.

Terminal worker `ses_e80bf2f7bffeHLtCDisBG582r7` independently confirmed; active
endpoint only parent overlord/self facilitator. No owned live resources or live
restoration owed. Historical host cleanup/install hashes are not fresh measurements.
All changes uncommitted, HEAD remains `e649d10`, no push.

**Retained, not fresh:** C3 `.06354818984885621` FAIL versus `.02`; C1 zoom1
`.0160863988750699` / elev_down `.016121214378294987` PASS. All16 gates/masks/
references/exclusions unchanged. B2/B3 unaccepted; C2 native gate unpassed; B1
conditional. D1n landed/D1o failing, Phase D/D3/digger-clock parks unchanged.

**Stop at this component result.** Recommend user review of the bounded source/
argument repair and an explicit decision on remaining safety components, not a
live trial or automatic next pass. Whole launcher remains unaccepted; this result
cannot spend any old or unused stock/time pool.
