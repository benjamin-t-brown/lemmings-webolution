# Current reconstruction roadmap

Updated 2026-11-12. The user approved a playable-clone-first direction. Current
milestone: **playable v1**. Full-project completion criteria in completion.json
are unchanged and are not satisfied by this milestone.

## Playable v1 (current milestone)
Goal: every one of the 102 installed levels can be entered from the ordinary
frontend, played with its supplied skills and gadgets, won by the clone's own
rules and persisted, without the original executable.

Explicitly set aside for this milestone, not resolved: original timing (D1/D3,
release/digger, base clock), full-frame pixel fidelity (C2/C3), original depth
state, Armageddon timing, and any comparison against the running original. Their
ledger entries in docs/STATUS.md stay FAIL/OPEN/PARKED exactly as recorded and
remain required for full completion. A playable-v1 win is clone-observed evidence
only. Original execution is approved for interactive sessions only (see below);
oracle-001 stays blocked for the queue.

## Sequence
1. **Finish the capped verification work.** Run test-parallel-isolation-001
   (already queued, prepared launcher run_test_parallelism.py), then the
   coordinator benchmarks serial versus four test jobs, repeats four jobs once and
   selects a default with a one-job fallback. Process work stops there. Focused
   preflight and verification reuse (former steps 4-5) are deferred, not queued;
   reopen them only if measured verification time still blocks gameplay work.
2. **Find what is broken, broadly.** Two sources, in either order:
   - User playtest of the first 10-15 map nodes in the clone, recorded as a ranked
     defect list (level, what happened, what was expected).
   - An all-level smoke sweep: for each of the 102 levels load, render a frame,
     run idle simulation, record crash/hang/blank render/zero-lemming outcomes and
     a contact sheet. Queued as user-authorized level-sweep-001, after the
     isolation task. A clean sweep shows integrity, not winnability.
   The user chose both.
3. **Fix in order of how many levels a defect blocks.** Expected early items,
   to be confirmed by step 2 rather than assumed: play evidence for Builder,
   Miner, Basher and Digger; the gadgets with no accepted route (teleport play,
   speed pad, remaining levers/fluids, anti-gravity, multi-hatch); frontend
   reachability of later map nodes through earned progress.
4. **Track one number.** Levels won through the ordinary frontend route in the
   clone, out of 102, with the save persisted. Today: 5 (map nodes 0-3 and 8).
   Scripted and human wins are both counted and labelled.

## Acceptance for playable v1
- 102/102 levels have a recorded clone win from the ordinary frontend route with
  fresh-process persistence, or an explicit user-accepted exclusion with reason.
- No known crash, hang or soft-lock on any installed level.
- Every skill and every gadget type present in the installed levels has at least
  one win that depends on it.
- Full registered main/recovery verification passes on the final content.
- Reports keep clone observation, source inference and original comparison
  distinct, and do not restate any historical gate as passed.

## Carried over from the previous plan
Visible fluid and ordinary lever interaction on MM_W_012 were accepted in
loop-7bcd0ea7da8241b58ce68791670dbbc5 (148 main / 84 recovery tests passed). No
ordinary second lever contact was observed; original depth fidelity remains
unresolved under the provisional Less/write-on policy. See docs/STATUS.md.

Verification inventory guidance still applies to step 1: inventory registered
tests from the actual CTest manifests; isolate shared mutable files and add
dependencies or resource locks; keep play/save/restart chains ordered; compile
at 12 jobs; trial four test processes on this 32-logical-processor/~31GiB host
and limit heavy render tests where measured contention demands it. Same tests,
assertions and timeouts. Protected controller changes occur only with idle
runners, backups and RepositoryLease.

## Execution and model choice
Recommend Sonnet 5.5/high for the isolation task and for routine gameplay fixes;
Opus only for a concrete unresolved cross-system difficulty. No automatic
escalation; Fable requires explicit approval. User overrides win. Model pins,
verifier defaults (12 compile jobs / 1 test job) and the queue are unchanged by
this plan update. The completed fluid launcher remains a no-op.

## Original game
The user approved original execution for interactive coordinating sessions only;
see AGENTS.md. Use it where a clone defect cannot be settled from static evidence,
for example to confirm what a level or gadget does. Queue workers stay static-only.
