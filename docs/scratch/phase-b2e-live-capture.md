# B2e: live frontend capture, counter 3/4/5 observed; short ESC left pause; chain never had two nodes

2026-10-23. Directive Q3 (B2 capture) round 2 of 3. Packet: `build/tmp/b2e-live-capture/` (`round-note.md`,
`prediction.md`, `report.json`, `raw/00..06-*.json`, hashes, driver and task scripts). One stock start
(Popen 19:20:27Z, pid 47320, session 1, one-shot task), spent; no retry. Script: `tools/harness/capture_b2_sequence.py`.

## Result (OBSERVED, one run, 7 of 8 steps)

| Step | counter `0x4C416C` | `ui_state` | list head `[0x5D59E4]` | node vtable |
| --- | --- | --- | --- | --- |
| baseline | 3 | 3 | 1 node | `0x4B2528` (menu) |
| highlight 0 | 3 | 3 | same as baseline | `0x4B2528` |
| RETURN | 4 | 3 | 1 node | `0x4B3F38` |
| RETURN | 5 | 3 | 1 node | `0x4B3EB8` |
| map node click (loaded `Compiled\MM_CLIMB_002.lvl`) | 5 | 0 | 0 (empty) | none |
| P | 5 | 1 | 0 | none |
| short ESC (hold 25) | 5 | **0** | 0 | none |
| long ESC (hold 40) | not sent | | | refused by guard |

- The counter sequence 3 / 4 / 5 at menu / confirm / map is now observed (it was a PLAN recon read).
- Each frontend screen was the only node: the list is replaced, not stacked. Inside a level the head is 0.
- The two baselines in the run agree (counter, head, node, vtable).
- **Short ESC left pause on this run** (`ui_state` 1 to 0), against the preserved quirk. One sample on one key
  path; the cause (hold length, focus, timing, or `ui_state` 0 meaning something else) is **not known**. The
  quirk is neither confirmed nor refuted: do not promote either way.
- Long ESC untested: the script's own guard refused ESC at `ui_state` 0 (`FAIL_CLOSED: refusing ESCAPE:
  ui_state 0 is not pause (1)`).
- The `+8` prev link on a non-head node is **unsettled**: every chain had one node.
- The vtables `0x4B3F38` and `0x4B3EB8` are observed addresses; what they are was not looked up.

## Facilitator check

Re-read all seven `raw/*.json` with a separate script (not the worker's): the counter dwords (3,3,4,5,5,5,5) and
the node first dwords (`0x4B2528`, `0x4B2528`, `0x4B3F38`, `0x4B3EB8`) equal `report.json`; head words are 0 for
the last three steps; step 4's path read is the ASCII `Compiled\MM_CLIMB_002.lvl`. Exe, `SavedGame.dat`,
`dgVoodoo.conf` and `dinput.dll` hashes are equal before and after in the worker's files, and the exe hash equals
the startup record. After the run, mouv has no Lemmings process and no `LemWeb-*` task. Script selftest 15/15; an
isolated mutation of the ESC guard (`!= 1` to `!= 0`) makes the selftest raise, so that guard is covered. Mac and
mouv copies of the script are byte-identical. Not verified by me: the live key delivery itself (taken from the
worker's report), and anything about why short ESC left pause.

## Consequence

Round 3 can implement the 3 / 4 / 5 construction-counter sequence and single-node screen replacement. It cannot
settle short or long ESC in pause, level-to-map, or the `+8` link; those need a second start Q3 does not grant.
