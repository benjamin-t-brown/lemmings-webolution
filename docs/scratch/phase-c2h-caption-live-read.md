# C2h: the G+0x25C90 quad reads [277,422,363,454) live, in every paused snapshot

2026-10-23. Directive Q3 round 3 of 4: the one stock start (pid 21492, session 1, task `LemWeb-c2h-1`). Memory reads only
(`PROCESS_VM_READ`), no debugger, no hardware breakpoints. Worker note `build/tmp/c2h-caption-live-read/round-note.md`; dumps
`caption-inputs.json` and `raw/*.bin`.

## MEASURED (live read, `ui_state == 1` in all five snapshots: entry pause, after the 14 s walk, three repeats 0.5 s apart)

- Quad doubles at `G+0x25C90` (`+0x20/+0x88`, `+0x28/+0x160`, `+0xF0/+0x158`, `+0x90/+0xF8`): 277 / 422 / 363 / 454, exact integers,
  mirrors equal, bytes identical across all five snapshots.
- `[0x58CB40]` = 640, `[0x58CB44]` = 480 (P44 was only implied by the C2f backward solve; it is now a read).
- `[G+0x25C68+0x14]` = 285, `+0x18` = 422.
- `[G+0x26024]` 1 at entry, 0 after the walk; `[G+0x25FF8]` 3.405 then -1.005 (c2e's prediction that the `0x47AD8C` rect is off after the walk holds).

## DERIVED

`half(W)` = 320 - 285 = 35 and `H` = (480 - 422) / 2 = 29 by arithmetic on the measured values; W itself (70 or 71) is not recoverable.
`rect_from_quad` and `rect_from_placement` both give (277, 422, 363, 454).

## REFERENCE, not read

The A7 target `[277,422,363,454)`; the quad was compared to it, not fitted.

## Still unverified

That the quad doubles are final frame pixels. At 640x480 an identity transform gives the same numbers, so this read does not test the raster
step (`0x44DE60` / `0x46D460`). The panel colour and alpha path is unread. The worker's `judge` flagged `[1C968]` against `[25C64]/1000`
at 1e-12; the field is a float32-rounded 0.006, a tolerance error in the copied check, nothing refuted.

## Facilitator check

Re-decoded all five `quad_25c90.bin`, `cursor_obj_58cb40.bin` and `list_25c68.bin` dumps with my own struct unpack: quad (277, 422, 363, 454), mirrors
equal, P40/P44 (640, 480), placement (285, 422), quad bytes identical in all five. Swapping the x0/y0 offsets reads (422, 277, ...), so the match
is not insensitive to offsets. After the run, from a separate SSH call: exe, `SavedGame.dat` and `dgVoodoo.conf` SHA-256 equal `AUTHORITY`,
`SAVE_SHA`, `CONF_SHA`; no Lemmings process; no `LemWeb-*` task. Starts used: 1 of 1. Not verified: that the process ran stock without a shim
difference beyond the A7p path, the other 15 raw dumps beyond the five I decoded.
