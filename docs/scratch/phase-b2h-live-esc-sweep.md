# B2h: ESC at holds 2/5/10/25/40 all leave pause; ESC in a level opens a one-node screen (counter +2)

2026-10-23. Directive Q3 (second B2 capture) round 2 of 3. Packet: `build/tmp/b2h-live-esc-sweep/`
(`round-note.md`, `prediction.md`, `report.json`, `raw/00..17-*.json`, driver and task script copies). One stock
start (Popen pid 38712, 20:10:58Z, session 1, one-shot task), spent; no retry. Script:
`tools/harness/capture_b2_sequence.py` (new `level-ESC-dialog` step behind `--allow-dialog`; selftest 35/35).

## Result (OBSERVED, one run)

| Step | `ui_state` `0x4CC134` | counter `0x4C416C` | list head `[0x5D59E4]` |
| --- | --- | --- | --- |
| menu / RETURN / RETURN | 3 / 3 / 3 | 3 / 4 / 5 | one node: `0x4B2528` / `0x4B3F38` / `0x4B3EB8` |
| map node click, P | 0, 1 | 5, 5 | empty |
| ESC hold 2, 5, 10, 25, 40 in pause (P before each) | **0** each | 5 | empty |
| ESC in the level (hold 20) | **2** | **7** | one node, vtable `0x4B26B0`, next 0, prev 0 |
| O | 1 | 7 | empty |
| P | 0 | 7 | empty |

- **Every ESC hold in pause left pause**, including hold 2. The B2g read (no hold-length dependence) held; its
  falsifier did not fire. One sample per hold, so a rare timing miss is not excluded.
- **The preserved quirk "a short ESC fails to leave pause" did not reproduce** in any of the five holds (nor in
  B2e's hold 25). Six samples, one process each; the cause of the original impression is unknown.
- ESC in a level gave `ui_state` 2 and a counter step of **+2** (5 to 7), not +1 as predicted: the exe counts
  two constructions for one ESC (which two is unread). The list was a single head node, `+4` and `+8` both 0.
  The identity of vtable `0x4B26B0` was not looked up; "the `0x1F` dialog" remains an inference.
- O in that state returned to pause (`ui_state` 1) and the list emptied; the counter stayed 7.
- The `+8` link on a non-head node is **still unobserved**; level-to-map is still not reached.

## Facilitator check

Re-read all eighteen `raw/*.json` with a separate script: counters (3,3,4,5,5 then 5 through the sweep, 7,7,7),
list heads (non-zero only at the first four steps and the dialog), and the node first dwords `0x4B2528`,
`0x4B3F38`, `0x4B3EB8`, `0x4B26B0` with next/prev 0 equal `report.json`. The report's `ui_state` series matches
the table. Exe, `SavedGame.dat`, `dgVoodoo.conf` and `dinput.dll` hashes equal the startup record, and the report
says `install_unchanged` and `writes_to_game` 0; a separate `Get-FileHash` I ran afterwards matches. No Lemmings or
gdb process and no `LemWeb-*` task afterwards (the driver terminated the pid it started). Selftest 35/35 on mouv.
Mutation alone: the "ESC only after the sweep" guard disabled gives 34/35, failing exactly that check; the real
file hash is unchanged. The worker records that three other new guards are redundant on the fake and are not
independently tested. Not verified by me: that each key reached the game as sent (taken from the `ui_state`
readings), and why the counter steps by 2.

## Consequence

The `Frontend` model can replace policy with measurement for ESC in pause (all five holds leave) and for the
level-ESC state (`ui_state` 2, counter +2, one node). Level-to-map, long-ESC beyond 40 frames, and `+8` stay
unobserved and remain labelled policy. The second stock start is spent.
