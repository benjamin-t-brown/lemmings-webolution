# A7d — base text-list draw reached; pause-selected contents still unlinked

2026-10-20 project date (worker finished 2026-10-21T05:07:11Z), `decompile`.
Explicit fifth-dispatch exception, user-selected Sol/max. **Partial READ map,
not a Paused-caption ownership proof, implementation or pixel pass.**

## Accepted route

Let G be the receiver of frame0x441845 and H=G+0x2018. No new runtime object
address or captured receiver identity was measured.

```text
frame0x441845
  0x441A79: add ecx,0x2018
  0x441A7F: call 0x44CD06(this=H)
    [0x4E33C8] != 0 and scalar [H+0x50] > 0
      0x44CEC4 / 0x44CFC4: call 0x44D0ED(this=H)
        [H+0x48] == 0 branch:
          0x44D1F1: push 432
          0x44D209: call 0x44DF40(this=H)
          0x44D223: call 0x44E150(this=H)
            0x44E190: call 0x414580
            0x44E197: call 0x414920(this=returned element)
```

This reaches a base text-list submission candidate from the accepted frame
route. **Neither 0x4E33C8 nor H+0x48/H+0x50 is established as gameplay pause
state.** The producer and exact text in H remain unknown; no STR_PAUSED index
or resource was bound to H. The immediate432 is a helper argument, not proof
of final screen placement, clipping, font geometry or ownership of band B/U.

The scalar branches distinguish >0.5 and 0<value<=0.5 for finite values; x87
unordered comparisons also bypass the draw. Do not replace this with “any
nonzero bits draw.” The >0.5 arm compares against original double at0x4B03A8;
the low arm compares against zero at0x4B0338. Lifetime/fade semantics are only
an inference. H+0x48's alternate arm submits H and H+0x24 separately; its
meaning is not identified.

## Raw-byte anchors

Authority SHA-256:
`32675bdb49866e76502713251fdfd2b09078c2481c72ca22ff68ebf4b3737a2f`.
PE32 x86 at0x400000; raw `.data` ends at0x4C6000. BSS addresses below are
encoded operands, **not file-observed values**.

| VA | Original bytes | READ |
| --- | --- | --- |
| 0x441A79 | `81c118200000` | receiver offset H=G+0x2018 |
| 0x441A7F | `e882b20000` | call0x44CD06 |
| 0x44CE10 | `833dc8334e0000` | compare global operand0x4E33C8 with0 |
| 0x44CE17 | `0f84ac010000` | zero skips to0x44CFC9 |
| 0x44CE23 | `dd4250dc1da8034b00` | compare receiver+0x50 double with0.5 |
| 0x44CEC4 | `e824020000` | high-arm call0x44D0ED |
| 0x44CFC4 | `e824010000` | low-arm call0x44D0ED |
| 0x44D0FB | `8a484885c90f84b9000000` | +0x48 flag chooses branch to0x44D1BF |
| 0x44D1F1 | `68b0010000` | push432 |
| 0x44D209 | `e8320d0000` | placement-helper call0x44DF40 |
| 0x44D223 | `e8280f0000` | list draw call0x44E150 |
| 0x44E190 | `e8eb63fcff` | per-element accessor0x414580 |
| 0x44E197 | `e88467fcff` | element receiver passed to0x414920 |

All were re-read from the original file using actual section mappings. The
worker manifest contains full windows and additional direct target pins.

## Keep different records distinct

0x44CD06 also passes indices0x44/0x45 to0x486847 and its returned pointers to
0x4404FA, with destinations **H+0x758/H+0x77C**, not H. The frame's other
post-scene receiver is G+0x27D0, passed to0x45B639. That body's content
producers were not exhausted, so it is not certified caption-free.

The helper0x486847 forwards global selection0x4C3434 to0x486819. The latter
loads a table via `[0x5D5998 + 4*selection]` and, on its signed bound branch,
returns `[table + 4*index + 4]`. Runtime table/string contents were not read.
0x5D5998 is not source-backed data; this is **unobserved runtime state**, not
proof that the contents cannot be recovered statically or otherwise.

0x4404FA walks a zero-terminated byte stream and builds list elements using
per-byte records and geometry/UV helpers. A glyph-quad interpretation is a
supported inference, **not an identified font atlas or caption string**.

A separate timer-like path divides0x4CBAAC by60, formats a buffer and builds
**H+0x224**, with placement arguments560/430. Its draw at0x44D0E4 is distinct
from H. Do not mistake that record or the index0x44/0x45 records for the caption
merely because they share text helpers.

## Facilitator check

The worker stayed at12 distinct function inspections, recorded in its research
ledger, then stopped expansion. It did not repeat0x45CFF9 or begin another
round. Candidate pseudocode-only producer leads are not promoted.

The facilitator independently:

- confirmed CLI session `ses_ea332d351ffeQ4l7QVMwHhWwh4` actually records
  `openai/gpt-6.1-sol`, variant max, agent round-agent; final message is `stop`
  and the owned CLI exited0;
- used a separate **pefile/Capstone** verifier, rather than the worker's stdlib
  PE/relative-opcode checker, to verify16 raw pin windows and25 relative targets;
- re-decoded all **eight** retained assembly ranges from the original bytes,
  matching every returned instruction boundary, and re-read the frame, base-list,
  scalar-gate, indexed-text and separate-timer operand/data chains;
- rejected the independent expected432→433 mutation, restored expectations and
  rechecked; separately reproduced worker process exits **[0,2,0]**. Negative
  exit2 is intentional. Authority and manifest bytes remain unchanged;
- authenticated the exported **39-record** REA bundle with
  `rea_import_evidence_bundle`, without reopening a target;
- ran the full build (no work) and fresh **24/24 CTest** passes.

Manifest SHA-256:
`43d0cd74d878f13462c64357c393fb8a3ca94391aa715fb27158ea601344d648`.
No production mutation applies: no production behavior changed. Main acceptance
narrowing is to **base text-list route and literal placement argument**, not a
pause-selected caption, screen-space identity, effective render state or attainable
error savings. Native READ is not runtime MEASURED.

Worker-owned Ghidra is closed; independent current-document check returns
target_unavailable. Importing evidence did not reopen a provider. Final process,
task and lint/diff checks are recorded in the queue. No stock/replacement game,
debugger or instrumentation was launched; no installation/source writes or
commits. Concurrent framework/model-mapping edits are preserved and not attributed
to this worker or verification round.

## Artifacts and remaining boundary

- Worker: `build/tmp/a7d-draw-loop-caption-bridge/round-note.md`, `pins.json`,
  `check_pins.py`, `verification.json`, `commands.md`, research ledger and bundle.
- Independent: `build/tmp/a7d-facilitator/verify.py`, `verification.json`, all
  `raw-*.txt`/`retained-*.txt` assembly checks.
- Dispatch/termination: `build/tmp/a7d-dispatch/worker.json`, event log and
  `exit.json` (returncode0). No process-owned model override was persisted.
- Reproduce: `python build/tmp/a7d-facilitator/verify.py`.

The missing discriminator is the **producer of exact base list H=G+0x2018**:
connect gameplay pause value1 to a selected string/index and destination H,
plus its display gate. H+0x224 and H+0x758/+0x77C are not substitutes. No
font/material/layout investigation or pixel fix is justified by the current
owner evidence alone.

Count remains5, with no automatic sixth dispatch or repair. Re-park at this
more specific producer boundary. The unchanged actual A7 score is0.0220410125,
and eight other view masks plus a second-level pair remain unidentified. No
new stock-start authorization is implied. Resume outline:
`build/tmp/a7-lower-band-native-resume-brief.md`.
