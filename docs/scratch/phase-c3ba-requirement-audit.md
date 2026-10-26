# C3ba — requirement ordering audit, with installation-hazard correction

2026-10-26; initial audit plus one repair. Sources and candidate matrices:
`build/tmp/c3ba-requirement-audit/`, especially `repair1/addendum.md`.
This is an offline architecture/safety review, not observer implementation.

## Accepted audit

AW's worker-authored site design operationalizes the capture proposal's span,
relocation and restoration requirements. The integration directive incorporates
that design; the current Astra directive explicitly preserves the requirement
while authorizing review, not a bypass. No cited user instruction prescribes a
particular Frida getter or proof-object schema.

Three different claims must stay separate:

1. **Before installation:** a reviewed prospective safety case tied to the
   actual tooling/build, exact site bytes and method; original-memory baseline
   and immediate preinstall recheck; treatment of concurrent execution and
   context preservation. Static instruction boundaries alone do not establish it.
2. **After installation, before collection:** any proposed observation of actual
   redirects/displaced instructions/relocation is a runtime outcome, not a
   supplied boolean. It cannot retroactively make the first installation safe.
3. **After detach:** independent complete-span/guard comparison to preinstall
   bytes, plus script/session/host cleanup. This is restoration evidence at that
   time, not proof of safe execution or non-interference in between.

AW says actual spans/readback are needed before arming (`site-spec.md:20-26`),
but its step3 says validate actual envelopes/support *then* install unarmed
(`207-212`). It does not specify a producer resolving that sequencing ambiguity.
AZ is stricter in its actual ordering: `validEngineProof` is required before
`Lifecycle` construction and before attach (`observer.js:97-119,219-227`). Its
fixed ranges and `readbackHex == originalHex` are supplied preinstall assertions;
`start()` and `arm()` do not independently inspect installed engine spans.
The constant-false runtime gate correctly prevents this from becoming native proof.

**Correction to the initial worker audit:** unarmed is not uninstrumented.
`safeCallback` returning before collection does not prevent engine redirects or
trampolines from executing. Rollback cannot undo an already misexecuted instruction.
No intrinsic cycle exists merely because restoration is checked after detach;
that narrow statement does not establish a safe install-then-inspect protocol.
Changing required proof ordering/evidence needs explicit approval, not an implicit
implementation choice. Keep refusal intact pending the separate feasibility result.

## Facilitator check

Independently read the full actual AZ source, AW sections1/6, capture proposal,
integration/current directives, canonical safety rules and AY/AZ accepted notes.
Authenticated12 source-hash citations and both preserved initial candidate hashes.
Returned the missing pre-constructor proof/installation-hazard distinctions as a
bounded repair; checked the addendum against those exact branches. Its reference
to AW "step2" validation is narrowed above: that validation is in step3 before
installation, not step2. No new native behavior is accepted.

Own independent PE parser re-reads the65-byte guard `0x46D5C4..0x46D605` and
disassembles21 instructions. PRE is a6-byte `mov edx,[0x58CDB8]` followed by
one-byte `push edx`; POST is a5-byte `mov ecx,0x58CB40`. AW's proposed7-byte PRE
envelope is not evidence that an engine must displace7 bytes (6 already covers
a5-byte redirect). This review does not choose an actual engine span.
`build/tmp/c3-readiness-review/check_site_bytes.py` and `site-read.txt` retain it.

Fresh independent Node VM execution of the unchanged RPC yields not-ready and
three setup refusals (null/empty/forged proof), with zero access to trap-backed
native API objects. This is a refusal test, not Frida execution or safe-hook proof.
Source SHA256 stays `f2d9da8017672a2efe2079b48e3877c45b00a100e48cb131375c1e4241fa0e47`.
Checker/result: `check_refusal.js`, `refusal-results.json` in the review folder.
No implementation mutation is applicable to this document-only round.

Fresh authoritative wrapper build and44/44CTest pass;25/25framework tests pass.
An initial foreground test command timed out; the complete rerun is retained.
Seven install hashes match, no Lemmings process or LemWeb task observed; no
start/attach/injection/host task/install write occurred. Prior dirty changes and
stopped-run archive were checked preserved. C3 **0.0635481771 FAIL**, C1
**0.0160864/0.0161212 PASS** are retained baselines, not new renders. Gates unchanged.

Dispatch01:55:14; repair01:59:30; final worker return02:00:01. Landing timing is
recorded in the queue archive. Concrete tooling feasibility remains the sibling
C3bb question; no implementation or native follow-on is authorized by this audit.
