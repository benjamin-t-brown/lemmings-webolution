# B1m: runtime X read; font binding capture incomplete

2026-10-23. Queue `b1m-runtime-font-read`, two dispatches including one offline
repair. One stock start (PID 45172), no retry. Candidate packets:
`build/tmp/b1m-runtime-font-read/round-note.md` and
`build/tmp/b1m-runtime-font-read/repair1/round-note.md`.

## Result and limits

Three samples retain `80 02 00 00` at `0x58CB40`: the signed low word is
640, giving X=320 under B1k's signed-half rule. **This is not a
highlight-0-bracketed measurement.** The probe copied B1b's process-local heap
address `0x027A987C`; all three highlight reads were unreadable, not zero.
The saved picture visibly shows the English menu with the first row orange,
but cannot replace missing memory reads.

The font slot `[0x4CBF78]` points to `0x02798990`. Its first four dwords are
`0x4B15C8, 0x4BF968, 36, 32`. Byte reads establish font+8 as a manager-local
resource index, not an archive ordinal. The source sheet and dimensions remain
**unknown**. No production change, render or B1 score was made.

## Acquisition defect and bounded repair

`0x474520 b948cb5800` supplies the embedded manager's address directly in ECX.
The probe instead dereferenced its first dword (`0x4B19E8`) and captured 0x400
bytes there. Those bytes equal the executable's `.rdata` exactly: a table of
code pointers and neighboring data, **not the runtime manager/pool**. The
original packet's manager-pointer/object labels are superseded by this diagnosis.
Syntax/import preflight did not validate the pointer interpretation or the
current-process highlight accessor; spending the sole start before resolving
those was an acquisition failure, not evidence of an unavailable game resource.

Offline repair re-read the lookup/binder/consumer operands. `0x44041F 83c208`
forms font+8 as the lookup output; `0x440434 8b5108` passes it to manager
acquire. The lookup selects manager+0x30 (`0x449B43 83c130`), and the record
accessor uses index*0x88 plus pool+0x0C (`0x44C10A 69c088000000`,
`0x44C113 8b510c`). Thus the **first missing bytes are the pool-base pointer
at `[0x58CB84,0x58CB88)`**, not captured anywhere in this packet. For this
ended process's index36, the selected record would be at P+0x1320.

Acquire reads record+0x80 and then the wrapper's first dword
(`0x414C34 8b8280000000`, `0x414C3A 8b00`). The existing font UV consumer
divides by descriptor+0x14/+0x18 (`0x440631 da7114`, `0x440648 da7118`).
These establish offsets, **not values**. No selected record, wrapper,
descriptor or source-name storage was retained. The name getter/lazy loader
below this boundary remains unread; a short name alone is not archive identity.
No claim that static analysis can never resolve the source is made.

## Facilitator check

Independent code `build/tmp/b1m-facilitator-check.py` imports no worker/harness
code. It decodes all three raw samples, checks JSON/binary equality, confirms
X640/index36, matches all 1024 mislabelled snapshot bytes against raw PE `.rdata`,
and independently enumerates captured ranges: none contains the pool pointer.
The PE raw bounds also reject reading it from disk; `.data` file bytes stop at
0x4C6000. A one-byte X-input control decodes 641 instead of 640, then restores
the exact original bytes/hash. This is decoder discrimination, not a pixel gate.

I independently re-read all 27 repair instruction pins and their surrounding
assembly, plus the X getter/halving and `0x414D00` accessor (same index*0x88 +
pool+0x0C rule). The insertion-return tail at `0x45FF35` returns the slot index.
Initial mid-instruction X-context reads were discarded and repeated from
0x488AE9. The six-record REA bundle authenticated through canonical import
(`imported=6`, no added unknowns); no duplicate decompilation was needed.
Original packet hashes still match the repair's preserved manifest.

The acquisition script's top-level `finally` restores config and save in the
same process that swapped config, then compares all three install files. My own
post-run exe/config/save hashes equal the independently recorded pre-dispatch
baselines. Current process/task checks find no Lemmings/webolution or LemWeb
task. Full wrapper build and CTest pass **30/30**; repair made no source changes.
Before landing I compared decoded queue history against the prior commit,
restoring four encoding-corrupted historical strings; all earlier entries and
the previous standing run now compare exactly. Tracked landing text is LF.

Accepted as a **bounded acquisition failure plus verified offline diagnosis**,
not a completed runtime binding read. Original X observation retained with its
qualification; source/dimensions and highlight bracket remain missing.

## Stop and resume boundary

Question 1 has used two of three rounds but its **one of one stock starts is
spent**. Question 2 depends on the missing binding; all other milestones are
excluded by this directive. No automatic third round or live retry is queued.
The standing run stops for a user authorization decision, not a gate change or
cap reached. B1 remains unaccepted and unscored.

Recommended next authorization: offline preflight proving a current-process
highlight accessor and name/source storage reads, then one separately authorized
runtime acquisition. Re-read the current font index, pool pointer, selected
0x88-byte record, wrapper, descriptor dimensions and referenced source data;
do not reuse this process's heap addresses/index or infer `_FONT.BMP` from its
filename. The repair packet specifies known offsets and the remaining unread
name/source edge. Preserve the gate and the no-fitting rule.
