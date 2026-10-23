# B2c: file-backed chain to the menu object read; key-to-event edge untouched

2026-10-23. B2 question round 3 of 3 (cap reached). Packet: `build/tmp/b2c-menu-chain/`
(`round-note.md`, `prediction.md`, `byte-manifest.json` with 22 pins, `derive_menu.py`,
`test_derive.py`). `tools/harness` untouched. Zero stock starts, no process.

## Result (READ, not observed execution)

**Edge A mostly closed; edge B not attempted.**

- Menu constructor `0x48EC90` has one caller, `0x487469`, inside the screen factory
  `0x48726F` case at `0x48744C` (allocates `0x5C` bytes). It writes vtable `0x4B2528` at
  `0x48ECB6` / `0x48ECCD`. The menu is created only for screen id 7.
- The factory tail makes each new screen the list head: `0x487C60` / `0x487C80` store
  `+0x24` / `+0x28`, `0x487CC0` stores the old head at `[obj+4]`, `0x487CA0` stores a link
  at `[oldhead+8]`, and `0x487FA0` writes `[0x5D59E4] = obj`.
- File-backed derivation: start at `[0x5D59E4]`, follow `+4`, take the single node whose
  first dword is `0x4B2528`. No heap constants.
- `[0x5D59E8]` has one writer (`0x486ECE`, stores its parameter) and is not guaranteed to be
  the menu. `[0x5D59EC]` is the chrome object (`0x4B1E88`), not the menu.
- `[menu+0x10]` as the dispatch container (B2b's candidate) is neither confirmed nor
  falsified: `0x4882C0` zeroes it, no writer found. `0x487C60` / `0x487C80` write `+0x24` /
  `+0x28`, so they are not its writers (the worker listed them as candidates).
- Predictions: P1 (`0x487180` is a screen switch) falsified; P2 (`0x486EC7` stores a fresh
  object) falsified as written; P3 (menu built in a screen-id switch, held in the list)
  confirmed. `prediction.md` (11:02:52) predates the first scan script (11:03:26).

## Not read

Edge B in full: the producer of the event number passed to `0x48896A`, the scancode-to-event
mapping, confirm, map, level entry, pause and back. Edge A: `0x488378` (destructor unlink),
the factory callers (`0x488713`, seven `0x486F60` callers), and the vcall `+4` target in
`0x4884F4`.

## Process notes

About 17 entry points counting stubs against a cap of 12 (12 non-trivial); the worker said
so in its note. About 12 of 40 minutes. No REA, capstone only. The note shows no mutation test of
`derive_menu.py`; I ran one (below). No "before" install snapshot of its own.

## Facilitator check

All 22 pins equal the exe. Re-read `0x48744C` (`push 0x5C`, call `0x48EC90`), the
constructor's vtable writes, `0x487FA0` (`mov [0x5D59E4], eax`), `0x487FB0`-style getter
`0x487F90` (`mov eax,[0x5D59E4]`), the link setters (`0x487CC0` writes `[eax+4]`, `0x487CA0`
writes `[eax+8]`) and `0x486EC7` (`mov [0x5D59E8], eax`). Callers of `0x48EC90`: one. The
12 offline controls pass; alone, `NEXT_OFF` 4 to 8 fails 5, and dropping the single-menu
check fails 3. Install hashes (exe, SavedGame.dat, dgVoodoo.conf, dinput.dll) re-hashed on mouv equal the startup record; no game process. Mac CTest 29/29 (stale build dir, not the 30 of earlier checks). I did not re-derive the
`0x48726F` id-7 switch or the list ordering at runtime.
