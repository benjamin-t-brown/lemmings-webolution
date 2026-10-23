# Overlord directive

Written by the [overlord](OVERLORD.md); the user signs off what it authorizes. `/facilitator`
reads this file at startup, in any client. A directive with `Status: active` is the user's
authorization for the next standing run. Record it in `docs/tasks.json` yourself (new
`standing_run`, owner and date), then follow
[the standing run](ORCHESTRATION.md#standing-run). When its stop point is reached, set
`Status: done` here and stop.

Status: **done** (2026-10-23: Q1 landed by b1v, one source observed; Q2 landed by b1w, B1 gate 0.005244 on the facilitator's re-score; Q3 parked, no named level-to-map path; one of two stock starts spent)
Issued: 2026-10-23 (user, overlord review 2026-10-23)
Supersedes: the B1 source / second B2 capture directive, done 2026-10-23 (B1t no hit, Q1
parked; B2g/B2h/B2i landed, short-ESC quirk retracted; both starts spent).

## Scope

Three questions. 1 and 3 each own the live game, so run them one after the other (either
order); 2 is offline and conditional on 1.

1. **B1 source, final live read (new question, cap two rounds: preflight, then one start).**
   Arm both arms of `0x47E40D` together, `{0x47E4AD, 0x47E463}` (archive hit and the
   fallback / loose side, per B1s's decision table), within the hardware-breakpoint limit.
   Address B1t's four unseparated causes in the preflight: show the breakpoints are live
   before the startup chain runs (or stop early enough to tell), that they reach the loading
   thread, and read the hit count before cleanup deletes them. Land when exactly one source
   is observed. **If this misses, park B1 for good** under this directive; record which
   causes the run excluded.
2. **B1 render and score (offline, up to two rounds; only if 1 observed exactly one
   source).** Main menu at highlight 0 from the read charmap, label table `0x4C4214`,
   Y = 320 + 29 x row, X = 320, the observed sheet, `UK.LTX`. Score full-frame against the
   B1b reference and the B1 gate.
3. **B2 third capture, then update `Frontend` (up to three rounds: preflight, one start,
   implement).** Name, from bytes, a key or click path from a level back to the map (B2g:
   all `ui_state` writers are in the main loop; no key named yet), and if one exists a path
   that stacks two screen nodes. Then one start recording counter `0x004C416C`, list head
   `[0x5D59E4]` with node vtables and `+4` / `+8`, and `ui_state` `0x4CC134` across level to
   map (and the stacked path, if named). If no path is named, record that and skip the
   start. Then update `src/shell/Frontend` and its test to replay the new rows; keep anything
   unobserved labelled as policy. Score the B2 gate row as now worded in `PLAN.md`.

## Budget

The grants of `d-playable-3`: per-round commits, up to three concurrent offline rounds, at
most one live owner. **Two stock starts: one for question 1, one for question 3**, no retry
after Popen, not transferable. Question 2 uses none. Question 1 may use the two hardware
breakpoints above, plus one GDB one-shot software int3 at the exe entry point `0x49D518`
(`--entry-stop soft`, removed by GDB on report) for its one start, and no other debugger
probe (amended, user, overlord review 2026-10-23, answering `q1-entry-int3` after B1u).

## Decisions already made (do not re-ask)

- Authorized as scoped above (user, overlord review 2026-10-23). B1 parks for good if
  question 1 misses.
- **`q1-entry-int3` answered: authorize the entry-point int3** (user, overlord review
  2026-10-23). If the stock exe does not tolerate it, that is a stop condition, not a
  retry. Q3 is parked (B2j, B2k: no named path); do not reopen it on this directive.
- **B2 gate row amended** (same review): the short-ESC clause is dropped, retracted after
  B2h. RETURN only at highlight 0 stays.
- **Time the pass** (same review, standing): see [FACILITATOR.md](FACILITATOR.md). The final
  report ends with total elapsed wall time and each round's dispatch-to-landing time.
- **Debugger rules:** delete the hardware breakpoints while the original is stopped, before
  detaching; stay within the hardware-breakpoint limit; require actual console completion
  markers; verify cleanup and the surviving or cleanly ended process. Close any game the
  round leaves running, after checking it, before the next live owner starts.
- **Read, do not fit, and do not choose by score.** No observed source or more than one: park.
- Settled and not to be re-derived: everything settled in the previous three directives,
  plus B1t (`starti` works; entry stop in SysWOW64 `ntdll`; site bytes at `0x47E4AD`; no hit
  in 90 s), B2g (P and ESC in pause both store `ui_state` 0 at `0x45D508`, no hold test) and
  B2h (ESC holds 2 to 40 all leave pause; ESC in a level gives `ui_state` 2, counter +2, one
  node vtable `0x4B26B0`; O returns to pause).
- `0x004C416C` is a construction counter, not an enum.
- Do not reuse heap addresses from an earlier process. Use `acquire_game_foreground`, never
  `goto_level.focus()`. Live steps run in session 1 per [MOUV_SSH.md](agents/MOUV_SSH.md).
- **Restoration:** restore and hash-check every install file in the same process that
  changed it.
- English menus are the reference; do not reset the install language.
- C1 stays held. Run CTest on mouv. Keep text files LF.

## Stop point

Stop and report when every question is landed or parked (the B1 gate passing on your own
independent re-score counts as landed). Also stop on any stop condition in ORCHESTRATION, or
if a live start crashes or leaves the install unrestored. Do not advance to B3, C1, C2 or C3
on this directive.

## Sync

Run from the Mac clone `~/progs/lemmings-webolution`. Pull before selecting work and push
after each round commit, per [ROLES.md](ROLES.md#opencode-on-the-mac-clone). If a push from
mouv fails on credentials, leave the commits local and say so in the handoff; the overlord
carries them to GitHub.
