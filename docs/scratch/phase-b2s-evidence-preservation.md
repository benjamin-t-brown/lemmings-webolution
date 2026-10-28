# B2s — offline evidence-preservation component verified

2026-10-28. **Verified only for the declared synthetic actual-entry cases.** Whole
launcher remains unwired/unaccepted; no deployment, route connection or live use.
User approved45min/ONEworkerattempt/zero game-desktop actions and an explicit cap
exception, implementationround5 SAME safety effort (B2n review included:6rounds/
7attempts). Original overwritten-evidence defect has a fixed behavioral falsifier;
this serves futureB2/B3 measurement, not new logging/capture architecture or a reset.

Directive SHA256 `1558246287b80550fa4523dc755491db482521c8c04a20c0865a39f7cf099a8b`.
Fresh activation02:13:36.909Z, overall deadline02:58:36.909Z, worker deadline
02:43:36.909Z reserves15min. Interim foreground responses did NOT finish/reset this
pass or release its worker. One dispatch only, no replacement/interruption/repair.

## Source and bounded implementation

Only the3existing candidate Python files changed. Exact B2r source/test snapshots
were preserved before dispatch under current `before/`. Current pass root was
established BEFORE saving any API/preparation dump. All new tests/temp/state/mutants/
children/prep belong under `build/tmp/offline-evidence-preservation-2026-10-28/`;
inherited test roots were retargeted before tests. Earlier scratch is read-only.

- Candidate retains atomic exclusive grant claim/fsync and exclusive attempt-directory
  creation. It adds fsynced append-only stage records within the owned attempt,
  exclusive synthetic-output records and separately named exclusive rejection files.
- Worker records started/incomplete before mock launch, launched/capture-started,
  then output-persisted/incomplete-until-final-record or a precise error. Final
  receipt remains separate. Occupied attempt destination emits a new rejection after
  spending the new grant; it never modifies the earlier attempt in protected cases.
- Attribution is exact launcher/worker source hashes, approved parsed arguments
  (including selected-worker path), attempt/mode and SHA256 token association.
  Source paths/pins and grant-store association are also explicit in the independent
  invocation fixtures. Not a signed grant service or self-contained deployment manifest.
- `stages.jsonl` intentionally appends within ONE owned invocation; cross-invocation
  immutability depends on exclusive destination ownership. No general journal or
  hostile-adapter/security architecture is claimed.

Delivered SHA256:

| Basename | SHA256 |
| --- | --- |
| `offline_entry_safety_candidate.py` | `7a866b7bfdc5c978b5a3a78b5fa2dc3aef8f9cd0c73e8e25e794cb3bc12c11b1` |
| `offline_entry_safety_worker.py` | `fbb40446cb9e7494707438897231e0431cac304361e686119e32e20362084aee` |
| `test_offline_entry_safety_candidate.py` | `ef6bf98a447b2d342b671b2724ab8bdc1ed111bfc474b52101b07ae42d615de8` |

Before/after snapshots and hash manifests are under the current pass root. Delivered
source was never temporarily mutated; every mutant used a disposable copy. No source
restoration/rollback is owed; all changes remain uncommitted for inspection.

## Facilitator check — independent actual boundary

Read complete3sources/diff and all inherited temporary/artifact paths before testing.
Own full suite **17/17 PASS**, including P/Q/R and their existing isolated controls.
[Worker packet](../../build/tmp/offline-evidence-preservation-2026-10-28/worker/round-note.md)
retains its initial failures/final suite and per-run raw evidence. Own suite refreshes
current-root fixed mutant summaries; uniquely named raw worker evidence folders remain.
No prior-pass packet or source snapshot is changed.

[Own checker](../../build/tmp/offline-evidence-preservation-2026-10-28/independent-check.py)
imports no worker test helpers. It uses literal reviewed source pins, matching fixed
supported-argument approvals and explicit synthetic grants. Actual candidate `run`
loads actual worker `main` in fresh bounded Python child interpreters. Complete run:
[results](../../build/tmp/offline-evidence-preservation-2026-10-28/independent-verified/results.json).

| Independent first invocation | Retained outcome |
| --- | --- |
| Synthetic success | return0; started/launched/capture/output stages, exact synthetic-output value and complete receipt |
| Ordinary capture error | return1; `capture-failure` / `capture` / `OSError:facilitator-capture-error`, failed stage/receipt, no output |
| Geometry refusal | return1; `geometry-refusal` / `geometry-check` / `ValueError:wrong-geometry:401x262` |
| Injected exception | return1; `exception` / `injected-exception` / `LookupError:injected-capture-exception` |
| Abrupt exit before mock launch | exit81 after durable started stage; incomplete-unknown, no output/final receipt |
| Abrupt exit during capture | exit82 after capture-started; incomplete-unknown, no output/final receipt |
| Abrupt exit after output write, before output stage | exit83; synthetic output retained, last capture stage incomplete, no final receipt |
| Abrupt exit after output stage, before final receipt | exit84; result/output retained, incomplete-until-final-record, no final receipt |

Every stage/receipt/rejection is independently checked against expected source hashes,
approved arguments, attempt/mode and grant association. Synthetic output is NOT a
screenshot. A success branch on output-persisted denotes produced capture output;
its explicit incomplete outcome and absent final receipt are never treated as normal
completion. No recovery process fabricates final receipts.

