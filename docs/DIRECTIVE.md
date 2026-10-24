# Overlord directive

Written by the [overlord](OVERLORD.md); the user signs off what it authorizes. `/facilitator`
reads this file at startup, in any client. A directive with `Status: active` is the user's
authorization for the next standing run. Record it in `docs/tasks.json` yourself (new
`standing_run`, owner and date), then follow
[the standing run](ORCHESTRATION.md#standing-run). When its stop point is reached, set
`Status: done` here and stop.

Status: **active**
Issued: 2026-10-23 (user, overlord review 2026-10-23)
Supersedes: the final B1 read / B2 level-to-map directive, done 2026-10-23 (B1v one source,
B1w B1 gate 0.005244 on the facilitator's re-score, Q3 parked, one of two starts spent).

## Scope

Three questions. 1 and 2 are offline and may run together; 3 owns the live game.

1. **B1 panel read (offline, cap two rounds).** B1w took the five menu panel rectangles and
   colours from the scored reference; the panel draw (code after `0x48B1C7`) is unopened.
   Read it: rectangle rule, colours or texture, border, and the glyph blend over it if it
   is on the same path. Replace every measured-from-reference value that the read settles,
   re-render, and re-score full-frame against the B1b reference. Whatever the score, report
   it; do not tune. If it fails, B1 goes back to **in progress**. If the draw is not read in
   two rounds, B1 stays accepted on the condition below and the panels stay labelled
   measured.
2. **C2 counters and timer (offline, cap three rounds).** Draw `Out` / `Saved` (indices
   `0x44` / `0x45`, C2a/C2b positions 520,15 and 520,45) with their counts, and the timer
   (`[0x4CBAAC] / 60`, MM:SS formats at `0x4C01DC` / `0x4C01E8`, at 560,430), using the
   `^Font` path B1w built. Read, not measure, the anchor, colour and where the counts come
   from. Score the A7 nine views and the second level as A7p/A7q do: the amended A7 gate
   must still pass, and report the counter and timer regions' residual before and after.
3. **C2 Paused caption (cap three rounds: preflight, one start, render).** Resume the A7
   caption question at its recorded point: read `[G+0x25C64]` live at pause (A7k/A7l: the
   caption top edge scales with it and it is not a file constant), plus whatever else the
   preflight shows the top edge needs. Then render the caption `[277,422,363,454)` and
   re-score A7 with the caption exclusion removed. If the preflight cannot name what to read,
   skip the start and park.

## Budget

Per-round commits, up to three concurrent offline rounds, at most one live owner. **One
stock start, for question 3 only**, no retry after Popen. Questions 1 and 2 use none. No
debugger probe: question 3 reads memory only, as A7m and C1f did.

## Decisions already made (do not re-ask)

- **B1 accepted on condition** (user, overlord review 2026-10-23): B1w's pass stands; question
  1 replaces the measured panel values with read ones and re-scores. A failing re-score
  reopens B1.
- **Next milestone is C2** (same review), ahead of B3. B2 level-to-map stays parked; a
  live UI-path capture is a candidate for a later directive, not this one.
- **The C2 gate cannot pass on this directive.** Its "no HUD exclusions" clause includes the
  ten skill-panel boxes, which are C1, and C1 stays held. The aim here is counters, timer
  and caption drawn, with only the skill-panel exclusion left. The cursor clause is not in
  scope.
- **Time the pass** (standing): the final report ends with total elapsed wall time and each
  round's dispatch-to-landing time.
- **Read, do not fit, and do not choose by score.** A value taken from the scored capture is
  labelled measured-from-reference and named in the report.
- Do not reuse heap addresses from an earlier process. Use `acquire_game_foreground`, never
  `goto_level.focus()`. Live steps run in session 1 per [MOUV_SSH.md](agents/MOUV_SSH.md).
  Close any game the round leaves running, after checking it.
- **Restoration:** restore and hash-check every install file in the same process that
  changed it.
- Settled and not to be re-derived: everything settled in the previous four directives,
  plus B1v (the `^Font` source is `LEMMINGS.BOX` `TEXTURES\^FONT.BMP` + `.ALF`) and B1w (centre
  anchor at `0x48A747`, glyph table `0x4BF968`, row rule 320 + 29 x row).
- English menus are the reference; do not reset the install language. Run CTest on mouv.
  Keep text files LF.

## Stop point

Stop and report when every question is landed or parked, on any stop condition in
ORCHESTRATION, or if the live start crashes or leaves the install unrestored. Do not
advance to B2, B3, C1 or C3, or to the C2 cursor clause, on this directive.

## Sync

Run from the Mac clone `~/progs/lemmings-webolution`. Pull before selecting work and push
after each round commit, per [ROLES.md](ROLES.md#opencode-on-the-mac-clone). If a push from
mouv fails on credentials, leave the commits local and say so in the handoff; the overlord
carries them to GitHub.
