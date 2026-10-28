# Overlord directive — source/argument binding only

Status: **consumed; scoped component verified and stopped**.
Launch-time SHA256: `be2ae51e77c15c01024813abfaa982e4032fbc42c374df29e0d059f4bdc946d5`.
Result: `docs/scratch/phase-b2p-source-argument-binding.md`; one attempt, zero starts,
no new commits. Operational launcher-to-worker route only; whole prototype remains
unaccepted. Original scope below is historical; no unused authority transfers.
Issued: 2026-10-27 (user, overlord review 2026-10-27, "yes").
Identity: `offline-source-argument-binding-2026-10-27`.
Approval: user approved the narrower recommendation: default facilitator, 45 minutes,
one worker attempt, zero starts; source/argument binding and its negative controls
only. Stop on independent verification; park this approach if it fails.
Client/tier: OpenCode on mouv, default `facilitator` (Sol/high per ROLES.md), delegated
resume of `ses_e80f12432ffebdYi3RdAhbYN8v`, no override/fallback. Manual launch remains
supported, never concurrent with this owner.

## One question and fixed acceptance

Can the offline candidate reject changes to approved source or arguments before
any simulated operational effect, with tests that demonstrably detect a disabled
binding guard? This is a component repair for future B2/B3 safety, not complete
launcher certification. Start from [B2o's rejected prototype and falsifier](scratch/phase-b2o-offline-launch-safety-rejected.md).
Preserve exact before-source and evidence before modifying candidate files.

The facilitator scopes one worker's allowlist within the existing candidate/worker/
tests and scratch evidence. Repair only the approved-source/argument boundary.
Use an independently fixed synthetic approval contract rather than trusting hashes
computed from whatever source happens to execute or mutable actual arguments.
Do not build a general authorization framework or repair unrelated safeguards.

Acceptance is an independently repeatable differential check through the candidate's
actual launcher-to-worker path, with all target/launch/capture/desktop effects mocked:

- Matching approved source/arguments reach the expected synthetic success path;
  the implementation must not obtain a pass by refusing everything.
- Changed source or supported arguments refuse before operational effects, including
  token consumption, attempt creation or synthetic capture. Existing B2o argument-drift
  falsifier must stop reaching synthetic success. State exact coverage/limitations.
- Tests of source and argument mismatch must fail when their respective binding
  protection is deliberately disabled alone. Execute the altered boundary; merely
  generating a mutant file or testing an unrelated wrong hash does not count.
- Demonstrate the relevant regression against the preserved pre-change behavior;
  retain commands, expected/observed results, mutant identity and restoration proof.
  Positive happy-path tests alone are insufficient. Verification remains facilitator-owned.

Successful component verification does NOT certify TestOnly, single-use/crash safety,
non-overwrite/failure receipts, dependency deployment, native capture, physical-input
binding or game fidelity. Preserve those open issues; no live readiness claim.

## Budget and boundaries

- **45 minutes total, one worker attempt / one round**, including preparation,
  independent verification, documentation and cleanup. Fresh activation/absolute
  deadline; do not reuse session creation time or any old unused pool.
- Worker return **at most 30 minutes**, tightened to reserve **15 minutes** for
  verification/closure within the total. No second attempt/repair dispatch. Failed
  final verification stops and parks this approach; no facilitator implementation
  takeover. Ask before interrupting any over-budget worker and disclose overruns.
- **Zero game starts, attaches, input/foreground or desktop actions/exercises.**
  No screenshots, native sampling, scheduled tasks, install/save/config writes,
  interactive app runs, debugger/Frida/hooks or new original-binary research.
- Only candidate offline execution with mocked effects; no live adapters or usable
  stock-start authorization. Never execute the rejected B2m launcher, even TestOnly.
  Preserve all historical failure packets and prior accounting unchanged.
- No new capture engine, general framework work, app entry integration, other safety
  repairs, B3k binding research, observer work, C3 polishing, Phase D or gate changes.
- Keep changes **uncommitted** for user review; no push. Preserve pre-existing dirty
  hunks and prototype snapshots; no reset/stash/checkout/destructive cleanup.

## Settled results and ownership

B1b's 640x480 capture and B2h's scripted entry remain valid at their scopes. B3k's
C-to-G/shared identity remains open. B2o's 7/7 tests did not establish its four
guard-bypass controls; actual supported-argument drift reached synthetic capture.
This narrowed pass does not erase that failed attempt or earlier native safety breach.

Retained, not fresh: C3 0.06354818984885621 FAIL versus 0.02; C1 zoom1/elev_down
0.0160863988750699 / 0.016121214378294987 PASS. All 16 gates/masks/references and
exclusions unchanged. B2/B3 unaccepted, C2 native gate unpassed; Phase D stays parked.

Parent check 2026-10-27T21:18:11Z: HEAD e649d10 on decompile, empty stopped queue.
Owning facilitator `ses_e80f12432ffebdYi3RdAhbYN8v` and last worker
`ses_e80d558e2ffex59m3wEXW6punp` succeeded/idle; active endpoint lists only overlord
`ses_e80f36524ffe7KzSDCj9yVcKCp`. Recheck queue/sessions/directive hash/dirty baseline
before writes or dispatch. Same facilitator records fresh scope/clock/user decision
and archives the stopped run exactly. [Previous directive](DIRECTIVE-LAUNCH-SAFETY-2026-10-27.md)
is consumed. No concurrent facilitator or renewal of exhausted authority.

## Stop and return

Stop after component verification or failure/cap/blocker. Ownership conflict,
unavailable pinned delegation or any safety breach also stops. If failed, record
the approach parked rather than automatically repeating it. Return changed paths,
exact independent positive/negative/mutant checks, before-source preservation and
restoration proof, precise component limits, retained gates, attempts/time/zero starts,
overruns and terminal-worker/resource state. No automatic next pass or live trial;
further safeguards and any live use need separate approval.
