# C3h: scene matrix stack bottom 0x58D5A0, offline preflight (Q2 round 1 of 2)

2026-10-23. Directive 943cd7e Q2. Worker note `build/tmp/c3h-stack-bottom-preflight/round-note.md`; `read_stack_bottom.py`, `run_live.ps1`, `callers.out`, `scan_abs.out`. REA not used; no game started, no debugger, no `src/` change. The script has **not** been run against a process.

## Result

| Value | Tag | Address |
| --- | --- | --- |
| stack object `0x58D5A0` = `{count, cap, grow, data}`, elements 0x60 B (12 doubles); ctor `0x471A30` stores 0, 0, 1, 0 | READ | `0x471A3A..0x471A57`, `0x43BE50`, `0x43BE70` |
| nothing seeds element 0 at load: 7 operand hits of `0x58D5A0` in all of `.text` (ctor/dtor, push, pop, top) | READ (literal-operand scan) | `scan_abs.out`, `callers.out` |
| element 0 = per-frame push of the global `[0x58D3D8]` at `0x45DD7B` (`push 0x58d3d8; call 0x45fd90` -> `0x45fdb0` -> `0x43bcf0`), popped at `0x45E347` | READ | re-read on mouv |
| element 1 = second push of `[0x58D3D8]` at `0x474979` inside `0x47496D` | READ | re-read on mouv |
| pop `0x43BBE0` -> `0x43BBF0` -> `0x43C300`: for `n <= cap` it jumps to `0x43C45A`, whose only effect is `[this] = n`; pop copies nothing | READ | `0x43C326..0x43C329`, `0x43C45A..0x43C460` re-read |
| the paused frame (`[+0x572C] == 1`) draws the pause UI then reaches the same pop `0x45E347` | READ | `0x45E31C..0x45E347` re-read |
| `[0x58CE24]` is a current-object `this` pointer, not a stack pointer; default `0x4C6038`, repointed by `0x47496D` to `holder+0x34`; six writers named | READ | `scan_abs.out`, `0x474993` |
| `[0x58D3D8]` is non-identity in the four A1 fixtures | CITED from the fixtures, not re-read | `tools/harness/fixtures/camera_a1/reference-repeat-state.json` |
| writer of `[0x58D3D8]` (probably a member of the `0x58D23C` object), whether it is rewritten between the push and the scene draw, whether the scene call `0x45E0E6` runs in the paused frame, `[node+0x110]`, `0x420BA0` | UNREAD | |

So the brief's "identity-like or per-frame view matrix" is half right: per-frame camera matrix at the push site, not identity.

## Correction flagged, not edited

`docs/scratch/phase-a4-pre-actor-subcalls-static.md:78` says the pop copies 0x40 bytes back into `0x58D3D8`, and cites `[0x58D59C]`. The pop path read here copies nothing, and the facilitator re-read `0x43C326`/`0x43C45A` to confirm. `[0x58D59C]` was found nowhere in `.text` by the scan. Treat that line as superseded by this note.

## The live read (next round)

`read_stack_bottom.py --live`, memory-only handle (`PROCESS_VM_READ|QUERY` = `0x0410`), reads the header, every live element plus up to 8 retained slots, `[0x58D3D8]`, `[0x58CE24]` and its holder window, twice per window with a 2 s gap, at `entry_pause` and `after_walk`. Stable-versus-stale test: live element 0 equals `[0x58D3D8]` read in the same snapshot in at least 95% of live observations; across snapshots, if `[0x58D3D8]` changes element 0 must follow it; if it does not change the script reports `INCONCLUSIVE`, expected for a static paused camera. Predictions P1-P8 and their falsifiers are in the worker note. Limit: if no snapshot lands inside the scene walk, element 1 is `INCONCLUSIVE`; element 0 still stands. One start, session 1, via `run_live.ps1` (task `LemWeb-c3h-1`).

## Facilitator check

Re-read on mouv: `0x45DD7B` and `0x474979` (both `push 0x58d3d8`), `0x45FD90`/`0x45FDB0` (`mov ecx, 0x58d5a0`, `call 0x43bcf0`), `0x43BBE0` (`mov ecx, 0x58d5a0`), `0x43C326`/`0x43C329`/`0x43C45A..0x43C460`, `0x45E31C..0x45E347`, `0x474993` (`[0x58ce24] = holder+0x34`). Selftest 40/40 on mouv. Mutate-one-alone on a `/tmp` copy: moving the `data` field offset from `+0xC` to `+0x8` fails the field-offsets check; moving `BASE_MAT` to `0x58D3E0` fails the base-matrix address check; each leaves 38 passes and the copy's sha1 restored. Narrowed: the selftest checks the script against its own constants and synthetic memory, so it protects the addresses and shapes, not the live behaviour; the writer scan (`scan_abs.py`) and the 26-hit `[0x58CE24]` list were not independently re-run by me; the A1 non-identity values were not re-read; the worker read about 14 bodies against a cap of 10 (named in its note).
