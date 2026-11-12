# Current reconstruction roadmap

Updated 2026-11-12. Next priority: safe test parallelism and shorter verification
cycles before another gameplay feature. The first bounded task,
test-parallel-isolation-001, is queued and prepared but not launched. Full-project completion criteria are unchanged.

## Completed milestone
Visible fluid and ordinary lever interaction on installed MM_W_012 were accepted
in loop-7bcd0ea7da8241b58ce68791670dbbc5, stopped at the requested boundary after
4h25m32s. The ordinary route toggled the lever, filled the tank and won20/20;
reload drained the tank and fresh-process persistence passed. Final verification:
148 main /84 recovery tests passed. See docs/STATUS.md for exact evidence links.
No ordinary second lever contact was observed. Review accepted the explicit
source/data finding that another contact would have no visible effect; this is
not observed repeat gameplay. Original depth fidelity remains unresolved under
the approved provisional Less/write-on policy. Other parks remain closed.

## Verification improvement plan
1. Inventory the registered main/recovery tests from the actual CTest manifests.
   Record duration, CPU/memory demand, output/temp/save paths, cleanup behavior,
   baseline/after relationships and play-save-restart prerequisites. Distinguish
   assertion tests from observation drivers whose zero exit only proves integrity.
2. Isolate shared mutable files and add explicit dependencies or resource locks.
   Keep dependent play/save/restart sequences ordered. Audit both fluid camera
   variants and other users of common evidence roots; names alone do not prove
   isolation. Read-only installed assets may be shared. Preserve normal saves.
3. Trial four concurrent test processes on this32-logical-processor/~31GiB host.
   Keep compilation at12 jobs. Limit heavy render tests separately where measured
   contention demands it; serialize remaining conflicting tests. Do not blindly
   select12 test jobs or launch competing builds/verifiers in one build directory.
4. Add a focused preflight before expensive full verification: select relevant
   registered tests plus prerequisites, using separate outputs where required.
   A failed focused check returns actionable feedback early. It must never mark a
   task accepted or replace the required full main/recovery suite for code changes.
5. Evaluate reuse separately: an unchanged source/build/test configuration may
   reuse authenticated verification for review/report reconciliation. Separate
   execution/control integrity from verified-content identity only with explicit
   invalidation tests. Test/build/asset/toolchain/environment changes must invalidate
   reuse; stale reports, missing logs or uncertain provenance require a fresh run.
   Do not weaken protected-control hashes or implement broad caching by assumption.
6. Report measured serial versus parallel wall time, peak memory, per-test outcomes,
   and artifact collisions. Select the default only after validation; retain a
   one-job fallback and report the actual selected concurrency in verification logs.

## Acceptance for the future workflow task
- Same registered tests, assertions and timeouts; required observation/review still
  separate from CTest success. No game behavior or fidelity acceptance changes.
- Successful full serial and four-job runs on the same content/configuration, with
  preserved logs and no save/output corruption. Re-run the parallel configuration
  once to check ordering/race sensitivity; no unsupported speedup claim.
- Known dependency chains preserve order. Relevant preflight failures demonstrably
  prevent starting the expensive full suite and retain a clear failure result.
- Any verification reuse is exercised with positive unchanged-content and negative
  source/test/CMake/config/toolchain/evidence-change cases before enabling it.
- Protected changes occur only with idle runners, backups and RepositoryLease.
  Add targeted controller tests, preview, and independently review the final diff.
  Generic worker permissions must not be widened to edit protected controls.

## Execution and model choice
Recommend Sonnet5.5/high for the bounded workflow implementation; recommend Opus
only if dependency/caching reasoning exposes a concrete unresolved difficulty.
No automatic escalation; Fable requires explicit approval. User overrides win.
The queue contains only the isolation/scheduling stage of this plan. Verifier
defaults, model pins and running processes are unchanged.
Current configuration remains12 compile jobs /1 test job. The completed fluid
launcher must remain a no-op; prepare a separate scoped workflow task when requested.
Original execution/capture, broader timing/skills/clipping/controller gameplay work
and unrelated features remain parked. This plan concerns build/test orchestration.

## Prepared next run
Use run_test_parallelism.py: one task, Claude Sonnet5.5/high, worker50min,
verification90min, review20min, maximum3 attempts and8 dispatch cycles.
It audits shared state and implements only test/CMake isolation. Stop after review.
Then the coordinator benchmarks serial versus four jobs and repeats four jobs
before choosing a default. Preflight and verification reuse are later work, not
part of this worker's permission to change protected live controller code.
