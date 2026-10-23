# C2b: indices 0x44 and 0x45 are Out and Saved

Queue `c2b-pause-index-text`. Offline disassembly read, round 2 of the
pause-caption question. C2 groundwork only. No stock start. Worker note
`build/tmp/c2b-pause-index-text/round-note.md`.

## Result

The file dword at `0x4C3434` is 2. Slot 2 of the five language names is `Uk`
at `0x4C3470`. The format at `0x4C3484` is `Language\%s.ltx`, so the loader
asks for `Language\Uk.ltx`.

Both archive copies of `LANGUAGE\UK.LTX` are in range at index `0x44` and
`0x45`:

| Index | `lemmings.box` | `patch.box` |
| --- | --- | --- |
| `0x44` | `Out` | `Out` |
| `0x45` | `Saved` | `Saved` |

The exe image itself still has only `"Unknown String"` at `0x4C3450`, the
out-of-range fallback. These two records are the Out and Saved counters, not
the word `Paused`. A later store at `0x48674A` can replace the language
slot; that argument was not followed. The file image selects Uk.

## Facilitator check

An independent LEMBOX walk, using the layout in `src/assets/Box.h` and not
the worker's extract, read both `LANGUAGE\UK.LTX` payloads from
`C:\progs\Lemmings Revolution`. `lemmings.box` entry 765 (2978 bytes, count
130) and `patch.box` entry 5 (5297 bytes, count 232) both store `Out` at
index 68 and `Saved` at index 69. The same walk reproduces the five menu
labels at 125, 126, 120, 3, and 43. File bytes `02 00 00 00` at `0x4C3434`,
`Uk` at `0x4C3470`, `German` at `0x4C3460`, and `Language\%s.ltx` at
`0x4C3484` match.

No production file changed. The suite run with the B1k check was 30/30 on
this unchanged tree. No Lemmings process. The C2 gate was not attempted.
