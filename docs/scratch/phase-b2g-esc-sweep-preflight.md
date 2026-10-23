# B2g: ESC hold sweep scripted; the exe shows P and ESC as the same one-edge exit from pause

2026-10-23. Directive Q3 (second B2 capture) round 1 of 3, offline preflight. Packet:
`build/tmp/b2g-esc-sweep-preflight/` (`round-note.md`, `prediction.md`, `plan-output.json`, `selftest-output.txt`,
scan scripts). Script: `tools/harness/capture_b2_sequence.py` (**never run against a process**). Zero stock
starts, no input, no REA; about 7 of 12 bodies read.

## Result (READ, not observed execution)

- The two ESC plan steps became one `esc_sweep` step, holds `(2, 5, 10, 25, 40)`. P is sent only at `ui_state` 0
  (at 1 a skip is recorded, because P also leaves pause). ESC runs through the existing guard (needs `ui_state`
  exactly 1) and is never sent at 0. After each hold the script records chain and `ui_state`, and fails closed if
  `ui_state` is outside {0, 1}, the counter moved or the list is non-empty. Still opt-in via `--allow-esc`.
- **Hold length (READ, unverified live):** the main loop switches on `ui_state` at `0x45D1AC` (table `0x45E487`).
  In pause (case 1, `0x45D459`) action `0x13` (P) **or** `0xE` (ESC) stores `ui_state` 0 at `0x45D508`. The poll
  `0x483212` sets a per-action flag only when the key newly went down (flags at `0x5D5850 + 2*action`, read by
  `0x48348F`). Default key table: ESC = DIK `0x01` = action `0xE`, P = DIK `0x19` = action `0x13`. No hold-length
  test was found, B2e's hold-25 result agrees, and nothing in these bytes supports "a short ESC fails to leave
  pause". Not excluded: a 2-frame hold falling between polls; a config loader changing the table (unread).
  The quirk is **neither confirmed nor refuted**; the sweep is the test.
- **Level to map (partly read, no key named):** all writers of `ui_state` (`0x4CC134`) are in the main loop.
  ESC in a level (case 0) stores `ui_state` 2 (`0x45D40A`) and calls `0x486F60(.., 0x1F)` (`0x45D432`); O in pause
  does the same (`0x45D49C`). In case 2 a flag `[+0x55A2]` gives `ui_state` 3 and `0x486F60(.., 0x23)`
  (`0x45D606`, `0x45D6AF`). Unread: whether `0x23` is the map, the setter of `[+0x55A2]`, and the key or click that
  reaches the dialog's button handlers (`0x486510`, `0x48F558`).
- **`+8` on a non-head node:** no key path that stacks two nodes is named. The skip stays.

## Facilitator check

Re-read on the exe: `0x45D199..0x45D1AC` (switch), `0x45D40A`/`0x45D432` (ESC in level), `0x45D4B0..0x45D518`
(pause: action `0x13` via `0x48373D`, action `0xE` via `0x4837FA`, then the store of 0 at `0x45D508`), `0x45D226`
(`ui_state` 1). Selftest 23/23 on mouv. Mutation alone: the leave-the-level check disabled gives 22/23 and only
"the last hold leaving the level fails closed" fails; the real file hash is unchanged afterwards. The Mac clone had
a stale copy of the script; I copied mouv's, which is the file that ran the selftest. Not verified: the key table
as the active binding, live behaviour, and that the sweep's P and ESC reach the game.

## Consequence

Round 2 can run the sweep in one stock start. It cannot reach level-to-map without a named path; the one reachable
observation is the ESC-in-level dialog (`ui_state` 2, screen `0x1F`), which is a different state from the map.
