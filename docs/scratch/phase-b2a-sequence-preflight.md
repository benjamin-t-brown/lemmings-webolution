# B2a: screen-sequence preflight fails closed; no stock start

2026-10-23. Question 3 round 1 of 3. Worker packet: `build/tmp/b2a-screen-sequence/`
(`round-note.md`, `address-input-manifest.md`, `byte-manifest.json`, `rea-evidence.json`).
Helper: `tools/harness/measure_b2_screen_sequence.py` (offline, fail-closed). Zero stock
starts; the single granted start is unused and not carried over.

## Result

**FAIL_DO_NOT_START** at the 12-new-body cap. No Popen, attach, input, foreground,
screenshot or install write. Not a B2 gate result.

Established (READ):

- `0x004C416C` is still the construction counter (`0x488359`..`0x48836B`), not an enum;
  the active getter `0x487FB3` reads `[0x5D59EC]`, a separate getter `0x487FD3` reads
  `[0x5D59E8]`.
- The five menu rows from `0x4C4214` populate an array via `0x488A92`; a candidate focus
  chain exists but is not proved to be the semantic highlight accessor.
- `[0x5D5970]` has a getter, but its producer is not shown to be a keyboard key.

Missing edges for a safe capture: the lifetime of `[0x5D59E8]`; keyboard traversal of
the candidate child structure (`0x488DB8`, `0x489395`, `0x4891BB`, gates `0x48FB00` /
`0x48FAE0`); the native key poll and dispatch (`0x48896A`); the meaning of row 0's
virtual +0x0C; confirm, map, safe level entry, loaded-path proof, pause and back.

## Facilitator check

Ran the helper's `--preflight` (Mac, venv with capstone): exit 2, `FAIL_DO_NOT_START`,
`runtime_reads: []`, `stock_starts: 0`, `input_events: []`. Compared all 24 `.bin` pins in
the packet against the exe by `VA - 0x400000`: 0 mismatches. Exe SHA256 equals the
manifest's. No game process or `LemWeb-*` task on mouv; install hashes equal startup.
I did not re-derive the unread edges. The worker's unit tests (9/9) were not re-run
because the test file resolves `tools/harness` relative to its own location and the
helper exists only in the Mac clone until pushed.
