# Overlord directive — at-most-once simulated launch

Status: **consumed; technical component verified, stopped with facilitator scratch-path violation**.
Launch-time SHA256: `1714e5cf08554fe8ea2b4fd501d611c1a5973cddc80f4031feb8406b9badae6e`.
Result: `docs/scratch/phase-b2r-single-use-authorization.md`; one attempt, zero game
actions, no new commits. Technical proof retained WITH the disclosed preparation
dump outside the prescribed scratch root; not a clean procedural pass or whole-
launcher acceptance. Original scope below is historical; no unused budget transfers.
Issued: 2026-10-27 (user, overlord review 2026-10-27, "i approve", then "ok proceed").
Identity: `offline-single-use-authorization-2026-10-27`.
Approval: one explicit round-cap exception, default facilitator, 45 minutes, one
worker attempt, zero game/desktop actions. Verify that a single authorization cannot
permit a second simulated launch across duplicates, concurrency, restart or capture
failure, then independently disable the protection and show the test catches it.
Client/tier: OpenCode on mouv, default `facilitator` (Sol/high per ROLES.md), delegated
resume of `ses_e80f12432ffebdYi3RdAhbYN8v`, no override/fallback. Manual launch remains
supported, never concurrent with the owner.

## Purpose and bounded approach

Address the original extra-start failure directly, not general launcher architecture.
The candidate is still offline/unwired. Existing helper-level exclusive-create tests
do not establish actual launcher-level at-most-once behavior. Verify and, only as
needed, repair that component using the actual candidate entry path with synthetic
operational effects. No live adapter, real game launch or transferable stock authority.

Preserve [B2p source/argument binding](scratch/phase-b2p-source-argument-binding.md) and
[B2q TestOnly/worker checks](scratch/phase-b2q-testonly-worker-boundary.md). The user's
approved direction retains B2q's technical result with its disclosed scratch-path
violation; do not erase the violation or relabel it a clean procedural pass. Do not
repeat a review-only round. Snapshot current source and keep old artifacts immutable.

Facilitator owns exact allowlist/decomposition and independent verification; worker
owns candidate/test implementation. Restrict all new temporary test state to this
pass's authorized scratch root; inherited prior-pass test paths are not writable.

## Fixed component acceptance

- A fresh synthetic authorization permits the expected positive simulated launch
  through the actual launcher/worker path; refusing everything is not acceptance.
- Reusing the same authorization cannot reach a second simulated launch: sequential
  duplicate invocations, concurrent invocations, fresh execution after restart, and
  retry after capture failure are tested through actual entry boundaries. A repeated
  call to the consume helper alone is insufficient. Count operational events and
  retained authorization state, not just expected error strings.
- Consumption precedes the simulated launch and remains spent after downstream
  failure or interrupted execution; no implicit refund, caller retry or changed
  attempt-output name may renew the same authorization. Test bounded interruption
  cases at the relevant boundary with fresh state on restart. State exact supported
  filesystem/process assumptions; do not claim power-loss or cross-host guarantees.
- Execute a disposable copy with only the at-most-once protection disabled. The same
  negative property must fail because a repeated authorization can reach a second
  simulated launch, not because an unrelated approval/hash/receipt check fires.
  Preserve valid matching source/argument fixtures and actual executed mutant evidence.
- Preserve B2p/B2q behavioral regressions, before/after source attribution and source
  restoration proof. Reject missing required evidence, a second simulated launch in
  any required protected case, or regression of established boundary protections.

All target/launch/capture/desktop effects stay synthetic. Offline execution/restart
tests are not native process lifecycle evidence. Ordinary synthetic test-state writes
are allowed only within the authorized new scratch root. Passing this component is
NOT complete launcher acceptance, evidence-preservation certification, deployment
qualification, original-input binding or game fidelity.

## Budget, exclusions and cap

- **45 minutes total / one worker attempt / one round**, including preparation,
  verification, documentation and cleanup. Fresh activation and absolute deadline;
  no old session-creation clock, spare budget or second repair dispatch.
- Worker return at most **30 minutes**, tightened to reserve **15 minutes** for
  independent verification/closure within total. No facilitator completion of worker
  implementation. Stop/park on failure; ask before interrupting an over-budget worker.
- **Zero game starts, attaches, input/foreground or desktop actions/exercises.** No
  screenshots, native sampling, scheduled tasks, install/save/config writes, interactive
  app runs, debugger/Frida/hooks or new original-binary research. Never execute the
  rejected B2m launcher, even TestOnly; use only saved evidence/synthetic regressions.
- No evidence-retention subsystem repair, general security/framework project, new
  capture engine, app integration, B3k research, observer work, C3 polishing or Phase D.
- This is implementation round **4** of the same safety effort, not a reset. Retain
  B2o failure, B2p scoped proof, B2q technical proof/deviation and B2n's separate review
  (including review: round5/attempt6 if this dispatch occurs). Record explicit approved
  exception/rationale: direct extra-start prevention with known bounded falsifiers,
  serving future B2/B3 measurement. No automatic next exception/component follows.
- Leave changes **uncommitted**, no push. Preserve unrelated dirty edits and old
  evidence; no reset/stash/checkout/destructive cleanup. No gate/mask/reference changes.

## Ownership and stop

Parent check 2026-10-27T23:06:30Z: HEAD e649d10 on decompile, queue empty/stopped.
Facilitator `ses_e80f12432ffebdYi3RdAhbYN8v` and prior worker
`ses_e80945d23ffeZFV0gzd2biKTPz` succeeded/idle; active endpoint lists only overlord
`ses_e80f36524ffe7KzSDCj9yVcKCp`. Recheck sessions, queue, directive hash and dirty
baseline before queue writes/dispatch. Same owner archives previous stopped run
exactly and records fresh authority/clock/user decision. Historical directive:
[TestOnly/worker boundary](DIRECTIVE-TESTONLY-WORKER-2026-10-27.md), consumed.

Retained, not fresh: C3 0.06354818984885621 FAIL versus0.02; C1 zoom1/elev_down
0.0160863988750699 / 0.016121214378294987 PASS. All16 signed gates remain unchanged;
B2/B3 unaccepted, C2 native gate unpassed; Phase D parks and B3k binding gap remain.

Stop after independently verified component or failure/cap/blocker; also stop on
safety breach, ownership conflict or unavailable pinned delegation. Return exact
positive/duplicate/concurrency/restart/failure/mutant results and limits, regression
preservation, changed paths/no commits, clocks/attempts/zero game actions, all overruns
and worker/resource release. Full launcher remains unaccepted. Evidence preservation
and deployment remain separate; no live trial or automatic next pass.
