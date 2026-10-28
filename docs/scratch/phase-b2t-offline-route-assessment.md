# B2t — smallest protected capture/navigation connection (assessment only)

2026-10-28. Directive `offline-route-assessment-2026-10-28`, SHA256
`7b36e64946789af5b1b5146f0f8f90d434645efd0bef1681a265712a11bc7555`.
Owner `ses_e7f967be2ffeZlKGBhAXeJprTk`; parent `ses_e7f9969f2ffeaoCoSSUJMuvDZG`.
One source-only assessment round/worker attempt, **no implementation or live authority**.
Packet: `build/tmp/offline-route-assessment-2026-10-28/`;
[worker assessment](../../build/tmp/offline-route-assessment-2026-10-28/worker/assessment.md).

## Decision: one protected owner adapter, not two old launchers

**Proposed, not implemented/qualified:** connect the existing approved
`candidate.run` → actual `worker.main` effect boundary to **one explicitly owned
process/input/capture adapter**. Reuse the guarded B2h `run_plan(read, io, …)` and
B1b's menu client-crop/state-bracket operation, not their whole launch scripts.
The smallest useful result is one qualified menu image **before** navigation,
then the recorded menu→confirm→map→`MM_CLIMB_002` path/state observations from the
same owned process. It serves future B2/B3 measurement; it cannot satisfy their
full back-navigation/two-unlock-state/application-entry gates.

For a later decision, recommend **one owner-created stock child**, with in-process
route operations where possible, over an attach-any-existing-PID design. Existing
process attachment would require a separate explicit ownership handoff contract.
No new capture engine, physical-input architecture or authorization service is
required by this assessment. A reviewed local approval record may be sufficient
under the existing trusted-local assumptions; its issuer, scope and durable store
must be explicitly chosen, not silently inferred from caller-supplied fixtures.

### Exact source-backed path and unqualified edges

All Python files below were read **as text**, never imported/executed.

| Step | Source and disposition |
| --- | --- |
| Fixed reviewed source/parsed arguments | `tools/harness/offline_entry_safety_candidate.py:102-134` compares approved semantics, executing path and worker digest before load; `:137-139` supplies no effects/approval and therefore cannot run operationally by default. B2p's proof remains synthetic, not deployment authority. |
| Validated no-op/direct worker | Candidate `:123-126`; `tools/harness/offline_entry_safety_worker.py:16-30` validate before TestOnly return. B2q proof/violation retained. |
| Claim, exclusive evidence, effect boundary | Candidate `:36-79` uses exclusive/fsynced local claim, exclusive attempt and output/rejection records; worker `:31-66` consumes before launch, records stages and synthetic completion. B2r/B2s qualify only their declared local synthetic cases. |
| Missing owner adapter | Candidate `:81-90` supplies only mock launch and synthetic result. Worker `:41-66` has **no real child cleanup/await interface**, and hardcodes `owned-mock-complete/failed`; those cannot become OS termination evidence by relabeling. Its JSON-only `write_output` (`candidate:76-78`) does not retain image bytes. A real image/artifact reference + identity and terminal outcome contract is missing. |
| Menu capture seam, before navigation | Saved `build/tmp/b1b-reference-screengrab/capture.py:151-205` releases input, brackets screen/highlight, obtains actual client rect and desktop crop, and checks 3/0 and640x480. Reuse this operation only. **Do not call its `main`**: `:122-140` swaps `dgVoodoo.conf` and starts its own stock child; `:195-199,272-275` writes reusable filenames. Its `:249-268` cleanup is useful historical code, not protected integrated cleanup acceptance. |
| Navigation seam | `tools/harness/capture_b2_sequence.py:146-240` already accepts `read/io/on_step`; `:112-143` preserves guarded menu RETURN and selected sequence. Existing concrete IO at `:295-330` acquires foreground, drives highlight and writes shim button0. A later adapter can reuse/extract these bounded operations without inventing a new engine. |
| Avoid hidden reattachment/second owner | `capture_b2_sequence.live:251-268` imports platform dependencies and calls `attach()` itself; it is not a protected launcher. `tools/harness/lrstate.py:75-84` selects **the first** matching process, not a uniqueness/owned-PID proof. Future operations must bind the handle/PID to the owner's created child; a `--live-owner` boolean is not that evidence. |
| Existing successful B2h owner, not current authority | Saved `build/tmp/b2h-live-esc-sweep/run_b2h.py.copy.txt:32-42,86-118` starts one stock child, checks ready PID, invokes a separate sequence process with420s timeout, then terminates/waits its stock child. `launch_task.ps1.copy.txt:1-17` runs pythonw through an interactive scheduled task. These establish concrete historical deployment/lifecycle seams, **not** a presently frozen/protected launch chain or permission to recreate tasks. |

## Remaining gaps and fixed later acceptance/refusal criteria

These criteria are for **separately approved implementation/verification**, not
checks executed during this pass. Current whole launcher remains unwired/unaccepted.

