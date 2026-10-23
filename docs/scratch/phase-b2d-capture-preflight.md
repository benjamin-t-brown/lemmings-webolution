# B2d: capture preflight read; key path covers menu, confirm, map, level entry and P-pause only

2026-10-23. Directive Q3 (B2 capture) round 1 of 3, the offline preflight. Packet:
`build/tmp/b2d-capture-preflight/` (`round-note.md`, `prediction.md`, `pins.py`, `pins-output.json`,
`aslr.py`, `selftest-output.txt`, hashes before/after). Script: `tools/harness/capture_b2_sequence.py`
(written, **never run against a process**). Zero stock starts, no input sent, no REA, 0 of 12 bodies.

## Result (READ, not observed execution)

- **File-backed** (`.data` raw ends `0x004C6000`): the counter `0x004C416C` (initial dword `1`), menu
  vtable `0x004B2528`, chrome vtable `0x004B1E88`. **Runtime-only**: list head `[0x5D59E4]`, `0x5D59E8`,
  `0x5D59EC`, key-poll flag `0x5D5970`, `ui_state` `0x004CC134`, path buffer, actor list. The worker's
  prediction that the counter was runtime-only was falsified.
- The exe has no relocation directory and `DllCharacteristics` 0, so static addresses match across
  processes. Only heap node pointers vary; the script derives them from the head on every run.
- **Key path** (`lrinput.LRInput` over `tools/dinput-shim`) is covered, by docstring and call
  pattern, for menu highlight to 0, the two RETURNs, the counter read, the map node click, loaded
  path, and P-pause. **Uncovered**: short ESC in pause, long ESC leaving pause, level back to the map
  (`dump_grids.py` relaunches instead), and any read of the `[0x5D59E4]` chain.
- The script records counter, `ui_state` and the chain (head, `+4`, `+8`, first-dword vtable) per step.
  RETURN only at highlight 0; P/ESC refused outside a level or pause; ESC steps opt-in; exact-hash and
  foreground guards. The `expect_counter` fields are recorded beside the observation, not asserted.

## Facilitator check

Own PE parse on mouv (not the worker's `pins.py`): exe SHA-256 equals the startup record, `DllCharacteristics`
0, relocation directory (0, 0), and the section mapping matches for `0x4C416C` (file, 1), `0x4B2528`,
`0x4B1E88` (file), `0x5D59E4` and `0x5D5970` (bss); `0x4C5FFC` is file-backed and `0x4C6000` is bss, so the
boundary is exact. `rg` over `tools/` finds no other reader of `0x5D59E4` and no script sending ESC in a
level (only `skip_intro.py` in the intro). Selftest 14/14. Mutation alone: next offset 4 to 8 gives 7/14
and exit 1; replacing only the RETURN highlight guard gives 13/14 (the highlight test fails), and the
original file hash is unchanged afterwards. dinput.dll hash and all four install hashes equal the worker's
before and after. No game process on mouv, no `LemWeb-*` task. Not verified: the live helpers (`aim_at`,
`find_menu_index`, foreground), the `+8` prev link, any real chain. The selftest images are the worker's own.

## Consequence for the live round

The capture can record menu, confirm, map, level entry and pause. ESC and level-to-map have no proven key
path, so those steps stay opt-in and the live round must say whether it risks them; the worker's own
docstring evidence says a long ESC after a level can quit the process.
