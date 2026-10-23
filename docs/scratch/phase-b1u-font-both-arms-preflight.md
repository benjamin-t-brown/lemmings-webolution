# B1u: both arms armed and counted offline; an entry-point int3 is needed and is not authorized

2026-10-23. Directive Q1 round 1 (preflight). Packet: `build/tmp/b1u-font-both-arms-preflight/` (`round-note.md`,
`script.diff`, `selftest-result.txt`, `mutation-result.txt`, `launch-command-sequence.txt`, `throwaway/`,
`throwaway-runs/`). Script: `tools/harness/trace_font_source.py`. Zero stock starts, no debugger on the stock exe,
no REA, one native body (`0x47E40D`).

## Result

READ (stock exe): `0x47E463` is `mov eax,[ebp-0x128]; mov byte [eax+0x11d],0`, the loose/fallback side. Exe entry point
is `0x49D518` (`push ebp; mov ebp,esp; push -1`). **A third route executes neither arm:** `jne 0x47E598` at `0x47E43C`
when byte `[0x5D5604] != 0`. Neither arm firing therefore does not prove no request was made.

OBSERVED (throwaway 32-bit exe at base `0x400000`, same GDB 17.2, real script): hardware breakpoints set at the `starti`
stop **never fire in the main thread** and do fire in later-created threads (`archive=0 loose=3`). This reproduces B1t
cause (3) and means B1t was probably blind for that reason. Arming from a one-shot software stop at the entry point
fires in the main thread. Four real-script runs: both names matching completes; a name mismatch shows arrivals 2 against
0 hits (separates cause 4 from not-reached); a liveness address that is not executed refuses before arming; `starti`
mode reproduces the blind main thread.

Script now: both arms, no GDB condition (name filter in the command list, arrivals counted), liveness proven by a
throwaway breakpoint at `0x49D51B` before the arms, `-break-list` must show exactly two enabled `hw breakpoint` entries
with `times=0`, per-breakpoint `times` and `FONT_COUNTS` read before `-break-delete` on both the stop path and the
cleanup path. Also fixed: GDB 17.2 words an empty list differently from the old delete check.

## Not provable by any start

The archive-only route above; TLS callbacks before `0x49D518` (directory unread); that the stock exe tolerates the
int3 (never run); that `^Font` loads through `0x47E40D` at all (B1s left it not cleared); that the loader runs in the
main thread (later threads inherit the arms on the throwaway).

## Decision needed

`--entry-stop soft` (default) plants GDB's one-shot int3 at `0x49D518`; GDB removes it when it reports the stop.
Directive Q1 allows two hardware breakpoints "and no other debugger probe". `--entry-stop starti` is within the
directive but the throwaway shows it cannot reach the main thread. See `standing_run.pending_questions`.

## Facilitator check

Selftest on mouv: 81 PASS, 0 FAIL, 0 SKIP lines. Script hash on mouv equals the Mac copy (`bb5b52e6...`). Mutation alone
on a scratch copy (deleted afterwards): `ARM_ARCHIVE` 0x47E4AD to 0x47E4AE fails exactly "launch stub (a) exactly two
hardware breakpoints, at 0x47E4AD and 0x47E463"; the real file hash is unchanged. Re-read `0x49D518` (`55`, the
instruction at `0x49D51B` is `6a ff`) and `0x47E463` with `read_steppers.py`. Full ctest 31/31. No game, gdb or `LemWeb-*`
task, no scratch files left. Not verified by me: the throwaway runs themselves (taken from the saved logs), anything about
the stock exe under the int3.