| Gap / source | Fixed acceptance | Failure / stop |
| --- | --- | --- |
| Dependency/deployment identity: candidate `:29-34` checks only Python major/worker_api; live sequence `:251-258` resolves imports through mutable `sys.path`. B1b imports at`:30-39` and platform capture at `screengrab.py:18-20,33-51`. | Pin and independently compare the actual executed coordinator/launcher/worker/adapter/route/crop source closure, interpreter/packages, resolved module locations, shim/wrapper/install/config identities and output schema needed by the selected route. Record exact interactive-session/hidden-worker deployment prerequisites. Matching approved case must reach the actual selected boundary; TestOnly cannot certify native readiness. | Stale/frozen source, failed copy, changed path/dependency/config or supported-argument drift reaches any operational effect. Current source audit is **not** a complete transitive dependency manifest or load-race/security certification. |
| Authorization: caller supplies `expected/effects`; candidate`:109-119`; local domain`:36-45`. | A reviewed authority separate from actual invocation fixes exact source/args/route, owner, start-versus-attach scope, cap, expiry, output destination and stable per-grant store. Atomic claim precedes any launch; sequential/concurrent/crash/restart cannot start twice or refund. Preserve trusted stable local-store assumptions. | Invocation manufactures its own approval, renamed output grants another start, claim follows launch, failure/timeout restores credit, or an unowned PID is accepted. Do not require an unapproved general token service. |
| No-op/refusal entry paths | Actual selected launcher→worker→adapter boundaries retain matching positive, unknown/malformed/stale/direct-worker negative cases and validated TestOnly zero process/input/foreground/desktop/capture/attempt-output effects. Isolated later controls must discriminate each relevant bypass, not merely wrong self-hash. | Any refused/TestOnly path reaches operational effects, or all positive cases merely refuse. Existing P/Q tests are retained evidence, not an integrated qualification. |
| Owned process lifetime: worker`:41-66` has no real disposition; B2h saved driver`:103-118` is outside protected boundary. | Name one owner, exact created child identity/handle, permitted helper children, deadlines and teardown policy. Offline harmless child/IO fakes at the **actual integration boundary** must cover success, driver exception, timeout, child exit/crash and owner interruption: wait/terminate/reap outcomes recorded, no retry/unattributed child. No “complete” until terminal ownership/cleanup is verified. | Orphan/unowned process, PID mismatch, unbounded wait, ambiguous exit called normal/clean, or fabricated terminal receipt. A hard owner death cannot be solved by its own `finally`: require an explicit surviving cleanup/reconciliation mechanism **or leave abrupt-owner cleanup/deployment blocked**. No general crash-resilience claim from synthetic finally blocks. |
| Input lifetime: `lrinput.py:95-116,128-141`; `acquire_game_foreground.py:115-131`; sequence`:332-346`. | Fix initial permitted input/enable state and teardown contract. Foreground helper snapshot restoration and route-level neutralization/release are separately checked; handles/maps close on each covered failure. Exact agreed snapshot/neutral state must be independently read back. | `release()` is confused with exact snapshot restoration: it clears keys/buttons/deltas and disables, not restore prior bytes. `LRInput.__exit__` closes mappings, not release input. Early failures before sequence's try or hard interruption without surviving cleanup remain blockers, not guaranteed restoration. |
| Capture/evidence: B1b`:169-205`; sequence`:275-288,337-346`; candidate`:47-99`. | Exclusive owned attempt output, attributable source/grant/route/PID records, immutable image bytes/hash and raw step packets. Always validate client geometry/state/focus around the selected capture (not only synthetic geometry mode); separate wrong geometry/state, actual capture exception, interruption and destination collision controls. Distinct valid later grants preserve the first file set/bytes. Output-persisted/incomplete is not final success; missing final receipt stays incomplete. | Geometry refusal earns exception PASS, capture/state/visibility prerequisites absent but image accepted, error record/first output lost or overwritten, or “owned-mock” used as native termination proof. B1b saves images **before** computing `ok`; preserving a rejected image is evidence retention, not acceptance. |
| Foreground/visibility/session: `acquire_game_foreground.py:65-112`; `screengrab.py:33-51`; saved interactive task above. | Separate later live approval, explicit start cap/no retry, fresh ownership and agreed exclusive desktop-use window. Owned HWND/path/client rect, actual foreground PID and unobscured game pixels/state all required. Offline mocks can verify refusal contracts, **not** establish actual visibility/native pixels. | Treating HWND/640x480/foreground alone as proof of unoccluded game pixels, using banned `goto_level.focus`, recreating old tasks/config swaps or spending frozen starts without approval. `goto_level.main:486` still calls that banned focus; reuse only selected safe seams, never that CLI wholesale. |
| Navigation/native binding limits | Preserve RETURN-at-highlight0, selected sequence and loaded-path evidence. Keep B1b menu scope and B2h synthetic-shim observation distinct. Any future live outputs are newly qualified at their actual scope. | Claim B2/B3 completion, physical-left/SDL binding, B3k C→G/sharedS identity or application entry from these mocks/historical observations. Level→map/back and two-unlock-state gates remain unpassed. |

