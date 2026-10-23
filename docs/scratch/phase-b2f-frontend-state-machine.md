# B2f: offline frontend state machine replays the b2e capture; unobserved behaviour is policy, not measurement

2026-10-23. Directive Q3 round 3 of 3 (cap reached). Packet: `build/tmp/b2f-frontend-state-machine/`
(`round-note.md`, `prediction.md`). Code: `src/shell/Frontend.{h,cpp}`, `src/__test__/test_frontend_sequence.cpp`,
`CMakeLists.txt` (test `frontend_sequence`; `Frontend.cpp` is built into the test, not yet in
`cmake/webolution_sources.cmake`). Zero stock starts, no process, no address read.

## Result

- `Frontend` is a pure machine (screen, construction counter, highlight, paused). The test replays the seven
  b2e rows (counter 3,3,4,5,5,5,5; `ui_state` 3,3,3,3,0,1,0; node vtables `0x4B2528`, `0x4B2528`, `0x4B3F38`,
  `0x4B3EB8`, then an empty list) and passes: 43 checks, 0 failures.
- **Implemented policy, not measured:** short ESC in pause defaults to `Refuse` (the preserved quirk); the b2e
  row is replayed with `LeavePause`. Level back to the map is `Unobserved` (no-op) with a `ToMap` switch
  (counter + 1, hypothesis). Long ESC always returns `Unobserved`. Menu RETURN acts only at highlight 0 (the
  preserved quirk; b2e only sent it at 0). Keys with no b2e data are ignored by assumption.
- **Hypothesis:** the counter starts at 2 (two startup constructions before the menu).
- **Not modelled:** the `+8` link, re-entering a level, and the cause of the short-ESC result.

## Facilitator check

Own run on mouv: `ctest -R frontend_sequence` passes, and the full suite passes 31/31. Isolated mutation
`++counter_;` to `counter_ += 2;` on mouv, rebuilt (a first attempt without a rebuild passed and was discarded as
invalid): the test fails; after restoring the file the blob hash equals the Mac copy and the test passes again.
Mac and mouv blobs of the four changed files are identical, all ASCII/LF. Read `Frontend.cpp` and `.h` in full.
No game or gdb process and no `LemWeb-*` task on mouv. Not verified: that the machine matches the original beyond
the seven observed rows (it cannot be, by construction).

## Consequence

B2 gate row stays unmet as a gate: the observed sequence is reproduced, but short ESC in pause, long ESC,
level-to-map and the `+8` link are unobserved, and the one short-ESC sample contradicts the preserved quirk. A
second start on a key path that reaches those would be needed; Q3 is at its cap and its start is spent.
