# B1s: one breakpoint address for the `^Font` source; arming before the load unproven

2026-10-23. Directive Q1 (B1 source, live read), offline preflight plus repair 1. Packet:
`build/tmp/b1s-font-live-preflight/` (`round-note.md`, `repair1-note.md`, `decision-table.md`,
`raw-pins.json`, `prediction.md`). Script: `tools/harness/trace_font_source.py` (**never run against a
process**). Zero stock starts, no debugger session, no REA.

## Result (READ, not observed execution)

- `0x47F230` returns `[ecx+8]`; `0x47E23B` is gated on it; the load site is `call 0x47E40D` at
  `0x45019E` after `push 0; push 1`. `[0x5D5610]` and the whole manager `0x5D5608` lie past `.data` raw
  size (`0x4C6000`): live-only, zero in the file.
- **One hardware execute breakpoint at `0x0047E4AD`** (first instruction of the archive-hit arm of
  `0x47E40D`) exposes all three values: request name `(char*)(*(u32*)(ebp-0x128)+0x14)`, matched entry
  `(char*)0x5D5734` (= `[ebp-0x18]+4`), archive `(char*)(*(u32*)([0x5D5614]+idx*4)+0x38)` with
  `idx=*(u32*)[ebp-0x18]`. The hit arm is silent on a loose-file open or a miss; fallback pair
  `{0x47E4AD, 0x47E463}` separates them.
- The `^Font` request is in the startup chain (`0x450A46`, arg 7). `0x486F60` has seven callers (checked
  against an independent scan): `0x450A46`, `0x45D432`, `0x45D4C4` (arg `0x1F`), `0x45D6AF`, `0x45D7EC`
  (`0x23`), `0x45DF25` (`0x21`), `0x45E0A9` (`0x20`). Whether the later calls reach `0x45019E` again is
  unread, and `0x414B90` is a cache lookup, so a later request may never reach the breakpoint.
- Every existing harness launcher does `Popen`, sleep, attach, so none can see startup. The repair added
  an opt-in `--launch` mode: GDB creates the process, `starti`, breakpoint inserted at the first
  instruction, continue. **A hypothesis; the stock game under it has not been run.**

## Facilitator check

Re-read from the exe on mouv: `0x47F230`, `0x47E23B`, `0x45019E` (`push 0; push 1; call 0x47e40d`),
`0x47E4A0..` (the arm and its `ebp-0x128` / `ebp-0x18` loads and `+0x104` / `+0x108` reads), `0x47E6A0..`
(`call 0x47f210`, `call 0x47f0b0`, `call 0x49db6a`), `0x47F210` (`[ecx+0xc]` list, index*4), `0x47F0B0`
(`add ecx,0x38`) and `0x406500` (identity), `0x47F9C8` (`mov eax,0x5d5730`). Own call scan: callers of
`0x486F60`, `0x486BD8`, `0x4403A7` equal the worker's lists. Selftest passes; the launch-guard
`first_start` disabled alone fails exactly that guard (original file hash unchanged); moving the
breakpoint address alone does **not** fail the selftest. GDB 17.2 is the mingw32 `gdb.exe` the harness
uses. On a throwaway test exe (not the game) I confirmed `starti` works on this GDB, a hardware breakpoint
set at the first instruction fires in a later-created thread and in the main thread, and
`delete` leaves no breakpoint listed; that says nothing about the stock game or a debugger check in it.
All four install hashes equal the startup record; no game, gdb or `LemWeb-*` task. Not verified: the GDB
command strings in launch mode, `starti` on the stock exe, `tasklist` in session 1, the gate value.

## Limits to carry into any live round

- `--stop-after` defaults to 4. The load probably makes two hits (`^FONT.BMP` and `.ALF`). On a timeout
  the hits are not copied into the report, only into `gdb-mi.log`.
- The attach mode's paused-level guard cannot see the startup load; only `--launch` can.
- The break at `0x47E4AD` fires only on an archive hit.
