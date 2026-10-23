# B2i: Frontend replays the b2e and b2h captures (offline)

2026-10-23. Directive Q3 (second B2 capture) round 3 of 3, implementation. No stock start, no debugger.
Packet: `build/tmp/b2i-frontend-measured/` (`round-note.md`, `prediction.md`). Changed:
`src/shell/Frontend.h`, `src/shell/Frontend.cpp`, `src/__test__/test_frontend_sequence.cpp`.

## What changed

- `Key` is `{Return, Esc, P, O, MapNodeClick}`; ESC carries `holdFrames` (default 20, the level-ESC sample).
- ESC in pause leaves pause at holds 2..40 (OBSERVED at 2, 5, 10, 25, 40; the range between is POLICY). Holds
  outside it return `Outcome::Unobserved` with state unchanged.
- ESC in an unpaused level at hold 20: `Screen::Dialog`, `ui_state` 2, counter +2, node vtable `0x4B26B0`
  (OBSERVED b2h). `O` there returns to the level paused (OBSERVED).
- `ShortEscPolicy` deleted: the short-ESC quirk is retracted. `Policy` keeps `levelExit` only.
- `leaveLevel()` stays `Unobserved` by default. Level-to-map, ESC beyond 40 frames, ESC/P/RETURN at the dialog,
  and the `+8` link remain unobserved and labelled.
- `Outcome::Ignored` now means only the preserved RETURN-at-highlight-not-0 quirk; other non-matching keys on
  menu / confirm / map became `Unobserved`.

## Facilitator check

Read all three files on mouv. `ctest -R frontend_sequence` 1/1 (109 checks), full suite 31/31 on mouv after
the mutations. I mutated one constant at a time in `Frontend.h`, rebuilt, ran, and restored by hash:
`kLevelEscConstructions` 2 to 1, `kEscHoldMax` 40 to 30 and `kEscHoldMin` 2 to 3 each fail the test
(a first attempt on `kEscHoldLevel` 20 to 25 passed, as the test uses the symbol on both sides, so that
constant is not test-protected). The worker's own six mutations (pause ESC refused, dialog vtable, O bumping
the counter, ESC at dialog advancing) are reported but I did not rerun them. My text-mode rewrite left
`Frontend.h` CRLF; converted back, `git ls-files --eol` shows `i/lf w/lf` for all four touched files.
`Frontend.cpp` is not in `cmake/webolution_sources.cmake`. No `._*`/`.tmp*` files; no game process and
no `LemWeb-*` task on mouv. Narrowed: the non-matching-keys `Ignored` to `Unobserved` change has no
dedicated test; the model reproduces single-sample captures, not a causal account of the +2.
