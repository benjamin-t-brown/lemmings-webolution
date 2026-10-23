# B1v: the ^Font source observed live, exactly one (archive)

2026-10-23. Directive Q1 round 2 (final). Packet: `build/tmp/b1v-font-live-read/` (`round-note.md`, `prediction.md`,
`checklist.md`, `trace-result.json`, `gdb-mi.log`, `launch-task.ps1.txt`, `run/`). Script `tools/harness/trace_font_source.py`
unchanged (sha256 `02692211...6da6fde`). One stock start, spent at Popen 16:37:13 MDT; no REA.

## Result

OBSERVED (stock exe, `--launch`, `--entry-stop soft`, `--stop-after 2`): the exe tolerated the one-shot int3 at `0x49D518`;
the liveness breakpoint at `0x49D51B` fired in the main thread; `-break-list` before the first continue showed exactly two
enabled `hw breakpoint` entries (`0x47E4AD`, `0x47E463`), `times=0`. Two `^Font` hits, both on the archive arm `0x47E4AD`,
gate `[0x5D5610]`=1, flag `[0x5D5604]`=0, archive object index 1:

| n | request | matched entry | size `+0x104` | offset `+0x108` |
|---|---|---|---|---|
| 1 | `Textures\^Font.bmp` | `TEXTURES\^FONT.BMP` | 77 | 44312738 |
| 2 | `Textures\^Font.alf` | `TEXTURES\^FONT.ALF` | 61440 | 44251298 |

Archive path stored by the exe for both: `C:\Users\bbrown\Downloads\Lemmings Revolution\LEMMINGS.BOX`. `FONT_COUNTS archive=121
loose=7 hits=2`; the 7 loose-arm arrivals were `c:\Lemmings.log`, `PATCH.BOX` x2, `LEMMINGS.BOX` x3, `SavedGame.dat`; none
`^Font`. Cleanup: breakpoints deleted before detach, `completed` true, gdb exit 0, install hashes identical before and after.

One source: archive `LEMMINGS.BOX`, members `TEXTURES\^FONT.BMP` and `.ALF`, zero loose. Q2 may proceed.

## Excluded and not

Excluded from B1t's four causes: main-thread blindness (arms fired in the main thread) and name-filter mismatch (arrivals and
hits counted apart). **Not settled:** the run stopped at hit 2, so "exactly one" holds up to that point and a later third
source is not excluded; TLS callbacks before `0x49D518` unread; the archive-only route `0x47E598` not taken (flag 0); that no
loose `^Font` exists in the install was not tested; whether the `Downloads` path is the install copy (B1q: the `CDPath`
copy is byte-identical, so the members are the same bytes either way).

## Facilitator check

Re-read on the exe with `read_steppers.py`: `0x47E4AD`, `0x47E463`, `0x49D518`/`0x49D51B`. Independent index read (`tools/proto/box.py`,
not the worker's script): install `lemmings.box` holds `TEXTURES\^FONT.BMP` at ordinal 633 (offset 44312738, size 77) and
`.ALF` at 632 (44251298, 61440), exactly the observed pairs; `patch.box` has neither. Raw `gdb-mi.log` carries both `FONT_HIT`
lines and `FONT_COUNTS archive=121 loose=7 hits=2`. Selftest on mouv 81 PASS, no FAIL/SKIP. Install hashes re-hashed on mouv
equal the script's table; no game, gdb or `LemWeb-*` task remains. Mutation alone: not repeated, since the script is unchanged
since b1u's ARM_ARCHIVE mutation. The worker also set `stock_starts_consumed` to 1 in its own entry; that is correct and kept.
