# Overlord directive

Written by the [overlord](OVERLORD.md); the user signs off what it authorizes. `/facilitator`
reads this file at startup, in any client. A directive with `Status: active` is the user's
authorization for the next standing run. Record it in `docs/tasks.json` yourself (new
`standing_run`, owner and date), then follow
[the standing run](ORCHESTRATION.md#standing-run). When its stop point is reached, set
`Status: done` here and stop.

Status: **active**
Issued: 2026-10-23 (user, overlord review 2026-10-23)
Supersedes: the live B1 source / B2 capture directive, done 2026-10-23 (B1s preflight landed,
Q1 live read parked on its round cap; B2d/B2e/B2f landed, one stock start spent, B2 gate
unmet). Answers that run's pending question `q1-live-read-round-cap`: **authorized**.

## Scope

Three questions. 1 and 3 each own the live game, so run them one after the other (1 first);
2 is offline and may run alongside 3.

1. **B1 source, live read (Q1 round 3, the explicit cap exception; one stock start).** Run
   `tools/harness/trace_font_source.py --launch` once, in session 1, with the one hardware
   execute breakpoint at `0x0047E4AD` as B1s read it. Before Popen, carry B1s's limits: set
   `--stop-after` from the expected hits, make sure a timeout still copies the hits into the
   report, and confirm the launch-mode GDB command strings offline. Land when exactly one
   source (name, entry, archive) is observed; otherwise park. If the original resists the
   debugger or crashes, stop and report; do not retry.
2. **B1 render and score (offline, up to two rounds; only if 1 observed exactly one
   source).** Main menu at highlight 0 from the read charmap, label table `0x4C4214`,
   Y = 320 + 29 x row, X = 320, the observed sheet, `UK.LTX`. Score full-frame against the
   B1b reference and the B1 gate.
3. **B2 second capture, then update `Frontend` (one stock start, up to three rounds).**
   Offline preflight, then one start covering, in one process:
   - an **ESC hold-length sweep in pause**: re-enter pause with P before each hold and record
     `ui_state` (`0x004CC134`) after each, over at least holds 2, 5, 10, 25 and 40 frames.
     This measures what "short ESC" means. Do not promote or drop the quirk without it;
   - **level back to the map**: the counter and list head across that transition;
   - a key path that **stacks two screen nodes**, if B2e's data or a byte read names one, to
     observe the `+8` link; if none is known, record that and skip it.
   Then update `src/shell/Frontend` and its test offline to replay the new rows. Replace
   policy with measurement where the capture settles it; keep anything still unobserved
   labelled as policy.

## Budget

The grants of `d-playable-3`: per-round commits, up to three concurrent offline rounds, at
most one live owner. **Two stock starts: one for question 1, one for question 3**, no retry
after Popen, not transferable. Question 2 uses none. Question 1 may use one hardware
breakpoint and no other debugger probe. Q1 gets this one round only; Q3 gets three new rounds
(preflight, capture, implement).

## Decisions already made (do not re-ask)

- Q1 live read authorized as round 3 (user, overlord review 2026-10-23). A second B2 start
  authorized for the scope above (same review).
- **Debugger rules:** delete the hardware breakpoint while the original is stopped, before
  detaching; stay within the hardware-breakpoint limit; require actual console completion
  markers; verify cleanup and the surviving or cleanly ended process. A detach message alone is
  insufficient. Elevate only for this probe.
- **Read, do not fit, and do not choose by score.** No observed source or more than one: park.
- **The short-ESC quirk:** B2e's one sample (hold 25) left pause. Treat it as one observation,
  not a refutation; the sweep decides. Until then `Frontend` keeps the quirk as its default.
- Settled and not to be re-derived: everything settled in the previous two directives, plus
  B1s (breakpoint `0x0047E4AD` and its three reads; manager `0x5D5608` is runtime-only, zero
  in the file; seven callers of `0x486F60`) and B2e (counter 3 / 4 / 5 at menu / confirm /
  map; single-node list, replaced not stacked; head 0 inside a level; vtables `0x4B2528`,
  `0x4B3F38`, `0x4B3EB8`).
- `0x004C416C` is a construction counter, not an enum.
- Do not reuse heap addresses from an earlier process. Use `acquire_game_foreground`, never
  `goto_level.focus()`. Live steps run in session 1 per [MOUV_SSH.md](agents/MOUV_SSH.md).
- **Restoration:** restore and hash-check every install file in the same process that
  changed it.
- English menus are the reference; do not reset the install language.
- C1 stays held. Run CTest on mouv.

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
