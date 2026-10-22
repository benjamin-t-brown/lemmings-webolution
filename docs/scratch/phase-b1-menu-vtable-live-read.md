# B1d: boot-menu vtables read live

Queue id `b1d-menu-vtable-live-read`. Original brief:
`build/tmp/b1d-menu-vtable-live-read-brief.md`; continuation brief:
`build/tmp/b1d-menu-vtable-live-read-continuation-brief.md`. Worker packet:
`build/tmp/b1d-menu-vtable-live-read/round-note.md`.

The first worker stopped at a session limit before launching, as the user confirmed.
Its prediction and script were reused, not rewritten. OpenCode continuation
`ses_e9ad7da41ffeqrumfySWhR0HGb` used the sole unused stock start. Two dispatches count
against this question's cap; only one stock process was started.

## MEASURED result

On PID 36132, with no game input, three polls one second apart returned identical data:

| Read | Observed value |
| --- | --- |
| Screen-id counter `0x004C416C` | 3 |
| Active pointer `[0x005D59EC]` | `0x02789B40` |
| First dword of active object | `0x004B1E88` (B1c chrome candidate) |
| List head `[0x005D59E4]` | `0x0278A590` |
| First dword of observed head node | `0x004B2528` (B1c ctor#15 content candidate) |
| Recorded nodes / cap reached | 1 / no (cap 32) |

This selects ctor#15 from B1c's seven candidates **for the observed boot-menu list
node**. It supports the chrome/content split, but neither proves what that object's
draw routine renders nor establishes the list's role on other screens. The saved
prediction's specific first-choice guess, ctor#5 (`0x4B1F88`), was wrong; its broader
chrome-plus-seven-candidate structural prediction was supported.

Raw data: `live-read.json`; logs: `run.stdout.txt`, `run.stderr.txt` (empty),
`round-note-log.txt`; integrity: `preflight.json`, `postflight.json` and
`SavedGame.dat.pre-run.backup`, all in the packet directory.

## READ method and reproduction

From the repository root, with normal Windows Python and the existing harness imports
and `build/tmp/a1-zoom-elevation/dgVoodoo.windowed.conf` available:

```text
python build/tmp/b1d-menu-vtable-live-read/read_menu_vtables.py
```

**This launches the original**: a future run needs its own authorized budget and live
ownership. The script makes one Popen, temporarily swaps/restores the wrapper config,
uses the masked foreground-acquisition helper, polls memory and terminates its owned
process. It uses no debugger, hook or injected code. REA was not used; the accessor
bytes were checked with Capstone and an independent raw-PE reader.

The original getters read `[0x5D59E4]` (`0x487F93: a1 e4 59 5d 00`) and
`[0x5D59EC]` (`0x487FB3: a1 ec 59 5d 00`); the next accessor reads object+4
(`0x487BCA: 8b 40 04`). These globals are runtime/BSS data, not file-initialized
pointer values.

## Facilitator check

Independently re-parsed the saved JSON with a separate checker
(`build/tmp/b1d-facilitator-check/verify.py`, no imports from the worker poll for its
headline computation): all three counter/pointer/vtable rows agree, the observed node
is in the seven-candidate set, and the cap was not reached. Accepted this one capture
under the original brief's explicit single-clean-capture option; no second live run.

Re-read all three complete accessor functions against the authority with
`read_steppers.py` at `0x487F90`, `0x487FB0` and `0x487BC0`. Independently parsed the
PE section table and decoded both observed eight-slot vtables from raw bytes; each
contains code pointers and slot 3 is B1c's shared `0x4886E4`. Both tables and the three
accessor operands match. This checks the memory-reading interpretation and candidate
classification, not a newly traced draw path.

**Narrowing:** the worker said the list ended at null and was therefore complete.
The saved poll records no terminal next-pointer value and `lrstate.u32()` returns
`None` on a failed read. Its `while node` exits identically for `None` and zero.
An offline mock changing only the final next read from zero to `None` produced exactly
the same report. Therefore only **one observed node** is established, not independently
proven null termination or complete-list cardinality. This does not invalidate either
positive vtable read; no extra live start was spent to remove this non-load-bearing limit.

The saved prediction/script SHA-256 values match the packet:
`0e7938a15f2a206519ccc80bf851eec3bf46a1bbbb917ee82679b6b8c047c91e` /
`7bd98a50ed7564159745946052c8dcd3a66e349c3f4a4cc4fb47a23e32303c74`.
Independently re-hashed the current install against both the worker's before/after data
and this facilitator's pre-dispatch reads:

- Exe: `32675bdb49866e76502713251fdfd2b09078c2481c72ca22ff68ebf4b3737a2f`.
- Config: `d639e220814d80b7956b96bad0ca708b768458bd348bfd607afbf1913598e72b`.
- Save: `f7f8603928890fb13d022d828c8a602cf7a7619ab24de82c7de4ae86ec337e6a`;
  direct byte comparison to the exact pre-run save backup also passes. No save restore
  was needed. Config swap/restore is the only reported install write.

Own process/task checks found no Lemmings/dgVoodoo process or LemWeb task after release.
Fresh build through `Invoke-Ucrt64.ps1` had no work; full CTest passed **29/29**. No
production implementation changed, so no production behavioural/pixel mutation applies
to this measurement-only round. Pre-existing framework edits were excluded from landing.

B1 remains open: no main-menu render or full-frame score exists yet. The next bounded
native read can start at the now-observed `0x4B2528` and chrome `0x4B1E88` interfaces
to locate actual draw submission/layout rather than repeat the seven-candidate search.
Font metrics remain unlocated. The language question is resolved separately by the
user: German was a harness mistake; accept the English reference and do not change
the install's language.
