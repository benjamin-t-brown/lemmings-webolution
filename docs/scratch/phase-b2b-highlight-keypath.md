# B2b: key poll read, highlight accessor shape read, menu chain not found

2026-10-23. B2 question round 2 of 3. Packet: `build/tmp/b2b-highlight-keypath/`
(`round-note.md`, `address-input-manifest.md`, `mutation-log.txt`, 22 pins). Helper:
`tools/harness/measure_b2_screen_sequence.py` (+33 lines, `candidate_highlight`, offline,
not called by `preflight()`). Zero stock starts.

## Result (READ, not observed execution)

**Partial; not closed.** The accessor cannot yet be derived from a file-backed global.

- `0x483212` (entered via `0x48316B`) scans 23 actions x 2 scancode slots (table
  `0x5D58B0`) against a byte array at `[this+8]` and sets `[0x5D5970]` = "any key newly
  pressed this poll". It is a flag, not a key identity. Writers: `0x483222` (0) and
  `0x4832B5` (1). Sole consumer is `0x488D10` via `0x48382C`.
- `0x48896A` is a shared handler (15 callers). Events 2..5 run a neighbour search through
  four wrappers and call `0x48F7A1(arg+0x0C, neighbour)`, which stores `[container+0x10]`
  (`0x48F80C`). That store is the only focus write found. Calling the events "directional"
  is an inference from shape; no key-to-event mapping is read.
- Candidate accessor: `rows.index([[menu+0x10]+0x10])`, rows at `menu+0x40+4i`, menu vtable
  `0x4B2528`. That the dispatcher's container is `[menu+0x10]` is unproved; no writer of
  `[menu+0x10]` was found.
- RETURN-at-highlight-0 still has no read path (`0x48ED89` calls `0x48896A` first).

## Missing edges (round 3 candidates)

A. Event producer: callers of `0x48316B` (`0x45D164`, `0x45D3FF`, `0x45D491`, `0x499E27`,
`0x49B171`), `0x483B5D`, `0x483E1A`.
B. Path to the menu object: `0x487180` (sole caller of the `[0x5D59EC]` setter), the
`[0x5D59E8]` writer `0x486ECE`, `0x487A08` / `0x488409` for list head `[0x5D59E4]`.

## Budget

About 17 distinct entry points against the brief's 12-body cap (the extras were four
wrappers and event-1 helpers); about 19 of 40 minutes. The overrun is recorded, not
excused; it does not change what was read. REA covered two bodies only, as a cross-check.
The worker also reported a `dis.py` stdlib shadow, a failed `sed -i` that left a stale pin,
and a wrong BSS classification of `0x4C36E4` (file-backed), all repaired.

## Facilitator check

Re-read `0x48F7A1` from the exe: the only `[reg+0x10]` stores are `0x48F7EE` (0) and
`0x48F80C`. Scanned `.text` for `[0x5D5970]` operands: stores at `0x483222` / `0x4832B5`
only. All 22 `.bin` pins equal the exe. Ran the helper's 8 controls (OK) and, alone,
removed the `focus in rows` check in a copy: the suite fails. `--preflight` still exits 2.
Install hashes equal startup; no game process. Not re-read: `0x483212` internals beyond the
two flag stores, `0x48896A` event routing, the wrappers, `0x489580`.
