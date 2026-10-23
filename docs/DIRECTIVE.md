# Overlord directive

Written by the [overlord](OVERLORD.md); the user signs off what it authorizes. `/facilitator`
reads this file at startup, in any client. A directive with `Status: active` is the user's
authorization for the next standing run. Record it in `docs/tasks.json` yourself (new
`standing_run`, owner and date), then follow
[the standing run](ORCHESTRATION.md#standing-run). When its stop point is reached, set
`Status: done` here and stop.

Status: **done**
Issued: 2026-10-23 (user, overlord review 2026-10-23)
Supersedes: the texture-loader / B2 directive, done 2026-10-23 (Q1 parked at cap after B1p,
B1q, B1r; B2 parked at cap after B2a, B2b, B2c; zero stock starts used).

## Why live now

Six offline rounds each closed edges and opened as many. B1 narrowed to one conditional
source; B2 still lacks the key-to-event producer. Static reading has stopped converging on
both. This directive reads the remaining edges from the running original instead.

## Scope

Three questions, in order. Only one live owner at a time, so 1 runs before 3; 2 may run
offline alongside 3.

1. **B1 source, live read (one stock start, new question, cap two rounds).** Offline
   preflight first, from exe bytes: confirm the static gate `[0x5D5610]` (field `+8` of
   manager `0x5D5608`, read by `0x47F230`) and the load site `0x45019E` are what B1r says.
   Then one start in session 1, to the main menu: read `[0x5D5610]`, and with **one hardware
   breakpoint** at the `^Font` load record the resolved name, archive and entry the loader
   actually opens. Land when exactly one source is observed; otherwise park. Precedent and
   method: `tools/harness/trace_cell_emission.py` and the
   [emission check](scratch/phase-a1-cell-emission.md).
2. **B1 render and score (offline, up to two rounds; only if 1 observed exactly one
   source).** As the previous directive: main menu at highlight 0 from the read charmap,
   label table `0x4C4214`, Y = 320 + 29 x row, X = 320, the observed sheet, `UK.LTX`. Score
   full-frame against the B1b reference and the B1 gate.
3. **B2 capture, then implement (one stock start, up to three rounds).** Round 1: one start
   in session 1. Drive the gate's scripted key sequences (menu, confirm, map, into a level,
   back) through `tools/dinput-shim` and record, per step, the counter `0x004C416C` and the
   list head `[0x5D59E4]` (with its node vtables), including RETURN-only-at-highlight-0 and
   short-ESC-in-pause. Later rounds: implement our state machine offline to match.
   **Narrowed preflight (user decision):** validate from bytes only the static addresses
   the capture reads, and use the shim key path already exercised live in earlier phases.
   The key-to-event producer (`0x48896A`) does **not** have to be read statically first;
   the capture records the behavior. Still required: loaded-path, camera/pause and exact
   restoration checks, foreground via `acquire_game_foreground`.

## Budget

The grants of `d-playable-3`: per-round commits, up to three concurrent offline rounds, at
most one live owner. **Two stock starts total: one for question 1, one for question 3**, no
retry after Popen for either, not transferable between questions. Question 2 uses none.
Question 1 may use one hardware breakpoint and no other debugger probe.

## Decisions already made (do not re-ask)

- **Live reads authorized** for B1 source and B2 sequence as scoped above (user, overlord
  review 2026-10-23), including the one hardware-breakpoint probe in question 1.
- **Debugger rules:** delete the hardware breakpoint while the original is stopped, before
  detaching; stay within the target's hardware-breakpoint limit; require actual console
  completion markers; verify cleanup and the surviving process. A detach message alone is
  insufficient. Elevate (`-RunLevel Highest`) only for this probe.
- **Read, do not fit, and do not choose by score.** If question 1 observes no source or more
  than one, park; do not render candidates and pick the best scorer.
- Settled and not to be re-derived: everything settled in the previous directive, plus
  B1p/B1q/B1r (loader rule, archive order PATCH, LEMMINGS, CDPath; `^Font` request chain
  `0x4403A7` <- `0x486BD8` <- `0x486F60`) and B2a/B2b/B2c (menu built only by factory id 7;
  `[0x5D59E4]` newest screen, next `+4`, prev `+8`; `[0x5D59EC]` is chrome, not the menu;
  key-poll flag `[0x5D5970]`).
- `0x004C416C` is a construction counter, not an enum; matching it means matching
  construction order.
- Do not reuse heap addresses from an earlier process. Use `acquire_game_foreground`, never
  `goto_level.focus()`. Live steps run in session 1 per [MOUV_SSH.md](agents/MOUV_SSH.md).
- **Restoration:** restore and hash-check every install file in the same process that
  changed it.
- English menus are the reference; do not reset the install language.
- C1 stays held. Run CTest on mouv, not a stale Mac build dir.

## Stop point

Stop and report when every question is landed or parked (the B1 gate passing on your own
independent re-score counts as landed). Also stop on any stop condition in ORCHESTRATION,
or if a live start crashes or leaves the install unrestored. Do not advance to B3, C1, C2 or
C3 on this directive.

## Sync

Pull before selecting work and push after each round commit, per
[ROLES.md](ROLES.md#opencode-on-the-mac-clone). Run from the Mac clone
`~/progs/lemmings-webolution`. If a push from mouv fails on credentials, leave the commits
local and say so in the handoff; the overlord carries them to GitHub.