For EACH of8cases, two later actual invocations use distinct valid tokens/stores and
the SAME evidence destination/attempt with valid argument approvals: **16 rejection
cases** preserve exact earlier file sets/bytes, including interrupted output/stages.
Each emits a separately attributable rejection with precise destination-collision
error; third invocation also preserves the preceding rejection bytes. These refusals
are NOT stale-source/argument/spent-token substitutes. Subsequent fresh-interpreter
retry using each ORIGINAL grant but renamed approved attempt/output refuses at the
spent claim: **8 restart/no-refund regressions**. An additional8process race yields
one mock launch and seven spent-claim refusals. Mock-launch observers check the exact
claim already persisted. Eight separate P/Q source/args/dependency/TestOnly controls
also pass; full suite retains direct-worker missing-approval/unknown-flag controls.

Complete independent run: **42 child invocations**, plus8in-process P/Q controls.
The earlier verifier run completed32protected child invocations before its own
mutant-fixture error; retained separately, not counted as42fresh unique cases.
Total bounded children across both runs: **74**, all terminal/reaped. No second
worker attempt or candidate repair occurred.

## Genuine non-overwrite-only mutant

Exact sole edit: attempt destination `p.mkdir()` → `p.mkdir(exist_ok=True)` in a
disposable launcher. Exact executed SHA256
`893110077ebd9fb7a723e7a2e85cad394626bb9ccc570f72a3898f764651631c` explicitly approved.
Actual worker remains unchanged; source/argument/TestOnly/single-use guards stay active.
Two distinct valid grants/stores target the same valid attempt/output destination.

First returns0; second reaches mock launch and appends new-attempt stages to the
earlier `stages.jsonl`. Final exclusive output/receipt guards still refuse, but
**after earlier stage bytes have already changed**. The ordinary exact file-map/
byte equality assertion genuinely fails. This is appended contamination of an old
record, not a claimed receipt truncation: original receipt/output bytes stay exact.
Thus unrelated final collision does not mask this mutation's behavioral failure.

Independent stage SHA256 before/after:
`a1ee447da33dd542941888f2646a0328cb44d9c18683ceb1db674a55ddf33c5f` →
`afab44f6dcb02afba4aaa93fc0a2b73a08e71f178d02da261aab2ee9fb598b7e`.
Source, earlier raw stage bytes, all records and comparisons remain under
`independent-verified/mutant/`. Delivered source pins still match after mutation tests.

### Verifier failure retained, not candidate repair

Initial facilitator checker read source using Windows cp1252 then explicitly wrote
UTF-8, unintentionally reencoding an arrow comment. The fixed exact mutant pin
detected this BEFORE the malformed mutant executed. A subsequent diagnostic print
also hit console cp1252 UnicodeEncodeError. Corrected only the checker to read UTF-8,
retained the partial run/error/diff, and reran completely in a fresh directory. Final
executed mutant has exactly the one guard edit. See
[verifier error](../../build/tmp/offline-evidence-preservation-2026-10-28/independent-first-run-verifier-error.json).
No error was hidden, permanent candidate repaired by facilitator, or attempt reset.

## Integrity, limits and stop

Own preservation checker confirms exact stoppedB2r run/all older archive entries,
16gate texts,20untouched baseline paths,3before sources,320prior evidence paths/hashes
(including the old EXTERNAL B2r dump) and236earlier artifacts. User approved retaining
B2r technical proof WITH facilitator violation, not erasure/clean relabel; external
dump remains preserved with no deletion authority. B2q's disclosed violation also
remains historical. Current new preparation files are all inside the current root.

Commands: `PYTHONDONTWRITEBYTECODE=1`, TMP/TEMP/TMPDIR inside current pass:

```
python -B -m unittest tools.harness.test_offline_entry_safety_candidate -v
python -B build/tmp/offline-evidence-preservation-2026-10-28/independent-check.py
python -B build/tmp/offline-evidence-preservation-2026-10-28/verify-integrity.py
python -B scripts/check_agent_framework.py --brief build/tmp/offline-evidence-preservation-2026-10-28/brief.md
git diff --check
```

Checker refuses an existing run destination; use a new deliberately named verifier
output directory for another replay, never delete/overwrite its saved evidence.
Only unwired Python changed, so no native build/CTest/app/pixel measurement performed.

Limits: trusted stable local per-grant store/paths, no deletion/movement/hostile
adapter/caller/source-load race qualification. File flush/fsync and these bounded
offline abrupt exits do NOT prove universal power-loss/storage-controller/cross-host
durability or native game lifecycle. Very-early pre-stage interruption, arbitrary I/O
failure, source-refusal audit trails and deployment are not newly certified. Passing
the declared component does not qualify whole launcher, original input or fidelity.

Worker default pin authenticated; created02:19:30.940Z, succeeded/idle02:25:59.861Z,
NOT handwritten02:25:03.751Z. Dispatch request02:19:13.201Z→idle406.660s; independent
complete run finished02:30:23Z. No owned game/window/task/install/debugger resources
or restoration, no fresh native host/install resampling claimed. Zero game/desktop
actions, native screenshots/sampling/original reads; no B2m execution/commit/push.
Exact final total/round closure clocks are in current queue and pass `final-clock.json`.

Retained—not fresh—C3 `.06354818984885621` FAIL; C1 `.0160863988750699` /
`.016121214378294987` PASS.16gates/masks/references/exclusions unchanged; B2/B3unaccepted,
C2nativeunpassed; B3k gap/PhaseD parks unchanged. **STOP at verified component.**
Recommend reviewing this scoped proof, then deciding separately whether to authorize
a bounded route-connection/deployment assessment. No automatic assessment/component/
exception or live trial follows; whole launcher remains unaccepted.
