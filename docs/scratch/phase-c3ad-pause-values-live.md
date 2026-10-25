# C3ad initial/repair1 — passive dry checks only; live approval refused

2026-10-25. User steering884798a authorized one live round from the existing
three-start pool. **No start, Popen, REA attach or live write occurred.** This is
not a permission blocker: the dry package failed verification before authorization
could safely be exercised. Initial and repair count as two attempts in this run.

Packets: `build/tmp/c3ad-pause-values-live/round-note.md`,
`repair1/round-note.md`, with initial artifacts preserved under `repair1/before/`.

## Facilitator check

Read complete payload, mock suite, controller and imported entry helper. Fresh
original-byte reads of473296..4732B5,472640..472659 and46D5C4..46D5F3 verify
descriptor EBP-18C / D=[EBP-B08] / D+8, returned EAX, and bind/draw stack layouts.
Five hook anchors independently match the installed authority bytes. Some first
disassembly starts were mid-instruction; only corrected aligned final windows
are used (`build/tmp/c3ad-facilitator-*-bytes.txt`). No new native behavior claim.

Independent Node VM (`build/tmp/c3ad-facilitator-drycheck.js`) reproduced the
initial failure:129 synthetic pre-pause binds globally stopped logging, so a
subsequent valid pause entry yielded zero snapshots. Repair isolates capped
streams and now yields one. This is synthetic load traffic, not a measured native
call count. Independent successful/failed HRESULT fixtures verify exact124-byte
descriptor versus null; a private single HRESULT-predicate inversion fails the
expected-byte assertion. On-disk repaired payload remains SHA256
`d36eb9ac46a05b29d90c3b73098102e61a22e7d5fb789903884f63338c0afca7`.

**Launcher rejected:** `live_controller.py` uses `entry.RECORD_SIZE`, but imported
`build/tmp/a1-zoom-elevation/enter_fast.py` neither defines nor imports that name.
The first such access is after stock Popen but before REA attach. Independent AST
export/reference check (`build/tmp/c3ad-facilitator-controller-check.py`, log)
fails on exactly that missing export, without importing/executing the controller
or target. Syntax-only compilation cannot catch it. Final controller SHA256
`1e8117c5b8ec21d997033b4fe49e2d26dab1d59db588b15fec3a5a50b8fc373b`;
earlier packet controller hashes differ and are not final-source authentication.
No approval marker was created. A known failing launch was not allowed to spend
the one-shot start. Other controller behavior was not dynamically dry-tested and
is not accepted safe merely because this first defect is understood.

## Disposition / resume boundary

Accept only bounded offline stream/ABI checks above; **reject live recipe**.
Pause gates, actual58D498, effective format/binding and wrapper texels remain
unobserved. New-pause evidence could not retroactively prove repair4 memory anyway.
The original plus repair, together with AE original/repair, use the final four
attempts of the unchanged24-round run budget; no further repair is dispatched.
Resume needs a new run budget, not another permission request for the already
specified one-shot method: fix the controller's exact record-size contract and
execute a complete harmless mocked controller lifecycle (Popen/menu/entry/output/
cleanup/finally failure paths) before any game attempt. Preserve one-start/crash
and read-only/no-COM-call constraints. Do not mistake this note for live approval.

Separate process/task check at17:19:51 finds no Lemmings process or LemWeb task.
No install or game-memory writes were made; no owned instrumentation exists.
Current-tree final build/CTest39/39 passed at17:19:21 while AE verification ran;
this no-production-edit packet adds no test target. All historical retractions
and native/effective-state limitations remain.

Dispatch16:34:28, repair16:54:52, disposition17:19:51 MDT:
**45m23s total**, **24m59s repair-to-disposition** including facilitator review.