**Smallest next decision:** whether to approve **one bounded offline implementation
and verification of this explicit owner-adapter contract**, using actual protected
entry boundaries, selected existing read/IO/crop seams and harmless child/fake effects;
no stock start, desktop action, deployment or automatic live follow-on. Before such
approval, name its exact source/dependency allowlist, reviewed local grant issuer/store,
one-owner-created-child versus explicitly handed-off attach policy, output schema and
timeout/interruption cleanup policy. Recommend owner-created-child; if hard-owner
cleanup cannot be specified without expanding scope, keep that deployment path blocked.
This is a proposal only, not a new queued task or a demand for another assessment.

## Facilitator check — independently verified and narrowed

Read every headline source range directly. Own
[citation audit](../../build/tmp/offline-route-assessment-2026-10-28/citation-audit.py)
checks25pinned source ranges with literal/AST assertions and independently rechecks
saved B1b3/0 brackets/640x480 plus B2h report/raw loaded path. It **imports no harness**,
runs no route/test campaign and reads no original binary. Output:
`citation-audit.json`, PASS. Reused [B2n review](phase-b2n-offline-recovery-review.md)
instead of repeating recovery research.

Worker's central result is supported: minimum missing edge is protected deployment/
owner lifecycle, not missing capture engine. Narrowings made here, not repairs:

- The proposal is **not simply calling B1b main plus B2h live**: that would reintroduce
  an extra launch/config writes, first-process attachment and replaceable output paths.
- Worker left start-versus-attach and grant-service alternatives open. Recommendation
  here chooses one owner-created child as the smaller attribution contract and does
  not invent a general authenticated service requirement.
- Historical driver does own/terminate a child, but aggregate failure/timeout/hard-owner
  cleanup is not protected or qualified. “No orphan on every crash” is a later fixed
  falsifier/blocker, not a property established by reading `finally`.
- Release/disable differs from exact prior input restoration; no-op boundary does not
  qualify dependency deployment; source/API geometry checks do not prove visibility.
- Worker reported02:52:34Z “start” from the queue's conservative round clock. Actual
  authenticated child creation02:52:52.294Z, first sampled worker clock02:53:46.998Z,
  packet end02:54:40.631Z; actual succeeded/idle02:54:52.248Z governs terminal accounting.

Own `verify-integrity.py`:944old packet files plus320inherited protected paths
(including external B2r dump) hash-exact,26untouched dirty paths,16signed gate texts,
exact complete prior B2s stopped run and all older archive objects preserved. Source/
tests unchanged. Brief lint and `git diff --check` PASS. Initial own archive comparison
used preparatory JSON decoded with default cp1252; corrected checker to use byte-exact
before-copy read as UTF8. CLI discovery/path-conversion errors and the check error are
disclosed in current `verification-plan.md`; no worker repair/candidate change resulted.
Worker tool audit also retains one malformed shell request rejected for missing
`command` before execution; all its actual shell commands were mkdir/date/hash/status,
and its only writes were the assigned candidate note. PLAN size lint initially rejected
new administrative prose; only that new prose was shortened to29992bytes, then PASS.

**Retained, not fresh:** C3 `.06354818984885621` FAIL versus`.02`; C1 zoom1
`.0160863988750699` / elev_down`.016121214378294987` PASS. All16signed gates,
masks/references/exclusions unchanged. B2/B3unaccepted; C2native gateunpassed;
B3k identity/physical-left gap, PhaseD/D1/D3/digger-clock parks retained.

Keep P/Q/R/S component proofs at their scopes, **whole launcher unaccepted**;
B2q worker scratch violation/B2r facilitator scratch violation remain disclosed,
external dump preserved. Original extra-start breach (2actual/1authorized), lost first
images, historical counts and all unused frozen allowances remain, not renewed.

## Stop, clocks and resources

Fresh activation02:49:21.238Z (new child; includes all preparation), first shell
02:49:26Z; total deadline03:34:21.238Z, worker return deadline03:19:21.238Z reserved15min.
Dispatch request02:52:45.728Z; tool start02:52:52.235Z; actual child idle02:54:52.248Z:
**126.520s request→idle**,119.954s child→idle. Final closure time and dispatch→landing
are recorded in queue/current `final-clock.json`, not inferred from packet time.

ONE assessment round/attempt; same effort **7total rounds/8total attempts**, still
5implementation rounds (B2n review1round/2attempts retained), no cap reset. Zero game
starts/attaches/input/foreground/desktop exercises/screenshots/install/save/config
writes/tasks/REA/debugger/hooks; no safety suite/mutation/build/CTest/app execution.
Worker independently succeeded/idle, active endpoint only parent+self; all own
inspection tools synchronous, no operational resources acquired/restoration owed.
Game/process/install state **not freshly queried**. No commits/push, HEADe649d10.
**STOP after independently verified assessment. No repair/next/deployment/live trial.**
