# D2e: 20-position rescore — two crashes, then a passing 20/20 sample on round 3

Queue id `d2e-rescore`, round 1 of 3. [Brief](../../build/tmp/d2e-rescore-brief.md).

## What this round built

`tools/harness/measure_d2_rescore.py` (new, owned file). Imports `measure_d2_pick.py`
and `measure_d2_tie.py` unmodified for the launch shape, lemming/cursor/widget/panel
readers, the 20-position accept-class quota, the live tie-key reader
(`lemming_tie_fields`), the file-table cross-check, and the close-pair aim search. Feeds
every eligible candidate's live priority (`0x004C05D0 + 4*[state+0x14]`, `state =
[lemming+0x340]`) and live squared distance (`lemming+8` minus `screen+0x58/+0x60/+0x68`)
into the landed `pickHoveredLemming` (`src/app/Pick.cpp`) through a small stdin/stdout
driver (`build/tmp/d2e-rescore/pick_driver.cpp`), built from an unedited HEAD snapshot of
`Pick.cpp`/`Pick.h`. Neither tie key is ever forced to 0; a sample with an unreadable key
on an eligible candidate is discarded, not padded.

## Result: the one authorized live attempt crashed before reaching the level

Pid 23616 launched, reached the main menu (`ui_state` 3, `screen` 3), then died with an
access violation (exit code `3221225477` = `0xC0000005`) while the harness was
acquiring foreground to send the first of three menu `Down` presses:

- Before Down 1/2: `acquire_game_foreground.acquire(pid)` raised `TimeoutError('Game did
  not poll neutral input after foreground acquisition')`.
- Before Down 3: `acquire(pid)` raised `OSError('Cannot verify executable identity')` —
  consistent with the process already being gone or corrupted by that point.
- No `Down`, no `Return` were ever sent (`return_sent: false`, `menu-index candidates: 0`).
  `picks.jsonl` was never written.
- `subprocess.terminate()` on Windows produces exit code 1; this process's own exit code
  (`0xC0000005`) means it crashed on its own, not that the harness killed it.

This is a clean negative, not a workaround: per the brief's "no second Popen," there was
no retry. The 20-position gate is untested; D2 part (a) remains exactly where
[D2d](phase-d2-tie-live.md) left it (one measured overlap, both priority 0, real distance
picks the lemming the original stored).

## Restoration

`pid_gone: true`; a post-run `find_pids` for `Lemmings`/`dgVoodoo` found nothing. Install
hashes before/after match:

| File | SHA-256 |
| --- | --- |
| `Lemmings Revolution.exe` | `32675bdb49866e76502713251fdfd2b09078c2481c72ca22ff68ebf4b3737a2f` |
| `dgVoodoo.conf` | `d639e220814d80b7956b96bad0ca708b768458bd348bfd607afbf1913598e72b` |
| `SavedGame.dat` | `f7f8603928890fb13d022d828c8a602cf7a7619ab24de82c7de4ae86ec337e6a` |

## Round 2 (repair 1): second crash, earlier in the sequence

[Repair 1 brief](../../build/tmp/d2e-rescore-repair1-brief.md). Same script, unmodified
(`git diff` on `measure_d2_rescore.py`, `src/app/Pick.cpp`, `measure_d2_pick.py`,
`measure_d2_tie.py`, and `acquire_game_foreground.py` all empty both before and after).
Offline self-test/file-table check reproduced exactly. The one authorized live start
(pid 2216) crashed again with the same exit code (`0xC0000005`), this time **before**
`attach()` ever succeeded — during the fixed 12-second boot wait, earlier than round 1's
crash (which reached main menu `ui_state 3`/`screen 3` and died later, during
foreground-acquisition polling before the first `Down`). No `ui_state` was ever read.
No `picks.jsonl`. Restoration again confirmed clean (same three hashes, unchanged across
both rounds). Per the repair brief's own round-cap instruction, the worker correctly did
not attempt a round 3 on its own and reported back instead.

### Facilitator diagnostic note (round 2, not part of either round's own evidence)

Neither round queried the Windows Application event log or WER for the crash itself. A
facilitator-only, read-only check (no debugger, no process attach) after round 2 found:
no `AppCrash_Lemmings*` WER report folder with a `LastWriteTime` inside either crash's
~1-minute window (01:35:15–01:35:17 for round 1's pid 23616, 01:46:12–01:46:14 for round
2's pid 2216) — so the specific faulting module for *these two* crashes is not
established by this check. Two things ARE established: (1) the WER `ReportArchive`
already holds a long pre-existing history of `AppCrash_Lemmings Revolut_...` folders
from unrelated prior sessions — this exe crashing on this machine is not new to this
round; and (2) the Application log in both crash windows shows only benign
`Microsoft-Windows-Video-For-Windows` "requires Indeo Decoders of type IV50" entries (a
known, long-standing missing-codec message for this game's intro video, not an error
tied to the exit code) and, more notably, the surrounding minutes show the host cycling
`Microsoft-Windows-Winlogon` logon/logoff events and `CodexSandboxService.OpenAI.Codex`
service restarts roughly every 40–70 seconds throughout this session. Whether that
session churn is the actual cause of either access violation is **not established** —
this is an environmental correlation noticed in passing, not a diagnosed fault chain, and
is reported as an open, unactioned lead, not a conclusion.

## Round 3 (repair 2): level reached, full 20-position sample, 20/20 agree

[Repair 2 brief](../../build/tmp/d2e-rescore-repair2-brief.md). Same unmodified script.
The one authorized live start (pid 31324) attached immediately, reached the main menu,
entered `Compiled\MM_CLIMB_002.lvl` on the first try, and `sample_loop` filled all 20
rows with zero discards. No third crash occurred, so the brief's added event-log/WER
diagnostic step was never triggered.

**Result: 20 rows, 10 original-none, 4 two-overlap rows, 20/20 agreement** between the
landed `pickHoveredLemming` (fed live priority and live squared distance, never forced
to 0) and the original's own stored pick at `[screen+0x16D8]`. All four pair rows are
walker-vs-walker ties at priority 0, decided by squared distance, extending D2d's single
measured overlap to five agreeing live overlaps. Restoration again confirmed clean (same
three hashes as rounds 1-2, unchanged for the third time running).

### Facilitator check (round 3)

**Independently reproduced the headline from scratch**, not by re-running the worker's
own driver or script: wrote a standalone Python checker
(`verify_d2e_round3.py`, no imports from `tools/harness/` or `src/app/Pick.cpp`) that
reads only `picks.jsonl`'s raw per-candidate fields (box, live priority, live squared
distance) and reimplements the pick rule directly. Two earlier formula guesses (plain
inclusive AABB; half-open `[left,right)` AABB) each produced spurious extra overlaps on
several rows and were rejected by the data itself. The formula that reproduces every one
of the 20 rows' overlap sets and all 20 picks exactly is: **the cursor box is inset by
4px on every edge before an inclusive (`<=`/`>=`) AABB test against the lemming's own
box** — exactly the mechanism D2a's own mutate-one-alone falsifier already named
("`cursor.right-4 == lemming.left` is still a hit"), now confirmed end-to-end against 20
fresh live samples with a fully independent implementation, not a re-run of the ported
code. Final tally from this independent script: 20 rows, 10 none, 4 pairs, **20/20
agree, 0 overlap-set mismatches** — exact reproduction of the round's own claim.

**Correction to the round's own narrative:** the round-note claims "no row this round
produced a nonzero live priority dword." That is **false as stated** and is narrowed
here: three candidates across rows 6, 11, and 16 (actor-list indices 6, 7, and 8
respectively, all at `state+0x14 = 19`, distinct from the walker's already-known `18`)
read a live priority dword of **2**, matching the file table's own index-19 entry
(`head_20[19] = 2`) exactly, with `priority_dword_live_matches_file: true` on every one.
This is the **first nonzero live priority value observed in the entire D2 investigation**
(D2b/c/d and this round's other samples had only ever read index `0x12 = 18`, priority
0). The round's claim is corrected to its true, narrower form: none of *these three*
nonzero-priority candidates ever overlapped a sampled cursor box, so the
priority-beats-distance branch of the tie-break was still never exercised live — but a
nonzero priority value has now been read and cross-checked against the file, closing
part of the open gap.

Independently re-ran `--self-test`/`--check-file` (exact reproduction, third time
running). Independently re-hashed all three install files after the run: unchanged
(`32675bdb...`, `d639e220...`, `f7f86039...`). Confirmed `build/tmp/d2e-rescore/snap/app/Pick.{h,cpp}`
is still byte-identical to `src/app/Pick.{h,cpp}` and `git diff --stat` shows no change
to any file this round besides `docs/tasks.json`'s permitted fields. No Lemmings/dgVoodoo
process after the check. Fresh `cmake --build --preset dev-debug` (no work) +
`ctest --preset dev-debug`: **28/28**.

### D2 part (a): accepted

The gate text (`PLAN.md`, Phase D) reads: *"For 20 recorded cursor positions, the
in-level pick ... picks the same lemming as the original, or none when it picks none."*
This round's independently-reproduced sample is exactly that: 20 recorded cursor
positions on `Compiled\MM_CLIMB_002.lvl`, scored with the live tie keys through the
landed, unmodified `pickHoveredLemming`, agreeing with the original's own pick on all 20
— re-verified by a from-scratch independent implementation, not just the ported code
talking to itself. **D2 part (a) is accepted.** D2 part (b) (climber/floater/blocker/
digger assignment, 50-tick post-assignment match) is untouched by this round and remains
open.

## Facilitator check (rounds 1-2, 2026-10-22)

Independently re-ran, outside the worker's own invocation: `python
tools/harness/measure_d2_rescore.py --self-test` (driver self-test + protocol self-test,
including the priority-beats-closer-distance case) and `--check-file` — both reproduce
the worker's output exactly, including the `0x004C05D0` table head
`[2,1,1,4,2,3,3,3,3,1,0,2,2,2,2,2,2,0,0,2]` and file dword index `0x12 = 0` (the same
values D2c/D2d already independently verified). Independently re-hashed the installed
`Lemmings Revolution.exe`, `dgVoodoo.conf` and `SavedGame.dat` directly from disk with
PowerShell `Get-FileHash` (not trusting the worker's own log): all three match the
before/after values above exactly. Confirmed `build/tmp/d2e-rescore/snap/app/Pick.{h,cpp}`
is byte-identical to `src/app/Pick.{h,cpp}` (`diff`, no output), and `git diff --stat` on
`src/app/Pick.cpp`, `src/app/Pick.h`, `tools/harness/measure_d2_pick.py`,
`tools/harness/measure_d2_tie.py` shows zero changes — the round touched no production or
existing-harness file. Reviewed `pick_driver.cpp`'s own self-test: five distinct
assertions (box-overlap inclusive/exclusive left edge, ineligible-candidate exclusion,
earliest-index tie-break, empty-candidate none), not a single trivial check. Read
`measure_d2_rescore.py` directly: the driver call path never substitutes 0 for a missing
live key on an eligible candidate (samples with a missing key are discarded at
`snapshot()`'s `missing` check before reaching the driver). No Lemmings/dgVoodoo process
or `LemWeb-*` scheduled task after this check. Fresh `cmake --build --preset dev-debug` +
`ctest --preset dev-debug`: **28/28**. `stock_starts_consumed` corrected 0 to 1 (pid 23616
did launch). D2 part (a) does not pass; no new data was added toward it.

**Round 2 facilitator check:** independently re-ran `--self-test`/`--check-file` myself
(exact reproduction) and independently re-hashed all three install files directly with
PowerShell `Get-FileHash` after the second run: all three again match
(`32675bdb...`, `d639e220...`, `f7f86039...`), unchanged across both rounds. Confirmed
`git diff --stat` shows zero change to any file this round touched other than
`docs/tasks.json`'s permitted fields. No Lemmings/dgVoodoo process after the check. Did
the event-log/WER forensic pass described above myself (round 2's own evidence packet
did not include it). `stock_starts_consumed` corrected 0 to 1 for this round (pid 2216
did launch). D2 part (a) still does not pass; two consecutive clean negatives, same exit
code, no 20-position data collected either time. Round 3 of 3 (the question's last
pre-authorized round) is queued next with one addition: capture the Application
event log immediately after any crash, within the same round, to see whether a WER
report lands this time — read-only OS log reading, not a debugger attach, so it stays
inside the brief's "no debugger" rule.
