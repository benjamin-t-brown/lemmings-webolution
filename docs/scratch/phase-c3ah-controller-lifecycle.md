# C3ah initial/repair1 — actual-main offline checks; no launch recipe accepted

2026-10-25. Mocked harness evidence only. Initial AH replaced the controller with
an abstract `run(io)` skeleton. Its six passing tests were **rejected as complete
controller coverage**; preserved under
`build/tmp/c3ah-controller-lifecycle/before-repair1/`. Repair packet:
`build/tmp/c3ah-controller-lifecycle/repair1/round-note.md`.

## Facilitator check

Read actual copied controller, helper and complete mock suite. Repair executes the
old controller's actual `main`, `load_entry` and `get_cleanup`, with scratch files
and mocked process/memory/task/clock boundaries. Worker suite rerun: **8 tests pass**,
including successful menu/attach/entry/output/cleanup, preflight, timeout, bad
output, slot/file/task and process-cleanup failures. Entry helper is really imported
under stubbed native dependencies; its device-driving `main` remains mocked.

Separate AST/source check `build/tmp/c3-current-entry-contract.py` confirms
`goto_level.RECORD_SIZE == SLOT_STRIDE == 0x118`, `LEVEL_TABLE == 0x4E35D4`, and
the original helper lacks the import. Scratch helper changes **only** that import.
This is a repository-source contract, not a new original-memory measurement.

Independent `build/tmp/c3ah-independent.py` imports no worker mocks. Three fresh
actual-main fixtures verify success with exactly one game and one distinct CLI
Popen, exact slot restoration; a surviving-process termination/wait failure;
and missing-size failure after the one mock game start but before CLI attach.
All real process/native interfaces are replaced, files are scratch-only, and
native entry is mocked. Result `build/tmp/c3ah-independent-result.json`.

The actual-copy repair fixes a false-success path when termination/wait raises.
Removing **only** `or result.get("process_cleanup_error")` in a private in-memory
source makes the independent failure assertion fail. On-disk source is untouched:
SHA256 `7ae5b5f2712a35347a7ee9673409ea2969d29087adafb11d69fe5b86a1870552`.
Worker's separate missing-import mutation restores helper bytes exactly to
`b2ce947134c2ed5ae77a838a09fc47fdb97e128dd79a99b56b65b4913e895aec`.

Own rebuild and **39/39 CTest** pass at18:05:58; source-independent harness result
adds no test target. Fresh `build/tmp/c3-check-ah-mocks-only/` explicit repair4
and independent histogram scores reproduce19715561/235008000=0.0838931483.
As for AG, this is the **rejected, uncommitted initial AF renderer**, not a new
accepted C3 baseline. All10 C1/full-frame reports retained; gated panel views
0.0160864/0.0161212 still pass. No gate, exclusion, source image or footprint changed.

## Remaining deployment boundary

**Do not run this copy as a launch recipe.** Its CLI guard is mock-only, and its
default ENTRY/OUT/APPROVAL still name the old AD paths; tests explicitly rebind
them to sandbox/repaired-helper paths. The original AD launcher/helper remain
unchanged and rejected. A future live brief must bind a unique output/ledger and
the corrected helper in the actual deployed script, then dry-test those exact
bindings before making an approval marker. No marker or live approval is created
by this landing. The static pending debt is ours to repair, not a missing user
start permission.

Full failure completeness is not claimed: real entry/input/session behavior,
subprocess startup and final result-write failures, and stale pre-existing backup
cleanup are not independently certified here. No actual REA detach or live
cleanup was observed. This accepts the bounded actual-main regression component,
not the directive's complete deployed-controller/live-observation gate.

Zero starts/attaches/native calls, no install/game write or owned resource.
Original controller/helper and all historical retractions are preserved. Two
attempts count this run; the fourth-total-controller-attempt exception (including
AD initial/repair) is recorded in the repair brief. Further controller work must
be scoped and justified rather than silently assumed safe. Dispatch17:44:17,
repair17:50:16; landing/elapsed in the archive.
